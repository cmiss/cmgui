/* OpenCMISS-Cmgui Application
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#if !defined (COMPUTED_FIELD_APPLY_APP_H)
#define COMPUTED_FIELD_APPLY_APP_H

/**
 * Sets up command data for alias, apply, argument fields.
 *
 * @param computed_field_package  Container for command data.
 * @param root_region  Root region for getting paths to fields in other regions.
 * @return 1 on success, 0 on failure.
 */
int Computed_field_register_types_apply(
	struct Computed_field_package *computed_field_package);

#endif /* !defined (COMPUTED_FIELD_APPLY_APP_H) */
