/* OpenCMISS-Cmgui Application
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "opencmiss/zinc/region.h"
#include "opencmiss/zinc/scenefilter.h"
#include "general/debug.h"
#include "general/message.h"
#include "command/parser.h"
#include "graphics/graphics.h"
#include "graphics/scenefilter.hpp"
#include "graphics/scenefilter_app.hpp"

struct Define_graphics_filter_data
{
	cmzn_region *root_region;
	cmzn_scenefiltermodule *filter_module;
	int number_of_filters;
	cmzn_scenefilter **source_filters;
};

int set_cmzn_scenefilter_source_data(struct Parse_state *state,
	void *filter_data_void,void *dummy_void)
{
	int return_code = 1;
	struct Define_graphics_filter_data *filter_data = (struct Define_graphics_filter_data *)filter_data_void;
	const char *current_token;
	cmzn_scenefilter *filter = NULL, **temp_source_filters = NULL;
	cmzn_scenefiltermodule *filter_module = filter_data->filter_module;

	USE_PARAMETER(dummy_void);
	if (state && filter_data && filter_module)
	{
		current_token=state->current_token;
		if (current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				while (return_code && (current_token = state->current_token))
				{
					/* first try to find a number in the token */
					filter=cmzn_scenefiltermodule_find_scenefilter_by_name(filter_module, current_token);
					if (filter)
					{
						shift_Parse_state(state,1);
					}
					else
					{
						display_message(ERROR_MESSAGE,"Unknown filter: %s",current_token);
						display_parse_state_location(state);
						return_code=0;
					}
					if (return_code)
					{
						if (REALLOCATE(temp_source_filters, filter_data->source_filters,
							cmzn_scenefilter *, filter_data->number_of_filters+1))
						{
							filter_data->source_filters = temp_source_filters;
							temp_source_filters[filter_data->number_of_filters] =	filter;
							(filter_data->number_of_filters)++;
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"set_Computed_field_composite_source_data.  "
								"Not enough memory");
							return_code=0;
						}
					}
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE, "FILTER_NAMES");
				return_code = 1;
			}
		}
		else
		{
			display_message(WARNING_MESSAGE,
				"Missing component filters");
			display_parse_state_location(state);
			return_code=0;
		}
	}


	return return_code;
}

enum cmzn_scenefilter_type cmzn_scenefilter_get_type(cmzn_scenefilter_id graphics_filter)
{
	enum cmzn_scenefilter_type filter_type = CMZN_SCENEFILTER_TYPE_INVALID;
	if (graphics_filter)
	{
		filter_type = graphics_filter->getType();
	}
	return filter_type;
}

int gfx_define_graphics_filter_operator_or(struct Parse_state *state, void *graphics_filter_handle_void,
	void *filter_data_void)
{
	int return_code = 1, add_filter = 1;
	enum cmzn_scenefilter_type filter_type;
	struct Define_graphics_filter_data *filter_data = (struct Define_graphics_filter_data *)filter_data_void;
	if (state && filter_data)
	{
		cmzn_scenefilter_id *graphics_filter_handle = (cmzn_scenefilter_id *)graphics_filter_handle_void; // can be null
		cmzn_scenefilter_id graphics_filter = *graphics_filter_handle;
		if (graphics_filter)
		{
			filter_type = cmzn_scenefilter_get_type(graphics_filter);
		}
		else
		{
			filter_type = CMZN_SCENEFILTER_TYPE_OPERATOR_OR;
		}
		struct Option_table *option_table = CREATE(Option_table)();
		Option_table_add_help(option_table," define an operator_or filter, multiple filters defined earlier "
			"can be added or removed. This filter will perform a boolean \"or\" check on the filters provided. "
			"Graphics that match any of the filters will be shown.");
		Option_table_add_switch(option_table,"add_filters","remove_filters",&add_filter);
		Option_table_add_entry(option_table, NULL, filter_data,
			NULL, set_cmzn_scenefilter_source_data);
		return_code = Option_table_multi_parse(option_table, state);
		if (return_code && (filter_type == CMZN_SCENEFILTER_TYPE_OPERATOR_OR))
		{
			if (!graphics_filter)
			{
				graphics_filter = cmzn_scenefiltermodule_create_scenefilter_operator_or(filter_data->filter_module);
				*graphics_filter_handle = graphics_filter;
			}
			cmzn_scenefilter_operator_id operator_filter = cmzn_scenefilter_cast_operator(graphics_filter);
			if (operator_filter)
			{
				for (int i = 0; i < filter_data->number_of_filters; i++)
				{
					if (add_filter)
					{
						cmzn_scenefilter_operator_append_operand(operator_filter, filter_data->source_filters[i]);
					}
					else
					{
						cmzn_scenefilter_operator_remove_operand(operator_filter, filter_data->source_filters[i]);
					}
				}
				cmzn_scenefilter_operator_destroy(&operator_filter);
			}
			else
			{
				return_code = 0;
			}
		}
		else if (return_code)
		{
			display_message(ERROR_MESSAGE,
				"gfx define graphics_filter operator_or:  Cannot change filter type.");
			return_code = 0;
		}
		DESTROY(Option_table)(&option_table);
	}

	return return_code;
}

