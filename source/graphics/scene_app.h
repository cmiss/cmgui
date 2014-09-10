/* OpenCMISS-Cmgui Application
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */


#if !defined (SCENE_APP_H_)
#define SCENE_APP_H_

#include "general/enumerator_app.h"
/**
 * Subset of command data passed to scene modify routines.
 */
struct Scene_command_data
{
	struct cmzn_graphics_module *graphics_module;
	struct cmzn_scene *scene;
	struct Graphical_material *default_material;
	struct cmzn_font *default_font;
	cmzn_glyphmodule_id glyphmodule;
	struct MANAGER(Computed_field) *computed_field_manager;
	struct cmzn_region *region;
	/* root_region used for seeding streamlines from the nodes in a region */
	struct cmzn_region *root_region;
	cmzn_materialmodule_id materialmodule;
	cmzn_tessellationmodule_id tessellationmodule;
	struct Spectrum *default_spectrum;
	struct MANAGER(Spectrum) *spectrum_manager;
};

/**
 * Executes a GFX MODIFY SCENE GENERAL command.
 * Allows general scene to be changed (eg. discretization) and
 * updates graphics of settings affected by the changes (probably all).
 */
int gfx_modify_scene_general(struct Parse_state *state,
	void *cmiss_region_void, void *dummy_void);

/**
 * Lists the general graphics defined for <scene>.
 */
int cmzn_scene_list_contents(struct cmzn_scene *scene);

/**
 * Shared gfx commands used via cmzn_scene_execute_command API and
 * command/cmiss
 * @param group  Optional group field for migrating group regions.
 */
int cmzn_scene_execute_command_internal(cmzn_scene_id scene,
	cmzn_field_group_id group, struct Parse_state *state);

/**
 * Convenience function for drawing a glyph as a point graphics with the
 * given name using all other defaults.
 */
int cmzn_scene_add_glyph(struct cmzn_scene *scene,
	cmzn_glyph *glyph, const char *graphics_name);

/**
 * Makes a new graphics of the supplied graphics_type, optionally a copy of an
 * existing graphics.
 *
 * @param scene  Source of graphics defaults if creating a new graphics.
 * @param graphics_type  The type of the new graphics.
 * @param graphics_to_copy  Optional graphics to copy settings from if of
 * same graphics_type.
 * @return  1 on success, 0 on failure.
 */
cmzn_graphics* cmzn_scene_create_graphics_app(cmzn_scene *scene,
	cmzn_graphics_type graphics_type, cmzn_graphics *graphics_to_copy);

float Scene_time(struct Scene *scene);

int Scene_set_time(struct Scene *scene, float time);

int set_Scene(struct Parse_state *state,
	void *scene_address_void,void *root_region_void);
/*******************************************************************************
LAST MODIFIED : 9 December 1997

DESCRIPTION :
Modifier function to set the scene from a command.
==============================================================================*/

/***************************************************************************//**
 * Parser commands for defining scenes - filters, lighting, etc.
 * @param define_scene_data_void  void pointer to a struct Define_scene_data
 * with contents filled.
 */
int define_Scene(struct Parse_state *state, void *scene_void,
	void *define_scene_data_void);

struct Define_scene_data
{
	struct cmzn_region *root_region;
	struct cmzn_graphics_module *graphics_module;
}; /* struct Define_scene_data */

cmzn_field_group_id cmzn_scene_get_or_create_selection_group(cmzn_scene_id scene);

int cmzn_scene_add_selection_from_element_list_of_dimension(cmzn_scene_id scene,
	struct LIST(FE_element) *element_list, int dimension);

int cmzn_scene_remove_selection_from_element_list_of_dimension(cmzn_scene_id scene,
	struct LIST(FE_element) *element_list, int dimension);

int cmzn_scene_add_selection_from_node_list(cmzn_scene_id scene,
	struct LIST(FE_node) *node_list, int use_data);

int cmzn_scene_remove_selection_from_node_list(cmzn_scene_id scene,
	struct LIST(FE_node) *node_list, int use_data);

PROTOTYPE_OPTION_TABLE_ADD_ENUMERATOR_FUNCTION( cmzn_scene_render_threejs_data_export_mode );

#endif
