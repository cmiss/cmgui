/* OpenCMISS-Cmgui Application
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */


#include "opencmiss/zinc/zincconfigure.h"


#if 1
#include "configure/cmgui_configure.h"
#endif /* defined (BUILD_WITH_CMAKE) */

int set_Texture_storage(struct Parse_state *state,void *enum_storage_void_ptr,
	void *dummy_user_data);
/*******************************************************************************
LAST MODIFIED : 29 June 2000

DESCRIPTION :
A modifier function to set the texture storage type.
==============================================================================*/


int Texture_get_physical_size(struct Texture *texture,ZnReal *width,
	ZnReal *height, ZnReal *depth);

int Texture_get_physical_size(struct Texture *texture,float *width,
	float *height, float *depth);

