import { load } from 'node-gyp-build-esm';

// In a CJS bundle (esbuild/webpack), `require` is defined — esbuild replaces it
// with its own `__require` implementation, webpack with `__webpack_require__`.
// Note: native .node addons cannot be loaded via ESM `import()` because
// `process.dlopen` is synchronous — `require` or `createRequire` is always needed.
const binding = load(import.meta.dirname, () => ({
  // In ESM environments, the caller is responsible for ensuring `require` is
  // available — e.g. via a banner that polyfills it with `createRequire`:
  //   globalThis["require"] ??= createRequire(import.meta.url);
  'linux-x64': () =>
    require(/* @vite-ignore */ './prebuilds/linux-x64+ia32/procstat-napi.node'),
  'darwin-x64': () =>
    require(
      /* @vite-ignore */ './prebuilds/darwin-x64+arm64/procstat-napi.node',
    ),
  'win32-x64': () =>
    require(/* @vite-ignore */ './prebuilds/win32-x64+ia32/procstat-napi.node'),
}));

const getContextSwitches = binding.getContextSwitches;

export { getContextSwitches };
