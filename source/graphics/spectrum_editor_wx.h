/*******************************************************************************
FILE : spectrum_editor_wx.h

LAST MODIFIED : 23 August 2007

DESCRIPTION :
Provides the widgets to manipulate graphical element group settings.
==============================================================================*/
/* OpenCMISS-Cmgui Application
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#if !defined (SPECTRUM_EDITOR_H)
#define SPECTRUM_EDITOR_H

#include "user_interface/user_interface.h"
#include "graphics/spectrum_editor_dialog_wx.h"
/*
Global Types
------------
*/
struct Spectrum_editor;


/*
Global Functions
----------------
*/

struct Spectrum_editor *CREATE(Spectrum_editor)(
	 struct Spectrum_editor_dialog **spectrum_editor_dialog_address,
	 struct cmzn_spectrum *spectrum,
	 struct cmzn_font *font,
	 struct Graphics_buffer_app_package *graphics_buffer_package,
	 struct User_interface *user_interface,
	 struct cmzn_graphics_module *graphics_module,
	 cmzn_region_id root_region,
	 struct cmzn_region *spectrum_region);
/*******************************************************************************
LAST MODIFIED : 4 May 2004

DESCRIPTION :
Creates a spectrum_editor widget.
==============================================================================*/

int spectrum_editor_wx_set_spectrum(
	struct Spectrum_editor *spectrum_editor,
	struct cmzn_spectrum *spectrum);
/*******************************************************************************
LAST MODIFIED : 23 August 2007

DESCRIPTION :
Set the <spectrum> for the <spectrum_editor_dialog>.
==============================================================================*/

int spectrum_editor_wx_add_item_to_spectrum_editor_check_list(struct cmzn_spectrum *spectrum, void *spectrum_editor_void);
/*******************************************************************************
LAST MODIFIED : 28 August 2007

DESCRIPTION :
Add spectrum item to the spectrum_editor.
==============================================================================*/

void spectrum_editor_wx_bring_up_editor(struct Spectrum_editor *spectrum_editor);
/*******************************************************************************
LAST MODIFIED : 10 Jan 2008

DESCRIPTION :
bring the spectrum editor to the front.
==============================================================================*/


int DESTROY(Spectrum_editor)(struct Spectrum_editor **spectrum_editor_address);
/*******************************************************************************
LAST MODIFIED : 12 August 2002

DESCRIPTION :
Destroys the <*spectrum_editor_address> and sets
<*spectrum_editor_address> to NULL.
==============================================================================*/

#endif /* !defined (SPECTRUM_EDITOR_H) */
