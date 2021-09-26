/* OpenCMISS-Cmgui Application
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "general/debug.h"
#include "general/message.h"
#include "command/parser.h"
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_app.h"
#include "computed_field/computed_field_private.hpp"
#include "computed_field/computed_field_private_app.hpp"
#include "computed_field/computed_field_set.h"
#include "computed_field/computed_field_set_app.h"
#include "computed_field/computed_field_format_output.h"

const char computed_field_format_output_type_string[] = "format_output";

int Computed_field_get_type_format_output(struct Computed_field *field,
	struct Computed_field **source_field, char **format_string_out);


int define_Computed_field_type_format_output(struct Parse_state *state,
	void *field_modify_void, void *)
/*******************************************************************************
LAST MODIFIED : 15 May 2008

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_FORMAT_OUTPUT (if it is not
already) and allows its contents to be modified.
==============================================================================*/
{
	int return_code;
	char *format_string;
	struct Computed_field *source_field;
	Computed_field_modify_data *field_modify = static_cast<Computed_field_modify_data *>(field_modify_void);
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_format_output);
	if ((state) && (field_modify))
	{
		return_code=1;
		source_field = (struct Computed_field *)NULL;
		format_string = NULL;
		if ((NULL != field_modify->get_field()) &&
			(computed_field_format_output_type_string ==
				Computed_field_get_type_string(field_modify->get_field())))
		{
			return_code=Computed_field_get_type_format_output(field_modify->get_field(),
				&source_field, &format_string);
		}
		if (return_code)
		{
			/* must access objects for set functions */
			if (source_field)
			{
				ACCESS(Computed_field)(source_field);
			}

			option_table = CREATE(Option_table)();
			/* fields */
			set_source_field_data.computed_field_manager=
				field_modify->get_field_manager();
			set_source_field_data.conditional_function=
				Computed_field_has_numerical_components;
			set_source_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_Computed_field_conditional_entry(option_table,"field",
				&source_field, &set_source_field_data);
			Option_table_add_string_entry(option_table, "format_string", &format_string,
				"C style formatting string");
			return_code=Option_table_multi_parse(option_table,state);
			/* no errors,not asking for help */
			if (return_code)
			{
				return_code = field_modify->define_field(
					cmzn_fieldmodule_create_field_format_output(field_modify->get_field_module(),
						source_field, format_string));
			}
			if (!return_code)
			{
				if ((!state->current_token)||
					(strcmp(PARSER_HELP_STRING,state->current_token)&&
						strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
				{
					/* error */
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_format_output.  Failed");
				}
			}
			if (source_field)
			{
				DEACCESS(Computed_field)(&source_field);
			}
			if (format_string)
				DEALLOCATE(format_string);
			DESTROY(Option_table)(&option_table);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"define_Computed_field_type_format_output.  Not enough memory");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_format_output.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_format_output */

int Computed_field_register_types_format_output(
	struct Computed_field_package *computed_field_package)
{
	int return_code;

	ENTER(Computed_field_register_types_format_output);
	if (computed_field_package)
	{
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_format_output_type_string,
			define_Computed_field_type_format_output,
			Computed_field_package_get_simple_package(computed_field_package));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_register_types_format_output.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_register_types_format_output */
