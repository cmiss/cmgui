/* OpenCMISS-Cmgui Application
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */


#include "general/debug.h"
#include "general/message.h"
#include "command/parser.h"
#include "computed_field/computed_field_app.h"
#include "computed_field/computed_field_private.hpp"
#include "computed_field/computed_field_private_app.hpp"
#include "computed_field/computed_field_set.h"
#include "computed_field/computed_field_set_app.h"
#include "computed_field/computed_field_arithmetic_operators.h"
#include "opencmiss/zinc/fieldarithmeticoperators.h"

const char computed_field_power_type_string[] = "power";

const char computed_field_multiply_components_type_string[] = "multiply_components";

const char computed_field_divide_components_type_string[] = "divide_components";

const char computed_field_add_type_string[] = "add";

const char computed_field_scale_type_string[] = "scale";

const char computed_field_clamp_maximum_type_string[] = "clamp_maximum";

const char computed_field_clamp_minimum_type_string[] = "clamp_minimum";

const char computed_field_abs_type_string[] = "abs";

const char computed_field_exp_type_string[] = "exp";

const char computed_field_sqrt_type_string[] = "sqrt";

const char computed_field_log_type_string[] = "log";

const char computed_field_edit_mask_type_string[] = "edit_mask";

const char computed_field_offset_type_string[] = "offset";

int Computed_field_get_type_abs(struct Computed_field *field,
	struct Computed_field **source_field);

int Computed_field_get_type_exp(struct Computed_field *field,
	struct Computed_field **source_field);

int Computed_field_get_type_sqrt(struct Computed_field *field,
	struct Computed_field **source_field);

int Computed_field_get_type_log(struct Computed_field *field,
	struct Computed_field **source_field);

cmzn_field *cmzn_fieldmodule_create_field_edit_mask(cmzn_fieldmodule *field_module,
	cmzn_field *source_field, double *edit_mask);

int Computed_field_get_type_edit_mask(struct Computed_field *field,
	struct Computed_field **source_field, double **edit_mask);

cmzn_field *cmzn_fieldmodule_create_field_offset(cmzn_fieldmodule *fieldmodule,
	cmzn_field *source_field, double *offsets);

int Computed_field_get_type_offset(struct Computed_field *field,
	struct Computed_field **source_field, double **offsets);

cmzn_field *cmzn_fieldmodule_create_field_clamp_maximum(cmzn_fieldmodule *fieldmodule,
	cmzn_field *source_field, double *maximums);

int Computed_field_get_type_clamp_minimum(struct Computed_field *field,
	struct Computed_field **source_field, double **minimums);

cmzn_field *cmzn_fieldmodule_create_field_clamp_minimum(cmzn_fieldmodule *fieldmodule,
	cmzn_field *source_field, double *minimums);

int Computed_field_get_type_clamp_maximum(struct Computed_field *field,
	struct Computed_field **source_field, double **maximums);

cmzn_field *cmzn_fieldmodule_create_field_scale(cmzn_fieldmodule *field_module,
	cmzn_field *source_field, double *scale_factors);

int Computed_field_get_type_scale(cmzn_field *field,
	cmzn_field **source_field, double **scale_factors);

int Computed_field_get_type_power(struct Computed_field *field,
	struct Computed_field **source_field_one,
	struct Computed_field **source_field_two);

int Computed_field_get_type_multiply_components(struct Computed_field *field,
	struct Computed_field **source_field_one,
	struct Computed_field **source_field_two);

int Computed_field_get_type_divide_components(struct Computed_field *field,
	struct Computed_field **source_field_one,
	struct Computed_field **source_field_two);

cmzn_field *cmzn_fieldmodule_create_field_weighted_add(cmzn_fieldmodule *fieldmodule,
	cmzn_field *source_field_one, double scale_factor1,
	cmzn_field *source_field_two, double scale_factor2);

int Computed_field_get_type_weighted_add(cmzn_field *field,
	cmzn_field **source_field_one, FE_value *scale_factor1,
	cmzn_field **source_field_two, FE_value *scale_factor2);

int define_Computed_field_type_power(struct Parse_state *state,
	void *field_modify_void, void *)
