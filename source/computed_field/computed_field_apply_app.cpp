/* OpenCMISS-Cmgui Application
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "opencmiss/zinc/fieldapply.h"
#include "general/debug.h"
#include "general/message.h"
#include "command/parser.h"
#include "computed_field/computed_field_app.h"
#include "computed_field/computed_field_apply_app.h"
#include "computed_field/computed_field_private.hpp"
#include "computed_field/computed_field_private_app.hpp"

const char computed_field_alias_type_string[] = "alias";
const char computed_field_apply_type_string[] = "apply";
const char computed_field_argument_real_type_string[] = "argument_real";

/**
 * Command modifier function for defining a field as type computed_field_apply
 * (if not already) and allowing its contents to be modified.
 *
 * @param state  The parse state containing the command tokens.
 * @param field_modify_void  Void pointer to Computed_field_modify_data containing
 *   the field and the region it will be added to.
 * @return  1 on success, 0 on failure.
 */
int define_Computed_field_type_apply(Parse_state *state,
	void *field_modify_void, void *)
{
	int return_code = 1;
	Computed_field_modify_data *field_modify = static_cast<Computed_field_modify_data *>(field_modify_void);
	if ((state) && (field_modify))
	{
		char *evaluateFieldPathAndName = nullptr;
		Multiple_strings bindArgumentSourceFieldNames;

		Option_table *option_table = CREATE(Option_table)();
		Option_table_add_help(option_table,
			"Creates a field which applies the function of the evaluate field, "
			"which can be from another region, with argument binding to modify behaviour. "
			"This is the main mechanism for reusing field definitions from other regions. "
			"Currently limited to real-valued fields only. "
			"The optional region path is an absolute path from the root region if preceded by / "
			"or relative to the current region if not; use '..' to go up a region. "
			"All argument fields which the evalaute field depend on must be bound to "
			"source fields from this region to be able to evaluate the apply field. "
			"See argument_real field. Deprecated alias field is now made as an apply field.");
		Option_table_add_string_entry(option_table,"field",
			&evaluateFieldPathAndName, " [[/|../]REGION_PATH/]EVALUATE_FIELD_NAME");
		Option_table_add_multiple_strings_entry(option_table, "bind", &bindArgumentSourceFieldNames,
			"ARGUMENT_FIELD & SOURCE_FIELD [& ARGUMENT_FIELD & SOURCE_FIELD [&...]]]");
		return_code = Option_table_multi_parse(option_table, state);
		DESTROY(Option_table)(&option_table);
		/* no errors, not asking for help */
		if (return_code)
		{
			char *regionPath = nullptr;
			char *evaluateFieldName = nullptr;
			cmzn_region *evaluateRegion = nullptr;
			cmzn_field *evaluateField = nullptr;
			if (evaluateFieldPathAndName)
			{
				cmzn_region *baseRegion = nullptr;
				if (evaluateFieldPathAndName[0] == CMZN_REGION_PATH_SEPARATOR_CHAR)
				{
					// absolute path
					baseRegion = field_modify->get_region()->getRoot();
				}
				else
				{
					// relative path
					baseRegion = field_modify->get_region();
				}
				if (cmzn_region_get_partial_region_path(baseRegion,
					evaluateFieldPathAndName, &evaluateRegion, &regionPath, &evaluateFieldName))
				{
					evaluateField = cmzn_region_find_field_by_name(evaluateRegion, evaluateFieldName);
					if (!evaluateField)
					{
						return_code = 0;
					}
				}
				else
				{
					return_code = 0;
				}
				if (!return_code)
				{
					display_message(ERROR_MESSAGE,
						"gfx define field apply:  Could not find evaluate region/field %s", evaluateFieldPathAndName);
					display_parse_state_location(state);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE, "gfx define field apply:  Must specify field to evaluate");
				display_parse_state_location(state);
				return_code = 0;
			}
			if (bindArgumentSourceFieldNames.number_of_strings % 2)
			{
				display_message(ERROR_MESSAGE, "gfx define field apply:  Must specify pairs of bind argument & source fields");
				display_parse_state_location(state);
				return_code = 0;
			}
			if (return_code)
			{
				cmzn_field *field = cmzn_fieldmodule_create_field_apply(field_modify->get_field_module(), evaluateField);
				cmzn_field_apply *applyField = cmzn_field_cast_apply(field);
				if (!applyField)
				{
					display_message(ERROR_MESSAGE, "gfx define field apply:  Invalid evaluate field %s -- must be real-valued", evaluateFieldName);
					display_parse_state_location(state);
					return_code = 0;
				}
				else
				{
					// bind arguments
					cmzn_fieldmodule *evaluateFieldmodule = cmzn_region_get_fieldmodule(evaluateRegion);
					cmzn_fieldmodule *sourceFieldmodule = cmzn_region_get_fieldmodule(field_modify->get_region());
					const int bindCount = bindArgumentSourceFieldNames.number_of_strings/2;
					for (int i = 0; i < bindArgumentSourceFieldNames.number_of_strings; i += 2)
					{
						const char *argumentFieldName = bindArgumentSourceFieldNames.strings[i];
						cmzn_field *argumentField = cmzn_fieldmodule_find_field_by_name(evaluateFieldmodule, argumentFieldName);
						const char *sourceFieldName = bindArgumentSourceFieldNames.strings[i + 1];
						cmzn_field *sourceField = cmzn_fieldmodule_find_field_by_name(sourceFieldmodule, sourceFieldName);
						if (CMZN_OK != cmzn_field_apply_set_bind_argument_source_field(applyField, argumentField, sourceField))
						{
							display_message(ERROR_MESSAGE, "gfx define field apply:  Failed to bind argument field %s%s to source field %s%s",
								argumentFieldName, (argumentField) ? "" : "(not found in evaluate region)",
								sourceFieldName, (sourceField) ? "" : "(not found in source region)");
							display_parse_state_location(state);
							return_code = 0;
						}
						cmzn_field_destroy(&sourceField);
						cmzn_field_destroy(&argumentField);
					}
					cmzn_fieldmodule_destroy(&sourceFieldmodule);
					cmzn_fieldmodule_destroy(&evaluateFieldmodule);
					if (return_code)
					{
						return_code = field_modify->define_field(field);
					}
				}
				cmzn_field_destroy(&field);
			}
			if (evaluateField)
			{
				cmzn_field_destroy(&evaluateField);
			}
			DEALLOCATE(regionPath);
			DEALLOCATE(evaluateFieldName);
		}
		DEALLOCATE(evaluateFieldPathAndName);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_apply.  Invalid argument(s)");
		return_code = 0;
	}
	return return_code;
}

