{
  'variables' : {
    'base_dir': '<(DEPTH)/base',
    'xmpmeta_dir': '<(DEPTH)/third_party/xmpmeta/internal/xmpmeta',
    'xmpmeta_strings_dir': '<(xmpmeta_dir)/external/strings',
    'xmpmeta_xml_dir': '<(xmpmeta_dir)/xml',
  },

  'target_defaults': {
    'type': 'static_library',
    'conditions': [
      ['OS=="win"', {
        'msvs_disabled_warnings': [
          '4267', # Conversion from size_t to int.
          '4996', # Use of function declared deprecated.
        ],
      }],
    ],
  },

  'targets': [
    {
      'target_name': 'xmpmeta',
      'dependencies': [
        '<(xmpmeta_xml_dir)/xml.gyp:xml',
        '<(xmpmeta_strings_dir)/strings.gyp:xmpmeta_strings',
        '<(DEPTH)/third_party/libxml/libxml.gyp:libxml',
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          '<(DEPTH)/third_party/libxml/src/include/',
          '<(DEPTH)/third_party/xmpmeta/includes/',
          '<(DEPTH)/third_party/xmpmeta/internal/',
          '<(DEPTH)/third_party/xmpmeta/internal/external/',
          '<(DEPTH)/third_party/xmpmeta/internal/external/miniglog/',
        ],
      },
      'include_dirs': [
        '<(DEPTH)/third_party/libxml/src/include/',
        '<(DEPTH)/third_party/xmpmeta/includes/',
        '<(DEPTH)/third_party/xmpmeta/internal/',
        '<(DEPTH)/third_party/xmpmeta/internal/external/',
        '<(DEPTH)/third_party/xmpmeta/internal/external/miniglog/',
      ],
      'sources': [
        '<(xmpmeta_dir)/base64.cc',
        '<(xmpmeta_dir)/gaudio.cc',
        '<(xmpmeta_dir)/gimage.cc',
        '<(xmpmeta_dir)/gpano.cc',
        '<(xmpmeta_dir)/jpeg_io.cc',
        '<(xmpmeta_dir)/md5.cc',
        '<(xmpmeta_dir)/vr_photo_writer.cc',
        '<(xmpmeta_dir)/xmp_const.cc',
        '<(xmpmeta_dir)/xmp_data.cc',
        '<(xmpmeta_dir)/xmp_parser.cc',
        '<(xmpmeta_dir)/xmp_writer.cc',
      ]
    },
  ],
}
