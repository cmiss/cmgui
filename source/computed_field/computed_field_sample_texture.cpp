/*******************************************************************************
FILE : computed_field_sample_texture.c

LAST MODIFIED : 6 July 2000

DESCRIPTION :
Implements a computed_field which maintains a graphics transformation 
equivalent to the scene_viewer assigned to it.
==============================================================================*/
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_private.h"
#include "general/debug.h"
#include "graphics/texture.h"
#include "graphics/scene_viewer.h"
#include "user_interface/message.h"
#include "computed_field/computed_field_sample_texture.h"

struct Computed_field_sample_texture_package 
{
	struct MANAGER(Computed_field) *computed_field_manager;
	struct MANAGER(Texture) *texture_manager;
};

struct Computed_field_sample_texture_type_specific_data
{
	float minimum, maximum;
	struct Texture *texture;
	struct Computed_field_sample_texture_package *package;
};

static char computed_field_sample_texture_type_string[] = "sample_texture";

char *Computed_field_sample_texture_type_string(void)
/*******************************************************************************
LAST MODIFIED : 6 July 2000

DESCRIPTION :
Return the static type string which identifies this type.
==============================================================================*/
{

	ENTER(Computed_field_sample_texture_type_string);
	LEAVE;

	return (computed_field_sample_texture_type_string);
} /* Computed_field_sample_texture_type_string */

