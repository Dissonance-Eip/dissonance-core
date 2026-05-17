{
  "targets": [
    {
      "target_name": "dissonance_core",
      "sources": [
        "src/addon/addon.cpp",
        "src/utils/WavParser.cpp",
        "src/utils/WavUtils.cpp",
        "src/audio/GainStage.cpp",
        "src/audio/Pipeline.cpp",
        "src/audio/WavProcessor.cpp",
        "src/audio/WindowedFFTStage.cpp",
        "src/audio/WindowFunctions.cpp",
        "src/audio/FFTProcessor.cpp",
        "vendor/kissfft/kiss_fft.c"
      ],
      "include_dirs": [
        "<!(node -p \"require('node-addon-api').include\")",
        "<(module_root_dir)/node_modules/node-addon-api",
        "<(module_root_dir)/include",
        "<(module_root_dir)/vendor/kissfft"
      ],
      "dependencies": [
        "<!(node -p \"require('node-addon-api').gyp\")"
      ],
      "defines": [
        "NAPI_CPP_EXCEPTIONS"
      ],
      "cflags": ["-fexceptions"],
      "cflags_cc": ["-fexceptions"],
      "xcode_settings": {
        "GCC_ENABLE_CPP_EXCEPTIONS": "YES"
      },
      "conditions": [
        ["OS == 'win'", {
          "msvs_settings": {
            "VCCLCompilerTool": {
              "ExceptionHandling": 1
            }
          }
        }]
      ]
    }
  ]
}
