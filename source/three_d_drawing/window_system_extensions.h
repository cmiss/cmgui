/*******************************************************************************
FILE : graphics_buffer.cpp

LAST MODIFIED : 19 February 2007

DESCRIPTION :
This provides a Cmgui interface to the OpenGL contexts of many types.
******************************************************************************/
/* OpenCMISS-Cmgui Application
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#if !defined (WINDOW_SYSTEM_EXTENSIONS_H)
#define WINDOW_SYSTEM_EXTENSIONS_H

#if defined (WIN32_USER_INTERFACE)

//#define WINDOWS_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

int Window_system_extensions_load_wgl_extension(const char *extension_name);
/*******************************************************************************
LAST MODIFIED : 3 October 2007

DESCRIPTION :
Attempts to load a single wgl openGL extension.
If the extension symbol and all the functions used in cmgui from that extension are
found then returns WGLEXTENSION_AVAILABLE.
If the extension list is defined but this extension is not available then
it returns WGLEXTENSION_UNAVAILABLE.
If the extension string is not yet defined then the test is not definitive and
so it returns WGLEXTENSION_UNSURE, allowing the calling procedure to react
appropriately.
==============================================================================*/

#  define WGLHANDLE( function_name ) function_name ## _handle

#define WINDOW_SYSTEM_EXTENSIONS_ASSIGN_HANDLE(function,type) function ## _handle = (type)

#define WGLEXTENSION_UNSURE (255)
#define WGLEXTENSION_AVAILABLE (1)
#define WGLEXTENSION_UNAVAILABLE (0)

