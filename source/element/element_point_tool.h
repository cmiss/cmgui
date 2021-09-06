/*******************************************************************************
FILE : element_point_tool.h

LAST MODIFIED : 5 July 2002

DESCRIPTION :
Interactive tool for selecting element/grid points with mouse and other devices.
==============================================================================*/
/* OpenCMISS-Cmgui Application
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#if !defined (ELEMENT_POINT_TOOL_H)
#define ELEMENT_POINT_TOOL_H

#include "opencmiss/zinc/types/materialid.h"
#include "interaction/interactive_tool.h"
#include "selection/element_point_ranges_selection.h"
#include "time/time_keeper_app.hpp"
/*
Global types
------------
*/

struct Element_point_tool;
/*******************************************************************************
LAST MODIFIED : 16 May 2000

DESCRIPTION :
The contents of this structure are private.
==============================================================================*/

/*
Global functions
----------------
*/

struct Element_point_tool *CREATE(Element_point_tool)(
	struct MANAGER(Interactive_tool) *interactive_tool_manager,
	struct cmzn_region *region,
	struct Element_point_ranges_selection *element_point_ranges_selection,
	cmzn_material *rubber_band_material,
	struct User_interface *user_interface,
	struct Time_keeper_app *time_keeper_app);
/*******************************************************************************
LAST MODIFIED : 5 July 2002

DESCRIPTION :
Creates an Element_point_tool with Interactive_tool in
<interactive_tool_manager>. Selects element/grid points in
<element_point_ranges_selection> in response to interactive_events.
==============================================================================*/

int DESTROY(Element_point_tool)(
	struct Element_point_tool **element_point_tool_address);
/*******************************************************************************
LAST MODIFIED : 16 May 2000

DESCRIPTION :
Frees and deaccesses objects in the <element_point_tool> and deallocates the
structure itself.
==============================================================================*/

int Element_point_tool_pop_up_dialog(
																		 struct Element_point_tool *element_point_tool,struct Graphics_window *graphics_window);
/*******************************************************************************
LAST MODIFIED : 5 July 2002

DESCRIPTION :
Pops up a dialog for editing settings of the Element_point_tool.
==============================================================================*/

int Element_point_tool_pop_down_dialog(
	struct Element_point_tool *element_point_tool);
/*******************************************************************************
LAST MODIFIED : 5 July 2002

DESCRIPTION :
Hides the dialog for editing settings of the Element_point_tool.
==============================================================================*/

struct Computed_field *Element_point_tool_get_command_field(
	struct Element_point_tool *element_point_tool);
/*******************************************************************************
LAST MODIFIED : 30 September 2003

DESCRIPTION :
Returns the command_field to be executed when the element is clicked on in the 
<element_point_tool>.
==============================================================================*/

int Element_point_tool_set_command_field(
	struct Element_point_tool *element_point_tool,
	struct Computed_field *command_field);
/*******************************************************************************
LAST MODIFIED : 30 September 2003

DESCRIPTION :
Sets the command_field to be executed when the element is clicked on in the 
<element_point_tool>.
==============================================================================*/

struct Interactive_tool *Element_point_tool_get_interactive_tool(
  struct Element_point_tool *element_point_tool);
/*******************************************************************************
LAST MODIFIED : 29 March 2007

DESCRIPTION :
Returns the generic interactive_tool the represents the <element_point_tool>.
==============================================================================*/

int Element_point_tool_set_execute_command(struct Element_point_tool *element_point_tool, 
	struct Execute_command *execute_command);
#endif /* !defined (ELEMENT_POINT_TOOL_H) */
