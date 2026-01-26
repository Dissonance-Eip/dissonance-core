{
  "targets": [
    {
      "target_name": "dissonance_core",
      "sources": [
        "src/addon.cpp",
        "src/AddonHelpers.cpp",
          "src/GainProcessor.cpp",
        "src/WavUtils.cpp",
        "src/WavProcessor.cpp",
        "src/WindowFunctions.cpp",
        "src/FFTProcessor.cpp"
      ],
      "include_dirs": [
        "<!(node -p \"require('node-addon-api').include\")",
        "<(module_root_dir)/node_modules/node-addon-api",
        "<(module_root_dir)/include"
      ],
      "dependencies": [
        "<!(node -p \"require('node-addon-api').gyp\")"
      ],
      "defines": [
        "NAPI_CPP_EXCEPTIONS"
      ]
    }
  ]
}
