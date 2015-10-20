/* OpenCMISS-Cmgui Application
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#if 1
#include "configure/cmgui_configure.h"
#endif /* defined (BUILD_WITH_CMAKE) */




#include "general/enumerator_private.h"
#include "general/message.h"
#include "general/enumerator_private.hpp"
#include "general/message.h"
#include "general/debug.h"
#include "command/parser.h"
#include "graphics/light.hpp"
#include "graphics/light_app.h"
#include "graphics/colour_app.h"

int modify_cmzn_light(struct Parse_state *state,void *light_void,
	void *modify_light_data_void)
/*******************************************************************************
LAST MODIFIED : 9 October 2002

DESCRIPTION :
==============================================================================*/
{
	const char *current_token, *light_type_string, **valid_strings;
	enum cmzn_light_type light_type;
	double constant_attenuation, direction[3], linear_attenuation, position[3],
		quadratic_attenuation, spot_cutoff, spot_exponent, light_colour[3];
	int num_floats, number_of_valid_strings, process, return_code;
	struct cmzn_light *light_to_be_modified;
	struct Modify_light_data *modify_light_data;
	struct Option_table *option_table;
	char infinite_viewer_flag=0,local_viewer_flag=0,single_sided_flag=0,
		double_sided_flag=0,disable_flag=0,enable_flag=0;

	modify_light_data=(struct Modify_light_data *)modify_light_data_void;
	if (state && modify_light_data)
	{
		current_token=state->current_token;
		if (current_token)
		{
			cmzn_lightmodule_begin_change(modify_light_data->lightmodule);
			process=0;
			light_to_be_modified=(struct cmzn_light *)light_void;
			if (light_to_be_modified)
			{
				light_to_be_modified = cmzn_light_access(light_to_be_modified);
				process=1;
			}
			else
			{
				if (strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
				{
					light_to_be_modified = cmzn_lightmodule_find_light_by_name(modify_light_data->lightmodule, current_token);
					if (light_to_be_modified)
					{
						return_code=shift_Parse_state(state,1);
						if (return_code)
						{
							process=1;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"Unknown light : %s",current_token);
						display_parse_state_location(state);
						return_code=0;
					}
				}
				else
				{
					light_to_be_modified=cmzn_light_create_private();
					if (light_to_be_modified)
					{
						option_table=CREATE(Option_table)();
						Option_table_add_entry(option_table, "LIGHT_NAME",
							light_to_be_modified, modify_light_data_void, modify_cmzn_light);
						return_code = Option_table_parse(option_table, state);
						DESTROY(Option_table)(&option_table);
						cmzn_light_destroy(&light_to_be_modified);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"modify_cmzn_light.  Could not create dummy light");
						return_code=0;
					}
				}
			}
			if (process)
			{
				num_floats=3;
				light_type = cmzn_light_get_type(light_to_be_modified);
				cmzn_light_get_colour(light_to_be_modified, &light_colour[0]);
				cmzn_light_get_direction(light_to_be_modified, &direction[0]);
				cmzn_light_get_position(light_to_be_modified, &position[0]);
				constant_attenuation = cmzn_light_get_constant_attenuation(light_to_be_modified);
				linear_attenuation = cmzn_light_get_linear_attenuation(light_to_be_modified);
				quadratic_attenuation = cmzn_light_get_quadratic_attenuation(light_to_be_modified);
				spot_cutoff = cmzn_light_get_spot_cutoff(light_to_be_modified);
				spot_exponent = cmzn_light_get_spot_exponent(light_to_be_modified);
				infinite_viewer_flag=0,local_viewer_flag=0,single_sided_flag=0,
						double_sided_flag=0,disable_flag=0,enable_flag=0;

				option_table = CREATE(Option_table)();
				/* colour */
				Option_table_add_entry(option_table, "colour",
					&light_colour, NULL, set_Colour);
				/* constant_attenuation */
				Option_table_add_entry(option_table, "constant_attenuation",
					&constant_attenuation, NULL, set_double_non_negative);
				/* cutoff */
				Option_table_add_entry(option_table, "cut_off",
					&spot_cutoff, NULL, set_float);
				/* direction */
				Option_table_add_entry(option_table, "direction",
					direction, &num_floats, set_double_vector);
				/* exponent */
				Option_table_add_entry(option_table, "exponent",
					&spot_exponent, NULL, set_double_non_negative);
				/* light_type: infinite/point/spot */
				light_type_string = ENUMERATOR_STRING(cmzn_light_type)(light_type);
				valid_strings = ENUMERATOR_GET_VALID_STRINGS(cmzn_light_type)(
					&number_of_valid_strings,
					(ENUMERATOR_CONDITIONAL_FUNCTION(cmzn_light_type) *)NULL,
					(void *)NULL);
				Option_table_add_enumerator(option_table, number_of_valid_strings,
					valid_strings, &light_type_string);
				DEALLOCATE(valid_strings);
				/* linear_attenuation */
				Option_table_add_entry(option_table, "linear_attenuation",
					&linear_attenuation, NULL, set_double_non_negative);
				/* position */
				Option_table_add_entry(option_table, "position",
					position, &num_floats, set_double_vector);
				/* quadratic_attenuation */
				Option_table_add_entry(option_table, "quadratic_attenuation",
					&quadratic_attenuation, NULL, set_double_non_negative);
				Option_table_add_entry(option_table,"single_sided",
					&single_sided_flag,(void *)NULL,set_char_flag);
				Option_table_add_entry(option_table,"double_sided",
					&double_sided_flag,(void *)NULL,set_char_flag);
				Option_table_add_entry(option_table,"infinite_viewer",
					&infinite_viewer_flag,(void *)NULL,set_char_flag);
				Option_table_add_entry(option_table,"local_viewer",
					&local_viewer_flag,(void *)NULL,set_char_flag);
				Option_table_add_entry(option_table,"enable",
					&enable_flag,(void *)NULL,set_char_flag);
				Option_table_add_entry(option_table,"disable",
					&disable_flag,(void *)NULL,set_char_flag);

				return_code = Option_table_multi_parse(option_table, state);
				if (return_code)
				{
					if (disable_flag&&enable_flag)
					{
						display_message(ERROR_MESSAGE,"modify_cmzn_light_model.  "
							"Only one of disable/enable");
						return_code=0;
					}
					if (single_sided_flag&&double_sided_flag)
					{
						display_message(ERROR_MESSAGE,"modify_cmzn_light_model.  "
							"Only one of single_sided/doule_sided");
						return_code=0;
					}
					if (infinite_viewer_flag&&local_viewer_flag)
					{
						display_message(ERROR_MESSAGE,"modify_cmzn_light_model.  "
							"Only one of infinite_viewer/local_viewer");
						return_code=0;
					}
					if ((0.0 > spot_cutoff) || ((90.0 < spot_cutoff)))
					{
						display_message(WARNING_MESSAGE,
							"Spotlight cut-off angle must be from 0 to 90 degrees");
						if (0.0 > spot_cutoff)
						{
							spot_cutoff = 0.0;
						}
						else
						{
							spot_cutoff = 90.0;
						}
					}
					if (return_code)
					{
						STRING_TO_ENUMERATOR(cmzn_light_type)(light_type_string, &light_type);
						cmzn_light_set_type(light_to_be_modified, light_type);
						cmzn_light_set_colour(light_to_be_modified, &light_colour[0]);
						cmzn_light_set_direction(light_to_be_modified, &direction[0]);
						cmzn_light_set_position(light_to_be_modified, &position[0]);
						cmzn_light_set_constant_attenuation(light_to_be_modified, constant_attenuation);
						cmzn_light_set_linear_attenuation(light_to_be_modified, linear_attenuation);
						cmzn_light_set_quadratic_attenuation(light_to_be_modified, quadratic_attenuation);
						cmzn_light_set_spot_cutoff(light_to_be_modified, spot_cutoff);
						cmzn_light_set_spot_exponent(light_to_be_modified, spot_exponent);
						if (local_viewer_flag)
						{
							cmzn_light_set_render_viewer_mode(light_to_be_modified,
								CMZN_LIGHT_RENDER_VIEWER_MODE_LOCAL);
						}
						if (infinite_viewer_flag)
						{
							cmzn_light_set_render_viewer_mode(light_to_be_modified,
								CMZN_LIGHT_RENDER_VIEWER_MODE_INFINITE);
						}
						if (single_sided_flag)
						{
							cmzn_light_set_render_side(light_to_be_modified,
								CMZN_LIGHT_RENDER_SIDE_SINGLE);
						}
						if (double_sided_flag)
						{
							cmzn_light_set_render_side(light_to_be_modified,
								CMZN_LIGHT_RENDER_SIDE_DOUBLE);
						}
						if (enable_flag)
						{
							cmzn_light_set_enabled(light_to_be_modified, 1);
						}
						if (disable_flag)
						{
							cmzn_light_set_enabled(light_to_be_modified, 0);
						}
					}
				}
				DESTROY(Option_table)(&option_table);
				if (light_to_be_modified)
				{
					cmzn_light_destroy(&light_to_be_modified);
				}
			}
			cmzn_lightmodule_end_change(modify_light_data->lightmodule);
		}
		else
		{
			if (light_void)
			{
				display_message(WARNING_MESSAGE, "Missing light modifications");
			}
			else
			{
				display_message(WARNING_MESSAGE, "Missing light name");
			}
			display_parse_state_location(state);
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"modify_cmzn_light.  Invalid argument(s)");
		return_code=0;
	}

	return (return_code);
} /* modify_cmzn_light */

