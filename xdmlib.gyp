
{
  'variables' : {
    'xdmlib_dir': '<(DEPTH)/third_party/xmpmeta/internal/xdmlib',
    'xmpmeta_dir': '<(DEPTH)/third_party/xmpmeta/internal/xmpmeta',
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
      'target_name': 'xdmlib',
      'dependencies': [
        '<(DEPTH)/third_party/libxml/libxml.gyp:libxml',
        '<(DEPTH)/third_party/xmpmeta/xmpmeta.gyp:xmpmeta',
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          '<(DEPTH)/third_party/libxml/src/include/',
          '<(DEPTH)/third_party/xmpmeta/google3/',
          '<(DEPTH)/third_party/xmpmeta/includes/',
          '<(DEPTH)/third_party/xmpmeta/internal/',
        ],
      },
      'include_dirs': [
        '<(DEPTH)/third_party/libxml/src/include/',
        '<(DEPTH)/third_party/xmpmeta/google3/',
        '<(DEPTH)/third_party/xmpmeta/includes/',
        '<(DEPTH)/third_party/xmpmeta/internal/',
      ],
      'sources': [
        '<(xdmlib_dir)/audio.cc',
        '<(xdmlib_dir)/camera.cc',
        '<(xdmlib_dir)/cameras.cc',
        '<(xdmlib_dir)/camera_pose.cc',
        '<(xdmlib_dir)/const.cc',
        '<(xdmlib_dir)/device.cc',
        '<(xdmlib_dir)/device_pose.cc',
        '<(xdmlib_dir)/equirect_model.cc',
        '<(xdmlib_dir)/image.cc',
        '<(xdmlib_dir)/point_cloud.cc',
        '<(xdmlib_dir)/profile.cc',
        '<(xdmlib_dir)/profiles.cc',
      ],
    },
  ],  # targets.
}
