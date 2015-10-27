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
#include "zinc/region.h"
#include "graphics/scene.h"
#include "graphics/glyph.hpp"
#include "graphics/glyph_app.h"

int set_Glyph(struct Parse_state *state, void *glyphAddress_void,
	void *glyphmodule_void)
{
	int return_code;
	cmzn_glyph **glyphAddress = reinterpret_cast<cmzn_glyph **>(glyphAddress_void);
	cmzn_glyphmodule *glyphmodule = reinterpret_cast<cmzn_glyphmodule *>(glyphmodule_void);
	if (state && glyphAddress && glyphmodule)
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
					cmzn_glyph *glyph = cmzn_glyphmodule_find_glyph_by_name(glyphmodule, current_token);
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

int gfx_define_glyph(struct Parse_state *state,
	void *root_region_void, void *glyphmodule_void)
/*******************************************************************************
LAST MODIFIED : 12 March 2008

DESCRIPTION :
Executes a GFX DEFINE glyphcommand.
==============================================================================*/
{
	const char *current_token, *glyph_name;
	int return_code = 1;
	cmzn_glyphmodule_id glyphmodule = 0;
	char *scene_path_name = 0;

	if (state && (glyphmodule = (cmzn_glyphmodule_id)glyphmodule_void))
	{
		if (NULL != (current_token = state->current_token))
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				glyph_name = current_token;
				if (shift_Parse_state(state,1)&&
					(current_token=state->current_token))
				{
					cmzn_glyph_id glyph = cmzn_glyphmodule_find_glyph_by_name(
						glyphmodule, glyph_name);
					char *scene_path_name = 0;
					const char *scene_name = 0;
					const char *graphics_name = NULL;
					struct cmzn_graphics* graphics = NULL;
					struct Option_table *option_table = CREATE(Option_table)();
					struct cmzn_region *input_region = 0, *root_region = (cmzn_region *)root_region_void;
					/* bold */
					Option_table_add_string_entry(option_table, "scene",
						&scene_path_name, " SCENE_NAME[/REGION_PATH][.GRAPHIC_NAME]{default}");

					return_code = Option_table_multi_parse(option_table, state);
					if (return_code)
					{
						if (scene_path_name)
						{
							export_object_name_parser(scene_path_name, &scene_name,
								&graphics_name);
						}
						if (scene_name && (0!=strcmp(scene_name,"default")))
						{
							input_region = cmzn_region_find_subregion_at_path(root_region,
								scene_name);
						}
						else
						{
							input_region = cmzn_region_access(root_region);
						}
						if (input_region && graphics_name)
						{
							cmzn_scene_id scene = cmzn_region_get_scene(input_region);
							graphics = cmzn_scene_find_graphics_by_name(scene, graphics_name);
							cmzn_scene_destroy(&scene);
						}
						if (!graphics)
						{
							display_message(ERROR_MESSAGE,
								"gfx_define_glyph.  Invalid input_scene");
							return_code = 0;
						}
					}
					if (return_code)
					{
						cmzn_glyphmodule_begin_change(glyphmodule);
						if (glyph)
						{
							struct GT_object *graphics_object = cmzn_graphics_copy_graphics_object(
								graphics);
							if (!cmzn_glyph_set_graphics_object(glyph, graphics_object))
								display_message(ERROR_MESSAGE, "gfx_define_glyph.  Unable to change glyph");
							DEACCESS(GT_object)(&graphics_object);
						}
						else
						{
							glyph = cmzn_glyphmodule_create_static_glyph_from_graphics(glyphmodule,
								graphics);
							cmzn_glyph_set_name(glyph, glyph_name);
							cmzn_glyph_set_managed(glyph, true);
						}
						cmzn_glyphmodule_end_change(glyphmodule);
					}
					if (graphics)
						cmzn_graphics_destroy(&graphics);
					if (input_region)
						cmzn_region_destroy(&input_region);
					if (scene_name)
						DEALLOCATE(scene_name);
					if (graphics_name)
						DEALLOCATE(graphics_name);
					cmzn_glyph_destroy(&glyph);
					if (scene_path_name)
						DEALLOCATE(scene_path_name);
					DESTROY(Option_table)(&option_table);
				}
				else
				{
					display_message(WARNING_MESSAGE,"Missing glyph string.");
					display_parse_state_location(state);
					return_code=0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE,
					" GLYPH_NAME GRAPHICS_NAME\n");
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
