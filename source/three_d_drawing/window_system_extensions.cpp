/*******************************************************************************
FILE : graphics_buffer.cpp

LAST MODIFIED : 10 March 2008

DESCRIPTION :
This provides a Cmgui interface to the window system specific OpenGL binding
(i.e. glx or wgl) extensions.
******************************************************************************/
/* OpenCMISS-Cmgui Application
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#if defined (WIN32_USER_INTERFACE)
//#define WINDOWS_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <GL/gl.h>
#endif /* defined (WIN32_USER_INTERFACE) */
#include "general/debug.h"

#define WINDOW_SYSTEM_EXTENSIONS_C
#include "three_d_drawing/window_system_extensions.h"

#if defined (WIN32_USER_INTERFACE)
int Window_system_extensions_load_wgl_extension(const char *extension_name)
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
{
	int return_code = 0;
	USE_PARAMETER(extension_name);

	/* Could also check wglGetExtensionStringARB but if the functions are available
		I'm going to try and use them */
	if (0)
	{
	}
#if defined WGL_ARB_pixel_format
	else if (!strcmp(extension_name, "WGL_ARB_pixel_format"))
	{
		if (WGLEXTENSION_UNSURE != WGLEXTENSIONFLAG(WGL_ARB_pixel_format))
		{
			return_code = WGLEXTENSIONFLAG(WGL_ARB_pixel_format);
		}
		else
		{
			if ((WINDOW_SYSTEM_EXTENSIONS_ASSIGN_HANDLE(wglGetPixelFormatAttribivARB, PFNWGLGETPIXELFORMATATTRIBIVARBPROC)
					wglGetProcAddress("wglGetPixelFormatAttribivARB")) &&
				(WINDOW_SYSTEM_EXTENSIONS_ASSIGN_HANDLE(wglGetPixelFormatAttribfvARB, PFNWGLGETPIXELFORMATATTRIBFVARBPROC)
					wglGetProcAddress("wglGetPixelFormatAttribfvARB")) &&
				(WINDOW_SYSTEM_EXTENSIONS_ASSIGN_HANDLE(wglChoosePixelFormatARB, PFNWGLCHOOSEPIXELFORMATARBPROC)
					wglGetProcAddress("wglChoosePixelFormatARB")))
			{
				return_code = WGLEXTENSION_AVAILABLE;
			}
			else
			{
				return_code = WGLEXTENSION_UNAVAILABLE;
			}
#if defined (DEBUG_CODE)
			printf("Handles for WGL_ARB_pixel_format %p %p %p\n",
				WGLHANDLE(wglGetPixelFormatAttribivARB),
				WGLHANDLE(wglGetPixelFormatAttribfvARB),
				WGLHANDLE(wglChoosePixelFormatARB));
#endif /* defined (DEBUG_CODE) */
			WGLEXTENSIONFLAG(WGL_ARB_pixel_format) = return_code;
		}
	}
#endif /* WGL_ARB_pixel_format */
#if defined WGL_ARB_pbuffer
	else if (!strcmp(extension_name, "WGL_ARB_pbuffer"))
	{
		if (WGLEXTENSION_UNSURE != WGLEXTENSIONFLAG(WGL_ARB_pbuffer))
		{
			return_code = WGLEXTENSIONFLAG(WGL_ARB_pbuffer);
		}
		else
		{
			if ((WINDOW_SYSTEM_EXTENSIONS_ASSIGN_HANDLE(wglCreatePbufferARB, PFNWGLCREATEPBUFFERARBPROC)
					wglGetProcAddress("wglCreatePbufferARB")) &&
				(WINDOW_SYSTEM_EXTENSIONS_ASSIGN_HANDLE(wglGetPbufferDCARB, PFNWGLGETPBUFFERDCARBPROC)
					wglGetProcAddress("wglGetPbufferDCARB")) &&
				(WINDOW_SYSTEM_EXTENSIONS_ASSIGN_HANDLE(wglReleasePbufferDCARB, PFNWGLRELEASEPBUFFERDCARBPROC)
					wglGetProcAddress("wglReleasePbufferDCARB")) &&
				(WINDOW_SYSTEM_EXTENSIONS_ASSIGN_HANDLE(wglDestroyPbufferARB, PFNWGLDESTROYPBUFFERARBPROC)
					wglGetProcAddress("wglDestroyPbufferARB")) &&
				(WINDOW_SYSTEM_EXTENSIONS_ASSIGN_HANDLE(wglQueryPbufferARB, PFNWGLQUERYPBUFFERARBPROC)
					wglGetProcAddress("wglQueryPbufferARB")))
			{
				return_code = WGLEXTENSION_AVAILABLE;
			}
			else
			{
				return_code = WGLEXTENSION_UNAVAILABLE;
			}
#if defined (DEBUG_CODE)
			printf("Handles for WGL_ARB_pbuffer %p %p %p %p %p\n",
				WGLHANDLE(wglCreatePbufferARB),
				WGLHANDLE(wglGetPbufferDCARB),
				WGLHANDLE(wglReleasePbufferDCARB),
				WGLHANDLE(wglDestroyPbufferARB),
				WGLHANDLE(wglQueryPbufferARB));
#endif /* defined (DEBUG_CODE) */
			WGLEXTENSIONFLAG(WGL_ARB_pbuffer) = return_code;
		}
	}
#endif /* WGL_ARB_pbuffer */
	else
	{
		return_code = 0;
	}

	return (return_code);
}
#endif /* defined (WIN32_USER_INTERFACE) */
