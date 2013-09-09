/*******************************************************************************
FILE : node_tool.h

LAST MODIFIED : 15 January 2003

DESCRIPTION :
Functions for mouse controlled node selection and position and vector editing
based on input from devices.
==============================================================================*/
/* OpenCMISS-Cmgui Application
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#if !defined (NODE_TOOL_H)
#define NODE_TOOL_H

#include "command/command.h"
#include "finite_element/finite_element.h"
#include "graphics/material.h"
#include "interaction/interactive_tool.h"
#include "user_interface/user_interface.h"

/*
Global types
------------
*/

enum Node_tool_edit_mode
{
	NODE_TOOL_EDIT_AUTOMATIC,
	NODE_TOOL_EDIT_POSITION,
	NODE_TOOL_EDIT_VECTOR
};

struct Node_tool;
/*******************************************************************************
LAST MODIFIED : 11 May 2000

DESCRIPTION :
Object storing all the parameters for converting scene input messages into
changes in node position and derivatives etc.
The contents of this structure are private.
==============================================================================*/

/*
Global functions
----------------
*/

/**
 * Creates a Node_tool for editing nodes/data from the region tree.
 * Domain type may be CMZN_FIELD_DOMAIN_NODES or CMZN_FIELD_DOMAIN_DATA.
 * <element_manager> should be NULL for CMZN_FIELD_DOMAIN_DATA.
 */
struct Node_tool *CREATE(Node_tool)(
	struct MANAGER(Interactive_tool) *interactive_tool_manager,
	struct cmzn_region *root_region, cmzn_field_domain_type domain_type,
	struct Graphical_material *rubber_band_material,
	struct User_interface *user_interface,
	struct Time_keeper_app *time_keeper_app);

int DESTROY(Node_tool)(struct Node_tool **node_tool_address);
/*******************************************************************************
LAST MODIFIED : 11 May 2000

DESCRIPTION :
Frees and deaccesses objects in the <node_tool> and deallocates the
structure itself.
==============================================================================*/

int Node_tool_pop_up_dialog(struct Node_tool *node_tool, struct Graphics_window *graphics_window);
/*******************************************************************************
LAST MODIFIED : 20 June 2001

DESCRIPTION :
Pops up a dialog for editing settings of the Node_tool.
==============================================================================*/

struct Computed_field *Node_tool_get_coordinate_field(
	struct Node_tool *node_tool);
/*******************************************************************************
LAST MODIFIED : 12 September 2000

DESCRIPTION :
Returns the coordinate field used by the <node_tool> when create/define are on.
==============================================================================*/

int Node_tool_set_coordinate_field(struct Node_tool *node_tool,
	struct Computed_field *coordinate_field);
/*******************************************************************************
LAST MODIFIED : 12 September 2000

DESCRIPTION :
Sets the coordinate field to be defined by the <node_tool> when create/define
are on.
==============================================================================*/

int Node_tool_get_create_enabled(struct Node_tool *node_tool);
/*******************************************************************************
LAST MODIFIED : 18 July 2000

DESCRIPTION :
Returns flag controlling whether nodes can be created when none are selected
on a mouse button press.
==============================================================================*/

int Node_tool_set_create_enabled(struct Node_tool *node_tool,
	int create_enabled);
/*******************************************************************************
LAST MODIFIED : 11 September 2000

DESCRIPTION :
Sets flag controlling whether nodes can be created when none are selected
on a mouse button press. Also ensures define is enabled if create is.
==============================================================================*/

int Node_tool_get_define_enabled(struct Node_tool *node_tool);
/*******************************************************************************
LAST MODIFIED : 18 July 2000

DESCRIPTION :
Returns flag controlling whether nodes can be defined when none are selected
on a mouse button press.
==============================================================================*/

int Node_tool_set_define_enabled(struct Node_tool *node_tool,
	int define_enabled);
/*******************************************************************************
LAST MODIFIED : 12 September 2000

DESCRIPTION :
Sets flag controlling whether the coordinate field can be defined on any new
or individually selected existing nodes.
==============================================================================*/

int Node_tool_get_edit_enabled(struct Node_tool *node_tool);
/*******************************************************************************
LAST MODIFIED : 11 May 2000

DESCRIPTION :
Returns flag controlling whether node edits are updated during motion_notify
events, not just at the end of a mouse gesture.
==============================================================================*/

int Node_tool_set_edit_enabled(struct Node_tool *node_tool,int edit_enabled);
/*******************************************************************************
LAST MODIFIED : 18 July 2000

DESCRIPTION :
Sets flag controlling whether node edits are updated during motion_notify
events, not just at the end of a mouse gesture.
==============================================================================*/

enum Node_tool_edit_mode Node_tool_get_edit_mode(struct Node_tool *node_tool);
/*******************************************************************************
LAST MODIFIED : 11 May 2000

DESCRIPTION :
Returns the current edit mode of <node_tool>.
==============================================================================*/

int Node_tool_set_edit_mode(struct Node_tool *node_tool,
	enum Node_tool_edit_mode edit_mode);
/*******************************************************************************
LAST MODIFIED : 11 May 2000

DESCRIPTION :
Sets the <edit_mode> of <node_tool> - controls whether the editor
can select or edit nodes, and whether the editing is restricted to position or
vector only.
==============================================================================*/

int Node_tool_get_motion_update_enabled(struct Node_tool *node_tool);
/*******************************************************************************
LAST MODIFIED : 11 May 2000

DESCRIPTION :
Returns flag controlling whether node edits are updated during motion_notify
events, not just at the end of a mouse gesture.
==============================================================================*/

