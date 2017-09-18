{
  "targets": [
    {
      "target_name": "nhdfs",
      "sources": [ 
        "src/nhdfs.cc", 
        "src/filereader.cc", 
        "src/filewriter.cc", 
        "src/filesystem.cc",
        "src/clusterinfo.cc"  
      ],
      "dependencies": ["<!(node -p \"require('node-addon-api').gyp\")"],
      "cflags!": [ "-fno-exceptions" ],
      "cflags_cc!": [ "-fno-exceptions" ],
      "include_dirs": [
        "<!@(node -p \"require('node-addon-api').include\")",
        "include", "build_deps/libhdfs3/dist/include"
      ],
      "xcode_settings": {
          "GCC_ENABLE_CPP_EXCEPTIONS": "YES",
          "CLANG_CXX_LIBRARY": "libc++",
          "OTHER_LDFLAGS": [
                '-Wl,-rpath,<(module_root_dir)/build_deps/libhdfs3/dist/lib' 
            ]
      }, 
      "link_settings": {
        "libraries": [
          "-lhdfs3",
          "-L<(module_root_dir)/build_deps/libhdfs3/dist/lib",
        ],
        'ldflags': [
          '-Wl,-rpath,<(module_root_dir)/build_deps/libhdfs3/dist/lib',
        ],
      },
      'cflags!': [ '-fno-exceptions' ],
	    'cflags_cc!': [ '-fno-exceptions' ],
    }	],
}
