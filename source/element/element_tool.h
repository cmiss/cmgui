/*******************************************************************************
FILE : element_tool.h

LAST MODIFIED : 20 March 2003

DESCRIPTION :
Interactive tool for selecting elements with mouse and other devices.
==============================================================================*/
/* OpenCMISS-Cmgui Application
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#if !defined (ELEMENT_TOOL_H)
#define ELEMENT_TOOL_H

#include "interaction/interactive_tool.h"
#include "time/time_keeper_app.hpp"

/*
Global types
------------
*/

struct Element_tool;
/*******************************************************************************
LAST MODIFIED : 5 July 2000

DESCRIPTION :
The contents of this structure are private.
==============================================================================*/

/*
Global functions
----------------
*/

struct Element_tool *CREATE(Element_tool)(
	struct MANAGER(Interactive_tool) *interactive_tool_manager,
	struct cmzn_region *region,
	struct Element_point_ranges_selection *element_point_ranges_selection,
	cmzn_material *rubber_band_material,
	struct User_interface *user_interface,
	struct Time_keeper_app *time_keeper_app);
/*******************************************************************************
LAST MODIFIED : 20 March 2003

DESCRIPTION :
Creates an Element_tool with Interactive_tool in <interactive_tool_manager>.
Selects elements in <element_selection> in response to interactive_events.
==============================================================================*/

int DESTROY(Element_tool)(struct Element_tool **element_tool_address);
/*******************************************************************************
LAST MODIFIED : 5 July 2000

DESCRIPTION :
Frees and deaccesses objects in the <element_tool> and deallocates the
structure itself.
==============================================================================*/

int Element_tool_pop_up_dialog(struct Element_tool *element_tool, struct Graphics_window *graphics_window);
/*******************************************************************************
LAST MODIFIED : 20 June 2001

DESCRIPTION :
Pops up a dialog for editing settings of the Element_tool.
==============================================================================*/

/**
 * Returns flag controlling whether top-level & 3-D elements can be selected.
 */
bool Element_tool_get_select_elements_enabled(struct Element_tool *element_tool);

/**
 * Sets flag controlling whether top-level & 3-D elements can be selected.
 */
int Element_tool_set_select_elements_enabled(struct Element_tool *element_tool,
	bool select_elements_enabled);

/**
 * Returns flag controlling whether face & 2-D top-level elements can be selected.
 */
bool Element_tool_get_select_faces_enabled(struct Element_tool *element_tool);

/**
 * Sets flag controlling whether face & 2-D top-level elements can be selected.
 */
int Element_tool_set_select_faces_enabled(struct Element_tool *element_tool,
	bool select_faces_enabled);

/**
 * Returns flag controlling whether line & 1-D top-level elements can be selected.
 */
bool Element_tool_get_select_lines_enabled(struct Element_tool *element_tool);

/**
 * Sets flag controlling whether line & 1-D top-level elements can be selected.
 */
int Element_tool_set_select_lines_enabled(struct Element_tool *element_tool,
	bool select_lines_enabled);

struct Computed_field *Element_tool_get_command_field(
	struct Element_tool *element_tool);
/*******************************************************************************
LAST MODIFIED : 30 September 2003

DESCRIPTION :
Returns the command_field to be executed when the element is clicked on in the
 <element_tool>.
==============================================================================*/

int Element_tool_set_command_field(struct Element_tool *element_tool,
	struct Computed_field *command_field);
/*******************************************************************************
LAST MODIFIED : 30 September 2003

DESCRIPTION :
Sets the command_field to be executed when the element is clicked on in the 
<element_tool>.
==============================================================================*/

struct Interactive_tool *Element_tool_get_interactive_tool(
   struct Element_tool *Element_tool);
/*******************************************************************************
LAST MODIFIED : 29 March 2007

DESCRIPTION :
Returns the generic interactive_tool the represents the <element_tool>.
==============================================================================*/

int Element_tool_set_execute_command(struct Element_tool *element_tool, 
	struct Execute_command *execute_command);
#endif /* !defined (ELEMENT_TOOL_H) */
