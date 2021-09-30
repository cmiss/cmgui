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
#include "image_processing/computed_field_threshold_image_filter.h"
#include "general/enumerator_private.hpp"
#include "general/enumerator_private_app.h"
#include "image_processing/computed_field_threshold_image_filter_app.h"

const char computed_field_threshold_image_filter_type_string[] = "threshold_filter";

int cmzn_field_get_type_threshold_image_filter(struct Computed_field *field,
	struct Computed_field **source_field,
	enum cmzn_field_imagefilter_threshold_condition *condition,
	double *outsideValue, double *lowerValue,	double *upperValue);

int define_Computed_field_type_threshold_image_filter(struct Parse_state *state,
	void *field_modify_void, void *)
/*******************************************************************************
LAST MODIFIED : 8 December 2006

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_THRESHOLD_IMAGE_FILTER (if it is not
already) and allows its contents to be modified.
==============================================================================*/
{
	int return_code;
	struct Computed_field *source_field;
	Computed_field_modify_data *field_modify = static_cast<Computed_field_modify_data *>(field_modify_void);
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_threshold_image_filter);
	if ((state) && (field_modify))
	{
		return_code = 1;
		/* get valid parameters for projection field */
		source_field = (struct Computed_field *)NULL;

		/* default values */
		enum cmzn_field_imagefilter_threshold_condition condition = CMZN_FIELD_IMAGEFILTER_THRESHOLD_CONDITION_BELOW;
		double outside_value = 0.0;
		double below_value = 0.5;
		double above_value = 0.5;

		if ((NULL != field_modify->get_field()) &&
			(computed_field_threshold_image_filter_type_string ==
				Computed_field_get_type_string(field_modify->get_field())))
		{
			return_code =
				cmzn_field_get_type_threshold_image_filter(field_modify->get_field(), &source_field,
					&condition, &outside_value, &below_value, &above_value);
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
				"The threshold_filter field uses the itk::ThresholdImageFilter code to change or identify pixels based on whether they are above or below a particular intensity value. The <field> it operates on is usually a sample_texture field, based on a texture that has been created from image file(s).  To specify an intensity range to change use one of the three threshold modes: <below>, <above> or <outside>.  Pixels within the specified range are changed to the <outside_value> intensity, the other pixels are left unchanged.  For the <below> mode all pixels are changed that are below the <below_value>.  For the <above> mode all pixels are changed that are above the <above_value>.  For the <outside> mode all pixels are changed that are oustide the range <below_value> to <above_value> .  See a/testing/image_processing_2D for examples of using this field. For more information see the itk software guide.");

			/* field */
			set_source_field_data.computed_field_manager =
				field_modify->get_field_manager();
			set_source_field_data.conditional_function = Computed_field_is_scalar;
			set_source_field_data.conditional_function_user_data = (void *)NULL;
			Option_table_add_entry(option_table, "field", &source_field,
				&set_source_field_data, set_Computed_field_conditional);
			/* threshold_mode */
			OPTION_TABLE_ADD_ENUMERATOR(cmzn_field_imagefilter_threshold_condition)(option_table,
				&condition);
			/* outside_value */
			Option_table_add_double_entry(option_table, "outside_value",
				&outside_value);
			/* below_value */
			Option_table_add_double_entry(option_table, "below_value",
				&below_value);
			/* above_value */
			Option_table_add_double_entry(option_table, "above_value",
				&above_value);

			return_code = Option_table_multi_parse(option_table, state);
			DESTROY(Option_table)(&option_table);

			/* no errors,not asking for help */
			if (return_code)
			{
				if (!source_field)
				{
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_threshold_image_filter.  "
						"Missing source field");
					return_code = 0;
				}
			}
			if (return_code)
			{
				cmzn_field_id filter_field = cmzn_fieldmodule_create_field_imagefilter_threshold(
					field_modify->get_field_module(), source_field);
				cmzn_field_imagefilter_threshold_id imagefilter =
					cmzn_field_cast_imagefilter_threshold(filter_field);
				cmzn_field_imagefilter_threshold_set_condition(imagefilter, condition);
				cmzn_field_imagefilter_threshold_set_outside_value(imagefilter, outside_value);
				cmzn_field_imagefilter_threshold_set_lower_threshold(imagefilter, below_value);
				cmzn_field_imagefilter_threshold_set_upper_threshold(imagefilter, above_value);
				cmzn_field_imagefilter_threshold_destroy(&imagefilter);
				return_code = field_modify->define_field(filter_field);
			}

			if (!return_code)
			{
				if ((!state->current_token) ||
					(strcmp(PARSER_HELP_STRING, state->current_token)&&
						strcmp(PARSER_RECURSIVE_HELP_STRING, state->current_token)))
				{
					/* error */
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_threshold_image_filter.  Failed");
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
			"define_Computed_field_type_threshold_image_filter.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_threshold_image_filter */

int Computed_field_register_types_threshold_image_filter(
	struct Computed_field_package *computed_field_package)
/*******************************************************************************
LAST MODIFIED : 30 August 2006

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_register_types_threshold_image_filter);
	if (computed_field_package)
	{
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_threshold_image_filter_type_string,
			define_Computed_field_type_threshold_image_filter,
			Computed_field_package_get_simple_package(computed_field_package));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_register_types_threshold_image_filter.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_register_types_threshold_image_filter */

DEFINE_DEFAULT_OPTION_TABLE_ADD_ENUMERATOR_FUNCTION(cmzn_field_imagefilter_threshold_condition);

