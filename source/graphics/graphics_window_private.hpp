/*******************************************************************************
FILE : graphics_window_private.hpp

LAST MODIFIED : 9 February 2007

DESCRIPTION :
Private interface for internal methods
==============================================================================*/
/* OpenCMISS-Cmgui Application
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#if !defined (GRAPHICS_WINDOW_PRIVATE_HPP)
#define GRAPHICS_WINDOW_PRIVATE_HPP

#if 1
#include "configure/cmgui_configure.h"
#endif /* defined (1) */

#if defined (WX_USER_INTERFACE)
class wxScrolledWindow;
wxScrolledWindow *Graphics_window_get_interactive_tool_panel(struct Graphics_window *graphics_window);
/*******************************************************************************
LAST MODIFIED : 9 February 2007

DESCRIPTION :
Returns the panel to embed the interactive tool into.
==============================================================================*/
#endif /* defined (WX_USER_INTERFACE) */

#endif /* !defined (GRAPHICS_WINDOW_PRIVATE_HPP) */
