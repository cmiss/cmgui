
#include "zinc/glyph.h"
#include "zinc/graphicsmaterial.h"
#include "zinc/status.h"
#include "zinc/tessellation.h"
#include "general/debug.h"
#include "general/message.h"
#include "command/parser.h"
#include "graphics/glyph.hpp"
#include "graphics/graphics_module.h"
#include "graphics/scene.h"
#include "graphics/render_gl.h"
#include "graphics/auxiliary_graphics_types_app.h"
#include "computed_field/computed_field_set.h"
#include "computed_field/computed_field_set_app.h"
#include "graphics/tessellation_app.hpp"
#include "user_interface/process_list_or_write_command.hpp"
#include "finite_element/finite_element_region_app.h"
#include "graphics/font.h"
// insert app headers here
#include "graphics/graphic_app.h"
#include "general/enumerator_private.hpp"
#include "graphics/scene_app.h"
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
		cmzn_scene_id scene = cmzn_region_get_scene_internal(cmiss_region);
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
						// support legacy command files by changing visibility of each graphic using group as its subgroup field
						cmzn_graphic_id graphic = cmzn_scene_get_first_graphic(scene);
						while (graphic)
						{
							cmzn_graphic_id this_graphic = graphic;
							graphic = cmzn_scene_get_next_graphic(scene, this_graphic);
							cmzn_field_id subgroup_field = cmzn_graphic_get_subgroup_field(this_graphic);
							if (subgroup_field == group_field)
							{
								cmzn_scene_remove_graphic(scene, this_graphic);
							}
							cmzn_field_destroy(&subgroup_field);
							cmzn_graphic_destroy(&this_graphic);
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
							"Please specify coordinate field for each graphic instead.");
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
	 struct cmzn_graphic_list_data *list_data;
};

/***************************************************************************//**
 * Writes out the <graphic> as a text string in the command window with the
 * <graphic_string_detail>, <line_prefix> and <line_suffix> given in the
 * <list_data>.
 */
int cmzn_scene_process_cmzn_graphic_list_contents(
	 struct cmzn_graphic *graphic,	void *process_temp_data_void)
{
	int return_code;
	char *graphic_string; /*,line[40];*/
	struct cmzn_graphic_list_data *list_data;
	class Process_list_or_write_command_class *process_message;
	struct cmzn_scene_process_temp_data *process_temp_data;

