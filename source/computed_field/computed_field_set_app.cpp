/* OpenCMISS-Cmgui Application
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "general/debug.h"
#include "general/message.h"
#include "general/mystring.h"
#include "command/parser.h"
#include "computed_field/computed_field_set.h"
#include "computed_field/computed_field_composite.h"
#include "computed_field/computed_field_private.hpp"
#include "computed_field/field_module.hpp"

int set_Computed_field_conditional(struct Parse_state *state,
	void *field_address_void, void *set_field_data_void)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Modifier function to set the field from a command. <set_field_data_void> should
point to a struct Set_Computed_field_conditional_data containing the
computed_field_manager and an optional conditional function for narrowing the
range of fields available for selection. If the conditional_function is NULL,
no criteria are placed on the chosen field.
Allows the construction field.component name to automatically make a component
wrapper for field and add it to the manager.
==============================================================================*/
{
	char *command_string, *temp_name;
	const char *current_token;
	int component_no, error, finished, index, number_of_values, return_code;
	double *values;
	struct Computed_field **field_address, *selected_field;
	struct Set_Computed_field_conditional_data *set_field_data;

	ENTER(set_Computed_field_conditional);
	if (state && (field_address = (struct Computed_field **)field_address_void) &&
		(set_field_data =
			(struct Set_Computed_field_conditional_data *)set_field_data_void) &&
		set_field_data->computed_field_manager)
	{
		selected_field = (struct Computed_field *)NULL;
		current_token = state->current_token;
		if (current_token != 0)
		{
			if (strcmp(PARSER_HELP_STRING, current_token) &&
				strcmp(PARSER_RECURSIVE_HELP_STRING, current_token))
			{
				if (fuzzy_string_compare(current_token, "NONE"))
				{
					if (*field_address)
					{
						DEACCESS(Computed_field)(field_address);
						*field_address = (struct Computed_field *)NULL;
					}
					return_code = 1;
				}
				else
				{
					if (current_token[0] == '[')
					{
						error = 0;
						command_string = duplicate_string(current_token);

						number_of_values = 0;
						/* Dont examine the '[' */
						current_token += 1;

						ALLOCATE(values, double, 1);
						/* This is a constant array, make a new private constant field */
						finished = 0;
						while (!finished)
						{
							while (current_token &&
								(1 == sscanf(current_token, "%lf%n",
								values + number_of_values, &index)))
							{
								number_of_values++;
								REALLOCATE(values, values, double, number_of_values + 1);
								current_token += index;
							}
							if (strchr(current_token,']') ||
								(!shift_Parse_state(state, 1)) ||
								(!(current_token = state->current_token)))
							{
								finished = 1;
							}
							else
							{
								append_string(&command_string, " ", &error);
								append_string(&command_string, current_token, &error);
							}
						}

						cmzn_region *region =
							Computed_field_manager_get_region(set_field_data->computed_field_manager);
						cmzn_fieldmodule *temp_field_module = cmzn_region_get_fieldmodule(region);
						cmzn_fieldmodule_set_field_name(temp_field_module, "constants");
						selected_field = cmzn_fieldmodule_create_field_constant(temp_field_module,
							number_of_values, values);
						cmzn_fieldmodule_destroy(&temp_field_module);

						DEALLOCATE(values);
						DEALLOCATE(command_string);
					}
					else
					{
						/* component_no = -1 denotes the whole field may be used */
						component_no = -1;
						selected_field = Computed_field_manager_get_field_or_component(
							set_field_data->computed_field_manager, current_token, &component_no);
						if (selected_field)
						{
							if (component_no == -1)
							{
								ACCESS(Computed_field)(selected_field);
							}
							else
							{
								selected_field = Computed_field_manager_get_component_wrapper(
									set_field_data->computed_field_manager, selected_field, component_no);
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,"Unknown field or field.component: %s", current_token);
						}
					}
					if (selected_field)
					{
						if ((NULL == set_field_data->conditional_function) ||
							((set_field_data->conditional_function)(selected_field,
								set_field_data->conditional_function_user_data)))
						{
							REACCESS(Computed_field)(field_address, selected_field);
							return_code=1;
						}
						else
						{
							display_message(ERROR_MESSAGE, "Field of incorrect type : %s",
								current_token);
							return_code = 0;
						}
					}
					else
					{
						return_code = 0;
					}
				}
				shift_Parse_state(state, 1);
			}
			else
			{
				display_message(INFORMATION_MESSAGE,
					" FIELD_NAME[.COMPONENT_NAME]|none");
				/* if possible, then write the name */
				selected_field = *field_address;
				if (selected_field != 0)
				{
					ACCESS(Computed_field)(selected_field);
					GET_NAME(Computed_field)(selected_field, &temp_name);
					display_message(INFORMATION_MESSAGE, "[%s]", temp_name);
					DEALLOCATE(temp_name);
				}
				else
				{
					display_message(INFORMATION_MESSAGE, "[none]");
				}
				return_code=1;
			}
		}
		else
		{
			display_message(WARNING_MESSAGE, "Missing field name");
			display_parse_state_location(state);
			return_code = 0;
		}
		REACCESS(Computed_field)(&selected_field, NULL);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_Computed_field_conditional.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* set_Computed_field_conditional */

int Option_table_add_Computed_field_conditional_entry(
	struct Option_table *option_table, const char *token,
	struct Computed_field **field_address,
	struct Set_Computed_field_conditional_data *set_field_data)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Adds the given <token> to the <option_table>.
==============================================================================*/
{
	int return_code;

	ENTER(Option_table_add_Computed_field_conditional_entry);
	if (option_table && token && field_address && set_field_data)
	{
		return_code = Option_table_add_entry(option_table, token, (void *)field_address,
			(void *)set_field_data, set_Computed_field_conditional);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Option_table_add_Computed_field_conditional_entry.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Option_table_add_Computed_field_conditional_entry */

int set_Computed_field_array(struct Parse_state *state,
	void *field_array_void, void *set_field_array_data_void)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Modifier function to set an array of field from a command.
<set_field_array_data_void> should point to a struct
Set_Computed_field_array_conditional_data containing the number_of_fields in the
array, the computed_field_package and an optional conditional function for
narrowing the range of fields available for selection.
Works by repeatedly calling set_Computed_field_conditional.
???RC Make this globally available for calling any modifier function?
==============================================================================*/
{
	int i, return_code;
	struct Computed_field **field_array;
	struct Set_Computed_field_array_data *set_field_array_data;

	ENTER(set_Computed_field_array);
	if (state && (field_array = (struct Computed_field **)field_array_void) &&
		(set_field_array_data =
			(struct Set_Computed_field_array_data *)set_field_array_data_void) &&
		(0 < set_field_array_data->number_of_fields) &&
		set_field_array_data->conditional_data)
	{
		return_code = 1;
		for (i = 0; i < set_field_array_data->number_of_fields; i++)
		{
			if (!set_Computed_field_conditional(state,
				&(field_array[i]), (void *)set_field_array_data->conditional_data))
			{
				return_code = 0;
			}
		}
		if (!return_code)
		{
			if ((!state->current_token) ||
				(strcmp(PARSER_HELP_STRING, state->current_token) &&
					strcmp(PARSER_RECURSIVE_HELP_STRING, state->current_token)))
			{
				display_message(ERROR_MESSAGE,
					"set_Computed_field_array.  Error parsing field array");
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_Computed_field_array.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* set_Computed_field_array */

