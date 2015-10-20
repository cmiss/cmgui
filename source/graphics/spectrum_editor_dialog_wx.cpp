/*******************************************************************************
FILE : spectrum_editor_dialog_wx.cpp

LAST MODIFIED : 23 Aug 2007

DESCRIPTION :
This module creates a spectrum_editor_dialog.
???SAB.  Basically pillaged from material/material_editor_dialog.c
==============================================================================*/
/* OpenCMISS-Cmgui Application
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <stdio.h>
#include "three_d_drawing/graphics_buffer.h"
#include "general/debug.h"
#include "general/manager.h"
#include "general/object.h"
#include "graphics/light.hpp"
#include "graphics/material.h"
#include "graphics/spectrum.h"
#include "graphics/spectrum_editor_wx.h"
#include "graphics/spectrum_editor_dialog_wx.h"
#include "general/message.h"

struct Spectrum_editor_dialog
/*******************************************************************************
LAST MODIFIED : 23 August 2007

DESCRIPTION :
Contains all the information carried by the spectrum_editor_dialog widget.
Note that we just hold a pointer to the spectrum_editor_dialog, and must access
and deaccess it.
==============================================================================*/
{
// 	struct Callback_data update_callback;
	struct cmzn_spectrum *current_value;
	struct MANAGER(cmzn_spectrum) *spectrum_manager;
	struct Scene *autorange_scene;
	struct Spectrum_editor *spectrum_editor;
	struct Spectrum_editor_dialog **spectrum_editor_dialog_address;
	struct User_interface *user_interface;
};

static struct Spectrum_editor_dialog *CREATE(Spectrum_editor_dialog)(
	 struct Spectrum_editor_dialog **spectrum_editor_dialog_address,
	 struct MANAGER(cmzn_spectrum) *spectrum_manager,
	 struct cmzn_spectrum *init_data,
	 struct cmzn_font *font,
	 struct Graphics_buffer_app_package *graphics_buffer_package,
	 struct User_interface *user_interface,
	 struct cmzn_graphics_module *graphics_module,
	 struct cmzn_region *root_region,
	 struct cmzn_region *spectrum_region)
/*******************************************************************************
LAST MODIFIED : 23 August 2007

DESCRIPTION :
Creates a dialog widget that allows the user to edit the properties of any of
the spectrums contained in the global list.
==============================================================================*/
{
// 	struct Callback_data callback;
	struct Spectrum_editor_dialog *spectrum_editor_dialog=NULL;

	ENTER(CREATE(Spectrum_editor_dialog));
	spectrum_editor_dialog = (struct Spectrum_editor_dialog *)NULL;
	/* check arguments */
	if (spectrum_manager&&user_interface)
	{
			/* allocate memory */
			if (ALLOCATE(spectrum_editor_dialog,
				struct Spectrum_editor_dialog,1))
			{
				/* initialise the structure */
				spectrum_editor_dialog->spectrum_editor_dialog_address=
					spectrum_editor_dialog_address;
				spectrum_editor_dialog->spectrum_manager=
					spectrum_manager;
				/* current_value set in spectrum_editor_dialog_set_spectrum */
				spectrum_editor_dialog->current_value=
					(struct cmzn_spectrum *)NULL;
				spectrum_editor_dialog->spectrum_editor =
					(struct Spectrum_editor *)NULL;
				spectrum_editor_dialog->autorange_scene = (struct Scene *)NULL;
				spectrum_editor_dialog->user_interface = user_interface;
				/* set the mode toggle to the correct position */
				if (!(spectrum_editor_dialog->spectrum_editor =
							CREATE(Spectrum_editor)(
								 spectrum_editor_dialog_address,
								 init_data, font,
								 graphics_buffer_package,
								 user_interface,
								 graphics_module,
								 root_region,
								 spectrum_region)))
				{
									 display_message(ERROR_MESSAGE,
											"CREATE(Spectrum_editor_dialog).  "
											"Could not create spectrum editor");
				}
			}
			else
			{
				 display_message(ERROR_MESSAGE,
						"CREATE(Spectrum_editor_dialog).  Could not allocate spectrum_editor_dialog");
			}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			 "CREATE(Spectrum_editor_dialog).  Invalid argument(s)");
	}
	if (spectrum_editor_dialog_address && spectrum_editor_dialog)
	{
		 *spectrum_editor_dialog_address = spectrum_editor_dialog;
	}
	LEAVE;

	return (spectrum_editor_dialog);
} /* CREATE(Spectrum_editor_dialog) */

