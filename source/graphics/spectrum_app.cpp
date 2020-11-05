/* OpenCMISS-Cmgui Application
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#include <stdlib.h>
#include <stdio.h>
#include "general/message.h"
#include "general/mystring.h"
#include "command/parser.h"
#include "graphics/scene.hpp"
#include "graphics/graphics_module.hpp"
#include "graphics/spectrum.h"
#include "graphics/render_gl.h"
#include "graphics/spectrum_component.h"
#include "graphics/spectrum_component_app.h"
#include "graphics/scene_app.h"
#include "user_interface/process_list_or_write_command.hpp"


int set_Spectrum(struct Parse_state *state,void *spectrum_address_void,
	void *spectrum_manager_void)
/*******************************************************************************
LAST MODIFIED : 22 June 1999

DESCRIPTION :
A modifier function to set the spectrum by finding in the spectrum manager
the name given in the next token of the parser
==============================================================================*/
{
	const char *current_token;
	int return_code;
	struct cmzn_spectrum *temp_spectrum,**spectrum_address;
	struct MANAGER(cmzn_spectrum) *spectrum_manager;

	ENTER(set_Spectrum);
	if (state)
	{
		current_token=state->current_token;
		if (current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				spectrum_address=(struct cmzn_spectrum **)spectrum_address_void;
				spectrum_manager=(struct MANAGER(cmzn_spectrum) *)spectrum_manager_void;
				if (spectrum_address && spectrum_manager)
				{
					if (fuzzy_string_compare(current_token,"NONE"))
					{
						if (*spectrum_address)
						{
							DEACCESS(cmzn_spectrum)(spectrum_address);
							*spectrum_address=(struct cmzn_spectrum *)NULL;
						}
						return_code=1;
					}
					else
					{
						temp_spectrum=FIND_BY_IDENTIFIER_IN_MANAGER(cmzn_spectrum,	name)(current_token,spectrum_manager);
						if (temp_spectrum)
						{
							if (*spectrum_address!=temp_spectrum)
							{
								DEACCESS(cmzn_spectrum)(spectrum_address);
								*spectrum_address=ACCESS(cmzn_spectrum)(temp_spectrum);
							}
							return_code=1;
						}
						else
						{
							display_message(ERROR_MESSAGE,"Unknown spectrum : %s",
								current_token);
							return_code=0;
						}
					}
					shift_Parse_state(state,1);
				}
				else
				{
					display_message(ERROR_MESSAGE,
"set_Spectrum.  Invalid argument(s).  spectrum_address %p.  spectrum_manager %p",
						spectrum_address_void,spectrum_manager_void);
					return_code=0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE," SPECTRUM_NAME|none");
				/* if possible, then write the name */
				spectrum_address = (struct cmzn_spectrum **)spectrum_address_void;
				if (spectrum_address)
				{
					temp_spectrum = *spectrum_address;
					if (temp_spectrum)
					{
						display_message(INFORMATION_MESSAGE,"[%s]",temp_spectrum->name);
					}
					else
					{
						display_message(INFORMATION_MESSAGE,"[none]");
					}
				}
				return_code=1;
			}
		}
		else
		{
			display_message(WARNING_MESSAGE,"Missing material name");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_Spectrum.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_Spectrum */

int set_Spectrum_minimum_command(struct Parse_state *state,
	void *spectrum_ptr_void,void *dummy_user_data)
/*******************************************************************************
LAST MODIFIED : 13 August 1997

DESCRIPTION :
A modifier function to set the spectrum minimum.
==============================================================================*/
{
	const char *current_token;
	float value;
	int return_code;
	struct cmzn_spectrum *spectrum;

	ENTER(set_Spectrum_minimum);
	if (state && (!dummy_user_data))
	{
		current_token=state->current_token;
		if (current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				spectrum = *((struct cmzn_spectrum **)spectrum_ptr_void);
				if (spectrum)
				{
					if (1==sscanf(current_token," %f ",&value))
					{
						set_Spectrum_minimum(spectrum,value);

						return_code=shift_Parse_state(state,1);
					}
					else
					{
						display_message(ERROR_MESSAGE,"Invalid spectrum minimum : %s",
							current_token);
						display_parse_state_location(state);
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"set_Spectrum_minimum_command.  Missing spectrum_ptr_void");
					return_code=0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE," MINIMUM_VALUE#");
				spectrum = *((struct cmzn_spectrum **)spectrum_ptr_void);
				if (spectrum)
				{
					display_message(INFORMATION_MESSAGE,"[%g]",spectrum->minimum);
				}
				else
				{
					display_message(INFORMATION_MESSAGE,"[calculated]");
				}
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Missing spectrum minimum");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_Spectrum_minimum_command.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_Spectrum_minimum_command */

int set_Spectrum_maximum_command(struct Parse_state *state,
	void *spectrum_ptr_void,void *dummy_user_data)
/*******************************************************************************
LAST MODIFIED : 13 August 1997

DESCRIPTION :
A modifier function to set the spectrum maximum.
==============================================================================*/
{
	const char *current_token;
	float value;
	int return_code;
	struct cmzn_spectrum *spectrum;

	ENTER(set_Spectrum_maximum);
	if (state && (!dummy_user_data))
	{
		current_token=state->current_token;
		if (current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				spectrum = *((struct cmzn_spectrum **)spectrum_ptr_void);
				if (spectrum)
				{
					if (1==sscanf(current_token," %f ",&value))
					{
						set_Spectrum_maximum(spectrum,value);
						return_code=shift_Parse_state(state,1);
					}
					else
					{
						display_message(WARNING_MESSAGE,"Invalid spectrum maximum : %s",
							current_token);
						display_parse_state_location(state);
						return_code=1;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"set_Spectrum_maximum_command.  Missing spectrum_ptr_void");
					return_code=0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE," MAXIMUM_VALUE#");
				spectrum = *((struct cmzn_spectrum **)spectrum_ptr_void);
				if (spectrum)
				{
					display_message(INFORMATION_MESSAGE,"[%g]",spectrum->maximum);
				}
				else
				{
					display_message(INFORMATION_MESSAGE,"[calculated]");
				}
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Missing spectrum maximum");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_Spectrum_maximum_command.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_Spectrum_maximum_command */

int gfx_destroy_spectrum(struct Parse_state *state,
	void *dummy_to_be_modified,void *spectrum_manager_void)
/*******************************************************************************
LAST MODIFIED : 28 October 1997

DESCRIPTION :
Executes a GFX DESTROY SPECTRUM command.
==============================================================================*/
{
	const char *current_token;
	int return_code;
	struct MANAGER(cmzn_spectrum) *spectrum_manager;
	struct cmzn_spectrum *spectrum;

	ENTER(gfx_destroy_spectrum);
	if (state && (!dummy_to_be_modified))
	{
		spectrum_manager = (struct MANAGER(cmzn_spectrum) *)spectrum_manager_void;
		if (spectrum_manager)
		{
			current_token=state->current_token;
			if (current_token)
			{
				if (strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
				{
					spectrum=FIND_BY_IDENTIFIER_IN_MANAGER(cmzn_spectrum,name)(current_token,spectrum_manager);
					if (spectrum)
					{
						return_code=REMOVE_OBJECT_FROM_MANAGER(cmzn_spectrum)(spectrum,
							spectrum_manager);
					}
					else
					{
						display_message(ERROR_MESSAGE,"Unknown spectrum: %s",current_token);
						display_parse_state_location(state);
						return_code=0;
					}
				}
				else
				{
					display_message(INFORMATION_MESSAGE," SPECTRUM_NAME");
					return_code=1;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,"Missing spectrum name");
				display_parse_state_location(state);
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_destroy_spectrum.  Missing spectrum_manager_void");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_destroy_spectrum.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_destroy_spectrum */

int process_list_or_write_Spectrum_commands(struct cmzn_spectrum *spectrum,
	 const char *command_prefix,char *command_suffix,
	 class Process_list_or_write_command_class *process_message)
/*******************************************************************************
LAST MODIFIED : 24 August 2007

DESCRIPTION :
List the properties of the <spectrum> to the command window or write
to the comfile.
==============================================================================*/
{
	char *line_prefix,*name;
	int return_code;
	struct cmzn_spectrumcomponent_list_data list_data;
	ENTER(Spectrum_list_commands);
	/* check the arguments */
	if (spectrum)
	{
		name=duplicate_string(spectrum->name);
		if (name)
		{
			/* put quotes around name if it contains special characters */
			make_valid_token(&name);
			/* add spectrum name to command_prefix */
			if (strcmp(name, "default") != 0)
			{
				 process_message->process_command(INFORMATION_MESSAGE,"gfx create spectrum %s",name);
				 process_message->process_command(INFORMATION_MESSAGE,";\n");
			}
			if (ALLOCATE(line_prefix,char,strlen(command_prefix)+strlen(name)+3))
			{
				sprintf(line_prefix,"%s %s ",command_prefix,name);
				process_message->process_command(INFORMATION_MESSAGE,line_prefix);
				process_message->process_command(INFORMATION_MESSAGE,"clear");
				if (spectrum->overwrite_colour)
				{
					process_message->process_command(INFORMATION_MESSAGE," overwrite_colour");
				}
				else
				{
					process_message->process_command(INFORMATION_MESSAGE," overlay_colour");
				}
				if (command_suffix)
				{
					process_message->process_command(INFORMATION_MESSAGE,command_suffix);
				}
				process_message->process_command(INFORMATION_MESSAGE,";\n");
				list_data.component_string_detail=SPECTRUM_COMPONENT_STRING_COMPLETE;
				list_data.line_prefix=line_prefix;
				list_data.line_suffix=command_suffix;
				return_code = process_message->write_enabled();
				if (return_code == 0)
				{
					 return_code=FOR_EACH_OBJECT_IN_LIST(cmzn_spectrumcomponent)(
							cmzn_spectrumcomponent_list_contents, (void *)&list_data,
							spectrum->list_of_components);
				}
				else
				{
					 return_code=FOR_EACH_OBJECT_IN_LIST(cmzn_spectrumcomponent)(
							cmzn_spectrumcomponent_write_contents, (void *)&list_data,
							spectrum->list_of_components);
				}
				DEALLOCATE(line_prefix);
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Spectrum_list_commands.  Unable to allocate string");
				return_code=0;
			}
			DEALLOCATE(name);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Spectrum_list_commands.  Unable to duplicate string");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Spectrum_list_commands.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Spectrum_list_commands */

int for_each_spectrum_list_or_write_commands(struct cmzn_spectrum *spectrum,void *write_enabled_void)
/*******************************************************************************
LAST MODIFIED : 18 August 2007

DESCRIPTION :
For each spectrum in manager, list the spectrum commands to the command windows or write them out.
==============================================================================*/
{
	 static const char *command_prefix="gfx modify spectrum";
	 char *write_enabled;
	 int return_code;
	 ENTER(for_each_spectrum_list_or_write_commands);
	 write_enabled = (char *)write_enabled_void;
	 if (write_enabled)
	 {
			if (strcmp(write_enabled, "false") == 0)
			{
				 if (Process_list_command_class *list_message =
						new Process_list_command_class())
				 {
						return_code = process_list_or_write_Spectrum_commands(spectrum, command_prefix,
							 (char *)NULL, list_message);
						delete list_message;
				 }
				 else
				 {
						return_code = 0;
						display_message(ERROR_MESSAGE,"Spectrum_list_commands. Cannot create a process_command_class");
				 }
			}
			else
			{
				 if (Process_write_command_class *write_message =
						new Process_write_command_class())
				 {
						return_code = process_list_or_write_Spectrum_commands(spectrum, command_prefix,
							 (char *)NULL, write_message);
						delete write_message;
				 }
				 else
				 {
						return_code = 0;
						display_message(ERROR_MESSAGE,"Spectrum_list_commands. Cannot create a process_command_class");
				 }
			}
	 }
	 else
	 {
			return_code = 0;
			display_message(ERROR_MESSAGE,"for_each_spectrum_list_or_write_commands.  Invalid argument(s)");
	 }
	 LEAVE;

	 return (return_code);
}

int Spectrum_list_commands(struct cmzn_spectrum *spectrum,
	const char *command_prefix,char *command_suffix)
/*******************************************************************************
LAST MODIFIED : 24 Aug 2007

DESCRIPTION :
Writes the properties of the <spectrum> to the command window.
==============================================================================*/
{
	 int return_code;
	 ENTER(Spectrum_list_commands);
	 if (spectrum)
	 {
			if (Process_list_command_class *list_message =
				 new Process_list_command_class())
			{
				 return_code = process_list_or_write_Spectrum_commands(spectrum, command_prefix,
						command_suffix, list_message);
				 delete list_message;
			}
			else
			{
				 return_code = 0;
				 display_message(ERROR_MESSAGE,"Spectrum_list_commands. Cannot create a process_command_class");
			}
	 }
	 else
	 {
			return_code = 0;
			display_message(ERROR_MESSAGE,"Spectrum_list_commands.  Invalid argument(s)");
	 }
	 return (return_code);
}

int Spectrum_list_app_contents(struct cmzn_spectrum *spectrum,void *dummy)
/*******************************************************************************
LAST MODIFIED : 18 May 2000

DESCRIPTION :
Writes the properties of the <spectrum> to the command window.
==============================================================================*/
{
	enum Spectrum_simple_type simple_type;
	int return_code;
	struct cmzn_spectrumcomponent_list_data list_data;

	ENTER(Spectrum_list_app_contents);
	/* check the arguments */
	if (spectrum && (!dummy))
	{
		display_message(INFORMATION_MESSAGE,"spectrum : ");
		display_message(INFORMATION_MESSAGE,spectrum->name);
		display_message(INFORMATION_MESSAGE,"\n");
		simple_type = Spectrum_get_simple_type(spectrum);
		switch(simple_type)
		{
			case BLUE_TO_RED_SPECTRUM:
			{
				display_message(INFORMATION_MESSAGE,"  simple spectrum type: BLUE_TO_RED\n");
			} break;
			case RED_TO_BLUE_SPECTRUM:
			{
				display_message(INFORMATION_MESSAGE,"  simple spectrum type: RED_TO_BLUE\n");
			} break;
			case LOG_BLUE_TO_RED_SPECTRUM:
			{
				display_message(INFORMATION_MESSAGE,"  simple spectrum type: LOG_BLUE_TO_RED\n");
			} break;
			case LOG_RED_TO_BLUE_SPECTRUM:
			{
				display_message(INFORMATION_MESSAGE,"  simple spectrum type: LOG_RED_TO_BLUE\n");
			} break;
			case BLUE_WHITE_RED_SPECTRUM:
			{
				display_message(INFORMATION_MESSAGE,"  simple spectrum type: BLUE_WHITE_RED\n");
			} break;
			default:
			{
				display_message(INFORMATION_MESSAGE,"  simple spectrum type: UNKNOWN\n");
			} break;
		}
		display_message(INFORMATION_MESSAGE,"  minimum=%.3g, maximum=%.3g\n",
			spectrum->minimum,spectrum->maximum);
		if (spectrum->overwrite_colour)
		{
			display_message(INFORMATION_MESSAGE,"  clear before component\n");
		}
		list_data.component_string_detail=SPECTRUM_COMPONENT_STRING_COMPLETE_PLUS;
		list_data.line_prefix="  ";
		list_data.line_suffix="";
		return_code=FOR_EACH_OBJECT_IN_LIST(cmzn_spectrumcomponent)(
			cmzn_spectrumcomponent_list_contents,(void *)&list_data,
			spectrum->list_of_components);
		display_message(INFORMATION_MESSAGE,"  access count=%d\n",
			spectrum->access_count);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"Spectrum_list_app_contents.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Spectrum_list_app_contents */
