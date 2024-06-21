{
    "targets": [
        {
            "target_name": "pgmjs",
            "sources": ["src/pgm.c", "src.cc"],
            "include_dirs": [
                "<!@(node -p \"require('node-addon-api').include\")",
                "src",
            ],
            "defines": ["NAPI_DISABLE_CPP_EXCEPTIONS"],
        }
    ]
}
