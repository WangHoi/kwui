#ifndef WINDOWS_HEADER_H
#define WINDOWS_HEADER_H
// Exclude rarely-used stuff from Windows headers
#define WIN32_LEAN_AND_MEAN 
#define NOGDICAPMASKS
#define NOMENUS
#define NOICONS
//#define NOKEYSTATES
//#define NOSYSCOMMANDS
#define NORASTEROPS
//#define NOSHOWWINDOW
#define OEMRESOURCE
#define NOATOM
//#define NOCLIPBOARD
//#define NOCTLMGR
#define NODRAWTEXT
//#define NOGDI
//#define NOMB
#define NOMEMMGR
#define NOMETAFILE
#ifndef NOMINMAX
#define NOMINMAX
#endif
#define NOOPENFILE
#define NOSCROLL
#define NOSERVICE
#define NOSOUND
#define NOTEXTMETRIC
//#define NOWH
//#define NOWINOFFSETS
#define NOCOMM
#define NOKANJI
#define NOHELP
#define NOPROFILER
//#define NODEFERWINDOWPOS
#define NOMCX
#include <windowsx.h>
#include <ShellScalingApi.h>
#include <guiddef.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <dxgi.h>
#include <d2d1.h>
#include <dwrite.h>
#include <wincodec.h>
#include <imm.h>
#pragma warning(disable:5043)
#include <wrl.h>
#pragma warning(default:5043)
#include <ShlObj.h>
using Microsoft::WRL::ComPtr;
namespace WRL = Microsoft::WRL;
// #undef near
// #undef far
#undef GetWindowFont
#undef Yield
#undef GetFirstChild
#endif // WINDOWS_HEADER_H