int set_cmzn_light(struct Parse_state *state,
	void *light_address_void,void *light_manager_void)
/*******************************************************************************
LAST MODIFIED : 22 June 1999

DESCRIPTION :
Modifier function to set the light from a command.
???RC set_Object routines could become a macro.
==============================================================================*/
{
	const char *current_token;
	int return_code;
	struct cmzn_light *temp_light,**light_address;
	struct MANAGER(cmzn_light) *light_manager;

	if (state)
	{
		current_token=state->current_token;
		if (current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				if ((light_address=
					(struct cmzn_light **)light_address_void)&&
					(light_manager=(struct MANAGER(cmzn_light) *)
					light_manager_void))
				{
					if (fuzzy_string_compare(current_token,"NONE"))
					{
						if (*light_address)
						{
							cmzn_light_destroy(light_address);
							*light_address=(struct cmzn_light *)NULL;
						}
						return_code=1;
					}
					else
					{
						temp_light=FIND_BY_IDENTIFIER_IN_MANAGER(cmzn_light,
							name)(current_token,light_manager);
						if (temp_light)
						{
							if (*light_address!=temp_light)
							{
								cmzn_light_destroy(light_address);
								*light_address=cmzn_light_access(temp_light);
							}
							return_code=1;
						}
						else
						{
							display_message(ERROR_MESSAGE,"Unknown light : %s",
								current_token);
							return_code=0;
						}
					}
					shift_Parse_state(state,1);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"set_cmzn_light.  Invalid argument(s)");
					return_code=0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE," LIGHT_NAME|none");
				/* if possible, then write the name */
				light_address=(struct cmzn_light **)light_address_void;
				if (light_address)
				{
					temp_light= *light_address;
					if (temp_light)
					{
						display_message(INFORMATION_MESSAGE,"[%s]",get_cmzn_light_name(temp_light));
					}
					else
					{
						display_message(INFORMATION_MESSAGE,"[none]");
					}
				}
				return_code=1;
			}
		}
		else
		{
			display_message(WARNING_MESSAGE,"Missing light name");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_cmzn_light.  Missing state");
		return_code=0;
	}

	return (return_code);
} /* set_cmzn_light */
