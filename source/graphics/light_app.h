/* OpenCMISS-Cmgui Application
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */



#include "opencmiss/zinc/light.h"


PROTOTYPE_OBJECT_FUNCTIONS(cmzn_light);


struct Modify_light_data
/*******************************************************************************
LAST MODIFIED : 17 February 1997

DESCRIPTION :
==============================================================================*/
{
	struct cmzn_light *default_light;
	struct cmzn_lightmodule *lightmodule;
}; /* struct Modify_light_data */

int modify_cmzn_light(struct Parse_state *state,void *light_void,
	void *modify_light_data_void);
/*******************************************************************************
LAST MODIFIED : 17 February 1997

DESCRIPTION :
Modifies the properties of a light.
==============================================================================*/


/*==============================================================================*/

int set_cmzn_light(struct Parse_state *state,
	void *light_address_void,void *light_manager_void);
/*******************************************************************************
LAST MODIFIED : 11 December 1997

DESCRIPTION :
Modifier function to set the light from a command.
*/
