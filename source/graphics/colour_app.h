/* OpenCMISS-Cmgui Application
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "command/parser.h"
#include "configure/cmgui_configure.h"

#define COLOUR_PRECISION_STRING "lf"
#define COLOUR_NUM_FORMAT "%6.4" COLOUR_PRECISION_STRING

int set_Colour(struct Parse_state *state,void *colour_void,
	void *dummy_user_data);
/*******************************************************************************
LAST MODIFIED : 18 June 1996

DESCRIPTION :
A modifier function to set the colour rgb values.
==============================================================================*/
