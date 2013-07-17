
#include "zinc/glyph.h"
#include "zinc/graphicsmaterial.h"
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
	Cmiss_region_id cmiss_region = reinterpret_cast<Cmiss_region_id>(cmiss_region_void);
	Cmiss_field_group_id group = reinterpret_cast<Cmiss_field_group_id>(group_void);
	if (state && cmiss_region)
	{
		/* if possible, get defaults from element_group on default scene */
		Cmiss_scene_id scene = Cmiss_region_get_scene_internal(cmiss_region);
		if (scene)
		{
			Cmiss_field_id default_coordinate_field = scene->default_coordinate_field;
			if (default_coordinate_field)
			{
				Cmiss_field_access(default_coordinate_field);
			}
			int clear_flag = 0;

			Option_table *option_table = CREATE(Option_table)();
			Option_table_add_help(option_table,
				"The clear option removes all graphics from the scene. "
				"Option 'circle_discretization' is deprecated:  use tessellation option on individual graphics instead. "
				"Option 'default_coordinate' is deprecated: use coordinate option on individual graphics instead. "
				"Option 'element_discretization' is deprecated: use tessellation option on individual graphics instead. "
				"Option 'native_discretization' is deprecated: use native_discretization option on individual graphics instead. ");
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
				Cmiss_region_get_Computed_field_manager(cmiss_region);
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
			Set_FE_field_conditional_FE_region_data native_discretization_field_conditional_data;
			native_discretization_field_conditional_data.conditional_function =
				(LIST_CONDITIONAL_FUNCTION(FE_field) *)NULL;
			native_discretization_field_conditional_data.user_data = (void *)NULL;
			native_discretization_field_conditional_data.fe_region =
				Cmiss_region_get_FE_region(cmiss_region);
			Option_table_add_entry(option_table, "native_discretization",
				(void *)&scene->native_discretization_field,
				(void *)&native_discretization_field_conditional_data,
				set_FE_field_conditional_FE_region);
			return_code = Option_table_multi_parse(option_table, state);
			DESTROY(Option_table)(&option_table);
			if (return_code)
			{
				if ((scene->circle_discretization > 0) && (scene->circle_discretization < 12))
				{
					// silently use minimum of 12 otherwise sphere glyphs will look poor
					scene->circle_discretization = 12;
				}
				if (clear_flag)
				{
					if (group)
					{
						// remove only graphics using group as subgroup field
						Cmiss_scene_begin_change(scene);
						Cmiss_field_id group_field = Cmiss_field_group_base_cast(group);
						// support legacy command files by changing visibility of each graphic using group as its subgroup field
						Cmiss_graphic_id graphic = Cmiss_scene_get_first_graphic(scene);
						while (graphic)
						{
							Cmiss_graphic_id this_graphic = graphic;
							graphic = Cmiss_scene_get_next_graphic(scene, this_graphic);
							Cmiss_field_id subgroup_field = Cmiss_graphic_get_subgroup_field(this_graphic);
							if (subgroup_field == group_field)
							{
								Cmiss_scene_remove_graphic(scene, this_graphic);
							}
							Cmiss_field_destroy(&subgroup_field);
							Cmiss_graphic_destroy(&this_graphic);
						}
						Cmiss_scene_end_change(scene);
					}
					else
					{
						return_code = Cmiss_scene_remove_all_graphics(scene);
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
					Cmiss_scene_set_default_coordinate_field(scene, default_coordinate_field);
				}
			}
			if (default_coordinate_field)
			{
				DEACCESS(Computed_field)(&default_coordinate_field);
			}
			Cmiss_scene_destroy(&scene);
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


struct Cmiss_scene_process_temp_data
{
	 class Process_list_or_write_command_class *process_message;
	 struct Cmiss_graphic_list_data *list_data;
};

/***************************************************************************//**
 * Writes out the <graphic> as a text string in the command window with the
 * <graphic_string_detail>, <line_prefix> and <line_suffix> given in the
 * <list_data>.
 */
int Cmiss_scene_process_Cmiss_graphic_list_contents(
	 struct Cmiss_graphic *graphic,	void *process_temp_data_void)
{
	int return_code;
	char *graphic_string; /*,line[40];*/
	struct Cmiss_graphic_list_data *list_data;
	class Process_list_or_write_command_class *process_message;
	struct Cmiss_scene_process_temp_data *process_temp_data;

	ENTER(Cmiss_scene_process_Cmiss_graphic_list_contents);
	if (graphic&&
		 (process_temp_data=(struct Cmiss_scene_process_temp_data *)process_temp_data_void))
	{
		 if (NULL != (process_message = process_temp_data->process_message) &&
			 NULL != (list_data = process_temp_data->list_data))
		 {
			 if (NULL != (graphic_string=Cmiss_graphic_string(graphic,
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
				"Cmiss_scene_process_Cmiss_graphic_list_contents.  Invalid argument(s)");
		 return_code=0;
	}
	LEAVE;

	return (return_code);
}

/***************************************************************************//**
 * Will write commands to comfile or list commands to the command windows
 * depending on the class.
 */
int Cmiss_scene_process_list_or_write_window_commands(struct Cmiss_scene *scene,
	 const char *command_prefix, const char *command_suffix, class Process_list_or_write_command_class *process_message)
{
	int return_code;
	struct Cmiss_graphic_list_data list_data;

	ENTER(Cmiss_scene_process_list_or_write_window_command);
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
		struct Cmiss_scene_process_temp_data *process_temp_data;
		if (ALLOCATE(process_temp_data,struct Cmiss_scene_process_temp_data,1))
		{
			 process_temp_data->process_message = process_message;
			 process_temp_data->list_data = &list_data;
			 return_code=FOR_EACH_OBJECT_IN_LIST(Cmiss_graphic)(
					Cmiss_scene_process_Cmiss_graphic_list_contents,(void *)process_temp_data,
					scene->list_of_graphics);
			 DEALLOCATE(process_temp_data);
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_scene_list_commands.  Invalid arguments");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_scene_process_list_or_write_window_command*/

int Cmiss_scene_list_commands(struct Cmiss_scene *scene,
	 const char *command_prefix, const char *command_suffix)
{
	 int return_code = 0;

	 ENTER(Cmiss_scene_list_commands);
	 Process_list_command_class *list_message =
			new Process_list_command_class();
	 if (list_message)
	 {
			return_code = Cmiss_scene_process_list_or_write_window_commands(
				scene, command_prefix, command_suffix, list_message);
			delete list_message;
	 }
	 LEAVE;

	 return (return_code);
}

int Cmiss_scene_list_contents(struct Cmiss_scene *scene)
{
	char *name = 0;
	int return_code;
	struct Cmiss_graphic_list_data list_data;

	ENTER(Cmiss_scene_list_contents);
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
		if (scene->native_discretization_field)
		{
			if (GET_NAME(FE_field)(scene->native_discretization_field,
				&name))
			{
				display_message(INFORMATION_MESSAGE,"  Legacy native discretization field: %s\n",name);
				DEALLOCATE(name);
			}
		}
		if (0 < NUMBER_IN_LIST(Cmiss_graphic)(
			scene->list_of_graphics))
		{
			display_message(INFORMATION_MESSAGE,"  graphics objects defined:\n");
			list_data.graphic_string_detail=GRAPHIC_STRING_COMPLETE_PLUS;
			list_data.line_prefix="  ";
			list_data.line_suffix="";
			return_code=FOR_EACH_OBJECT_IN_LIST(Cmiss_graphic)(
				Cmiss_graphic_list_contents,(void *)&list_data,
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
			"Cmiss_scene_list_contents.  Invalid arguments");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_scene_list_contents */

int Cmiss_scene_fill_scene_command_data(Cmiss_scene_id scene,
	struct Scene_command_data *scene_command_data)
{
	int return_code = 0;
	if (scene)
	{
		scene_command_data->graphics_module = scene->graphics_module;
		scene_command_data->scene = scene;
		scene_command_data->material_module =
			Cmiss_graphics_module_get_material_module(scene->graphics_module);
		scene_command_data->default_material =
			Cmiss_graphics_material_module_get_default_material(scene_command_data->material_module);
		scene_command_data->default_font =
			Cmiss_graphics_module_get_default_font(scene->graphics_module);
		scene_command_data->spectrum_manager =
			Cmiss_graphics_module_get_spectrum_manager(scene->graphics_module);
		Cmiss_spectrum_module_id spectrum_module =
			Cmiss_graphics_module_get_spectrum_module(scene->graphics_module);
		scene_command_data->default_spectrum =
				Cmiss_spectrum_module_get_default_spectrum(spectrum_module);
		Cmiss_spectrum_module_destroy(&spectrum_module);
		scene_command_data->glyph_module =
			Cmiss_graphics_module_get_glyph_module(scene->graphics_module);
		scene_command_data->computed_field_manager =
			 Cmiss_region_get_Computed_field_manager(scene->region);
		scene_command_data->region = scene->region;
		scene_command_data->root_region = Cmiss_region_get_root(scene->region);
		scene_command_data->tessellation_module =
			Cmiss_graphics_module_get_tessellation_module(scene->graphics_module);
		return_code = 1;
	}
	return return_code;
}

int Cmiss_scene_cleanup_scene_command_data(
	struct Scene_command_data *scene_command_data)
{
	int return_code = 0;
	if (scene_command_data)
	{
		Cmiss_graphics_material_module_destroy(&(scene_command_data->material_module));
		Cmiss_graphics_material_destroy(&scene_command_data->default_material);
		Cmiss_font_destroy(&scene_command_data->default_font);
		Cmiss_spectrum_destroy(&scene_command_data->default_spectrum);
		Cmiss_glyph_module_destroy(&(scene_command_data->glyph_module));
		Cmiss_tessellation_module_destroy(&(scene_command_data->tessellation_module));
		Cmiss_region_destroy(&(scene_command_data->root_region));
		return_code = 1;
	}
	return return_code;
}

int Cmiss_scene_execute_command_internal(Cmiss_scene_id scene,
	Cmiss_field_group_id group, struct Parse_state *state)
{
	int return_code = 0;
	if (scene && state)
	{
		struct Cmiss_graphics_module *graphics_module =	scene->graphics_module;
		if(graphics_module)
		{
			struct Option_table *option_table;
			struct Modify_scene_data modify_scene_data;
			modify_scene_data.delete_flag = 0;
			modify_scene_data.position = -1;
			modify_scene_data.graphic = (struct Cmiss_graphic *)NULL;
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
						Cmiss_scene_get_first_graphic_with_condition(
							scene, Cmiss_graphic_has_name, (void *)graphic_name)))
					{
						ACCESS(Cmiss_graphic)(modify_scene_data.graphic);
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
			Cmiss_scene_fill_scene_command_data(scene,
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
				return_code = Cmiss_region_modify_scene(scene->region,
					modify_scene_data.graphic,
					modify_scene_data.delete_flag,
					modify_scene_data.position);
			} /* parse error,help */
			DESTROY(Option_table)(&option_table);
			Cmiss_graphic_destroy(&modify_scene_data.graphic);
			Cmiss_scene_cleanup_scene_command_data(&scene_command_data);
		}
	}
	return return_code;
}

int Cmiss_scene_execute_command(Cmiss_scene_id scene, const char *command_string)
{
	int return_code = 0;
	if (scene && command_string)
	{
		struct Parse_state *state = create_Parse_state(command_string);
		return_code = Cmiss_scene_execute_command_internal(scene, (Cmiss_field_group_id)0, state);
		destroy_Parse_state(&state);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Cmiss_scene_transformation_commands.  Invalid argument(s)");
		return_code=0;
	}
	return return_code;
}

// GRC needed?
int Cmiss_scene_add_glyph(struct Cmiss_scene *scene,
	Cmiss_glyph *glyph, const char *cmiss_graphic_name)
{
	int return_code = 0;
	if (scene && glyph)
	{
		if (!FIRST_OBJECT_IN_LIST_THAT(Cmiss_graphic)(Cmiss_graphic_has_name,
				(void *)cmiss_graphic_name, scene->list_of_graphics))
		{
			Cmiss_scene_begin_change(scene);
			Cmiss_graphic *graphic = Cmiss_graphic_points_base_cast(Cmiss_scene_create_graphic_points(scene));
			Cmiss_graphic_set_name(graphic, cmiss_graphic_name);
			Cmiss_graphic_point_attributes_id point_attributes = Cmiss_graphic_get_point_attributes(graphic);
			Cmiss_graphic_point_attributes_set_glyph(point_attributes, reinterpret_cast<Cmiss_glyph_id>(glyph));
			const double one = 1.0;
			Cmiss_graphic_point_attributes_set_base_size(point_attributes, 1, &one);
			Cmiss_graphic_point_attributes_destroy(&point_attributes);
			Cmiss_graphic_destroy(&graphic);
			Cmiss_scene_end_change(scene);
			return_code = 1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_scene_add_glyph.  Graphic with the same name already exists");
		}
	}
	return return_code;
}

Cmiss_graphic* Cmiss_scene_create_graphic_app(Cmiss_scene *scene,
	Cmiss_graphic_type graphic_type, Cmiss_graphic *graphic_to_copy)
{
	Cmiss_graphic *graphic = CREATE(Cmiss_graphic)(graphic_type);
	if (graphic_to_copy &&
		(graphic_type == Cmiss_graphic_get_graphic_type(graphic_to_copy)))
	{
		Cmiss_graphic_copy_without_graphics_object(graphic, graphic_to_copy);
	}
	else
	{
		Cmiss_scene_set_minimum_graphic_defaults(scene, graphic);
		Cmiss_graphic_line_attributes_id line_attributes = Cmiss_graphic_get_line_attributes(graphic);
		Cmiss_graphic_point_attributes_id point_attributes = Cmiss_graphic_get_point_attributes(graphic);
		if (graphic_type == CMISS_GRAPHIC_STREAMLINES)
		{
			// use previous default of 1.0 for streamline width
			const double one = 1.0;
			Cmiss_graphic_line_attributes_set_base_size(line_attributes, 1, &one);
		}
		if (point_attributes)
		{
			// use previous default of 1.0 for base size
			const double one = 1.0;
			Cmiss_graphic_point_attributes_set_base_size(point_attributes, 1, &one);
		}
		Cmiss_graphic_line_attributes_destroy(&line_attributes);
		Cmiss_graphic_point_attributes_destroy(&point_attributes);
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
	struct Cmiss_region *root_region;
	Cmiss_scene **scene_address = 0;

	if (state && (root_region = static_cast<struct Cmiss_region *>(root_region_void)) &&
		(scene_address = static_cast<struct Cmiss_scene **>(scene_address_void)))
	{
		if ((current_token = state->current_token))
		{
			if (!Parse_state_help_mode(state))
			{
				Cmiss_region *scene_region = Cmiss_region_access(root_region);
				if ((0 != strcmp(current_token, "default")))
				{
					display_message(INFORMATION_MESSAGE,
						"set_Scene:  scene is now a graphical representation of a region,"
						"specified scene will be interpret as region path");
					return_code = set_Cmiss_region(state, &scene_region, root_region);
				}
				else
				{
					return_code = shift_Parse_state(state, 1);
				}
				if (return_code && scene_region)
				{
					Cmiss_scene_destroy(scene_address);
					*scene_address = Cmiss_region_get_scene_internal(scene_region);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"set_Scene:  Could not find scene %s", current_token);
					display_parse_state_location(state);
					return_code = 0;
				}
				Cmiss_region_destroy(&scene_region);
			}
			else
			{
				display_message(INFORMATION_MESSAGE," PATH_TO_SCENE");
				if (*scene_address)
				{
					struct Cmiss_region *region = Cmiss_scene_get_region(*scene_address);
					char *path = Cmiss_region_get_path(region);
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

