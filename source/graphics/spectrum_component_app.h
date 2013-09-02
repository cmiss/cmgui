
#if !defined (SPECTRUM_COMPONENT_APP_H_)
#define SPECTRUM_COMPONENT_APP_H_

#include "general/enumerator_app.h"

struct Modify_spectrum_app_data
/*******************************************************************************
LAST MODIFIED : 4 August 1998

DESCRIPTION :
Structure modified by spectrum modify routine.
==============================================================================*/
{
	int position;
	ZnReal spectrum_minimum, spectrum_maximum;
	struct MANAGER(Computed_field) *computed_field_manager;
	struct cmzn_spectrum_component *component;
};

int gfx_modify_spectrum_settings_linear(struct Parse_state *state,
	void *modify_spectrum_data_void,void *dummy);
/*******************************************************************************
LAST MODIFIED : 17 January 2001

DESCRIPTION :
Executes a GFX MODIFY SPECTRUM LINEAR command.
If return_code is 1, returns the completed Modify_spectrum_app_data with the
parsed settings. Note that the settings are ACCESSed once on valid return.
==============================================================================*/

int gfx_modify_spectrum_settings_log(struct Parse_state *state,
	void *modify_spectrum_data_void,void *dummy);
/*******************************************************************************
LAST MODIFIED : 17 January 2001

DESCRIPTION :
Executes a GFX MODIFY SPECTRUM LOG command.
If return_code is 1, returns the completed Modify_spectrum_app_data with the
parsed settings. Note that the settings are ACCESSed once on valid return.
==============================================================================*/

int gfx_modify_spectrum_settings_field(struct Parse_state *state,
	void *modify_spectrum_data_void,void *dummy);
/*******************************************************************************
LAST MODIFIED : 11 April 2007

DESCRIPTION :
Executes a GFX MODIFY SPECTRUM FIELD command.
If return_code is 1, returns the completed Modify_spectrum_app_data with the
parsed settings. Note that the settings are ACCESSed once on valid return.
==============================================================================*/

int cmzn_spectrum_component_modify(struct cmzn_spectrum_component *component,
	struct cmzn_spectrum_component *new_component,
	struct LIST(cmzn_spectrum_component) *list_of_components);
/*******************************************************************************
LAST MODIFIED : 10 March 1998

DESCRIPTION :
Changes the contents of component to match new_component, with no change in
priority.
==============================================================================*/

PROTOTYPE_OPTION_TABLE_ADD_ENUMERATOR_FUNCTION(cmzn_spectrum_component_colour_mapping);

enum cmzn_spectrum_component_string_details
/*******************************************************************************
LAST MODIFIED : 10 March 1998

DESCRIPTION :
Parameter for selecting detail included by cmzn_spectrum_component_string:
COMPONENT_STRING_SPACE_ONLY = only those component that control the space;
COMPONENT_STRING_COMPLETE = all component including appearance.
COMPONENT_STRING_COMPLETE_PLUS = as above, but with * added if component_changed.
==============================================================================*/
{
	SPECTRUM_COMPONENT_STRING_SPACE_ONLY,
	SPECTRUM_COMPONENT_STRING_COMPLETE,
	SPECTRUM_COMPONENT_STRING_COMPLETE_PLUS
}; /* enum cmzn_spectrum_component_string_details */


struct cmzn_spectrum_component_list_data
/*******************************************************************************
LAST MODIFIED : 5 August 1998

DESCRIPTION :
Data for formating output with Spectrum_list_app_contents function.
==============================================================================*/
{
	const char *line_prefix,*line_suffix;
	enum cmzn_spectrum_component_string_details component_string_detail;
}; /* cmzn_spectrum_component_list_data */


char *cmzn_spectrum_component_string(struct cmzn_spectrum_component *component,
	enum cmzn_spectrum_component_string_details component_detail);
/*******************************************************************************
LAST MODIFIED : 10 March 1998

DESCRIPTION :
Returns a string describing the component, suitable for entry into the command
line. Parameter <component_detail> selects whether appearance component are
included in the string. User must remember to DEALLOCATE the name afterwards.
==============================================================================*/


int cmzn_spectrum_component_list_contents(struct cmzn_spectrum_component *component,
	void *list_data_void);
/*******************************************************************************
LAST MODIFIED : 5 August 1998

DESCRIPTION :
Writes out the <component> as a text string in the command window with the
<component_string_detail>, <line_prefix> and <line_suffix> given in the
<list_data>.
==============================================================================*/

int cmzn_spectrum_component_write_contents(struct cmzn_spectrum_component *component,
	void *list_data_void);
/*******************************************************************************
LAST MODIFIED : 5 August 1998

DESCRIPTION :
Writes out the <component> as a text string in the command window with the
<component_string_detail>, <line_prefix> and <line_suffix> given in the
<list_data>.
==============================================================================*/


#endif