struct cmzn_spectrum *spectrum_editor_dialog_get_spectrum(
	struct Spectrum_editor_dialog *spectrum_editor_dialog)
/*******************************************************************************
LAST MODIFIED : 23 August 2007

DESCRIPTION :
Returns the spectrum edited by the <spectrum_editor_dialog>.
==============================================================================*/
{
	struct cmzn_spectrum *spectrum;

	ENTER(spectrum_editor_dialog_get_spectrum);
	if (spectrum_editor_dialog)
	{
		spectrum = spectrum_editor_dialog->current_value;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"spectrum_editor_dialog_get_spectrum.  Invalid argument(s)");
		spectrum = (struct cmzn_spectrum *)NULL;
	}
	LEAVE;

	return (spectrum);
} /* spectrum_editor_dialog_get_spectrum */

int bring_up_spectrum_editor_dialog(
	struct Spectrum_editor_dialog **spectrum_editor_dialog_address,
	struct MANAGER(cmzn_spectrum) *spectrum_manager,
	struct cmzn_spectrum *spectrum,
	struct cmzn_font *font,
	struct Graphics_buffer_app_package *graphics_buffer_package,
	struct User_interface *user_interface,
	struct cmzn_graphics_module *graphics_module,
	struct cmzn_region *root_region,
	struct cmzn_region *spectrum_region)
/*******************************************************************************
LAST MODIFIED : 18 November 2005

DESCRIPTION :
If there is a spectrum_editor dialog in existence, then de-iconify it and
bring it to the front, otherwise create a new one.
==============================================================================*/
{
	int return_code = 0;
	struct Spectrum_editor_dialog *spectrum_editor_dialog;

	ENTER(bring_up_spectrum_editor_dialog);
	if (spectrum_editor_dialog_address)
	{
		spectrum_editor_dialog = *spectrum_editor_dialog_address;
		if (spectrum_editor_dialog != 0)
		{
			spectrum_editor_wx_bring_up_editor(spectrum_editor_dialog->spectrum_editor);
			return_code = 1;
		}
		else
		{
			if (CREATE(Spectrum_editor_dialog)(spectrum_editor_dialog_address,
					spectrum_manager, spectrum, font, graphics_buffer_package,
					user_interface, graphics_module, root_region,
					spectrum_region))
			{
				return_code = 1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"bring_up_spectrum_editor_dialog.  Error creating dialog");
				return_code = 0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"bring_up_spectrum_editor_dialog.  Invalid argument");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* bring_up_spectrum_editor_dialog */

int DESTROY(Spectrum_editor_dialog)(
	struct Spectrum_editor_dialog **spectrum_editor_dialog_address)
/*******************************************************************************
LAST MODIFIED : 12 August 2002

DESCRIPTION :
Destroy the <*spectrum_editor_dialog_address> and sets
<*spectrum_editor_dialog_address> to NULL.
==============================================================================*/
{
	int return_code;
	struct Spectrum_editor_dialog *spectrum_editor_dialog;

	ENTER(spectrum_editor_dialog_destroy);
	if (spectrum_editor_dialog_address &&
		(spectrum_editor_dialog= *spectrum_editor_dialog_address))
	{
		return_code = 1;
		DESTROY(Spectrum_editor)(&(spectrum_editor_dialog->spectrum_editor));
		DEALLOCATE(*spectrum_editor_dialog_address);
		*spectrum_editor_dialog_address = (struct Spectrum_editor_dialog *)NULL;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(Spectrum_editor_dialog).  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Spectrum_editor_dialog) */
