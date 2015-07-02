/* OpenCMISS-Cmgui Application
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "zinc/glyph.h"
#include "zinc/material.h"
#include "zinc/status.h"
#include "zinc/stream.h"
#include "zinc/streamscene.h"
#include "zinc/tessellation.h"
#include "general/debug.h"
#include "general/enumerator_private_app.h"
#include "general/message.h"
#include "command/parser.h"
#include "graphics/glyph.hpp"
#include "graphics/graphics_module.h"
#include "graphics/scene.h"
#include "graphics/render_gl.h"
#include "graphics/auxiliary_graphics_types_app.h"
#include "computed_field/computed_field_set.h"
#include "computed_field/computed_field_set_app.h"
#include "graphics/tessellation.hpp"
#include "graphics/tessellation_app.hpp"
#include "user_interface/process_list_or_write_command.hpp"
#include "finite_element/finite_element_region_app.h"
#include "graphics/font.h"
// insert app headers here
#include "graphics/graphics_app.h"
#include "general/enumerator_private.hpp"
#include "graphics/scene_app.h"
#include "region/cmiss_region_private.h"
#include "region/cmiss_region_app.h"
#include "user_interface/user_interface.h"

#if 1
#include "configure/cmgui_configure.h"
#endif /* defined (BUILD_WITH_CMAKE) */

