{
  'variables' : {
    'xmpmeta_dir': '<(DEPTH)/third_party/xmpmeta/internal/xmpmeta',
  },
  'targets' : [
    {
      'target_name': 'xmpmeta_glog',
      'type': 'static_library',
      'sources': [
        'logging.cc',
      ],
      'include_dirs' : [
        '<(xmpmeta_dir)/external',
        '<(xmpmeta_dir)/external/miniglog',
      ],
      'conditions': [
        ['OS=="win"', {
          'msvs_disabled_warnings': [
            '4996', # Posix name for this item is deprecated.
          ]}
        ]
      ],
    },  # glog target.
  ],  # targets.
}
