/*******************************************************************************
FILE : User_interface_module.h

LAST MODIFIED : 7 January 2003

DESCRIPTION :
==============================================================================*/
/* OpenCMISS-Cmgui Application
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#if !defined (USER_INTERFACE_MODULE_H)
#define USER_INTERFACE_MODULE_H

#include "comfile/comfile.h"
#include "general/debug.h"
#include "graphics/colour.h"
#include "graphics/graphics_window.h"
#include "user_interface/event_dispatcher.h"
#include "general/message.h"

struct User_interface_module
{
	int access_count;
	int argc, cleanup_argc;
	char **argv, **cleanup_argv, **unmodified_argv;
	struct Colour background_colour,foreground_colour;
	struct Element_point_tool *element_point_tool;
	struct Element_tool *element_tool;
#if defined (USE_OPENCASCADE)
	struct Cad_tool *cad_tool;
#endif /* defined (USE_OPENCASCADE) */
	struct Interactive_tool *transform_tool;
	struct Node_tool *data_tool,*node_tool;
	struct Event_dispatcher *event_dispatcher;
	struct cmzn_sceneviewermodule_app *sceneviewermodule;
#if defined (USE_CMGUI_GRAPHICS_WINDOW)
	struct MANAGER(Graphics_window) *graphics_window_manager;
#endif /* defined (USE_CMGUI_GRAPHICS_WINDOW) */
	struct Time_keeper_app *default_time_keeper_app;
	struct User_interface *user_interface;
#if defined (WX_USER_INTERFACE)
	struct MANAGER(Comfile_window) *comfile_window_manager;
	struct Node_viewer *data_viewer,*node_viewer;
	struct Element_point_viewer *element_point_viewer;
	struct Material_editor_dialog *material_editor_dialog;
	struct Region_tree_viewer *region_tree_viewer;
	struct Spectrum_editor_dialog *spectrum_editor_dialog;
#endif /* defined (WX_USER_INTERFACE) */
	struct Graphics_buffer_app_package *graphics_buffer_package;
	struct MANAGER(Interactive_tool) *interactive_tool_manager;
	int external;
};

/***************************************************************************//**
 * Create a new user_interaface_module, access count will be set to 1.
 *
 * @param context  pointer to a context object which this user interface module
 *   will be built on.
 * @param in_argc  number of arguments
 * @param in_argv  array to the value of each argument
 * @return  handle to an user_interface_module.
 */
#if defined (WX_USER_INTERFACE) || (!defined (WIN32_USER_INTERFACE) && !defined (_MSC_VER))
struct User_interface_module *User_interface_module_create(
	struct cmzn_context_app *context, int in_argc, char *in_argv[]);
#else
struct User_interface_module *User_interface_module_create(
	struct cmzn_context_app *context, int in_argc, char *in_argv[],
	HINSTANCE current_instance, HINSTANCE previous_instance,
	LPSTR command_line,int initial_main_window_state);
#endif

/***************************************************************************//**
 * Access the user_interaface_module, increase the access count of the module
 * by one.
 *
 * @param UI_module  pointer to the "to be accessed" user interface module.
 * @return  handle to an user_interface_module if successfully called,
 *   otherwise NULL.
 */
struct User_interface_module *User_interface_module_access(
	struct User_interface_module *UI_module);

/***************************************************************************//**
 * Dereference a user_interaface_module, decrease the access count of the module
 * by one.
 *
 * @param UI_module  address to a pointer to the "to be accessed"
 *   User_interface_module.
 * @return  address of the handle of an User_interface_module.
 */
int User_interface_module_destroy(
	struct User_interface_module **UI_module_address);

#endif
