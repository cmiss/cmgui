/*******************************************************************************
FILE : emoter_dialog.h

LAST MODIFIED : 4 May 2004

DESCRIPTION :
This module creates a emoter_slider input device.  A node group slider is
used to scale the distance between a fixed node and a group of nodes.
???DB.  Extend to other options
==============================================================================*/
/* OpenCMISS-Cmgui Application
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#if !defined (EMOTER_SLIDER_DIALOG_H)
#define EMOTER_SLIDER_DIALOG_H

#include "general/io_stream.h"
#include "graphics/graphics_window.h"
#include "region/cmiss_region.h"
#include "user_interface/user_interface.h"

/*
Global types
------------
*/
struct Emoter_dialog;
/*******************************************************************************
LAST MODIFIED : 9 December 2003

DESCRIPTION :
==============================================================================*/

struct Create_emoter_slider_data
/*******************************************************************************
LAST MODIFIED : 22 January 2003

DESCRIPTION :
==============================================================================*/
{
	struct Execute_command *execute_command;
	struct cmzn_region *root_region;
	struct MANAGER(FE_basis) *basis_manager;
#if defined (USE_CMGUI_GRAPHICS_WINDOW)
	struct MANAGER(Graphics_window) *graphics_window_manager;
#endif /* defined (USE_CMGUI_GRAPHICS_WINDOW) */
	struct MANAGER(Curve) *curve_manager;
	struct MANAGER(Scene) *scene_manager;
	struct IO_stream_package *io_stream_package;
	struct Scene *viewer_scene;
	struct Colour viewer_background_colour;
	struct Graphics_buffer_app_package *graphics_buffer_package;
	struct Light *viewer_light;
	struct Light_model *viewer_light_model;
	struct Emoter_dialog **emoter_dialog_address;
	struct User_interface *user_interface;
}; /* struct Create_emoter_slider_data */

/*
Global functions
----------------
*/
int gfx_create_emoter(struct Parse_state *state,void *dummy_to_be_modified,
	void *create_emoter_slider_data_void);
/*******************************************************************************
LAST MODIFIED : 9 December 2003

DESCRIPTION :
Executes a GFX CREATE EMOTER command.  If there is a emoter dialog
in existence, then bring it to the front, otherwise create new one.
==============================================================================*/

int set_emoter_slider_value(struct Parse_state *state,
	void *dummy_to_be_modified,void *emoter_slider_dialog_widget_void);
/*******************************************************************************
LAST MODIFIED : 1 June 1997

DESCRIPTION :
==============================================================================*/

int gfx_modify_emoter(struct Parse_state *state,
	void *dummy_to_be_modified, void *emoter_dialog_widget_void);
/*******************************************************************************
LAST MODIFIED : 7 September 1999

DESCRIPTION :
Executes a GFX MODIFY EMOTER command.
==============================================================================*/

int DESTROY(Emoter_dialog)(struct Emoter_dialog **emoter_dialog_address);
/*******************************************************************************
LAST MODIFIED : 9 December 2003

DESCRIPTION :
Callback for the emoter dialog - tidies up all details - mem etc
==============================================================================*/
#endif /* !defined (EMOTER_SLIDER_DIALOG_H) */
