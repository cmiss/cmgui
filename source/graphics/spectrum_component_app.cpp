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

DEFINE_DEFAULT_OPTION_TABLE_ADD_ENUMERATOR_FUNCTION(Cmiss_spectrum_component_colour_mapping);

int gfx_modify_spectrum_settings_linear(struct Parse_state *state,
	void *modify_spectrum_data_void,void *dummy)
/*******************************************************************************
LAST MODIFIED : 17 January 2001

DESCRIPTION :
Executes a GFX MODIFY SPECTRUM LINEAR command.
If return_code is 1, returns the completed Modify_spectrum_app_data with the
parsed settings. Note that the settings are ACCESSed once on valid return.
==============================================================================*/
{
	char ambient,amb_diff,diffuse,emission,extend_above,
		extend_below,fix_maximum,fix_minimum,reverse,specular,
		transparent_above,transparent_below;
	enum Cmiss_spectrum_component_colour_mapping colour_mapping;
	int black_band_int,component,number_of_bands,range_components,return_code;
	float band_ratio,step_value;
	FE_value colour_range[2],range[2];
	struct Modify_spectrum_app_data *modify_spectrum_data;
	struct Option_table *option_table, *render_option_table;
	struct Cmiss_spectrum_component *settings;

	ENTER(gfx_modify_spectrum_settings_linear);
	USE_PARAMETER(dummy);
	if (state)
	{
		modify_spectrum_data=
			(struct Modify_spectrum_app_data *)modify_spectrum_data_void;
		if (modify_spectrum_data)
		{
			/* create the spectrum_settings: */
			settings=modify_spectrum_data->component=CREATE(Cmiss_spectrum_component)();
			if (settings)
			{
				/* access since deaccessed in gfx_modify_spectrum */

				Cmiss_spectrum_component_set_interpolation_mode(settings,CMISS_SPECTRUM_COMPONENT_INTERPOLATION_LINEAR);

				colour_mapping = Cmiss_spectrum_component_get_colour_mapping(settings);
				ambient = 0;
				amb_diff = 0;
				band_ratio = 0.01;
				diffuse = 0;
				emission = 0;
				extend_above = 0;
				extend_below = 0;
				fix_maximum=0;
				fix_minimum=0;
				number_of_bands = 10;
				reverse = 0;
				specular = 0;
				step_value = 0.5;
				transparent_above = 0;
				transparent_below = 0;
				range_components = 2;
				colour_range[0] = 0.0;
				colour_range[1] = 1.0;
				range[0] = (FE_value)(modify_spectrum_data->spectrum_minimum);
				range[1] = (FE_value)(modify_spectrum_data->spectrum_maximum);
				component = Cmiss_spectrum_component_get_field_component_lookup_number(settings);

				option_table = CREATE(Option_table)();
				/* band_ratio */
				Option_table_add_float_entry(option_table, "band_ratio",
					&band_ratio);
				/* colour_range */
				Option_table_add_FE_value_vector_entry(option_table, "colour_range",
					colour_range, &range_components);
				/* component */
				Option_table_add_int_positive_entry(option_table, "component",
					&component);
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
				/* step_value */
				Option_table_add_float_entry(option_table, "step_value",
					&step_value);
				/* transparent_above */
				Option_table_add_char_flag_entry(option_table, "transparent_above",
					&transparent_above);
				/* transparent_below */
				Option_table_add_char_flag_entry(option_table, "transparent_below",
					&transparent_below);
				/* conversion_mode */
				OPTION_TABLE_ADD_ENUMERATOR(Cmiss_spectrum_component_colour_mapping)(
					option_table, &colour_mapping);
				/* render_option */
				render_option_table = CREATE(Option_table)();
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
					DEACCESS(Cmiss_spectrum_component)(&(modify_spectrum_data->component));
				}
				DESTROY(Option_table)(&option_table);
				if (return_code)
				{
					Cmiss_spectrum_component_set_colour_mapping(settings,
						colour_mapping);
				}
				if ( return_code )
				{
					if (specular + emission > 1)
					{
						display_message(ERROR_MESSAGE,"gfx_modify_spectrum_settings_linear.  "
							"All spectrums are ambient diffuse.  The specular and emission flags are ignored.");
						return_code=0;
					}
				}
				if ( return_code )
				{
					if ( extend_above && transparent_above)
					{
						display_message(ERROR_MESSAGE,"gfx_modify_spectrum_settings_linear.  "
							"Specify only one of extend_above and transparent_above");
						return_code=0;
					}
					else if (extend_above)
					{
						Cmiss_spectrum_component_set_extend_above_flag(settings, true);
					}
					else if (transparent_above)
					{
						Cmiss_spectrum_component_set_extend_above_flag(settings, false);
					}
				}
				if ( return_code )
				{
					if ( extend_below && transparent_below)
					{
						display_message(ERROR_MESSAGE,"gfx_modify_spectrum_settings_linear.  "
							"Specify only one of extend_below and transparent_below");
						return_code=0;
					}
					else if (extend_below)
					{
						Cmiss_spectrum_component_set_extend_below_flag(settings, true);
					}
					else if (transparent_below)
					{
						Cmiss_spectrum_component_set_extend_below_flag(settings, false);
					}
				}
				if ( return_code )
				{
					Cmiss_spectrum_component_set_field_component_lookup_number(settings,
						component);
					Cmiss_spectrum_component_set_attribute_real(settings,
						CMISS_SPECTRUM_COMPONENT_ATTRIBUTE_COLOUR_MINIMUM,
						colour_range[0]);
					Cmiss_spectrum_component_set_attribute_real(settings,
						CMISS_SPECTRUM_COMPONENT_ATTRIBUTE_COLOUR_MAXIMUM,
						colour_range[1]);
					Cmiss_spectrum_component_set_attribute_real(settings,
						CMISS_SPECTRUM_COMPONENT_ATTRIBUTE_RANGE_MINIMUM,
						range[0]);
					Cmiss_spectrum_component_set_attribute_real(settings,
						CMISS_SPECTRUM_COMPONENT_ATTRIBUTE_RANGE_MAXIMUM,
						range[1]);
					Cmiss_spectrum_component_set_reverse_flag(settings,
						reverse);
					Cmiss_spectrum_component_set_number_of_bands(settings,
						number_of_bands);
					black_band_int = (band_ratio * 1000.0 + 0.5);
					Cmiss_spectrum_component_set_black_band_proportion(settings,
						black_band_int);
					/* Must set step value after setting minimum and maximum range */
					Cmiss_spectrum_component_set_attribute_real(settings,
						CMISS_SPECTRUM_COMPONENT_ATTRIBUTE_STEP_VALUE, step_value);
				}
				/* Must set fix_maximum,fix_minimum after setting minimum and maximum range */
				if ( return_code )
				{
					if (fix_maximum)
					{
						Cmiss_spectrum_component_set_fix_maximum_flag(settings, 1);
					}
				}
				if ( return_code )
				{
					if (fix_minimum)
					{
						Cmiss_spectrum_component_set_fix_minimum_flag(settings, 1);
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,"gfx_modify_spectrum_settings_linear.  "
					"Could not create settings");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"gfx_modify_spectrum_settings_linear.  "
				"No modify data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_modify_spectrum_settings_linear.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_modify_spectrum_settings_linear */

int gfx_modify_spectrum_settings_log(struct Parse_state *state,
	void *modify_spectrum_data_void,void *dummy)
/*******************************************************************************
LAST MODIFIED : 17 January 2001

DESCRIPTION :
Executes a GFX MODIFY SPECTRUM LOG command.
If return_code is 1, returns the completed Modify_spectrum_app_data with the
parsed settings. Note that the settings are ACCESSed once on valid return.
==============================================================================*/
{
	char ambient,amb_diff,diffuse,emission,extend_above,
		extend_below,fix_maximum,fix_minimum,left,reverse,right,
		specular,transparent_above,transparent_below;
	enum Cmiss_spectrum_component_colour_mapping colour_mapping;
	int black_band_int,component,number_of_bands,range_components,return_code;
	float band_ratio,exaggeration,step_value;
	FE_value colour_range[2],range[2];
	struct Modify_spectrum_app_data *modify_spectrum_data;
	struct Option_table *option_table, *render_option_table;
	struct Cmiss_spectrum_component *settings;

	ENTER(gfx_modify_spectrum_settings_log);
	USE_PARAMETER(dummy);
	if (state)
	{
		modify_spectrum_data=(struct Modify_spectrum_app_data *)modify_spectrum_data_void;
		if (modify_spectrum_data)
		{
			/* create the spectrum_settings: */
			settings=modify_spectrum_data->component=CREATE(Cmiss_spectrum_component)();
			if (settings)
			{
				Cmiss_spectrum_component_set_interpolation_mode(settings,CMISS_SPECTRUM_COMPONENT_INTERPOLATION_LOG);

				colour_mapping = Cmiss_spectrum_component_get_colour_mapping(settings);
				ambient = 0;
				amb_diff = 0;
				band_ratio = 0.01;
				diffuse = 0;
				emission = 0;
				extend_above = 0;
				extend_below = 0;
				fix_maximum=0;
				fix_minimum=0;
				left = 0;
				number_of_bands = 10;
				reverse = 0;
				right = 0;
				specular = 0;
				step_value = 0.5;
				transparent_above = 0;
				transparent_below = 0;
				range_components = 2;
				colour_range[0] = 0.0;
				colour_range[1] = 1.0;
				range[0] = (FE_value)(modify_spectrum_data->spectrum_minimum);
				range[1] = (FE_value)(modify_spectrum_data->spectrum_maximum);
				component = Cmiss_spectrum_component_get_field_component_lookup_number(settings);
				exaggeration =  Cmiss_spectrum_component_get_attribute_real(
					settings, CMISS_SPECTRUM_COMPONENT_ATTRIBUTE_EXAGGERATION);

				option_table = CREATE(Option_table)();
				/* band_ratio */
				Option_table_add_float_entry(option_table, "band_ratio",
					&band_ratio);
				/* colour_range */
				Option_table_add_FE_value_vector_entry(option_table, "colour_range",
					colour_range, &range_components);
				/* component */
				Option_table_add_int_positive_entry(option_table, "component",
					&component);
				/* exaggeration */
				Option_table_add_float_entry(option_table, "exaggeration",
					&exaggeration);
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
				/* left */
				Option_table_add_char_flag_entry(option_table, "left",
					&left);
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
				/* right */
				Option_table_add_char_flag_entry(option_table, "right",
					&right);
				/* step_value */
				Option_table_add_float_entry(option_table, "step_value",
					&step_value);
				/* transparent_above */
				Option_table_add_char_flag_entry(option_table, "transparent_above",
					&transparent_above);
				/* transparent_below */
				Option_table_add_char_flag_entry(option_table, "transparent_below",
					&transparent_below);
				/* conversion_mode */
				Option_table_add_enumerator_Cmiss_spectrum_component_colour_mapping(
					option_table, &colour_mapping);
				/* render_option */
				render_option_table = CREATE(Option_table)();
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
					DEACCESS(Cmiss_spectrum_component)(&(modify_spectrum_data->component));
				}
				DESTROY(Option_table)(&option_table);
				if (return_code)
				{
					Cmiss_spectrum_component_set_colour_mapping(settings,
						colour_mapping);
					Cmiss_spectrum_component_set_number_of_bands(settings,
						number_of_bands);
					black_band_int = (band_ratio * 1000.0 + 0.5);
					Cmiss_spectrum_component_set_black_band_proportion(settings,
						black_band_int);
					Cmiss_spectrum_component_set_attribute_real(settings,
						CMISS_SPECTRUM_COMPONENT_ATTRIBUTE_STEP_VALUE, step_value);
				}
				if ( return_code )
				{
					if (specular + emission > 1)
					{
						display_message(ERROR_MESSAGE,"gfx_modify_spectrum_settings_linear.  "
							"All spectrums are ambient diffuse.  The specular and emission flags are ignored.");
						return_code=0;
					}
				}
				if ( return_code )
				{
					if ( extend_above && transparent_above)
					{
						display_message(ERROR_MESSAGE,"gfx_modify_spectrum_settings_log.  "
							"Specify only one of extend_above and transparent_above");
						return_code=0;
					}
					else if (extend_above)
					{
						Cmiss_spectrum_component_set_extend_above_flag(settings, true);
					}
					else if (transparent_above)
					{
						Cmiss_spectrum_component_set_extend_above_flag(settings, false);
					}
				}
				if ( return_code )
				{
					if ( extend_below && transparent_below)
					{
						display_message(ERROR_MESSAGE,"gfx_modify_spectrum_settings_log.  "
							"Specify only one of extend_below and transparent_below");
						return_code=0;
					}
					else if (extend_below)
					{
						Cmiss_spectrum_component_set_extend_below_flag(settings, true);
					}
					else if (transparent_below)
					{
						Cmiss_spectrum_component_set_extend_below_flag(settings, false);
					}
				}
				if ( return_code )
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
					Cmiss_spectrum_component_set_attribute_real(settings,
						CMISS_SPECTRUM_COMPONENT_ATTRIBUTE_EXAGGERATION, exaggeration);
				}
				if ( return_code )
				{
					Cmiss_spectrum_component_set_field_component_lookup_number(settings,
						component);
					Cmiss_spectrum_component_set_attribute_real(settings,
						CMISS_SPECTRUM_COMPONENT_ATTRIBUTE_COLOUR_MINIMUM,
						colour_range[0]);
					Cmiss_spectrum_component_set_attribute_real(settings,
						CMISS_SPECTRUM_COMPONENT_ATTRIBUTE_COLOUR_MAXIMUM,
						colour_range[1]);
					Cmiss_spectrum_component_set_attribute_real(settings,
						CMISS_SPECTRUM_COMPONENT_ATTRIBUTE_RANGE_MINIMUM,
						range[0]);
					Cmiss_spectrum_component_set_attribute_real(settings,
						CMISS_SPECTRUM_COMPONENT_ATTRIBUTE_RANGE_MAXIMUM,
						range[1]);
					Cmiss_spectrum_component_set_reverse_flag(settings,
						reverse);
				}
				/* Must set fix_maximum,fix_minimum after setting minimum and maximum range */
				if ( return_code )
				{
					if (fix_maximum)
					{
						Cmiss_spectrum_component_set_fix_maximum_flag(settings, 1);
					}
				}
				if ( return_code )
				{
					if (fix_minimum)
					{
						Cmiss_spectrum_component_set_fix_minimum_flag(settings, 1);
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,"gfx_modify_spectrum_settings_log.  "
					"Could not create settings");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"gfx_modify_spectrum_settings_log.  "
				"No modify data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_modify_spectrum_settings_log.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_modify_spectrum_log */

int gfx_modify_spectrum_settings_field(struct Parse_state *state,
	void *modify_spectrum_data_void,void *dummy)
/*******************************************************************************
LAST MODIFIED : 11 April 2007

DESCRIPTION :
Executes a GFX MODIFY SPECTRUM FIELD command.
If return_code is 1, returns the completed Modify_spectrum_app_data with the
parsed settings. Note that the settings are ACCESSed once on valid return.
==============================================================================*/
{
	enum Cmiss_spectrum_component_colour_mapping colour_mapping;
	int component, return_code;
	struct Computed_field *input_field, *output_field;
	struct Modify_spectrum_app_data *modify_spectrum_data;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_input_field_data,
		set_output_field_data;
	struct Cmiss_spectrum_component *settings;

	ENTER(gfx_modify_spectrum_settings_field);
	USE_PARAMETER(dummy);
	if (state)
	{
		modify_spectrum_data=(struct Modify_spectrum_app_data *)modify_spectrum_data_void;
		if (modify_spectrum_data)
		{
			/* create the spectrum_settings: */
			settings=modify_spectrum_data->component=CREATE(Cmiss_spectrum_component)();
			if (settings)
			{
				Cmiss_spectrum_component_set_interpolation_mode(settings,CMISS_SPECTRUM_COMPONENT_INTERPOLATION_FIELD);

				colour_mapping = Cmiss_spectrum_component_get_colour_mapping(settings);
				component = Cmiss_spectrum_component_get_field_component_lookup_number(settings);

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
				Option_table_add_enumerator_Cmiss_spectrum_component_colour_mapping(
					option_table, &colour_mapping);

				if (!(return_code=Option_table_multi_parse(option_table,state)))
				{
					DEACCESS(Cmiss_spectrum_component)(&(modify_spectrum_data->component));
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
					Cmiss_spectrum_component_set_field_component_lookup_number(settings,
						component);
					Cmiss_spectrum_component_set_colour_mapping(settings,
						colour_mapping);
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

int Cmiss_spectrum_component_modify(struct Cmiss_spectrum_component *component,
	struct Cmiss_spectrum_component *new_component,
	struct LIST(Cmiss_spectrum_component) *list_of_components)
/*******************************************************************************
LAST MODIFIED : 10 March 1998

DESCRIPTION :
Changes the contents of component to match new_component, with no change in
priority. Sets component->changed to force graphics to be regenerated.
==============================================================================*/
{
	int return_code,old_position;

	ENTER(Cmiss_spectrum_component_modify);
	if (component&&new_component&&list_of_components)
	{
		/* make sure graphics for these component are regenerated */
		component->changed=1;
		/* make sure position stays the same */
		old_position=component->position;
		return_code=COPY(Cmiss_spectrum_component)(component,new_component);
		component->position=old_position;
	}
	else
	{
		display_message(ERROR_MESSAGE,"Cmiss_spectrum_component_modify.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_spectrum_component_modify */


char *Cmiss_spectrum_component_string(struct Cmiss_spectrum_component *component,
	enum Cmiss_spectrum_component_string_details component_detail)
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

	ENTER(Cmiss_spectrum_component_string);
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
		switch (component->component_interpolation)
		{
			case CMISS_SPECTRUM_COMPONENT_INTERPOLATION_LINEAR:
			{
				append_string(&component_string,"linear",&error);
			} break;
			case CMISS_SPECTRUM_COMPONENT_INTERPOLATION_LOG:
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
			case CMISS_SPECTRUM_COMPONENT_INTERPOLATION_FIELD:
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
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"Cmiss_spectrum_component_string.  Unknown element component type");
			} break;
		}
		if ( component->reverse )
		{
			append_string(&component_string," reverse",&error);
		}
		sprintf(temp_string," range %g %g",component->minimum,
			component->maximum);
		append_string(&component_string,temp_string,&error);
		if ((component->extend_above)&&(component->colour_mapping!=CMISS_SPECTRUM_COMPONENT_COLOUR_MAPPING_STEP))
		{
			append_string(&component_string," extend_above",&error);
		}
		if ((component->extend_below)&&(component->colour_mapping!=CMISS_SPECTRUM_COMPONENT_COLOUR_MAPPING_STEP))
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
		if (component->component_interpolation == CMISS_SPECTRUM_COMPONENT_INTERPOLATION_LINEAR ||
			component->component_interpolation == CMISS_SPECTRUM_COMPONENT_INTERPOLATION_LOG )
		{
			switch (component->colour_mapping)
			{
				case CMISS_SPECTRUM_COMPONENT_COLOUR_MAPPING_ALPHA:
				case CMISS_SPECTRUM_COMPONENT_COLOUR_MAPPING_BLUE:
				case CMISS_SPECTRUM_COMPONENT_COLOUR_MAPPING_GREEN:
				case CMISS_SPECTRUM_COMPONENT_COLOUR_MAPPING_MONOCHROME:
				case CMISS_SPECTRUM_COMPONENT_COLOUR_MAPPING_RAINBOW:
				case CMISS_SPECTRUM_COMPONENT_COLOUR_MAPPING_RED:
				case CMISS_SPECTRUM_COMPONENT_COLOUR_MAPPING_WHITE_TO_BLUE:
				case CMISS_SPECTRUM_COMPONENT_COLOUR_MAPPING_WHITE_TO_RED:
				case CMISS_SPECTRUM_COMPONENT_COLOUR_MAPPING_WHITE_TO_GREEN:
				{
					sprintf(temp_string," %s colour_range %g %g",
						ENUMERATOR_STRING(Cmiss_spectrum_component_colour_mapping)(component->colour_mapping),
						component->min_value, component->max_value);
					append_string(&component_string,temp_string,&error);
				} break;
				default:
				{
				} break;
				case CMISS_SPECTRUM_COMPONENT_COLOUR_MAPPING_BANDED:
				{
					sprintf(temp_string," banded number_of_bands %d band_ratio %g",
						component->number_of_bands,
						(ZnReal)(component->black_band_proportion)/1000.0);
					append_string(&component_string,temp_string,&error);
				} break;
				case CMISS_SPECTRUM_COMPONENT_COLOUR_MAPPING_STEP:
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
		display_message(ERROR_MESSAGE,"Cmiss_spectrum_component_string.  "
			"Invalid argument(s)");
	}
	LEAVE;

	return component_string;
} /* Cmiss_spectrum_component_string */

int Cmiss_spectrum_component_list_contents(struct Cmiss_spectrum_component *component,
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
	struct Cmiss_spectrum_component_list_data *list_data;

	ENTER(Cmiss_spectrum_component_list_contents);
	if (component&&
		(list_data=(struct Cmiss_spectrum_component_list_data *)list_data_void))
	{
		component_string=Cmiss_spectrum_component_string(component,
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
			"Cmiss_spectrum_component_list_contents.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_spectrum_component_list_contents */

int Cmiss_spectrum_component_write_contents(struct Cmiss_spectrum_component *component,
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
	struct Cmiss_spectrum_component_list_data *list_data;

	ENTER(Cmiss_spectrum_component_write_contents);
	list_data=(struct Cmiss_spectrum_component_list_data *)list_data_void;
	if (component&&list_data)
	{
		component_string=Cmiss_spectrum_component_string(component,
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
			"Cmiss_spectrum_component_list_contents.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_spectrum_component_write_contents */
