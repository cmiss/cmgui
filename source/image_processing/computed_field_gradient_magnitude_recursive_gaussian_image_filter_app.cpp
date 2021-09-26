/* OpenCMISS-Cmgui Application
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#include "opencmiss/zinc/fieldimageprocessing.h"
#include "general/debug.h"
#include "general/message.h"
#include "command/parser.h"
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_app.h"
#include "computed_field/computed_field_private.hpp"
#include "computed_field/computed_field_private_app.hpp"
#include "computed_field/computed_field_set.h"
#include "computed_field/computed_field_set_app.h"
#include "image_processing/computed_field_gradient_magnitude_recursive_gaussian_image_filter.h"

const char computed_field_gradient_magnitude_recursive_gaussian_image_filter_type_string[] = "gradient_magnitude_recursive_gaussian_filter";

int cmzn_field_get_type_gradient_magnitude_recursive_gaussian_image_filter(struct Computed_field *field,
	struct Computed_field **source_field, double *sigma);

int define_Computed_field_type_gradient_magnitude_recursive_gaussian_image_filter(struct Parse_state *state,
	void *field_modify_void, void *)
/*******************************************************************************
LAST MODIFIED : 18 October 2006

DESCRIPTION :
Converts <field> into type GRADIENT_MAGNITUDE_RECURSIVE_GAUSSIAN (if it is not
already) and allows its contents to be modified.
==============================================================================*/
{
	int return_code;
	double sigma;
	struct Computed_field *source_field;
	Computed_field_modify_data *field_modify = static_cast<Computed_field_modify_data *>(field_modify_void);
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_gradient_magnitude_recursive_gaussian_image_filter);
	if ((state) && (field_modify))
	{
		return_code = 1;
		/* get valid parameters for projection field */
		source_field = (struct Computed_field *)NULL;
		sigma = 2.0;

		if ((NULL != field_modify->get_field()) &&
			(computed_field_gradient_magnitude_recursive_gaussian_image_filter_type_string ==
				Computed_field_get_type_string(field_modify->get_field())))
		{
			return_code =
				cmzn_field_get_type_gradient_magnitude_recursive_gaussian_image_filter(field_modify->get_field(), &source_field,
					&sigma);
		}
		if (return_code)
		{
			/* must access objects for set functions */
			if (source_field)
			{
				ACCESS(Computed_field)(source_field);
			}

			option_table = CREATE(Option_table)();
			Option_table_add_help(option_table,
				"The gradient_magnitude_recursive_filter field uses the itk::GradientMagnitudeRecursiveImageFilter code to compute the magnitude of the image gradient at each location in the field. It is useful for identifying regions where the pixel intensities change rapidly.  The <field> it operates on is usually a sample_texture field, based on a texture that has been created from image file(s).  The filter first smooths the image using a discrete gaussian image subfilter before calculating the gradient and magnitudes.  Increasing <sigma> increases the width of the gaussian distribution used during the smoothing and hence the number of pixels used to calculate the weighted average. This smooths the image more.  See a/testing/image_processing_2D for an example of using this field.  For more information see the itk software guide.");


			/* field */
			set_source_field_data.computed_field_manager =
				field_modify->get_field_manager();
			set_source_field_data.conditional_function = Computed_field_is_scalar;
			set_source_field_data.conditional_function_user_data = (void *)NULL;
			Option_table_add_entry(option_table, "field", &source_field,
				&set_source_field_data, set_Computed_field_conditional);
			/* sigma */
			Option_table_add_double_entry(option_table, "sigma",&sigma);

			return_code = Option_table_multi_parse(option_table, state);
			DESTROY(Option_table)(&option_table);

			/* no errors,not asking for help */
			if (return_code)
			{
				if (!source_field)
				{
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_gradient_magnitude_recursive_gaussian_image_filter.  "
						"Missing source field");
					return_code = 0;
				}
			}
			if (return_code)
			{
				return_code = field_modify->define_field(
					cmzn_fieldmodule_create_field_imagefilter_gradient_magnitude_recursive_gaussian(
						field_modify->get_field_module(),
						source_field, sigma));
			}

			if (!return_code)
			{
				if ((!state->current_token) ||
					(strcmp(PARSER_HELP_STRING, state->current_token)&&
						strcmp(PARSER_RECURSIVE_HELP_STRING, state->current_token)))
				{
					/* error */
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_gradient_magnitude_recursive_gaussian_image_filter.  Failed");
				}
			}
			if (source_field)
			{
				DEACCESS(Computed_field)(&source_field);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_gradient_magnitude_recursive_gaussian_image_filter.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_gradient_magnitude_recursive_gaussian_image_filter */

int Computed_field_register_types_gradient_magnitude_recursive_gaussian_image_filter(
	struct Computed_field_package *computed_field_package)
/*******************************************************************************
LAST MODIFIED : 18 October 2006

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_register_types_gradient_magnitude_recursive_gaussian_image_filter);
	if (computed_field_package)
	{
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_gradient_magnitude_recursive_gaussian_image_filter_type_string,
			define_Computed_field_type_gradient_magnitude_recursive_gaussian_image_filter,
			Computed_field_package_get_simple_package(computed_field_package));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_register_types_gradient_magnitude_recursive_gaussian_image_filter.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_register_types_gradient_magnitude_recursive_gaussian_image_filter */
