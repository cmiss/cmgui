/* OpenCMISS-Cmgui Application
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#if defined (BUILD_WITH_CMAKE)
#include "configure/cmgui_configure.h"
#endif /* defined (BUILD_WITH_CMAKE) */
#include "command/parser.h"

int set_Spectrum(struct Parse_state *state,void *spectrum_address_void,
	void *spectrum_manager_void);
/*******************************************************************************
LAST MODIFIED : 13 August 1997

DESCRIPTION :
A modifier function to set the spectrum by finding in the spectrum manager
the name given in the next token of the parser
==============================================================================*/

int set_Spectrum_minimum(struct cmzn_spectrum *spectrum,float minimum);
int set_Spectrum_maximum(struct cmzn_spectrum *spectrum,float maximum);
int set_Spectrum_minimum_command(struct Parse_state *state,void *spectrum_ptr_void,
	void *dummy_user_data);
/*******************************************************************************
LAST MODIFIED : 29 July 1998

DESCRIPTION :
A modifier function to set the spectrum minimum.
==============================================================================*/

int set_Spectrum_maximum_command(struct Parse_state *state,void *spectrum_ptr_void,
	void *dummy_user_data);
/*******************************************************************************
LAST MODIFIED : 29 July 1998

DESCRIPTION :
A modifier function to set the spectrum maximum.
==============================================================================*/

int gfx_destroy_spectrum(struct Parse_state *state,
	void *dummy_to_be_modified,void *Spectrum_manager_void);
/*******************************************************************************
LAST MODIFIED : 28 October 1997

DESCRIPTION :
Executes a GFX DESTROY SPECTRUM command.
*/

int Spectrum_list_app_contents(struct cmzn_spectrum *spectrum,void *dummy);
/*******************************************************************************
LAST MODIFIED : 5 August 1998

DESCRIPTION :
Writes the properties of the <spectrum> to the command window.
==============================================================================*/


int Spectrum_list_commands(struct cmzn_spectrum *spectrum,
	const char *command_prefix,char *command_suffix);
/*******************************************************************************
LAST MODIFIED : 5 August 1998

DESCRIPTION :
Writes the properties of the <spectrum> to the command window in a
form that can be directly pasted into a com file.
==============================================================================*/

int for_each_spectrum_list_or_write_commands(
	struct cmzn_spectrum *spectrum,void *write_enabled_void);
/*******************************************************************************
LAST MODIFIED : 18 August 2007

DESCRIPTION :
For each spectrum in manager, list the spectrum commands to the command windows or write them out.
==============================================================================*/
