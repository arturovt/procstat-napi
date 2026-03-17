{
  'variables': {
    'openssl_fips': '0'
  },
  'targets': [
    {
      'target_name': 'node_procstat',
      "include_dirs": [
        "<!@(node -p \"require('node-addon-api').include\")"
      ],
      'dependencies': [
        "<!(node -p \"require('node-addon-api').gyp\")"
      ],
      'defines': ['NDEBUG'],
      'sources': [
        'src/node_procstat.cc'
      ],
      'libraries': [],
      'cflags_cc': ['-std=c++17', '-fexceptions', '-O3', '-Wall', '-Wextra'],
      'defines': ['NAPI_DISABLE_CPP_EXCEPTIONS'],
      'conditions': [
        ['OS == "mac"', {
          'xcode_settings': {
            'GCC_ENABLE_CPP_EXCEPTIONS': 'YES'
          }
        }]
      ]
    }
  ]
}