int gfx_define_graphics_filter_operator_and(struct Parse_state *state, void *graphics_filter_handle_void,
	void *filter_data_void)
{
	int return_code = 1, add_filter = 1;
	enum cmzn_scenefilter_type filter_type;
	struct Define_graphics_filter_data *filter_data = (struct Define_graphics_filter_data *)filter_data_void;
	if (state && filter_data)
	{
		cmzn_scenefilter_id *graphics_filter_handle = (cmzn_scenefilter_id *)graphics_filter_handle_void; // can be null
		cmzn_scenefilter_id graphics_filter = *graphics_filter_handle;
		if (graphics_filter)
		{
			filter_type = cmzn_scenefilter_get_type(graphics_filter);
		}
		else
		{
			filter_type = CMZN_SCENEFILTER_TYPE_OPERATOR_AND;
		}
		struct Option_table *option_table = CREATE(Option_table)();
		Option_table_add_help(option_table," define an operator_and filter, multiple filters defined earlier "
			"can be added or removed. This filter will perform a boolean \"and\" check on the filters provided. "
			"Only graphics that match all of the filters will be shown.");
		Option_table_add_switch(option_table,"add_filters","remove_filters",&add_filter);
		Option_table_add_entry(option_table, NULL, filter_data,
			NULL, set_cmzn_scenefilter_source_data);
		return_code = Option_table_multi_parse(option_table, state);
		if (return_code && (filter_type == CMZN_SCENEFILTER_TYPE_OPERATOR_AND))
		{
			if (!graphics_filter)
			{
				graphics_filter = cmzn_scenefiltermodule_create_scenefilter_operator_and(filter_data->filter_module);
				*graphics_filter_handle = graphics_filter;
			}
			cmzn_scenefilter_operator_id operator_filter = cmzn_scenefilter_cast_operator(graphics_filter);
			if (operator_filter)
			{
				for (int i = 0; i < filter_data->number_of_filters; i++)
				{
					if (add_filter)
					{
						cmzn_scenefilter_operator_append_operand(operator_filter, filter_data->source_filters[i]);
					}
					else
					{
						cmzn_scenefilter_operator_remove_operand(operator_filter, filter_data->source_filters[i]);
					}
				}
				cmzn_scenefilter_operator_destroy(&operator_filter);
			}
			else
			{
				return_code = 0;
			}
		}
		else if (return_code)
		{
			display_message(ERROR_MESSAGE,
				"gfx define graphics_filter operator_and:  Cannot change filter type.");
			return_code = 0;
		}
		DESTROY(Option_table)(&option_table);
	}

	return return_code;
}

