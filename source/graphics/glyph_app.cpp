/**
 * FILE : glyph_app.h
 *
 */
 /* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is cmgui.
 *
 * The Initial Developer of the Original Code is
 * Auckland Uniservices Ltd, Auckland, New Zealand.
 * Portions created by the Initial Developer are Copyright (C) 2013
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

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

