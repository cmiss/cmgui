/* OpenCMISS-Cmgui Application
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "opencmiss/zinc/region.h"
#include "opencmiss/zinc/streamregion.h"
#include "general/message.h"
#include "general/mystring.h"
#include "general/debug.h"
#include "general/object.h"
#include "command/parser.h"
#include "region/cmiss_region.h"
#include "region/cmiss_region_app.h"
#include "stream/region_stream.hpp"

int set_cmzn_region(struct Parse_state *state, void *region_address_void,
	void *root_region_void)
{
	const char *current_token;
	int return_code;
	struct cmzn_region *region, **region_address, *root_region;

	ENTER(set_cmzn_region);
	if (state && (root_region = static_cast<struct cmzn_region *>(root_region_void)) &&
		(region_address = static_cast<struct cmzn_region **>(region_address_void)))
	{
		if ((current_token = state->current_token))
		{
			if (!Parse_state_help_mode(state))
			{
				region = cmzn_region_find_subregion_at_path(root_region, current_token);
				if (region)
				{
					cmzn_region_destroy(region_address);
					*region_address = region;
					return_code = shift_Parse_state(state, 1);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"set_cmzn_region:  Could not find subregion %s", current_token);
					display_parse_state_location(state);
					return_code = 0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE," PATH_TO_REGION");
				if (*region_address)
				{
					char *path = cmzn_region_get_path(*region_address);
					display_message(INFORMATION_MESSAGE, "[%s]", path);
					DEALLOCATE(path);
				}
				return_code = 1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE, "Missing region path");
			display_parse_state_location(state);
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE, "set_cmzn_region.  Missing state");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
}

int Option_table_add_set_cmzn_region(struct Option_table *option_table,
	const char *token, struct cmzn_region *root_region,
	struct cmzn_region **region_address)
{
	int return_code;

	ENTER(Option_table_add_set_cmzn_region);
	if (option_table && root_region && region_address)
	{
		return_code = Option_table_add_entry(option_table, token,
			(void *)region_address, (void *)root_region, set_cmzn_region);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Option_table_add_set_cmzn_region.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
}

int set_cmzn_region_path(struct Parse_state *state, void *path_address_void,
	void *root_region_void)
/*******************************************************************************
LAST MODIFIED : 13 January 2003

DESCRIPTION :
Modifier function for entering a path to a cmzn_region, starting at
<root_region>.
==============================================================================*/
{
	const char *current_token;
	char **path_address;
	int return_code;
	struct cmzn_region *region, *root_region;

	ENTER(set_cmzn_region_path);
	if (state && (root_region = (struct cmzn_region *)root_region_void))
	{
		if ((current_token = state->current_token))
		{
			if (!Parse_state_help_mode(state))
			{
				region = cmzn_region_find_subregion_at_path(
					root_region, current_token);
				if (region)
				{
					if ((path_address = (char **)path_address_void))
					{
						if (*path_address)
						{
							DEALLOCATE(*path_address);
						}
						if ((*path_address = duplicate_string(current_token)))
						{
							return_code = shift_Parse_state(state, 1);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"set_cmzn_region_path.  Could not allocate memory for path");
							return_code = 0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"set_cmzn_region_path.  Missing path_address");
						return_code = 0;
					}
					DEACCESS(cmzn_region)(&region);
				}
				else
				{
					display_message(ERROR_MESSAGE, "Invalid region path: %s",
						current_token);
					display_parse_state_location(state);
					return_code = 0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE," PATH_TO_REGION");
				if ((path_address = (char **)path_address_void) && (*path_address))
				{
					display_message(INFORMATION_MESSAGE, "[%s]", *path_address);
				}
				return_code = 1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE, "Missing region path");
			display_parse_state_location(state);
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE, "set_cmzn_region_path.  Missing state");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* set_cmzn_region_path */

int Option_table_add_set_cmzn_region_path(struct Option_table *option_table,
	const char *entry_string, struct cmzn_region *root_region, char **path_address)
/*******************************************************************************
LAST MODIFIED : 13 March 2003

DESCRIPTION :
Adds an entry to the <option_table> with name <entry_name> that returns a
region path in <path_address> relative to the <root_region>.
==============================================================================*/
{
	int return_code = 0;

	ENTER(Option_table_add_set_cmzn_region_path);
	if (option_table && entry_string && root_region && path_address)
	{
		return_code = Option_table_add_entry(option_table, entry_string,
			(void *)path_address, (void *)root_region,
			set_cmzn_region_path);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Option_table_add_set_cmzn_region_path.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* Option_table_add_set_cmzn_region_path */



/***************************************************************************//**
 * Modifier function to set the region, region path and field name.
 * Fields must not be the same name as a child region.
 *
 * Examples:
 *   /heart/coordinates = region path and field name
 *   heart              = region path only
 *   coordinates        = field name only
 * @param region_path_and_name a struct cmzn_region_path_and_name which if
 *   set contains an ACCESSed region and allocated path and name which caller
 *   is required to clean up. Name may be NULL if path is fully resolved.
 */
static int set_region_path_and_or_field_name(struct Parse_state *state,
	void *region_path_and_name_void, void *root_region_void)
{
	const char *current_token;
	int return_code;
	struct cmzn_region_path_and_name *name_data;
	struct cmzn_region *root_region;

	ENTER(set_region_path_and_or_field_name);
	if (state && (name_data = (struct cmzn_region_path_and_name *)region_path_and_name_void) &&
		(root_region = (struct cmzn_region *)root_region_void))
	{
		current_token = state->current_token;
		if (!current_token)
		{
			display_message(WARNING_MESSAGE, "Missing region path and/or field name");
			display_parse_state_location(state);
			return_code = 0;
		}
		else if (Parse_state_help_mode(state))
		{
			display_message(INFORMATION_MESSAGE, " REGION_PATH|REGION_PATH/FIELD_NAME|FIELD_NAME");
			return_code = 1;
		}
		else if (cmzn_region_get_partial_region_path(root_region, current_token,
			&name_data->region, &name_data->region_path, &name_data->name))
		{
			ACCESS(cmzn_region)(name_data->region);
			if (!name_data->name || (NULL == strchr(name_data->name, CMZN_REGION_PATH_SEPARATOR_CHAR)))
			{
				return_code = shift_Parse_state(state, 1);
			}
			else
			{
				display_message(ERROR_MESSAGE, "Bad region path and/or field name: %s",current_token);
				display_parse_state_location(state);
				return_code = 0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"set_region_path_and_or_field_name.  Failed to get path and name");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_region_path_and_or_field_name.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
}

int Option_table_add_region_path_and_or_field_name_entry(
	struct Option_table *option_table, char *token,
	struct cmzn_region_path_and_name *region_path_and_name,
	struct cmzn_region *root_region)
{
	int return_code;

	ENTER(Option_table_add_region_path_and_or_field_name_entry);
	if (option_table && region_path_and_name && root_region)
	{
		return_code = Option_table_add_entry(option_table, token,
			(void *)region_path_and_name, (void *)root_region,
			set_region_path_and_or_field_name);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Option_table_add_region_path_and_or_field_name_entry.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Option_table_add_region_path_and_or_field_name_entry */

int cmzn_region_path_to_subregion_and_group(cmzn_region_id root_region,
	const char *path, cmzn_region_id *region_address,
	cmzn_field_group_id *group_address)
{
	int return_code = CMZN_OK;
	if (root_region && path && path[0] && region_address && group_address)
	{
		char *region_path = 0;
		char *field_name = 0;
		cmzn_region_id output_region = 0;
		if (cmzn_region_get_partial_region_path(root_region, path,
			&output_region, &region_path, &field_name) && output_region)
		{
			cmzn_region_access(output_region);
			cmzn_region_destroy(region_address);
			*region_address = output_region;
			// release handle to old group later in case reusing
			cmzn_field_group_id old_group = *group_address;
			*group_address = 0;
			if (field_name)
			{
				cmzn_field *field = FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field,name)(
					field_name, cmzn_region_get_Computed_field_manager(output_region));
				*group_address = cmzn_field_cast_group(field);
				if (0 == *group_address)
					return_code = CMZN_ERROR_NOT_FOUND;
			}
			cmzn_field_group_destroy(&old_group);
		}
		else
			return_code = CMZN_ERROR_NOT_FOUND;
		DEALLOCATE(region_path);
		DEALLOCATE(field_name);
	}
	else
		return_code = CMZN_ERROR_ARGUMENT;
	return return_code;
}

int set_cmzn_region_or_group(struct Parse_state *state,
	void *region_address_void, void *group_address_void)
{
	cmzn_region_id *region_address = reinterpret_cast<cmzn_region_id *>(region_address_void);
	cmzn_field_group_id *group_address = reinterpret_cast<cmzn_field_group_id *>(group_address_void);
	if (!(state && region_address && *region_address && group_address))
	{
		display_message(ERROR_MESSAGE, "set_cmzn_region_or_group.  Invalid argument(s)");
		return 0;
	}
	const char *current_token = state->current_token;
	int return_code = 1;
	if (!current_token)
	{
		display_message(WARNING_MESSAGE, "Missing region/group path");
		display_parse_state_location(state);
		return_code =  0;
	}
	else if (Parse_state_help_mode(state))
		display_message(INFORMATION_MESSAGE, " REGION_PATH/GROUP");
	else
	{
		cmzn_region_id root_region = cmzn_region_get_root(*region_address);
		int result = cmzn_region_path_to_subregion_and_group(root_region,
			current_token, region_address, group_address);
		cmzn_region_destroy(&root_region);
		if (CMZN_OK == result)
			shift_Parse_state(state, 1);
		else
		{
			return_code = 0;
			if (CMZN_ERROR_NOT_FOUND == result)
			{
				display_message(ERROR_MESSAGE, "Invalid region/group path: %s", current_token);
				display_parse_state_location(state);
			}
		}
	}
	return return_code;
}

int Option_table_add_region_or_group_entry(struct Option_table *option_table,
	const char *token, cmzn_region_id *region_address,
	cmzn_field_group_id *group_address)
{
	if (!(option_table && region_address && *region_address && group_address))
	{
		display_message(ERROR_MESSAGE, "Option_table_add_region_or_group_entry.  Invalid argument(s)");
		return 0;
	}
	return Option_table_add_entry(option_table, token,
		(void *)region_address, (void *)group_address, set_cmzn_region_or_group);
}

int export_region_file_of_name(const char *file_name,
	struct cmzn_region *region, const char *group_name,
	struct cmzn_region *root_region,
	int write_elements, int write_nodes, int write_data,
	int number_of_field_names, char **field_names, FE_value time,
	enum cmzn_streaminformation_region_recursion_mode recursion_mode,
	int isFieldML)
{
	int return_code = 0;
	if (file_name && region && root_region)
	{
		cmzn_streaminformation_id si = cmzn_region_create_streaminformation_region(
			region);
		cmzn_streaminformation_region_id si_region = cmzn_streaminformation_cast_region(
			si);
		cmzn_streamresource_id sr = cmzn_streaminformation_create_streamresource_file(si, file_name);
		si_region->setRootRegion(root_region);
		cmzn_streaminformation_region_set_resource_recursion_mode(si_region, sr,
			recursion_mode);
		cmzn_field_domain_types domain_types = write_elements;
		if (write_nodes)
			domain_types = domain_types | CMZN_FIELD_DOMAIN_TYPE_NODES;
		if (write_data)
			domain_types = domain_types | CMZN_FIELD_DOMAIN_TYPE_DATAPOINTS;
		cmzn_streaminformation_region_set_resource_domain_types(si_region, sr,
			domain_types);
		if (isFieldML)
			cmzn_streaminformation_region_set_file_format(si_region, CMZN_STREAMINFORMATION_REGION_FILE_FORMAT_FIELDML);
		else
			cmzn_streaminformation_region_set_file_format(si_region, CMZN_STREAMINFORMATION_REGION_FILE_FORMAT_EX);
		if (number_of_field_names && field_names)
		{
			if (number_of_field_names == 1 && (0 == (strcmp(field_names[0], "none"))))
			{
				si_region->setWriteNoField(1);
			}
			else
			{
				const char **temp_names = new const char *[number_of_field_names];

				for (int i = 0; i < number_of_field_names; i++)
				{
					temp_names[i] = field_names[i];
				}
				cmzn_streaminformation_region_set_resource_field_names(si_region,
					sr, number_of_field_names, temp_names);
				delete[] temp_names;
			}
		}
		cmzn_streaminformation_region_set_resource_group_name(si_region,
			sr, group_name);
		cmzn_streaminformation_region_set_resource_attribute_real(
			si_region, sr, CMZN_STREAMINFORMATION_REGION_ATTRIBUTE_TIME,
			(double)time);
		return_code = cmzn_region_write(region, si_region);
		cmzn_streamresource_destroy(&sr);
		cmzn_streaminformation_region_destroy(&si_region);
		cmzn_streaminformation_destroy(&si);
	}

	return return_code;
}
