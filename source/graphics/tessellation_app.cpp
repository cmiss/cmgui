/* OpenCMISS-Cmgui Application
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "zinc/core.h"
#include "general/debug.h"
#include "general/message.h"
#include "command/parser.h"
#include "graphics/auxiliary_graphics_types_app.h"
#include "graphics/tessellation.hpp"

/***************************************************************************//**
 * Modifier function for setting positive numbers of divisions separated by *.
 *
 * @param divisions_address_void  address of where to allocate return int array.
 * Must be initialised to NULL or allocated array.
 * @param size_address_void  address of int to receive array size. Must be
 * initialised to size of initial divisions_address, or 0 if none.
 */
int set_divisions(struct Parse_state *state,
	void *divisions_address_void, void *size_address_void)
{
	int return_code = 1;

	int **divisions_address = (int **)divisions_address_void;
	int *size_address = (int *)size_address_void;
	if (state && divisions_address && size_address &&
		((NULL == *divisions_address) || (0 < *size_address)))
	{
		const char *current_token = state->current_token;
		if (current_token)
		{
			if (strcmp(PARSER_HELP_STRING, current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING, current_token))
			{
				int size = 0;
				int *values = NULL;
				if (current_token)
				{
					return_code = string_to_divisions(current_token, &values, &size);
				}
				if (!return_code)
				{
					display_parse_state_location(state);
				}
				if (return_code)
				{
					shift_Parse_state(state,1);
					DEALLOCATE(*divisions_address);
					*divisions_address = values;
					*size_address = size;
				}
				else
				{
					DEALLOCATE(values);
				}
			}
			else
			{
				/* write help */
				display_message(INFORMATION_MESSAGE, " \"#*#*...\"");
				if (*divisions_address)
				{
					display_message(INFORMATION_MESSAGE, "[\"");
					list_divisions(*size_address, *divisions_address);
					display_message(INFORMATION_MESSAGE, "\"]");
				}
				display_message(INFORMATION_MESSAGE, "{>=0}");
				return_code = 1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE, "Missing values");
			display_parse_state_location(state);
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE, "set_divisions.  Invalid argument(s)");
		return_code = 0;
	}
	return (return_code);
}

int Option_table_add_divisions_entry(struct Option_table *option_table,
	const char *token, int **divisions_address, int *size_address)
{
	int return_code;
	if (option_table && token && divisions_address && size_address)
	{
		return_code = Option_table_add_entry(option_table, token,
			(void *)divisions_address, (void *)size_address, set_divisions);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Option_table_add_divisions_entry.  Invalid argument(s)");
		return_code=0;
	}
	return (return_code);
}

int gfx_define_tessellation_contents(struct Parse_state *state, void *tessellation_void,
	void *tessellationmodule_void)
{
	int return_code = 1;
	cmzn_tessellationmodule *tessellationmodule =
		reinterpret_cast<cmzn_tessellationmodule *>(tessellationmodule_void);
	if (state && tessellationmodule)
	{
		cmzn_tessellation *tessellation = (cmzn_tessellation *)tessellation_void; // can be null
		int circleDivisions = 0;
		int minimum_divisions_size = 1;
		int refinement_factors_size = 1;
		if (tessellation)
		{
			circleDivisions = cmzn_tessellation_get_circle_divisions(tessellation);
			minimum_divisions_size = cmzn_tessellation_get_minimum_divisions(tessellation, 0, 0);
			refinement_factors_size = cmzn_tessellation_get_refinement_factors(tessellation, 0, 0);
		}
		int *minimum_divisions;
		int *refinement_factors;
		ALLOCATE(minimum_divisions, int, minimum_divisions_size);
		ALLOCATE(refinement_factors, int, refinement_factors_size);
		if (tessellation)
		{
			cmzn_tessellation_get_minimum_divisions(tessellation, minimum_divisions_size, minimum_divisions);
			cmzn_tessellation_get_refinement_factors(tessellation, refinement_factors_size, refinement_factors);
		}
		else
		{
			minimum_divisions[0] = 1;
			refinement_factors[0] = 1;
		}
		Option_table *option_table = CREATE(Option_table)();
		Option_table_add_help(option_table,
			"Defines tessellation objects which control how finite elements are "
			"subdivided into graphics. The minimum_divisions option gives the "
			"minimum number of linear segments approximating geometry in each xi "
			"dimension of the element. If the coordinate field of a graphics uses "
			"non-linear basis functions the minimum_divisions is multiplied by "
			"the refinement_factors to give the refined number of segments. "
			"Both minimum_divisions and refinement_factors use the last supplied "
			"number for all higher dimensions, so \"4\" = \"4*4\" and so on. "
			"The circle_divisions sets the number of line segments used to "
			"approximate circles in cylinders, spheres etc.");
		Option_table_add_entry(option_table, "circle_divisions",
			(void *)&circleDivisions, (void *)NULL, set_circle_divisions);
		Option_table_add_divisions_entry(option_table, "minimum_divisions",
			&minimum_divisions, &minimum_divisions_size);
		Option_table_add_divisions_entry(option_table, "refinement_factors",
			&refinement_factors, &refinement_factors_size);
		return_code = Option_table_multi_parse(option_table,state);
		DESTROY(Option_table)(&option_table);
		if (return_code && tessellation)
		{
			return_code =
				cmzn_tessellation_set_minimum_divisions(tessellation, minimum_divisions_size, minimum_divisions) &&
				cmzn_tessellation_set_refinement_factors(tessellation, refinement_factors_size, refinement_factors) &&
				((circleDivisions < 3) || cmzn_tessellation_set_circle_divisions(tessellation, circleDivisions));
		}
		DEALLOCATE(minimum_divisions);
		DEALLOCATE(refinement_factors);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_define_tessellation_contents.  Invalid arguments");
		return_code = 0;
	}
	return return_code;
}

