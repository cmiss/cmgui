/*******************************************************************************
FILE : spectrum.h

LAST MODIFIED : 15 October 1998

DESCRIPTION :
Spectrum structures and support code.
==============================================================================*/
#if !defined(SPECTRUM_H)
#define SPECTRUM_H
#include "general/list.h"
#include "general/manager.h"
#include "general/object.h"
#include "command/parser.h"

struct Graphical_material;
struct Spectrum_settings;

/*
Global types
------------
*/


struct Spectrum;
/*******************************************************************************
LAST MODIFIED : 18 October 1997

DESCRIPTION :
Spectrum type is private.
==============================================================================*/

DECLARE_LIST_TYPES(Spectrum);

DECLARE_MANAGER_TYPES(Spectrum);

struct Spectrum_render_data
/*******************************************************************************
LAST MODIFIED : 15 October 1998

DESCRIPTION :
Used to pass information through iterator function to each setting when
rendering.
==============================================================================*/
{
	struct Graphical_material *material;
	float *data;
	int rendering_flags;
	int number_of_data_components;
}; /* struct Spectrum_render_data */

enum Spectrum_simple_type
{
	UNKNOWN_SPECTRUM,
	RED_TO_BLUE_SPECTRUM,
	BLUE_TO_RED_SPECTRUM,
	LOG_RED_TO_BLUE_SPECTRUM,
	LOG_BLUE_TO_RED_SPECTRUM
};

/*
Global functions
----------------
*/
PROTOTYPE_OBJECT_FUNCTIONS(Spectrum);

PROTOTYPE_COPY_OBJECT_FUNCTION(Spectrum);

PROTOTYPE_LIST_FUNCTIONS(Spectrum);

PROTOTYPE_FIND_BY_IDENTIFIER_IN_LIST_FUNCTION(Spectrum,name,char *);

PROTOTYPE_MANAGER_COPY_FUNCTIONS(Spectrum,name,char *);

PROTOTYPE_MANAGER_FUNCTIONS(Spectrum);

PROTOTYPE_MANAGER_IDENTIFIER_FUNCTIONS(Spectrum,name,char *);

int Spectrum_set_simple_type(struct Spectrum *spectrum,
	enum Spectrum_simple_type type);
/*******************************************************************************
LAST MODIFIED : 6 August 1998

DESCRIPTION :
A convienience routine that allows a spectrum to be automatically set into
some predetermined simple types.
==============================================================================*/

enum Spectrum_simple_type Spectrum_get_simple_type(struct Spectrum *spectrum);
/*******************************************************************************
LAST MODIFIED : 28 July 1998

DESCRIPTION :
A convienience routine that interrogates a spectrum to see if it is one of the
simple types.  If it does not comform exactly to one of the simple types then
it returns UNKNOWN_SPECTRUM
==============================================================================*/

int Spectrum_add_settings(struct Spectrum *spectrum,
	struct Spectrum_settings *settings,int position);
/*******************************************************************************
LAST MODIFIED : 24 July 1998

DESCRIPTION :
Adds the <settings> to <spectrum> at the given <position>, where 1 is
the top of the list (rendered first), and values less than 1 or greater than the
last position in the list cause the settings to be added at its end, with a
position one greater than the last.
==============================================================================*/

int Spectrum_remove_settings(struct Spectrum *spectrum,
	struct Spectrum_settings *settings);
/*******************************************************************************
LAST MODIFIED : 24 July 1998

DESCRIPTION :
Removes the <settings> from <spectrum> and decrements the position
of all subsequent settings.
==============================================================================*/

int Spectrum_remove_all_settings(struct Spectrum *spectrum);
/*******************************************************************************
LAST MODIFIED : 4 August 1998

DESCRIPTION :
Removes the all the settings from <spectrum>.
==============================================================================*/

int Spectrum_get_settings_position(struct Spectrum *spectrum,
	struct Spectrum_settings *settings);
/*******************************************************************************
LAST MODIFIED : 24 July 1998

DESCRIPTION :
Returns the position of <settings> in <spectrum>.
==============================================================================*/

int set_Spectrum(struct Parse_state *state,void *spectrum_address_void,
	void *spectrum_manager_void);
/*******************************************************************************
LAST MODIFIED : 13 August 1997

DESCRIPTION :
A modifier function to set the spectrum by finding in the spectrum manager
the name given in the next token of the parser
==============================================================================*/

int set_Spectrum_minimum(struct Spectrum *spectrum,float minimum);
/*******************************************************************************
LAST MODIFIED : 29 July 1998

DESCRIPTION :
A function to set the spectrum minimum.
==============================================================================*/

int set_Spectrum_maximum(struct Spectrum *spectrum,float maximum);
/*******************************************************************************
LAST MODIFIED : 29 July 1998

DESCRIPTION :
A function to set the spectrum maximum.
==============================================================================*/

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

float get_Spectrum_minimum(struct Spectrum *spectrum);
/*******************************************************************************
LAST MODIFIED : 13 August 1997

DESCRIPTION :
Returns the value of the spectrum minimum.
==============================================================================*/

float get_Spectrum_maximum(struct Spectrum *spectrum);
/*******************************************************************************
LAST MODIFIED : 13 August 1997

DESCRIPTION :
Returns the value of the spectrum maximum.
==============================================================================*/

