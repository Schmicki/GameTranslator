project "zlib"
    kind "StaticLib"

    location "../../build"
    targetdir "../../bin/%{cfg.buildcfg}"
    language "C"

    filter "action:vs*"
        defines{"_WINSOCK_DEPRECATED_NO_WARNINGS", "_CRT_SECURE_NO_WARNINGS"}
        characterset ("MBCS")

    filter{}

    files {"./*.h", "./*.c"}