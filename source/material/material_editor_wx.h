/*******************************************************************************
FILE : material_editor_wx.h

LAST MODIFIED : 6 Nov 2007

DESCRIPTION :
Widgets for editing a graphical material.
==============================================================================*/
/* OpenCMISS-Cmgui Application
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#if !defined (MATERIAL_EDITOR_WX_H)
#define MATERIAL_EDITOR_WX_H

#include "graphics/material.h"
#include "graphics/material_app.h"
#include "user_interface/user_interface.h"

#define MATERIAL_NUM_FORMAT "%6.4" MATERIAL_PRECISION_STRING
struct cmzn_graphics_module;
struct Graphics_buffer_app_package;
struct Material_editor;

int material_editor_bring_up_editor(struct Material_editor **material_editor_address,
	struct cmzn_region *root_region,
	struct cmzn_graphics_module *graphics_module,
	Graphics_buffer_app_package *graphics_buffer_package,
	struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 10 Jan 2008

DESCRIPTION :
bring the material editor to the front.
==============================================================================*/

int material_editor_wx_set_material(
	 struct Material_editor *material_editor, cmzn_material *material);
/*******************************************************************************
LAST MODIFIED : 6 November 2007

DESCRIPTION :
Sets the <material> to be edited by the <material_editor>.
==============================================================================*/

void material_editor_update_colour(void *material_editor_void);
/*******************************************************************************
LAST MODIFIED : 30 November 2007

DESCRIPTION :
Update the material colour and settings in the material editor .
==============================================================================*/

int DESTROY(Material_editor)(struct Material_editor **material_editor_address);
#endif /* #define MATERIAL_EDITOR_WX_H */
