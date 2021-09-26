/*******************************************************************************
FILE : computed_field_scene_viewer_projection.c

LAST MODIFIED : 29 August 2011

DESCRIPTION :
Implements a computed_field which maintains a graphics transformation
equivalent to the scene_viewer assigned to it.
==============================================================================*/
/* OpenCMISS-Cmgui Application
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#include <cmath>
#include <string>

#include "opencmiss/zinc/fieldmodule.h"
#include "opencmiss/zinc/fieldsceneviewerprojection.h"
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_app.h"
#include "computed_field/computed_field_private.hpp"
#include "computed_field/computed_field_image.h"
#include "computed_field/computed_field_set.h"
#include "general/debug.h"
#include "general/matrix_vector.h"
#include "general/mystring.h"
#include "graphics/graphics_window.h"
#include "graphics/scene.hpp"
#include "graphics/scene_coordinate_system.hpp"
#include "graphics/scene_viewer.h"
#include "general/message.h"
#include "computed_field/computed_field_scene_viewer_projection.h"
#include "command/parser.h"
#include "graphics/scene_viewer_app.h"
// insert app headers here
#include "computed_field/computed_field_private_app.hpp"


int cmzn_field_projection_set_window_name(struct Computed_field *field, const char *graphics_window_name);

int cmzn_field_projection_set_pane_number(struct Computed_field *field, int pane_number);

class Computed_field_scene_viewer_projection_package : public Computed_field_type_package
{
public:
	struct MANAGER(Graphics_window) *graphics_window_manager;
};

const char computed_field_scene_viewer_projection_type_string[] = "window_projection";

int Computed_field_get_type_scene_viewer_projection(struct Computed_field *field,
	struct Scene_viewer **scene_viewer, char **graphics_window_name, int *pane_number,
	enum cmzn_scenecoordinatesystem *from_coordinate_system,
	enum cmzn_scenecoordinatesystem *to_coordinate_system);

/* For gfx command */
cmzn_field *cmzn_fieldmodule_create_field_scene_viewer_projection_with_window_name(
	struct cmzn_fieldmodule *field_module, struct Scene_viewer *scene_viewer,
	const char *graphics_window_name, int pane_number,
	enum cmzn_scenecoordinatesystem from_coordinate_system,
	enum cmzn_scenecoordinatesystem to_coordinate_system)
{
	Computed_field *field = NULL;
	if (scene_viewer)
	{
		field = cmzn_fieldmodule_create_field_sceneviewer_projection(field_module, scene_viewer, from_coordinate_system, to_coordinate_system);
		cmzn_field_projection_set_window_name(field, graphics_window_name);
		cmzn_field_projection_set_pane_number(field, pane_number);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmzn_fieldmodule_create_field_scene_viewer_projection_with_window_name.  Invalid argument(s)");
	}

	return (field);
}

