/**
 * FILE : user_interface_module.c
 *
 */
/* OpenCMISS-Cmgui Application
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "opencmiss/zinc/graphics.h"
#include "opencmiss/zinc/material.h"
#include "opencmiss/zinc/timekeeper.h"
#include "time/time_keeper_app.hpp"
#include "comfile/comfile.h"
#include "command/command_window.h"
#include "command/cmiss.h"
#include "element/element_point_tool.h"
#include "element/element_tool.h"
#include "general/debug.h"
#include "general/mystring.h"
#include "graphics/colour.h"
#include "graphics/graphics_module.h"
#include "graphics/transform_tool.h"
#include "graphics/scene.h"
#include "interaction/interactive_tool.h"
#include "node/node_tool.h"
#include "three_d_drawing/graphics_buffer.h"
#include "three_d_drawing/graphics_buffer_app.h"
#include "user_interface/event_dispatcher.h"
#include "general/message.h"
#include "context/user_interface_module.h"
#if defined (USE_OPENCASCADE)
#include "cad/cad_tool.h"
#endif /* defined (USE_OPENCASCADE) */
#include "context/context_app.h"
#include "graphics/scene_viewer_app.h"

#if defined (WX_USER_INTERFACE) || (!defined (WIN32_USER_INTERFACE) && !defined (_MSC_VER))
struct User_interface_module *User_interface_module_create(
	struct cmzn_context_app *context, int in_argc, char *in_argv[])
#else
struct User_interface_module *User_interface_module_create(
	struct cmzn_context_app *context, int in_argc, char *in_argv[],
	HINSTANCE current_instance, HINSTANCE previous_instance,
	LPSTR command_line,int initial_main_window_state)
