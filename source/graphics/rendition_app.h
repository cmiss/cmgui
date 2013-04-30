

#if !defined (RENDITION_APP_H_)
#define RENDITION_APP_H_


/**
 * Executes a GFX MODIFY RENDITION GENERAL command.
 * Allows general rendition to be changed (eg. discretization) and
 * updates graphics of settings affected by the changes (probably all).
 */
int gfx_modify_rendition_general(struct Parse_state *state,
	void *cmiss_region_void, void *dummy_void);

/**
 * Lists the general graphic defined for <rendition>.
 */
int Cmiss_rendition_list_contents(struct Cmiss_rendition *rendition);

/**
 * Shared gfx commands used via Cmiss_rendition_execute_command API and
 * command/cmiss
 * @param group  Optional group field for migrating group regions.
 */
int Cmiss_rendition_execute_command_internal(Cmiss_rendition_id rendition,
	Cmiss_field_group_id group, struct Parse_state *state);

/**
 * Convenience function for drawing a glyph as a point graphic with the
 * given name using all other defaults.
 */
int Cmiss_rendition_add_glyph(struct Cmiss_rendition *rendition,
	struct GT_object *glyph, const char *cmiss_graphic_name);

/**
 * Makes a new graphic of the supplied graphic_type, optionally a copy of an
 * existing graphic.
 *
 * @param rendition  Source of graphics defaults if creating a new graphic.
 * @param graphic_type  The type of the new graphic.
 * @param graphic_to_copy  Optional graphic to copy settings from if of
 * same graphic_type.
 * @return  1 on success, 0 on failure.
 */
Cmiss_graphic* Cmiss_rendition_create_graphic_app(Cmiss_rendition *rendition,
	Cmiss_graphic_type graphic_type, Cmiss_graphic *graphic_to_copy);

#endif
