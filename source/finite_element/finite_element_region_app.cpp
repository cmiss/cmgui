/* OpenCMISS-Cmgui Application
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <stdio.h>

#include "opencmiss/zinc/element.h"
#include "opencmiss/zinc/node.h"
#include "general/debug.h"
#include "general/message.h"
#include "command/parser.h"
#include "finite_element/finite_element_mesh.hpp"
#include "finite_element/finite_element_nodeset.hpp"
#include "finite_element/finite_element_region.h"
// insert app headers here
#include "finite_element/finite_element_app.h"


int set_FE_field_conditional_FE_region(struct Parse_state *state,
	void *fe_field_address_void, void *parse_field_data_void)
/*******************************************************************************
LAST MODIFIED : 27 February 2003

DESCRIPTION :
FE_region wrapper for set_FE_field_conditional. <parse_field_data_void> points
at a struct Set_FE_field_conditional_FE_region_data.
==============================================================================*/
{
	int return_code;
	struct FE_region *fe_region;
	struct Set_FE_field_conditional_data set_field_data;
	struct Set_FE_field_conditional_FE_region_data *parse_field_data;

	ENTER(set_FE_field_conditional_FE_region);
	if (state && fe_field_address_void && (parse_field_data =
		(struct Set_FE_field_conditional_FE_region_data *)parse_field_data_void)
		&& (fe_region = parse_field_data->fe_region))
	{
		set_field_data.conditional_function = parse_field_data->conditional_function;
		set_field_data.conditional_function_user_data = parse_field_data->user_data;
		set_field_data.fe_field_list = FE_region_get_FE_field_list(fe_region);
		return_code = set_FE_field_conditional(state, fe_field_address_void,
			(void *)&set_field_data);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_FE_field_conditional_FE_region.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* set_FE_field_conditional_FE_region */

int set_FE_fields_FE_region(struct Parse_state *state,
	void *fe_field_order_info_address_void, void *fe_region_void)
/*******************************************************************************
LAST MODIFIED : 7 March 2003

DESCRIPTION :
FE_region wrapper for set_FE_fields.
==============================================================================*/
{
	int return_code;
	struct FE_region *fe_region;

	ENTER(set_FE_fields_FE_region);
	if (state && fe_field_order_info_address_void &&
		(fe_region = (struct FE_region *)fe_region_void))
	{
		return_code = set_FE_fields(state, fe_field_order_info_address_void,
			(void *)(FE_region_get_FE_field_list(fe_region)));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_FE_fields_FE_region.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* set_FE_fields_FE_region */

int set_FE_element_top_level_FE_region(struct Parse_state *state,
	void *element_address_void, void *fe_region_void)
/*******************************************************************************
LAST MODIFIED : 26 February 2003

DESCRIPTION :
A modifier function for specifying a top level element, used, for example, to
set the seed element for a xi_texture_coordinate computed_field.
==============================================================================*/
{
	const char *current_token;
	int return_code = 0;
	struct FE_element *element, **element_address;
	struct FE_region *fe_region;

	ENTER(set_FE_element_top_level_FE_region);
	if (state && (element_address = (struct FE_element **)element_address_void) &&
		(fe_region = (struct FE_region *)fe_region_void))
	{
		current_token = state->current_token;
		if (current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				int element_number = 0;
				if (1 == sscanf(current_token, "%d", &element_number))
				{
					shift_Parse_state(state,1);
					element = FE_region_get_top_level_FE_element_from_identifier(fe_region, element_number);
					if (element)
					{
						cmzn_element::reaccess(*element_address, element);
						return_code = 1;
					}
				}
				if (!return_code)
				{
					display_message(WARNING_MESSAGE,
						"Unknown seed element: %s", current_token);
					display_parse_state_location(state);
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE, " ELEMENT_NUMBER");
				element = *element_address;
				if (element)
				{
					display_message(INFORMATION_MESSAGE, "[%d]", cmzn_element_get_identifier(element));
				}
				return_code = 1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE, "Missing seed element number");
			display_parse_state_location(state);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_FE_element_top_level_FE_region.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* set_FE_element_top_level_FE_region */
