#pragma once

#ifdef KWUI_STATIC_LIBRARY
#define KWUI_EXPORT

#else // KWUI_STATIC_LIBRARY

#ifdef KWUI_SHARED_LIBRARY

// Add visibility/export annotations when building the library.
#ifdef _WIN32
#define KWUI_EXPORT __declspec(dllexport)
#else
#define KWUI_EXPORT __attribute__((visibility("default")))
#endif

#else // KWUI_SHARED_LIBRARY

// Add import annotations when consuming the library.
#ifdef _WIN32
#define KWUI_EXPORT __declspec(dllimport)
#else
#define KWUI_EXPORT
#endif

#endif // KWUI_SHARED_LIBRARY

#endif // KWUI_STATIC_LIBRARY