int define_Computed_field_type_scene_viewer_projection(struct Parse_state *state,
	void *field_modify_void, void *computed_field_scene_viewer_projection_package_void)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_scene_viewer_projection (if it is not
already) and allows its contents to be modified.
==============================================================================*/
{
	char *graphics_window_name;
	int pane_number, return_code;
	Computed_field_scene_viewer_projection_package *computed_field_scene_viewer_projection_package =
		static_cast<Computed_field_scene_viewer_projection_package *>(computed_field_scene_viewer_projection_package_void);
	Computed_field_modify_data *field_modify = static_cast<Computed_field_modify_data *>(field_modify_void);
	struct Graphics_window *graphics_window;
	struct Option_table *option_table;
	struct Scene_viewer_app *scene_viewer = 0;

	ENTER(define_Computed_field_type_scene_viewer_projection);
	if ((state) && (field_modify) && (computed_field_scene_viewer_projection_package))
	{
		return_code=1;
		/* get valid parameters for projection field */
		pane_number = 1;
		graphics_window = (struct Graphics_window *)NULL;
		cmzn_scenecoordinatesystem from_coordinate_system = CMZN_SCENECOORDINATESYSTEM_INVALID;
		cmzn_scenecoordinatesystem to_coordinate_system = CMZN_SCENECOORDINATESYSTEM_INVALID;
		if ((NULL != field_modify->get_field()) &&
			(computed_field_scene_viewer_projection_type_string == Computed_field_get_type_string(field_modify->get_field())))
		{
			return_code=Computed_field_get_type_scene_viewer_projection(field_modify->get_field(),
				&(scene_viewer->core_scene_viewer), &graphics_window_name, &pane_number,
				&from_coordinate_system, &to_coordinate_system);
			pane_number++;
			if (graphics_window_name)
			{
				graphics_window = FIND_BY_IDENTIFIER_IN_MANAGER(Graphics_window,name)(graphics_window_name,
					computed_field_scene_viewer_projection_package->graphics_window_manager);
				DEALLOCATE(graphics_window_name);
			}
		}
		if (return_code)
		{
			/* must access objects for set functions */
			if (graphics_window)
			{
				ACCESS(Graphics_window)(graphics_window);
			}
			char *from_coordinate_system_string = 0;
			char *to_coordinate_system_string = 0;
			int number_of_valid_strings = 0;
			const char **valid_strings = ENUMERATOR_GET_VALID_STRINGS(cmzn_scenecoordinatesystem)(
				&number_of_valid_strings,
				(ENUMERATOR_CONDITIONAL_FUNCTION(cmzn_scenecoordinatesystem) *)NULL,
				(void *)NULL);
			std::string all_coordinate_systems = " ";
			for (int i = 0; i < number_of_valid_strings; i++)
			{
				if (i)
					all_coordinate_systems += "|";

				all_coordinate_systems += valid_strings[i];
			}
			const char *all_coordinate_systems_help = all_coordinate_systems.c_str();

			option_table = CREATE(Option_table)();
			Option_table_add_help(option_table,
				"A 16 component computed field which continuously update the transformation between "
				"'from' and 'to' graphics_coordinate systems in the give pane of window.");
			/* pane_number */
			Option_table_add_entry(option_table,"pane_number",&pane_number,
				NULL,set_int_positive);
			/* window */
			Option_table_add_entry(option_table,"window",&graphics_window,
				computed_field_scene_viewer_projection_package->graphics_window_manager,
				set_Graphics_window);
			Option_table_add_string_entry(option_table, "from_coordinate_system",
				&from_coordinate_system_string, all_coordinate_systems_help);
			Option_table_add_string_entry(option_table, "to_coordinate_system",
				&to_coordinate_system_string, all_coordinate_systems_help);
			return_code=Option_table_multi_parse(option_table,state);
			DEALLOCATE(valid_strings);
			/* no errors,not asking for help */
			if (return_code)
			{
				if (from_coordinate_system_string)
				{
					STRING_TO_ENUMERATOR(cmzn_scenecoordinatesystem)(from_coordinate_system_string,
						&from_coordinate_system);
					if (CMZN_SCENECOORDINATESYSTEM_INVALID == from_coordinate_system)
					{
						display_message(ERROR_MESSAGE,
							"gfx define field ~ window_projection:  Invalid coordinate system %s", from_coordinate_system_string);
						return_code = 0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"gfx define field ~ window_projection:  Missing from_coordinate_system argument");
					return_code = 0;
				}
				if (to_coordinate_system_string)
				{
					STRING_TO_ENUMERATOR(cmzn_scenecoordinatesystem)(to_coordinate_system_string,
						&to_coordinate_system);
					if (CMZN_SCENECOORDINATESYSTEM_INVALID == to_coordinate_system)
					{
						display_message(ERROR_MESSAGE,
							"gfx define field ~ window_projection:  Invalid coordinate system %s", to_coordinate_system_string);
						return_code = 0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"gfx define field ~ window_projection:  Missing to_coordinate_system argument");
					return_code = 0;
				}
			}
			if (return_code)
			{
				if (!(scene_viewer = Graphics_window_get_Scene_viewer(graphics_window,
					pane_number - 1)))
				{
					return_code = 0;
				}
			}
			if (return_code)
			{
				GET_NAME(Graphics_window)(graphics_window, &graphics_window_name);
				return_code = field_modify->define_field(
					cmzn_fieldmodule_create_field_scene_viewer_projection_with_window_name(
						field_modify->get_field_module(),
						scene_viewer->core_scene_viewer, graphics_window_name, pane_number - 1,
						from_coordinate_system, to_coordinate_system));
				DEALLOCATE(graphics_window_name);
			}
			if (!return_code)
			{
				if ((!state->current_token)||
					(strcmp(PARSER_HELP_STRING,state->current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
				{
					/* error */
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_scene_viewer_projection.  Failed");
				}
			}
			DEALLOCATE(from_coordinate_system_string);
			DEALLOCATE(to_coordinate_system_string);
			if (graphics_window)
			{
				DEACCESS(Graphics_window)(&graphics_window);
			}
			DESTROY(Option_table)(&option_table);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_scene_viewer_projection.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_scene_viewer_projection */

int Computed_field_register_types_sceneviewer_projection(
	struct Computed_field_package *computed_field_package,
	struct MANAGER(Graphics_window) *graphics_window_manager)
{
	int return_code;
	Computed_field_scene_viewer_projection_package
		*computed_field_scene_viewer_projection_package =
		new Computed_field_scene_viewer_projection_package;
	if (computed_field_package && graphics_window_manager)
	{
		computed_field_scene_viewer_projection_package->graphics_window_manager =
			graphics_window_manager;
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_scene_viewer_projection_type_string,
			define_Computed_field_type_scene_viewer_projection,
			computed_field_scene_viewer_projection_package);
	}
	else
	{
		return_code=0;
	}
	return (return_code);
}

