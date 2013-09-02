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
	void *dummy_to_be_modified,void *font_module_void)
/*******************************************************************************
LAST MODIFIED : 12 March 2008

DESCRIPTION :
Executes a GFX DEFINE FONT command.
==============================================================================*/
{
	const char *current_token, *font_name;
	int return_code;
	cmzn_font_module_id font_module = 0;

	if (state && (font_module = (cmzn_font_module_id)font_module_void))
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
					cmzn_font_id font = cmzn_font_module_find_font_by_name(
						font_module, font_name);
					if (!font)
					{
						font = cmzn_font_module_create_font(font_module);
						cmzn_font_set_name(font, font_name);
					}
					cmzn_font_render_type render_type = cmzn_font_get_render_type(font);
					cmzn_font_type font_type = cmzn_font_get_font_type(font);
					char *render_type_string = cmzn_font_render_type_enum_to_string(render_type);
					char *font_type_string = cmzn_font_type_enum_to_string(font_type);
					int number_of_valid_strings_render_type = 0;
					int number_of_valid_strings_font_type = 0;
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
					const char **valid_font_font_type_strings = ENUMERATOR_GET_VALID_STRINGS(cmzn_font_type)(
						&number_of_valid_strings_font_type,
						(ENUMERATOR_CONDITIONAL_FUNCTION(cmzn_font_type) *)NULL,
						(void *)NULL);
					std::string all_font_font_types = " ";
					for (int i = 0; i < number_of_valid_strings_font_type; i++)
					{
						if (i)
							all_font_font_types += "|";

						all_font_font_types += valid_font_font_type_strings[i];
					}
					const char *all_font_font_types_help = all_font_font_types.c_str();

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
					Option_table_add_string_entry(option_table, "font_type",
						&font_type_string, all_font_font_types_help);
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
						if (font_type_string)
						{
							STRING_TO_ENUMERATOR(cmzn_font_type)(font_type_string,
								&font_type);
							if (CMZN_FONT_TYPE_INVALID == font_type)
							{
								display_message(ERROR_MESSAGE,
									"gfx_define_font:  Invalid true type %s", font_type_string);
								return_code = 0;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"gfx_define_font:  Missing font_type argument");
							return_code = 0;
						}
						if (font)
						{
							cmzn_font_set_bold(font, bold_flag);
							cmzn_font_set_italic(font, italic_flag);
							cmzn_font_set_depth(font, depth);
							cmzn_font_set_size(font, size);
							cmzn_font_set_font_type(font, font_type);
							cmzn_font_set_render_type(font, render_type);
						}

					}
					DEALLOCATE(valid_render_type_strings);
					DEALLOCATE(valid_font_font_type_strings);
					cmzn_font_destroy(&font);
					DEALLOCATE(render_type_string);
					DEALLOCATE(font_type_string);
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