#  define WGLEXTENSIONFLAG( extension_name ) extension_name ## _wglextension_flag
#  define Window_system_extensions_check_wgl_extension(extension_name) \
	(WGLEXTENSION_UNSURE == extension_name ## _wglextension_flag ? (WGLEXTENSION_AVAILABLE == Window_system_extensions_load_wgl_extension( #extension_name)) : extension_name ## _wglextension_flag)
/*******************************************************************************
LAST MODIFIED : 3 October 2007

DESCRIPTION :
Ensure that the extension is available.
Use before calling functions from that extension.
==============================================================================*/

#define WGL_WGLEXT_PROTOTYPES

/* Define the pbuffer extension here as Mesa does not include the WGL extension header
	at the moment. */
#ifndef WGL_ARB_pixel_format
#define WGL_NUMBER_PIXEL_FORMATS_ARB   0x2000
#define WGL_DRAW_TO_WINDOW_ARB         0x2001
#define WGL_DRAW_TO_BITMAP_ARB         0x2002
#define WGL_ACCELERATION_ARB           0x2003
#define WGL_NEED_PALETTE_ARB           0x2004
#define WGL_NEED_SYSTEM_PALETTE_ARB    0x2005
#define WGL_SWAP_LAYER_BUFFERS_ARB     0x2006
#define WGL_SWAP_METHOD_ARB            0x2007
#define WGL_NUMBER_OVERLAYS_ARB        0x2008
#define WGL_NUMBER_UNDERLAYS_ARB       0x2009
#define WGL_TRANSPARENT_ARB            0x200A
#define WGL_TRANSPARENT_RED_VALUE_ARB  0x2037
#define WGL_TRANSPARENT_GREEN_VALUE_ARB 0x2038
#define WGL_TRANSPARENT_BLUE_VALUE_ARB 0x2039
#define WGL_TRANSPARENT_ALPHA_VALUE_ARB 0x203A
#define WGL_TRANSPARENT_INDEX_VALUE_ARB 0x203B
#define WGL_SHARE_DEPTH_ARB            0x200C
#define WGL_SHARE_STENCIL_ARB          0x200D
#define WGL_SHARE_ACCUM_ARB            0x200E
#define WGL_SUPPORT_GDI_ARB            0x200F
#define WGL_SUPPORT_OPENGL_ARB         0x2010
#define WGL_DOUBLE_BUFFER_ARB          0x2011
#define WGL_STEREO_ARB                 0x2012
#define WGL_PIXEL_TYPE_ARB             0x2013
#define WGL_COLOR_BITS_ARB             0x2014
#define WGL_RED_BITS_ARB               0x2015
#define WGL_RED_SHIFT_ARB              0x2016
#define WGL_GREEN_BITS_ARB             0x2017
#define WGL_GREEN_SHIFT_ARB            0x2018
#define WGL_BLUE_BITS_ARB              0x2019
#define WGL_BLUE_SHIFT_ARB             0x201A
#define WGL_ALPHA_BITS_ARB             0x201B
#define WGL_ALPHA_SHIFT_ARB            0x201C
#define WGL_ACCUM_BITS_ARB             0x201D
#define WGL_ACCUM_RED_BITS_ARB         0x201E
#define WGL_ACCUM_GREEN_BITS_ARB       0x201F
#define WGL_ACCUM_BLUE_BITS_ARB        0x2020
#define WGL_ACCUM_ALPHA_BITS_ARB       0x2021
#define WGL_DEPTH_BITS_ARB             0x2022
#define WGL_STENCIL_BITS_ARB           0x2023
#define WGL_AUX_BUFFERS_ARB            0x2024
#define WGL_NO_ACCELERATION_ARB        0x2025
#define WGL_GENERIC_ACCELERATION_ARB   0x2026
#define WGL_FULL_ACCELERATION_ARB      0x2027
#define WGL_SWAP_EXCHANGE_ARB          0x2028
#define WGL_SWAP_COPY_ARB              0x2029
#define WGL_SWAP_UNDEFINED_ARB         0x202A
#define WGL_TYPE_RGBA_ARB              0x202B
#define WGL_TYPE_COLORINDEX_ARB        0x202C
#endif

#ifndef WGL_ARB_pbuffer
#define WGL_DRAW_TO_PBUFFER_ARB        0x202D
#define WGL_MAX_PBUFFER_PIXELS_ARB     0x202E
#define WGL_MAX_PBUFFER_WIDTH_ARB      0x202F
#define WGL_MAX_PBUFFER_HEIGHT_ARB     0x2030
#define WGL_PBUFFER_LARGEST_ARB        0x2033
#define WGL_PBUFFER_WIDTH_ARB          0x2034
#define WGL_PBUFFER_HEIGHT_ARB         0x2035
#define WGL_PBUFFER_LOST_ARB           0x2036
#endif

#ifndef WGL_ARB_pixel_format
#define WGL_ARB_pixel_format 1
#ifdef WGL_WGLEXT_PROTOTYPES
extern BOOL WINAPI wglGetPixelFormatAttribivARB (HDC, int, int, UINT, const int *, int *);
extern BOOL WINAPI wglGetPixelFormatAttribfvARB (HDC, int, int, UINT, const int *, FLOAT *);
extern BOOL WINAPI wglChoosePixelFormatARB (HDC, const int *, const FLOAT *, UINT, int *, UINT *);
#endif /* WGL_WGLEXT_PROTOTYPES */
typedef BOOL (WINAPI * PFNWGLGETPIXELFORMATATTRIBIVARBPROC) (HDC hdc, int iPixelFormat, int iLayerPlane, UINT nAttributes, const int *piAttributes, int *piValues);
typedef BOOL (WINAPI * PFNWGLGETPIXELFORMATATTRIBFVARBPROC) (HDC hdc, int iPixelFormat, int iLayerPlane, UINT nAttributes, const int *piAttributes, FLOAT *pfValues);
typedef BOOL (WINAPI * PFNWGLCHOOSEPIXELFORMATARBPROC) (HDC hdc, const int *piAttribIList, const FLOAT *pfAttribFList, UINT nMaxFormats, int *piFormats, UINT *nNumFormats);
#endif

#ifndef WGL_ARB_pbuffer
DECLARE_HANDLE(HPBUFFERARB);
#endif

#ifndef WGL_ARB_pbuffer
#define WGL_ARB_pbuffer 1
#ifdef WGL_WGLEXT_PROTOTYPES
extern HPBUFFERARB WINAPI wglCreatePbufferARB (HDC, int, int, int, const int *);
extern HDC WINAPI wglGetPbufferDCARB (HPBUFFERARB);
extern int WINAPI wglReleasePbufferDCARB (HPBUFFERARB, HDC);
extern BOOL WINAPI wglDestroyPbufferARB (HPBUFFERARB);
extern BOOL WINAPI wglQueryPbufferARB (HPBUFFERARB, int, int *);
#endif /* WGL_WGLEXT_PROTOTYPES */
typedef HPBUFFERARB (WINAPI * PFNWGLCREATEPBUFFERARBPROC) (HDC hDC, int iPixelFormat, int iWidth, int iHeight, const int *piAttribList);
typedef HDC (WINAPI * PFNWGLGETPBUFFERDCARBPROC) (HPBUFFERARB hPbuffer);
typedef int (WINAPI * PFNWGLRELEASEPBUFFERDCARBPROC) (HPBUFFERARB hPbuffer, HDC hDC);
typedef BOOL (WINAPI * PFNWGLDESTROYPBUFFERARBPROC) (HPBUFFERARB hPbuffer);
typedef BOOL (WINAPI * PFNWGLQUERYPBUFFERARBPROC) (HPBUFFERARB hPbuffer, int iAttribute, int *piValue);
#endif
/* END Define the pbuffer extension here as Mesa does not include the WGL extension header
	at the moment. */

#ifdef WGL_ARB_pixel_format
#if defined (WINDOW_SYSTEM_EXTENSIONS_C)
unsigned char WGLEXTENSIONFLAG(WGL_ARB_pixel_format) = WGLEXTENSION_UNSURE;
#else /* defined (WINDOW_SYSTEM_EXTENSIONS_C) */
extern unsigned char WGLEXTENSIONFLAG(WGL_ARB_pixel_format);
#endif /* defined (WINDOW_SYSTEM_EXTENSIONS_C) */
#endif /* WGL_ARB_pbuffer */

#if defined (WGL_ARB_pixel_format)
PFNWGLGETPIXELFORMATATTRIBIVARBPROC WGLHANDLE(wglGetPixelFormatAttribivARB);
#define wglGetPixelFormatAttribivARB (WGLHANDLE(wglGetPixelFormatAttribivARB))
PFNWGLGETPIXELFORMATATTRIBFVARBPROC WGLHANDLE(wglGetPixelFormatAttribfvARB);
#define wglGetPixelFormatAttribfvARB (WGLHANDLE(wglGetPixelFormatAttribfvARB))
PFNWGLCHOOSEPIXELFORMATARBPROC WGLHANDLE(wglChoosePixelFormatARB);
#define wglChoosePixelFormatARB (WGLHANDLE(wglChoosePixelFormatARB))
#endif /* defined (WGL_ARB_pixel_format) */

#ifdef WGL_ARB_pbuffer
#if defined (WINDOW_SYSTEM_EXTENSIONS_C)
unsigned char WGLEXTENSIONFLAG(WGL_ARB_pbuffer) = WGLEXTENSION_UNSURE;
#else /* defined (WINDOW_SYSTEM_EXTENSIONS_C) */
extern unsigned char WGLEXTENSIONFLAG(WGL_ARB_pbuffer);
#endif /* defined (WINDOW_SYSTEM_EXTENSIONS_C) */
#endif /* WGL_ARB_pbuffer */

#if defined (WGL_ARB_pbuffer)
PFNWGLCREATEPBUFFERARBPROC WGLHANDLE(wglCreatePbufferARB);
#define wglCreatePbufferARB (WGLHANDLE(wglCreatePbufferARB))
PFNWGLGETPBUFFERDCARBPROC WGLHANDLE(wglGetPbufferDCARB);
#define wglGetPbufferDCARB (WGLHANDLE(wglGetPbufferDCARB))
PFNWGLRELEASEPBUFFERDCARBPROC WGLHANDLE(wglReleasePbufferDCARB);
#define wglReleasePbufferDCARB (WGLHANDLE(wglReleasePbufferDCARB))
PFNWGLDESTROYPBUFFERARBPROC WGLHANDLE(wglDestroyPbufferARB);
#define wglDestroyPbufferARB (WGLHANDLE(wglDestroyPbufferARB))
PFNWGLQUERYPBUFFERARBPROC WGLHANDLE(wglQueryPbufferARB);
#define wglQueryPbufferARB (WGLHANDLE(wglQueryPbufferARB))
#endif /* defined (WGL_ARB_pbuffer) */

#endif /* defined (WIN32_USER_INTERFACE) */

#endif /* !defined (WINDOW_SYSTEM_EXTENSIONS_H) */
