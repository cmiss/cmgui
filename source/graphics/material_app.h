/* OpenCMISS-Cmgui Application
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#if !defined (MATERIAL_APP_H)
#define MATERIAL_APP_H

#include "opencmiss/zinc/types/materialid.h"

#define MATERIAL_PRECISION_STRING "lf"

struct Material_module_app
/** Note none of these objects are accessed */
{
	void *module;
	void *region;
	void *shadermodule;
};

int gfx_create_material(struct Parse_state *state,
	void *dummy_to_be_modified, void *materialmodule_void);
/*******************************************************************************
LAST MODIFIED : 20 November 2003

DESCRIPTION :
Shifted from command/cmiss.c now that there is a material module.
If the material already exists, then behaves like gfx modify material.
==============================================================================*/

int modify_Graphical_material(struct Parse_state *parse_state,void *material,
	void *materialmodule_void);
/*******************************************************************************
LAST MODIFIED : 5 September 1996

DESCRIPTION :
==============================================================================*/

int set_Graphical_material(struct Parse_state *state,
	void *material_address_void,void *graphical_material_manager_void);
/*******************************************************************************
LAST MODIFIED : 20 June 1996

DESCRIPTION :
Modifier function to set the material from a command.
==============================================================================*/

/**
 * Adds the given <token> to the <option_table>.  The <material> is selected from
 * the <materialmodule> by name.
 */
int Option_table_add_set_Material_entry(
	struct Option_table *option_table, const char *token,
	cmzn_material **material, struct cmzn_materialmodule *materialmodule);

#endif