int gfx_define_graphics_filter_contents(struct Parse_state *state, void *graphics_filter_handle_void,
	void *filter_data_void)
{
	int return_code = 1, number_of_valid_strings = 0;
	struct Define_graphics_filter_data *filter_data = (struct Define_graphics_filter_data *)filter_data_void;
	char *match_graphics_name, match_visibility_flags, *match_region_path;
	enum cmzn_graphics_type graphics_type;
	const char **valid_strings = NULL, *graphics_type_string = NULL;
	if (state && filter_data)
	{
		cmzn_scenefilter_id *graphics_filter_handle = (cmzn_scenefilter_id *)graphics_filter_handle_void; // can be null
		cmzn_scenefilter_id graphics_filter = *graphics_filter_handle;
		match_graphics_name = NULL;
		match_visibility_flags = 0;
		match_region_path = NULL;
		graphics_type = CMZN_GRAPHICS_TYPE_INVALID;
		int inverse = 0;
		if (graphics_filter)
		{
			inverse = static_cast<int>(graphics_filter->isInverse());
		}
		valid_strings = ENUMERATOR_GET_VALID_STRINGS(cmzn_graphics_type)(
			&number_of_valid_strings,
			(ENUMERATOR_CONDITIONAL_FUNCTION(cmzn_graphics_type) *)NULL, (void *)NULL);

		struct Option_table *option_table = CREATE(Option_table)();
		Option_table_add_help(option_table," Filter to set up what will be and "
				"what will not be included in a scene. The optional inverse_match "
				"flag will invert the filter's match criterion. The behaviour is to show matching graphics "
				"with the matching criteria. <match_graphics_name> filters graphics with the matching name. "
				"<match_visibility_flags> filters graphics with the setting on the visibility flag. "
				"<match_region_path> filters graphics in the specified region or its subregion. "
				"<operator_or> filters the scene using the logical operation 'or' on a collective of filters. "
				"<operator_and> filters the scene using the logical operation 'and' on a collective of filters. "
				"Filters created earlier can be added or removed from the <operator_or> and <operator_and> filter.");

		Option_table_add_entry(option_table, "operator_or", graphics_filter_handle_void, filter_data_void,
			gfx_define_graphics_filter_operator_or);
		Option_table_add_entry(option_table, "operator_and", graphics_filter_handle_void, filter_data_void,
			gfx_define_graphics_filter_operator_and);
		Option_table_add_enumerator(option_table,number_of_valid_strings,
			valid_strings,&(graphics_type_string));
		DEALLOCATE(valid_strings);
		Option_table_add_string_entry(option_table, "match_graphics_name",
			&(match_graphics_name), " MATCH_NAME");
		Option_table_add_char_flag_entry(option_table, "match_visibility_flags",
			&(match_visibility_flags));
		Option_table_add_string_entry(option_table, "match_region_path",
			&(match_region_path), " REGION_PATH");
		Option_table_add_switch(option_table, "inverse_match", "normal_match",
			&(inverse));
		return_code = Option_table_multi_parse(option_table, state);
		if (return_code)
		{
			int number_of_match_criteria = match_visibility_flags +
				(NULL != match_region_path) +	(NULL != match_graphics_name);
			if (1 < number_of_match_criteria)
			{
				display_message(ERROR_MESSAGE,
					"Only one match criterion can be specified per filter.");
				display_parse_state_location(state);
				return_code = 0;
			}
		}
		DESTROY(Option_table)(&option_table);
		if (return_code)
		{
			if (!graphics_filter)
			{
				if (match_visibility_flags)
				{
					graphics_filter = cmzn_scenefiltermodule_create_scenefilter_visibility_flags(
						filter_data->filter_module);
				}
				else if (match_graphics_name)
				{
					graphics_filter = cmzn_scenefiltermodule_create_scenefilter_graphics_name(
						filter_data->filter_module, match_graphics_name);
				}
				else if (match_region_path)
				{
					cmzn_region *match_region = cmzn_region_find_subregion_at_path(
						filter_data->root_region, match_region_path);
					if (match_region)
					{
						graphics_filter = cmzn_scenefiltermodule_create_scenefilter_region(
							filter_data->filter_module, match_region);
						cmzn_region_destroy(&match_region);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Cannot create filter region.  %s is not in region tree",
							match_region_path);
					}
				}
				else if (graphics_type_string)
				{
					STRING_TO_ENUMERATOR(cmzn_graphics_type)(graphics_type_string, &graphics_type);
					graphics_filter = cmzn_scenefiltermodule_create_scenefilter_graphics_type(
						filter_data->filter_module, graphics_type);
				}
			}

			if (graphics_filter)
			{
				cmzn_scenefilter_set_inverse(graphics_filter, 0 != inverse);
				*graphics_filter_handle = graphics_filter;
			}
		}
		if (match_graphics_name)
			DEALLOCATE(match_graphics_name);
		if (match_region_path)
			DEALLOCATE(match_region_path);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_define_graphics_filter_contents.  Invalid arguments");
		return_code = 0;
	}
	return return_code;
}

