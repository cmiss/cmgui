/**
 * FILE : glyph_app.h
 *
 */
 /* OpenCMISS-Cmgui Application
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#if 1
#include "configure/cmgui_configure.h"
#endif /* defined (BUILD_WITH_CMAKE) */

#include "command/parser.h"
#include "general/message.h"
#include "graphics/glyph.hpp"
#include "graphics/glyph_app.h"

int set_Glyph(struct Parse_state *state, void *glyphAddress_void,
	void *glyphModule_void)
{
	int return_code;
	cmzn_glyph **glyphAddress = reinterpret_cast<cmzn_glyph **>(glyphAddress_void);
	cmzn_glyph_module *glyphModule = reinterpret_cast<cmzn_glyph_module *>(glyphModule_void);
	if (state && glyphAddress && glyphModule)
	{
		const char *current_token = state->current_token;
		if (current_token)
		{
			if (!Parse_state_help_mode(state))
			{
				if (fuzzy_string_compare(current_token, "NONE"))
				{
					if (*glyphAddress)
					{
						cmzn_glyph_destroy(glyphAddress);
					}
					return_code=1;
				}
				else
				{
					cmzn_glyph *glyph = cmzn_glyph_module_find_glyph_by_name(glyphModule, current_token);
					if (glyph)
					{
						if (*glyphAddress)
						{
							cmzn_glyph_destroy(glyphAddress);
						}
						*glyphAddress = glyph;
						return_code = 1;
					}
					else
					{
						display_message(ERROR_MESSAGE, "Unknown glyph : %s", current_token);
						return_code = 0;
					}
				}
				shift_Parse_state(state, 1);
			}
			else
			{
				display_message(INFORMATION_MESSAGE," GLYPH_NAME|none");
				/* if possible, then write the name */
				cmzn_glyph *glyph = *glyphAddress;
				if (glyph)
				{
					display_message(INFORMATION_MESSAGE, "[%s]", glyph->getName());
				}
				else
				{
					display_message(INFORMATION_MESSAGE, "[none]");
				}
				return_code = 1;
			}
		}
		else
		{
			display_message(WARNING_MESSAGE, "Missing glyph name");
			display_parse_state_location(state);
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE, "set_Glyph.  Invalid argument(s)");
		return_code = 0;
	}
	return (return_code);
}

