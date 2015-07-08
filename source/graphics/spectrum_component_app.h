/* OpenCMISS-Cmgui Application
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#if !defined (SPECTRUM_COMPONENT_APP_H_)
#define SPECTRUM_COMPONENT_APP_H_

#include "general/enumerator_app.h"

/**
 * Structure modified by spectrum command modifier functions.
 */
struct Modify_spectrum_app_data
{
	int position;
	ZnReal spectrum_minimum, spectrum_maximum;
	struct MANAGER(Computed_field) *computed_field_manager;
	struct cmzn_spectrumcomponent *component;
};

/**
 * Executes a GFX MODIFY SPECTRUM LINEAR command.
 * If return_code is 1, returns the completed Modify_spectrum_app_data with the
 * parsed settings. Note that the settings are ACCESSed once on valid return.
 */
int gfx_modify_spectrum_settings_linear(struct Parse_state *state,
	void *modify_spectrum_data_void, void *dummy);

/**
 * Executes a GFX MODIFY SPECTRUM LOG command.
 * If return_code is 1, returns the completed Modify_spectrum_app_data with the
 * parsed settings. Note that the settings are ACCESSed once on valid return.
 */
int gfx_modify_spectrum_settings_log(struct Parse_state *state,
	void *modify_spectrum_data_void, void *dummy);

/**
 * Executes a GFX MODIFY SPECTRUM FIELD command.
 * If return_code is 1, returns the completed Modify_spectrum_app_data with the
 * parsed settings. Note that the settings are ACCESSed once on valid return.
 */
int gfx_modify_spectrum_settings_field(struct Parse_state *state,
	void *modify_spectrum_data_void, void *dummy);

int cmzn_spectrumcomponent_modify(struct cmzn_spectrumcomponent *component,
	struct cmzn_spectrumcomponent *new_component,
	struct LIST(cmzn_spectrumcomponent) *list_of_components);
/*******************************************************************************
LAST MODIFIED : 10 March 1998

DESCRIPTION :
Changes the contents of component to match new_component, with no change in
priority.
==============================================================================*/

PROTOTYPE_OPTION_TABLE_ADD_ENUMERATOR_FUNCTION(cmzn_spectrumcomponent_colour_mapping_type);

enum cmzn_spectrumcomponent_string_details
/*******************************************************************************
LAST MODIFIED : 10 March 1998

DESCRIPTION :
Parameter for selecting detail included by cmzn_spectrumcomponent_string:
COMPONENT_STRING_SPACE_ONLY = only those component that control the space;
COMPONENT_STRING_COMPLETE = all component including appearance.
COMPONENT_STRING_COMPLETE_PLUS = as above, but with * added if component_changed.
==============================================================================*/
{
	SPECTRUM_COMPONENT_STRING_SPACE_ONLY,
	SPECTRUM_COMPONENT_STRING_COMPLETE,
	SPECTRUM_COMPONENT_STRING_COMPLETE_PLUS
}; /* enum cmzn_spectrumcomponent_string_details */


struct cmzn_spectrumcomponent_list_data
/*******************************************************************************
LAST MODIFIED : 5 August 1998

DESCRIPTION :
Data for formating output with Spectrum_list_app_contents function.
==============================================================================*/
{
	const char *line_prefix,*line_suffix;
	enum cmzn_spectrumcomponent_string_details component_string_detail;
}; /* cmzn_spectrumcomponent_list_data */


char *cmzn_spectrumcomponent_string(struct cmzn_spectrumcomponent *component,
	enum cmzn_spectrumcomponent_string_details component_detail);
/*******************************************************************************
LAST MODIFIED : 10 March 1998

DESCRIPTION :
Returns a string describing the component, suitable for entry into the command
line. Parameter <component_detail> selects whether appearance component are
included in the string. User must remember to DEALLOCATE the name afterwards.
==============================================================================*/


int cmzn_spectrumcomponent_list_contents(struct cmzn_spectrumcomponent *component,
	void *list_data_void);
/*******************************************************************************
LAST MODIFIED : 5 August 1998

DESCRIPTION :
Writes out the <component> as a text string in the command window with the
<component_string_detail>, <line_prefix> and <line_suffix> given in the
<list_data>.
==============================================================================*/

int cmzn_spectrumcomponent_write_contents(struct cmzn_spectrumcomponent *component,
	void *list_data_void);
/*******************************************************************************
LAST MODIFIED : 5 August 1998

DESCRIPTION :
Writes out the <component> as a text string in the command window with the
<component_string_detail>, <line_prefix> and <line_suffix> given in the
<list_data>.
==============================================================================*/


#endif
