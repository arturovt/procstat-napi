#include <node_api.h>
#include <stdio.h>
#include <string.h>

typedef struct {
  unsigned long voluntary;
  unsigned long nonvoluntary;
} ContextSwitches;

static inline ContextSwitches read_context_switches(int pid) {
#ifndef __linux__
  // Apple: context switches not easily available without external lib
  // closest is proc_pidinfo() from <libproc.h>
  // Windows: no context switch API exposed in Win32 at per-process level
  // would need ETW (Event Tracing for Windows) — overkill
  // ❌ MSVC doesn't like this (ContextSwitches){0, 0}
  return ContextSwitches{0, 0};
#endif

  // /proc/[pid]/status is a Linux kernel virtual file exposing process
  // metadata. 64 bytes is enough for "/proc/" + max 10-digit PID + "/status" +
  // null terminator.
  char path[64];
  snprintf(path, sizeof(path), "/proc/%d/status", pid);

  ContextSwitches result = {0, 0};

  // 256 bytes is enough for any single line in /proc/[pid]/status —
  // the longest lines are well under 100 characters.
  char line[256];

  FILE* file = fopen(path, "r");
  // fopen fails if the process no longer exists — return zeros gracefully.
  if (!file) return result;

  while (fgets(line, sizeof(line), file)) {
    // strncmp compares only the first N characters so we match the key prefix
    // without caring about the value that follows on the same line.
    // 24 = strlen("voluntary_ctxt_switches:")
    if (strncmp(line, "voluntary_ctxt_switches:", 24) == 0) {
      sscanf(line, "voluntary_ctxt_switches: %lu", &result.voluntary);
    }
    // 27 = strlen("nonvoluntary_ctxt_switches:")
    else if (strncmp(line, "nonvoluntary_ctxt_switches:", 27) == 0) {
      sscanf(line, "nonvoluntary_ctxt_switches: %lu", &result.nonvoluntary);
    }
  }

  fclose(file);
  return result;
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
