#pragma once

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#if defined(_WIN32) || defined(_WIN64)
#define PLATFORM_WINDOWS
#elif defined(__linux__)
#define PLATFORM_LINUX
#elif defined(__APPLE__) && defined(__MACH__)
#define PLATFORM_MACOS
#else
#define PLATFORM_UNKNOWN
#endif
