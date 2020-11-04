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
#include "opencmiss/zinc/region.h"
#include "graphics/scene.hpp"
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

/**
 * Executes a GFX DEFINE GLYPH command.
 */
int gfx_define_glyph(struct Parse_state *state,
	void *glyph_name_void, void *define_glyph_data_void)
{
	int return_code = 1;
	Define_glyph_data *define_glyph_data = static_cast<Define_glyph_data *>(define_glyph_data_void);
	if ((state) && (define_glyph_data))
	{
		if (state->current_token)
		{
			const char *glyph_name = static_cast<char *>(glyph_name_void);
			bool process = false;
			if (glyph_name)
			{
				process = true;
			}
			else
			{
				if (Parse_state_help_mode(state))
				{
					glyph_name = "GLYPH_NAME";
					Option_table *option_table = CREATE(Option_table)();
					Option_table_add_entry(option_table, "GLYPH_NAME",
						(void *)glyph_name, define_glyph_data_void, gfx_define_glyph);
					return_code = Option_table_parse(option_table, state);
					DESTROY(Option_table)(&option_table);
				}
				else
				{
					glyph_name = state->current_token;
					shift_Parse_state(state, 1);
					process = true;
				}
			}
			if (process)
			{
				char *scene_path_name = 0;
				struct Option_table *option_table = CREATE(Option_table)();
				Option_table_add_help(option_table,
					"The 'scene' option defines a static glyph from the named graphics "
					"at the region/scene path.");
				/* scene (graphics) */
				Option_table_add_string_entry(option_table, "scene",
					&scene_path_name, " [/REGION_PATH.]GRAPHICS_NAME");

				return_code = Option_table_multi_parse(option_table, state);
				if (return_code)
				{
					cmzn_graphics* graphics = 0;
					const char *scene_name = 0;
					const char *graphics_name = 0;
					if (scene_path_name)
						export_object_name_parser(scene_path_name, &scene_name, &graphics_name);
					cmzn_region *input_region =
						cmzn_region_find_subregion_at_path(define_glyph_data->root_region, scene_name);
					if (input_region && graphics_name)
					{
						cmzn_scene_id scene = cmzn_region_get_scene(input_region);
						graphics = cmzn_scene_find_graphics_by_name(scene, graphics_name);
						cmzn_scene_destroy(&scene);
					}
					if (!graphics)
					{
						display_message(ERROR_MESSAGE,
							"gfx_define_glyph.  Invalid scene region path or graphics name");
						return_code = 0;
					}
					else
					{
						cmzn_glyphmodule_begin_change(define_glyph_data->glyphmodule);
						cmzn_glyph_id glyph = cmzn_glyphmodule_find_glyph_by_name(define_glyph_data->glyphmodule, glyph_name);
						if (glyph)
						{
							struct GT_object *graphics_object = cmzn_graphics_copy_graphics_object(
								graphics);
							if (!cmzn_glyph_set_graphics_object(glyph, graphics_object))
							{
								display_message(ERROR_MESSAGE, "gfx_define_glyph.  Unable to change glyph");
								return_code = 0;
							}
							DEACCESS(GT_object)(&graphics_object);
						}
						else
						{
							glyph = cmzn_glyphmodule_create_static_glyph_from_graphics(define_glyph_data->glyphmodule,
								graphics);
							if ((CMZN_OK != cmzn_glyph_set_name(glyph, glyph_name)) ||
								(CMZN_OK != cmzn_glyph_set_managed(glyph, true)))
							{
								display_message(ERROR_MESSAGE, "gfx_define_glyph.  Failed");
								return_code = 0;
							}
						}
						cmzn_glyph_destroy(&glyph);
						cmzn_glyphmodule_end_change(define_glyph_data->glyphmodule);
					}
					cmzn_graphics_destroy(&graphics);
					cmzn_region_destroy(&input_region);
					if (scene_name)
						DEALLOCATE(scene_name);
					if (graphics_name)
						DEALLOCATE(graphics_name);
				}
				if (scene_path_name)
					DEALLOCATE(scene_path_name);
				DESTROY(Option_table)(&option_table);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE, "Missing glyph name");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE, "gfx_define_glyph.  Invalid argument(s)");
		return_code = 0;
	}
	return (return_code);
}
