#include <napi.h>
#include <uv.h>

#include <map>
#include <string>
#include <vector>

#if defined(__linux__) || defined(__APPLE__)
#include <sys/resource.h>
#endif

class Monitor;

#if defined(__SANITIZE_ADDRESS__)
#include <sanitizer/asan_interface.h>

// g_pending_report is written on ASan's thread (OnAsanReport) and read on the
// uv event loop thread (OnAsyncLeak).  In practice a leak report is terminal
// — the process is about to exit — so a plain std::string plus the async
// signal is sufficient; no mutex needed for this use case.
static std::string g_pending_report;
static bool g_async_initialized = false;
static uv_async_t g_async;
static std::vector<Monitor*> g_monitors;

// Called by ASan on its own internal thread immediately after writing the
// report.  Must not touch the uv loop directly — signal it instead.
static void OnAsanReport(const char* report) {
  g_pending_report = report;
  if (uv_is_active(reinterpret_cast<uv_handle_t*>(&g_async))) {
    uv_async_send(&g_async);
  }
}

// OnAsyncLeak is defined after class Monitor since it dereferences Monitor
// members. Forward-declared here so On() can reference it by name.
static void OnAsyncLeak(uv_async_t* handle);
#endif

class Monitor : public Napi::ObjectWrap<Monitor> {
 private:
#if defined(__SANITIZE_ADDRESS__)
  friend void OnAsyncLeak(uv_async_t*);
#endif

  uv_timer_t timer_;
  uv_loop_t* loop_;

  uint32_t intervalMs_;
  std::map<std::string, std::vector<Napi::FunctionReference>> listeners_;
  napi_env env_;

  void DisposeTimer() {
    if (!uv_is_closing(reinterpret_cast<uv_handle_t*>(&timer_))) {
      uv_timer_stop(&timer_);
      uv_close(reinterpret_cast<uv_handle_t*>(&timer_), nullptr);
    }
  }

#if defined(__SANITIZE_ADDRESS__)
  // Static because g_async and g_async_initialized are module-level globals —
  // not owned by any single Monitor instance.
  static void DisposeAsync() {
    if (!g_async_initialized) return;
    __asan_set_error_report_callback(nullptr);
    if (!uv_is_closing(reinterpret_cast<uv_handle_t*>(&g_async))) {
      uv_close(reinterpret_cast<uv_handle_t*>(&g_async), nullptr);
    }
    g_async_initialized = false;
  }
#endif

  static void OnTick(uv_timer_t* handle) {
    Monitor* self = static_cast<Monitor*>(handle->data);

    Napi::Env env = Napi::Env(self->env_);
    Napi::HandleScope scope(env);

#if defined(__linux__) || defined(__APPLE__)
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);

    Napi::Object stats = Napi::Object::New(env);

    // Context switches
    stats.Set("voluntaryContextSwitches",
              Napi::Number::New(env, usage.ru_nvcsw));
    stats.Set("involuntaryContextSwitches",
              Napi::Number::New(env, usage.ru_nivcsw));

    // CPU times in seconds
    stats.Set("userCpuTime",
              Napi::Number::New(
                  env, usage.ru_utime.tv_sec + usage.ru_utime.tv_usec / 1e6));
    stats.Set("systemCpuTime",
              Napi::Number::New(
                  env, usage.ru_stime.tv_sec + usage.ru_stime.tv_usec / 1e6));

    // Memory
    stats.Set("maxRss", Napi::Number::New(env, usage.ru_maxrss));

    // Page faults
    stats.Set("minorFaults", Napi::Number::New(env, usage.ru_minflt));
    stats.Set("majorFaults", Napi::Number::New(env, usage.ru_majflt));

    // I/O
    stats.Set("blockInputOps", Napi::Number::New(env, usage.ru_inblock));
    stats.Set("blockOutputOps", Napi::Number::New(env, usage.ru_oublock));