/***************************************************************************//**
 * @see Option_table_add_cmzn_tessellation_entry
 */
int set_cmzn_tessellation(struct Parse_state *state,
	void *tessellation_address_void, void *tessellationmodule_void)
{
	int return_code = 1;

	cmzn_tessellation **tessellation_address = (cmzn_tessellation **)tessellation_address_void;
	cmzn_tessellationmodule *tessellationmodule =
		reinterpret_cast<cmzn_tessellationmodule *>(tessellationmodule_void);
	if (state && tessellation_address && tessellationmodule)
	{
		const char *current_token = state->current_token;
		if (current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				cmzn_tessellation *tessellation = NULL;
				if (!fuzzy_string_compare(current_token, "NONE"))
				{
					tessellation = cmzn_tessellationmodule_find_tessellation_by_name(tessellationmodule, current_token);
					if (!tessellation)
					{
						display_message(ERROR_MESSAGE, "Unknown tessellation : %s", current_token);
						display_parse_state_location(state);
						return_code = 0;
					}
				}
				if (return_code)
				{
					REACCESS(cmzn_tessellation)(tessellation_address, tessellation);
					shift_Parse_state(state,1);
				}
				if (tessellation)
					cmzn_tessellation_destroy(&tessellation);
			}
			else
			{
				char *name = cmzn_tessellation_get_name(*tessellation_address);
				display_message(INFORMATION_MESSAGE," TESSELLATION_NAME|none[%s]",
					(*tessellation_address) ? name : "none");
				cmzn_deallocate(name);
			}
		}
		else
		{
			display_message(WARNING_MESSAGE, "Missing tessellation name");
			display_parse_state_location(state);
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_cmzn_tessellation.  Invalid argument(s)");
		return_code = 0;
	}
	return (return_code);
}

