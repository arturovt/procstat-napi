#include <node_api.h>
#include <sys/resource.h>

typedef struct {
  long voluntary;
  long nonvoluntary;
} ContextSwitches;

static inline ContextSwitches read_context_switches_linux(int pid) {
  struct rusage usage;
  getrusage(RUSAGE_SELF, &usage);
  ContextSwitches result = {.voluntary = usage.ru_nvcsw,
                            .nonvoluntary = usage.ru_nivcsw};
  return result;
}

static inline ContextSwitches read_context_switches(int pid) {
#ifdef __linux__
  return read_context_switches_linux(pid);
#else
  // Apple: context switches not easily available without external lib
  // closest is proc_pidinfo() from <libproc.h>
  // Windows: no context switch API exposed in Win32 at per-process level
  // would need ETW (Event Tracing for Windows) — overkill
  return ContextSwitches{0, 0};
#endif
}

napi_value get_context_switches(napi_env env, napi_callback_info info) {
  size_t argc = 1;
  napi_value argv[1];
  napi_get_cb_info(env, info, &argc, argv, NULL, NULL);

  if (argc < 1) {
    napi_throw_error(env, NULL, "expected 1 argument");
    return nullptr;
  }

  napi_valuetype pid_type;
  napi_typeof(env, argv[0], &pid_type);

  if (pid_type != napi_number) {
    napi_throw_error(env, NULL, "pid should be a number");
    return nullptr;
  }

  int pid;
  napi_get_value_int32(env, argv[0], &pid);

  const ContextSwitches context_switches = read_context_switches(pid);

  napi_value result, voluntary, nonvoluntary;
  napi_create_object(env, &result);
  napi_create_int64(env, context_switches.voluntary, &voluntary);
  napi_create_int64(env, context_switches.nonvoluntary, &nonvoluntary);
  napi_set_named_property(env, result, "voluntary", voluntary);
  napi_set_named_property(env, result, "nonvoluntary", nonvoluntary);

  return result;
}

napi_value init(napi_env env, napi_value exports) {
  napi_value get_context_switches_fn;
  napi_create_function(env, NULL, NAPI_AUTO_LENGTH, get_context_switches, NULL,
                       &get_context_switches_fn);
  napi_set_named_property(env, exports, "getContextSwitches",
                          get_context_switches_fn);

  return exports;
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, init)