int Spectrum_calculate_range(struct Spectrum *spectrum);
/*******************************************************************************
LAST MODIFIED : 22 July 1998

DESCRIPTION :
Calculates the range of the spectrum from the settings it contains and updates
the minimum and maximum contained inside it.
==============================================================================*/

int Spectrum_set_minimum_and_maximum(struct Spectrum *spectrum,
	float minimum, float maximum);
/*******************************************************************************
LAST MODIFIED : 29 July 1998

DESCRIPTION :
Expands the range of this spectrum by adjusting the range of each settings
it contains.  The ratios of the different settings are preserved.
==============================================================================*/

int Spectrum_get_opaque_colour_flag(struct Spectrum *spectrum);
/*******************************************************************************
LAST MODIFIED : 30 July 1998

DESCRIPTION :
Returns the value of the spectrum opaque flag which indicates whether the
spectrum clears the material colour before applying the settings or not.
==============================================================================*/

int Spectrum_set_opaque_colour_flag(struct Spectrum *spectrum, int opaque);
/*******************************************************************************
LAST MODIFIED : 30 July 1998

DESCRIPTION :
Sets the value of the spectrum opaque flag which indicates whether the
spectrum clears the material colour before applying the settings or not.
==============================================================================*/

#if defined (OPENGL_API)
struct Spectrum_render_data *spectrum_start_renderGL(
	struct Spectrum *spectrum,struct Graphical_material *material,
	int number_of_data_components);
/*******************************************************************************
LAST MODIFIED : 3 June 1999

DESCRIPTION :
Initialises the graphics state for rendering values on the current material.
==============================================================================*/

int spectrum_renderGL_value(struct Spectrum *spectrum,
	struct Graphical_material *material,struct Spectrum_render_data *render_data,
	float *data);
/*******************************************************************************
LAST MODIFIED : 1 June 1999

DESCRIPTION :
Sets the graphics rendering state to represent the value 'data' in
accordance with the spectrum.
==============================================================================*/

int spectrum_end_renderGL(struct Spectrum *spectrum,
	struct Spectrum_render_data *render_data);
/*******************************************************************************
LAST MODIFIED : 13 March 1998

DESCRIPTION :
Resets the graphics state after rendering values on current material.
==============================================================================*/
#endif /* defined (OPENGL_API) */

int spectrum_render_value_on_material(struct Spectrum *spectrum,
	struct Graphical_material *material, int number_of_data_components,
	float *data);
/*******************************************************************************
LAST MODIFIED : 15 June 1999

DESCRIPTION :
Uses the <spectrum> to modify the <material> to represent the <number_of_data_components>
<data> values given.
==============================================================================*/

int spectrum_value_to_rgb(struct Spectrum *spectrum,int number_of_data_components,
	float *data,float *red, float *green,float *blue);
/*******************************************************************************
LAST MODIFIED : 30 July 1998

DESCRIPTION :
Uses the <spectrum> to calculate RGB components to represent the 
<number_of_data_components> <data> values.
The colour returned is diffuse colour value of a spectrum modified black material.
This function is inefficient as a material is created and destroyed every time,
preferable to make your own base material, use spectrum_render_value_on_material,
and then interpret the resulting material how you want.
==============================================================================*/

struct LIST(Spectrum_settings) *get_Spectrum_settings_list(
	struct Spectrum *spectrum );
/*******************************************************************************
LAST MODIFIED : 12 March 1998

DESCRIPTION :
Returns the settings list that describes the spectrum.  This
is the pointer to the object inside the spectrum so do not
destroy it, any changes to it change that spectrum
==============================================================================*/

int gfx_destroy_spectrum(struct Parse_state *state,
	void *dummy_to_be_modified,void *Spectrum_manager_void);
/*******************************************************************************
LAST MODIFIED : 28 October 1997

DESCRIPTION :
Executes a GFX DESTROY SPECTRUM command.
==============================================================================*/

int Spectrum_list_commands(struct Spectrum *spectrum,
	char *command_prefix,char *command_suffix);
/*******************************************************************************
LAST MODIFIED : 5 August 1998

DESCRIPTION :
Writes the properties of the <spectrum> to the command window in a
form that can be directly pasted into a com file.
==============================================================================*/

int Spectrum_list_contents(struct Spectrum *spectrum,void *dummy);
/*******************************************************************************
LAST MODIFIED : 5 August 1998

DESCRIPTION :
Writes the properties of the <spectrum> to the command window.
==============================================================================*/

struct Spectrum *CREATE(Spectrum)(char *name);
/*******************************************************************************
LAST MODIFIED : 13 August 1997

DESCRIPTION :
Allocates memory and assigns fields for a Spectrum object.
==============================================================================*/

int DESTROY(Spectrum)(struct Spectrum **spectrum_ptr);
/*******************************************************************************
LAST MODIFIED : 11 August 1997

DESCRIPTION :
Frees the memory for the fields of <**spectrum>, frees the memory for
<**spectrum> and sets <*spectrum> to NULL.
==============================================================================*/

PROTOTYPE_GET_OBJECT_NAME_FUNCTION(Spectrum);

#endif /* !defined(SPECTRUM_H) */