	ENTER(cmzn_scene_process_cmzn_graphic_list_contents);
	if (graphic&&
		 (process_temp_data=(struct cmzn_scene_process_temp_data *)process_temp_data_void))
	{
		 if (NULL != (process_message = process_temp_data->process_message) &&
			 NULL != (list_data = process_temp_data->list_data))
		 {
			 if (NULL != (graphic_string=cmzn_graphic_string(graphic,
						 list_data->graphic_string_detail)))
				{
					 if (list_data->line_prefix)
					 {
							process_message->process_command(INFORMATION_MESSAGE,list_data->line_prefix);
					 }
					 process_message->process_command(INFORMATION_MESSAGE,"%s",graphic_string);
					 if (list_data->line_suffix)
					 {
							process_message->process_command(INFORMATION_MESSAGE,list_data->line_suffix);
					 }
					 process_message->process_command(INFORMATION_MESSAGE,"\n");
					 DEALLOCATE(graphic_string);
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
				"cmzn_scene_process_cmzn_graphic_list_contents.  Invalid argument(s)");
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
	struct cmzn_graphic_list_data list_data;

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
		list_data.graphic_string_detail=GRAPHIC_STRING_COMPLETE;
		list_data.line_prefix=command_prefix;
		list_data.line_suffix=command_suffix;
		struct cmzn_scene_process_temp_data *process_temp_data;
		if (ALLOCATE(process_temp_data,struct cmzn_scene_process_temp_data,1))
		{
			 process_temp_data->process_message = process_message;
			 process_temp_data->list_data = &list_data;
			 return_code=FOR_EACH_OBJECT_IN_LIST(cmzn_graphic)(
					cmzn_scene_process_cmzn_graphic_list_contents,(void *)process_temp_data,
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
	struct cmzn_graphic_list_data list_data;

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
		if (0 < NUMBER_IN_LIST(cmzn_graphic)(
			scene->list_of_graphics))
		{
			display_message(INFORMATION_MESSAGE,"  graphics objects defined:\n");
			list_data.graphic_string_detail=GRAPHIC_STRING_COMPLETE_PLUS;
			list_data.line_prefix="  ";
			list_data.line_suffix="";
			return_code=FOR_EACH_OBJECT_IN_LIST(cmzn_graphic)(
				cmzn_graphic_list_contents,(void *)&list_data,
				scene->list_of_graphics);
		}
		else
		{
			display_message(INFORMATION_MESSAGE,"  no graphics graphic defined\n");
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
		scene_command_data->material_module =
			cmzn_graphics_module_get_material_module(scene->graphics_module);
		scene_command_data->default_material =
			cmzn_graphics_material_module_get_default_material(scene_command_data->material_module);
		scene_command_data->default_font =
			cmzn_graphics_module_get_default_font(scene->graphics_module);
		scene_command_data->spectrum_manager =
			cmzn_graphics_module_get_spectrum_manager(scene->graphics_module);
		cmzn_spectrum_module_id spectrum_module =
			cmzn_graphics_module_get_spectrum_module(scene->graphics_module);
		scene_command_data->default_spectrum =
				cmzn_spectrum_module_get_default_spectrum(spectrum_module);
		cmzn_spectrum_module_destroy(&spectrum_module);
		scene_command_data->glyph_module =
			cmzn_graphics_module_get_glyph_module(scene->graphics_module);
		scene_command_data->computed_field_manager =
			 cmzn_region_get_Computed_field_manager(scene->region);
		scene_command_data->region = scene->region;
		scene_command_data->root_region = cmzn_region_get_root(scene->region);
		scene_command_data->tessellation_module =
			cmzn_graphics_module_get_tessellation_module(scene->graphics_module);
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
		cmzn_graphics_material_module_destroy(&(scene_command_data->material_module));
		cmzn_graphics_material_destroy(&scene_command_data->default_material);
		cmzn_font_destroy(&scene_command_data->default_font);
		cmzn_spectrum_destroy(&scene_command_data->default_spectrum);
		cmzn_glyph_module_destroy(&(scene_command_data->glyph_module));
		cmzn_tessellation_module_destroy(&(scene_command_data->tessellation_module));
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
			modify_scene_data.graphic = (struct cmzn_graphic *)NULL;
			modify_scene_data.modify_this_graphic = 0;
			modify_scene_data.group = group;
			int previous_state_index;
			if (state && (previous_state_index = state->current_index))
			{
				option_table = CREATE(Option_table)();
				/* as */
				char *graphic_name = (char *)NULL;
				Option_table_add_name_entry(option_table, "as", &graphic_name);
				/* default to absorb everything else */
				char *dummy_string = (char *)NULL;
				Option_table_add_name_entry(option_table, (char *)NULL, &dummy_string);
				return_code = Option_table_multi_parse(option_table, state);
				DESTROY(Option_table)(&option_table);
				if (return_code)
				{
					if (graphic_name && (modify_scene_data.graphic =
						cmzn_scene_get_first_graphic_with_condition(
							scene, cmzn_graphic_has_name, (void *)graphic_name)))
					{
						ACCESS(cmzn_graphic)(modify_scene_data.graphic);
					}
				}
				if (dummy_string)
				{
					DEALLOCATE(dummy_string);
				}
				if (graphic_name)
				{
					DEALLOCATE(graphic_name);
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
			if (return_code && (modify_scene_data.graphic))
			{
				return_code = cmzn_region_modify_scene(scene->region,
					modify_scene_data.graphic,
					modify_scene_data.delete_flag,
					modify_scene_data.position);
			} /* parse error,help */
			DESTROY(Option_table)(&option_table);
			cmzn_graphic_destroy(&modify_scene_data.graphic);
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
	cmzn_glyph *glyph, const char *cmiss_graphic_name)
{
	int return_code = 0;
	if (scene && glyph)
	{
		if (!FIRST_OBJECT_IN_LIST_THAT(cmzn_graphic)(cmzn_graphic_has_name,
				(void *)cmiss_graphic_name, scene->list_of_graphics))
		{
			cmzn_scene_begin_change(scene);
			cmzn_graphic *graphic = cmzn_graphic_points_base_cast(cmzn_scene_create_graphic_points(scene));
			cmzn_graphic_set_name(graphic, cmiss_graphic_name);
			cmzn_graphic_point_attributes_id point_attributes = cmzn_graphic_get_point_attributes(graphic);
			cmzn_graphic_point_attributes_set_glyph(point_attributes, reinterpret_cast<cmzn_glyph_id>(glyph));
			const double one = 1.0;
			cmzn_graphic_point_attributes_set_base_size(point_attributes, 1, &one);
			cmzn_graphic_point_attributes_destroy(&point_attributes);
			cmzn_graphic_destroy(&graphic);
			cmzn_scene_end_change(scene);
			return_code = 1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"cmzn_scene_add_glyph.  Graphic with the same name already exists");
		}
	}
	return return_code;
}

cmzn_graphic* cmzn_scene_create_graphic_app(cmzn_scene *scene,
	cmzn_graphic_type graphic_type, cmzn_graphic *graphic_to_copy)
{
	cmzn_graphic *graphic = CREATE(cmzn_graphic)(graphic_type);
	if (graphic_to_copy &&
		(graphic_type == cmzn_graphic_get_graphic_type(graphic_to_copy)))
	{
		cmzn_graphic_copy_without_graphics_object(graphic, graphic_to_copy);
	}
	else
	{
		cmzn_scene_set_minimum_graphic_defaults(scene, graphic);
		cmzn_graphic_line_attributes_id line_attributes = cmzn_graphic_get_line_attributes(graphic);
		cmzn_graphic_point_attributes_id point_attributes = cmzn_graphic_get_point_attributes(graphic);
		if (graphic_type == CMZN_GRAPHIC_STREAMLINES)
		{
			// use previous default of 1.0 for streamline width
			const double one = 1.0;
			cmzn_graphic_line_attributes_set_base_size(line_attributes, 1, &one);
		}
		if (point_attributes)
		{
			// use previous default of 1.0 for base size
			const double one = 1.0;
			cmzn_graphic_point_attributes_set_base_size(point_attributes, 1, &one);
		}
		cmzn_graphic_line_attributes_destroy(&line_attributes);
		cmzn_graphic_point_attributes_destroy(&point_attributes);
	}
	return (graphic);
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
					*scene_address = cmzn_region_get_scene_internal(scene_region);
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
					struct cmzn_region *region = cmzn_scene_get_region(*scene_address);
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

