/* OpenCMISS-Cmgui Application
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "zinc/fieldmeshoperators.h"
#include "general/debug.h"
#include "general/message.h"
#include "command/parser.h"
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_private.hpp"
#include "computed_field/computed_field_private_app.hpp"
#include "computed_field/computed_field_set.h"
#include "computed_field/computed_field_set_app.h"
#include "computed_field/computed_field_mesh_operators.hpp"
#include "computed_field/computed_field_mesh_operators_app.hpp"
#include "mesh/cmiss_element_private.hpp"

class Computed_field_mesh_operators_package : public Computed_field_type_package
{
};

const char computed_field_mesh_integral_type_string[] = "mesh_integral";

/**
 * Command modifier function for getting the arguments common to several
 * mesh operator field types.
 * @return  1 on success with mesh, integrand_field and coordinate_field accessing
 * respective objects, or 0 on failure with no objects accessed.
 */
int define_Computed_field_type_mesh_operator(struct Parse_state *state,
	Computed_field_modify_data *field_modify, const char *type_name, const char *help_string,
	cmzn_field_id &integrand_field, cmzn_field_id &coordinate_field, cmzn_mesh_id &mesh,
	int &order)
{
	if (!(state && field_modify && help_string))
		return 0;
	int return_code = 1;
	integrand_field = 0;
	coordinate_field = 0;
	mesh = 0;
	char *mesh_name = 0;
	Option_table *option_table = CREATE(Option_table)();
	Option_table_add_help(option_table, help_string);
	Set_Computed_field_conditional_data set_coordinate_field_data =
		{ Computed_field_has_up_to_3_numerical_components, (void *)0, field_modify->get_field_manager() };
	Option_table_add_entry(option_table, "coordinate_field", &coordinate_field,
		&set_coordinate_field_data, set_Computed_field_conditional);
	Set_Computed_field_conditional_data set_integrand_field_data =
		{ Computed_field_has_numerical_components, (void *)0, field_modify->get_field_manager() };
	Option_table_add_entry(option_table, "integrand_field", &integrand_field,
		&set_integrand_field_data, set_Computed_field_conditional);
	Option_table_add_string_entry(option_table, "mesh", &mesh_name,
		" ELEMENT_GROUP_FIELD_NAME|[GROUP_NAME.]mesh1d|mesh2d|mesh3d");
	Option_table_add_int_non_negative_entry(option_table, "order", &order);
	return_code = Option_table_multi_parse(option_table, state);
	DESTROY(Option_table)(&option_table);
	if (return_code)
	{
		if (mesh_name)
		{
			mesh = cmzn_fieldmodule_find_mesh_by_name(
				field_modify->get_field_module(), mesh_name);
			if (!mesh)
			{
				mesh = cmzn_mesh_group_base_cast(
					cmzn_fieldmodule_create_mesh_group_from_name_internal(
						field_modify->get_field_module(), mesh_name));
			}
			if (!mesh)
			{
				display_message(ERROR_MESSAGE,
					"gfx define field %s:  Unable to find mesh %s", type_name, mesh_name);
				return_code = 0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx define field %s:  Must specify mesh", type_name);
			return_code = 0;
		}
		if (!integrand_field)
		{
			display_message(ERROR_MESSAGE,
				"gfx define field %s:  Must specify integrand field", type_name);
			return_code = 0;
		}
		if (coordinate_field)
		{
			if (mesh && (cmzn_field_get_number_of_components(coordinate_field) < cmzn_mesh_get_dimension(mesh)))
			{
				display_message(ERROR_MESSAGE,
					"gfx define field %s:  Coordinate field has fewer components than element dimension", type_name);
				return_code = 0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx define field %s:  Must specify integrand field", type_name);
			return_code = 0;
		}
		if (order > 4)
		{
			display_message(ERROR_MESSAGE,
				"gfx define field %s:  Order must be from 1 to 4", type_name);
			return_code = 0;
		}
	}
	if (mesh_name)
		DEALLOCATE(mesh_name);
	if (!return_code)
	{
		cmzn_mesh_destroy(&mesh);
		cmzn_field_destroy(&integrand_field);
		cmzn_field_destroy(&coordinate_field);
	}
	return (return_code);
}

/**
 * Converts <field> into type mesh_integral (if it is not already) and allows its
 * contents to be modified.
 */
int define_Computed_field_type_mesh_integral(struct Parse_state *state,
	void *field_modify_void, void *computed_field_mesh_operators_package_void)
{
	int return_code = 0;
	USE_PARAMETER(computed_field_mesh_operators_package_void);
	Computed_field_modify_data * field_modify =
		reinterpret_cast<Computed_field_modify_data *>(field_modify_void);
	cmzn_field_id integrand_field = 0;
	cmzn_field_id coordinate_field = 0;
	cmzn_mesh_id mesh = 0;
	int order = 1;
	if (define_Computed_field_type_mesh_operator(state, field_modify, "mesh_integral",
		"A mesh_integral field calculates the integral of each of the supplied field's "
		"component values over the mesh multiplied by differential volume/area/length "
		"depending on the mesh dimension. "
		"The order specifies the number of Gauss points in each element direction and "
		"may be from 1 to 4; use 1 for linear basis functions, 2 for quadratic, etc.",
		integrand_field, coordinate_field, mesh, order))
	{
		return_code = field_modify->update_field_and_deaccess(
			cmzn_fieldmodule_create_field_mesh_integral(field_modify->get_field_module(),
				integrand_field, coordinate_field, mesh));
	}
	cmzn_field_destroy(&integrand_field);
	cmzn_field_destroy(&coordinate_field);
	cmzn_mesh_destroy(&mesh);
	return return_code;
}

int Computed_field_register_types_mesh_operators(
	struct Computed_field_package *computed_field_package)
{
	int return_code;
	Computed_field_mesh_operators_package
		*computed_field_mesh_operators_package =
		new Computed_field_mesh_operators_package;

	ENTER(Computed_field_register_types_mesh_operators);
	if (computed_field_package)
	{
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_mesh_integral_type_string,
			define_Computed_field_type_mesh_integral,
			computed_field_mesh_operators_package);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_register_types_mesh_operators.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
}
