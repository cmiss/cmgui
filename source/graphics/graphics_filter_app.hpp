


/***************************************************************************//**
 * graphics_filter.hpp
 *
 * Declaration of scene graphic filter classes and functions.
 */
/* OpenCMISS-Cmgui Application
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef GRAPHICS_FILTER_APP_HPP
#define GRAPHICS_FILTER_APP_HPP

#include "zinc/scene.h"
#include "general/list.h"
#include "general/manager.h"
#include "general/mystring.h"
#include "general/object.h"

struct cmzn_rendition;
struct cmzn_graphic;

DECLARE_LIST_TYPES(cmzn_graphics_filter);

PROTOTYPE_OBJECT_FUNCTIONS(cmzn_graphics_filter);
PROTOTYPE_GET_OBJECT_NAME_FUNCTION(cmzn_graphics_filter);

PROTOTYPE_LIST_FUNCTIONS(cmzn_graphics_filter);
PROTOTYPE_FIND_BY_IDENTIFIER_IN_LIST_FUNCTION(cmzn_graphics_filter,name,const char *);

PROTOTYPE_MANAGER_FUNCTIONS(cmzn_graphics_filter);
PROTOTYPE_MANAGER_IDENTIFIER_FUNCTIONS(cmzn_graphics_filter,name,const char *);


int cmzn_graphics_filter_manager_set_owner_private(struct MANAGER(cmzn_graphics_filter) *manager,
	struct cmzn_graphics_module *graphics_module);

int gfx_define_graphics_filter(struct Parse_state *state, void *root_region_void,
	void *filter_module_void);

int gfx_list_graphics_filter(struct Parse_state *state, void *dummy_to_be_modified,
	void *filter_module_void);

int set_cmzn_graphics_filter(struct Parse_state *state,
	void *graphics_filter_address_void, void *filter_module_void);
#endif /* GRAPHICS_FILTER_HPP_ */
