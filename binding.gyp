{
  'variables': {
    'openssl_fips': '0'
  },
  'targets': [
    {
      'target_name': 'node_procstat',
      'include_dirs': [
        "<!@(node -p \"require('node-addon-api').include\")"
      ],
      'dependencies': [
        "<!(node -p \"require('node-addon-api').gyp\")"
      ],
      'sources': [
        'src/node_procstat.cc'
      ],
      'libraries': [],
      'cflags_cc': [
        '-std=c++17',
        '-fexceptions',
        '-O3',
        '-Wall',
        '-Wextra',
        '-fsanitize=address',
        '-fno-omit-frame-pointer'
      ],
      'ldflags': [
        '-fsanitize=address'
      ],
      'defines': [
        'NAPI_DISABLE_CPP_EXCEPTIONS',
        'NDEBUG',
        '__SANITIZE_ADDRESS__'
      ],
      'conditions': [
        ['OS == "mac"', {
          'xcode_settings': {
            'GCC_ENABLE_CPP_EXCEPTIONS': 'YES',
            'OTHER_CFLAGS': [
              '-fsanitize=address',
              '-fno-omit-frame-pointer'
            ],
            'OTHER_LDFLAGS': [
              '-fsanitize=address'
            ]
          }
        }]
      ]
    }
  ]
}
