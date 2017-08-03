/* OpenCMISS-Cmgui Application
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#if !defined (FINITE_ELEMENT_APP_H_)
#define FINITE_ELEMENT_APP_H_

#include "command/parser.h"
#include "finite_element/finite_element.h"

int set_FE_field(struct Parse_state *state,void *field_address_void,
	void *fe_field_list_void);
/*******************************************************************************
LAST MODIFIED : 3 December 2002

DESCRIPTION :
Modifier function to set the field from the command line.
==============================================================================*/

int set_FE_field_conditional(struct Parse_state *state,
	void *field_address_void,void *set_field_data_void);
/*******************************************************************************
LAST MODIFIED : 3 December 2002

DESCRIPTION :
Modifier function to set the field from a command. <set_field_data_void> should
point to a struct Set_FE_field_conditional_data containing the
fe_field_list and an optional conditional function for narrowing the
range of fields available for selection. If the conditional_function is NULL,
this function works just like set_FE_field.
==============================================================================*/

int set_FE_fields(struct Parse_state *state,
	void *field_order_info_address_void, void *fe_field_list_void);
/*******************************************************************************
LAST MODIFIED : 3 December 2002

DESCRIPTION :
Modifier function to set an ordered list of fields, each separated by white
space until an unrecognised field name is encountered. Two special tokens are
understood in place of any fields: 'all' and 'none'.
For the case of 'all', a NULL FE_field_order_info structure is returned.
For the case of 'none', an empty FE_field_order_info structure is returned.
It is up to the calling function to destroy any FE_field_order_info structure
returned by this function, however, any such structure passed to this function
may be destroyed here - ie. in the 'all' case.
==============================================================================*/

// for passing to setEnum modifier function template
struct cmzn_element_face_type_to_string
{
	static inline const char *toString(cmzn_element_face_type faceType)
	{
		return ENUMERATOR_STRING(cmzn_element_face_type)(faceType);
	}
};

/** @return  Non-accessed first top-level ancestor element for supplied element. Can return itself. */
cmzn_element *cmzn_element_get_first_top_level_ancestor(cmzn_element *element);

#endif

