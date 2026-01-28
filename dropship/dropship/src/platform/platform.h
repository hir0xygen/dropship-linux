#pragma once

// Platform detection macros
#if defined(_WIN32) || defined(_WIN64)
    #define DROPSHIP_WINDOWS 1
    #define DROPSHIP_LINUX 0
#elif defined(__linux__)
    #define DROPSHIP_WINDOWS 0
    #define DROPSHIP_LINUX 1
#else
    #error "Unsupported platform"
#endif

// Platform-specific includes
#if DROPSHIP_WINDOWS
    #define NOMINMAX
    #define WIN32_LEAN_AND_MEAN
    #include <Windows.h>
    #include <atlcomcli.h>
#endif

#if DROPSHIP_LINUX
    #include <unistd.h>
    #include <sys/types.h>
#endif
