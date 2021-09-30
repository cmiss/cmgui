/* OpenCMISS-Cmgui Application
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */


#if !defined (COMPUTED_FIELD_THRESHOLD_IMAGE_FILTER_APP_H_)
#define COMPUTED_FIELD_THRESHOLD_IMAGE_FILTER_APP_H_

#include "opencmiss/zinc/types/fieldimageprocessingid.h"
#include "general/enumerator_app.h"

PROTOTYPE_OPTION_TABLE_ADD_ENUMERATOR_FUNCTION(cmzn_field_imagefilter_threshold_condition);

int Computed_field_register_types_threshold_image_filter(
	struct Computed_field_package *computed_field_package);

#endif
