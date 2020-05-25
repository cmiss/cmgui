/* OpenCMISS-Cmgui Application
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
/*******************************************************************************
FILE : region_tree_viewer_wx.h

LAST MODIFIED : 26 Febuary 2007

DESCRIPTION :
Widgets for editing scene, esp. changing visibility of members.
==============================================================================*/

#if !defined (REGION_TREE_VIEWER_WX_H)
#define REGION_TREE_VIEWER_WX_H

#include "user_interface/user_interface.h"

struct Scene;
struct MANAGER(Scene);

/*
Global types
------------
*/

struct Region_tree_viewer;

/*
Global functions
----------------
*/

struct Region_tree_viewer *CREATE(Region_tree_viewer)(
	struct Region_tree_viewer **region_tree_viewer_address,
	struct cmzn_graphics_module *graphics_module,
	struct cmzn_region *root_region,
	struct MANAGER(cmzn_material) *graphical_material_manager,
	cmzn_material *default_material,
	struct cmzn_font *default_font,
	cmzn_glyphmodule *glyphmodule,
	struct MANAGER(cmzn_spectrum) *spectrum_manager,
	struct MANAGER(VT_volume_texture) *volume_texture_manager,
	struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 24 November 2005

DESCRIPTION :
Note on successful return the dialog is put at <*region_tree_viewer_address>.
==============================================================================*/

int DESTROY(Region_tree_viewer)(struct Region_tree_viewer **region_tree_viewer_address);
/*******************************************************************************
LAST MODIFIED : 31 October 2001

DESCRIPTION :
==============================================================================*/

int Region_tree_viewer_bring_to_front(struct Region_tree_viewer *region_tree_viewer);
/*******************************************************************************
LAST MODIFIED : 31 October 2001

DESCRIPTION :
De-iconifies and brings the scene editor to the front.
==============================================================================*/

#endif /* !defined (REGION_TREE_VIEWER_WX_H) */
