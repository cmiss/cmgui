/* OpenCMISS-Cmgui Application
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */


#if !defined (SCENE_VIEWER_APP_H_)
#define SCENE_VIEWER_APP_H_

#include "computed_field/computed_field.h"

#include "interaction/interactive_tool.h"

#define Scene_viewer_set_interactive_tool_by_name cmzn_sceneviewer_set_interactive_tool_by_name

DECLARE_CMZN_CALLBACK_TYPES(cmzn_sceneviewermodule_app_callback, \
	struct cmzn_sceneviewermodule_app *, void *, void);

DECLARE_CMZN_CALLBACK_TYPES(Scene_viewer_app_callback, \
	struct Scene_viewer_app *, void *, void);

DECLARE_CMZN_CALLBACK_TYPES(Scene_viewer_app_input_callback, \
	struct Scene_viewer_app *, struct Graphics_buffer_input *, int);

struct cmzn_sceneviewermodule_app
/*******************************************************************************
LAST MODIFIED : 19 January 2007

DESCRIPTION:
The default data used to create cmzn_sceneviewers.
==============================================================================*/
{
	int access_count;
	struct cmzn_sceneviewermodule *core_sceneviewermodule;
	struct Graphics_buffer_app_package *graphics_buffer_package;
	struct User_interface *user_interface;
	struct MANAGER(Interactive_tool) *interactive_tool_manager;
	//-- struct User_interface *user_interface;
	/* List of scene_viewers created with this package,
		generally all scene_viewers that are not in graphics windows */
	struct LIST(Scene_viewer_app) *scene_viewer_app_list;
	struct LIST(CMZN_CALLBACK_ITEM(cmzn_sceneviewermodule_app_callback))
		*destroy_callback_list;
};

struct Scene_viewer_app
{
	int access_count;
	struct Graphics_buffer_app *graphics_buffer;
	struct Scene_viewer *core_scene_viewer;
	struct User_interface *user_interface;
	/* interaction */
	struct Event_dispatcher_idle_callback *idle_update_callback_id;
	/* Note: interactive_tool is NOT accessed by Scene_viewer; up to dialog
		 owning it to clear it if it is destroyed. This is usually ensured by having
		 a tool chooser in the parent dialog */
	struct Interactive_tool *interactive_tool;
	struct MANAGER(Interactive_tool) *interactive_tool_manager;
	/* Callbacks that are told about input (mouse clicks etc.) into the scene_viewer */
	struct LIST(CMZN_CALLBACK_ITEM(Scene_viewer_app_input_callback)) *input_callback_list;
	/* list of callbacks requested by other objects when view changes */
	struct LIST(CMZN_CALLBACK_ITEM(Scene_viewer_app_callback)) *sync_callback_list;
	cmzn_sceneviewernotifier_id notifier;
};

DECLARE_LIST_TYPES(Scene_viewer_app);
PROTOTYPE_LIST_FUNCTIONS(Scene_viewer_app);


int Scene_viewer_set_interactive_tool(struct Scene_viewer_app *scene_viewer,
	struct Interactive_tool *interactive_tool);

int cmzn_sceneviewermodule_update_Interactive_tool(cmzn_sceneviewermodule_app *cmiss_sceneviewermodule,
	struct Interactive_tool *interactive_tool);
/*******************************************************************************
LAST MODIFIED : 26 April 2007

DESCRIPTION :
Updates the interactive tools in each of the scene_viewers created with the
<cmiss_sceneviewermodule> to have the same settings as the <interactive_tool>.
This enables the old global commands to continue to work for all scene_viewers,
however new code should probably modify the particular tools for the
particular scene_viewer intended.
==============================================================================*/


int Scene_viewer_app_add_sync_callback(struct Scene_viewer_app *scene_viewer,
	CMZN_CALLBACK_FUNCTION(Scene_viewer_app_callback) *function,void *user_data);
/*******************************************************************************
LAST MODIFIED : 5 July 2000

DESCRIPTION :
==============================================================================*/

int Scene_viewer_app_remove_sync_callback(struct Scene_viewer_app *scene_viewer,
	CMZN_CALLBACK_FUNCTION(Scene_viewer_app_callback) *function,void *user_data);
/*******************************************************************************
LAST MODIFIED : 5 July 2000

DESCRIPTION :
Removes the callback calling <function> with <user_data> from
<scene_viewer>.
==============================================================================*/

Render_graphics_opengl *Scene_viewer_rendering_data_get_renderer(
	Scene_viewer_rendering_data *rendering_data);

struct Graphics_buffer_app *Scene_viewer_app_get_graphics_buffer(struct Scene_viewer_app *scene_viewer);

int Scene_viewer_get_opengl_information(struct Scene_viewer_app *scene_viewer,
	char **opengl_version, char **opengl_vendor, char **opengl_extensions,
	int *visual_id, int *colour_buffer_depth, int *depth_buffer_depth,
	int *accumulation_buffer_depth);
/*******************************************************************************
LAST MODIFIED : 9 August 2002

DESCRIPTION :
Returns the OpenGL state information.  The <opengl_version>, <opengl_vendor> and
<opengl_extensions> strings are static pointers supplied from the driver and
so should not be modified or deallocated.
==============================================================================*/
int Scene_viewer_app_idle_update_callback(void *scene_viewer_void);

int Scene_viewer_app_redraw(struct Scene_viewer_app *scene_viewer);

int Scene_viewer_app_redraw_now(struct Scene_viewer_app *scene_viewer);

int Scene_viewer_app_redraw_in_idle_time(struct Scene_viewer_app *scene_viewer);

int Scene_viewer_app_redraw_now_with_overrides(struct Scene_viewer_app *scene_viewer,
	int antialias, int transparency_layers);

int Scene_viewer_app_redraw_now_without_swapbuffers(
	struct Scene_viewer_app *scene_viewer);

struct cmzn_sceneviewermodule_app *CREATE(cmzn_sceneviewermodule_app)(
	struct Graphics_buffer_app_package *graphics_buffer_package,
	cmzn_scene_id scene,
	struct User_interface *user_interface);

int DESTROY(cmzn_sceneviewermodule_app)(struct cmzn_sceneviewermodule_app **scene_viewer_app_package_address);

struct Scene_viewer_app *CREATE(Scene_viewer_app)(struct Graphics_buffer_app *graphics_buffer,
	cmzn_sceneviewermodule_id sceneviewermodule,
	cmzn_scenefilter_id filter, struct cmzn_scene *scene,
	struct User_interface *user_interface);

struct Scene_viewer_app *Scene_viewer_app_for_spectrum_create(struct Graphics_buffer_app *graphics_buffer,
	struct Colour *background_colour,
	struct cmzn_light *default_light,
	struct cmzn_light *default_ambient_light,
	cmzn_scenefilter_id filter, struct cmzn_scene *scene,
	struct User_interface *user_interface);

int DESTROY(Scene_viewer_app)(struct Scene_viewer_app **scene_viewer_app_address);

int Scene_viewer_app_add_input_callback(struct Scene_viewer_app *scene_viewer,
	CMZN_CALLBACK_FUNCTION(Scene_viewer_app_input_callback) *function,
	void *user_data, int add_first);

int Scene_viewer_app_remove_input_callback(struct Scene_viewer_app *scene_viewer,
	CMZN_CALLBACK_FUNCTION(Scene_viewer_app_input_callback) *function,
	void *user_data);

#endif

