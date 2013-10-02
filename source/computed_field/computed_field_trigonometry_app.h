/* OpenCMISS-Cmgui Application
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */




Computed_field *Computed_field_create_sin(
	struct cmzn_fieldmodule *field_module,
	struct Computed_field *source_field);

Computed_field *Computed_field_create_cos(
	struct cmzn_fieldmodule *field_module,
	struct Computed_field *source_field);

Computed_field *Computed_field_create_tan(
	struct cmzn_fieldmodule *field_module,
	struct Computed_field *source_field);

Computed_field *Computed_field_create_asin(
	struct cmzn_fieldmodule *field_module,
	struct Computed_field *source_field);

Computed_field *Computed_field_create_acos(
	struct cmzn_fieldmodule *field_module,
	struct Computed_field *source_field);

Computed_field *Computed_field_create_atan(
	struct cmzn_fieldmodule *field_module,
	struct Computed_field *source_field);

Computed_field *Computed_field_create_atan2(
	struct cmzn_fieldmodule *field_module,
	struct Computed_field *source_field_one,
	struct Computed_field *source_field_two);

int Computed_field_register_types_trigonometry(
	struct Computed_field_package *computed_field_package);
/*******************************************************************************
LAST MODIFIED : 10 June 2004

DESCRIPTION :
==============================================================================*/
