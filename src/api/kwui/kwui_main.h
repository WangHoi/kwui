#pragma once

#ifndef KWUI_MAIN
#ifdef __ANDROID__
#ifdef __cplusplus
#define KWUI_MAIN() extern "C" int __attribute__((visibility("default"))) kwui_main(int argc, char* argv[])
#else
#define KWUI_MAIN() int __attribute__((visibility("default"))) kwui_main(int argc, char* argv[])
#endif
#else
#define KWUI_MAIN() int main(int argc, char* argv[])
#endif
#endif