int Node_tool_set_motion_update_enabled(struct Node_tool *node_tool,
	int motion_update_enabled);
/*******************************************************************************
LAST MODIFIED : 11 May 2000

DESCRIPTION :
Sets flag controlling whether node edits are updated during motion_notify
events, not just at the end of a mouse gesture.
==============================================================================*/

int Node_tool_get_select_enabled(struct Node_tool *node_tool);
/*******************************************************************************
LAST MODIFIED : 11 May 2000

DESCRIPTION :
Returns flag controlling whether existing nodes can be selected.
==============================================================================*/

int Node_tool_set_select_enabled(struct Node_tool *node_tool,
	int select_enabled);
/*******************************************************************************
LAST MODIFIED : 18 July 2000

DESCRIPTION :
Sets flag controlling whether existing nodes can be selected.
==============================================================================*/

int Node_tool_get_streaming_create_enabled(struct Node_tool *node_tool);
/*******************************************************************************
LAST MODIFIED : 14 May 2001

DESCRIPTION :
Returns flag controlling, if create_enabled, whether a stream of nodes is
created as the user drags the mouse around.
==============================================================================*/

int Node_tool_set_streaming_create_enabled(struct Node_tool *node_tool,
	int streaming_create_enabled);
/*******************************************************************************
LAST MODIFIED : 14 May 2001

DESCRIPTION :
Sets flag controlling, if create_enabled, whether a stream of nodes is
created as the user drags the mouse around.
==============================================================================*/

int Node_tool_get_constrain_to_surface(struct Node_tool *node_tool);
/*******************************************************************************
LAST MODIFIED : 26 May 2005

DESCRIPTION :
Returns flag controlling, if create_enabled, whether new nodes will be created
on the closest surface element or just halfway between near and far.
==============================================================================*/

int Node_tool_set_constrain_to_surface(struct Node_tool *node_tool,
	int constrain_to_surface);
/*******************************************************************************
LAST MODIFIED : 26 May 2005

DESCRIPTION :
Sets flag controlling, if create_enabled, whether new nodes will be created
on the closest surface element or just halfway between near and far.
==============================================================================*/

struct Computed_field *Node_tool_get_element_xi_field(
	struct Node_tool *node_tool);
/*******************************************************************************
LAST MODIFIED : 18 February 2008

DESCRIPTION :
Returns the elementxi_field to define when the node is created in the <node_tool>
and node is constrained to surface.
==============================================================================*/

int Node_tool_set_element_xi_field(struct Node_tool *node_tool,
	struct Computed_field *element_xi_field);
/*******************************************************************************
LAST MODIFIED : 18 February 2008

DESCRIPTION :
Sets the elementxi_field to define when the node is created in the <node_tool>
and node is constrained to surface.
==============================================================================*/

struct Computed_field *Node_tool_get_command_field(
	struct Node_tool *node_tool);
/*******************************************************************************
LAST MODIFIED : 30 September 2003

DESCRIPTION :
Returns the command_field to be executed when the node is clicked on in the <node_tool>.
==============================================================================*/

int Node_tool_set_command_field(struct Node_tool *node_tool,
	struct Computed_field *command_field);
/*******************************************************************************
LAST MODIFIED : 12 April 2007

DESCRIPTION :
Sets the command_field to be executed when the node is clicked on in the <node_tool>.
==============================================================================*/

int Node_tool_get_element_create_enabled(struct Node_tool *node_tool);
/*******************************************************************************
LAST MODIFIED : 12 April 2007

DESCRIPTION :
Returns flag controlling whether node edits are updated during motion_notify
events, not just at the end of a mouse gesture.
==============================================================================*/

int Node_tool_get_element_dimension(
	 struct Node_tool *node_tool);
/*******************************************************************************
LAST MODIFIED : 12 April 2007

DESCRIPTION :
Returns the dimension of elements to be created by the <node_tool>.
==============================================================================*/

int Node_tool_set_element_dimension(
	 struct Node_tool *node_tool,int element_dimension);
/*******************************************************************************
LAST MODIFIED : 11 April 2007

DESCRIPTION :
Sets the <element_dimension> of elements to be created by <node_tool>.
==============================================================================*/

int Node_tool_set_element_create_enabled(struct Node_tool  *node_tool ,
	 int element_create_enabled);
/*******************************************************************************
LAST MODIFIED : 11 April 2007

DESCRIPTION :
Sets flag controlling whether elements are created in response to
node selection.
==============================================================================*/

struct Interactive_tool *Node_tool_get_interactive_tool(
	struct Node_tool *node_tool);
/*******************************************************************************
LAST MODIFIED : 7 April 2005

DESCRIPTION :
Returns the generic interactive_tool the represents the <node_tool>.
==============================================================================*/

void Node_tool_set_wx_interface(void *node_tool_void);
/*******************************************************************************
LAST MODIFIED : 13 April 2007

DESCRIPTION :
Set the wx_interface for new settings.
==============================================================================*/	 

int Node_tool_set_execute_command(struct Node_tool *node_tool, 
	struct Execute_command *execute_command);

int Node_tool_execute_command(struct Node_tool *node_tool, const char *command_string);

int Node_tool_execute_command_with_parse_state(struct Node_tool *node_tool, struct Parse_state *state);
#endif /* !defined (NODE_TOOL_H) */