    for (Napi::FunctionReference& fn : self->listeners_["stats"]) {
      fn.Call({stats});
    }
#endif
  }

 public:
  static Napi::Function Init(Napi::Env env) {
    return DefineClass(env, "Monitor",
                       {InstanceMethod("on", &Monitor::On),
                        InstanceMethod("off", &Monitor::Off)});
  }

  Monitor(const Napi::CallbackInfo& info) : Napi::ObjectWrap<Monitor>(info) {
    const Napi::Object options = info[0].As<Napi::Object>();
    intervalMs_ = options.Get("intervalMs").As<Napi::Number>().Uint32Value();
    env_ = info.Env();

    napi_get_uv_event_loop(info.Env(), &loop_);
    uv_timer_init(loop_, &timer_);
    timer_.data = this;

#if defined(__SANITIZE_ADDRESS__)
    g_monitors.push_back(this);
#endif
  }

  ~Monitor() {
    listeners_.clear();
    DisposeTimer();
#if defined(__SANITIZE_ADDRESS__)
    g_monitors.erase(std::remove(g_monitors.begin(), g_monitors.end(), this),
                     g_monitors.end());
#endif
  }

  Napi::Value On(const Napi::CallbackInfo& info) {
    std::string event = info[0].As<Napi::String>();
    Napi::Function callback = info[1].As<Napi::Function>();
    listeners_[event].push_back(Napi::Persistent(callback));
    if (event == "stats") {
      // Start a timer only once.
      if (!uv_is_active(reinterpret_cast<uv_handle_t*>(&timer_))) {
        uv_timer_start(&timer_, OnTick, 0, intervalMs_);
      }
    } else if (event == "leak") {
#if defined(__SANITIZE_ADDRESS__)
      if (!g_async_initialized) {
        uv_async_init(loop_, &g_async, OnAsyncLeak);
        __asan_set_error_report_callback(OnAsanReport);
        g_async_initialized = true;
      }
#endif
    }
    return info.Env().Undefined();
  }

  Napi::Value Off(const Napi::CallbackInfo& info) {
    std::string event = info[0].As<Napi::String>();
    Napi::Function callback = info[1].As<Napi::Function>();

    // Remove the specific callback from the listener list for this event.
    auto it = listeners_.find(event);
    if (it != listeners_.end()) {
      auto& vec = it->second;
      vec.erase(
          std::remove_if(vec.begin(), vec.end(),
                         [&](const Napi::FunctionReference& ref) {
                           return ref.Value() == callback;
                         }),
          vec.end());

      if (event == "stats") {
        // Stop the timer only once there are no remaining stats listeners.
        if (vec.empty()) DisposeTimer();
      } else if (event == "leak") {
#if defined(__SANITIZE_ADDRESS__)
        // Tear down the shared async handle only when no monitor anywhere
        // still has a leak listener — it is a module-level resource.
        bool anyLeak = false;
        for (const Monitor* m : g_monitors) {
          auto lit = m->listeners_.find("leak");
          if (lit != m->listeners_.end() && !lit->second.empty()) {
            anyLeak = true;
            break;
          }
        }
        if (!anyLeak) DisposeAsync();
#endif
      }
    }

    return info.Env().Undefined();
  }
};

#if defined(__SANITIZE_ADDRESS__)
// Defined here — after class Monitor — because it dereferences monitor->env_
// and monitor->listeners_, which require the complete type.
static void OnAsyncLeak(uv_async_t* /* handle */) {
  const std::string report = g_pending_report;
  for (Monitor* monitor : g_monitors) {
    Napi::Env env = Napi::Env(monitor->env_);
    Napi::HandleScope scope(env);
    for (Napi::FunctionReference& fn : monitor->listeners_["leak"]) {
      fn.Call({Napi::String::New(env, report)});
    }
  }
}
#endif

Napi::Value CreateMonitor(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  Napi::Function ctor = Monitor::Init(env);
  return ctor.New({info[0]});
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
  exports.Set(Napi::String::New(env, "createMonitor"),
              Napi::Function::New<CreateMonitor>(env));
  return exports;
}

NODE_API_MODULE(NODE_GYP_MODULE_NAME, Init)
