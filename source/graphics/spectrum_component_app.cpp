/* OpenCMISS-Cmgui Application
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#include <math.h>
#include "general/mystring.h"

#include "zinc/zincconfigure.h"
#if 1
#include "configure/cmgui_configure.h"
#endif /* defined (BUILD_WITH_CMAKE) */

#include "computed_field/computed_field.h"
#include "general/enumerator.h"
#include "general/enumerator_private.h"
//#include "general/enumerator_private.hpp"
#include "general/message.h"
#include "general/debug.h"
#include "command/parser.h"
#include "graphics/spectrum.h"
#include "graphics/spectrum_component.h"
#include "graphics/spectrum_component_app.h"
#include "computed_field/computed_field_set.h"
#include "computed_field/computed_field_set_app.h"
#include "general/enumerator_private_app.h"

DEFINE_DEFAULT_OPTION_TABLE_ADD_ENUMERATOR_FUNCTION(cmzn_spectrumcomponent_colour_mapping_type);

/**
 * Executes a GFX MODIFY SPECTRUM LINEAR|LOG command.
 * @param isLog  True if log scale, otherwise linear scale.
 * If return_code is 1, returns the completed Modify_spectrum_app_data with the
 * parsed settings. Note that the settings are ACCESSed once on valid return.
 */
