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
#include "finite_element/finite_element.h"
#include "graphics/tessellation_app.hpp"
#include "mesh/cmiss_element_private.hpp"

class Computed_field_mesh_operators_package : public Computed_field_type_package
{
};

/**
 * Command modifier function for getting the arguments common to several
 * mesh operator field types.
 * @return  1 on success with mesh, integrand_field and coordinate_field accessing
 * respective objects, or 0 on failure with no objects accessed.
 */
int define_Computed_field_type_mesh_operator(struct Parse_state *state,
	Computed_field_modify_data *field_modify, const char *type_name, const char *help_string,
	cmzn_field_id &integrand_field, cmzn_field_id &coordinate_field, cmzn_mesh_id &mesh,
	cmzn_element_quadrature_rule &quadrature_rule, int *&numbers_of_points, int &numbers_of_points_size)
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
	// quadrature rule
	const char *quadrature_rule_string =
		ENUMERATOR_STRING(cmzn_element_quadrature_rule)(quadrature_rule);
	int number_of_valid_strings;
	const char **valid_strings = ENUMERATOR_GET_VALID_STRINGS(cmzn_element_quadrature_rule)(
		&number_of_valid_strings, (ENUMERATOR_CONDITIONAL_FUNCTION(cmzn_element_quadrature_rule) *)NULL,
		(void *)NULL);
	Option_table_add_enumerator(option_table, number_of_valid_strings,
		valid_strings, &quadrature_rule_string);
	DEALLOCATE(valid_strings);
	Set_Computed_field_conditional_data set_integrand_field_data =
		{ Computed_field_has_numerical_components, (void *)0, field_modify->get_field_manager() };
	Option_table_add_entry(option_table, "integrand_field", &integrand_field,
		&set_integrand_field_data, set_Computed_field_conditional);
	Option_table_add_string_entry(option_table, "mesh", &mesh_name,
		" ELEMENT_GROUP_FIELD_NAME|[GROUP_NAME.]mesh1d|mesh2d|mesh3d");
	Option_table_add_divisions_entry(option_table, "numbers_of_points",
		&numbers_of_points, &numbers_of_points_size);
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
		STRING_TO_ENUMERATOR(cmzn_element_quadrature_rule)(quadrature_rule_string, &quadrature_rule);
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
	int numbers_of_points_size = 1;
	int *numbers_of_points;
	ALLOCATE(numbers_of_points, int, numbers_of_points_size);
	numbers_of_points[0] = 1;
	cmzn_element_quadrature_rule quadrature_rule = CMZN_ELEMENT_QUADRATURE_RULE_GAUSSIAN;
	if (define_Computed_field_type_mesh_operator(state, field_modify, "mesh_integral",
		"A mesh_integral field calculates the integral of each of the integrand field's "
		"component values over the mesh multiplied by differential volume/area/length, "
		"depending on the mesh dimension, calculated from the coordinate field which "
		"is assumed to be rectangular Cartesian; use a coordinate_transformation "
		"field if it is not. Different quadrature rules are supported. "
		"The numbers of quadrature points are applied on the top-level element with "
		"the appropriate values inherited on faces and lines; this is only important "
		"if different numbers are used in each element axis. "
		"Simplex elements use the maximum number set on on any linked dimension. "
		"For 1-D Gaussian quadrature, N points exactly integrates a polynomial of "
		"degree 2N - 1, however 1 more point than the degree is often needed to avoid "
		"spurious modes in many numerical solutions. Simplex elements use specialised "
		"point arrangements that are sufficient for integrating a polynomial with the "
		"same degree as the number of points. "
		"The maximum number of Gauss points on each element axis is currently 4; if "
		"a higher number is supplied, only 4 are used. "
		"There is no upper limit on the numbers of points with midpoint quadrature. "
		"Note: assumes all elements of the mesh have a right-handed coordinate "
		"system; if this is not the case the integral will be incorrect.",
		integrand_field, coordinate_field, mesh,
		quadrature_rule, numbers_of_points, numbers_of_points_size))
	{
		cmzn_field_id field =
			cmzn_fieldmodule_create_field_mesh_integral(field_modify->get_field_module(),
				integrand_field, coordinate_field, mesh);
		cmzn_field_mesh_integral_id mesh_integral_field = cmzn_field_cast_mesh_integral(field);
		cmzn_field_mesh_integral_set_element_quadrature_rule(mesh_integral_field, quadrature_rule);
		cmzn_field_mesh_integral_set_numbers_of_points(mesh_integral_field,
			numbers_of_points_size, numbers_of_points);
		cmzn_field_mesh_integral_destroy(&mesh_integral_field);
		return_code = field_modify->update_field_and_deaccess(field);
	}
	cmzn_field_destroy(&integrand_field);
	cmzn_field_destroy(&coordinate_field);
	cmzn_mesh_destroy(&mesh);
	DEALLOCATE(numbers_of_points);
	return return_code;
}

/**
 * Converts <field> into type mesh_integral_squares (if it is not already) and allows its
 * contents to be modified.
 */
int define_Computed_field_type_mesh_integral_squares(struct Parse_state *state,
	void *field_modify_void, void *computed_field_mesh_operators_package_void)
{
	int return_code = 0;
	USE_PARAMETER(computed_field_mesh_operators_package_void);
	Computed_field_modify_data * field_modify =
		reinterpret_cast<Computed_field_modify_data *>(field_modify_void);
	cmzn_field_id integrand_field = 0;
	cmzn_field_id coordinate_field = 0;
	cmzn_mesh_id mesh = 0;
	int numbers_of_points_size = 1;
	int *numbers_of_points;
	ALLOCATE(numbers_of_points, int, numbers_of_points_size);
	numbers_of_points[0] = 1;
	cmzn_element_quadrature_rule quadrature_rule = CMZN_ELEMENT_QUADRATURE_RULE_GAUSSIAN;
	if (define_Computed_field_type_mesh_operator(state, field_modify, "mesh_integral_squares",
		"A mesh_integral_squares field is a specialisation of the mesh_integral "
		"field type that integrates the squares of the components of the integrand field. "
		"Note that the volume/area/length and weights are not squared in the integral. "
		"This field type supports least-squares optimisation by giving individual "
		"terms being squared and summed. ",
		integrand_field, coordinate_field, mesh,
		quadrature_rule, numbers_of_points, numbers_of_points_size))
	{
		cmzn_field_id field =
			cmzn_fieldmodule_create_field_mesh_integral_squares(field_modify->get_field_module(),
				integrand_field, coordinate_field, mesh);
		cmzn_field_mesh_integral_id mesh_integral_field = cmzn_field_cast_mesh_integral(field);
		cmzn_field_mesh_integral_set_element_quadrature_rule(mesh_integral_field, quadrature_rule);
		cmzn_field_mesh_integral_set_numbers_of_points(mesh_integral_field,
			numbers_of_points_size, numbers_of_points);
		cmzn_field_mesh_integral_destroy(&mesh_integral_field);
		return_code = field_modify->update_field_and_deaccess(field);
	}
	cmzn_field_destroy(&integrand_field);
	cmzn_field_destroy(&coordinate_field);
	cmzn_mesh_destroy(&mesh);
	DEALLOCATE(numbers_of_points);
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
			"mesh_integral", define_Computed_field_type_mesh_integral,
			computed_field_mesh_operators_package);
		return_code = Computed_field_package_add_type(computed_field_package,
			"mesh_integral_squares", define_Computed_field_type_mesh_integral_squares,
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
