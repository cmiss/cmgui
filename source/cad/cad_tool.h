/**
FILE : cad_tool.h

CREATED : 20 June 2010

DESCRIPTION :
Interactive tool for selecting cad primitives with a mouse and other devices.
*/
/* OpenCMISS-Cmgui Application
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#if !defined (CAD_TOOL_H)
#define CAD_TOOL_H

#include "interaction/interactive_tool.h"
#include "time/time_keeper.h"

/*
Global types
------------
*/

struct Cad_tool;

/*
Global functions
----------------
*/

struct Cad_tool *CREATE(Cad_tool)(
	struct MANAGER(Interactive_tool) *interactive_tool_manager,
	struct cmzn_region *region,
	struct Element_point_ranges_selection *element_point_ranges_selection,
	struct Graphical_material *rubber_band_material,
	struct User_interface *user_interface,
	struct Time_keeper *time_keeper);

int DESTROY(Cad_tool)(struct Cad_tool **cad_tool_address);

int Cad_tool_pop_up_dialog(struct Cad_tool *cad_tool, struct Graphics_window *graphics_window);

int Cad_tool_pop_down_dialog(struct Cad_tool *cad_tool);

int Cad_tool_get_select_surfaces_enabled(struct Cad_tool *cad_tool);

int Cad_tool_set_select_surfaces_enabled(struct Cad_tool *cad_tool,
	int select_surfaces_enabled);

int Cad_tool_get_select_lines_enabled(struct Cad_tool *cad_tool);

int Cad_tool_set_select_lines_enabled(struct Cad_tool *cad_tool,
	int select_lines_enabled);

struct Computed_field *Cad_tool_get_command_field(
	struct Cad_tool *cad_tool);

int Cad_tool_set_command_field(struct Cad_tool *cad_tool,
	struct Computed_field *command_field);

struct Interactive_tool *Cad_tool_get_interactive_tool(
	struct Cad_tool *cad_tool);

int Cad_tool_set_execute_command(struct Cad_tool *cad_tool, 
	struct Execute_command *execute_command);

#endif /* !defined (CAD_TOOL_H) */
