
/******************************************************************
  FILE : computed_field_iteration_threshold.c

  LAST MODIFIED: 18 February 2004
  
  DESCRIPTION: Implement image segmentation operation.
===================================================================*/
#include <math.h>
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_find_xi.h"
#include "computed_field/computed_field_private.h"
#include "computed_field/computed_field_set.h"
#include "image_processing/image_cache.h"
#include "general/image_utilities.h"
#include "general/debug.h"
#include "general/mystring.h"
#include "user_interface/message.h"
#include "image_processing/computed_field_iteration_threshold.h"

#define my_Min(x,y) ((x) <= (y) ? (x) : (y))
#define my_Max(x,y) ((x) <= (y) ? (y) : (x))


struct Computed_field_iteration_threshold_package
/*******************************************************************************
LAST MODIFIED : 17 February 2004

DESCRIPTION :
A container for objects required to define fields in this module.
==============================================================================*/
{
	struct MANAGER(Computed_field) *computed_field_manager;
	struct Cmiss_region *root_region;
	struct User_interface *user_interface;
};



struct Computed_field_iteration_threshold_type_specific_data
{

	float cached_time;
	int element_dimension;
	struct Cmiss_region *region;
	struct User_interface *user_interface;
	struct Image_cache *image;
	struct MANAGER(Computed_field) *computed_field_manager;
	void *computed_field_manager_callback_id;
};
static char computed_field_iteration_threshold_type_string[] = "iteration_threshold";

int Computed_field_is_type_iteration_threshold(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 18 December 2003

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_is_type_iteration_threshold);
	if (field)
	{
		return_code =
		  (field->type_string == computed_field_iteration_threshold_type_string);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_is_type_iteration_threshold.  Missing field");
		return_code = 0;
	}

	return (return_code);
} /* Computed_field_is_type_iteration_threshold */

static void Computed_field_iteration_threshold_field_change(
	struct MANAGER_MESSAGE(Computed_field) *message, void *field_void)