int gfx_define_graphics_filter(struct Parse_state *state, void *root_region_void,
	void *filter_module_void)
{
	int return_code = 1;
	cmzn_scenefiltermodule *filter_module =
		(cmzn_scenefiltermodule *)filter_module_void;
	cmzn_region *root_region = (cmzn_region *)root_region_void;
	if (state && filter_module && root_region)
	{
		const char *current_token = state->current_token;
		if (current_token)
		{
			cmzn_scenefilter *graphics_filter = NULL;
			struct Define_graphics_filter_data filter_data;
			filter_data.filter_module = filter_module;
			filter_data.root_region = root_region;
			filter_data.number_of_filters = 0;
			filter_data.source_filters = NULL;
			if (strcmp(PARSER_HELP_STRING, current_token) &&
				strcmp(PARSER_RECURSIVE_HELP_STRING, current_token))
			{
				cmzn_scenefiltermodule_begin_change(filter_module);
				graphics_filter = cmzn_scenefiltermodule_find_scenefilter_by_name(filter_module, current_token);
				bool existing_filter = (graphics_filter != 0);
				shift_Parse_state(state,1);
				return_code = gfx_define_graphics_filter_contents(state, (void *)&graphics_filter, (void*)&filter_data);
				if (return_code)
				{
					cmzn_scenefilter_set_managed(graphics_filter, true);
					if (!existing_filter)
						cmzn_scenefilter_set_name(graphics_filter, current_token);
				}
				cmzn_scenefiltermodule_end_change(filter_module);
			}
			else
			{
				Option_table *option_table = CREATE(Option_table)();
				Option_table_add_entry(option_table, "GRAPHICS_FILTER_NAME",
					/*graphics_filter*/(void *)&graphics_filter, (void*)&filter_data,
					gfx_define_graphics_filter_contents);
				return_code = Option_table_parse(option_table,state);
				DESTROY(Option_table)(&option_table);
			}
			if (filter_data.source_filters)
			{
				for (int i = 0; i < filter_data.number_of_filters; i++)
				{
					cmzn_scenefilter_destroy(&filter_data.source_filters[i]);
				}
				DEALLOCATE(filter_data.source_filters);
			}
			if (graphics_filter)
				cmzn_scenefilter_destroy(&graphics_filter);
		}
		else
		{
			display_message(ERROR_MESSAGE, "Missing graphics_filter name");
			display_parse_state_location(state);
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_define_graphics_filter.  Invalid arguments");
		return_code = 0;
	}
	return return_code;
}

int gfx_list_graphics_filter(struct Parse_state *state, void *dummy_to_be_modified,
	void *filter_module_void)
{
	USE_PARAMETER(dummy_to_be_modified);
	int return_code = 1;
	cmzn_scenefiltermodule_id filter_module = (cmzn_scenefiltermodule_id)filter_module_void;
	if (state && filter_module)
	{
		char *filter_name = NULL;
		Option_table *option_table = CREATE(Option_table)();
		Option_table_add_default_string_entry(option_table,
			&filter_name, "GRAPHICS_FILTER_NAME[all]");
		return_code = Option_table_multi_parse(option_table,state);
		if (return_code)
		{
			if (filter_name)
			{
				cmzn_scenefilter *filter =
					cmzn_scenefiltermodule_find_scenefilter_by_name(filter_module, filter_name);
				if (filter)
				{
					filter->list("gfx define graphics_filter");
					cmzn_scenefilter_destroy(&filter);
				}
				else
				{
					display_message(ERROR_MESSAGE, "Unknown graphics_filter %s", filter_name);
					display_parse_state_location(state);
					return_code = 0;
				}
			}
			else
			{
				//-- MANAGER(cmzn_scenefilter) *manager =
				//-- 	cmzn_graphics_module_get_filter_manager(graphics_module);
				//-- cmzn_set_cmzn_scenefilter *filter_list =
				//-- 	reinterpret_cast<cmzn_set_cmzn_scenefilter *>(manager->object_list);
				// Note: doesn't list in dependency order
				//-- for (cmzn_set_cmzn_scenefilter::iterator iter = filter_list->begin();
				//-- 	iter != filter_list->end(); ++iter)
				{
					//-- (*iter)->list("gfx define graphics_filter");
					display_message(ERROR_MESSAGE, "filter manager list functionality not implemented!!");
				}
			}
		}
		DEALLOCATE(filter_name);
		DESTROY(Option_table)(&option_table);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_list_graphics_filter.  Invalid arguments");
		return_code = 0;
	}
	return return_code;
}

int set_cmzn_scenefilter(struct Parse_state *state,
	void *scenefilter_address_void, void *filter_module_void)
{
	int return_code = 1;

	cmzn_scenefilter **scenefilter_address = (cmzn_scenefilter **)scenefilter_address_void;
	cmzn_scenefiltermodule *filter_module = (cmzn_scenefiltermodule *)filter_module_void;
	if (state && scenefilter_address && filter_module)
	{
		const char *current_token = state->current_token;
		if (current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				cmzn_scenefilter *graphics_filter = NULL;
				if (!fuzzy_string_compare(current_token, "NONE"))
				{
					graphics_filter = cmzn_scenefiltermodule_find_scenefilter_by_name(filter_module, current_token);
					if (!graphics_filter)
					{
						display_message(ERROR_MESSAGE, "Unknown graphics_filter : %s", current_token);
						display_parse_state_location(state);
						return_code = 0;
					}
				}
				if (return_code)
				{
					REACCESS(cmzn_scenefilter)(scenefilter_address, graphics_filter);
					shift_Parse_state(state,1);
				}
				if (graphics_filter)
					cmzn_scenefilter_destroy(&graphics_filter);
			}
			else
			{
				display_message(INFORMATION_MESSAGE," GRAPHICS_FILTER_NAME|none[%s]",
					(*scenefilter_address) ? (*scenefilter_address)->name : "none");
			}
		}
		else
		{
			display_message(WARNING_MESSAGE, "Missing graphics_filter name");
			display_parse_state_location(state);
			return_code = 0;
		}
	}
	else
	{
		return_code = 0;
	}
	return (return_code);
}

