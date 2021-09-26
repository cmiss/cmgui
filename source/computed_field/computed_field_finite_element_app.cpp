/* OpenCMISS-Cmgui Application
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <stdio.h>

#include "opencmiss/zinc/mesh.h"
#include "opencmiss/zinc/fieldderivatives.h"
#include "opencmiss/zinc/fieldfiniteelement.h"
#include "general/mystring.h"
#include "general/debug.h"
#include "general/message.h"
#include "command/parser.h"
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_app.h"
#include "computed_field/computed_field_private.hpp"
#include "computed_field/computed_field_private_app.hpp"
#include "computed_field/computed_field_set.h"
#include "computed_field/computed_field_set_app.h"
#include "computed_field/computed_field_finite_element.h"
#include "computed_field/field_module.hpp"
#include "finite_element/finite_element_region.h"
#include "mesh/cmiss_element_private.hpp"
#include "mesh/cmiss_element_private_app.hpp"
#include "finite_element/finite_element_app.h"

#if defined (DEBUG_CODE)
/* SAB This field is useful for debugging when things don't clean up properly
	but has to be used carefully, especially as operations such as caching
	accesses the node or element being considered so you get effects like
	the first point evaluated in an element having a count one less than
	all the others */
#define COMPUTED_FIELD_ACCESS_COUNT
#endif /* defined (DEBUG_CODE) */

const char computed_field_basis_derivative_type_string[] = "basis_derivative";
const char computed_field_xi_coordinates_type_string[] = "xi_coordinates";
const char computed_field_find_mesh_location_type_string[] = "find_mesh_location";
const char computed_field_edge_discontinuity_type_string[] = "edge_discontinuity";
const char computed_field_embedded_type_string[] = "embedded";
const char computed_field_node_value_type_string[] = "node_value";
const char computed_field_access_count_type_string[] = "access_count";
const char computed_field_cmiss_number_type_string[] = "cmiss_number";
const char computed_field_finite_element_type_string[] = "finite_element";
const char computed_field_is_exterior_type_string[] = "is_exterior";
const char computed_field_is_on_face_type_string[] = "is_on_face";

int Computed_field_get_type_embedded(struct Computed_field *field,
	struct Computed_field **source_field_address,
	struct Computed_field **embedded_location_field_address);

cmzn_field *cmzn_fieldmodule_create_field_access_count(
	cmzn_fieldmodule *fieldmodule);

int define_Computed_field_type_finite_element(struct Parse_state *state,
	void *field_modify_void, void *)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Creates FE_fields with the given name, coordinate_system, value_type,
