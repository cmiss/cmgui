/**
 * FILE : auxiliary_graphics_types_app.h
 */
/* OpenCMISS-Cmgui Application
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef AUXILIARY_GRAPHICS_TYPES_APP_H
#define AUXILIARY_GRAPHICS_TYPES_APP_H

#include "graphics/auxiliary_graphics_types.h"

struct User_interface;
struct Parse_state;

/**
 * A modifier function for setting graphics face type XI1_0, XI1_1 etc.
 */
int set_graphics_face_type(struct Parse_state *state, void *face_type_address_void,
	void *dummy_user_data);

/** A modifier function for setting number of segments used to draw circles */
int set_circle_divisions(struct Parse_state *state,
	void *circle_divisions_void, void *dummy_void);

int set_Element_discretization(struct Parse_state *state,
	void *element_discretization_void,void *user_interface_void);
/*******************************************************************************
LAST MODIFIED : 30 October 1996

DESCRIPTION :
A modifier function for setting discretization in each element direction.
==============================================================================*/

#endif // AUXILIARY_GRAPHICS_TYPES_APP_H
