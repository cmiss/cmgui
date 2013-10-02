/* OpenCMISS-Cmgui Application
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "zinc/field.h"
#include "zinc/fieldmodule.h"
#include "zinc/fieldgroup.h"
#include "zinc/region.h"
#include "zinc/optimisation.h"
#include "general/debug.h"
#include "general/message.h"
#include "command/parser.h"
#include "general/enumerator.h"
#include "general/enumerator_private.hpp"
#include "region/cmiss_region_app.h"
#include "minimise/minimise.h"

int gfx_minimise(struct Parse_state *state, void *dummy_to_be_modified,
	void *root_region_void)
{
	int return_code;
	USE_PARAMETER(dummy_to_be_modified);
	cmzn_region_id root_region = reinterpret_cast<cmzn_region_id>(root_region_void);
	if (state && root_region)
	{
		enum cmzn_optimisation_method optimisation_method = CMZN_OPTIMISATION_METHOD_QUASI_NEWTON;
		int maxIters = 100; // default value
		int showReport = 1; // output solution report by default
		const char *optimisation_method_string = 0;
		Multiple_strings independentFieldNames;
		Multiple_strings objectiveFieldNames;
		cmzn_region_id region = cmzn_region_access(root_region);

		Option_table *option_table = CREATE(Option_table)();
		/* independent field(s) */
		independentFieldNames.number_of_strings = 0;
		independentFieldNames.strings = (char **)NULL;
		Option_table_add_multiple_strings_entry(option_table, "independent_fields",
			&independentFieldNames, "FIELD_NAME [& FIELD_NAME [& ...]]");
		/* limit the number of iterations (really the number of objective
		   function evaluations) */
		Option_table_add_entry(option_table, "maximum_iterations", &maxIters,
			NULL, set_int_positive);
		/* method */
		optimisation_method_string =
			ENUMERATOR_STRING(cmzn_optimisation_method)(optimisation_method);
		int number_of_valid_strings = 0;
		const char **valid_strings;
		valid_strings = ENUMERATOR_GET_VALID_STRINGS(cmzn_optimisation_method)(
			&number_of_valid_strings,
			(ENUMERATOR_CONDITIONAL_FUNCTION(cmzn_optimisation_method) *)NULL,
			(void *)NULL);
		Option_table_add_enumerator(option_table, number_of_valid_strings,
			valid_strings, &optimisation_method_string);
		DEALLOCATE(valid_strings);
		/* objective field(s) */
		objectiveFieldNames.number_of_strings = 0;
		objectiveFieldNames.strings = (char **)NULL;
		Option_table_add_multiple_strings_entry(option_table, "objective_fields",
			&objectiveFieldNames, "FIELD_NAME [& FIELD_NAME [& ...]]");
		/* region */
		Option_table_add_set_cmzn_region(option_table, "region", root_region, &region);
		/* flag whether to show or hide the optimisation output */
		Option_table_add_switch(option_table, "show_output", "hide_output", &showReport);
		return_code = Option_table_multi_parse(option_table, state);
		if (return_code)
		{
			cmzn_fieldmodule_id fieldModule = cmzn_region_get_fieldmodule(region);
			cmzn_optimisation_id optimisation = cmzn_fieldmodule_create_optimisation(fieldModule);
			STRING_TO_ENUMERATOR(cmzn_optimisation_method)(
				optimisation_method_string, &optimisation_method);
			if (!cmzn_optimisation_set_method(optimisation, optimisation_method))
			{
				display_message(ERROR_MESSAGE, "gfx minimise:  Invalid optimisation method");
				return_code = 0;
			}
			for (int i = 0; i < independentFieldNames.number_of_strings; i++)
			{
				cmzn_field_id independentField = cmzn_fieldmodule_find_field_by_name(
					fieldModule, independentFieldNames.strings[i]);
				if (!cmzn_optimisation_add_independent_field(optimisation, independentField))
				{
					display_message(ERROR_MESSAGE, "gfx minimise:  Invalid or unrecognised independent field '%s'",
						independentFieldNames.strings[i]);
					return_code = 0;
				}
				cmzn_field_destroy(&independentField);
			}
			for (int i = 0; i < objectiveFieldNames.number_of_strings; i++)
			{
				cmzn_field_id objectiveField = cmzn_fieldmodule_find_field_by_name(
					fieldModule, objectiveFieldNames.strings[i]);
				if (!cmzn_optimisation_add_objective_field(optimisation, objectiveField))
				{
					display_message(ERROR_MESSAGE, "gfx minimise:  Invalid or unrecognised objective field '%s'",
						objectiveFieldNames.strings[i]);
					return_code = 0;
				}
				cmzn_field_destroy(&objectiveField);
			}
			if (!cmzn_optimisation_set_attribute_integer(optimisation,
				CMZN_OPTIMISATION_ATTRIBUTE_MAXIMUM_ITERATIONS, maxIters))
			{
				display_message(ERROR_MESSAGE, "gfx minimise:  Invalid maximum_iterations %d", maxIters);
				return_code = 0;
			}
			if (return_code)
			{
				return_code = cmzn_optimisation_optimise(optimisation);
				if (showReport)
				{
					char *report = cmzn_optimisation_get_solution_report(optimisation);
					if (report)
					{
						display_message_string(INFORMATION_MESSAGE, report);
						DEALLOCATE(report);
					}
				}
				if (!return_code)
				{
					display_message(ERROR_MESSAGE, "gfx minimise.  Optimisation failed.");
				}
			}
			cmzn_optimisation_destroy(&optimisation);
			cmzn_fieldmodule_destroy(&fieldModule);
		}
		DESTROY(Option_table)(&option_table);
		if (independentFieldNames.strings)
		{
			for (int i = 0; i < independentFieldNames.number_of_strings; i++)
			{
				DEALLOCATE(independentFieldNames.strings[i]);
			}
			DEALLOCATE(independentFieldNames.strings);
		}
		if (objectiveFieldNames.strings)
		{
			for (int i = 0; i < objectiveFieldNames.number_of_strings; i++)
			{
				DEALLOCATE(objectiveFieldNames.strings[i]);
			}
			DEALLOCATE(objectiveFieldNames.strings);
		}
		cmzn_region_destroy(&region);
	}
	else
	{
		display_message(ERROR_MESSAGE, "gfx_minimise.  Invalid argument(s)");
		return_code = 0;
	}
	return (return_code);
}
