/**
 * FILE : glyph_app.h
 *
 */
 /* OpenCMISS-Cmgui Application
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "zinc/zincconfigure.h"

struct Parse_state;

/**
 * Modifier function to set a glyph from a command.
 * @param glyphAddress_void  Address of accessing pointer to cmzn_glyph
 * @param glyphmodule_void  A cmzn_glyphmodule *.
 */
int set_Glyph(struct Parse_state *state, void *glyphAddress_void,
	void *glyphmodule_void);

struct Define_glyph_data
{
	cmzn_region *root_region;
	cmzn_glyphmodule *glyphmodule;
};

int gfx_define_glyph(struct Parse_state *state,
	void *glyph_name_void, void *define_glyph_data_void);