static int gfx_modify_spectrum_linear_log(struct Parse_state *state,
  Modify_spectrum_app_data *modify_spectrum_data, bool isLogScale)
{
	int return_code;
	if (state)
	{
		if (modify_spectrum_data)
		{
			/* create the spectrum_settings: */
			cmzn_spectrumcomponent *settings = modify_spectrum_data->component = CREATE(cmzn_spectrumcomponent)();
			if (settings)
			{
				// Zinc API spectrum components extend below and above by default, but for
				// backward compatibility with old commands this must be reversed
				cmzn_spectrumcomponent_set_extend_below(settings, false);
				cmzn_spectrumcomponent_set_extend_above(settings, false);

				cmzn_spectrumcomponent_set_scale_type(settings,
					isLogScale ? CMZN_SPECTRUMCOMPONENT_SCALE_TYPE_LOG : CMZN_SPECTRUMCOMPONENT_SCALE_TYPE_LINEAR);
				settings->is_field_lookup = false;
				cmzn_spectrumcomponent_colour_mapping_type colour_mapping_type =
					cmzn_spectrumcomponent_get_colour_mapping_type(settings);
				char ambient = 0;
				char amb_diff = 0;
				double band_ratio = 0.01;
				char diffuse = 0;
				char emission = 0;
				char extend_above = 0;
				char extend_below = 0;
				char fix_maximum = 0;
				char fix_minimum = 0;
				char left = 0;
				int number_of_bands = 10;
				char reverse = 0;
				char right = 0;
				char specular = 0;
				double step_value = 0.5;
				char transparent_above = 0;
				char transparent_below = 0;
				int range_components = 2;
				FE_value colour_range[2] = { 0.0, 1.0 };
				FE_value range[2] = {
					(FE_value)(modify_spectrum_data->spectrum_minimum),
					(FE_value)(modify_spectrum_data->spectrum_maximum)
				};
				int field_component = cmzn_spectrumcomponent_get_field_component(settings);
				double exaggeration = (isLogScale) ? cmzn_spectrumcomponent_get_exaggeration(settings) : 0.0;

				Option_table *option_table = CREATE(Option_table)();
				/* band_ratio */
				Option_table_add_double_entry(option_table, "band_ratio",
					&band_ratio);
				/* colour_range */
				Option_table_add_FE_value_vector_entry(option_table, "colour_range",
					colour_range, &range_components);
				/* component */
				Option_table_add_int_positive_entry(option_table, "component",
					&field_component);
				if (isLogScale)
				{
					/* exaggeration */
					Option_table_add_double_entry(option_table, "exaggeration", &exaggeration);
				}
				/* extend_above */
				Option_table_add_char_flag_entry(option_table, "extend_above",
					&extend_above);
				/* extend_below */
				Option_table_add_char_flag_entry(option_table, "extend_below",
					&extend_below);
				/* fix_maximum */
				Option_table_add_char_flag_entry(option_table, "fix_maximum",
					&fix_maximum);
				/* fix_minimum */
				Option_table_add_char_flag_entry(option_table, "fix_minimum",
					&fix_minimum);
				if (isLogScale)
				{
					/* left */
					Option_table_add_char_flag_entry(option_table, "left", &left);
				}
				/* number_of_bands */
				Option_table_add_int_positive_entry(option_table, "number_of_bands",
					&number_of_bands);
				/* position */
				Option_table_add_int_non_negative_entry(option_table, "position",
					&(modify_spectrum_data->position));
				/* range */
				Option_table_add_FE_value_vector_entry(option_table, "range",
					range, &range_components);
				/* reverse */
				Option_table_add_char_flag_entry(option_table, "reverse",
					&reverse);
				if (isLogScale)
				{
					/* right */
					Option_table_add_char_flag_entry(option_table, "right", &right);
				}
				/* step_value */
				Option_table_add_double_entry(option_table, "step_value",
					&step_value);
				/* transparent_above */
				Option_table_add_char_flag_entry(option_table, "transparent_above",
					&transparent_above);
				/* transparent_below */
				Option_table_add_char_flag_entry(option_table, "transparent_below",
					&transparent_below);
				/* conversion_mode */
				OPTION_TABLE_ADD_ENUMERATOR(cmzn_spectrumcomponent_colour_mapping_type)(
					option_table, &colour_mapping_type);
				/* render_option */
				Option_table *render_option_table = CREATE(Option_table)();
				Option_table_add_char_flag_entry(render_option_table, "ambient",
					&ambient);
				Option_table_add_char_flag_entry(render_option_table, "amb_diff",
					&amb_diff);
				Option_table_add_char_flag_entry(render_option_table, "diffuse",
					&diffuse);
				Option_table_add_char_flag_entry(render_option_table, "emission",
					&emission);
				Option_table_add_char_flag_entry(render_option_table, "specular",
					&specular);
				Option_table_add_suboption_table(option_table, render_option_table);

				if (!(return_code=Option_table_multi_parse(option_table,state)))
				{
					DEACCESS(cmzn_spectrumcomponent)(&(modify_spectrum_data->component));
				}
				DESTROY(Option_table)(&option_table);
				if (return_code)
				{
					cmzn_spectrumcomponent_set_colour_mapping_type(settings,
						colour_mapping_type);
				}
				if ( return_code )
				{
					if (specular + emission > 1)
					{
						display_message(ERROR_MESSAGE,"gfx modify spectrum linear|log.  "
							"All spectrums are ambient diffuse.  The specular and emission flags are ignored.");
						return_code=0;
					}
				}
				if ( return_code )
				{
					if ( extend_above && transparent_above)
					{
						display_message(ERROR_MESSAGE,"gfx modify spectrum linear|log.  "
							"Specify only one of extend_above and transparent_above");
						return_code=0;
					}
					else if (extend_above)
					{
						cmzn_spectrumcomponent_set_extend_above(settings, true);
					}
					else if (transparent_above)
					{
						cmzn_spectrumcomponent_set_extend_above(settings, false);
					}
				}
				if ( return_code )
				{
					if ( extend_below && transparent_below)
					{
						display_message(ERROR_MESSAGE,"gfx modify spectrum linear|log.  "
							"Specify only one of extend_below and transparent_below");
						return_code=0;
					}
					else if (extend_below)
					{
						cmzn_spectrumcomponent_set_extend_below(settings, true);
					}
					else if (transparent_below)
					{
						cmzn_spectrumcomponent_set_extend_below(settings, false);
					}
				}
				if ( return_code && isLogScale )
				{
					if (left && right)
					{
						display_message(ERROR_MESSAGE,"gfx_modify_spectrum_settings_log.  "
							"Specify only one of left or right");
						return_code=0;
					}
					else if (left)
					{
						exaggeration = fabs(exaggeration);
					}
					else if (right)
					{
						exaggeration = -fabs(exaggeration);
					}
					cmzn_spectrumcomponent_set_exaggeration(settings, exaggeration);
				}
				if ( return_code )
				{
					cmzn_spectrumcomponent_set_field_component(settings, field_component);
					cmzn_spectrumcomponent_set_colour_minimum(settings,	colour_range[0]);
					cmzn_spectrumcomponent_set_colour_maximum(settings,	colour_range[1]);
					cmzn_spectrumcomponent_set_range_minimum(settings,	range[0]);
					cmzn_spectrumcomponent_set_range_maximum(settings,	range[1]);
					cmzn_spectrumcomponent_set_colour_reverse(settings, 0 != reverse);
					cmzn_spectrumcomponent_set_number_of_bands(settings,
						number_of_bands);
					int black_band_int = static_cast<int>(band_ratio * 1000.0 + 0.5);
					cmzn_spectrumcomponent_set_black_band_proportion(settings,
						black_band_int);
					/* Must set step value after setting minimum and maximum range */
					cmzn_spectrumcomponent_set_step_value(settings, step_value);
				}
				/* Must set fix_maximum,fix_minimum after setting minimum and maximum range */
				if ( return_code )
				{
					if (fix_maximum)
					{
						cmzn_spectrumcomponent_set_fix_maximum_flag(settings, 1);
					}
				}
				if ( return_code )
				{
					if (fix_minimum)
					{
						cmzn_spectrumcomponent_set_fix_minimum_flag(settings, 1);
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_modify_spectrum_linear_log.  Could not create settings");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_modify_spectrum_linear_log.  No modify data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE, "gfx_modify_spectrum_linear_log.  Missing state");
		return_code=0;
	}
	return (return_code);
}

int gfx_modify_spectrum_settings_linear(struct Parse_state *state,
	void *modify_spectrum_data_void, void *dummy)
{
	USE_PARAMETER(dummy);
	return gfx_modify_spectrum_linear_log(state,
		static_cast<Modify_spectrum_app_data *>(modify_spectrum_data_void), /*isLogScale*/false);
}

int gfx_modify_spectrum_settings_log(struct Parse_state *state,
	void *modify_spectrum_data_void, void *dummy)
{
	USE_PARAMETER(dummy);
	return gfx_modify_spectrum_linear_log(state,
		static_cast<Modify_spectrum_app_data *>(modify_spectrum_data_void), /*isLogScale*/true);
}

int gfx_modify_spectrum_settings_field(struct Parse_state *state,
	void *modify_spectrum_data_void,void *dummy)
{
	enum cmzn_spectrumcomponent_colour_mapping_type colour_mapping_type;
	int component, return_code;
	struct Computed_field *input_field, *output_field;
	struct Modify_spectrum_app_data *modify_spectrum_data;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_input_field_data,
		set_output_field_data;
	struct cmzn_spectrumcomponent *settings;

	ENTER(gfx_modify_spectrum_settings_field);
	USE_PARAMETER(dummy);
	if (state)
	{
		modify_spectrum_data=(struct Modify_spectrum_app_data *)modify_spectrum_data_void;
		if (modify_spectrum_data)
		{
			/* create the spectrum_settings: */
			settings=modify_spectrum_data->component=CREATE(cmzn_spectrumcomponent)();
			if (settings)
			{
				cmzn_spectrumcomponent_set_scale_type(settings,CMZN_SPECTRUMCOMPONENT_SCALE_TYPE_INVALID);
				settings->is_field_lookup = true;

				colour_mapping_type = cmzn_spectrumcomponent_get_colour_mapping_type(settings);
				component = cmzn_spectrumcomponent_get_field_component(settings);

				input_field = (struct Computed_field *)NULL;
				output_field = (struct Computed_field *)NULL;
				if (settings->input_field)
				{
					input_field = ACCESS(Computed_field)(settings->input_field);
				}
				if (settings->output_field)
				{
					output_field = ACCESS(Computed_field)(settings->output_field);
				}

				option_table = CREATE(Option_table)();
				/* component */
				Option_table_add_int_positive_entry(option_table, "component",
					&component);
				/* input_field */
				set_input_field_data.computed_field_manager=
					modify_spectrum_data->computed_field_manager;
				set_input_field_data.conditional_function=
					Computed_field_has_numerical_components;
				set_input_field_data.conditional_function_user_data=(void *)NULL;
				Option_table_add_entry(option_table,"input",
					&input_field ,&set_input_field_data,
					set_Computed_field_conditional);
				/* output_field */
				set_output_field_data.computed_field_manager=
					modify_spectrum_data->computed_field_manager;
				set_output_field_data.conditional_function=
					Computed_field_has_numerical_components;
				set_output_field_data.conditional_function_user_data=(void *)NULL;
				Option_table_add_entry(option_table,"output",
					&output_field, &set_output_field_data,
					set_Computed_field_conditional);
				/* conversion_mode */
				Option_table_add_enumerator_cmzn_spectrumcomponent_colour_mapping_type(
					option_table, &colour_mapping_type);

				if (!(return_code=Option_table_multi_parse(option_table,state)))
				{
					DEACCESS(cmzn_spectrumcomponent)(&(modify_spectrum_data->component));
				}
				DESTROY(Option_table)(&option_table);
				if (return_code)
				{
					if (!input_field)
					{
						display_message(ERROR_MESSAGE,"gfx_modify_spectrum_settings_field.  "
							"You must specify a numerical input field.");
						return_code=0;
					}
					if (!output_field)
					{
						display_message(ERROR_MESSAGE,"gfx_modify_spectrum_settings_field.  "
							"You must specify a numerical input field.");
						return_code=0;
					}
				}
				if (return_code)
				{
					if (input_field != settings->input_field)
					{
						REACCESS(Computed_field)(&settings->input_field,
							input_field);
						settings->changed = 1;
					}
					if (output_field != settings->output_field)
					{
						REACCESS(Computed_field)(&settings->output_field,
							output_field);
						settings->changed = 1;
					}
					cmzn_spectrumcomponent_set_field_component(settings,
						component);
					cmzn_spectrumcomponent_set_colour_mapping_type(settings,
						colour_mapping_type);
				}
				DEACCESS(Computed_field)(&input_field);
				DEACCESS(Computed_field)(&output_field);
			}
			else
			{
				display_message(ERROR_MESSAGE,"gfx_modify_spectrum_settings_field.  "
					"Could not create settings");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"gfx_modify_spectrum_settings_field.  "
				"No modify data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_modify_spectrum_settings_field.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_modify_spectrum_field */

int cmzn_spectrumcomponent_modify(struct cmzn_spectrumcomponent *component,
	struct cmzn_spectrumcomponent *new_component,
	struct LIST(cmzn_spectrumcomponent) *list_of_components)
/*******************************************************************************
LAST MODIFIED : 10 March 1998

DESCRIPTION :
Changes the contents of component to match new_component, with no change in
priority. Sets component->changed to force graphics to be regenerated.
==============================================================================*/
{
	int return_code,old_position;

	ENTER(cmzn_spectrumcomponent_modify);
	if (component&&new_component&&list_of_components)
	{
		/* make sure graphics for these component are regenerated */
		component->changed=1;
		/* make sure position stays the same */
		old_position=component->position;
		return_code=COPY(cmzn_spectrumcomponent)(component,new_component);
		component->position=old_position;
	}
	else
	{
		display_message(ERROR_MESSAGE,"cmzn_spectrumcomponent_modify.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* cmzn_spectrumcomponent_modify */


char *cmzn_spectrumcomponent_string(struct cmzn_spectrumcomponent *component,
	enum cmzn_spectrumcomponent_string_details component_detail)
/*******************************************************************************
LAST MODIFIED : 15 March 2002

DESCRIPTION :
Returns a string describing the component, suitable for entry into the command
line. Parameter <component_detail> selects whether appearance component are
included in the string. User must remember to DEALLOCATE the name afterwards.
==============================================================================*/
{
	char *name,*component_string,temp_string[80];
	int error;

	ENTER(cmzn_spectrumcomponent_string);
	component_string=(char *)NULL;
	error=0;
	if (component&&(
		(SPECTRUM_COMPONENT_STRING_SPACE_ONLY==component_detail)||
		(SPECTRUM_COMPONENT_STRING_COMPLETE==component_detail)||
		(SPECTRUM_COMPONENT_STRING_COMPLETE_PLUS==component_detail)))
	{
		if (SPECTRUM_COMPONENT_STRING_COMPLETE_PLUS==component_detail)
		{
			sprintf(temp_string,"%i. ",component->position);
			append_string(&component_string,temp_string,&error);
		}
		switch (component->component_scale)
		{
			case CMZN_SPECTRUMCOMPONENT_SCALE_TYPE_LINEAR:
			{
				append_string(&component_string,"linear",&error);
			} break;
			case CMZN_SPECTRUMCOMPONENT_SCALE_TYPE_LOG:
			{
				sprintf(temp_string,"log exaggeration %g",fabs(component->exaggeration));
				append_string(&component_string,temp_string,&error);
				if (component->exaggeration >= 0)
				{
					append_string(&component_string," left",&error);
				}
				else
				{
					append_string(&component_string," right",&error);
				}
			} break;
			case CMZN_SPECTRUMCOMPONENT_SCALE_TYPE_INVALID:
			{
				if (component->is_field_lookup)
				{
					append_string(&component_string,"field",&error);
					append_string(&component_string," input ",&error);
					name=(char *)NULL;
					if (GET_NAME(Computed_field)(component->input_field,&name))
					{
						/* put quotes around name if it contains special characters */
						make_valid_token(&name);
						append_string(&component_string,name,&error);
						DEALLOCATE(name);
					}
					append_string(&component_string," output ",&error);
					name=(char *)NULL;
					if (GET_NAME(Computed_field)(component->output_field,&name))
					{
						/* put quotes around name if it contains special characters */
						make_valid_token(&name);
						append_string(&component_string,name,&error);
						DEALLOCATE(name);
					}
				}
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"cmzn_spectrumcomponent_string.  Unknown element component type");
			} break;
		}
		if ( component->reverse )
		{
			append_string(&component_string," reverse",&error);
		}
		sprintf(temp_string," range %g %g",component->minimum,
			component->maximum);
		append_string(&component_string,temp_string,&error);
		if ((component->extend_above)&&(component->colour_mapping_type!=CMZN_SPECTRUMCOMPONENT_COLOUR_MAPPING_TYPE_STEP))
		{
			append_string(&component_string," extend_above",&error);
		}
		if ((component->extend_below)&&(component->colour_mapping_type!=CMZN_SPECTRUMCOMPONENT_COLOUR_MAPPING_TYPE_STEP))
		{
			append_string(&component_string," extend_below",&error);
		}
		if (component->fix_maximum)
		{
			append_string(&component_string," fix_maximum",&error);
		}
		if (component->fix_minimum)
		{
			append_string(&component_string," fix_minimum",&error);
		}
		if (component->component_scale == CMZN_SPECTRUMCOMPONENT_SCALE_TYPE_LINEAR ||
			component->component_scale == CMZN_SPECTRUMCOMPONENT_SCALE_TYPE_LOG )
		{
			switch (component->colour_mapping_type)
			{
				case CMZN_SPECTRUMCOMPONENT_COLOUR_MAPPING_TYPE_ALPHA:
				case CMZN_SPECTRUMCOMPONENT_COLOUR_MAPPING_TYPE_BLUE:
				case CMZN_SPECTRUMCOMPONENT_COLOUR_MAPPING_TYPE_GREEN:
				case CMZN_SPECTRUMCOMPONENT_COLOUR_MAPPING_TYPE_MONOCHROME:
				case CMZN_SPECTRUMCOMPONENT_COLOUR_MAPPING_TYPE_RAINBOW:
				case CMZN_SPECTRUMCOMPONENT_COLOUR_MAPPING_TYPE_RED:
				case CMZN_SPECTRUMCOMPONENT_COLOUR_MAPPING_TYPE_WHITE_TO_BLUE:
				case CMZN_SPECTRUMCOMPONENT_COLOUR_MAPPING_TYPE_WHITE_TO_RED:
				case CMZN_SPECTRUMCOMPONENT_COLOUR_MAPPING_TYPE_WHITE_TO_GREEN:
				{
					sprintf(temp_string," %s colour_range %g %g",
						ENUMERATOR_STRING(cmzn_spectrumcomponent_colour_mapping_type)(component->colour_mapping_type),
						component->min_value, component->max_value);
					append_string(&component_string,temp_string,&error);
				} break;
				default:
				{
				} break;
				case CMZN_SPECTRUMCOMPONENT_COLOUR_MAPPING_TYPE_BANDED:
				{
					sprintf(temp_string," banded number_of_bands %d band_ratio %g",
						component->number_of_bands,
						(ZnReal)(component->black_band_proportion)/1000.0);
					append_string(&component_string,temp_string,&error);
				} break;
				case CMZN_SPECTRUMCOMPONENT_COLOUR_MAPPING_TYPE_STEP:
				{
					sprintf(temp_string," step_texture step_value %g",component->step_value);
					append_string(&component_string,temp_string,&error);
				} break;
			}
			sprintf(temp_string," component %d",component->component_number + 1);
			append_string(&component_string,temp_string,&error);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"cmzn_spectrumcomponent_string.  "
			"Invalid argument(s)");
	}
	LEAVE;

	return component_string;
} /* cmzn_spectrumcomponent_string */

int cmzn_spectrumcomponent_list_contents(struct cmzn_spectrumcomponent *component,
	void *list_data_void)
/*******************************************************************************
LAST MODIFIED : 22 January 2002

DESCRIPTION :
Writes out the <component> as a text string in the command window with the
<component_string_detail>, <line_prefix> and <line_suffix> given in the
<list_data>.
==============================================================================*/
{
	int return_code;
	char *component_string,line[80];
	struct cmzn_spectrumcomponent_list_data *list_data;

	ENTER(cmzn_spectrumcomponent_list_contents);
	if (component&&
		(list_data=(struct cmzn_spectrumcomponent_list_data *)list_data_void))
	{
		component_string=cmzn_spectrumcomponent_string(component,
			list_data->component_string_detail);
		if (component_string)
		{
			if (list_data->line_prefix)
			{
				display_message(INFORMATION_MESSAGE,list_data->line_prefix);
			}
			display_message(INFORMATION_MESSAGE,component_string);
			if (list_data->line_suffix)
			{
				display_message(INFORMATION_MESSAGE,list_data->line_suffix);
			}
			/*???RC temp */
			if ((SPECTRUM_COMPONENT_STRING_COMPLETE_PLUS==list_data->component_string_detail)&&
				(component->access_count != 1))
			{
				sprintf(line," (access count = %i)",component->access_count);
				display_message(INFORMATION_MESSAGE,line);
			}
			display_message(INFORMATION_MESSAGE,";\n");
			DEALLOCATE(component_string);
			return_code=1;
		}
		else
		{
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmzn_spectrumcomponent_list_contents.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* cmzn_spectrumcomponent_list_contents */

int cmzn_spectrumcomponent_write_contents(struct cmzn_spectrumcomponent *component,
	void *list_data_void)
/*******************************************************************************
LAST MODIFIED : 22 January 2002

DESCRIPTION :
Writes out the <component> as a text string in the command window with the
<component_string_detail>, <line_prefix> and <line_suffix> given in the
<list_data>.
==============================================================================*/
{
	int return_code;
	char *component_string,line[80];
	struct cmzn_spectrumcomponent_list_data *list_data;

	ENTER(cmzn_spectrumcomponent_write_contents);
	list_data=(struct cmzn_spectrumcomponent_list_data *)list_data_void;
	if (component&&list_data)
	{
		component_string=cmzn_spectrumcomponent_string(component,
			list_data->component_string_detail);
		if (component_string)
		{
			if (list_data->line_prefix)
			{
				write_message_to_file(INFORMATION_MESSAGE,list_data->line_prefix);
			}
			write_message_to_file(INFORMATION_MESSAGE,component_string);
			if (list_data->line_suffix)
			{
				 write_message_to_file(INFORMATION_MESSAGE,list_data->line_suffix);
			}
			/*???RC temp */
			if ((SPECTRUM_COMPONENT_STRING_COMPLETE_PLUS==list_data->component_string_detail)&&
				(component->access_count != 1))
			{
				sprintf(line," (access count = %i)",component->access_count);
				write_message_to_file(INFORMATION_MESSAGE,line);
			}
			write_message_to_file(INFORMATION_MESSAGE,";\n");
			DEALLOCATE(component_string);
			return_code=1;
		}
		else
		{
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmzn_spectrumcomponent_list_contents.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* cmzn_spectrumcomponent_write_contents */
