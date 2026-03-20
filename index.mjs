import { load } from 'node-gyp-build-esm';
import { createRequire } from 'node:module';
import { join } from 'node:path';

let binding;

// In a CJS bundle (esbuild/webpack), `require` is defined — esbuild replaces it
// with its own `__require` implementation, webpack with `__webpack_require__`.
// In a pure ESM environment (no bundler), `require` is undefined, so we fall back
// to `createRequire(import.meta.url)` which provides a CJS-style require anchored
// to the current module's path.
// Note: native .node addons cannot be loaded via ESM `import()` because
// `process.dlopen` is synchronous — `require` or `createRequire` is always needed.
//
// IMPORTANT: The `if/else` structure is intentional and must not be refactored
// into a ternary or other expression. Bundlers like esbuild and webpack statically
// analyze `typeof require === 'function'` as a known condition at build time,
// allowing them to tree-shake the `else` branch entirely. Collapsing this into
// an expression (e.g. `const _require = typeof require === 'function' ? require : ...`)
// breaks that static analyzability.
if (typeof require === 'function') {
  const isVite =
    typeof import.meta !== 'undefined' &&
    !!import.meta.env?.MODE &&
    !!import.meta.env.BASE_URL;

  if (isVite) {
    binding = load(import.meta.dirname, () => ({
      'linux-x64': () =>
        require(
          join(
            process.cwd(),
            'node_modules/procstat-napi/prebuilds/linux-x64+ia32/procstat-napi.node',
          ),
        ),
      'darwin-x64': () =>
        require(
          join(
            process.cwd(),
            'node_modules/procstat-napi/prebuilds/darwin-x64+arm64/procstat-napi.node',
          ),
        ),
      'win32-x64': () =>
        require(
          join(
            process.cwd(),
            'node_modules/procstat-napi/prebuilds/win32-x64+ia32/procstat-napi.node',
          ),
        ),
    }));
  } else {
    binding = load(import.meta.dirname, () => ({
      'linux-x64': () =>
        require(
          /* @vite-ignore */ './prebuilds/linux-x64+ia32/procstat-napi.node',
        ),
      'darwin-x64': () =>
        require(
          /* @vite-ignore */ './prebuilds/darwin-x64+arm64/procstat-napi.node',
        ),
      'win32-x64': () =>
        require(
          /* @vite-ignore */ './prebuilds/win32-x64+ia32/procstat-napi.node',
        ),
    }));
  }
} else {
  // `require` is block-scoped here intentionally — it avoids a duplicate
  // binding conflict with any `require` that a bundler may inject globally.
  const require = createRequire(import.meta.url);

  binding = load(import.meta.dirname, () => ({
    'linux-x64': () => require('./prebuilds/linux-x64+ia32/procstat-napi.node'),
    'darwin-x64': () =>
      require('./prebuilds/darwin-x64+arm64/procstat-napi.node'),
    'win32-x64': () => require('./prebuilds/win32-x64+ia32/procstat-napi.node'),
  }));
}

const createMonitor = binding.createMonitor;

export { createMonitor };
