const { load } = require('node-gyp-build-esm');

const binding = load(__dirname, () => ({
  'linux-x64': () => require('./prebuilds/linux-x64+ia32/procstat-napi.node'),
  'darwin-x64': () =>
    require('./prebuilds/darwin-x64+arm64/procstat-napi.node'),
  'win32-x64': () => require('./prebuilds/win32-x64+ia32/procstat-napi.node'),
}));

module.exports = { xml2json: binding.xml2json };
