/* OpenCMISS-Cmgui Application
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */


int set_FE_field_conditional_FE_region(struct Parse_state *state,
	void *fe_field_address_void, void *parse_field_data_void);
/*******************************************************************************
LAST MODIFIED : 3 December 2002

DESCRIPTION :
FE_region wrapper for set_FE_field_conditional. <parse_field_data_void> points
at a struct Set_FE_field_conditional_FE_region_data.
==============================================================================*/

int set_FE_fields_FE_region(struct Parse_state *state,
	void *fe_field_order_info_address_void, void *fe_region_void);
/*******************************************************************************
LAST MODIFIED : 7 March 2003

DESCRIPTION :
FE_region wrapper for set_FE_fields.
==============================================================================*/

int set_FE_element_top_level_FE_region(struct Parse_state *state,
	void *element_address_void, void *fe_region_void);
/*******************************************************************************
LAST MODIFIED : 26 February 2003

DESCRIPTION :
A modifier function for specifying a top level element, used, for example, to
set the seed element for a xi_texture_coordinate computed_field.
==============================================================================*/
