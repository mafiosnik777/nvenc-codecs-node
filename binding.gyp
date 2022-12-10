{
  "targets": [
    {
      "target_name": "nvenc_codecs_node",
      "type": "executable",
      "sources": [
        "./nvenc_codecs.c"
      ],
      "include_dirs": [
        "./include",
      ],
      "cflags!": [
        "-fno-exceptions"
      ],
      "cflags_cc!": [
        "-fno-exceptions"
      ],
      "conditions": [
        [
          "OS=='win'",
          {
            "defines": [
              "_CRT_SECURE_NO_WARNINGS"
            ]
          }
        ]
      ],
      "xcode_settings": {
        "GCC_ENABLE_CPP_EXCEPTIONS": "NO"
      },
      "link_settings": {
        "libraries": [
        ]
      },
      "link_flags": [
        "-static-libgcc"
      ],
      "msvs_settings": {
        "VCCLCompilerTool": {
          "ExceptionHandling": 0
        }
      }
    }
  ]
}