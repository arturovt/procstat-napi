#include <napi.h>
#include <uv.h>

#if defined(__linux__) || defined(__APPLE__)
#include <sys/resource.h>
#endif

class Monitor : public Napi::ObjectWrap<Monitor> {
 private:
  uv_timer_t timer_;
  uint32_t intervalMs_;
  Napi::FunctionReference callback_;

  static void OnTick(uv_timer_t* handle) {
    Monitor* self = static_cast<Monitor*>(handle->data);

    Napi::Env env = self->callback_.Env();
    Napi::HandleScope scope(env);

    // Metrics we're reading:
    // - Context switches
    // - CPU times
    // - Memory / RSS
    // - Page faults
    // - I/O bytes
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

    self->callback_.Call({stats});
#endif
  }

 public:
  static Napi::Function Init(Napi::Env env) {
    return DefineClass(env, "Monitor",
                       {
                           InstanceMethod("stop", &Monitor::Stop),
                           InstanceMethod("start", &Monitor::Start),
                       });
  }

  Monitor(const Napi::CallbackInfo& info) : Napi::ObjectWrap<Monitor>(info) {
    const Napi::Object options = info[0].As<Napi::Object>();
    uint32_t intervalMs = 1000;
    if (options.Has("intervalMs")) {
      intervalMs = options.Get("intervalMs").As<Napi::Number>().Uint32Value();
    }
    intervalMs_ = intervalMs;

    uv_loop_t* loop;
    napi_get_uv_event_loop(info.Env(), &loop);
    uv_timer_init(loop, &timer_);
    timer_.data = this;
  }

  ~Monitor() {
    if (!uv_is_closing(reinterpret_cast<uv_handle_t*>(&timer_))) {
      uv_close(reinterpret_cast<uv_handle_t*>(&timer_), nullptr);
    }
  }

  Napi::Value Stop(const Napi::CallbackInfo& info) {
    if (!uv_is_closing(reinterpret_cast<uv_handle_t*>(&timer_))) {
      uv_timer_stop(&timer_);
      uv_close(reinterpret_cast<uv_handle_t*>(&timer_), nullptr);
    }
    return info.Env().Undefined();
  }

  Napi::Value Start(const Napi::CallbackInfo& info) {
    callback_ = Napi::Persistent(info[0].As<Napi::Function>());
    uv_timer_start(&timer_, OnTick, 0, intervalMs_);
    return info.Env().Undefined();  // missing
  }
};

Napi::Value CreateMonitor(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  Napi::Function ctor = Monitor::Init(env);
  return ctor.New({info[0]});
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
  exports.Set(Napi::String::New(env, "createMonitor"),
              Napi::Function::New<CreateMonitor>(env));  // was Fn
  return exports;
}

NODE_API_MODULE(NODE_GYP_MODULE_NAME, Init)
