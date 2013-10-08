/* OpenCMISS-Cmgui Application
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "general/manager.h"

struct Computed_field_package;
struct Graphics_window;

/** Register types for command usage */
int Computed_field_register_types_sceneviewer_projection(
	struct Computed_field_package *computed_field_package,
	struct MANAGER(Graphics_window) *graphics_window_manager);