cm_field_type, number_of_components and component_names.
The actual finite_element wrapper is not made here but in response to the
FE_field being made and/or modified.
==============================================================================*/
{
	char **component_names,**temp_component_names;
	const char *current_token;
	const char *cm_field_type_string,**valid_strings,*value_type_string;
	enum CM_field_type cm_field_type;
	enum Value_type value_type;
	int i,number_of_components,number_of_valid_strings,
		original_number_of_components,return_code;
	Computed_field_modify_data *field_modify = static_cast<Computed_field_modify_data *>(field_modify_void);
	struct Option_table *option_table;

	ENTER(define_Computed_field_type_finite_element);
	if ((state) && (field_modify))
	{
		return_code = 1;
		cmzn_field_id existing_field = field_modify->get_field();
		struct FE_field *existing_fe_field = (struct FE_field *)NULL;
		if (existing_field && Computed_field_is_type_finite_element(existing_field))
		{
			return_code =
				Computed_field_get_type_finite_element(field_modify->get_field(), &existing_fe_field);
		}
		if (return_code)
		{
			cmzn_mesh_id host_mesh = 0;
			if (existing_fe_field)
			{
				number_of_components = existing_field->getNumberOfComponents();
				cm_field_type = existing_fe_field->get_CM_field_type();
				value_type = get_FE_field_value_type(existing_fe_field);
			}
			else
			{
				/* default to real, scalar general field */
				number_of_components = 1;
				cm_field_type = CM_GENERAL_FIELD;
				value_type = FE_VALUE_VALUE;
			}
			if (return_code)
			{
				cm_field_type_string = ENUMERATOR_STRING(CM_field_type)(cm_field_type);
				value_type_string = Value_type_string(value_type);
			}
			if (ALLOCATE(component_names, char *, number_of_components))
			{
				for (i = 0; i < number_of_components; i++)
				{
					if (existing_fe_field)
					{
						component_names[i] =
							get_FE_field_component_name(existing_fe_field, i);
					}
					else
					{
						char temp[20];
						sprintf(temp, "%d", i+1);
						component_names[i] = duplicate_string(temp);
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"define_Computed_field_type_finite_element.  Not enough memory");
				return_code = 0;
			}
			original_number_of_components = number_of_components;
			const char *help_text =
				"Define a finite element field with parameters storable at nodes and, "
				"if of value type real, able to be interpolated on elements. Option "
				"'number_of_components' must be specified first if not 1. Special type "
				"'element_xi' is used to store embedded locations: the 'host_mesh' "
				"should be specified in this case e.g. mesh1d|mesh2d|mesh3d, and if not "
				"a warning is issued that it may or may not be discovered automatically "
				"by later commands. Specify field type 'coordinate' if real-valued with "
				"1-3 components to enable automatic use as a coordinate field.";
			/* try to handle help first */
			if (return_code && (current_token = state->current_token))
			{
				if (!(strcmp(PARSER_HELP_STRING, current_token) &&
					strcmp(PARSER_RECURSIVE_HELP_STRING, current_token)))
				{
					option_table = CREATE(Option_table)();
					Option_table_add_help(option_table, help_text);
					/* cm_field_type */
					valid_strings = ENUMERATOR_GET_VALID_STRINGS(CM_field_type)(
						&number_of_valid_strings,
						(ENUMERATOR_CONDITIONAL_FUNCTION(CM_field_type) *)NULL,
						(void *)NULL);
					Option_table_add_enumerator(option_table,number_of_valid_strings,
						valid_strings,&cm_field_type_string);
					DEALLOCATE(valid_strings);
					/* component_names */
					Option_table_add_entry(option_table,"component_names",component_names,
						&original_number_of_components,set_names);
					/* value_type */
					valid_strings=Value_type_get_valid_strings_simple(
						&number_of_valid_strings);
					Option_table_add_enumerator(option_table,number_of_valid_strings,
						valid_strings,&value_type_string);
					DEALLOCATE(valid_strings);
					// host_mesh
					Option_table_add_mesh_entry(option_table, "host_mesh", field_modify->get_region(), &host_mesh);
					/* number_of_components */
					Option_table_add_entry(option_table,"number_of_components",
						&number_of_components,NULL,set_int_positive);
					return_code=Option_table_multi_parse(option_table,state);
					DESTROY(Option_table)(&option_table);
				}
			}
			/* parse the number_of_components */
			current_token=state->current_token;
			if (return_code && current_token)
			{
				/* ... only if the "number_of_components" token is next */
				if (fuzzy_string_compare(current_token,"number_of_components"))
				{
					option_table=CREATE(Option_table)();
					Option_table_add_entry(option_table,"number_of_components",
						&number_of_components,NULL,set_int_positive);
					return_code=Option_table_parse(option_table,state);
					if (return_code)
					{
						if (number_of_components != original_number_of_components)
						{
							/* deallocate any names that will no longer be with the field */
							for (i=number_of_components;i<original_number_of_components;i++)
							{
								if (component_names[i])
								{
									DEALLOCATE(component_names[i]);
								}
							}
							if (REALLOCATE(temp_component_names,component_names,char *,
								number_of_components))
							{
								component_names=temp_component_names;
								/* clear any new names */
								for (i=original_number_of_components;i<number_of_components;i++)
								{
									component_names[i]=(char *)NULL;
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"define_Computed_field_type_finite_element.  "
									"Not enough memory");
								number_of_components=original_number_of_components;
								return_code=0;
							}
						}
					}
					DESTROY(Option_table)(&option_table);
				}
			}
			/* parse the remainder */
			if (return_code && state->current_token)
			{
				option_table = CREATE(Option_table)();
				Option_table_add_help(option_table, help_text);
				/* cm_field_type */
				valid_strings = ENUMERATOR_GET_VALID_STRINGS(CM_field_type)(
					&number_of_valid_strings,
					(ENUMERATOR_CONDITIONAL_FUNCTION(CM_field_type) *)NULL,
					(void *)NULL);
				Option_table_add_enumerator(option_table, number_of_valid_strings,
					valid_strings, &cm_field_type_string);
				DEALLOCATE(valid_strings);
				/* component_names */
				Option_table_add_entry(option_table, "component_names", component_names,
					&number_of_components, set_names);
				/* value_type */
				valid_strings = Value_type_get_valid_strings_simple(
					&number_of_valid_strings);
				Option_table_add_enumerator(option_table, number_of_valid_strings,
					valid_strings, &value_type_string);
				DEALLOCATE(valid_strings);
				// host_mesh
				Option_table_add_mesh_entry(option_table, "host_mesh", field_modify->get_region(), &host_mesh);
				return_code = Option_table_multi_parse(option_table,state);
				DESTROY(Option_table)(&option_table);
			}
			if (return_code)
			{
				value_type = Value_type_from_string(value_type_string);
				STRING_TO_ENUMERATOR(CM_field_type)(cm_field_type_string, &cm_field_type);
				cmzn_fieldmodule *field_module = field_modify->get_field_module();
				// cache changes to ensure FE_field not automatically wrapped already
				cmzn_fieldmodule_begin_change(field_module);
				const char *field_name = field_modify->get_field_name();
				FE_field *fe_field = FE_region_get_FE_field_with_general_properties(
					cmzn_region_get_FE_region(cmzn_fieldmodule_get_region_internal(field_module)),
					field_name, value_type, number_of_components);
				if ((fe_field) && (value_type == ELEMENT_XI_VALUE))
				{
					if (host_mesh)
					{
						FE_mesh *fe_mesh = cmzn_mesh_get_FE_mesh_internal(host_mesh);
						const int result = fe_field->setElementXiHostMesh(fe_mesh);
						if (result != CMZN_OK)
						{
							return_code = 0;
						}
					}
					else
					{
						display_message(WARNING_MESSAGE, "gfx define field %s finite_element element_xi: "
							"It is recommended that the host_mesh be specified for element_xi field value type "
							"as automatic discovery is not guaranteed to work in future", field_name);
					}
				}
				if (return_code)
				{
					if (fe_field)
					{
						fe_field->set_CM_field_type(cm_field_type);
						if (component_names)
						{
							for (i = 0; i<number_of_components; i++)
							{
								if (component_names[i])
								{
									set_FE_field_component_name(fe_field, i, component_names[i]);
								}
							}
						}
						return_code = field_modify->define_field(
							cmzn_fieldmodule_create_field_finite_element_wrapper(field_module, fe_field, field_name));
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"gfx define field finite_element.  Cannot change value type or number of components of existing field");
						return_code = 0;
					}
				}
				DEALLOCATE(field_name);
				DEACCESS(FE_field)(&fe_field);
				cmzn_fieldmodule_end_change(field_module);
			}
			/* clean up the component_names array */
			if (component_names)
			{
				for (i=0;i<number_of_components;i++)
				{
					if (component_names[i])
					{
						DEALLOCATE(component_names[i]);
					}
				}
				DEALLOCATE(component_names);
			}
			cmzn_mesh_destroy(&host_mesh);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_finite_element.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_finite_element */


int define_Computed_field_type_cmiss_number(struct Parse_state *state,
	void *field_modify_void, void *)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_CMZN_NUMBER.
==============================================================================*/
{
	Computed_field_modify_data *field_modify = static_cast<Computed_field_modify_data *>(field_modify_void);
	int return_code;

	ENTER(define_Computed_field_type_cmiss_number);
	if (state && (field_modify = (Computed_field_modify_data *)field_modify_void))
	{
		if (!state->current_token)
		{
			return_code = field_modify->define_field(
				cmzn_fieldmodule_create_field_cmiss_number(field_modify->get_field_module()));
		}
		else
		{
			if (strcmp(PARSER_HELP_STRING,state->current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token))
			{
				display_message(ERROR_MESSAGE,
					"Unknown option <%s>",state->current_token);
				display_parse_state_location(state);
			}
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_cmiss_number.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_cmiss_number */

#if defined (COMPUTED_FIELD_ACCESS_COUNT)

int define_Computed_field_type_access_count(struct Parse_state *state,
	void *field_modify_void, void *)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_ACCESS_COUNT.
==============================================================================*/
{
	int return_code;
	Computed_field_modify_data *field_modify = static_cast<Computed_field_modify_data *>(field_modify_void);

	ENTER(define_Computed_field_type_access_count);
	if ((state) && (field_modify))
	{
		if (!state->current_token)
		{
			return_code = field_modify->define_field(
				cmzn_fieldmodule_create_field_access_count(field_modify->get_field_module()));
		}
		else
		{
			if (strcmp(PARSER_HELP_STRING,state->current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token))
			{
				display_message(ERROR_MESSAGE,
					"Unknown option <%s>",state->current_token);
				display_parse_state_location(state);
			}
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_access_count.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_access_count */

#endif

/**
 * Convert field into type NODE_VALUE, if not already, and edit definition.
 */
int define_Computed_field_type_node_value(struct Parse_state *state,
	void *field_modify_void, void *)
{
	Computed_field_modify_data *field_modify = static_cast<Computed_field_modify_data *>(field_modify_void);
	int return_code;
	if ((state) && (field_modify))
	{
		return_code = 1;
		cmzn_field_id finite_element_field = 0;
		cmzn_node_value_label nodeValueLabel = CMZN_NODE_VALUE_LABEL_VALUE;
		int versionNumber = 1;
		if ((NULL != field_modify->get_field()) &&
			(computed_field_node_value_type_string ==
				Computed_field_get_type_string(field_modify->get_field())))
		{
			// note this is ACCESSed:
			finite_element_field = cmzn_field_get_source_field(field_modify->get_field(), 1);
			nodeValueLabel = cmzn_field_node_value_get_value_label(field_modify->get_field());
			versionNumber = cmzn_field_node_value_get_version_number(field_modify->get_field());
		}
		struct Option_table *option_table = CREATE(Option_table)();
		struct Set_Computed_field_conditional_data set_fe_field_data;
		set_fe_field_data.computed_field_manager = field_modify->get_field_manager();
		set_fe_field_data.conditional_function = Computed_field_is_type_finite_element_iterator;
		set_fe_field_data.conditional_function_user_data = (void *)NULL;
		/* fe_field */
		Option_table_add_Computed_field_conditional_entry(option_table, "fe_field",
			&finite_element_field, &set_fe_field_data);
		/* value|d/ds1|d/ds2|d2/ds1ds2|d/ds3|d2/ds1ds3|d2/ds2ds3|d3/ds1ds2ds3 */
		const char *nodeValueLabelName = ENUMERATOR_STRING(cmzn_node_value_label)(nodeValueLabel);
		int number_of_valid_strings;
		const char **valid_strings = ENUMERATOR_GET_VALID_STRINGS(cmzn_node_value_label)(
			&number_of_valid_strings, (ENUMERATOR_CONDITIONAL_FUNCTION(cmzn_node_value_label) *)0, (void *)0);
		Option_table_add_enumerator(option_table, number_of_valid_strings,
			valid_strings, &nodeValueLabelName);
		DEALLOCATE(valid_strings);
		/* version_number */
		Option_table_add_entry(option_table, "version", &versionNumber,
			NULL, set_int_positive);
		return_code = Option_table_multi_parse(option_table, state);
		DESTROY(Option_table)(&option_table);
		if (return_code)
		{
			if (!finite_element_field)
			{
				display_message(ERROR_MESSAGE, "Missing or invalid fe_field");
				display_parse_state_location(state);
				return_code = 0;
			}
		}
		if (return_code)
		{
			STRING_TO_ENUMERATOR(cmzn_node_value_label)(nodeValueLabelName, &nodeValueLabel);
			cmzn_field_id field = cmzn_fieldmodule_create_field_node_value(field_modify->get_field_module(),
				finite_element_field, nodeValueLabel, versionNumber);
			return_code = field_modify->define_field(field);
			if (!return_code)
			{
				display_message(ERROR_MESSAGE, "gfx define field node_value.  Failed");
			}
		}
		cmzn_field_destroy(&finite_element_field);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_node_value.  Invalid argument(s)");
		return_code = 0;
	}
	return (return_code);
}

/**
 * Converts field to type EDGE_DISCONTINUITY (if it is not already)
 * and allows its contents to be modified.
 */
int define_Computed_field_type_edge_discontinuity(struct Parse_state *state,
	void *field_modify_void, void *)
{
	int return_code = 1;
	Computed_field_modify_data *field_modify = static_cast<Computed_field_modify_data *>(field_modify_void);
	if ((state) && (field_modify))
	{
		return_code = 1;
		cmzn_field_id source_field = 0;
		cmzn_field_id conditional_field = 0;
		cmzn_field *existingField = field_modify->get_field(); // not accessed
		cmzn_field_edge_discontinuity_measure measure = CMZN_FIELD_EDGE_DISCONTINUITY_MEASURE_C1;
		if (existingField)
		{
			cmzn_field_edge_discontinuity_id field_edge_discontinuity = cmzn_field_cast_edge_discontinuity(existingField);
			if (field_edge_discontinuity)
			{
				source_field = cmzn_field_get_source_field(existingField, 1);
				conditional_field = cmzn_field_edge_discontinuity_get_conditional_field(field_edge_discontinuity);
				measure = cmzn_field_edge_discontinuity_get_measure(field_edge_discontinuity);
				cmzn_field_edge_discontinuity_destroy(&field_edge_discontinuity);
			}
		}
		Option_table *option_table = CREATE(Option_table)();
		Option_table_add_help(option_table,
			"An edge_discontinuity field produces a value on 1-D line elements with as many "
			"components as the source field, which gives the discontinuity of that field "
			"between two adjacent surfaces by a chosen measure. An optional conditional "
			"field restricts which adjacent surfaces qualify to those where the conditional "
			"field is true (non-zero). The first two qualifying surfaces are always used. "
			"The field values are zero when the surfaces are continuous by the chosen "
			"measure, and when there are fewer than two qualifying adjacent surfaces. "
			"Supported measures of discontinuity are C1 (the default, giving the difference "
			"in transverse xi derivatives), G1 (difference in transverse xi derivatives, "
			"normalised so only direction matters) and SURFACE_NORMAL (difference in unit "
			"surface normals, for 3-component coordinate source field only). In optimisation "
			"problems, adding an objective field consisting of the integral of [squares of] "
			"this field over a 1-D mesh will favour high-continuity solutions.");
		/* conditional_field */
		struct Set_Computed_field_conditional_data set_conditional_field_data;
		set_conditional_field_data.conditional_function = Computed_field_is_scalar;
		set_conditional_field_data.conditional_function_user_data = (void *)NULL;
		set_conditional_field_data.computed_field_manager = field_modify->get_field_manager();
		Option_table_add_entry(option_table, "conditional_field", &conditional_field,
			&set_conditional_field_data, set_Computed_field_conditional);
		/* measure */
		int number_of_valid_strings = 0;
		const char **valid_strings = ENUMERATOR_GET_VALID_STRINGS(cmzn_field_edge_discontinuity_measure)(
			&number_of_valid_strings, (ENUMERATOR_CONDITIONAL_FUNCTION(cmzn_field_edge_discontinuity_measure) *)NULL, (void *)NULL);
		const char *measure_string = ENUMERATOR_STRING(cmzn_field_edge_discontinuity_measure)(measure);
		Option_table_add_enumerator(option_table,number_of_valid_strings,
			valid_strings, &measure_string);
		DEALLOCATE(valid_strings);
		/* source_field */
		struct Set_Computed_field_conditional_data set_source_field_data;
		set_source_field_data.conditional_function = Computed_field_has_numerical_components;
		set_source_field_data.conditional_function_user_data = (void *)NULL;
		set_source_field_data.computed_field_manager = field_modify->get_field_manager();
		Option_table_add_entry(option_table, "source_field", &source_field,
			&set_source_field_data, set_Computed_field_conditional);
		return_code = Option_table_multi_parse(option_table, state);
		DESTROY(Option_table)(&option_table);
		if (return_code)
		{
			cmzn_field_id field = cmzn_fieldmodule_create_field_edge_discontinuity(field_modify->get_field_module(), source_field);
			if (field)
			{
				cmzn_field_edge_discontinuity_id edge_discontinuity_field = cmzn_field_cast_edge_discontinuity(field);
				if (!STRING_TO_ENUMERATOR(cmzn_field_edge_discontinuity_measure)(measure_string, &measure) ||
					(CMZN_OK != cmzn_field_edge_discontinuity_set_measure(edge_discontinuity_field, measure)))
				{
					display_message(ERROR_MESSAGE, "gfx define field edge_discontinuity.  Invalid measure for source_field");
					return_code = 0;
				}
				if (CMZN_OK != cmzn_field_edge_discontinuity_set_conditional_field(edge_discontinuity_field, conditional_field))
				{
					display_message(ERROR_MESSAGE, "gfx define field edge_discontinuity.  Could not set conditional_field");
					return_code = 0;
				}
				cmzn_field_edge_discontinuity_destroy(&edge_discontinuity_field);
			}
			else
			{
				return_code = 0;
				if (!source_field)
					display_message(ERROR_MESSAGE, "gfx define field edge_discontinuity.  Must set source_field");
				else
					display_message(ERROR_MESSAGE, "gfx define field edge_discontinuity.  Failed");
			}
			if (return_code)
			{
				return_code = field_modify->define_field(field);
				field = 0;
			}
			else
				cmzn_field_destroy(&field);
		}
		cmzn_field_destroy(&source_field);
		cmzn_field_destroy(&conditional_field);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_edge_discontinuity.  Invalid argument(s)");
		return_code = 0;
	}
	return (return_code);
}

int define_Computed_field_type_embedded(struct Parse_state *state,
	void *field_modify_void, void *)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_EMBEDDED (if it is not already)
and allows its contents to be modified.
==============================================================================*/
{
	int return_code;

	ENTER(define_Computed_field_type_embedded);
	Computed_field_modify_data *field_modify = static_cast<Computed_field_modify_data *>(field_modify_void);
	if ((state) && (field_modify))
	{
		return_code = 1;
		cmzn_field_id source_field = 0;
		cmzn_field_id embedded_location_field = 0;
		if ((NULL != field_modify->get_field()) &&
			(computed_field_embedded_type_string ==
				Computed_field_get_type_string(field_modify->get_field())))
		{
			Computed_field_get_type_embedded(field_modify->get_field(),
				&source_field, &embedded_location_field);
			cmzn_field_access(source_field);
			cmzn_field_access(embedded_location_field);
		}
		Option_table *option_table = CREATE(Option_table)();
		Option_table_add_help(option_table,
			"An embedded field returns the value of a source field at the "
			"location given by the element_xi field - a field whose value is "
			"a location in a mesh.");
		/* embedded_location_field */
		struct Set_Computed_field_conditional_data set_embedded_location_field_data;
		set_embedded_location_field_data.conditional_function = Computed_field_has_value_type_mesh_location;
		set_embedded_location_field_data.conditional_function_user_data = (void *)NULL;
		set_embedded_location_field_data.computed_field_manager = field_modify->get_field_manager();
		Option_table_add_entry(option_table, "element_xi", &embedded_location_field,
			&set_embedded_location_field_data, set_Computed_field_conditional);
		/* source_field */
		struct Set_Computed_field_conditional_data set_source_field_data;
		set_source_field_data.conditional_function = Computed_field_has_numerical_components;
		set_source_field_data.conditional_function_user_data = (void *)NULL;
		set_source_field_data.computed_field_manager = field_modify->get_field_manager();
		Option_table_add_entry(option_table, "field", &source_field,
			&set_source_field_data, set_Computed_field_conditional);
		return_code = Option_table_multi_parse(option_table, state);
		DESTROY(Option_table)(&option_table);
		if (return_code)
		{
			if (embedded_location_field && source_field)
			{
				return_code = field_modify->define_field(
					cmzn_fieldmodule_create_field_embedded(field_modify->get_field_module(),
						source_field, embedded_location_field));
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"define_Computed_field_type_embedded.  "
					"Must specify both source field and element_xi field");
				return_code = 0;
			}
		}
		if (source_field)
		{
			cmzn_field_destroy(&source_field);
		}
		if (embedded_location_field)
		{
			cmzn_field_destroy(&embedded_location_field);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_embedded.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_embedded */


/***************************************************************************//**
 * Command modifier function which converts field into find_mesh_location type
 * (if it is not already) and allows its contents to be modified.
 */
int define_Computed_field_type_find_mesh_location(struct Parse_state *state,
	void *field_modify_void, void *)
{
	int return_code;

	ENTER(define_Computed_field_type_find_mesh_location);
	Computed_field_modify_data * field_modify = (Computed_field_modify_data *)field_modify_void;
	if ((state) && (field_modify))
	{
		return_code = 1;
		cmzn_mesh_id mesh = 0;
		cmzn_field_id mesh_field = 0;
		cmzn_mesh_id search_mesh = 0;
		cmzn_field_id source_field = 0;
		int find_nearest_flag = 0;
		if (NULL != field_modify->get_field())
		{
			cmzn_field_find_mesh_location_id find_mesh_location_field =
				cmzn_field_cast_find_mesh_location(field_modify->get_field());
			if (find_mesh_location_field)
			{
				mesh = cmzn_field_find_mesh_location_get_mesh(find_mesh_location_field);
				source_field = cmzn_field_get_source_field(field_modify->get_field(), 1);
				mesh_field = cmzn_field_get_source_field(field_modify->get_field(), 2);
				find_nearest_flag = (CMZN_FIELD_FIND_MESH_LOCATION_SEARCH_MODE_EXACT !=
					cmzn_field_find_mesh_location_get_search_mode(find_mesh_location_field));
				search_mesh = cmzn_field_find_mesh_location_get_search_mesh(find_mesh_location_field);
				cmzn_field_find_mesh_location_destroy(&find_mesh_location_field);
			}
		}
		if (return_code)
		{
			struct Set_Computed_field_conditional_data set_mesh_field_data, set_source_field_data;
			Option_table *option_table = CREATE(Option_table)();
			Option_table_add_help(option_table,
				"A find_mesh_location field calculates the source_field then finds "
				"and returns the location in the mesh where the mesh_field has the "
				"same value. Use an embedded field to evaluate other fields on the "
				"mesh at the found location. Option find_nearest returns the location "
				"with the nearest value of the mesh_field if no exact match is found. "
				"The optional search_mesh is used to limit the search to a subset e.g. "
				"face mesh, but values are stored on the main mesh.");
			// find_nearest|find_exact
			Option_table_add_switch(option_table,
				"find_nearest", "find_exact", &find_nearest_flag);
			// mesh
			Option_table_add_mesh_entry(option_table, "mesh", field_modify->get_region(), &mesh);
			// mesh_field
			set_mesh_field_data.conditional_function =
				Computed_field_has_numerical_components;
			set_mesh_field_data.conditional_function_user_data = (void *)NULL;
			set_mesh_field_data.computed_field_manager =
				field_modify->get_field_manager();
			Option_table_add_entry(option_table, "mesh_field", &mesh_field,
				&set_mesh_field_data, set_Computed_field_conditional);
			// search_mesh_field
			Option_table_add_mesh_entry(option_table, "search_mesh", field_modify->get_region(), &search_mesh);
			// source_field
			set_source_field_data.conditional_function =
				Computed_field_has_numerical_components;
			set_source_field_data.conditional_function_user_data = (void *)NULL;
			set_source_field_data.computed_field_manager =
				field_modify->get_field_manager();
			Option_table_add_entry(option_table, "source_field", &source_field,
				&set_source_field_data, set_Computed_field_conditional);
			return_code = Option_table_multi_parse(option_table, state);
			DESTROY(Option_table)(&option_table);
			if (return_code)
			{
				if (!mesh)
				{
					display_message(ERROR_MESSAGE, "gfx define field find_mesh_location.  Must specify mesh.");
					return_code = 0;
				}
				if (return_code)
				{
					cmzn_field_id field = cmzn_fieldmodule_create_field_find_mesh_location(field_modify->get_field_module(),
						source_field, mesh_field, mesh);
					if (field)
					{
						cmzn_field_find_mesh_location_id find_mesh_location_field = cmzn_field_cast_find_mesh_location(field);
						cmzn_field_find_mesh_location_set_search_mode(find_mesh_location_field,
							(find_nearest_flag ? CMZN_FIELD_FIND_MESH_LOCATION_SEARCH_MODE_NEAREST
							: CMZN_FIELD_FIND_MESH_LOCATION_SEARCH_MODE_EXACT));
						if ((search_mesh) && (CMZN_OK != cmzn_field_find_mesh_location_set_search_mesh(find_mesh_location_field, search_mesh)))
						{
							display_message(ERROR_MESSAGE, "gfx define field find_mesh_location.  Invalid search mesh.");
							cmzn_field_destroy(&field);
							return_code = 0;
						}
						else
						{
							return_code = field_modify->define_field(field);
							field = 0;
						}
						cmzn_field_find_mesh_location_destroy(&find_mesh_location_field);
					}
					else
					{
						if ((!source_field) || (!mesh_field) ||
							(cmzn_field_get_number_of_components(source_field) !=
								cmzn_field_get_number_of_components(mesh_field)))
						{
							display_message(ERROR_MESSAGE, "gfx define field find_mesh_location.  "
								"Failed due to source_field or mesh_field unspecified, or number of components different or lower than mesh dimension.");
							return_code = 0;
						}
					}
				}
			}
		}
		if (mesh)
			cmzn_mesh_destroy(&mesh);
		if (mesh_field)
			cmzn_field_destroy(&mesh_field);
		if (search_mesh)
			cmzn_mesh_destroy(&search_mesh);
		if (source_field)
			cmzn_field_destroy(&source_field);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_find_mesh_location.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
}

int define_Computed_field_type_xi_coordinates(struct Parse_state *state,
	void *field_modify_void, void *)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_XI_COORDINATES.
==============================================================================*/
{
	int return_code;
	Computed_field_modify_data *field_modify = static_cast<Computed_field_modify_data *>(field_modify_void);

	ENTER(define_Computed_field_type_xi_coordinates);
	if ((state) && (field_modify))
	{
		if (!state->current_token)
		{
			return_code = field_modify->define_field(
				cmzn_fieldmodule_create_field_xi_coordinates(field_modify->get_field_module()));
		}
		else
		{
			if (strcmp(PARSER_HELP_STRING,state->current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token))
			{
				display_message(ERROR_MESSAGE,
					"Unknown option <%s>",state->current_token);
				display_parse_state_location(state);
			}
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_xi_coordinates.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_xi_coordinates */

int define_Computed_field_type_basis_derivative(struct Parse_state *state,
	void *field_modify_void, void *)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Creates FE_fields with the given name, coordinate_system, value_type,
cm_field_type, number_of_components and component_names.
The actual finite_element wrapper is not made here but in response to the
FE_field being made and/or modified.
==============================================================================*/
{
	char basis_derivative_help[] =
		"Creates a field giving derivative of arbitrary <order> w.r.t. element "
		"<xi_indices> of length order. This derivative then becomes the value "
		"for the field. Higher order derivatives are formed as derivatives of "
		"derivatives. Deprecated: use derivative field directly now.";
	int return_code = 1;
	Computed_field_modify_data *field_modify = static_cast<Computed_field_modify_data *>(field_modify_void);
	struct Option_table *option_table;

	if ((state) && (field_modify))
	{
		int order = 1;
		int *xi_indices = (int *)NULL;
		cmzn_field_id source_field = 0;
		cmzn_field_id field = field_modify->get_field();

		/* Assign default values for xi_indices */
		ALLOCATE(xi_indices, int, order);
		for (int i = 0 ; i < order ; i++)
		{
			xi_indices[i] = 1;
		}

		struct Set_Computed_field_conditional_data set_fe_field_data;
		set_fe_field_data.computed_field_manager = field_modify->get_field_manager();
		set_fe_field_data.conditional_function = Computed_field_has_numerical_components;
		set_fe_field_data.conditional_function_user_data = (void *)NULL;

		/* try to handle help first */
		if (Parse_state_help_mode(state))
		{
			option_table=CREATE(Option_table)();
			Option_table_add_help(option_table, basis_derivative_help);
			/* fe_field */
			Option_table_add_Computed_field_conditional_entry(option_table, "fe_field",
				&source_field, &set_fe_field_data);
			Option_table_add_int_positive_entry(option_table,
				"order", &order);
			Option_table_add_int_vector_entry(option_table,
				"xi_indices", xi_indices, &order);
			return_code=Option_table_multi_parse(option_table,state);
			DESTROY(Option_table)(&option_table);
		}
		/* parse the order first */
		if (return_code)
		{
			// store previous state so that we can return to it
			int previous_state_index = state->current_index;

			/* parse the order of the differentiation. */
			option_table = CREATE(Option_table)();
			Option_table_add_help(option_table, basis_derivative_help);
			Option_table_add_int_positive_entry(option_table, "order",
				&order);
			/* Ignore all the other entries */
			Option_table_ignore_all_unmatched_entries(option_table);
			return_code = Option_table_multi_parse(option_table, state);
			DESTROY(Option_table)(&option_table);
			/* Return back to where we were */
			shift_Parse_state(state, previous_state_index - state->current_index);
		}
		if (return_code)
		{
			/* Allocate memory for xi_indices array based on order
				 and default all index values to 1 */
			int *temp_xi_indices;
			if (REALLOCATE(temp_xi_indices, xi_indices, int, order))
			{
				xi_indices = temp_xi_indices;
				for (int i = 0 ; i < order ; i++)
				{
					xi_indices[i] = 1;
				}
			}

			option_table=CREATE(Option_table)();
			/* fe_field */
			Option_table_add_Computed_field_conditional_entry(option_table, "fe_field",
				&source_field, &set_fe_field_data);
			Option_table_add_int_positive_entry(option_table,
				"order", &order);
			Option_table_add_int_vector_entry(option_table,
				"xi_indices", xi_indices, &order);
			return_code=Option_table_multi_parse(option_table,state);
			DESTROY(Option_table)(&option_table);
		}
		if (return_code)
		{
			cmzn_fieldmodule *fieldmodule = field_modify->get_field_module();
			cmzn_fieldmodule_begin_change(fieldmodule);
			// need a temporary fieldmodule for intermediate fields otherwise they take name of final field from fieldmodule
			cmzn_fieldmodule *tmp_fieldmodule = cmzn_region_get_fieldmodule(field_modify->get_region());
			cmzn_field *derivative_field = cmzn_field_access(source_field);
			for (int i = 0; i < order; i++)
			{
				cmzn_field *tmp_field = derivative_field;
				derivative_field = cmzn_fieldmodule_create_field_derivative(
					(i < (order - 1)) ? tmp_fieldmodule : fieldmodule, derivative_field, xi_indices[i]);
				cmzn_field_destroy(&tmp_field);
			}
			return_code = field_modify->define_field(derivative_field);
			cmzn_fieldmodule_destroy(&tmp_fieldmodule);
			cmzn_fieldmodule_end_change(fieldmodule);
		}
		if (!return_code)
		{
			if ((!state->current_token)||
				(strcmp(PARSER_HELP_STRING,state->current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
			{
				/* error */
				display_message(ERROR_MESSAGE,
					"define_Computed_field_type_basis_derivative.  Failed");
			}
		}

		DEALLOCATE(xi_indices);
		cmzn_field_destroy(&source_field);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_basis_derivative.  Invalid argument(s)");
		return_code=0;
	}

	return (return_code);
}

int define_Computed_field_type_is_exterior(struct Parse_state *state,
	void *field_modify_void, void *)
{
	int return_code = 0;
	Computed_field_modify_data *field_modify = static_cast<Computed_field_modify_data *>(field_modify_void);
	if ((state) && (field_modify))
	{
		Option_table *option_table = CREATE(Option_table)();
		Option_table_add_help(option_table,
			"An is_exterior field returns 1 on 2-D faces and 1-D lines considered as "
			"exterior to their top-level element, and 0 elsewhere.");
		return_code = Option_table_multi_parse(option_table, state);
		DESTROY(Option_table)(&option_table);
		if (return_code)
		{
			return_code = field_modify->define_field(
				cmzn_fieldmodule_create_field_is_exterior(field_modify->get_field_module()));
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_is_exterior.  Invalid argument(s)");
	}
	return (return_code);
}

int define_Computed_field_type_is_on_face(struct Parse_state *state,
	void *field_modify_void, void *)
{
	int return_code = 0;
	Computed_field_modify_data *field_modify = static_cast<Computed_field_modify_data *>(field_modify_void);
	if ((state) && (field_modify))
	{
		Option_table *option_table = CREATE(Option_table)();
		Option_table_add_help(option_table,
			"An is_on_face field returns 1 on 2-D faces and 1-D lines considered "
			"to lie on the specified face of their top-level element, and 0 elsewhere.");
		/* face {all|any_face|no_face|xi1_0|xi1_1|xi2_0|...} */
		cmzn_element_face_type face = CMZN_ELEMENT_FACE_TYPE_INVALID;
		Option_table_add_entry(option_table, "face", &face, /*type name*/const_cast<char *>("element face type"),
			setEnum<cmzn_element_face_type, /*firstEnum*/CMZN_ELEMENT_FACE_TYPE_ALL, cmzn_element_face_type_to_string>);
		return_code = Option_table_multi_parse(option_table, state);
		DESTROY(Option_table)(&option_table);
		if (return_code)
		{
			if (CMZN_ELEMENT_FACE_TYPE_INVALID == face)
			{
				display_message(ERROR_MESSAGE, "Must specify the element face type.");
				display_parse_state_location(state);
				return_code = 0;
			}
			if (return_code)
			{
				return_code = field_modify->define_field(
					cmzn_fieldmodule_create_field_is_on_face(field_modify->get_field_module(), face));
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_is_on_face.  Invalid argument(s)");
	}
	return (return_code);
}

int Computed_field_register_types_finite_element(
	struct Computed_field_package *computed_field_package)
/*******************************************************************************
LAST MODIFIED : 24 July 2008

DESCRIPTION :
This function registers the finite_element related types of Computed_fields.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_register_types_finite_element);
	if (computed_field_package)
	{
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_finite_element_type_string,
			define_Computed_field_type_finite_element,
			Computed_field_package_get_simple_package(computed_field_package));
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_cmiss_number_type_string,
			define_Computed_field_type_cmiss_number,
			Computed_field_package_get_simple_package(computed_field_package));
#if defined (COMPUTED_FIELD_ACCESS_COUNT)
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_access_count_type_string,
			define_Computed_field_type_access_count,
			Computed_field_package_get_simple_package(computed_field_package));
#endif /* defined (COMPUTED_FIELD_ACCESS_COUNT) */
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_xi_coordinates_type_string,
			define_Computed_field_type_xi_coordinates,
			Computed_field_package_get_simple_package(computed_field_package));
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_node_value_type_string,
			define_Computed_field_type_node_value,
			Computed_field_package_get_simple_package(computed_field_package));
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_find_mesh_location_type_string,
			define_Computed_field_type_find_mesh_location,
			Computed_field_package_get_simple_package(computed_field_package));
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_edge_discontinuity_type_string,
			define_Computed_field_type_edge_discontinuity,
			Computed_field_package_get_simple_package(computed_field_package));
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_embedded_type_string,
			define_Computed_field_type_embedded,
			Computed_field_package_get_simple_package(computed_field_package));
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_basis_derivative_type_string,
			define_Computed_field_type_basis_derivative,
			Computed_field_package_get_simple_package(computed_field_package));
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_is_exterior_type_string,
			define_Computed_field_type_is_exterior,
			Computed_field_package_get_simple_package(computed_field_package));
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_is_on_face_type_string,
			define_Computed_field_type_is_on_face,
			Computed_field_package_get_simple_package(computed_field_package));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_register_types_finite_element.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_register_types_finite_element */
