
#include "zinc/graphicsfilter.h"
#include "zinc/region.h"
#include "general/debug.h"
#include "general/message.h"
#include "command/parser.h"
#include "graphics/graphic.h"
#include "graphics/graphics_filter.hpp"

struct Define_graphics_filter_data
{
	cmzn_region *root_region;
	cmzn_graphics_filter_module *filter_module;
	int number_of_filters;
	cmzn_graphics_filter **source_filters;
};

int set_cmzn_graphics_filter_source_data(struct Parse_state *state,
	void *filter_data_void,void *dummy_void)
{
	int return_code = 1;
	struct Define_graphics_filter_data *filter_data = (struct Define_graphics_filter_data *)filter_data_void;
	const char *current_token;
	cmzn_graphics_filter *filter = NULL, **temp_source_filters = NULL;
	cmzn_graphics_filter_module *filter_module = filter_data->filter_module;

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
					filter=cmzn_graphics_filter_module_find_filter_by_name(filter_module, current_token);
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
							cmzn_graphics_filter *, filter_data->number_of_filters+1))
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

enum cmzn_graphics_filter_type cmzn_graphics_filter_get_type(cmzn_graphics_filter_id graphics_filter)
{
	enum cmzn_graphics_filter_type filter_type = CMZN_GRAPHICS_FILTER_TYPE_INVALID;
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
	enum cmzn_graphics_filter_type filter_type;
	struct Define_graphics_filter_data *filter_data = (struct Define_graphics_filter_data *)filter_data_void;
	if (state && filter_data)
	{
		cmzn_graphics_filter_id *graphics_filter_handle = (cmzn_graphics_filter_id *)graphics_filter_handle_void; // can be null
		cmzn_graphics_filter_id graphics_filter = *graphics_filter_handle;
		if (graphics_filter)
		{
			filter_type = cmzn_graphics_filter_get_type(graphics_filter);
		}
		else
		{
			filter_type = CMZN_GRAPHICS_FILTER_TYPE_OPERATOR_OR;
		}
		struct Option_table *option_table = CREATE(Option_table)();
		Option_table_add_help(option_table," define an operator_or filter, multiple filters defined earlier "
			"can be added or removed. This filter will perform a boolean \"or\" check on the filters provided. "
			"Graphics that match any of the filters will be shown.");
		Option_table_add_switch(option_table,"add_filters","remove_filters",&add_filter);
		Option_table_add_entry(option_table, NULL, filter_data,
			NULL, set_cmzn_graphics_filter_source_data);
		return_code = Option_table_multi_parse(option_table, state);
		if (return_code && (filter_type == CMZN_GRAPHICS_FILTER_TYPE_OPERATOR_OR))
		{
			if (!graphics_filter)
			{
				graphics_filter = cmzn_graphics_filter_module_create_filter_operator_or(filter_data->filter_module);
				*graphics_filter_handle = graphics_filter;
			}
			cmzn_graphics_filter_operator_id operator_filter = cmzn_graphics_filter_cast_operator(graphics_filter);
			if (operator_filter)
			{
				for (int i = 0; i < filter_data->number_of_filters; i++)
				{
					if (add_filter)
					{
						cmzn_graphics_filter_operator_append_operand(operator_filter, filter_data->source_filters[i]);
					}
					else
					{
						cmzn_graphics_filter_operator_remove_operand(operator_filter, filter_data->source_filters[i]);
					}
				}
				cmzn_graphics_filter_operator_destroy(&operator_filter);
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
	enum cmzn_graphics_filter_type filter_type;
	struct Define_graphics_filter_data *filter_data = (struct Define_graphics_filter_data *)filter_data_void;
	if (state && filter_data)
	{
		cmzn_graphics_filter_id *graphics_filter_handle = (cmzn_graphics_filter_id *)graphics_filter_handle_void; // can be null
		cmzn_graphics_filter_id graphics_filter = *graphics_filter_handle;
		if (graphics_filter)
		{
			filter_type = cmzn_graphics_filter_get_type(graphics_filter);
		}
		else
		{
			filter_type = CMZN_GRAPHICS_FILTER_TYPE_OPERATOR_AND;
		}
		struct Option_table *option_table = CREATE(Option_table)();
		Option_table_add_help(option_table," define an operator_and filter, multiple filters defined earlier "
			"can be added or removed. This filter will perform a boolean \"and\" check on the filters provided. "
			"Only graphics that match all of the filters will be shown.");
		Option_table_add_switch(option_table,"add_filters","remove_filters",&add_filter);
		Option_table_add_entry(option_table, NULL, filter_data,
			NULL, set_cmzn_graphics_filter_source_data);
		return_code = Option_table_multi_parse(option_table, state);
		if (return_code && (filter_type == CMZN_GRAPHICS_FILTER_TYPE_OPERATOR_AND))
		{
			if (!graphics_filter)
			{
				graphics_filter = cmzn_graphics_filter_module_create_filter_operator_and(filter_data->filter_module);
				*graphics_filter_handle = graphics_filter;
			}
			cmzn_graphics_filter_operator_id operator_filter = cmzn_graphics_filter_cast_operator(graphics_filter);
			if (operator_filter)
			{
				for (int i = 0; i < filter_data->number_of_filters; i++)
				{
					if (add_filter)
					{
						cmzn_graphics_filter_operator_append_operand(operator_filter, filter_data->source_filters[i]);
					}
					else
					{
						cmzn_graphics_filter_operator_remove_operand(operator_filter, filter_data->source_filters[i]);
					}
				}
				cmzn_graphics_filter_operator_destroy(&operator_filter);
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
	char *match_graphic_name, match_visibility_flags, *match_region_path;
	enum cmzn_graphic_type graphic_type;
	const char **valid_strings = NULL, *graphic_type_string = NULL;
	if (state && filter_data)
	{
		cmzn_graphics_filter_id *graphics_filter_handle = (cmzn_graphics_filter_id *)graphics_filter_handle_void; // can be null
		cmzn_graphics_filter_id graphics_filter = *graphics_filter_handle;
		match_graphic_name = NULL;
		match_visibility_flags = 0;
		match_region_path = NULL;
		graphic_type = CMZN_GRAPHIC_TYPE_INVALID;
		int inverse = 0;
		if (graphics_filter)
		{
			inverse = static_cast<int>(graphics_filter->isInverse());
		}
		valid_strings = ENUMERATOR_GET_VALID_STRINGS(cmzn_graphic_type)(
			&number_of_valid_strings,
			(ENUMERATOR_CONDITIONAL_FUNCTION(cmzn_graphic_type) *)NULL, (void *)NULL);

		struct Option_table *option_table = CREATE(Option_table)();
		Option_table_add_help(option_table," Filter to set up what will be and "
				"what will not be included in a scene. The optional inverse_match "
				"flag will invert the filter's match criterion. The behaviour is to show matching graphic "
				"with the matching criteria. <match_graphic_name> filters graphic with the matching name. "
				"<match_visibility_flags> filters graphic with the setting on the visibility flag. "
				"<match_region_path> filters graphic in the specified region or its subregion. "
				"<operator_or> filters the scene using the logical operation 'or' on a collective of filters. "
				"<operator_and> filters the scene using the logical operation 'and' on a collective of filters. "
				"Filters created earlier can be added or removed from the <operator_or> and <operator_and> filter.");

		Option_table_add_entry(option_table, "operator_or", graphics_filter_handle_void, filter_data_void,
			gfx_define_graphics_filter_operator_or);
		Option_table_add_entry(option_table, "operator_and", graphics_filter_handle_void, filter_data_void,
			gfx_define_graphics_filter_operator_and);
		Option_table_add_enumerator(option_table,number_of_valid_strings,
			valid_strings,&(graphic_type_string));
		DEALLOCATE(valid_strings);
		Option_table_add_string_entry(option_table, "match_graphic_name",
			&(match_graphic_name), " MATCH_NAME");
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
				(NULL != match_region_path) +	(NULL != match_graphic_name);
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
					graphics_filter = cmzn_graphics_filter_module_create_filter_visibility_flags(
						filter_data->filter_module);
				}
				else if (match_graphic_name)
				{
					graphics_filter = cmzn_graphics_filter_module_create_filter_graphic_name(
						filter_data->filter_module, match_graphic_name);
				}
				else if (match_region_path)
				{
					cmzn_region *match_region = cmzn_region_find_subregion_at_path(
						filter_data->root_region, match_region_path);
					if (match_region)
					{
						graphics_filter = cmzn_graphics_filter_module_create_filter_region(
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
				else if (graphic_type_string)
				{
					STRING_TO_ENUMERATOR(cmzn_graphic_type)(graphic_type_string, &graphic_type);
					graphics_filter = cmzn_graphics_filter_module_create_filter_graphic_type(
						filter_data->filter_module, graphic_type);
				}
			}

			if (graphics_filter)
			{
				cmzn_graphics_filter_set_inverse(graphics_filter, 0 != inverse);
				*graphics_filter_handle = graphics_filter;
			}
		}
		if (match_graphic_name)
			DEALLOCATE(match_graphic_name);
		if (match_region_path)
			DEALLOCATE(match_region_path);
		if (graphic_type_string)
			DEALLOCATE(graphic_type_string);
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
	cmzn_graphics_filter_module *filter_module =
		(cmzn_graphics_filter_module *)filter_module_void;
	cmzn_region *root_region = (cmzn_region *)root_region_void;
	if (state && filter_module && root_region)
	{
		const char *current_token = state->current_token;
		if (current_token)
		{
			cmzn_graphics_filter *graphics_filter = NULL;
			struct Define_graphics_filter_data filter_data;
			filter_data.filter_module = filter_module;
			filter_data.root_region = root_region;
			filter_data.number_of_filters = 0;
			filter_data.source_filters = NULL;
			if (strcmp(PARSER_HELP_STRING, current_token) &&
				strcmp(PARSER_RECURSIVE_HELP_STRING, current_token))
			{
				cmzn_graphics_filter_module_begin_change(filter_module);
				graphics_filter = cmzn_graphics_filter_module_find_filter_by_name(filter_module, current_token);
				bool existing_filter = (graphics_filter != 0);
				shift_Parse_state(state,1);
				return_code = gfx_define_graphics_filter_contents(state, (void *)&graphics_filter, (void*)&filter_data);
				if (return_code)
				{
					cmzn_graphics_filter_set_managed(graphics_filter, true);
					if (!existing_filter)
						cmzn_graphics_filter_set_name(graphics_filter, current_token);
				}
				cmzn_graphics_filter_module_end_change(filter_module);
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
					cmzn_graphics_filter_destroy(&filter_data.source_filters[i]);
				}
				DEALLOCATE(filter_data.source_filters);
			}
			if (graphics_filter)
				cmzn_graphics_filter_destroy(&graphics_filter);
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
	cmzn_graphics_filter_module_id filter_module = (cmzn_graphics_filter_module_id)filter_module_void;
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
				cmzn_graphics_filter *filter =
					cmzn_graphics_filter_module_find_filter_by_name(filter_module, filter_name);
				if (filter)
				{
					filter->list("gfx define graphics_filter");
					cmzn_graphics_filter_destroy(&filter);
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
				//-- MANAGER(cmzn_graphics_filter) *manager =
				//-- 	cmzn_graphics_module_get_filter_manager(graphics_module);
				//-- cmzn_set_cmzn_graphics_filter *filter_list =
				//-- 	reinterpret_cast<cmzn_set_cmzn_graphics_filter *>(manager->object_list);
				// Note: doesn't list in dependency order
				//-- for (cmzn_set_cmzn_graphics_filter::iterator iter = filter_list->begin();
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

int set_cmzn_graphics_filter(struct Parse_state *state,
	void *graphics_filter_address_void, void *filter_module_void)
{
	int return_code = 1;

	cmzn_graphics_filter **graphics_filter_address = (cmzn_graphics_filter **)graphics_filter_address_void;
	cmzn_graphics_filter_module *filter_module = (cmzn_graphics_filter_module *)filter_module_void;
	if (state && graphics_filter_address && filter_module)
	{
		const char *current_token = state->current_token;
		if (current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				cmzn_graphics_filter *graphics_filter = NULL;
				if (!fuzzy_string_compare(current_token, "NONE"))
				{
					graphics_filter = cmzn_graphics_filter_module_find_filter_by_name(filter_module, current_token);
					if (!graphics_filter)
					{
						display_message(ERROR_MESSAGE, "Unknown graphics_filter : %s", current_token);
						display_parse_state_location(state);
						return_code = 0;
					}
				}
				if (return_code)
				{
					REACCESS(cmzn_graphics_filter)(graphics_filter_address, graphics_filter);
					shift_Parse_state(state,1);
				}
				if (graphics_filter)
					cmzn_graphics_filter_destroy(&graphics_filter);
			}
			else
			{
				display_message(INFORMATION_MESSAGE," GRAPHICS_FILTER_NAME|none[%s]",
					(*graphics_filter_address) ? (*graphics_filter_address)->name : "none");
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

