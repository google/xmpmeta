{
  'variables' : {
    'xmpmeta_dir': '<(DEPTH)/third_party/xmpmeta',
    'xml_dir': '<(xmpmeta_dir)/internal/xmpmeta/xml',
  },

  'target_defaults': {
    'type': 'static_library',
    'conditions': [
      ['OS=="win"', {
         'msvs_disabled_warnings': [
         '4267', # Conversion from size_t to int.
       ]}
      ]
    ],
  },

  'targets': [
    {
      'target_name': 'xml',
      'dependencies': [
        '<(xmpmeta_dir)/internal/xmpmeta/external/strings/strings.gyp:xmpmeta_strings',
        '<(DEPTH)/third_party/libxml/libxml.gyp:libxml',
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          '<(DEPTH)',
          '<(DEPTH)/third_party/libxml/src/include/',
          '<(xmpmeta_dir)/includes/',
          '<(xmpmeta_dir)/internal/',
          '<(xmpmeta_dir)/internal/xmpmeta/external',
          '<(xmpmeta_dir)/internal/xmpmeta/external/miniglog',
       ],
      },
      'include_dirs': [
        '<(DEPTH)',
        '<(DEPTH)/third_party/libxml/src/include/',
        '<(xmpmeta_dir)/includes/',
        '<(xmpmeta_dir)/internal/',
        '<(xmpmeta_dir)/internal/xmpmeta/external',
        '<(xmpmeta_dir)/internal/xmpmeta/external/miniglog',
      ],
      'sources': [
        '<(xml_dir)/const.cc',
        '<(xml_dir)/deserializer_impl.cc',
        '<(xml_dir)/search.cc',
        '<(xml_dir)/serializer_impl.cc',
        '<(xml_dir)/utils.cc',
      ],
    },
  ],  # targets.
}
