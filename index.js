'use strict';

const binding = require('node-gyp-build')(__dirname);

module.exports = { getContextSwitches: binding.getContextSwitches };
