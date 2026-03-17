const { load } = require('node-gyp-build-esm');

const binding = load(__dirname, () => ({
  'linux-x64': () =>
    require(/* @vite-ignore */ './prebuilds/linux-x64+ia32/procstat-napi.node'),
  'darwin-x64': () =>
    require(
      /* @vite-ignore */ './prebuilds/darwin-x64+arm64/procstat-napi.node',
    ),
  'win32-x64': () =>
    require(/* @vite-ignore */ './prebuilds/win32-x64+ia32/procstat-napi.node'),
}));

module.exports = { createMonitor: binding.createMonitor };