#endif
{
	struct User_interface_module *UI_module = NULL;
	cmzn_region *root_region = NULL;;
	struct cmzn_graphics_module *graphics_module = NULL;
	int visual_id = 0;

	if (context && ALLOCATE(UI_module, struct User_interface_module, 1))
	{
		root_region = cmzn_context_get_default_region(cmzn_context_app_get_core_context(context));
		graphics_module = cmzn_context_get_graphics_module(cmzn_context_app_get_core_context(context));
		UI_module->event_dispatcher = NULL;
#if defined (USE_CMGUI_GRAPHICS_WINDOW)
		UI_module->graphics_window_manager = NULL;
#endif /* defined (USE_CMGUI_GRAPHICS_WINDOW) */
		UI_module->default_time_keeper_app = NULL;
		UI_module->user_interface = NULL;
#if defined (WX_USER_INTERFACE)
		UI_module->comfile_window_manager = NULL;
		UI_module->data_viewer = NULL;
		UI_module->node_viewer = NULL;
		UI_module->element_point_viewer = NULL;
		UI_module->material_editor_dialog = NULL;
		UI_module->region_tree_viewer = NULL;
		UI_module->spectrum_editor_dialog = NULL;
#endif /* defined (WX_USER_INTERFACE) */
		UI_module->sceneviewermodule = NULL;
		UI_module->graphics_buffer_package = NULL;
		UI_module->interactive_tool_manager = NULL;
		UI_module->background_colour.red=(float)0;
		UI_module->background_colour.green=(float)0;
		UI_module->background_colour.blue=(float)0;
		UI_module->foreground_colour.red=(float)1;
		UI_module->foreground_colour.green=(float)1;
		UI_module->foreground_colour.blue=(float)1;
		UI_module->access_count = 1;
		UI_module->argc = in_argc;
		UI_module->argv = NULL;
		UI_module->external = 0;
		UI_module->unmodified_argv = NULL;
		UI_module->cleanup_argc = in_argc;
		UI_module->cleanup_argv = NULL;
		struct Cmgui_command_line_options command_line_options;
		cmzn_command_data_process_command_line(in_argc, in_argv,
			&command_line_options);
		visual_id = command_line_options.visual_id_number;
		if (0 < in_argc)
		{
			ALLOCATE(UI_module->argv, char *, in_argc);
			ALLOCATE(UI_module->unmodified_argv, char *, in_argc);
			ALLOCATE(UI_module->cleanup_argv, char *, in_argc);
			for (int ai = 0; ai < in_argc; ai++)
			{
				UI_module->cleanup_argv[ai] = UI_module->argv[ai] = UI_module->unmodified_argv[ai] =
					duplicate_string(in_argv[ai]);
			}
		}

		if ((!command_line_options.command_list_flag) && (!command_line_options.write_help_flag))
		{
			if (NULL != (UI_module->event_dispatcher = cmzn_context_app_get_default_event_dispatcher(
				context)))
			{
				if (!command_line_options.no_display_flag)
				{
#if defined (WX_USER_INTERFACE) || (!defined (WIN32_USER_INTERFACE) && !defined (_MSC_VER))
					if (NULL == (UI_module->user_interface = CREATE(User_interface)
							(&(UI_module->argc), UI_module->argv, UI_module->event_dispatcher, "Cmgui",
								"cmgui")))
					{
						display_message(ERROR_MESSAGE,"Could not create User interface");
					}
#else /* !defined (WIN32_USER_INTERFACE) && !defined (_MSC_VER) */
					if (NULL == (UI_module->user_interface = CREATE(User_interface)
							(current_instance, previous_instance, command_line,
								initial_main_window_state, &(UI_module->argc), UI_module->argv, UI_module->event_dispatcher)))
					{
						display_message(ERROR_MESSAGE,"Could not create User interface");
					}
#endif /* !defined (WIN32_USER_INTERFACE) && !defined (_MSC_VER) */
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Could not create Event dispatcher.");
		}
		if (command_line_options.example_file_name)
		{
			DEALLOCATE(command_line_options.example_file_name);
		}
		if (command_line_options.id_name)
		{
			DEALLOCATE(command_line_options.id_name);
		}
		if (command_line_options.execute_string)
		{
			DEALLOCATE(command_line_options.execute_string);
		}
		if (command_line_options.command_file_name)
		{
			DEALLOCATE(command_line_options.command_file_name);
		}
		if (command_line_options.epath_directory_name)
		{
			DEALLOCATE(command_line_options.epath_directory_name);
		}
#if defined (WX_USER_INTERFACE)
		/* comfile window manager */
		UI_module->comfile_window_manager = CREATE(MANAGER(Comfile_window))();
#endif /* defined (WX_USER_INTERFACE) */

#if defined (USE_CMGUI_GRAPHICS_WINDOW)
		if (UI_module->user_interface)
		{
			UI_module->graphics_buffer_package = CREATE(Graphics_buffer_app_package)(UI_module->user_interface);
			Graphics_buffer_package_set_override_visual_id(Graphics_buffer_package_get_core_package(UI_module->graphics_buffer_package),
				visual_id);
		}
		/* graphics window manager.  Note there is no default window. */
		UI_module->graphics_window_manager=CREATE(MANAGER(Graphics_window))();
#endif /* defined (USE_CMGUI_GRAPHICS_WINDOW) */
		cmzn_timekeepermodule_id timekeepermodule = cmzn_context_get_timekeepermodule(cmzn_context_app_get_core_context(context));
		cmzn_timekeeper *timekeeper = cmzn_timekeepermodule_get_default_timekeeper(timekeepermodule);
		cmzn_timekeepermodule_destroy(&timekeepermodule);
		UI_module->default_time_keeper_app = new Time_keeper_app(timekeeper, UI_module->event_dispatcher);
		cmzn_timekeeper_destroy(&timekeeper);
		UI_module->interactive_tool_manager=CREATE(MANAGER(Interactive_tool))();
		if (UI_module->user_interface)
		{
			struct cmzn_materialmodule *materialmodule =
				cmzn_graphics_module_get_materialmodule(graphics_module);
			cmzn_material_id defaultMaterial =
				cmzn_materialmodule_get_default_material(materialmodule);
			UI_module->transform_tool=create_Interactive_tool_transform(
				UI_module->user_interface);
			ADD_OBJECT_TO_MANAGER(Interactive_tool)(UI_module->transform_tool,
				UI_module->interactive_tool_manager);
			UI_module->node_tool=CREATE(Node_tool)(
				UI_module->interactive_tool_manager,
				root_region, CMZN_FIELD_DOMAIN_TYPE_NODES,
				defaultMaterial,
				UI_module->user_interface,
				UI_module->default_time_keeper_app);
			UI_module->data_tool=CREATE(Node_tool)(
				UI_module->interactive_tool_manager,
				root_region, CMZN_FIELD_DOMAIN_TYPE_DATAPOINTS,
				defaultMaterial,
				UI_module->user_interface,
				UI_module->default_time_keeper_app);
			UI_module->element_tool=CREATE(Element_tool)(
				UI_module->interactive_tool_manager,
				root_region,
				cmzn_context_get_element_point_ranges_selection(cmzn_context_app_get_core_context(context)),
				defaultMaterial,
				UI_module->user_interface,
				UI_module->default_time_keeper_app);
#if defined (USE_OPENCASCADE)
			UI_module->cad_tool=CREATE(Cad_tool)(
				UI_module->interactive_tool_manager,
				root_region,
				cmzn_context_get_element_point_ranges_selection(context),
				defaultMaterial,
				UI_module->user_interface,
				UI_module->default_time_keeper_app);
#endif /* defined (USE_OPENCASCADE) */
			UI_module->element_point_tool=CREATE(Element_point_tool)(
				UI_module->interactive_tool_manager,
				root_region,
				cmzn_context_get_element_point_ranges_selection(cmzn_context_app_get_core_context(context)),
				defaultMaterial,
				UI_module->user_interface,
				UI_module->default_time_keeper_app);
			cmzn_material_destroy(&defaultMaterial);
			cmzn_materialmodule_destroy(&materialmodule);
		}
		if (UI_module->user_interface)
		{
			/* set up image library */
#if defined (UNIX) /* switch (Operating_System) */
			if (UI_module->argv != 0)
			{
				Open_image_environment(*(UI_module->argv));
			}
			else
			{
				Open_image_environment("cmgui");
			}
#elif defined (WIN32_USER_INTERFACE) /* switch (Operating_System) */
			/* SAB Passing a string to this function so that it
				starts up, should get the correct thing from
				the windows system */
			Open_image_environment("cmgui");
#endif /* switch (Operating_System) */
		}
#if defined (USE_CMGUI_GRAPHICS_WINDOW)
		if (UI_module->user_interface)
		{
			if (graphics_module)
			{
				cmzn_scene_id default_scene =	cmzn_region_get_scene(root_region);
				UI_module->sceneviewermodule = CREATE(cmzn_sceneviewermodule_app)(
					UI_module->graphics_buffer_package, default_scene,
					UI_module->user_interface);
				cmzn_scene_destroy(&default_scene);
			}
		}
#endif /* defined (USE_CMGUI_GRAPHICS_WINDOW) */
		cmzn_region_destroy(&root_region);
		cmzn_graphics_module_destroy(&graphics_module);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"User_interface_module_create.  Invalid argument(s)");
	}

	return UI_module;
}

struct User_interface_module *User_interface_module_access(
	struct User_interface_module *UI_module)
{
	if (UI_module)
	{
		UI_module->access_count++;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"User_interface_module_access.  Invalid argument(s)");
	}

	return UI_module;
}

