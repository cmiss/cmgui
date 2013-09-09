/*******************************************************************************
FILE : texturemap.h

LAST MODIFIED : 15 September 1997

DESCRIPTION :
==============================================================================*/
/* OpenCMISS-Cmgui Application
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#if !defined (TEXTUREMAP_H)
#define TEXTUREMAP_H

#include "finite_element/finite_element.h"
#include "graphics/graphics_window.h"

/*
Global types
------------
*/
struct Image_buffer
/*******************************************************************************
LAST MODIFIED : 7 April 1997

DESCRIPTION :
==============================================================================*/
{
	short xsize,ysize,zsize;
	short **rbuf,**gbuf,**bbuf,**abuf;	
}; /* struct Image_buffer */

/*
Global functions
----------------
*/

#endif /* !defined (TEXTUREMAP_H) */
