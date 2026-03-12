import { load } from 'node-gyp-build-esm';
import { createRequire } from 'node:module';

// In a CJS bundle (esbuild/webpack), `require` is defined — esbuild replaces it
// with its own `__require` implementation, webpack with `__webpack_require__`.
// In a pure ESM environment (no bundler), `require` is undefined, so we fall back
// to `createRequire(import.meta.url)` which provides a CJS-style require anchored
// to the current module's path.
// Note: native .node addons cannot be loaded via ESM `import()` because
// `process.dlopen` is synchronous — `require` or `createRequire` is always needed.
const runtimeRequire =
  typeof require === 'function' ? require : createRequire(import.meta.url);

const binding = load(import.meta.dirname, () => ({
  'linux-x64': () =>
    runtimeRequire('./prebuilds/linux-x64+ia32/procstat-napi.node'),
  'darwin-x64': () =>
    runtimeRequire('./prebuilds/darwin-x64+arm64/procstat-napi.node'),
  'win32-x64': () =>
    runtimeRequire('./prebuilds/win32-x64+ia32/procstat-napi.node'),
}));

const getContextSwitches = binding.getContextSwitches;

export { getContextSwitches };
