/*******************************************************************************
FILE : spectrum_editor_dialog.h

LAST MODIFIED : 23 Aug 2007

DESCRIPTION :
Header description for spectrum_editor_dialog widget.
==============================================================================*/
/* OpenCMISS-Cmgui Application
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#if !defined (SPECTRUM_EDITOR_DIALOG_H)
#define SPECTRUM_EDITOR_DIALOG_H

#include "general/manager.h"
#include "user_interface/user_interface.h"
#include "graphics/scene.h"

/*
Global Types
------------
*/

struct Spectrum_editor_dialog;

/*
Global Functions
----------------
*/

int bring_up_spectrum_editor_dialog(
	struct Spectrum_editor_dialog **spectrum_editor_dialog_address,
	struct MANAGER(Spectrum) *spectrum_manager,
	struct Spectrum *spectrum,
	struct cmzn_font *font,
	struct Graphics_buffer_app_package *graphics_buffer_package,
	struct User_interface *user_interface,
	struct cmzn_graphics_module *graphics_module,
	struct cmzn_region *root_region,
	struct cmzn_region *spectrum_region);
/*******************************************************************************
LAST MODIFIED : 18 November 2005

DESCRIPTION :
If there is a spectrum_editor dialog in existence, then de-iconify it and
bring it to the front, otherwise create a new one.
==============================================================================*/

int DESTROY(Spectrum_editor_dialog)(
	struct Spectrum_editor_dialog **spectrum_editor_dialog_address);
/*******************************************************************************
LAST MODIFIED : 12 August 2002

DESCRIPTION :
Destroy the <*spectrum_editor_dialog_address> and sets
<*spectrum_editor_dialog_address> to NULL.
==============================================================================*/

struct Spectrum *spectrum_editor_dialog_get_spectrum(
	struct Spectrum_editor_dialog *spectrum_editor_dialog);
/*******************************************************************************
LAST MODIFIED : 12 August 2002

DESCRIPTION :
Returns the spectrum edited by the <spectrum_editor_dialog>.
==============================================================================*/

int spectrum_editor_dialog_set_spectrum(
	struct Spectrum_editor_dialog *spectrum_editor_dialog,
	struct Spectrum *spectrum);
/*******************************************************************************
LAST MODIFIED : 12 August 2002

DESCRIPTION :
Set the <spectrum> for the <spectrum_editor_dialog>.
==============================================================================*/

#endif
