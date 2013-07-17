

#if !defined (SCENE_APP_H_)
#define SCENE_APP_H_

/**
 * Subset of command data passed to scene modify routines.
 */
struct Scene_command_data
{
	struct Cmiss_graphics_module *graphics_module;
	struct Cmiss_scene *scene;
	struct Graphical_material *default_material;
	struct Cmiss_font *default_font;
	Cmiss_glyph_module_id glyph_module;
	struct MANAGER(Computed_field) *computed_field_manager;
	struct Cmiss_region *region;
	/* root_region used for seeding streamlines from the nodes in a region */
	struct Cmiss_region *root_region;
	Cmiss_graphics_material_module_id material_module;
	Cmiss_tessellation_module_id tessellation_module;
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
 * Lists the general graphic defined for <scene>.
 */
int Cmiss_scene_list_contents(struct Cmiss_scene *scene);

/**
 * Shared gfx commands used via Cmiss_scene_execute_command API and
 * command/cmiss
 * @param group  Optional group field for migrating group regions.
 */
int Cmiss_scene_execute_command_internal(Cmiss_scene_id scene,
	Cmiss_field_group_id group, struct Parse_state *state);

/**
 * Convenience function for drawing a glyph as a point graphic with the
 * given name using all other defaults.
 */
int Cmiss_scene_add_glyph(struct Cmiss_scene *scene,
	Cmiss_glyph *glyph, const char *cmiss_graphic_name);

/**
 * Makes a new graphic of the supplied graphic_type, optionally a copy of an
 * existing graphic.
 *
 * @param scene  Source of graphics defaults if creating a new graphic.
 * @param graphic_type  The type of the new graphic.
 * @param graphic_to_copy  Optional graphic to copy settings from if of
 * same graphic_type.
 * @return  1 on success, 0 on failure.
 */
Cmiss_graphic* Cmiss_scene_create_graphic_app(Cmiss_scene *scene,
	Cmiss_graphic_type graphic_type, Cmiss_graphic *graphic_to_copy);

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
	struct Cmiss_region *root_region;
	struct Cmiss_graphics_module *graphics_module;
}; /* struct Define_scene_data */

#endif
