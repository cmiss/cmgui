/* OpenCMISS-Cmgui Application
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#if 1
#include "configure/cmgui_configure.h"
#endif /* defined (BUILD_WITH_CMAKE) */
#include <string>
#include "zinc/font.h"
#include "zinc/font.h"
#include "general/enumerator.h"
#include "general/enumerator_private.hpp"
#include "general/message.h"
#include "command/parser.h"
#include "general/debug.h"
#include "graphics/font.h"


int gfx_define_font(struct Parse_state *state,
	void *dummy_to_be_modified,void *fontmodule_void)
/*******************************************************************************
LAST MODIFIED : 12 March 2008

DESCRIPTION :
Executes a GFX DEFINE FONT command.
==============================================================================*/
{
	const char *current_token, *font_name;
	int return_code;
	cmzn_fontmodule_id fontmodule = 0;

	if (state && (fontmodule = (cmzn_fontmodule_id)fontmodule_void))
	{
		if (NULL != (current_token = state->current_token))
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				font_name = current_token;
				if (shift_Parse_state(state,1)&&
					(current_token=state->current_token))
				{
					cmzn_font_id font = cmzn_fontmodule_find_font_by_name(
						fontmodule, font_name);
					if (!font)
					{
						font = cmzn_fontmodule_create_font(fontmodule);
						cmzn_font_set_name(font, font_name);
					}
					cmzn_font_render_type render_type = cmzn_font_get_render_type(font);
					cmzn_font_typeface typeface = cmzn_font_get_typeface(font);
					char *render_type_string = cmzn_font_render_type_enum_to_string(render_type);
					char *typeface_string = cmzn_font_typeface_enum_to_string(typeface);
					int number_of_valid_strings_render_type = 0;
					int number_of_valid_strings_typeface = 0;
					const char **valid_render_type_strings = ENUMERATOR_GET_VALID_STRINGS(cmzn_font_render_type)(
						&number_of_valid_strings_render_type,
						(ENUMERATOR_CONDITIONAL_FUNCTION(cmzn_font_render_type) *)NULL,
						(void *)NULL);
					std::string all_render_types = " ";
					for (int i = 0; i < number_of_valid_strings_render_type; i++)
					{
						if (i)
							all_render_types += "|";

						all_render_types += valid_render_type_strings[i];
					}
					const char *all_render_types_help = all_render_types.c_str();
					const char **valid_font_typeface_strings = ENUMERATOR_GET_VALID_STRINGS(cmzn_font_typeface)(
						&number_of_valid_strings_typeface,
						(ENUMERATOR_CONDITIONAL_FUNCTION(cmzn_font_typeface) *)NULL,
						(void *)NULL);
					std::string all_font_typefaces = " ";
					for (int i = 0; i < number_of_valid_strings_typeface; i++)
					{
						if (i)
							all_font_typefaces += "|";

						all_font_typefaces += valid_font_typeface_strings[i];
					}
					const char *all_font_typefaces_help = all_font_typefaces.c_str();

					struct Option_table *option_table = CREATE(Option_table)();
					int bold_flag = 0;
					int italic_flag = 0;
					float depth = (float)cmzn_font_get_depth(font);
					int size = cmzn_font_get_size(font);
					/* bold */
					Option_table_add_entry(option_table, "bold",
						(void *)&bold_flag, NULL, set_char_flag);
					Option_table_add_entry(option_table, "italic",
						(void *)&italic_flag, NULL, set_char_flag);
					Option_table_add_entry(option_table,"depth",
						&(depth),NULL,set_float);
					Option_table_add_entry(option_table,"size",
						&(size),NULL,set_int_non_negative);
					Option_table_add_string_entry(option_table, "render_type",
						&render_type_string, all_render_types_help);
					Option_table_add_string_entry(option_table, "typeface",
						&typeface_string, all_font_typefaces_help);
					return_code = Option_table_multi_parse(option_table, state);
					if (return_code)
					{
						if (render_type_string)
						{
							STRING_TO_ENUMERATOR(cmzn_font_render_type)(render_type_string,
								&render_type);
							if (CMZN_FONT_RENDER_TYPE_INVALID == render_type)
							{
								display_message(ERROR_MESSAGE,
									"gfx_define_font:  Invalid font type %s", render_type_string);
								return_code = 0;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"gfx_define_font:  Missing render_type argument");
							return_code = 0;
						}
						if (typeface_string)
						{
							STRING_TO_ENUMERATOR(cmzn_font_typeface)(typeface_string,
								&typeface);
							if (CMZN_FONT_TYPEFACE_INVALID == typeface)
							{
								display_message(ERROR_MESSAGE,
									"gfx_define_font:  Invalid true type %s", typeface_string);
								return_code = 0;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"gfx_define_font:  Missing typeface argument");
							return_code = 0;
						}
						if (font)
						{
							cmzn_font_set_bold(font, 0 != bold_flag);
							cmzn_font_set_italic(font, 0 != italic_flag);
							cmzn_font_set_depth(font, depth);
							cmzn_font_set_size(font, size);
							cmzn_font_set_typeface(font, typeface);
							cmzn_font_set_render_type(font, render_type);
						}

					}
					DEALLOCATE(valid_render_type_strings);
					DEALLOCATE(valid_font_typeface_strings);
					cmzn_font_destroy(&font);
					DEALLOCATE(render_type_string);
					DEALLOCATE(typeface_string);
					DESTROY(Option_table)(&option_table);
				}
				else
				{
					display_message(WARNING_MESSAGE,"Missing font string.");
					display_parse_state_location(state);
					return_code=0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE,
					" FONT_NAME FONT_STRING\n");
				return_code = 0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Missing font name");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE, "gfx_define_font.  Invalid argument(s)");
		return_code = 0;
	}

	return (return_code);
} /* gfx_define_font */

