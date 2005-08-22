/*******************************************************************************
FILE : computed_value_private.h

LAST MODIFIED : 30 July 2003

DESCRIPTION :
==============================================================================*/
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is cmgui.
 *
 * The Initial Developer of the Original Code is
 * Auckland Uniservices Ltd, Auckland, New Zealand.
 * Portions created by the Initial Developer are Copyright (C) 2005
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */
#if !defined (__CMISS_VALUE_PRIVATE_H__)
#define __CMISS_VALUE_PRIVATE_H__

#include "computed_variable/computed_value.h"

/*
Method types
------------
*/
typedef void Cmiss_value_type_specific_data;

typedef int (*Cmiss_value_clear_type_specific_function)(
	Cmiss_value_id value);
/*******************************************************************************
LAST MODIFIED : 9 April 2003

DESCRIPTION :
Clear the type specific data for <value> (passed into the _set_type_
function), but don't DEALLOCATE the data.
==============================================================================*/

#define START_CMISS_VALUE_CLEAR_TYPE_SPECIFIC_FUNCTION( value_type ) \
int Cmiss_value_ ## value_type ## _clear_type_specific( \
	Cmiss_value_id value) \
{ \
	int return_code; \
	struct Cmiss_value_ ## value_type ## _type_specific_data *data; \
\
	ENTER(Cmiss_value_ ## value_type ## _clear_type_specific); \
	return_code=0; \
	data=(struct Cmiss_value_ ## value_type ## _type_specific_data *) \
		Cmiss_value_get_type_specific_data(value); \
	ASSERT_IF(data,return_code,0) \
	ASSERT_IF(Cmiss_value_ ## value_type ## _type_string== \
		Cmiss_value_get_type_id_string(value),return_code,0)

#define END_CMISS_VALUE_CLEAR_TYPE_SPECIFIC_FUNCTION( value_type ) \
	LEAVE; \
\
	return (return_code); \
} /* Cmiss_value_ ## value_type ## _clear_type_specific */

typedef Cmiss_value_type_specific_data*
	(*Cmiss_value_duplicate_data_type_specific_function)(
	Cmiss_value_id value);
/*******************************************************************************
LAST MODIFIED : 9 April 2003

DESCRIPTION :
Returns a duplicate of the <value>'s type specific data.
==============================================================================*/

#define START_CMISS_VALUE_DUPLICATE_DATA_TYPE_SPECIFIC_FUNCTION( \
	value_type ) \
Cmiss_value_type_specific_data * \
	Cmiss_value_ ## value_type ## _duplicate_data_type_specific( \
	Cmiss_value_id value) \
{ \
	struct Cmiss_value_ ## value_type ## _type_specific_data \
		*destination,*source; \
\
	ENTER(Cmiss_value_ ## value_type ## _duplicate_data_type_specific); \
	/* check arguments */ \
	ASSERT_IF(value&&(Cmiss_value_ ## value_type ## _type_string== \
		Cmiss_value_get_type_id_string(value)),destination,NULL) \
	{ \
		source=Cmiss_value_get_type_specific_data(value); \
		ASSERT_IF(source,destination,NULL) \
		if (!ALLOCATE(destination, \
			struct Cmiss_value_ ## value_type ## _type_specific_data,1)) \
		{ \
			display_message(ERROR_MESSAGE,"Cmiss_value_" #value_type \
				"_duplicate_data_type_specific.  Could not ALLOCATE destination"); \
		} \
		else

#define END_CMISS_VALUE_DUPLICATE_DATA_TYPE_SPECIFIC_FUNCTION( \
	value_type ) \
	} \
	LEAVE; \
\
	return (destination); \
} /* Cmiss_value_ ## value_type ## _duplicate_data_type_specific */

typedef int (*Cmiss_value_get_reals_type_specific_function)(
	Cmiss_value_id value,int *number_of_reals_address,FE_value **reals_address);
/*******************************************************************************
LAST MODIFIED : 16 July 2003

DESCRIPTION :
Gets the <*number_of_reals_address> for the <value>.  This is needed when
calculating derivatives.  If <real_address> is not NULL, then an array is
allocated and the reals put in it.  A zero return code means that <value> is not
represented by reals or the array could not be allocated or the arguments are
invalid.
==============================================================================*/

#define START_CMISS_VALUE_GET_REALS_TYPE_SPECIFIC_FUNCTION( value_type ) \
int Cmiss_value_ ## value_type ## _get_reals_type_specific( \
	Cmiss_value_id value,int *number_of_reals_address,FE_value **reals_address) \
{ \
	int return_code; \
	struct Cmiss_value_ ## value_type ## _type_specific_data *data; \
\
	ENTER(Cmiss_value_ ## value_type ## _get_reals_type_specific); \
	return_code=0; \
	data=(struct Cmiss_value_ ## value_type ## _type_specific_data *) \
		Cmiss_value_get_type_specific_data(value); \
	ASSERT_IF(data,return_code,0) \
	ASSERT_IF(number_of_reals_address,return_code,0) \
	ASSERT_IF(Cmiss_value_ ## value_type ## _type_string== \
		Cmiss_value_get_type_id_string(value),return_code,0)

#define END_CMISS_VALUE_GET_REALS_TYPE_SPECIFIC_FUNCTION( value_type ) \
	LEAVE; \
\
	return (return_code); \
} /* Cmiss_value_ ## value_type ## _get_reals_type_specific */

typedef int (*Cmiss_value_get_string_type_specific_function)(
	Cmiss_value_id value, char **string);
/*******************************************************************************
LAST MODIFIED : 25 July 2003

DESCRIPTION :
Creates a string representation of the Cmiss_value useful for output.
==============================================================================*/

#define START_CMISS_VALUE_GET_STRING_TYPE_SPECIFIC_FUNCTION( value_type ) \
int Cmiss_value_ ## value_type ## _get_string_type_specific( \
	Cmiss_value_id value, char **string) \
{ \
	int return_code; \
	struct Cmiss_value_ ## value_type ## _type_specific_data *data; \
\
	ENTER(Cmiss_value_ ## value_type ## _get_string_type_specific); \
	return_code=0; \
	data=(struct Cmiss_value_ ## value_type ## _type_specific_data *) \
		Cmiss_value_get_type_specific_data(value); \
	ASSERT_IF(data,return_code,0) \
	ASSERT_IF(value&&(Cmiss_value_ ## value_type ## _type_string== \
		Cmiss_value_get_type_id_string(value))&&string,return_code,0)

#define END_CMISS_VALUE_GET_STRING_TYPE_SPECIFIC_FUNCTION( value_type ) \
	LEAVE; \
\
	return (return_code); \
} /* Cmiss_value_ ## value_type ## _get_string_type_specific */

typedef int (*Cmiss_value_increment_type_specific_function)(
	Cmiss_value_id value,Cmiss_value_id increment);
/*******************************************************************************
LAST MODIFIED : 30 July 2003

DESCRIPTION :
Perform <value> += <increment>.
==============================================================================*/

#define START_CMISS_VALUE_INCREMENT_TYPE_SPECIFIC_FUNCTION( value_type ) \
int Cmiss_value_ ## value_type ## _increment_type_specific( \
	Cmiss_value_id value,Cmiss_value_id increment) \
{ \
	int return_code; \
	struct Cmiss_value_ ## value_type ## _type_specific_data *data_value; \
\
	ENTER(Cmiss_value_ ## value_type ## _increment_type_specific); \
	return_code=0; \
	data_value=(struct Cmiss_value_ ## value_type ## _type_specific_data *) \
		Cmiss_value_get_type_specific_data(value); \
	ASSERT_IF(data_value&&increment,return_code,0) \
	ASSERT_IF(Cmiss_value_ ## value_type ## _type_string== \
		Cmiss_value_get_type_id_string(value),return_code,0)

#define END_CMISS_VALUE_INCREMENT_TYPE_SPECIFIC_FUNCTION( value_type ) \
	LEAVE; \
\
	return (return_code); \
} /* Cmiss_value_ ## value_type ## _increment_type_specific */

typedef int (*Cmiss_value_multiply_and_accumulate_type_specific_function)(
	Cmiss_value_id total,Cmiss_value_id value_1,
	Cmiss_value_id value_2);
/*******************************************************************************
LAST MODIFIED : 20 July 2003

DESCRIPTION :
Check that <value_1> and <value_2> are of the same type.
==============================================================================*/

#define START_CMISS_VALUE_MULTIPLY_AND_ACCUMULATE_TYPE_SPECIFIC_FUNCTION( \
	value_type ) \
int Cmiss_value_ ## value_type ## _multiply_and_accumulate_type_specific( \
	Cmiss_value_id total,Cmiss_value_id value_1,Cmiss_value_id value_2) \
{ \
	int return_code; \
\
	ENTER(Cmiss_value_ ## value_type ## \
		_multiply_and_accumulater_type_specific); \
	return_code=0; \
	ASSERT_IF(value_1&&(Cmiss_value_ ## value_type ## _type_string== \
		Cmiss_value_get_type_id_string(value_1))&&value_2&&(Cmiss_value_ ## \
		value_type ## _type_string==Cmiss_value_get_type_id_string(value_2))&& \
		total&&(Cmiss_value_ ## value_type ## _type_string== \
		Cmiss_value_get_type_id_string(total)),return_code,0)

#define END_CMISS_VALUE_MULTIPLY_AND_ACCUMULATE_TYPE_SPECIFIC_FUNCTION( \
	value_type ) \
	LEAVE; \
\
	return (return_code); \
} /* Cmiss_value_ ## value_type ## \
	_multiply_and_accumulate_type_specific */

typedef int (*Cmiss_value_same_sub_type_type_specific_function)(
	Cmiss_value_id value_1,Cmiss_value_id value_2);
/*******************************************************************************
LAST MODIFIED : 9 April 2003

DESCRIPTION :
Check that <value_1> and <value_2> are of the same sub-type (same type plus
extra restrictions such as length for a vector).
==============================================================================*/

#define START_CMISS_VALUE_SAME_SUB_TYPE_TYPE_SPECIFIC_FUNCTION( \
	value_type ) \
int Cmiss_value_ ## value_type ## _same_sub_type_type_specific( \
	Cmiss_value_id value_1,Cmiss_value_id value_2) \
{ \
	int return_code; \
\
	ENTER(Cmiss_value_ ## value_type ## _same_sub_type_type_specific); \
	return_code=0; \
	ASSERT_IF(value_1&&(Cmiss_value_ ## value_type ## _type_string== \
		Cmiss_value_get_type_id_string(value_1))&&value_2&&Cmiss_value_ ## \
		value_type ## _type_string==Cmiss_value_get_type_id_string(value_2), \
		return_code,0)

#define END_CMISS_VALUE_SAME_SUB_TYPE_TYPE_SPECIFIC_FUNCTION( value_type ) \
	LEAVE; \
\
	return (return_code); \
} /* Cmiss_value_ ## value_type ## _same_sub_type_type_specific */

typedef int (*Cmiss_value_scalar_multiply_type_specific_function)(
	Cmiss_value_id value,FE_value scalar);
/*******************************************************************************
LAST MODIFIED : 28 July 2003

DESCRIPTION :
Performs <value>=<scalar>*<value>.
==============================================================================*/

#define START_CMISS_VALUE_SCALAR_MULTIPLY_TYPE_SPECIFIC_FUNCTION( value_type ) \
int Cmiss_value_ ## value_type ## _scalar_multiply_type_specific( \
	Cmiss_value_id value,FE_value scalar) \
{ \
	int return_code; \
	struct Cmiss_value_ ## value_type ## _type_specific_data *data; \
\
	ENTER(Cmiss_value_ ## value_type ## _scalar_multiply_type_specific); \
	return_code=0; \
	data=(struct Cmiss_value_ ## value_type ## _type_specific_data *) \
		Cmiss_value_get_type_specific_data(value); \
	ASSERT_IF(data,return_code,0) \
	ASSERT_IF(Cmiss_value_ ## value_type ## _type_string== \
		Cmiss_value_get_type_id_string(value),return_code,0)

#define END_CMISS_VALUE_SCALAR_MULTIPLY_TYPE_SPECIFIC_FUNCTION( value_type ) \
	LEAVE; \
\
	return (return_code); \
} /* Cmiss_value_ ## value_type ## _scalar_multiply_type_specific */


/*
Friend macros
-------------
*/
#define DECLARE_CMISS_VALUE_IS_TYPE_FUNCTION( value_type ) \
PROTOTYPE_CMISS_VALUE_IS_TYPE_FUNCTION(value_type) \
{ \
	int return_code; \
\
	ENTER(CMISS_VALUE_IS_TYPE(value_type)); \
	return_code=0; \
	/* check argument */ \
	if (value) \
	{ \
		if (Cmiss_value_ ## value_type ## _type_string== \
			Cmiss_value_get_type_id_string(value)) \
		{ \
			return_code=1; \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE,"CMISS_VALUE_IS_TYPE(" #value_type ").  " \
			"Missing value"); \
	} \
	LEAVE; \
\
	return (return_code); \
} /* CMISS_VALUE_IS_TYPE(value_type) */

#define CMISS_VALUE_ESTABLISH_METHODS( value, value_type ) \
/***************************************************************************** \
LAST MODIFIED : 28 July 2003 \
\
DESCRIPTION : \
Each Cmiss_value_set_type function should call this macro to establish the \
virtual functions that give the value its particular behaviour.  Each function \
must therefore be defined for each value type, even if it is set to NULL or \
some default function. \
============================================================================*/ \
Cmiss_value_establish_methods(value, \
	Cmiss_value_ ## value_type ## _clear_type_specific, \
	Cmiss_value_ ## value_type ## _duplicate_data_type_specific, \
	Cmiss_value_ ## value_type ## _get_reals_type_specific, \
	Cmiss_value_ ## value_type ## _get_string_type_specific, \
	Cmiss_value_ ## value_type ## _increment_type_specific, \
	Cmiss_value_ ## value_type ## _multiply_and_accumulate_type_specific, \
	Cmiss_value_ ## value_type ## _same_sub_type_type_specific, \
	Cmiss_value_ ## value_type ## _scalar_multiply_type_specific)

/*
Friend functions
----------------
*/
int Cmiss_value_establish_methods(Cmiss_value_id value,
	Cmiss_value_clear_type_specific_function clear_type_specific_function,
	Cmiss_value_duplicate_data_type_specific_function
	duplicate_data_type_specific_function,
	Cmiss_value_get_reals_type_specific_function get_reals_type_specific_function,
	Cmiss_value_get_string_type_specific_function
	get_string_type_specific_function,
	Cmiss_value_increment_type_specific_function increment_type_specific_function,
	Cmiss_value_multiply_and_accumulate_type_specific_function
	multiply_and_accumulate_type_specific_function,
	Cmiss_value_same_sub_type_type_specific_function
	same_sub_type_type_specific_function,
	Cmiss_value_scalar_multiply_type_specific_function
	scalar_multiply_type_specific_function);
/*******************************************************************************
LAST MODIFIED : 28 July 2003

DESCRIPTION :
Sets the methods for the <value>.
==============================================================================*/

Cmiss_value_type_specific_data *Cmiss_value_get_type_specific_data(
	Cmiss_value_id value);
/*******************************************************************************
LAST MODIFIED : 12 February 2003

DESCRIPTION :
Returns the type specific data for the <value>.
==============================================================================*/

int Cmiss_value_set_type_specific_information(Cmiss_value_id value,
	char *type_string,void *type_specific_data);
/*******************************************************************************
LAST MODIFIED : 19 February 2003

DESCRIPTION :
Sets the type specific information for the <value>.
==============================================================================*/

int Cmiss_value_clear_type(Cmiss_value_id value);
/*******************************************************************************
LAST MODIFIED : 2 February 2003

DESCRIPTION :
Used internally by DESTROY and Cmiss_value_set_type_*() functions to
deallocate or deaccess data specific to any Cmiss_value_type.  Functions
changing the type of the Cmiss_value should
- allocate any dynamic data needed for the type
- call this function to clear what is currently in the value
- then set values
to ensure that the value is not left in an invalid state.
==============================================================================*/
#endif /* !defined (__CMISS_VALUE_PRIVATE_H__) */