int User_interface_module_destroy(
	struct User_interface_module **UI_module_address)
{
	int return_code = 0;
	struct User_interface_module *UI_module  = NULL;

	if (UI_module_address && NULL != (UI_module = *UI_module_address))
	{
		UI_module->access_count--;
		if (0 == UI_module->access_count)
		{
#if defined (USE_CMGUI_GRAPHICS_WINDOW)
			if (UI_module->sceneviewermodule)
			{
				DESTROY(cmzn_sceneviewermodule_app)(&UI_module->sceneviewermodule);
			}
#endif /* defined (USE_CMGUI_GRAPHICS_WINDOW) */
#if defined (USE_CMGUI_GRAPHICS_WINDOW)
			DESTROY(MANAGER(Graphics_window))(&UI_module->graphics_window_manager);
			/* Must destroy the graphics_buffer_package after the windows which use it */
			if (UI_module->graphics_buffer_package)
			{
				DESTROY(Graphics_buffer_app_package)(&UI_module->graphics_buffer_package);
			}
#endif /* defined (USE_CMGUI_GRAPHICS_WINDOW) */
			DESTROY(MANAGER(Interactive_tool))(&(UI_module->interactive_tool_manager));

#if defined (WX_USER_INTERFACE)
			DESTROY(MANAGER(Comfile_window))(&UI_module->comfile_window_manager);
#endif /* defined (WX_USER_INTERFACE) */
			if (UI_module->default_time_keeper_app)
				DEACCESS(Time_keeper_app)(&UI_module->default_time_keeper_app);
			if (UI_module->user_interface)
			{
				/* close the user interface */
				DESTROY(User_interface)(&(UI_module->user_interface));
			}
			if (UI_module->cleanup_argv != NULL)
			{
				for (int ai = 0; ai < UI_module->cleanup_argc; ai++)
				{
					DEALLOCATE(UI_module->cleanup_argv[ai]);
				}
				DEALLOCATE(UI_module->cleanup_argv);
				DEALLOCATE(UI_module->unmodified_argv);
				DEALLOCATE(UI_module->argv);
			}
			DEALLOCATE(*UI_module_address);
		}
		*UI_module_address = NULL;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
				"User_interface_module_destroy.  Missing user interface module address");
		return_code = 0;
	}

	return return_code;
}