static int Computed_field_sample_texture_clear_type_specific(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 6 July 2000

DESCRIPTION :
Clear the type specific data used by this type.
==============================================================================*/
{
	int return_code;
	struct Computed_field_sample_texture_type_specific_data *data;

	ENTER(Computed_field_sample_texture_clear_type_specific);
	if (field && (data = 
		(struct Computed_field_sample_texture_type_specific_data *)
		field->type_specific_data))
	{
		if (data->texture)
		{
			DEACCESS(Texture)(&(data->texture));
		}
		DEALLOCATE(field->type_specific_data);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_sample_texture_clear_type_specific.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_sample_texture_clear_type_specific */

static void *Computed_field_sample_texture_copy_type_specific(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 6 July 2000

DESCRIPTION :
Copy the type specific data used by this type.
==============================================================================*/
{
	struct Computed_field_sample_texture_type_specific_data *destination,
		*source;

	ENTER(Computed_field_sample_texture_copy_type_specific);
	if (field && (source = 
		(struct Computed_field_sample_texture_type_specific_data *)
		field->type_specific_data))
	{
		if (ALLOCATE(destination,
			struct Computed_field_sample_texture_type_specific_data, 1))
		{
			destination->texture = ACCESS(Texture)(source->texture);
			destination->minimum = source->minimum;
			destination->maximum = source->maximum;
			destination->package = source->package;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_sample_texture_copy_type_specific.  "
				"Unable to allocate memory.");
			destination = NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_sample_texture_copy_type_specific.  "
			"Invalid arguments.");
		destination = NULL;
	}
	LEAVE;

	return (destination);
} /* Computed_field_sample_texture_copy_type_specific */

#define Computed_field_sample_texture_clear_cache_type_specific \
   (Computed_field_clear_cache_type_specific_function)NULL
/*******************************************************************************
LAST MODIFIED : 4 July 2000

DESCRIPTION :
This function is not needed for this type.
==============================================================================*/

static int Computed_field_sample_texture_type_specific_contents_match(
	struct Computed_field *field, struct Computed_field *other_computed_field)
/*******************************************************************************
LAST MODIFIED : 4 July 2000

DESCRIPTION :
Compare the type specific data
==============================================================================*/
{
	int return_code;
	struct Computed_field_sample_texture_type_specific_data *data,
		*other_data;

	ENTER(Computed_field_sample_texture_type_specific_contents_match);
	if (field && other_computed_field && (data = 
		(struct Computed_field_sample_texture_type_specific_data *)
		field->type_specific_data) && (other_data =
		(struct Computed_field_sample_texture_type_specific_data *)
		other_computed_field->type_specific_data))
	{
		if ((data->texture == other_data->texture) &&
			(data->minimum == other_data->minimum) &&
			(data->maximum == other_data->maximum))
		{
			return_code = 1;
		}
		else
		{
			return_code = 0;
		}
	}
	else
	{
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_sample_texture_type_specific_contents_match */

#define Computed_field_sample_texture_is_defined_in_element \
	Computed_field_default_is_defined_in_element
/*******************************************************************************
LAST MODIFIED : 4 July 2000

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

#define Computed_field_sample_texture_is_defined_at_node \
	Computed_field_default_is_defined_at_node
/*******************************************************************************
LAST MODIFIED : 4 July 2000

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

#define Computed_field_sample_texture_has_numerical_components \
	Computed_field_default_has_numerical_components
/*******************************************************************************
LAST MODIFIED : 4 July 2000

DESCRIPTION :
Window projection does have numerical components.
==============================================================================*/

static int Computed_field_sample_texture_evaluate_cache_at_node(
	struct Computed_field *field, struct FE_node *node)
/*******************************************************************************
LAST MODIFIED : 4 July 2000

DESCRIPTION :
Evaluate the fields cache at the node.
==============================================================================*/
{
	double texture_values[3];
	int return_code;
	struct Computed_field_sample_texture_type_specific_data *data;

	ENTER(Computed_field_sample_texture_evaluate_cache_at_node);
	if (field && Computed_field_has_at_least_2_components(field, NULL) 
		&& node && 
		(data = (struct Computed_field_sample_texture_type_specific_data *)
		field->type_specific_data))
	{
		/* 1. Precalculate any source fields that this field depends on */
		if (return_code = 
			Computed_field_evaluate_source_fields_cache_at_node(field, node))
		{
			/* 2. Calculate the field */
			/* Could generalise to 1D and 3D textures */
			Texture_get_pixel_values(data->texture,
				field->source_fields[0]->values[0],
				field->source_fields[0]->values[1], texture_values);
			if (data->minimum == 0.0)
			{
				if (data->maximum == 1.0)
				{
					field->values[0] =  texture_values[0];
					field->values[1] =  texture_values[1];
					field->values[2] =  texture_values[2];
				}
				else
				{
					field->values[0] =  texture_values[0] * data->maximum;
					field->values[1] =  texture_values[1] * data->maximum;
					field->values[2] =  texture_values[2] * data->maximum;
				}
			}
			else
			{
				field->values[0] =  (texture_values[0] - data->minimum) 
					* (data->maximum - data->minimum);
				field->values[1] =  (texture_values[1] - data->minimum) 
					* (data->maximum - data->minimum);
				field->values[2] =  (texture_values[2] - data->minimum) 
					* (data->maximum - data->minimum);
			}
			field->derivatives_valid = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_sample_texture_evaluate_cache_at_node.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_sample_texture_evaluate_cache_at_node */

static int Computed_field_sample_texture_evaluate_cache_in_element(
	struct Computed_field *field, struct FE_element *element, FE_value *xi,
	struct FE_element *top_level_element,int calculate_derivatives)
/*******************************************************************************
LAST MODIFIED : 4 July 2000

DESCRIPTION :
Evaluate the fields cache at the node.
==============================================================================*/
{
	double texture_values[3];
	int return_code;
	struct Computed_field_sample_texture_type_specific_data *data;

	ENTER(Computed_field_sample_texture_evaluate_cache_in_element);
	if (field && Computed_field_has_at_least_2_components(field, NULL) && 
		element && xi 
		&& (data = (struct Computed_field_sample_texture_type_specific_data *)
		field->type_specific_data))
	{
		/* 1. Precalculate any source fields that this field depends on */
		if (return_code = 
			Computed_field_evaluate_source_fields_cache_in_element(field, element,
				xi, top_level_element, calculate_derivatives))
		{
			/* 2. Calculate the field */
			/* Could generalise to 1D and 3D textures */
			Texture_get_pixel_values(data->texture,
				field->source_fields[0]->values[0],
				field->source_fields[0]->values[1], texture_values);
			if (data->minimum == 0.0)
			{
				if (data->maximum == 1.0)
				{
					field->values[0] =  texture_values[0];
					field->values[1] =  texture_values[1];
					field->values[2] =  texture_values[2];
				}
				else
				{
					field->values[0] =  texture_values[0] * data->maximum;
					field->values[1] =  texture_values[1] * data->maximum;
					field->values[2] =  texture_values[2] * data->maximum;
				}
			}
			else
			{
				field->values[0] =  (texture_values[0] - data->minimum) 
					* (data->maximum - data->minimum);
				field->values[1] =  (texture_values[1] - data->minimum) 
					* (data->maximum - data->minimum);
				field->values[2] =  (texture_values[2] - data->minimum) 
					* (data->maximum - data->minimum);
			}
			field->derivatives_valid = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_sample_texture_evaluate_cache_in_element.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_sample_texture_evaluate_cache_in_element */

#define Computed_field_sample_texture_evaluate_as_string_at_node \
	Computed_field_default_evaluate_as_string_at_node
/*******************************************************************************
LAST MODIFIED : 4 July 2000

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_sample_texture_evaluate_as_string_in_element \
	Computed_field_default_evaluate_as_string_in_element
/*******************************************************************************
LAST MODIFIED : 4 July 2000

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_sample_texture_set_values_at_node \
   (Computed_field_set_values_at_node_function)NULL
/*******************************************************************************
LAST MODIFIED : 4 July 2000

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_sample_texture_set_values_in_element \
   (Computed_field_set_values_in_element_function)NULL
/*******************************************************************************
LAST MODIFIED : 4 July 2000

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_sample_texture_get_native_discretization_in_element \
	Computed_field_default_get_native_discretization_in_element
/*******************************************************************************
LAST MODIFIED : 4 July 2000

DESCRIPTION :
Inherit result from first source field.
==============================================================================*/

#define Computed_field_sample_texture_find_element_xi \
   (Computed_field_find_element_xi_function)NULL
/*******************************************************************************
LAST MODIFIED : 4 July 2000

DESCRIPTION :
Not implemented yet.
==============================================================================*/

static int list_Computed_field_sample_texture(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 4 July 2000

DESCRIPTION :
==============================================================================*/
{
	char *texture_name;
	int return_code;
	struct Computed_field_sample_texture_type_specific_data *data;

	ENTER(List_Computed_field_sample_texture);
	if (field && (data = 
		(struct Computed_field_sample_texture_type_specific_data *)
		field->type_specific_data))
	{
		display_message(INFORMATION_MESSAGE,
			"    texture coordinate field : %s\n",field->source_fields[0]->name);
		if (return_code=GET_NAME(Texture)(data->texture,&texture_name))
		{
			display_message(INFORMATION_MESSAGE,
				"    texture : %s\n",texture_name);
			DEALLOCATE(texture_name);
		}
		display_message(INFORMATION_MESSAGE,"    minimum : %f\n",data->minimum);
		display_message(INFORMATION_MESSAGE,"    maximum : %f\n",data->maximum);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_sample_texture.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_sample_texture */

static int list_Computed_field_sample_texture_commands(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 4 July 2000

DESCRIPTION :
==============================================================================*/
{
	char *texture_name;
	int return_code;
	struct Computed_field_sample_texture_type_specific_data *data;

	ENTER(list_Computed_field_sample_texture_commands);
	if (field && (data = 
		(struct Computed_field_sample_texture_type_specific_data *)
		field->type_specific_data))
	{
		display_message(INFORMATION_MESSAGE,
			" coordinates %s",field->source_fields[0]->name);
		if (return_code=GET_NAME(Texture)(data->texture,&texture_name))
		{
			display_message(INFORMATION_MESSAGE,
				" texture %s",texture_name);
			DEALLOCATE(texture_name);
		}
		display_message(INFORMATION_MESSAGE," minimum %f",data->minimum);
		display_message(INFORMATION_MESSAGE," maximum %f",data->maximum);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_sample_texture_commands.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_sample_texture_commands */

int Computed_field_set_type_sample_texture(struct Computed_field *field,
	struct Computed_field *texture_coordinate_field, struct Texture *texture,
	float minimum, float maximum,
	struct Computed_field_sample_texture_package *package)
/*******************************************************************************
LAST MODIFIED : 6 July 2000

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_SAMPLE_TEXTURE with the supplied
texture.  Sets the number of components to 3.
The returned values are scaled so that they range from <minimum> to <maximum>.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
{
	int number_of_source_fields,return_code;
	struct Computed_field **source_fields;
	struct Computed_field_sample_texture_type_specific_data *data;

	ENTER(Computed_field_set_type_sample_texture);
	if (field&&texture&&(3>=texture_coordinate_field->number_of_components))
	{
		return_code=1;
		/* 1. make dynamic allocations for any new type-specific data */
		number_of_source_fields=1;
		if (ALLOCATE(source_fields,struct Computed_field *,number_of_source_fields)&&
			ALLOCATE(data,struct Computed_field_sample_texture_type_specific_data, 1))
		{
			/* 2. free current type-specific data */
			Computed_field_clear_type(field);
			/* 3. establish the new type */
			field->type=COMPUTED_FIELD_NEW_TYPES;
			field->type_string = Computed_field_sample_texture_type_string();
			field->number_of_components = 3;
			source_fields[0]=ACCESS(Computed_field)(texture_coordinate_field);
			field->source_fields=source_fields;
			field->number_of_source_fields=number_of_source_fields;			
			field->type_specific_data = (void *)data;
			data->texture = ACCESS(Texture)(texture);
			data->minimum = minimum;
			data->maximum = maximum;
			data->package = package;

			/* Set all the methods */
			field->computed_field_clear_type_specific_function =
				Computed_field_sample_texture_clear_type_specific;
			field->computed_field_copy_type_specific_function =
				Computed_field_sample_texture_copy_type_specific;
			field->computed_field_clear_cache_type_specific_function =
				Computed_field_sample_texture_clear_cache_type_specific;
			field->computed_field_type_specific_contents_match_function =
				Computed_field_sample_texture_type_specific_contents_match;
			field->computed_field_is_defined_in_element_function =
				Computed_field_sample_texture_is_defined_in_element;
			field->computed_field_is_defined_at_node_function =
				Computed_field_sample_texture_is_defined_at_node;
			field->computed_field_has_numerical_components_function =
				Computed_field_sample_texture_has_numerical_components;
			field->computed_field_evaluate_cache_at_node_function =
				Computed_field_sample_texture_evaluate_cache_at_node;
			field->computed_field_evaluate_cache_in_element_function =
				Computed_field_sample_texture_evaluate_cache_in_element;
			field->computed_field_evaluate_as_string_at_node_function =
				Computed_field_sample_texture_evaluate_as_string_at_node;
			field->computed_field_evaluate_as_string_in_element_function =
				Computed_field_sample_texture_evaluate_as_string_in_element;
			field->computed_field_set_values_at_node_function =
				Computed_field_sample_texture_set_values_at_node;
			field->computed_field_set_values_in_element_function =
				Computed_field_sample_texture_set_values_in_element;
			field->computed_field_get_native_discretization_in_element_function =
				Computed_field_sample_texture_get_native_discretization_in_element;
			field->computed_field_find_element_xi_function =
				Computed_field_sample_texture_find_element_xi;
			field->list_Computed_field_function = 
				list_Computed_field_sample_texture;
			field->list_Computed_field_commands_function = 
				list_Computed_field_sample_texture_commands;
		}
		else
		{
			DEALLOCATE(source_fields);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_set_type_sample_texture.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_sample_texture */

int Computed_field_get_type_sample_texture(struct Computed_field *field,
	struct Computed_field **texture_coordinate_field, struct Texture **texture,
	float *minimum, float *maximum)
/*******************************************************************************
LAST MODIFIED : 6 July 2000

DESCRIPTION :
If the field is of type COMPUTED_FIELD_SAMPLE_TEXTURE, the 
<texture_coordinate_field>, <texture>, <minimum> and <maximum> used by it are 
returned.
==============================================================================*/
{
	int return_code;
	struct Computed_field_sample_texture_type_specific_data *data;

	ENTER(Computed_field_get_type_sample_texture);
	if (field&&(COMPUTED_FIELD_NEW_TYPES==field->type)&&
		(field->type_string==Computed_field_sample_texture_type_string())
		&&(data = 
		(struct Computed_field_sample_texture_type_specific_data *)
		field->type_specific_data)&&texture_coordinate_field&&texture)
	{
		*texture_coordinate_field = field->source_fields[0];
		*texture = data->texture;
		*minimum = data->minimum;
		*maximum = data->maximum;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_sample_texture.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_sample_texture */

static int define_Computed_field_type_sample_texture(struct Parse_state *state,
	void *field_void,void *computed_field_sample_texture_package_void)
/*******************************************************************************
LAST MODIFIED : 6 July 2000

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_SAMPLE_TEXTURE (if it is not 
already) and allows its contents to be modified.
==============================================================================*/
{
	float minimum, maximum;
	int return_code;
	struct Computed_field *field,*texture_coordinate_field;
	struct Computed_field_sample_texture_package 
		*computed_field_sample_texture_package;
	struct Texture *texture;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_sample_texture);
	if (state&&(field=(struct Computed_field *)field_void)&&
		(computed_field_sample_texture_package=
		(struct Computed_field_sample_texture_package *)
		computed_field_sample_texture_package_void))
	{
		return_code=1;
		/* get valid parameters for projection field */
		texture_coordinate_field = (struct Computed_field *)NULL;
		texture = (struct Texture *)NULL;
		if (computed_field_sample_texture_type_string ==
			Computed_field_get_type_string(field))
		{
			return_code=Computed_field_get_type_sample_texture(field,
				&texture_coordinate_field, &texture, &minimum, &maximum);
		}
		else
		{
			minimum = 0.0;
			maximum = 1.0;
			if (!((texture_coordinate_field=
				FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
				Computed_field_has_at_least_2_components,(void *)NULL,
				computed_field_sample_texture_package->computed_field_manager))))
			{
				if (strcmp(PARSER_HELP_STRING,state->current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token))
				{
					/* This is only a failure if we aren't asking for help */
					display_message(ERROR_MESSAGE,
						"At least one field of 3 components must exist for a sample_texture field.");
					return_code = 0;
				}
			}
		}
		if (return_code)
		{
			/* must access objects for set functions */
			if (texture_coordinate_field)
			{
				ACCESS(Computed_field)(texture_coordinate_field);
			}
			if (texture)
			{
				ACCESS(Texture)(texture);
			}

			option_table = CREATE(Option_table)();
			/* coordinates */
			set_source_field_data.computed_field_manager=
				computed_field_sample_texture_package->computed_field_manager;
			set_source_field_data.conditional_function=Computed_field_has_at_least_2_components;
			set_source_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"coordinates",&texture_coordinate_field,
				&set_source_field_data,set_Computed_field_conditional);
			/* maximum */
			Option_table_add_entry(option_table,"maximum",&maximum,
				NULL,set_float);
			/* minimum */
			Option_table_add_entry(option_table,"minimum",&minimum,
				NULL,set_float);
			/* texture */
			Option_table_add_entry(option_table,"texture",&texture,
				computed_field_sample_texture_package->texture_manager,
				set_Texture);
			return_code=Option_table_multi_parse(option_table,state);
			/* no errors,not asking for help */
			if (return_code)
			{
				return_code = Computed_field_set_type_sample_texture(field,
					texture_coordinate_field, texture, minimum, maximum,
					computed_field_sample_texture_package);
			}
			if (!return_code)
			{
				if ((!state->current_token)||
					(strcmp(PARSER_HELP_STRING,state->current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
				{
					/* error */
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_sample_texture.  Failed");
				}
			}
			if (texture_coordinate_field)
			{
				DEACCESS(Computed_field)(&texture_coordinate_field);
			}
			if (texture)
			{
				DEACCESS(Texture)(&texture);
			}
			DESTROY(Option_table)(&option_table);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_sample_texture.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_sample_texture */

int Computed_field_register_type_sample_texture(
	struct Computed_field_package *computed_field_package, 
	struct MANAGER(Texture) *texture_manager)
/*******************************************************************************
LAST MODIFIED : 6 July 2000

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	static struct Computed_field_sample_texture_package 
		computed_field_sample_texture_package;

	ENTER(Computed_field_register_type_sample_texture);
	if (computed_field_package && texture_manager)
	{
		computed_field_sample_texture_package.computed_field_manager =
			Computed_field_package_get_computed_field_manager(
				computed_field_package);
		computed_field_sample_texture_package.texture_manager =
			texture_manager;
		return_code = Computed_field_package_add_type(computed_field_package,
			Computed_field_sample_texture_type_string(), 
			define_Computed_field_type_sample_texture,
			&computed_field_sample_texture_package);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_register_type_sample_texture.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_register_type_sample_texture */

