/**
 * scenefilter_app.hpp
 *
 * Declaration of scene filter classes and functions.
 */
/* OpenCMISS-Cmgui Application
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef SCENEFILTER_APP_HPP
#define SCENEFILTER_APP_HPP

#include "opencmiss/zinc/scene.h"
#include "opencmiss/zinc/scenefilter.h"
#include "general/list.h"
#include "general/manager.h"
#include "general/mystring.h"
#include "general/object.h"

struct cmzn_rendition;
struct cmzn_graphics;

DECLARE_LIST_TYPES(cmzn_scenefilter);

PROTOTYPE_OBJECT_FUNCTIONS(cmzn_scenefilter);
PROTOTYPE_GET_OBJECT_NAME_FUNCTION(cmzn_scenefilter);

PROTOTYPE_LIST_FUNCTIONS(cmzn_scenefilter);
PROTOTYPE_FIND_BY_IDENTIFIER_IN_LIST_FUNCTION(cmzn_scenefilter,name,const char *);

PROTOTYPE_MANAGER_FUNCTIONS(cmzn_scenefilter);
PROTOTYPE_MANAGER_IDENTIFIER_FUNCTIONS(cmzn_scenefilter,name,const char *);


int cmzn_scenefilter_manager_set_owner_private(struct MANAGER(cmzn_scenefilter) *manager,
	struct cmzn_graphics_module *graphics_module);

int gfx_define_graphics_filter(struct Parse_state *state, void *root_region_void,
	void *filter_module_void);

int gfx_list_graphics_filter(struct Parse_state *state, void *dummy_to_be_modified,
	void *filter_module_void);

int set_cmzn_scenefilter(struct Parse_state *state,
	void *scenefilter_address_void, void *filter_module_void);
#endif /* SCENEFILTER_APP_HPP */