/*******************************************************************************
LAST MODIFIED : 5 December 2003

DESCRIPTION :
Manager function called back when either of the source fields change so that
we know to invalidate the image cache.
==============================================================================*/
{
	struct Computed_field *field;
	struct Computed_field_iteration_threshold_type_specific_data *data;

	ENTER(Computed_field_iteration_threshold_source_field_change);
	if (message && (field = (struct Computed_field *)field_void) && (data =
		(struct Computed_field_iteration_threshold_type_specific_data *)
		field->type_specific_data))
	{
		switch (message->change)
		{
			case MANAGER_CHANGE_OBJECT_NOT_IDENTIFIER(Computed_field):
			case MANAGER_CHANGE_OBJECT(Computed_field):
			{
				if (Computed_field_depends_on_Computed_field_in_list(
					field->source_fields[0], message->changed_object_list) ||
					Computed_field_depends_on_Computed_field_in_list(
					field->source_fields[1], message->changed_object_list))
				{
					if (data->image)
					{
						data->image->valid = 0;
					}
				}
			} break;
			case MANAGER_CHANGE_ADD(Computed_field):
			case MANAGER_CHANGE_REMOVE(Computed_field):
			case MANAGER_CHANGE_IDENTIFIER(Computed_field):
			{
				/* do nothing */
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_iteration_threshold_source_field_change.  "
			"Invalid arguments.");
	}
	LEAVE;
} /* Computed_field_iteration_threshold_source_field_change */

static int Computed_field_iteration_threshold_clear_type_specific(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 18 December 2003

DESCRIPTION :
Clear the type specific data used by this type.
==============================================================================*/
{
	int return_code;
	struct Computed_field_iteration_threshold_type_specific_data *data;

	ENTER(Computed_field_iteration_threshold_clear_type_specific);
	if (field && (data =
		(struct Computed_field_iteration_threshold_type_specific_data *)
		field->type_specific_data))
	{
		if (data->region)
		{
			DEACCESS(Cmiss_region)(&data->region);
		}
		if (data->image)
		{
			DEACCESS(Image_cache)(&data->image);
		}
		if (data->computed_field_manager && data->computed_field_manager_callback_id)
		{
			MANAGER_DEREGISTER(Computed_field)(
				data->computed_field_manager_callback_id,
				data->computed_field_manager);
		}
		DEALLOCATE(field->type_specific_data);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_iteration_threshold_clear_type_specific.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_iteration_threshold_clear_type_specific */

static void *Computed_field_iteration_threshold_copy_type_specific(
	struct Computed_field *source_field, struct Computed_field *destination_field)
/*******************************************************************************
LAST MODIFIED : 18 December 2003

DESCRIPTION :
Copy the type specific data used by this type.
==============================================================================*/
{
	struct Computed_field_iteration_threshold_type_specific_data *destination,
		*source;

	ENTER(Computed_field_iteration_threshold_copy_type_specific);
	if (source_field && destination_field && (source =
		(struct Computed_field_iteration_threshold_type_specific_data *)
		source_field->type_specific_data))
	{
		if (ALLOCATE(destination,
			struct Computed_field_iteration_threshold_type_specific_data, 1))
		{
			destination->cached_time = source->cached_time;
			destination->region = ACCESS(Cmiss_region)(source->region);
			destination->element_dimension = source->element_dimension;
			destination->user_interface = source->user_interface;
			destination->computed_field_manager = source->computed_field_manager;
			destination->computed_field_manager_callback_id =
				MANAGER_REGISTER(Computed_field)(
				Computed_field_iteration_threshold_field_change, (void *)destination_field,
				destination->computed_field_manager);
			if (source->image)
			{
				destination->image = ACCESS(Image_cache)(CREATE(Image_cache)());
				Image_cache_update_dimension(destination->image,
					source->image->dimension, source->image->depth,
					source->image->sizes, source->image->minimums,
					source->image->maximums);
			}
			else
			{
				destination->image = (struct Image_cache *)NULL;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_iteration_threshold_copy_type_specific.  "
				"Unable to allocate memory.");
			destination = NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_iteration_threshold_copy_type_specific.  "
			"Invalid arguments.");
		destination = NULL;
	}
	LEAVE;

	return (destination);
} /* Computed_field_iteration_threshold_copy_type_specific */

int Computed_field_iteration_threshold_clear_cache_type_specific
   (struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 18 December 2003

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	struct Computed_field_iteration_threshold_type_specific_data *data;

	ENTER(Computed_field_iteration_threshold_clear_type_specific);
	if (field && (data =
		(struct Computed_field_iteration_threshold_type_specific_data *)
		field->type_specific_data))
	{
		if (data->image)
		{
			/* data->image->valid = 0; */
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_iteration_threshold_clear_type_specific.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_iteration_threshold_clear_type_specific */

static int Computed_field_iteration_threshold_type_specific_contents_match(
	struct Computed_field *field, struct Computed_field *other_computed_field)
/*******************************************************************************
LAST MODIFIED : 18 December 2003

DESCRIPTION :
Compare the type specific data
==============================================================================*/
{
	int return_code;
	struct Computed_field_iteration_threshold_type_specific_data *data,
		*other_data;

	ENTER(Computed_field_iteration_threshold_type_specific_contents_match);
	if (field && other_computed_field && (data =
		(struct Computed_field_iteration_threshold_type_specific_data *)
		field->type_specific_data) && (other_data =
		(struct Computed_field_iteration_threshold_type_specific_data *)
		other_computed_field->type_specific_data))
	{
		if (data->image && other_data->image &&
			(data->image->dimension == other_data->image->dimension) &&
			(data->image->depth == other_data->image->depth))
		{
			/* Check sizes and minimums and maximums */
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
} /* Computed_field_iteration_threshold_type_specific_contents_match */

#define Computed_field_iteration_threshold_is_defined_in_element \
	Computed_field_default_is_defined_in_element
/*******************************************************************************
LAST MODIFIED : 18 December 2003

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

#define Computed_field_iteration_threshold_is_defined_at_node \
	Computed_field_default_is_defined_at_node
/*******************************************************************************
LAST MODIFIED : 18 December 2003

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

#define Computed_field_iteration_threshold_has_numerical_components \
	Computed_field_default_has_numerical_components
/*******************************************************************************
LAST MODIFIED : 18 December 2003

DESCRIPTION :
Window projection does have numerical components.
==============================================================================*/

#define Computed_field_iteration_threshold_not_in_use \
	(Computed_field_not_in_use_function)NULL
/*******************************************************************************
LAST MODIFIED : 18 December 2003

DESCRIPTION :
No special criteria.
==============================================================================*/

static int Image_cache_iteration_threshold(struct Image_cache *image)
/*******************************************************************************
LAST MODIFIED : 18 December 2003

DESCRIPTION :
Perform a automatic thresholding operation on the image cache.
==============================================================================*/
{
        char *storage;
	FE_value *data_index, *result_index, T0, T, T1, T2, *gray_img, *gray1_img, *gray2_img;
	int i, k, return_code, storage_size, counter, counter1, counter2;


	ENTER(Image_cache_iteration_threshold);
	if (image && (image->dimension > 0) && (image->depth > 0))
	{
		return_code = 1;
		T = 0.0;
		T0 = 0.0;
		T1 = 0.0;
		T2 = 0.0;
		counter1 = 0; /*the size of the support set of segmented result1*/
		counter2 = 0; /*the size of the support set of segmented result2*/

		/* Allocate a new storage block for our data */
		storage_size = image->depth;
		for (i = 0 ; i < image->dimension ; i++)
		{
			storage_size *= image->sizes[i];
		}
		counter = storage_size/image->depth; /*the size of gray scale image*/
		if (ALLOCATE(storage, char, storage_size * sizeof(FE_value))&&
                        ALLOCATE(gray_img, FE_value, counter)&&
			ALLOCATE(gray1_img, FE_value, counter)&&
			ALLOCATE(gray2_img, FE_value, counter))
		{

			result_index = (FE_value *)storage;
			for (i = 0 ; i < storage_size ; i++)
			{
				*result_index = 0.0;
				result_index++;
			}

			data_index = (FE_value *)image->data;

			result_index = (FE_value *)storage;
			/*compute the intensity image*/
			for (i = 0; return_code && i < counter ; i++)
			{

		        	gray_img[i] = *(data_index);
			        data_index += image->depth;
			        result_index += image->depth;
			}

			/*compute the intensity mean value*/
                        for (i = 0; i < counter; i++)
			{
                                T += gray_img[i]/counter;
			}
			T0 = T;
                        /* perform the initial thresholding by choosing the mean as threshold value*/
			for (i = 0; i < counter; i++)
			{
                                if (gray_img[i] <= T)
			        {
			               gray1_img[i] = gray_img[i];
				       gray2_img[i] = 0.0;
				       T1 += gray1_img[i];
				       counter1 ++;
			        }
			        else
			        {
                                       gray1_img[i] = 0.0;
				       gray2_img[i] = gray_img[i];
				       T2 += gray2_img[i];
				       counter2 ++;
			        }
			}
			T1 = T1/counter1;
			T2 = T2/counter2;
			T = (T1 + T2)/2;
			counter1 = 0;
			counter2 = 0;
			/*adjust the threshold value*/
			while (T != T0)
			{
			         T0 = T;
				 for (i = 0; i < counter; i++)
			         {
                                         if (gray_img[i] <= T)
			                 {
			                         gray1_img[i] = gray_img[i];

				                 gray2_img[i] = 0.0;
				                 T1 += gray1_img[i];
				                 counter1 ++;
			                 }
			                 else
			                 {
                                                 gray1_img[i] = 0.0;
				                 gray2_img[i] = gray_img[i];
				                 T2 += gray2_img[i];
				                 counter2 ++;
			                 }
			         }
			         T1 = T1/counter1;
			         T2 = T2/counter2;
			         T = (T1 + T2)/2;
				 counter1 = 0;
				 counter2 = 2;
				 if ((T-T0) <= 0.0001 && (T-T0) >= -0.0001) break;
			}
			for (i = (counter - 1); i >= 0; i--)
			{
			         data_index -= image->depth;
				 result_index -= image->depth;
                                 if (gray_img[i] <= T)
				 {
                                          for (k = 0; k < image->depth; k++)
					           result_index[k] = 0.0;
				 }
				 else
				 {
				          for (k = 0; k < image->depth; k++)

						   result_index[k] = 1.0;
				 }
			}

			if (return_code)
			{
				DEALLOCATE(image->data);
				image->data = storage;
				image->valid = 1;

			}
			else
			{
				DEALLOCATE(storage);
			}
			DEALLOCATE(gray_img);
			DEALLOCATE(gray1_img);
			DEALLOCATE(gray2_img);

		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Image_cache_image_segment.  Not enough memory");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE, "Image_cache_image_segment.  "
			"Invalid arguments.");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Image_cache_iteration_threshold */


static int Computed_field_iteration_threshold_evaluate_cache_at_node(
	struct Computed_field *field, struct FE_node *node, FE_value time)
/*******************************************************************************
LAST MODIFIED : 18 December 2003

DESCRIPTION :
Evaluate the fields cache at the node.
==============================================================================*/
{
	int return_code;
	struct Computed_field_iteration_threshold_type_specific_data *data;

	ENTER(Computed_field_iteration_threshold_evaluate_cache_at_node);
	if (field && node &&
		(data = (struct Computed_field_iteration_threshold_type_specific_data *)field->type_specific_data))
	{
		return_code = 1;
		/* 1. Precalculate the Image_cache */
		if (!data->image->valid)
		{
			return_code = Image_cache_update_from_fields(data->image, field->source_fields[0],
				field->source_fields[1], data->element_dimension, data->region,
				data->user_interface);
			/* 2. Perform image processing operation */
			return_code = Image_cache_iteration_threshold(data->image);
		}
		/* 3. Evaluate texture coordinates and copy image to field */
		Computed_field_evaluate_cache_at_node(field->source_fields[1],
			node, time);
		Image_cache_evaluate_field(data->image,field);

	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_iteration_threshold_evaluate_cache_at_node.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_iteration_threshold_evaluate_cache_at_node */

static int Computed_field_iteration_threshold_evaluate_cache_in_element(
	struct Computed_field *field, struct FE_element *element, FE_value *xi,
	FE_value time, struct FE_element *top_level_element,int calculate_derivatives)
/*******************************************************************************
LAST MODIFIED : 18 December 2003

DESCRIPTION :
Evaluate the fields cache at the node.
==============================================================================*/
{
	int return_code;
	struct Computed_field_iteration_threshold_type_specific_data *data;

	ENTER(Computed_field_iteration_threshold_evaluate_cache_in_element);
	USE_PARAMETER(calculate_derivatives);
	if (field && element && xi && (field->number_of_source_fields > 0) &&
		(field->number_of_components == field->source_fields[0]->number_of_components) &&
		(data = (struct Computed_field_iteration_threshold_type_specific_data *) field->type_specific_data) &&
		data->image && (field->number_of_components == data->image->depth))
	{
		return_code = 1;
		/* 1. Precalculate the Image_cache */
		if (!data->image->valid)
		{
			return_code = Image_cache_update_from_fields(data->image, field->source_fields[0],
				field->source_fields[1], data->element_dimension, data->region,
				data->user_interface);
			/* 2. Perform image processing operation */
			return_code = Image_cache_iteration_threshold(data->image);
		}
		/* 3. Evaluate texture coordinates and copy image to field */
		Computed_field_evaluate_cache_in_element(field->source_fields[1],
			element, xi, time, top_level_element, /*calculate_derivatives*/0);
		Image_cache_evaluate_field(data->image,field);

	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_iteration_threshold_evaluate_cache_in_element.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_iteration_threshold_evaluate_cache_in_element */

#define Computed_field_iteration_threshold_evaluate_as_string_at_node \
	Computed_field_default_evaluate_as_string_at_node
/*******************************************************************************
LAST MODIFIED : 18 December 2003

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_iteration_threshold_evaluate_as_string_in_element \
	Computed_field_default_evaluate_as_string_in_element
/*******************************************************************************
LAST MODIFIED : 18 December 2003

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_iteration_threshold_set_values_at_node \
   (Computed_field_set_values_at_node_function)NULL
/*******************************************************************************
LAST MODIFIED : 18 December 2003

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_iteration_threshold_set_values_in_element \
   (Computed_field_set_values_in_element_function)NULL
/*******************************************************************************
LAST MODIFIED : 18 December 2003

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_iteration_threshold_get_native_discretization_in_element \
	Computed_field_default_get_native_discretization_in_element
/*******************************************************************************
LAST MODIFIED : 18 December 2003

DESCRIPTION :
Inherit result from first source field.
==============================================================================*/

#define Computed_field_iteration_threshold_find_element_xi \
   (Computed_field_find_element_xi_function)NULL
/*******************************************************************************
LAST MODIFIED : 18 December 2003

DESCRIPTION :
Not implemented yet.
==============================================================================*/

static int list_Computed_field_iteration_threshold(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 4 December 2003

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(List_Computed_field_iteration_threshold);
	if (field)
	{
		display_message(INFORMATION_MESSAGE,
			"    source field : %s\n",field->source_fields[0]->name);
		display_message(INFORMATION_MESSAGE,
			"    texture coordinate field : %s\n",field->source_fields[1]->name);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_iteration_threshold.  Invalid field");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_iteration_threshold */

static char *Computed_field_iteration_threshold_get_command_string(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 4 December 2003

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string, *field_name;
	char temp_string[40], temp_string1[40], temp_string2[40], temp_string3[40];
	int error;
	struct Computed_field_iteration_threshold_type_specific_data *data;

	ENTER(Computed_field_iteration_threshold_get_command_string);
	command_string = (char *)NULL;
	if (field && (field->type_string==computed_field_iteration_threshold_type_string)
		&& (data = (struct Computed_field_iteration_threshold_type_specific_data *)
		field->type_specific_data) )
	{
		error = 0;
		append_string(&command_string,
			computed_field_iteration_threshold_type_string, &error);
		append_string(&command_string, " field ", &error);
		if (GET_NAME(Computed_field)(field->source_fields[0], &field_name))
		{
			make_valid_token(&field_name);
			append_string(&command_string, field_name, &error);
			DEALLOCATE(field_name);
		}
		append_string(&command_string, " texture_coordinate_field ", &error);
		if (GET_NAME(Computed_field)(field->source_fields[1], &field_name))
		{
			make_valid_token(&field_name);
			append_string(&command_string, field_name, &error);
			DEALLOCATE(field_name);
		}
		sprintf(temp_string, " dimension %d", data->image->dimension);
		append_string(&command_string, temp_string, &error);

		sprintf(temp_string1, " sizes %d %d",
		                    data->image->sizes[0], data->image->sizes[1]);
		append_string(&command_string, temp_string1, &error);

		sprintf(temp_string2, " minimums %f %f",
		                    data->image->minimums[0], data->image->minimums[1]);
		append_string(&command_string, temp_string2, &error);

		sprintf(temp_string3, " maximums %f %f",
		                    data->image->maximums[0], data->image->maximums[1]);
		append_string(&command_string, temp_string3, &error);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_iteration_threshold_get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_iteration_threshold_get_command_string */

#define Computed_field_iteration_threshold_has_multiple_times \
	Computed_field_default_has_multiple_times
/*******************************************************************************
LAST MODIFIED : 4 December 2003

DESCRIPTION :
Works out whether time influences the field.
==============================================================================*/

int Computed_field_set_type_iteration_threshold(struct Computed_field *field,
	struct Computed_field *source_field,
	struct Computed_field *texture_coordinate_field,
	int dimension, int *sizes, FE_value *minimums, FE_value *maximums,
	int element_dimension, struct MANAGER(Computed_field) *computed_field_manager,
	struct Cmiss_region *region, struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 4 December 2003

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_iteration_threshold with the supplied
fields, <source_field> and <texture_coordinate_field>.  The <dimension> is the
size of the <sizes>, <minimums> and <maximums> vectors and should be less than
or equal to the number of components in the <texture_coordinate_field>.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
{
	int depth, number_of_source_fields, return_code;
	struct Computed_field **source_fields;
	struct Computed_field_iteration_threshold_type_specific_data *data;

	ENTER(Computed_field_set_type_iteration_threshold);
	if (field && source_field && texture_coordinate_field &&
		(depth = source_field->number_of_components) &&
		(dimension <= texture_coordinate_field->number_of_components) &&
		region && user_interface)
	{
		return_code=1;
		/* 1. make dynamic allocations for any new type-specific data */
		number_of_source_fields=2;
		data = (struct Computed_field_iteration_threshold_type_specific_data *)NULL;
		if (ALLOCATE(source_fields, struct Computed_field *, number_of_source_fields) &&
			ALLOCATE(data, struct Computed_field_iteration_threshold_type_specific_data, 1) &&
			(data->image = ACCESS(Image_cache)(CREATE(Image_cache)())) &&
			Image_cache_update_dimension(
			data->image, dimension, depth, sizes, minimums, maximums) &&
			Image_cache_update_data_storage(data->image))
		{
			/* 2. free current type-specific data */
			Computed_field_clear_type(field);
			/* 3. establish the new type */
			field->type_string = computed_field_iteration_threshold_type_string;
			field->number_of_components = source_field->number_of_components;
			source_fields[0]=ACCESS(Computed_field)(source_field);
			source_fields[1]=ACCESS(Computed_field)(texture_coordinate_field);
			field->source_fields=source_fields;
			field->number_of_source_fields=number_of_source_fields;
			data->element_dimension = element_dimension;
			data->region = ACCESS(Cmiss_region)(region);
			data->user_interface = user_interface;
			data->computed_field_manager = computed_field_manager;
			data->computed_field_manager_callback_id =
				MANAGER_REGISTER(Computed_field)(
				Computed_field_iteration_threshold_field_change, (void *)field,
				computed_field_manager);

			field->type_specific_data = data;

			/* Set all the methods */
			COMPUTED_FIELD_ESTABLISH_METHODS(iteration_threshold);
		}
		else
		{
			DEALLOCATE(source_fields);
			if (data)
			{
				if (data->image)
				{
					DESTROY(Image_cache)(&data->image);
				}
				DEALLOCATE(data);
			}
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_set_type_iteration_threshold.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_iteration_threshold */

int Computed_field_get_type_iteration_threshold(struct Computed_field *field,
	struct Computed_field **source_field,
	struct Computed_field **texture_coordinate_field,
	int *dimension, int **sizes, FE_value **minimums, FE_value **maximums,
	int *element_dimension)
/*******************************************************************************
LAST MODIFIED : 4 December 2003

DESCRIPTION :
If the field is of type COMPUTED_FIELD_iteration_threshold, the
parameters defining it are returned.
==============================================================================*/
{
	int i, return_code;
	struct Computed_field_iteration_threshold_type_specific_data *data;

	ENTER(Computed_field_get_type_iteration_threshold);
	if (field && (field->type_string==computed_field_iteration_threshold_type_string)
		&& (data = (struct Computed_field_iteration_threshold_type_specific_data *)
		field->type_specific_data) && data->image)
	{
		*dimension = data->image->dimension;
		if (ALLOCATE(*sizes, int, *dimension)
			&& ALLOCATE(*minimums, FE_value, *dimension)
			&& ALLOCATE(*maximums, FE_value, *dimension))
		{
			*source_field = field->source_fields[0];
			*texture_coordinate_field = field->source_fields[1];
			for (i = 0 ; i < *dimension ; i++)
			{
				(*sizes)[i] = data->image->sizes[i];
				(*minimums)[i] = data->image->minimums[i];
				(*maximums)[i] = data->image->maximums[i];
			}
			*element_dimension = data->element_dimension;
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_get_type_iteration_threshold.  Unable to allocate vectors.");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_iteration_threshold.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_iteration_threshold */

static int define_Computed_field_type_iteration_threshold(struct Parse_state *state,
	void *field_void, void *computed_field_iteration_threshold_package_void)
/*******************************************************************************
LAST MODIFIED : 4 December 2003

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_iteration_threshold (if it is not
already) and allows its contents to be modified.
==============================================================================*/
{
	char *current_token;
	FE_value *minimums, *maximums;
	int dimension, element_dimension, return_code, *sizes;
	struct Computed_field *field, *source_field, *texture_coordinate_field;
	struct Computed_field_iteration_threshold_package
		*computed_field_iteration_threshold_package;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_source_field_data,
		set_texture_coordinate_field_data;

	ENTER(define_Computed_field_type_iteration_threshold);
	if (state&&(field=(struct Computed_field *)field_void)&&
		(computed_field_iteration_threshold_package=
		(struct Computed_field_iteration_threshold_package *)
		computed_field_iteration_threshold_package_void))
	{
		return_code=1;
		source_field = (struct Computed_field *)NULL;
		texture_coordinate_field = (struct Computed_field *)NULL;
		dimension = 0;
		sizes = (int *)NULL;
		minimums = (FE_value *)NULL;
		maximums = (FE_value *)NULL;
		element_dimension = 0;
		/* field */
		set_source_field_data.computed_field_manager =
			computed_field_iteration_threshold_package->computed_field_manager;
		set_source_field_data.conditional_function =
			Computed_field_has_numerical_components;
		set_source_field_data.conditional_function_user_data = (void *)NULL;
		/* texture_coordinate_field */
		set_texture_coordinate_field_data.computed_field_manager =
			computed_field_iteration_threshold_package->computed_field_manager;
		set_texture_coordinate_field_data.conditional_function =
			Computed_field_has_numerical_components;
		set_texture_coordinate_field_data.conditional_function_user_data = (void *)NULL;

		if (computed_field_iteration_threshold_type_string ==
			Computed_field_get_type_string(field))
		{
			return_code = Computed_field_get_type_iteration_threshold(field,
				&source_field, &texture_coordinate_field, &dimension,
				&sizes, &minimums, &maximums, &element_dimension);
		}
		if (return_code)
		{
			/* must access objects for set functions */
			if (source_field)
			{
				ACCESS(Computed_field)(source_field);
			}
			if (texture_coordinate_field)
			{
				ACCESS(Computed_field)(texture_coordinate_field);
			}

			if ((current_token=state->current_token) &&
				(!(strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))))
			{
				option_table = CREATE(Option_table)();
				/* dimension */
				Option_table_add_int_positive_entry(option_table, "dimension",
					&dimension);
				/* element_dimension */
				Option_table_add_int_non_negative_entry(option_table, "element_dimension",
					&element_dimension);
				/* field */
				Option_table_add_Computed_field_conditional_entry(option_table,
					"field", &source_field, &set_source_field_data);
				/* maximums */
				Option_table_add_FE_value_vector_entry(option_table,
					"maximums", maximums, &dimension);
				/* minimums */
				Option_table_add_FE_value_vector_entry(option_table,
					"minimums", minimums, &dimension);
				/* sizes */
				Option_table_add_int_vector_entry(option_table,
					"sizes", sizes, &dimension);
				/* texture_coordinate_field */
				Option_table_add_Computed_field_conditional_entry(option_table,
					"texture_coordinate_field", &texture_coordinate_field,
					&set_texture_coordinate_field_data);
				return_code=Option_table_multi_parse(option_table,state);
				DESTROY(Option_table)(&option_table);
			}
			/* parse the dimension ... */
			if (return_code && (current_token = state->current_token))
			{
				/* ... only if the "dimension" token is next */
				if (fuzzy_string_compare(current_token, "dimension"))
				{
					option_table = CREATE(Option_table)();
					/* dimension */
					Option_table_add_int_positive_entry(option_table, "dimension",
						&dimension);
					if (return_code = Option_table_parse(option_table, state))
					{
						if (!(REALLOCATE(sizes, sizes, int, dimension) &&
							REALLOCATE(minimums, minimums, FE_value, dimension) &&
							REALLOCATE(maximums, maximums, FE_value, dimension)))
						{
							return_code = 0;
						}
					}
					DESTROY(Option_table)(&option_table);
				}
			}
			if (return_code && (dimension < 1))
			{
				display_message(ERROR_MESSAGE,
					"define_Computed_field_type_scale.  Must specify a dimension first.");
				return_code = 0;
			}
			/* parse the rest of the table */
			if (return_code&&state->current_token)
			{
				option_table = CREATE(Option_table)();
				/* element_dimension */
				Option_table_add_int_non_negative_entry(option_table, "element_dimension",
					&element_dimension);
				/* field */
				Option_table_add_Computed_field_conditional_entry(option_table,
					"field", &source_field, &set_source_field_data);
				/* maximums */
				Option_table_add_FE_value_vector_entry(option_table,
					"maximums", maximums, &dimension);
				/* minimums */
				Option_table_add_FE_value_vector_entry(option_table,
					"minimums", minimums, &dimension);
				/* sizes */
				Option_table_add_int_vector_entry(option_table,
					"sizes", sizes, &dimension);
				/* texture_coordinate_field */
				Option_table_add_Computed_field_conditional_entry(option_table,
					"texture_coordinate_field", &texture_coordinate_field,
					&set_texture_coordinate_field_data);
				return_code=Option_table_multi_parse(option_table,state);
				DESTROY(Option_table)(&option_table);
			}
			/* no errors,not asking for help */
			if (return_code)
			{
				return_code = Computed_field_set_type_iteration_threshold(field,
					source_field, texture_coordinate_field, dimension,
					sizes, minimums, maximums, element_dimension,
					computed_field_iteration_threshold_package->computed_field_manager,
					computed_field_iteration_threshold_package->root_region,
					computed_field_iteration_threshold_package->user_interface);
			}
			if (!return_code)
			{
				if ((!state->current_token)||
					(strcmp(PARSER_HELP_STRING,state->current_token)&&
						strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
				{
					/* error */
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_iteration_threshold.  Failed");
				}
			}
			if (source_field)
			{
				DEACCESS(Computed_field)(&source_field);
			}
			if (texture_coordinate_field)
			{
				DEACCESS(Computed_field)(&texture_coordinate_field);
			}
			if (sizes)
			{
				DEALLOCATE(sizes);
			}
			if (minimums)
			{
				DEALLOCATE(minimums);
			}
			if (maximums)
			{
				DEALLOCATE(maximums);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_iteration_threshold.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_iteration_threshold */

int Computed_field_register_types_iteration_threshold(
	struct Computed_field_package *computed_field_package,
	struct Cmiss_region *root_region, struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 12 December 2003

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	static struct Computed_field_iteration_threshold_package
		computed_field_iteration_threshold_package;

	ENTER(Computed_field_register_types_iteration_threshold);
	if (computed_field_package)
	{
		computed_field_iteration_threshold_package.computed_field_manager =
			Computed_field_package_get_computed_field_manager(
				computed_field_package);
		computed_field_iteration_threshold_package.root_region = root_region;
		computed_field_iteration_threshold_package.user_interface = user_interface;
		return_code = Computed_field_package_add_type(computed_field_package,
			            computed_field_iteration_threshold_type_string,
			            define_Computed_field_type_iteration_threshold,
			            &computed_field_iteration_threshold_package);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_register_types_iteration_threshold.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_register_types_iteration_threshold */
