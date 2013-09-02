
/* * Executes a GFX MODIFY SCENE GRAPHIC_TYPE command.
 * If return_code is 1, returns the completed Modify_scene_data with the
 * parsed graphic. Note that the graphic are ACCESSed once on valid return.
 */
int gfx_modify_scene_graphic(struct Parse_state *state,
	enum cmzn_graphic_type graphic_type, const char *help_text,
	struct Modify_scene_data *modify_scene_data,
	struct Scene_command_data *scene_command_data);

/**
 * Executes a GFX MODIFY SCENE CONTOURS command.
 * If return_code is 1, returns the completed Modify_scene_data with the
 * parsed graphic. Note that the graphic are ACCESSed once on valid return.
 */
int gfx_modify_scene_contours(struct Parse_state *state,
	void *modify_scene_data_void,void *scene_command_data_void);

/***************************************************************************//**
 * Executes a GFX MODIFY SCENE CYLINDERS command.
 * If return_code is 1, returns the completed Modify_scene_data with the
 * parsed graphic. Note that the graphic are ACCESSed once on valid return.
 */
int gfx_modify_scene_cylinders(struct Parse_state *state,
	void *modify_scene_data_void,void *scene_command_data_void);

/***************************************************************************//**
 * Executes a GFX MODIFY SCENE DATA_POINTS command.
 * If return_code is 1, returns the completed Modify_scene_data with the
 * parsed graphic. Note that the graphic are ACCESSed once on valid return.
 */
int gfx_modify_scene_data_points(struct Parse_state *state,
	void *modify_scene_data_void,void *scene_command_data_void);

/***************************************************************************//**
 * Executes a GFX MODIFY SCENE ELEMENT_POINTS command.
 * If return_code is 1, returns the completed Modify_scene_data with the
 * parsed graphic. Note that the graphic are ACCESSed once on valid return.
 */
int gfx_modify_scene_element_points(struct Parse_state *state,
	void *modify_scene_data_void,void *scene_command_data_void);

/***************************************************************************//**
 * Executes a GFX MODIFY SCENE ISO_SURFACES command.
 * If return_code is 1, returns the completed Modify_scene_data with the
 * parsed graphic. Note that the graphic are ACCESSed once on valid return.
 */
int gfx_modify_scene_iso_surfaces(struct Parse_state *state,
	void *modify_scene_data_void,void *scene_command_data_void);

/***************************************************************************//**
 * Executes a GFX MODIFY SCENE LINES command.
 * If return_code is 1, returns the completed Modify_scene_data with the
 * parsed graphic. Note that the graphic are ACCESSed once on valid return.
 */
int gfx_modify_scene_lines(struct Parse_state *state,
	void *modify_scene_data_void,void *scene_command_data_void);

/***************************************************************************//**
 * Executes a GFX MODIFY SCENE NODE_POINTS command.
 * If return_code is 1, returns the completed Modify_g_element_data with the
 * parsed graphic. Note that the graphic are ACCESSed once on valid return.
 */
int gfx_modify_scene_node_points(struct Parse_state *state,
	void *modify_scene_data_void,void *scene_command_data_void);

/***************************************************************************//**
 * Executes a GFX MODIFY SCENE POINT command.
 * If return_code is 1, returns the completed Modify_scene_data with the
 * parsed graphic. Note that the graphic are ACCESSed once on valid return.
 */
int gfx_modify_scene_point(struct Parse_state *state,
		void *modify_scene_data_void,void *scene_command_data_void);

/**
 * Executes a GFX MODIFY SCENE POINTS command.
 * If return_code is 1, returns the completed Modify_scene_data with the
 * parsed graphic. Note that the graphic are ACCESSed once on valid return.
 * @param state Parse state
 * @param modify_scene_data_void void pointer to a container object
 * @param command_data_void void pointer to a container object
 * @return if successfully modify surface returns 1, else 0
 */
int gfx_modify_scene_points(struct Parse_state *state,
	void *modify_scene_data_void,void *scene_command_data_void);

/***************************************************************************//**
 * Executes a GFX MODIFY SCENE STREAMLINES command.
 * If return_code is 1, returns the completed Modify_scene_data with the
 * parsed graphic. Note that the graphic are ACCESSed once on valid return.
 */
int gfx_modify_scene_streamlines(struct Parse_state *state,
	void *modify_scene_data_void,void *scene_command_data_void);

/***************************************************************************//**
 * Executes a GFX MODIFY SCENE SURFACES command.
 * If return_code is 1, returns the completed Modify_scene_data with the
 * parsed graphic. Note that the graphic are ACCESSed once on valid return.
 * @param state Parse state
 * @param modify_scene_data_void void pointer to a container object
 * @param command_data_void void pointer to a container object
 * @return if successfully modify surface returns 1, else 0
 */
int gfx_modify_scene_surfaces(struct Parse_state *state,
	void *modify_scene_data_void,void *scene_command_data_void);