/*******************************************************************************
LAST MODIFIED : 15 May 2008

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_POWER (if it is not
already) and allows its contents to be modified.
==============================================================================*/
{
	int return_code;
	struct Computed_field **source_fields;
	Computed_field_modify_data *field_modify = static_cast<Computed_field_modify_data *>(field_modify_void);
	struct Option_table *option_table;
	struct Set_Computed_field_array_data set_source_field_array_data;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_power);
	if ((state) && (field_modify))
	{
		return_code=1;
		/* get valid parameters for projection field */
		source_fields = (struct Computed_field **)NULL;
		if (ALLOCATE(source_fields, struct Computed_field *, 2))
		{
			source_fields[0] = (struct Computed_field *)NULL;
			source_fields[1] = (struct Computed_field *)NULL;
			if ((NULL != field_modify->get_field()) &&
				(computed_field_power_type_string ==
					Computed_field_get_type_string(field_modify->get_field())))
			{
				return_code=Computed_field_get_type_power(field_modify->get_field(),
					source_fields, source_fields + 1);
			}
			if (return_code)
			{
				/* must access objects for set functions */
				if (source_fields[0])
				{
					ACCESS(Computed_field)(source_fields[0]);
				}
				if (source_fields[1])
				{
					ACCESS(Computed_field)(source_fields[1]);
				}

				option_table = CREATE(Option_table)();
				/* fields */
				set_source_field_data.computed_field_manager=
					field_modify->get_field_manager();
				set_source_field_data.conditional_function=
					Computed_field_has_numerical_components;
				set_source_field_data.conditional_function_user_data=(void *)NULL;
				set_source_field_array_data.number_of_fields=2;
				set_source_field_array_data.conditional_data= &set_source_field_data;
				Option_table_add_entry(option_table,"fields",source_fields,
					&set_source_field_array_data,set_Computed_field_array);
				return_code=Option_table_multi_parse(option_table,state);
				/* no errors,not asking for help */
				if (return_code)
				{
					return_code = field_modify->define_field(
						cmzn_fieldmodule_create_field_power(field_modify->get_field_module(),
							source_fields[0], source_fields[1]));
				}
				if (!return_code)
				{
					if ((!state->current_token)||
						(strcmp(PARSER_HELP_STRING,state->current_token)&&
							strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
					{
						/* error */
						display_message(ERROR_MESSAGE,
							"define_Computed_field_type_power.  Failed");
					}
				}
				if (source_fields[0])
				{
					DEACCESS(Computed_field)(&source_fields[0]);
				}
				if (source_fields[1])
				{
					DEACCESS(Computed_field)(&source_fields[1]);
				}
				DESTROY(Option_table)(&option_table);
			}
			DEALLOCATE(source_fields);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"define_Computed_field_type_power.  Not enough memory");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_power.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_power */


int define_Computed_field_type_multiply_components(struct Parse_state *state,
	void *field_modify_void, void *)
/*******************************************************************************
LAST MODIFIED : 15 May 2008

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_MULTIPLY_COMPONENTS (if it is not
already) and allows its contents to be modified.
==============================================================================*/
{
	int return_code;
	struct Computed_field **source_fields;
	Computed_field_modify_data *field_modify = static_cast<Computed_field_modify_data *>(field_modify_void);
	struct Option_table *option_table;
	struct Set_Computed_field_array_data set_source_field_array_data;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_multiply_components);
	if ((state) && (field_modify))
	{
		return_code=1;
		/* get valid parameters for projection field */
		source_fields = (struct Computed_field **)NULL;
		if (ALLOCATE(source_fields, struct Computed_field *, 2))
		{
			source_fields[0] = (struct Computed_field *)NULL;
			source_fields[1] = (struct Computed_field *)NULL;
			if ((NULL != field_modify->get_field()) &&
				(computed_field_multiply_components_type_string ==
					Computed_field_get_type_string(field_modify->get_field())))
			{
				return_code=Computed_field_get_type_multiply_components(field_modify->get_field(),
					source_fields, source_fields + 1);
			}
			if (return_code)
			{
				/* must access objects for set functions */
				if (source_fields[0])
				{
					ACCESS(Computed_field)(source_fields[0]);
				}
				if (source_fields[1])
				{
					ACCESS(Computed_field)(source_fields[1]);
				}

				option_table = CREATE(Option_table)();
				/* fields */
				set_source_field_data.computed_field_manager=
					field_modify->get_field_manager();
				set_source_field_data.conditional_function=Computed_field_has_numerical_components;
				set_source_field_data.conditional_function_user_data=(void *)NULL;
				set_source_field_array_data.number_of_fields=2;
				set_source_field_array_data.conditional_data= &set_source_field_data;
				Option_table_add_entry(option_table,"fields",source_fields,
					&set_source_field_array_data,set_Computed_field_array);
				return_code=Option_table_multi_parse(option_table,state);
				/* no errors,not asking for help */
				if (return_code)
				{
					return_code = field_modify->define_field(
						cmzn_fieldmodule_create_field_multiply(field_modify->get_field_module(),
							source_fields[0], source_fields[1]));
				}
				if (!return_code)
				{
					if ((!state->current_token)||
						(strcmp(PARSER_HELP_STRING,state->current_token)&&
							strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
					{
						/* error */
						display_message(ERROR_MESSAGE,
							"define_Computed_field_type_multiply_components.  Failed");
					}
				}
				if (source_fields[0])
				{
					DEACCESS(Computed_field)(&source_fields[0]);
				}
				if (source_fields[1])
				{
					DEACCESS(Computed_field)(&source_fields[1]);
				}
				DESTROY(Option_table)(&option_table);
			}
			DEALLOCATE(source_fields);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"define_Computed_field_type_multiply_components.  Not enough memory");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_multiply_components.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_multiply_components */


int define_Computed_field_type_divide_components(struct Parse_state *state,
	void *field_modify_void, void *)
/*******************************************************************************
LAST MODIFIED : 15 May 2008

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_DIVIDE_COMPONENTS (if it is not
already) and allows its contents to be modified.
==============================================================================*/
{
	int return_code;
	struct Computed_field **source_fields;
	Computed_field_modify_data *field_modify = static_cast<Computed_field_modify_data *>(field_modify_void);
	struct Option_table *option_table;
	struct Set_Computed_field_array_data set_source_field_array_data;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_divide_components);
	if ((state) && (field_modify))
	{
		return_code=1;
		/* get valid parameters for projection field */
		source_fields = (struct Computed_field **)NULL;
		if (ALLOCATE(source_fields, struct Computed_field *, 2))
		{
			source_fields[0] = (struct Computed_field *)NULL;
			source_fields[1] = (struct Computed_field *)NULL;
			if ((NULL != field_modify->get_field()) &&
				(computed_field_divide_components_type_string ==
					Computed_field_get_type_string(field_modify->get_field())))
			{
				return_code=Computed_field_get_type_divide_components(field_modify->get_field(),
					source_fields, source_fields + 1);
			}
			if (return_code)
			{
				/* must access objects for set functions */
				if (source_fields[0])
				{
					ACCESS(Computed_field)(source_fields[0]);
				}
				if (source_fields[1])
				{
					ACCESS(Computed_field)(source_fields[1]);
				}

				option_table = CREATE(Option_table)();
				/* fields */
				set_source_field_data.computed_field_manager=
					field_modify->get_field_manager();
				set_source_field_data.conditional_function=Computed_field_has_numerical_components;
				set_source_field_data.conditional_function_user_data=(void *)NULL;
				set_source_field_array_data.number_of_fields=2;
				set_source_field_array_data.conditional_data= &set_source_field_data;
				Option_table_add_entry(option_table,"fields",source_fields,
					&set_source_field_array_data,set_Computed_field_array);
				return_code=Option_table_multi_parse(option_table,state);
				/* no errors,not asking for help */
				if (return_code)
				{
					return_code = field_modify->define_field(
						cmzn_fieldmodule_create_field_divide(field_modify->get_field_module(),
							source_fields[0], source_fields[1]));
				}
				if (!return_code)
				{
					if ((!state->current_token)||
						(strcmp(PARSER_HELP_STRING,state->current_token)&&
							strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
					{
						/* error */
						display_message(ERROR_MESSAGE,
							"define_Computed_field_type_divide_components.  Failed");
					}
				}
				if (source_fields[0])
				{
					DEACCESS(Computed_field)(&source_fields[0]);
				}
				if (source_fields[1])
				{
					DEACCESS(Computed_field)(&source_fields[1]);
				}
				DESTROY(Option_table)(&option_table);
			}
			DEALLOCATE(source_fields);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"define_Computed_field_type_divide_components.  Not enough memory");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_divide_components.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_divide_components */


int define_Computed_field_type_add(struct Parse_state *state,
	void *field_modify_void, void *)
/*******************************************************************************
LAST MODIFIED : 15 May 2008

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_ADD (if it is not
already) and allows its contents to be modified.
==============================================================================*/
{
	FE_value *scale_factors;
	int number_of_scale_factors,return_code;
	struct Computed_field **source_fields;
	Computed_field_modify_data *field_modify = static_cast<Computed_field_modify_data *>(field_modify_void);
	struct Option_table *option_table;
	struct Set_Computed_field_array_data set_source_field_array_data;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_add);
	if ((state) && (field_modify))
	{
		return_code=1;
		/* get valid parameters for projection field */
		source_fields = (struct Computed_field **)NULL;
		scale_factors=(FE_value *)NULL;
		if (ALLOCATE(source_fields, struct Computed_field *, 2)&&
			ALLOCATE(scale_factors, FE_value, 2))
		{
			source_fields[0] = (struct Computed_field *)NULL;
			source_fields[1] = (struct Computed_field *)NULL;
			scale_factors[0] = 1.0;
			scale_factors[1] = 1.0;
			if ((NULL != field_modify->get_field()) &&
				(computed_field_add_type_string ==
					Computed_field_get_type_string(field_modify->get_field())))
			{
				return_code=Computed_field_get_type_weighted_add(field_modify->get_field(),
					source_fields, scale_factors,
					source_fields + 1, scale_factors + 1);
			}
			if (return_code)
			{
				/* must access objects for set functions */
				if (source_fields[0])
				{
					ACCESS(Computed_field)(source_fields[0]);
				}
				if (source_fields[1])
				{
					ACCESS(Computed_field)(source_fields[1]);
				}

				option_table = CREATE(Option_table)();
				/* fields */
				set_source_field_data.computed_field_manager=
					field_modify->get_field_manager();
				set_source_field_data.conditional_function=Computed_field_has_numerical_components;
				set_source_field_data.conditional_function_user_data=(void *)NULL;
				set_source_field_array_data.number_of_fields=2;
				set_source_field_array_data.conditional_data= &set_source_field_data;
				Option_table_add_entry(option_table,"fields",source_fields,
					&set_source_field_array_data,set_Computed_field_array);
				number_of_scale_factors=2;
				Option_table_add_entry(option_table,"scale_factors",scale_factors,
					&number_of_scale_factors,set_FE_value_array);
				return_code=Option_table_multi_parse(option_table,state);
				/* no errors,not asking for help */
				if (return_code)
				{
					return_code = field_modify->define_field(
						cmzn_fieldmodule_create_field_weighted_add(field_modify->get_field_module(),
							source_fields[0], scale_factors[0],
							source_fields[1], scale_factors[1]));
				}
				if (!return_code)
				{
					if ((!state->current_token)||
						(strcmp(PARSER_HELP_STRING,state->current_token)&&
							strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
					{
						/* error */
						display_message(ERROR_MESSAGE,
							"define_Computed_field_type_add.  Failed");
					}
				}
				if (source_fields[0])
				{
					DEACCESS(Computed_field)(&source_fields[0]);
				}
				if (source_fields[1])
				{
					DEACCESS(Computed_field)(&source_fields[1]);
				}
				DESTROY(Option_table)(&option_table);
			}
			DEALLOCATE(source_fields);
			DEALLOCATE(scale_factors);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"define_Computed_field_type_add.  Not enough memory");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_add.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_add */


int define_Computed_field_type_scale(struct Parse_state *state,
	void *field_modify_void, void *)
/*******************************************************************************
LAST MODIFIED : 15 May 2008

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_SCALE (if it is not
already) and allows its contents to be modified.
==============================================================================*/
{
	const char *current_token;
	double *scale_factors, *temp_scale_factors;
	int i, number_of_scale_factors, previous_number_of_scale_factors, return_code;
	struct Computed_field *source_field;
	Computed_field_modify_data *field_modify = static_cast<Computed_field_modify_data *>(field_modify_void);
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_scale);
	if ((state) && (field_modify))
	{
		return_code = 1;
		/* get valid parameters for projection field */
		set_source_field_data.computed_field_manager =
			field_modify->get_field_manager();
		set_source_field_data.conditional_function =
			Computed_field_has_numerical_components;
		set_source_field_data.conditional_function_user_data = (void *)NULL;
		source_field = (struct Computed_field *)NULL;
		scale_factors = (double *)NULL;
		previous_number_of_scale_factors = 0;
		if ((NULL != field_modify->get_field()) &&
			(computed_field_scale_type_string ==
				Computed_field_get_type_string(field_modify->get_field())))
		{
			return_code =
				Computed_field_get_type_scale(field_modify->get_field(), &source_field, &scale_factors);
		}
		if (return_code)
		{
			if (source_field)
			{
				previous_number_of_scale_factors = source_field->number_of_components;
				ACCESS(Computed_field)(source_field);
			}
			if ((current_token=state->current_token) &&
				(!(strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))))
			{
				option_table = CREATE(Option_table)();
				Option_table_add_entry(option_table, "field", &source_field,
					&set_source_field_data, set_Computed_field_conditional);
				Option_table_add_entry(option_table, "scale_factors", scale_factors,
					&previous_number_of_scale_factors, set_double_vector);
				return_code = Option_table_multi_parse(option_table, state);
				DESTROY(Option_table)(&option_table);
			}
			/* parse the field ... */
			if (return_code && (current_token = state->current_token))
			{
				/* ... only if the "field" token is next */
				if (fuzzy_string_compare(current_token, "field"))
				{
					option_table = CREATE(Option_table)();
					/* field */
					Option_table_add_entry(option_table, "field", &source_field,
						&set_source_field_data, set_Computed_field_conditional);
					if (0 != (return_code = Option_table_parse(option_table, state)))
					{
						if (source_field)
						{
							number_of_scale_factors = source_field->number_of_components;
							if (REALLOCATE(temp_scale_factors, scale_factors, double,
								number_of_scale_factors))
							{
								scale_factors = temp_scale_factors;
								/* make any new scale_factors equal to 1.0 */
								for (i = previous_number_of_scale_factors;
									i < number_of_scale_factors; i++)
								{
									scale_factors[i] = 1.0;
								}
							}
							else
							{
								return_code = 0;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"define_Computed_field_type_scale.  Invalid field");
							return_code = 0;
						}
					}
					DESTROY(Option_table)(&option_table);
				}
			}
			if (return_code && !source_field)
			{
				display_message(ERROR_MESSAGE,
					"define_Computed_field_type_scale.  Must specify a source field before the scale_factors.");
				return_code = 0;
			}
			if (return_code && !source_field)
			{
				display_message(ERROR_MESSAGE,
					"define_Computed_field_type_scale.  "
					"You must specify the source field before the offsets.");
				return_code = 0;
			}
			/* parse the scale_factors */
			if (return_code&&state->current_token)
			{
				option_table = CREATE(Option_table)();
				number_of_scale_factors=source_field->number_of_components;
				Option_table_add_entry(option_table,"scale_factors",scale_factors,
					&number_of_scale_factors, set_double_vector);
				return_code=Option_table_multi_parse(option_table,state);
				DESTROY(Option_table)(&option_table);
			}
			/* no errors, not asking for help */
			if (return_code)
			{
				return_code = field_modify->define_field(
					cmzn_fieldmodule_create_field_scale(field_modify->get_field_module(),
						source_field, scale_factors));
			}
			if (!return_code)
			{
				if ((!state->current_token) ||
					(strcmp(PARSER_HELP_STRING, state->current_token) &&
						strcmp(PARSER_RECURSIVE_HELP_STRING, state->current_token)))
				{
					/* error */
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_scale.  Failed");
				}
			}
			if (source_field)
			{
				DEACCESS(Computed_field)(&source_field);
			}
		}
		DEALLOCATE(scale_factors);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_scale.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_scale */

int define_Computed_field_type_clamp_maximum(struct Parse_state *state,
	void *field_modify_void, void *)
/*******************************************************************************
LAST MODIFIED : 15 May 2008

class Computed_field_clamp_minimum : public Computed_field_core
{

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_CLAMP_MAXIMUM (if it is not
already) and allows its contents to be modified.
==============================================================================*/
{
	const char *current_token;
	double *maximums, *temp_maximums;
	int i, number_of_maximums, previous_number_of_maximums, return_code;
	struct Computed_field *source_field;
	Computed_field_modify_data *field_modify = static_cast<Computed_field_modify_data *>(field_modify_void);
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_clamp_maximum);
	if ((state) && (field_modify))
	{
		return_code=1;
		/* get valid parameters for projection field */
		set_source_field_data.computed_field_manager =
			field_modify->get_field_manager();
		set_source_field_data.conditional_function =
			Computed_field_has_numerical_components;
		set_source_field_data.conditional_function_user_data = (void *)NULL;
		source_field = (struct Computed_field *)NULL;
		maximums = (double *)NULL;
		previous_number_of_maximums = 0;
		if ((NULL != field_modify->get_field()) &&
			(computed_field_clamp_maximum_type_string ==
				Computed_field_get_type_string(field_modify->get_field())))
		{
			return_code = Computed_field_get_type_clamp_maximum(field_modify->get_field(),
				&source_field, &maximums);
		}
		if (return_code)
		{
			if (source_field)
			{
				previous_number_of_maximums = source_field->number_of_components;
				ACCESS(Computed_field)(source_field);
			}
			if ((current_token=state->current_token) &&
				(!(strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))))
			{
				option_table = CREATE(Option_table)();
				Option_table_add_entry(option_table,"field",&source_field,
					&set_source_field_data,set_Computed_field_conditional);
				Option_table_add_entry(option_table,"maximums",maximums,
					&previous_number_of_maximums,set_double_vector);
				return_code=Option_table_multi_parse(option_table,state);
				DESTROY(Option_table)(&option_table);
			}
			/* parse the field ... */
			if (return_code && (current_token = state->current_token))
			{
				/* ... only if the "field" token is next */
				if (fuzzy_string_compare(current_token, "field"))
				{
					option_table = CREATE(Option_table)();
					/* field */
					Option_table_add_entry(option_table, "field", &source_field,
						&set_source_field_data, set_Computed_field_conditional);
					if (0 != (return_code = Option_table_parse(option_table, state)))
					{
						if (source_field)
						{
							number_of_maximums = source_field->number_of_components;
							if (REALLOCATE(temp_maximums, maximums, double,
								number_of_maximums))
							{
								maximums = temp_maximums;
								/* make any new maximums equal to 1.0 */
								for (i = previous_number_of_maximums; i < number_of_maximums;
									i++)
								{
									maximums[i] = 1.0;
								}
							}
							else
							{
								return_code = 0;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"define_Computed_field_type_clamp_maximum.  Invalid field");
							return_code = 0;
						}
					}
					DESTROY(Option_table)(&option_table);
				}
			}
			if (return_code && !source_field)
			{
				display_message(ERROR_MESSAGE,
					"define_Computed_field_type_clamp_maximum.  "
					"You must specify the source field before the offsets.");
				return_code = 0;
			}
			/* parse the maximums */
			if (return_code && state->current_token)
			{
				option_table = CREATE(Option_table)();
				number_of_maximums = source_field->number_of_components;
				Option_table_add_entry(option_table, "maximums", maximums,
					&number_of_maximums, set_double_vector);
				return_code = Option_table_multi_parse(option_table, state);
				DESTROY(Option_table)(&option_table);
			}
			/* no errors, not asking for help */
			if (return_code)
			{
				return_code = field_modify->define_field(
					cmzn_fieldmodule_create_field_clamp_maximum(field_modify->get_field_module(),
						source_field, maximums));
			}
			if (!return_code)
			{
				if ((!state->current_token) ||
					(strcmp(PARSER_HELP_STRING, state->current_token) &&
						strcmp(PARSER_RECURSIVE_HELP_STRING, state->current_token)))
				{
					/* error */
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_clamp_maximum.  Failed");
				}
			}
			if (source_field)
			{
				DEACCESS(Computed_field)(&source_field);
			}
			DESTROY(Option_table)(&option_table);
		}
		DEALLOCATE(maximums);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_clamp_maximum.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_clamp_maximum */

int define_Computed_field_type_clamp_minimum(struct Parse_state *state,
	void *field_modify_void, void *)
/*******************************************************************************
LAST MODIFIED : 15 May 2008

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_CLAMP_MINIMUM (if it is not
already) and allows its contents to be modified.
==============================================================================*/
{
	const char *current_token;
	double *minimums, *temp_minimums;
	int i, number_of_minimums, previous_number_of_minimums, return_code;
	struct Computed_field *source_field;
	Computed_field_modify_data *field_modify = static_cast<Computed_field_modify_data *>(field_modify_void);
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_clamp_minimum);
	if ((state) && (field_modify))
	{
		return_code=1;
		/* get valid parameters for projection field */
		set_source_field_data.computed_field_manager =
			field_modify->get_field_manager();
		set_source_field_data.conditional_function =
			Computed_field_has_numerical_components;
		set_source_field_data.conditional_function_user_data = (void *)NULL;
		source_field = (struct Computed_field *)NULL;
		minimums = (double *)NULL;
		previous_number_of_minimums = 0;
		if ((NULL != field_modify->get_field()) &&
			(computed_field_clamp_minimum_type_string ==
				Computed_field_get_type_string(field_modify->get_field())))
		{
			return_code = Computed_field_get_type_clamp_minimum(field_modify->get_field(),
				&source_field, &minimums);
		}
		if (return_code)
		{
			if (source_field)
			{
				previous_number_of_minimums = source_field->number_of_components;
				ACCESS(Computed_field)(source_field);
			}
			if ((current_token=state->current_token) &&
				(!(strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))))
			{
				option_table = CREATE(Option_table)();
				Option_table_add_entry(option_table,"field",&source_field,
					&set_source_field_data,set_Computed_field_conditional);
				Option_table_add_entry(option_table,"minimums",minimums,
					&previous_number_of_minimums,set_double_vector);
				return_code=Option_table_multi_parse(option_table,state);
				DESTROY(Option_table)(&option_table);
			}
			/* parse the field ... */
			if (return_code && (current_token = state->current_token))
			{
				/* ... only if the "field" token is next */
				if (fuzzy_string_compare(current_token, "field"))
				{
					option_table = CREATE(Option_table)();
					/* field */
					Option_table_add_entry(option_table, "field", &source_field,
						&set_source_field_data, set_Computed_field_conditional);
					if (0 != (return_code = Option_table_parse(option_table, state)))
					{
						if (source_field)
						{
							number_of_minimums = source_field->number_of_components;
							if (REALLOCATE(temp_minimums, minimums, double,
								number_of_minimums))
							{
								minimums = temp_minimums;
								/* make any new minimums equal to 1.0 */
								for (i = previous_number_of_minimums; i < number_of_minimums;
									i++)
								{
									minimums[i] = 1.0;
								}
							}
							else
							{
								return_code = 0;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"define_Computed_field_type_clamp_minimum.  Invalid field");
							return_code = 0;
						}
					}
					DESTROY(Option_table)(&option_table);
				}
			}
			if (return_code && !source_field)
			{
				display_message(ERROR_MESSAGE,
					"define_Computed_field_type_clamp_minimum.  "
					"You must specify the source field before the offsets.");
				return_code = 0;
			}
			/* parse the minimums */
			if (return_code && state->current_token)
			{
				option_table = CREATE(Option_table)();
				number_of_minimums = source_field->number_of_components;
				Option_table_add_entry(option_table, "minimums", minimums,
					&number_of_minimums, set_double_vector);
				return_code = Option_table_multi_parse(option_table, state);
				DESTROY(Option_table)(&option_table);
			}
			/* no errors, not asking for help */
			if (return_code)
			{
				return_code = field_modify->define_field(
					cmzn_fieldmodule_create_field_clamp_minimum(field_modify->get_field_module(),
						source_field, minimums));
			}
			if (!return_code)
			{
				if ((!state->current_token) ||
					(strcmp(PARSER_HELP_STRING, state->current_token) &&
						strcmp(PARSER_RECURSIVE_HELP_STRING, state->current_token)))
				{
					/* error */
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_clamp_minimum.  Failed");
				}
			}
			if (source_field)
			{
				DEACCESS(Computed_field)(&source_field);
			}
			DESTROY(Option_table)(&option_table);
		}
		DEALLOCATE(minimums);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_clamp_minimum.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_clamp_minimum */


int define_Computed_field_type_offset(struct Parse_state *state,
	void *field_modify_void, void *)
/*******************************************************************************
LAST MODIFIED : 15 May 2008

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_OFFSET (if it is not
already) and allows its contents to be modified.
==============================================================================*/
{
	const char *current_token;
	double *offsets, *temp_offsets;
	int i, number_of_offsets, previous_number_of_offsets, return_code;
	struct Computed_field *source_field;
	Computed_field_modify_data *field_modify = static_cast<Computed_field_modify_data *>(field_modify_void);
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_offset);
	if ((state) && (field_modify))
	{
		return_code = 1;
		/* get valid parameters for projection field */
		set_source_field_data.computed_field_manager =
			field_modify->get_field_manager();
		set_source_field_data.conditional_function =
			Computed_field_has_numerical_components;
		set_source_field_data.conditional_function_user_data = (void *)NULL;
		source_field = (struct Computed_field *)NULL;
		offsets = (double *)NULL;
		previous_number_of_offsets = 0;
		if ((NULL != field_modify->get_field()) &&
			(computed_field_offset_type_string ==
				Computed_field_get_type_string(field_modify->get_field())))
		{
			return_code =
				Computed_field_get_type_offset(field_modify->get_field(), &source_field, &offsets);
		}
		if (return_code)
		{
			if (source_field)
			{
				previous_number_of_offsets = source_field->number_of_components;
				ACCESS(Computed_field)(source_field);
			}
			if ((current_token=state->current_token) &&
				(!(strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))))
			{
				option_table = CREATE(Option_table)();
				/* field */
				Option_table_add_entry(option_table, "field", &source_field,
					&set_source_field_data, set_Computed_field_conditional);
				/* offsets */
				Option_table_add_entry(option_table, "offsets", offsets,
					&previous_number_of_offsets, set_double_vector);
				return_code = Option_table_multi_parse(option_table, state);
				DESTROY(Option_table)(&option_table);
			}
			/* parse the field ... */
			if (return_code && (current_token = state->current_token))
			{
				/* ... only if the "field" token is next */
				if (fuzzy_string_compare(current_token, "field"))
				{
					option_table = CREATE(Option_table)();
					/* field */
					Option_table_add_entry(option_table, "field", &source_field,
						&set_source_field_data, set_Computed_field_conditional);
					if (0 != (return_code = Option_table_parse(option_table,state)))
					{
						if (source_field)
						{
							number_of_offsets = source_field->number_of_components;
							if (REALLOCATE(temp_offsets, offsets, double,
								number_of_offsets))
							{
								offsets = temp_offsets;
								/* set new offsets to 0.0 */
								for (i = previous_number_of_offsets; i < number_of_offsets; i++)
								{
									offsets[i] = 0.0;
								}
							}
							else
							{
								return_code = 0;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"define_Computed_field_type_offset.  Invalid field");
							return_code = 0;
						}
					}
					DESTROY(Option_table)(&option_table);
				}
			}
			if (return_code && !source_field)
			{
				display_message(ERROR_MESSAGE,
					"define_Computed_field_type_offset.  "
					"You must specify the source field before the offsets.");
				return_code = 0;
			}
			/* parse the offsets */
			if (return_code && state->current_token)
			{
				option_table = CREATE(Option_table)();
				/* offsets */
				number_of_offsets=source_field->number_of_components;
				Option_table_add_entry(option_table,"offsets",offsets,
					&number_of_offsets, set_double_vector);
				return_code=Option_table_multi_parse(option_table,state);
				DESTROY(Option_table)(&option_table);
			}
			/* no errors,not asking for help */
			if (return_code)
			{
				return_code = field_modify->define_field(
					cmzn_fieldmodule_create_field_offset(field_modify->get_field_module(),
						source_field, offsets));
			}
			if (!return_code)
			{
				if ((!state->current_token)||
					(strcmp(PARSER_HELP_STRING,state->current_token)&&
						strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
				{
					/* error */
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_offset.  Failed");
				}
			}
			if (source_field)
			{
				DEACCESS(Computed_field)(&source_field);
			}
		}
		DEALLOCATE(offsets);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_offset.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_offset */

int define_Computed_field_type_edit_mask(struct Parse_state *state,
	void *field_modify_void, void *)
/*******************************************************************************
LAST MODIFIED : 15 May 2008

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_EDIT_MASK (if it is not
already) and allows its contents to be modified.
==============================================================================*/
{
	const char *current_token;
	double *edit_mask, *temp_edit_mask;
	int i, number_of_edit_mask, previous_number_of_edit_mask, return_code;
	struct Computed_field *source_field;
	Computed_field_modify_data *field_modify = static_cast<Computed_field_modify_data *>(field_modify_void);
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_edit_mask);
	if ((state) && (field_modify))
	{
		return_code = 1;
		/* get valid parameters for projection field */
		set_source_field_data.computed_field_manager =
			field_modify->get_field_manager();
		set_source_field_data.conditional_function =
			Computed_field_has_numerical_components;
		set_source_field_data.conditional_function_user_data = (void *)NULL;
		source_field = (struct Computed_field *)NULL;
		edit_mask = (double *)NULL;
		previous_number_of_edit_mask = 0;
		if ((NULL != field_modify->get_field()) &&
			(computed_field_edit_mask_type_string ==
				Computed_field_get_type_string(field_modify->get_field())))
		{
			return_code =
				Computed_field_get_type_edit_mask(field_modify->get_field(), &source_field, &edit_mask);
		}
		if (return_code)
		{
			if (source_field)
			{
				previous_number_of_edit_mask = source_field->number_of_components;
				ACCESS(Computed_field)(source_field);
			}
			if ((current_token = state->current_token) &&
				(!(strcmp(PARSER_HELP_STRING, current_token) &&
					strcmp(PARSER_RECURSIVE_HELP_STRING, current_token))))
			{
				option_table = CREATE(Option_table)();
				Option_table_add_entry(option_table, "field", &source_field,
					&set_source_field_data, set_Computed_field_conditional);
				Option_table_add_entry(option_table, "edit_mask", edit_mask,
					&previous_number_of_edit_mask, set_double_vector);
				return_code = Option_table_multi_parse(option_table, state);
				DESTROY(Option_table)(&option_table);
			}
			/* parse the field ... */
			if (return_code && (current_token = state->current_token))
			{
				/* ... only if the "field" token is next */
				if (fuzzy_string_compare(current_token, "field"))
				{
					option_table = CREATE(Option_table)();
					/* field */
					Option_table_add_entry(option_table, "field", &source_field,
						&set_source_field_data, set_Computed_field_conditional);
					if (0 != (return_code = Option_table_parse(option_table, state)))
					{
						if (source_field)
						{
							number_of_edit_mask = source_field->number_of_components;
							if (REALLOCATE(temp_edit_mask, edit_mask, double,
								number_of_edit_mask))
							{
								edit_mask = temp_edit_mask;
								/* make any new edit_mask equal to 1.0 */
								for (i = previous_number_of_edit_mask; i < number_of_edit_mask;
									i++)
								{
									edit_mask[i] = 1.0;
								}
							}
							else
							{
								return_code = 0;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"define_Computed_field_type_edit_mask.  Invalid field");
							return_code = 0;
						}
					}
					DESTROY(Option_table)(&option_table);
				}
			}
			if (return_code && !source_field)
			{
				display_message(ERROR_MESSAGE,
					"define_Computed_field_type_edit_mask.  "
					"You must specify the source field before the offsets.");
				return_code = 0;
			}
			/* parse the edit_mask */
			if (return_code && state->current_token)
			{
				option_table = CREATE(Option_table)();
				number_of_edit_mask = source_field->number_of_components;
				Option_table_add_entry(option_table, "edit_mask", edit_mask,
					&number_of_edit_mask, set_double_vector);
				return_code = Option_table_multi_parse(option_table, state);
				DESTROY(Option_table)(&option_table);
			}
			/* no errors,not asking for help */
			if (return_code)
			{
				return_code = field_modify->define_field(
					cmzn_fieldmodule_create_field_edit_mask(field_modify->get_field_module(),
						source_field, edit_mask));
			}
			if (!return_code)
			{
				if ((!state->current_token) ||
					(strcmp(PARSER_HELP_STRING, state->current_token) &&
						strcmp(PARSER_RECURSIVE_HELP_STRING, state->current_token)))
				{
					/* error */
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_edit_mask.  Failed");
				}
			}
			if (source_field)
			{
				DEACCESS(Computed_field)(&source_field);
			}
			DESTROY(Option_table)(&option_table);
		}
		DEALLOCATE(edit_mask);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_edit_mask.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_edit_mask */


int define_Computed_field_type_log(struct Parse_state *state,
	void *field_modify_void, void *)
/*******************************************************************************
LAST MODIFIED : 15 May 2008

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_LOG (if it is not
already) and allows its contents to be modified.
==============================================================================*/
{
	int return_code;
	struct Computed_field **source_fields;
	Computed_field_modify_data *field_modify = static_cast<Computed_field_modify_data *>(field_modify_void);
	struct Option_table *option_table;
	struct Set_Computed_field_array_data set_source_field_array_data;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_log);
	if ((state) && (field_modify))
	{
		return_code=1;
		/* get valid parameters for projection field */
		source_fields = (struct Computed_field **)NULL;
		if (ALLOCATE(source_fields, struct Computed_field *, 1))
		{
			source_fields[0] = (struct Computed_field *)NULL;
			if ((NULL != field_modify->get_field()) &&
				(computed_field_log_type_string ==
					Computed_field_get_type_string(field_modify->get_field())))
			{
				return_code=Computed_field_get_type_log(field_modify->get_field(),
					source_fields);
			}
			if (return_code)
			{
				/* must access objects for set functions */
				if (source_fields[0])
				{
					ACCESS(Computed_field)(source_fields[0]);
				}

				option_table = CREATE(Option_table)();
				/* fields */
				set_source_field_data.computed_field_manager=
					field_modify->get_field_manager();
				set_source_field_data.conditional_function=
					Computed_field_has_numerical_components;
				set_source_field_data.conditional_function_user_data=(void *)NULL;
				set_source_field_array_data.number_of_fields=1;
				set_source_field_array_data.conditional_data= &set_source_field_data;
				Option_table_add_entry(option_table,"field",source_fields,
					&set_source_field_array_data,set_Computed_field_array);
				return_code=Option_table_multi_parse(option_table,state);
				/* no errors,not asking for help */
				if (return_code)
				{
					return_code = field_modify->define_field(
						cmzn_fieldmodule_create_field_log(field_modify->get_field_module(),
							source_fields[0]));
				}
				if (!return_code)
				{
					if ((!state->current_token)||
						(strcmp(PARSER_HELP_STRING,state->current_token)&&
							strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
					{
						/* error */
						display_message(ERROR_MESSAGE,
							"define_Computed_field_type_log.  Failed");
					}
				}
				if (source_fields[0])
				{
					DEACCESS(Computed_field)(&source_fields[0]);
				}
				DESTROY(Option_table)(&option_table);
			}
			DEALLOCATE(source_fields);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"define_Computed_field_type_log.  Not enough memory");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_log.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_log */


int define_Computed_field_type_sqrt(struct Parse_state *state,
	void *field_modify_void, void *)
/*******************************************************************************
LAST MODIFIED : 15 May 2008

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_SQRT (if it is not
already) and allows its contents to be modified.
==============================================================================*/
{
	int return_code;
	struct Computed_field **source_fields;
	Computed_field_modify_data *field_modify = static_cast<Computed_field_modify_data *>(field_modify_void);
	struct Option_table *option_table;
	struct Set_Computed_field_array_data set_source_field_array_data;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_sqrt);
	if ((state) && (field_modify))
	{
		return_code=1;
		/* get valid parameters for projection field */
		source_fields = (struct Computed_field **)NULL;
		if (ALLOCATE(source_fields, struct Computed_field *, 1))
		{
			source_fields[0] = (struct Computed_field *)NULL;
			if ((NULL != field_modify->get_field()) &&
				(computed_field_sqrt_type_string ==
					Computed_field_get_type_string(field_modify->get_field())))
		{
				return_code=Computed_field_get_type_sqrt(field_modify->get_field(),
					source_fields);
			}
			if (return_code)
			{
				/* must access objects for set functions */
				if (source_fields[0])
				{
					ACCESS(Computed_field)(source_fields[0]);
				}

				option_table = CREATE(Option_table)();
				/* fields */
				set_source_field_data.computed_field_manager=
					field_modify->get_field_manager();
				set_source_field_data.conditional_function=
					Computed_field_has_numerical_components;
				set_source_field_data.conditional_function_user_data=(void *)NULL;
				set_source_field_array_data.number_of_fields=1;
				set_source_field_array_data.conditional_data= &set_source_field_data;
				Option_table_add_entry(option_table,"field",source_fields,
					&set_source_field_array_data,set_Computed_field_array);
				return_code=Option_table_multi_parse(option_table,state);
				/* no errors,not asking for help */
				if (return_code)
				{
					return_code = field_modify->define_field(
						cmzn_fieldmodule_create_field_sqrt(field_modify->get_field_module(),
							source_fields[0]));
				}
				if (!return_code)
				{
					if ((!state->current_token)||
						(strcmp(PARSER_HELP_STRING,state->current_token)&&
							strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
					{
						/* error */
						display_message(ERROR_MESSAGE,
							"define_Computed_field_type_sqrt.  Failed");
					}
				}
				if (source_fields[0])
				{
					DEACCESS(Computed_field)(&source_fields[0]);
				}
				DESTROY(Option_table)(&option_table);
			}
			DEALLOCATE(source_fields);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"define_Computed_field_type_sqrt.  Not enough memory");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_sqrt.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_sqrt */


int define_Computed_field_type_exp(struct Parse_state *state,
	void *field_modify_void, void *)
/*******************************************************************************
LAST MODIFIED : 15 May 2008

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_EXP (if it is not
already) and allows its contents to be modified.
==============================================================================*/
{
	int return_code;
	struct Computed_field **source_fields;
	Computed_field_modify_data *field_modify = static_cast<Computed_field_modify_data *>(field_modify_void);
	struct Option_table *option_table;
	struct Set_Computed_field_array_data set_source_field_array_data;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_exp);
	if ((state) && (field_modify))
	{
		return_code=1;
		/* get valid parameters for projection field */
		source_fields = (struct Computed_field **)NULL;
		if (ALLOCATE(source_fields, struct Computed_field *, 1))
		{
			source_fields[0] = (struct Computed_field *)NULL;
			if ((NULL != field_modify->get_field()) &&
				(computed_field_exp_type_string ==
					Computed_field_get_type_string(field_modify->get_field())))
			{
				return_code=Computed_field_get_type_exp(field_modify->get_field(),
					source_fields);
			}
			if (return_code)
			{
				/* must access objects for set functions */
				if (source_fields[0])
				{
					ACCESS(Computed_field)(source_fields[0]);
				}

				option_table = CREATE(Option_table)();
				/* fields */
				set_source_field_data.computed_field_manager=
					field_modify->get_field_manager();
				set_source_field_data.conditional_function=
					Computed_field_has_numerical_components;
				set_source_field_data.conditional_function_user_data=(void *)NULL;
				set_source_field_array_data.number_of_fields=1;
				set_source_field_array_data.conditional_data= &set_source_field_data;
				Option_table_add_entry(option_table,"field",source_fields,
					&set_source_field_array_data,set_Computed_field_array);
				return_code=Option_table_multi_parse(option_table,state);
				/* no errors,not asking for help */
				if (return_code)
				{
					return_code = field_modify->define_field(
						cmzn_fieldmodule_create_field_exp(field_modify->get_field_module(),
							source_fields[0]));
				}
				if (!return_code)
				{
					if ((!state->current_token)||
						(strcmp(PARSER_HELP_STRING,state->current_token)&&
							strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
					{
						/* error */
						display_message(ERROR_MESSAGE,
							"define_Computed_field_type_exp.  Failed");
					}
				}
				if (source_fields[0])
				{
					DEACCESS(Computed_field)(&source_fields[0]);
				}
				DESTROY(Option_table)(&option_table);
			}
			DEALLOCATE(source_fields);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"define_Computed_field_type_exp.  Not enough memory");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_exp.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_exp */



int define_Computed_field_type_abs(struct Parse_state *state,
	void *field_modify_void, void *)
/*******************************************************************************
DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_EXP (if it is not
already) and allows its contents to be modified.
==============================================================================*/
{
	int return_code;
	struct Computed_field **source_fields;
	Computed_field_modify_data *field_modify = static_cast<Computed_field_modify_data *>(field_modify_void);
	struct Option_table *option_table;
	struct Set_Computed_field_array_data set_source_field_array_data;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_abs);
	if ((state) && (field_modify))
	{
		return_code=1;
		/* get valid parameters for projection field */
		source_fields = (struct Computed_field **)NULL;
		if (ALLOCATE(source_fields, struct Computed_field *, 1))
		{
			source_fields[0] = (struct Computed_field *)NULL;
			if ((NULL != field_modify->get_field()) &&
				(computed_field_abs_type_string ==
					Computed_field_get_type_string(field_modify->get_field())))
			{
				return_code=Computed_field_get_type_abs(field_modify->get_field(),
					source_fields);
			}
			if (return_code)
			{
				/* must access objects for set functions */
				if (source_fields[0])
				{
					ACCESS(Computed_field)(source_fields[0]);
				}

				option_table = CREATE(Option_table)();
				/* fields */
				set_source_field_data.computed_field_manager=
					field_modify->get_field_manager();
				set_source_field_data.conditional_function=
					Computed_field_has_numerical_components;
				set_source_field_data.conditional_function_user_data=(void *)NULL;
				set_source_field_array_data.number_of_fields=1;
				set_source_field_array_data.conditional_data= &set_source_field_data;
				Option_table_add_entry(option_table,"field",source_fields,
					&set_source_field_array_data,set_Computed_field_array);
				return_code=Option_table_multi_parse(option_table,state);
				/* no errors,not asking for help */
				if (return_code)
				{
					return_code = field_modify->define_field(
						cmzn_fieldmodule_create_field_abs(field_modify->get_field_module(),
							source_fields[0]));
				}
				if (!return_code)
				{
					if ((!state->current_token)||
						(strcmp(PARSER_HELP_STRING,state->current_token)&&
							strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
					{
						/* error */
						display_message(ERROR_MESSAGE,
							"define_Computed_field_type_abs.  Failed");
					}
				}
				if (source_fields[0])
				{
					DEACCESS(Computed_field)(&source_fields[0]);
				}
				DESTROY(Option_table)(&option_table);
			}
			DEALLOCATE(source_fields);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"define_Computed_field_type_abs.  Not enough memory");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_abs.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_abs */

int Computed_field_register_types_arithmetic_operators(
	struct Computed_field_package *computed_field_package)
/*******************************************************************************
DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_register_types_arithmetic_operators);
	if (computed_field_package)
	{
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_power_type_string,
			define_Computed_field_type_power,
			Computed_field_package_get_simple_package(computed_field_package));
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_multiply_components_type_string,
			define_Computed_field_type_multiply_components,
			Computed_field_package_get_simple_package(computed_field_package));
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_divide_components_type_string,
			define_Computed_field_type_divide_components,
			Computed_field_package_get_simple_package(computed_field_package));
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_add_type_string,
			define_Computed_field_type_add,
			Computed_field_package_get_simple_package(computed_field_package));
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_scale_type_string,
			define_Computed_field_type_scale,
			Computed_field_package_get_simple_package(computed_field_package));
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_clamp_maximum_type_string,
			define_Computed_field_type_clamp_maximum,
			Computed_field_package_get_simple_package(computed_field_package));
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_clamp_minimum_type_string,
			define_Computed_field_type_clamp_minimum,
			Computed_field_package_get_simple_package(computed_field_package));
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_offset_type_string,
			define_Computed_field_type_offset,
			Computed_field_package_get_simple_package(computed_field_package));
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_edit_mask_type_string,
			define_Computed_field_type_edit_mask,
			Computed_field_package_get_simple_package(computed_field_package));
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_log_type_string,
			define_Computed_field_type_log,
			Computed_field_package_get_simple_package(computed_field_package));
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_sqrt_type_string,
			define_Computed_field_type_sqrt,
			Computed_field_package_get_simple_package(computed_field_package));
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_exp_type_string,
			define_Computed_field_type_exp,
			Computed_field_package_get_simple_package(computed_field_package));
		return_code = Computed_field_package_add_type(computed_field_package,
			computed_field_abs_type_string,
			define_Computed_field_type_abs,
			Computed_field_package_get_simple_package(computed_field_package));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_register_types_arithmetic_operators.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_register_types_arithmetic_operators */