/**
 * Command modifier function for defining a field as type argument_real.
 *
 * @param state  The parse state containing the command tokens.
 * @param field_modify_void  Void pointer to Computed_field_modify_data containing
 *   the field and the region it will be added to.
 * @return  1 on success, 0 on failure.
 */
int define_Computed_field_type_argument_real(struct Parse_state *state,
	void *field_modify_void, void *)
{
	int return_code = 1;
	Computed_field_modify_data *field_modify = static_cast<Computed_field_modify_data *>(field_modify_void);
	if ((state) && (field_modify))
	{
		int number_of_components = 0;
		cmzn_field *field = field_modify->get_field();
		if (field)
		{
			cmzn_field_argument_real *argument_real_field = cmzn_field_cast_argument_real(field);
			if (argument_real_field)
			{
				number_of_components = cmzn_field_get_number_of_components(field);
				cmzn_field_argument_real_destroy(&argument_real_field);
			}
		}
		Option_table *option_table = CREATE(Option_table)();
		Option_table_add_help(option_table,
			"Define an argument field promising a number of real components. "
			"Can define other fields as functions of the unknown argument. "
			"Use an apply field to evaluate the function field by binding "
			"all argument fields it depends on to actual fields supplying "
			"the same number of real components. See apply field.");
		Option_table_add_int_positive_entry(option_table, "number_of_components", &number_of_components);
		return_code = Option_table_multi_parse(option_table, state);
		DESTROY(Option_table)(&option_table);
		if (return_code)
		{
			if (number_of_components > 0)
			{
				return_code = field_modify->define_field(
					cmzn_fieldmodule_create_field_argument_real(field_modify->get_field_module(),
						number_of_components));
			}
			else
			{
				display_message(ERROR_MESSAGE, "gfx define field argument_real.  Must specify number of components");
				return_code = 0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_argument_real.  Invalid argument(s)");
		return_code = 0;
	}
	return return_code;
}

int Computed_field_register_types_apply(
	struct Computed_field_package *computed_field_package)
{
	int return_code;
	if (computed_field_package)
	{
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_alias_type_string,
			define_Computed_field_type_apply,
			Computed_field_package_get_simple_package(computed_field_package));
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_apply_type_string,
			define_Computed_field_type_apply,
			Computed_field_package_get_simple_package(computed_field_package));
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_argument_real_type_string,
			define_Computed_field_type_argument_real,
			Computed_field_package_get_simple_package(computed_field_package));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_register_type_apply.  Invalid argument(s)");
		return_code=0;
	}
	return (return_code);
}