int gfx_modify_scene_general(struct Parse_state *state,
	void *cmiss_region_void, void *group_void)
{
	int return_code = 0;

	ENTER(gfx_modify_scene_general);
	cmzn_region_id cmiss_region = reinterpret_cast<cmzn_region_id>(cmiss_region_void);
	cmzn_field_group_id group = reinterpret_cast<cmzn_field_group_id>(group_void);
	if (state && cmiss_region)
	{
		/* if possible, get defaults from element_group on default scene */
		cmzn_scene_id scene = cmzn_region_get_scene(cmiss_region);
		if (scene)
		{
			cmzn_field_id default_coordinate_field = scene->default_coordinate_field;
			if (default_coordinate_field)
			{
				cmzn_field_access(default_coordinate_field);
			}
			int clear_flag = 0;

			Option_table *option_table = CREATE(Option_table)();
			Option_table_add_help(option_table,
				"The clear option removes all graphics from the scene. "
				"Option 'circle_discretization' is deprecated:  use tessellation option on individual graphics instead. "
				"Option 'default_coordinate' is deprecated: use coordinate option on individual graphics instead. "
				"Option 'element_discretization' is deprecated: use tessellation option on individual graphics instead. "
				"Option 'native_discretization' is obsolete: use native_discretization option on individual graphics instead. ");
			/* circle_discretization */
			Option_table_add_entry(option_table, "circle_discretization",
				(void *)&scene->circle_discretization, (void *)NULL,
				set_circle_divisions);
			/* clear */
			Option_table_add_entry(option_table, "clear",
				(void *)&clear_flag, NULL, set_char_flag);
			/* default_coordinate */
			Set_Computed_field_conditional_data set_coordinate_field_data;
			set_coordinate_field_data.computed_field_manager=
				cmzn_region_get_Computed_field_manager(cmiss_region);
			set_coordinate_field_data.conditional_function=
				Computed_field_has_up_to_3_numerical_components;
			set_coordinate_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table, "default_coordinate",
				(void *)&default_coordinate_field, (void *)&set_coordinate_field_data,
				set_Computed_field_conditional);
			/* element_discretization */
			Option_table_add_divisions_entry(option_table, "element_discretization",
				&(scene->element_divisions), &(scene->element_divisions_size));
			/* native_discretization */
			char *native_discretization_field_string = 0;
			Option_table_add_string_entry(option_table, "native_discretization",
				&native_discretization_field_string, " OBSOLETE");
			return_code = Option_table_multi_parse(option_table, state);
			DESTROY(Option_table)(&option_table);
			if (return_code)
			{
				if ((scene->circle_discretization > 0) && (scene->circle_discretization < 12))
				{
					// silently use minimum of 12 otherwise sphere glyphs will look poor
					scene->circle_discretization = 12;
				}
				if ((native_discretization_field_string) &&
					(!fuzzy_string_compare_same_length(native_discretization_field_string, "none")))
				{
					display_parse_state_location(state);
					display_message(WARNING_MESSAGE,
						"Option 'native_discretization FIELD' on gfx modify g_element general has been removed.\n"
						"Please move this option to individual graphics instead.");
				}
				if (clear_flag)
				{
					if (group)
					{
						// remove only graphics using group as subgroup field
						cmzn_scene_begin_change(scene);
						cmzn_field_id group_field = cmzn_field_group_base_cast(group);
						// support legacy command files by changing visibility of each graphics using group as its subgroup field
						cmzn_graphics_id graphics = cmzn_scene_get_first_graphics(scene);
						while (graphics)
						{
							cmzn_graphics_id this_graphics = graphics;
							graphics = cmzn_scene_get_next_graphics(scene, this_graphics);
							cmzn_field_id subgroup_field = cmzn_graphics_get_subgroup_field(this_graphics);
							if (subgroup_field == group_field)
							{
								cmzn_scene_remove_graphics(scene, this_graphics);
							}
							cmzn_field_destroy(&subgroup_field);
							cmzn_graphics_destroy(&this_graphics);
						}
						cmzn_scene_end_change(scene);
					}
					else
					{
						return_code = (CMZN_OK == cmzn_scene_remove_all_graphics(scene));
					}
				}
				if (default_coordinate_field)
				{
					if (scene->default_coordinate_field && default_coordinate_field &&
						(scene->default_coordinate_field != default_coordinate_field))
					{
						display_message(WARNING_MESSAGE,
							"Change of default_coordinate field can have unexpected results. "
							"Please specify coordinate field for each graphics instead.");
						display_parse_state_location(state);
					}
					cmzn_scene_set_default_coordinate_field(scene, default_coordinate_field);
				}
			}
			if (default_coordinate_field)
			{
				DEACCESS(Computed_field)(&default_coordinate_field);
			}
			cmzn_scene_destroy(&scene);
			if (native_discretization_field_string)
			{
				DEALLOCATE(native_discretization_field_string);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_modify_scene_general.  Missing scene");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_modify_scene_general.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;
	return (return_code);
}


struct cmzn_scene_process_temp_data
{
	 class Process_list_or_write_command_class *process_message;
	 struct cmzn_graphics_list_data *list_data;
};

/***************************************************************************//**
 * Writes out the <graphics> as a text string in the command window with the
 * <graphics_string_detail>, <line_prefix> and <line_suffix> given in the
 * <list_data>.
 */
int cmzn_scene_process_cmzn_graphics_list_contents(
	 struct cmzn_graphics *graphics,	void *process_temp_data_void)
{
	int return_code;
	char *graphics_string; /*,line[40];*/
	struct cmzn_graphics_list_data *list_data;
	class Process_list_or_write_command_class *process_message;
	struct cmzn_scene_process_temp_data *process_temp_data;

	ENTER(cmzn_scene_process_cmzn_graphics_list_contents);
	if (graphics&&
		 (process_temp_data=(struct cmzn_scene_process_temp_data *)process_temp_data_void))
	{
		 if (NULL != (process_message = process_temp_data->process_message) &&
			 NULL != (list_data = process_temp_data->list_data))
		 {
			 if (NULL != (graphics_string=cmzn_graphics_string(graphics,
						 list_data->graphics_string_detail)))
				{
					 if (list_data->line_prefix)
					 {
							process_message->process_command(INFORMATION_MESSAGE,list_data->line_prefix);
					 }
					 process_message->process_command(INFORMATION_MESSAGE,"%s",graphics_string);
					 if (list_data->line_suffix)
					 {
							process_message->process_command(INFORMATION_MESSAGE,list_data->line_suffix);
					 }
					 process_message->process_command(INFORMATION_MESSAGE,"\n");
					 DEALLOCATE(graphics_string);
					 return_code=1;
				}
			 return_code= 1;
		 }
		 else
		 {
				return_code=0;
		 }
	}
	else
	{
		 display_message(ERROR_MESSAGE,
				"cmzn_scene_process_cmzn_graphics_list_contents.  Invalid argument(s)");
		 return_code=0;
	}
	LEAVE;

	return (return_code);
}

/***************************************************************************//**
 * Will write commands to comfile or list commands to the command windows
 * depending on the class.
 */
int cmzn_scene_process_list_or_write_window_commands(struct cmzn_scene *scene,
	 const char *command_prefix, const char *command_suffix, class Process_list_or_write_command_class *process_message)
{
	int return_code;
	struct cmzn_graphics_list_data list_data;

	ENTER(cmzn_scene_process_list_or_write_window_command);
	if (scene && command_prefix)
	{
		process_message->process_command(INFORMATION_MESSAGE,command_prefix);
		process_message->process_command(INFORMATION_MESSAGE,"general clear");
		if (command_suffix)
		{
			process_message->process_command(INFORMATION_MESSAGE,command_suffix);
		}
		process_message->process_command(INFORMATION_MESSAGE,"\n");
		list_data.graphics_string_detail=GRAPHICS_STRING_COMPLETE;
		list_data.line_prefix=command_prefix;
		list_data.line_suffix=command_suffix;
		struct cmzn_scene_process_temp_data *process_temp_data;
		if (ALLOCATE(process_temp_data,struct cmzn_scene_process_temp_data,1))
		{
			 process_temp_data->process_message = process_message;
			 process_temp_data->list_data = &list_data;
			 return_code=FOR_EACH_OBJECT_IN_LIST(cmzn_graphics)(
					cmzn_scene_process_cmzn_graphics_list_contents,(void *)process_temp_data,
					scene->list_of_graphics);
			 DEALLOCATE(process_temp_data);
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmzn_scene_list_commands.  Invalid arguments");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* cmzn_scene_process_list_or_write_window_command*/

int cmzn_scene_list_commands(struct cmzn_scene *scene,
	 const char *command_prefix, const char *command_suffix)
{
	 int return_code = 0;

	 ENTER(cmzn_scene_list_commands);
	 Process_list_command_class *list_message =
			new Process_list_command_class();
	 if (list_message)
	 {
			return_code = cmzn_scene_process_list_or_write_window_commands(
				scene, command_prefix, command_suffix, list_message);
			delete list_message;
	 }
	 LEAVE;

	 return (return_code);
}

int cmzn_scene_list_contents(struct cmzn_scene *scene)
{
	char *name = 0;
	int return_code;
	struct cmzn_graphics_list_data list_data;

	ENTER(cmzn_scene_list_contents);
	if (scene)
	{
		if (scene->circle_discretization)
		{
			display_message(INFORMATION_MESSAGE,"  Legacy circle discretization: %d\n",
				scene->circle_discretization);
		}
		if (scene->default_coordinate_field)
		{
			if (GET_NAME(Computed_field)(scene->default_coordinate_field,
				&name))
			{
				display_message(INFORMATION_MESSAGE,
					"  Legacy default coordinate field: %s\n",name);
				DEALLOCATE(name);
			}
		}
		if (scene->element_divisions)
		{
			display_message(INFORMATION_MESSAGE, "  Legacy element discretization: \"");
			for (int i = 0; i < scene->element_divisions_size; i++)
			{
				if (i)
					display_message(INFORMATION_MESSAGE, "*");
				display_message(INFORMATION_MESSAGE, "%d", scene->element_divisions[i]);
			}
			display_message(INFORMATION_MESSAGE, "\"\n");
		}
		if (0 < NUMBER_IN_LIST(cmzn_graphics)(
			scene->list_of_graphics))
		{
			display_message(INFORMATION_MESSAGE,"  graphics objects defined:\n");
			list_data.graphics_string_detail=GRAPHICS_STRING_COMPLETE_PLUS;
			list_data.line_prefix="  ";
			list_data.line_suffix="";
			return_code=FOR_EACH_OBJECT_IN_LIST(cmzn_graphics)(
				cmzn_graphics_list_contents,(void *)&list_data,
				scene->list_of_graphics);
		}
		else
		{
			display_message(INFORMATION_MESSAGE,"  no graphics graphics defined\n");
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmzn_scene_list_contents.  Invalid arguments");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* cmzn_scene_list_contents */

int cmzn_scene_fill_scene_command_data(cmzn_scene_id scene,
	struct Scene_command_data *scene_command_data)
{
	int return_code = 0;
	if (scene)
	{
		scene_command_data->graphics_module = scene->graphics_module;
		scene_command_data->scene = scene;
		scene_command_data->materialmodule =
			cmzn_graphics_module_get_materialmodule(scene->graphics_module);
		scene_command_data->default_material =
			cmzn_materialmodule_get_default_material(scene_command_data->materialmodule);
		scene_command_data->default_font =
			cmzn_graphics_module_get_default_font(scene->graphics_module);
		scene_command_data->spectrum_manager =
			cmzn_graphics_module_get_spectrum_manager(scene->graphics_module);
		cmzn_spectrummodule_id spectrummodule =
			cmzn_graphics_module_get_spectrummodule(scene->graphics_module);
		scene_command_data->default_spectrum =
				cmzn_spectrummodule_get_default_spectrum(spectrummodule);
		cmzn_spectrummodule_destroy(&spectrummodule);
		scene_command_data->glyphmodule =
			cmzn_graphics_module_get_glyphmodule(scene->graphics_module);
		scene_command_data->computed_field_manager =
			 cmzn_region_get_Computed_field_manager(scene->region);
		scene_command_data->region = scene->region;
		scene_command_data->root_region = cmzn_region_get_root(scene->region);
		scene_command_data->tessellationmodule =
			cmzn_graphics_module_get_tessellationmodule(scene->graphics_module);
		return_code = 1;
	}
	return return_code;
}

int cmzn_scene_cleanup_scene_command_data(
	struct Scene_command_data *scene_command_data)
{
	int return_code = 0;
	if (scene_command_data)
	{
		cmzn_materialmodule_destroy(&(scene_command_data->materialmodule));
		cmzn_material_destroy(&scene_command_data->default_material);
		cmzn_font_destroy(&scene_command_data->default_font);
		cmzn_spectrum_destroy(&scene_command_data->default_spectrum);
		cmzn_glyphmodule_destroy(&(scene_command_data->glyphmodule));
		cmzn_tessellationmodule_destroy(&(scene_command_data->tessellationmodule));
		cmzn_region_destroy(&(scene_command_data->root_region));
		return_code = 1;
	}
	return return_code;
}

int cmzn_scene_execute_command_internal(cmzn_scene_id scene,
	cmzn_field_group_id group, struct Parse_state *state)
{
	int return_code = 0;
	if (scene && state)
	{
		struct cmzn_graphics_module *graphics_module =	scene->graphics_module;
		if(graphics_module)
		{
			struct Option_table *option_table;
			struct Modify_scene_data modify_scene_data;
			modify_scene_data.delete_flag = 0;
			modify_scene_data.position = -1;
			modify_scene_data.graphics = (struct cmzn_graphics *)NULL;
			modify_scene_data.modify_this_graphics = 0;
			modify_scene_data.group = group;
			int previous_state_index;
			if (state && (previous_state_index = state->current_index))
			{
				option_table = CREATE(Option_table)();
				/* as */
				char *graphics_name = (char *)NULL;
				Option_table_add_name_entry(option_table, "as", &graphics_name);
				/* default to absorb everything else */
				char *dummy_string = (char *)NULL;
				Option_table_add_name_entry(option_table, (char *)NULL, &dummy_string);
				return_code = Option_table_multi_parse(option_table, state);
				DESTROY(Option_table)(&option_table);
				if (return_code)
				{
					if (graphics_name && (modify_scene_data.graphics =
						cmzn_scene_get_first_graphics_with_condition(
							scene, cmzn_graphics_has_name, (void *)graphics_name)))
					{
						modify_scene_data.modify_this_graphics = 1;
						ACCESS(cmzn_graphics)(modify_scene_data.graphics);
					}
				}
				if (dummy_string)
				{
					DEALLOCATE(dummy_string);
				}
				if (graphics_name)
				{
					DEALLOCATE(graphics_name);
				}
				/* Return back to where we were */
				shift_Parse_state(state, previous_state_index - state->current_index);
			}

			struct Scene_command_data scene_command_data;
			cmzn_scene_fill_scene_command_data(scene,
				&scene_command_data);

			option_table = CREATE(Option_table)();
			/* contours */
			Option_table_add_entry(option_table, "contours",
				(void *)&modify_scene_data, (void *)&scene_command_data,
				gfx_modify_scene_contours);
			/* cylinders */
			Option_table_add_entry(option_table, "cylinders",
				(void *)&modify_scene_data, (void *)&scene_command_data,
				gfx_modify_scene_cylinders);
			/* data_points */
			Option_table_add_entry(option_table, "data_points",
				(void *)&modify_scene_data, (void *)&scene_command_data,
				gfx_modify_scene_data_points);
			/* element_points */
			Option_table_add_entry(option_table, "element_points",
				(void *)&modify_scene_data, (void *)&scene_command_data,
				gfx_modify_scene_element_points);
			/* general */
			Option_table_add_entry(option_table, "general",
				(void *)scene->region, (void *)group, gfx_modify_scene_general);
			/* iso_surfaces */
			Option_table_add_entry(option_table, "iso_surfaces",
				(void *)&modify_scene_data, (void *)&scene_command_data,
				gfx_modify_scene_iso_surfaces);
			/* lines */
			Option_table_add_entry(option_table, "lines",
				(void *)&modify_scene_data, (void *)&scene_command_data,
				gfx_modify_scene_lines);
			/* node_points */
			Option_table_add_entry(option_table, "node_points",
				(void *)&modify_scene_data, (void *)&scene_command_data,
				gfx_modify_scene_node_points);
			/* point */
			Option_table_add_entry(option_table, "point",
				(void *)&modify_scene_data, (void *)&scene_command_data,
				gfx_modify_scene_point);
			/* points */
			Option_table_add_entry(option_table, "points",
				(void *)&modify_scene_data, (void *)&scene_command_data,
				gfx_modify_scene_points);
			/* streamlines */
			Option_table_add_entry(option_table, "streamlines",
				(void *)&modify_scene_data, (void *)&scene_command_data,
				gfx_modify_scene_streamlines);
			/* surfaces */
			Option_table_add_entry(option_table, "surfaces",
				(void *)&modify_scene_data, (void *)&scene_command_data,
				gfx_modify_scene_surfaces);

			return_code = Option_table_parse(option_table, state);
			if (return_code && (modify_scene_data.graphics))
			{
				return_code = cmzn_region_modify_scene(scene->region,
					modify_scene_data.graphics,
					modify_scene_data.delete_flag,
					modify_scene_data.position);
			} /* parse error,help */
			DESTROY(Option_table)(&option_table);
			cmzn_graphics_destroy(&modify_scene_data.graphics);
			cmzn_scene_cleanup_scene_command_data(&scene_command_data);
		}
	}
	return return_code;
}

int cmzn_scene_execute_command(cmzn_scene_id scene, const char *command_string)
{
	int return_code = 0;
	if (scene && command_string)
	{
		struct Parse_state *state = create_Parse_state(command_string);
		return_code = cmzn_scene_execute_command_internal(scene, (cmzn_field_group_id)0, state);
		destroy_Parse_state(&state);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_cmzn_scene_transformation_commands.  Invalid argument(s)");
		return_code=0;
	}
	return return_code;
}

// GRC needed?
int cmzn_scene_add_glyph(struct cmzn_scene *scene,
	cmzn_glyph *glyph, const char *graphics_name)
{
	int return_code = 0;
	if (scene && glyph)
	{
		if (!FIRST_OBJECT_IN_LIST_THAT(cmzn_graphics)(cmzn_graphics_has_name,
				(void *)graphics_name, scene->list_of_graphics))
		{
			cmzn_scene_begin_change(scene);
			cmzn_graphics *graphics = cmzn_scene_create_graphics_points(scene);
			cmzn_graphics_set_name(graphics, graphics_name);
			cmzn_graphicspointattributes_id point_attributes = cmzn_graphics_get_graphicspointattributes(graphics);
			cmzn_graphicspointattributes_set_glyph(point_attributes, reinterpret_cast<cmzn_glyph_id>(glyph));
			const double one = 1.0;
			cmzn_graphicspointattributes_set_base_size(point_attributes, 1, &one);
			cmzn_graphicspointattributes_destroy(&point_attributes);
			cmzn_graphics_destroy(&graphics);
			cmzn_scene_end_change(scene);
			return_code = 1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"cmzn_scene_add_glyph.  Graphics with the same name already exists");
		}
	}
	return return_code;
}

cmzn_graphics* cmzn_scene_create_graphics_app(cmzn_scene *scene,
	cmzn_graphics_type graphics_type, cmzn_graphics *graphics_to_copy)
{
	cmzn_graphics *graphics = CREATE(cmzn_graphics)(graphics_type);
	if (graphics_to_copy &&
		(graphics_type == cmzn_graphics_get_type(graphics_to_copy)))
	{
		cmzn_graphics_copy_without_graphics_object(graphics, graphics_to_copy);
	}
	else
	{
		cmzn_scene_set_minimum_graphics_defaults(scene, graphics);
		cmzn_graphicslineattributes_id line_attributes = cmzn_graphics_get_graphicslineattributes(graphics);
		cmzn_graphicspointattributes_id point_attributes = cmzn_graphics_get_graphicspointattributes(graphics);
		if (graphics_type == CMZN_GRAPHICS_TYPE_STREAMLINES)
		{
			// use previous default of 1.0 for streamline width
			const double one = 1.0;
			cmzn_graphicslineattributes_set_base_size(line_attributes, 1, &one);
		}
		if (point_attributes)
		{
			// use previous default of 1.0 for base size
			const double one = 1.0;
			cmzn_graphicspointattributes_set_base_size(point_attributes, 1, &one);
		}
		cmzn_graphicslineattributes_destroy(&line_attributes);
		cmzn_graphicspointattributes_destroy(&point_attributes);
	}
	return (graphics);
}

int set_Scene(struct Parse_state *state,
	void *scene_address_void,void *root_region_void)
/*******************************************************************************
LAST MODIFIED : 22 June 1999

DESCRIPTION :
Modifier function to set the scene from a command.
???RC set_Object routines could become a macro.
==============================================================================*/
{
	const char *current_token;
	int return_code = 1;
	struct cmzn_region *root_region;
	cmzn_scene **scene_address = 0;

	if (state && (root_region = static_cast<struct cmzn_region *>(root_region_void)) &&
		(scene_address = static_cast<struct cmzn_scene **>(scene_address_void)))
	{
		if ((current_token = state->current_token))
		{
			if (!Parse_state_help_mode(state))
			{
				cmzn_region *scene_region = cmzn_region_access(root_region);
				if ((0 != strcmp(current_token, "default")))
				{
					display_message(INFORMATION_MESSAGE,
						"set_Scene:  scene is now a graphical representation of a region,"
						"specified scene will be interpret as region path");
					return_code = set_cmzn_region(state, &scene_region, root_region);
				}
				else
				{
					return_code = shift_Parse_state(state, 1);
				}
				if (return_code && scene_region)
				{
					cmzn_scene_destroy(scene_address);
					*scene_address = cmzn_region_get_scene(scene_region);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"set_Scene:  Could not find scene %s", current_token);
					display_parse_state_location(state);
					return_code = 0;
				}
				cmzn_region_destroy(&scene_region);
			}
			else
			{
				display_message(INFORMATION_MESSAGE," PATH_TO_SCENE");
				if (*scene_address)
				{
					struct cmzn_region *region = cmzn_scene_get_region_internal(*scene_address);
					char *path = cmzn_region_get_path(region);
					display_message(INFORMATION_MESSAGE, "[%s]", path);
					DEALLOCATE(path);
				}
				return_code = 1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE, "Missing scene path");
			display_parse_state_location(state);
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE, "set_Scene.  Missing state");
		return_code = 0;
	}

	return (return_code);
} /* set_Scene */

int define_Scene(struct Parse_state *state, void *dummy_to_be_modified,
	void *define_scene_data_void)
{
	display_message(WARNING_MESSAGE, "To define a filter and light on a graphics window please see "
		"gfx modify window command");

	return 1;
}

static cmzn_scene_id cmzn_scene_get_parent_scene_internal(cmzn_scene_id scene)
{
	cmzn_scene_id parent_scene = 0;
	if (scene)
	{
		cmzn_region_id parent_region = cmzn_region_get_parent_internal(scene->region);
		if (parent_region)
		{
			parent_scene = FIRST_OBJECT_IN_LIST_THAT(ANY_OBJECT(cmzn_scene))(
				(ANY_OBJECT_CONDITIONAL_FUNCTION(cmzn_scene) *)NULL, (void *)NULL,
				cmzn_region_private_get_any_object_list(parent_region));
		}
	}
	return parent_scene;
}

cmzn_field_group_id cmzn_scene_get_or_create_selection_group(cmzn_scene_id scene)
{
	if (!scene)
		return 0;
	cmzn_field_group_id selection_group = scene->selection_group;
	if (selection_group)
		cmzn_field_access(cmzn_field_group_base_cast(selection_group));
	else
	{
		cmzn_scene_id parent_scene = cmzn_scene_get_parent_scene_internal(scene);
		if (parent_scene)
		{
			cmzn_field_group_id parent_selection_group = cmzn_scene_get_or_create_selection_group(parent_scene);
			selection_group = cmzn_field_group_get_subregion_field_group(parent_selection_group, scene->region);
			if (!selection_group)
				selection_group = cmzn_field_group_create_subregion_field_group(parent_selection_group, scene->region);
			cmzn_field_group_destroy(&parent_selection_group);
		}
		else
		{
			// find by name or create
			const char *default_selection_group_name = "cmiss_selection";
			cmzn_fieldmodule_id field_module = cmzn_region_get_fieldmodule(scene->region);
			cmzn_field_id field = cmzn_fieldmodule_find_field_by_name(field_module, default_selection_group_name);
			if (field)
			{
				selection_group = cmzn_field_cast_group(field);
				cmzn_field_destroy(&field);
			}
			if (!selection_group)
			{
				field = cmzn_fieldmodule_create_field_group(field_module);
				cmzn_field_set_name(field, default_selection_group_name);
				selection_group = cmzn_field_cast_group(field);
				cmzn_field_group_set_subelement_handling_mode(selection_group, CMZN_FIELD_GROUP_SUBELEMENT_HANDLING_MODE_FULL);
				cmzn_field_destroy(&field);
			}
			cmzn_fieldmodule_destroy(&field_module);
		}
		if (selection_group)
			cmzn_scene_set_selection_field(scene, cmzn_field_group_base_cast(selection_group));
	}
	return selection_group;
}

int cmzn_scene_change_selection_from_node_list(cmzn_scene_id scene,
		struct LIST(FE_node) *node_list, int add_flag, int use_data)
{
	int return_code = 1;

	ENTER(cmzn_scene_add_selection_from_node_list);
	if (scene && node_list && (NUMBER_IN_LIST(FE_node)(node_list) > 0))
	{
		cmzn_fieldmodule_id field_module = cmzn_region_get_fieldmodule(scene->region);
		cmzn_fieldmodule_begin_change(field_module);
		cmzn_field_group_id selection_group = cmzn_scene_get_or_create_selection_group(scene);
		cmzn_nodeset_id temp_nodeset = cmzn_fieldmodule_find_nodeset_by_field_domain_type(
			field_module, use_data ? CMZN_FIELD_DOMAIN_TYPE_DATAPOINTS : CMZN_FIELD_DOMAIN_TYPE_NODES);
		cmzn_field_node_group_id node_group = cmzn_field_group_get_field_node_group(selection_group, temp_nodeset);
		if (!node_group)
			node_group = cmzn_field_group_create_field_node_group(selection_group, temp_nodeset);
		cmzn_nodeset_destroy(&temp_nodeset);
		cmzn_nodeset_group_id nodeset_group = cmzn_field_node_group_get_nodeset_group(node_group);
		cmzn_field_node_group_destroy(&node_group);
		cmzn_nodeiterator_id iterator = CREATE_LIST_ITERATOR(FE_node)(node_list);
		cmzn_node_id node = 0;
		while (0 != (node = cmzn_nodeiterator_next_non_access(iterator)))
		{
			if (add_flag)
			{
				cmzn_nodeset_group_add_node(nodeset_group, node);
			}
			else
			{
				cmzn_nodeset_group_remove_node(nodeset_group, node);
			}
		}
		cmzn_nodeiterator_destroy(&iterator);
		cmzn_nodeset_group_destroy(&nodeset_group);
		cmzn_field_group_destroy(&selection_group);
		cmzn_fieldmodule_end_change(field_module);
		cmzn_fieldmodule_destroy(&field_module);
	}
	LEAVE;

	return (return_code);
}

int cmzn_scene_add_selection_from_node_list(cmzn_scene_id scene,
	struct LIST(FE_node) *node_list, int use_data)
/*******************************************************************************
LAST MODIFIED : 28 April 2000

DESCRIPTION :
Create a node list selection
==============================================================================*/
{
	int return_code = 0;
	return_code = cmzn_scene_change_selection_from_node_list(scene,
		node_list, /*add_flag*/1, use_data);
	return return_code;
}

int cmzn_scene_remove_selection_from_node_list(cmzn_scene_id scene,
	struct LIST(FE_node) *node_list, int use_data)
{
	int return_code = 0;
	if (cmzn_scene_change_selection_from_node_list(scene,
		node_list, /*add_flag*/0, use_data))
	{
		cmzn_scene_flush_tree_selections(scene);
		return_code = 1;
	}
	return return_code;
}


int cmzn_scene_change_selection_from_element_list_of_dimension(cmzn_scene_id scene,
	struct LIST(FE_element) *element_list, int add_flag, int dimension)
{
	int return_code = 1;

	ENTER(cmzn_scene_change_selection_from_element_list_of_dimension);
	if (scene && element_list && (NUMBER_IN_LIST(FE_element)(element_list) > 0))
	{
		cmzn_fieldmodule_id field_module = cmzn_region_get_fieldmodule(scene->region);
		cmzn_fieldmodule_begin_change(field_module);
		cmzn_field_group_id selection_group = cmzn_scene_get_or_create_selection_group(scene);
		cmzn_mesh_id temp_mesh = cmzn_fieldmodule_find_mesh_by_dimension(field_module, dimension);
		cmzn_field_element_group_id element_group = cmzn_field_group_get_field_element_group(selection_group, temp_mesh);
		if (!element_group)
			element_group = cmzn_field_group_create_field_element_group(selection_group, temp_mesh);
		cmzn_mesh_destroy(&temp_mesh);
		cmzn_mesh_group_id mesh_group = cmzn_field_element_group_get_mesh_group(element_group);
		cmzn_field_element_group_destroy(&element_group);
		cmzn_elementiterator_id iterator = CREATE_LIST_ITERATOR(FE_element)(element_list);
		cmzn_element_id element = 0;
		while (0 != (element = cmzn_elementiterator_next_non_access(iterator)))
		{
			if (add_flag)
			{
				cmzn_mesh_group_add_element(mesh_group, element);
			}
			else
			{
				cmzn_mesh_group_remove_element(mesh_group, element);
			}
		}
		cmzn_elementiterator_destroy(&iterator);
		cmzn_mesh_group_destroy(&mesh_group);
		cmzn_field_group_destroy(&selection_group);
		cmzn_fieldmodule_end_change(field_module);
		cmzn_fieldmodule_destroy(&field_module);
	}
	LEAVE;

	return (return_code);
}

int cmzn_scene_add_selection_from_element_list_of_dimension(cmzn_scene_id scene,
	struct LIST(FE_element) *element_list, int dimension)
{
	return cmzn_scene_change_selection_from_element_list_of_dimension(scene,
		element_list, /*add_flag*/1, dimension);
}

int cmzn_scene_remove_selection_from_element_list_of_dimension(cmzn_scene_id scene,
	struct LIST(FE_element) *element_list, int dimension)
{
	int return_code = 0;
	if (cmzn_scene_change_selection_from_element_list_of_dimension(scene,
			element_list, /*add_flag*/0, dimension))
	{
		cmzn_scene_flush_tree_selections(scene);
		return_code = 1;
	}
	return return_code;
}

int scene_app_export_threejs(cmzn_scene_id scene, cmzn_scenefilter_id scenefilter,
	char *file_prefix, int number_of_time_steps, double begin_time, double end_time,
	cmzn_streaminformation_scene_io_data_type data_type,
	int morphVertices, int morphColours, int morphNormals)
{
	if (scene)
	{
		int return_code = 0;
		cmzn_streaminformation_id streaminformation = cmzn_scene_create_streaminformation_scene(scene);
		cmzn_streaminformation_scene_id streaminformation_scene = cmzn_streaminformation_cast_scene(
			streaminformation);
		cmzn_streaminformation_scene_set_scenefilter(streaminformation_scene, scenefilter);
		cmzn_streaminformation_scene_set_io_format(
			streaminformation_scene, CMZN_STREAMINFORMATION_SCENE_IO_FORMAT_THREEJS);
		cmzn_streaminformation_scene_set_number_of_time_steps(
			streaminformation_scene, number_of_time_steps);
		cmzn_streaminformation_scene_set_initial_time(
			streaminformation_scene,  begin_time);
		cmzn_streaminformation_scene_set_finish_time(
			streaminformation_scene, end_time);
		cmzn_streaminformation_scene_set_io_data_type(
			streaminformation_scene, data_type);
		cmzn_streaminformation_scene_set_output_time_dependent_vertices(
			streaminformation_scene, morphVertices);
		cmzn_streaminformation_scene_set_output_time_dependent_colours(
			streaminformation_scene, morphColours);
		cmzn_streaminformation_scene_set_output_time_dependent_normals(
			streaminformation_scene, morphNormals);

		int number_of_resources_required =
			cmzn_streaminformation_scene_get_number_of_resources_required(streaminformation_scene);
		cmzn_streamresource_id *streamresources = new cmzn_streamresource_id[number_of_resources_required];
		if (number_of_resources_required > 0)
		{
			char temp[200];
			for (int i = 0; i < number_of_resources_required; i++)
			{
				sprintf(temp, "%s_%d.json", file_prefix, i+1);
				streamresources[i] =
					cmzn_streaminformation_create_streamresource_file(streaminformation,temp);
			}
			return_code = cmzn_scene_export_scene(scene, streaminformation_scene);
		}
		for (int i = 0; i < number_of_resources_required; i++)
		{
			cmzn_streamresource_destroy(&(streamresources[i]));
		}
		delete[] streamresources;
		cmzn_streaminformation_scene_destroy(&streaminformation_scene);
		cmzn_streaminformation_destroy(&streaminformation);
		return 1;
	}

	return 0;
}

DEFINE_DEFAULT_OPTION_TABLE_ADD_ENUMERATOR_FUNCTION(cmzn_streaminformation_scene_io_data_type)

int cmzn_scene_set_graphics_defaults_gfx_modify(struct cmzn_scene *scene,
	struct cmzn_graphics *graphics)
{
	int return_code = 1;
	if (scene && graphics)
	{
		cmzn_graphics_type graphics_type = cmzn_graphics_get_type(graphics);
		cmzn_field_domain_type domain_type = cmzn_graphics_get_field_domain_type(graphics);

		if ((graphics_type != CMZN_GRAPHICS_TYPE_POINTS) || (domain_type != CMZN_FIELD_DOMAIN_TYPE_POINT))
		{
			cmzn_field_id coordinate_field = cmzn_scene_get_default_coordinate_field(scene);
			if (!coordinate_field)
				coordinate_field = cmzn_scene_guess_coordinate_field(scene, domain_type);
			if (coordinate_field)
				cmzn_graphics_set_coordinate_field(graphics, coordinate_field);
		}

		bool use_element_discretization = (0 != scene->element_divisions) &&
			(graphics_type != CMZN_GRAPHICS_TYPE_POINTS) && (graphics_type != CMZN_GRAPHICS_TYPE_STREAMLINES);
		bool use_circle_discretization = (scene->circle_discretization >= 3) &&
			(graphics_type == CMZN_GRAPHICS_TYPE_LINES);
		if (use_circle_discretization)
		{
			cmzn_graphicslineattributes_id lineAttr = cmzn_graphics_get_graphicslineattributes(graphics);
			use_circle_discretization = (cmzn_graphicslineattributes_get_shape_type(lineAttr) ==
				CMZN_GRAPHICSLINEATTRIBUTES_SHAPE_TYPE_CIRCLE_EXTRUSION);
			cmzn_graphicslineattributes_destroy(&lineAttr);
		}
		if (use_element_discretization || use_circle_discretization)
		{
			cmzn_tessellationmodule_id tessellationModule =
				cmzn_graphics_module_get_tessellationmodule(scene->graphics_module);
			cmzn_tessellation_id currentTessellation = cmzn_graphics_get_tessellation(graphics);
			cmzn_tessellation_id tessellation =
				cmzn_tessellationmodule_find_or_create_fixed_tessellation(tessellationModule,
					use_element_discretization ? scene->element_divisions_size : 0,
					use_element_discretization ? scene->element_divisions : 0,
					use_circle_discretization ? scene->circle_discretization : 0,
					currentTessellation);
			cmzn_graphics_set_tessellation(graphics, tessellation);
			cmzn_tessellation_destroy(&tessellation);
			cmzn_tessellation_destroy(&currentTessellation);
			cmzn_tessellationmodule_destroy(&tessellationModule);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmzn_scene_set_graphics_defaults_gfx_modify.  Invalid argument(s)");
		return_code = 0;
	}
	return return_code;
}