int Option_table_add_cmzn_tessellation_entry(struct Option_table *option_table,
	const char *token, struct cmzn_tessellationmodule *tessellationmodule,
	struct cmzn_tessellation **tessellation_address)
{
	int return_code;
	if (option_table && token && tessellationmodule && tessellation_address)
	{
		return_code = Option_table_add_entry(option_table, token,
			(void *)tessellation_address, (void *)tessellationmodule,
			set_cmzn_tessellation);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Option_table_add_cmzn_tessellation_entry.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
}

int gfx_define_tessellation(struct Parse_state *state, void *dummy_to_be_modified,
	void *tessellationmodule_void)
{
	int return_code = 1;

	USE_PARAMETER(dummy_to_be_modified);
	cmzn_tessellationmodule *tessellationmodule =
		reinterpret_cast<cmzn_tessellationmodule *>(tessellationmodule_void);
	if (state && tessellationmodule)
	{
		const char *current_token = state->current_token;
		if (current_token)
		{
			if (strcmp(PARSER_HELP_STRING, current_token) &&
				strcmp(PARSER_RECURSIVE_HELP_STRING, current_token))
			{
				cmzn_tessellationmodule_begin_change(tessellationmodule);
				cmzn_tessellation *tessellation =
					cmzn_tessellationmodule_find_tessellation_by_name(tessellationmodule, current_token);
				if (!tessellation)
				{
					tessellation = cmzn_tessellationmodule_create_tessellation(tessellationmodule);
					cmzn_tessellation_set_name(tessellation, current_token);
				}
				shift_Parse_state(state,1);
				if (tessellation)
				{
					// set managed state for all tessellations created or edited otherwise
					// cleaned up at end of command.
					cmzn_tessellation_set_managed(tessellation, true);
					return_code = gfx_define_tessellation_contents(state, (void *)tessellation, tessellationmodule_void);
				}
				cmzn_tessellation_destroy(&tessellation);
				cmzn_tessellationmodule_end_change(tessellationmodule);
			}
			else
			{
				Option_table *option_table = CREATE(Option_table)();
				Option_table_add_entry(option_table, "TESSELLATION_NAME",
					/*tessellation*/(void *)NULL, tessellationmodule_void,
					gfx_define_tessellation_contents);
				return_code = Option_table_parse(option_table,state);
				DESTROY(Option_table)(&option_table);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE, "Missing tessellation name");
			display_parse_state_location(state);
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_define_tessellation.  Invalid arguments");
		return_code = 0;
	}
	return return_code;
}

int gfx_destroy_tessellation(struct Parse_state *state,
	void *dummy_to_be_modified, void *tessellationmodule_void)
{
	int return_code;
	USE_PARAMETER(dummy_to_be_modified);
	cmzn_tessellationmodule *tessellationmodule =
		reinterpret_cast<cmzn_tessellationmodule *>(tessellationmodule_void);
	if (state && tessellationmodule)
	{
		cmzn_tessellation *tessellation = NULL;
		Option_table *option_table = CREATE(Option_table)();
		Option_table_add_entry(option_table, /*token*/(const char *)NULL,
			(void *)&tessellation, tessellationmodule_void, set_cmzn_tessellation);
		return_code = Option_table_multi_parse(option_table,state);
		if (return_code)
		{
			if (tessellation)
			{
				cmzn_tessellation_set_managed(tessellation, false);
				//-- if (tessellation->access_count > 2)
				//-- {
				//-- 	display_message(INFORMATION_MESSAGE, "Tessellation marked for destruction when no longer in use.\n");
				//-- }
			}
			else
			{
				display_message(ERROR_MESSAGE, "Missing tessellation name");
				display_parse_state_location(state);
				return_code = 0;
			}
		}
		if (tessellation)
		{
			cmzn_tessellation_destroy(&tessellation);
		}
		DESTROY(Option_table)(&option_table);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_destroy_tessellation.  Invalid argument(s)");
		return_code = 0;
	}
	return (return_code);
}

int gfx_list_tessellation(struct Parse_state *state,
	void *dummy_to_be_modified, void *tessellationmodule_void)
{
	int return_code;
	USE_PARAMETER(dummy_to_be_modified);
	cmzn_tessellationmodule *tessellationmodule =
		reinterpret_cast<cmzn_tessellationmodule *>(tessellationmodule_void);
	if (state && tessellationmodule)
	{
		cmzn_tessellation *tessellation = NULL;
		Option_table *option_table = CREATE(Option_table)();
		Option_table_add_entry(option_table, /*token*/(const char *)NULL,
			(void *)&tessellation, tessellationmodule_void, set_cmzn_tessellation);
		return_code = Option_table_multi_parse(option_table,state);
		if (return_code)
		{
			if (tessellation)
			{
				return_code = list_cmzn_tessellation_iterator(tessellation, (void *)NULL);
			}
			else
			{
				return_code = FOR_EACH_OBJECT_IN_MANAGER(cmzn_tessellation)(list_cmzn_tessellation_iterator,
					(void *)NULL, cmzn_tessellationmodule_get_manager(tessellationmodule));
			}
		}
		if (tessellation)
		{
			cmzn_tessellation_destroy(&tessellation);
		}
		DESTROY(Option_table)(&option_table);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_list_tessellation.  Invalid argument(s)");
		return_code = 0;
	}
	return (return_code);
}

