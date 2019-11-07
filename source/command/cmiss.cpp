/***************************************************************************//**
 * cmiss.cpp
 *
 * Functions for executing cmiss commands.
 */
/* OpenCMISS-Cmgui Application
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#if 1
#include "configure/cmgui_configure.h"
#endif /* defined (BUILD_WITH_CMAKE) */

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#if defined (WIN32_SYSTEM)
#  include <direct.h>
#else /* !defined (WIN32_SYSTEM) */
#  include <unistd.h>
#endif /* !defined (WIN32_SYSTEM) */
#include <math.h>
#include <time.h>
#include "opencmiss/zinc/context.h"
#include "opencmiss/zinc/element.h"
#include "opencmiss/zinc/elementbasis.h"
#include "opencmiss/zinc/elementtemplate.h"
#include "opencmiss/zinc/field.h"
#include "opencmiss/zinc/fieldfiniteelement.h"
#include "opencmiss/zinc/fieldmatrixoperators.h"
#include "opencmiss/zinc/fieldmodule.h"
#include "opencmiss/zinc/fieldsmoothing.h"
#include "opencmiss/zinc/fieldsubobjectgroup.h"
#include "opencmiss/zinc/glyph.h"
#include "opencmiss/zinc/light.h"
#include "opencmiss/zinc/material.h"
#include "opencmiss/zinc/node.h"
#include "opencmiss/zinc/nodeset.h"
#include "opencmiss/zinc/nodetemplate.h"
#include "opencmiss/zinc/region.h"
#include "opencmiss/zinc/scene.h"
#include "opencmiss/zinc/sceneviewer.h"
#include "opencmiss/zinc/result.h"
#include "opencmiss/zinc/stream.h"
#include "opencmiss/zinc/streamregion.h"
#include "comfile/comfile.h"
#if defined (WX_USER_INTERFACE)
#include "comfile/comfile_window_wx.h"
#endif /* defined (WX_USER_INTERFACE) */
#include "command/console.h"
#include "command/command_window.h"
#include "command/example_path.h"
#include "command/parser.h"
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_alias.h"
#include "computed_field/computed_field_arithmetic_operators.h"
#include "computed_field/computed_field_compose.h"
#include "computed_field/computed_field_composite.h"
#include "computed_field/computed_field_conditional.h"
#include "computed_field/computed_field_coordinate.h"
#include "computed_field/computed_field_deformation.h"
#include "computed_field/computed_field_derivatives.h"
#include "computed_field/computed_field_find_xi.h"
#include "computed_field/computed_field_find_xi_graphics.h"
#include "computed_field/computed_field_finite_element.h"
#include "computed_field/computed_field_function.h"
#include "computed_field/computed_field_group.hpp"
#include "computed_field/computed_field_image.h"
#include "computed_field/computed_field_integration.h"
#include "computed_field/computed_field_logical_operators.h"
#include "computed_field/computed_field_lookup.h"
#include "computed_field/computed_field_matrix_operators.hpp"
#include "computed_field/computed_field_mesh_operators.hpp"
#include "computed_field/computed_field_nodeset_operators.hpp"
#include "computed_field/computed_field_subobject_group.hpp"
#include "computed_field/computed_field_vector_operators.hpp"
#include "computed_field/computed_field_set.h"
#include "computed_field/computed_field_string_constant.h"
#include "computed_field/computed_field_time.h"
#include "computed_field/computed_field_trigonometry.h"
#include "computed_field/computed_field_update.h"
#include "computed_field/computed_field_wrappers.h"
#include "context/context.h"
#include "element/element_operations.h"
#include "element/element_point_tool.h"
#if defined (WX_USER_INTERFACE)
#include "element/element_point_viewer_wx.h"
#endif /* defined (WX_USER_INTERFACE) */
#include "element/element_tool.h"
#include "finite_element/export_cm_files.h"
#if defined (ZINC_USE_NETGEN)
#include "finite_element/generate_mesh_netgen.h"
#endif /* defined (ZINC_USE_NETGEN) */
#include "finite_element/export_finite_element.h"
#include "finite_element/finite_element.h"
#include "finite_element/finite_element_conversion.h"
#include "finite_element/finite_element_mesh.hpp"
#include "finite_element/finite_element_region.h"
#include "finite_element/finite_element_to_graphics_object.h"
#include "finite_element/finite_element_to_iges.h"
#include "finite_element/finite_element_to_iso_lines.h"
#include "finite_element/finite_element_to_streamlines.h"
#include "finite_element/import_finite_element.h"
#include "finite_element/snake.h"
#include "general/debug.h"
#include "general/error_handler.h"
#include "general/image_utilities.h"
#include "general/io_stream.h"
#include "general/matrix_vector.h"
#include "general/multi_range.h"
#include "general/mystring.h"
#include "graphics/environment_map.h"
#include "graphics/graphics_object.h"
#include "graphics/graphics_window.h"
#include "graphics/iso_field_calculation.h"
#include "graphics/light.hpp"
#include "graphics/material.h"
#include "graphics/glyph.hpp"
#include "graphics/glyph_colour_bar.hpp"
#include "graphics/graphics.h"
#include "graphics/graphics_module.h"
#include "graphics/render_to_finite_elements.h"
#include "graphics/render_stl.h"
#include "graphics/render_vrml.h"
#include "graphics/render_wavefront.h"
#include "graphics/scene.h"
#include "graphics/triangle_mesh.hpp"
#include "graphics/render_triangularisation.hpp"
#include "graphics/import_graphics_object.h"
#include "graphics/scene.hpp"
#include "graphics/scenefilter.hpp"
#include "graphics/tessellation.hpp"
#if defined (WX_USER_INTERFACE)
#include "graphics/region_tree_viewer_wx.h"
#endif /* switch(USER_INTERFACE)*/
#include "graphics/spectrum.h"
#if defined (WX_USER_INTERFACE)
#include "graphics/spectrum_editor_wx.h"
#include "graphics/spectrum_editor_dialog_wx.h"
#endif /* defined (WX_USER_INTERFACE) */
#include "graphics/spectrum_component.h"
#include "graphics/texture.h"
#include "graphics/transform_tool.h"
#include "graphics/volume_texture.h"
#if defined (GTK_USER_INTERFACE)
#include "gtk/gtk_cmiss_scene_viewer.h"
#endif /* defined (GTK_USER_INTERFACE) */
#include "image_processing/computed_field_image_resample.h"
#if defined (ZINC_USE_ITK)
#include "image_processing/computed_field_threshold_image_filter.h"
#include "image_processing/computed_field_binary_threshold_image_filter.h"
#include "image_processing/computed_field_canny_edge_detection_filter.h"
#include "image_processing/computed_field_mean_image_filter.h"
#include "image_processing/computed_field_sigmoid_image_filter.h"
#include "image_processing/computed_field_discrete_gaussian_image_filter.h"
#include "image_processing/computed_field_curvature_anisotropic_diffusion_image_filter.h"
#include "image_processing/computed_field_derivative_image_filter.h"
#include "image_processing/computed_field_rescale_intensity_image_filter.h"
#include "image_processing/computed_field_connected_threshold_image_filter.h"
#include "image_processing/computed_field_gradient_magnitude_recursive_gaussian_image_filter.h"
#include "image_processing/computed_field_histogram_image_filter.h"
#include "image_processing/computed_field_fast_marching_image_filter.h"
#include "image_processing/computed_field_binary_dilate_image_filter.h"
#include "image_processing/computed_field_binary_erode_image_filter.h"
#endif /* defined (ZINC_USE_ITK) */
#if defined (SELECT_DESCRIPTORS)
#include "io_devices/io_device.h"
#endif /* !defined (SELECT_DESCRIPTORS) */
#if defined (WX_USER_INTERFACE)
#include "material/material_editor_wx.h"
#endif /* defined (SWITCH_USER_INTERFACE) */
#include "minimise/minimise.h"
#include "node/node_operations.h"
#include "node/node_tool.h"
#if defined (WX_USER_INTERFACE)
#include "node/node_viewer_wx.h"
#endif /* defined (WX_USER_INTERFACE) */
#include "region/cmiss_region.h"
#include "region/cmiss_region_app.h"
#include "three_d_drawing/graphics_buffer.h"
#include "graphics/font.h"
#include "time/time_keeper_app.hpp"
#include "user_interface/filedir.h"
#include "user_interface/confirmation.h"
#include "general/message.h"
#include "user_interface/user_interface.h"
#if defined (USE_PERL_INTERPRETER)
#include "perl_interpreter.h"
#endif /* defined (USE_PERL_INTERPRETER) */
#include "user_interface/fd_io.h"
#include "user_interface/idle.h"
#include "command/cmiss.h"
#include "mesh/cmiss_element_private.hpp"
#include "mesh/cmiss_node_private.hpp"
#if defined (USE_OPENCASCADE)
#include "cad/graphicimporter.h"
#include "cad/point.h"
#include "cad/curve.h"
#include "cad/surface.h"
#include "cad/geometricshape.h"
#include "graphics/graphics_object.hpp"
#include "cad/opencascadeimporter.h"
#include "cad/cad_tool.h"
#include "cad/computed_field_cad_topology.h"
#endif /* defined (USE_OPENCASCADE) */

// insert app headers here
#include "computed_field/computed_field_image_app.h"
#include "computed_field/computed_field_integration_app.h"
#include "computed_field/computed_field_alias_app.h"
#include "computed_field/computed_field_coordinate_app.h"
#include "image_processing/computed_field_sigmoid_image_filter_app.h"
#include "image_processing/computed_field_mean_image_filter_app.h"
#include "image_processing/computed_field_rescale_intensity_image_filter_app.h"
#include "image_processing/computed_field_derivative_image_filter_app.h"
#include "image_processing/computed_field_canny_edge_detection_filter_app.h"
#include "image_processing/computed_field_curvature_anisotropic_diffusion_image_filter_app.h"
#include "image_processing/computed_field_histogram_image_filter_app.h"
#include "image_processing/computed_field_discrete_gaussian_image_filter_app.h"
#include "image_processing/computed_field_connected_threshold_image_filter_app.h"
#include "image_processing/computed_field_gradient_magnitude_recursive_gaussian_image_filter_app.h"
#include "image_processing/computed_field_fast_marching_image_filter_app.h"
#include "image_processing/computed_field_binary_erode_image_filter_app.h"
#include "image_processing/computed_field_binary_dilate_image_filter_app.h"
#include "computed_field/computed_field_time_app.h"
#include "image_processing/computed_field_binary_threshold_image_filter_app.h"
#include "image_processing/computed_field_threshold_image_filter_app.h"
#include "image_processing/computed_field_image_resample_app.h"
#include "computed_field/computed_field_string_constant_app.h"
#include "computed_field/computed_field_deformation_app.h"
#include "computed_field/computed_field_finite_element_app.h"
#include "computed_field/computed_field_format_output_app.h"
#include "computed_field/computed_field_vector_operators_app.hpp"
#include "computed_field/computed_field_matrix_operators_app.hpp"
#include "computed_field/computed_field_mesh_operators_app.hpp"
#include "computed_field/computed_field_nodeset_operators_app.hpp"
#include "computed_field/computed_field_lookup_app.h"
#include "computed_field/computed_field_logical_operators_app.h"
#include "computed_field/computed_field_function_app.h"
#include "computed_field/computed_field_fibres_app.h"
#include "computed_field/computed_field_derivatives_app.h"
#include "computed_field/computed_field_conditional_app.h"
#include "computed_field/computed_field_composite_app.h"
#include "computed_field/computed_field_compose_app.h"
#include "computed_field/computed_field_format_output_app.h"
#include "computed_field/computed_field_trigonometry_app.h"
#include "computed_field/computed_field_arithmetic_operators_app.h"
#include "computed_field/computed_field_scene_viewer_projection_app.h"
#include "minimise/minimise_app.h"
#include "finite_element/export_finite_element_app.h"
#include "graphics/element_point_ranges_app.h"
#include "graphics/environment_map_app.h"
#include "finite_element/finite_element_region_app.h"
#include "graphics/scene_viewer_app.h"
#include "graphics/font_app.h"
#include "graphics/glyph_app.h"
#include "graphics/scenefilter_app.hpp"
#include "graphics/tessellation_app.hpp"
#include "graphics/tessellation_app.hpp"
#include "computed_field/computed_field_app.h"
#include "general/enumerator_app.h"
#include "graphics/render_to_finite_elements_app.h"
#include "graphics/auxiliary_graphics_types_app.h"
#include "finite_element/finite_element_conversion_app.h"
#include "graphics/texture_app.h"
#include "graphics/colour_app.h"
#include "graphics/scene_app.h"
#include "graphics/spectrum_component_app.h"
#include "graphics/light_app.h"
#include "graphics/material_app.h"
#include "graphics/spectrum_app.h"
#include "general/multi_range_app.h"
#include "computed_field/computed_field_set_app.h"
#include "context/context_app.h"
#include "three_d_drawing/graphics_buffer_app.h"

#include "image_io/analyze.h"
#include "image_io/analyze_object_map.hpp"
/*
Module types
------------
*/


struct cmzn_command_data
/*******************************************************************************
LAST MODIFIED : 12 August 2002

DESCRIPTION :
==============================================================================*/
{
	int access_count;
	char *cm_examples_directory,*cm_parameters_file_name,*example_directory,
		*examples_directory,*example_comfile,
		*example_requirements,*help_directory,*help_url;
	bool start_event_dispatcher;
	struct Console *command_console;
#if defined (USE_CMGUI_COMMAND_WINDOW)
	struct Command_window *command_window;
#endif /* USE_CMGUI_COMMAND_WINDOW */
	struct Execute_command *execute_command,*set_command;
	struct Element_point_tool *element_point_tool;
	struct Element_tool *element_tool;
#if defined (USE_OPENCASCADE)
	struct Cad_tool *cad_tool;
#endif /* defined (USE_OPENCASCADE) */
	struct Event_dispatcher *event_dispatcher;
	struct Node_tool *data_tool,*node_tool;
	struct Interactive_tool *transform_tool;
#if defined (USE_PERL_INTERPRETER)
	struct Interpreter *interpreter;
#endif /* defined (USE_PERL_INTERPRETER) */
#if defined (SELECT_DESCRIPTORS)
	struct LIST(Io_device) *device_list;
#endif /* defined (SELECT_DESCRIPTORS) */
	/* list of glyphs = simple graphics objects with only geometry */
	cmzn_glyphmodule_id glyphmodule;
#if defined (WX_USER_INTERFACE)
	struct MANAGER(Comfile_window) *comfile_window_manager;
#endif /* defined (WX_USER_INTERFACE)*/
	struct cmzn_region *root_region;
	struct Computed_field_package *computed_field_package;
	struct MANAGER(Environment_map) *environment_map_manager;
	struct MANAGER(FE_basis) *basis_manager;
	struct LIST(FE_element_shape) *element_shape_list;
	/* Always want the entry for graphics_buffer_package even if it will
		not be available on this implementation */
	struct Graphics_buffer_app_package *graphics_buffer_package;
	struct cmzn_sceneviewermodule_app *sceneviewermodule;
#if defined (USE_CMGUI_GRAPHICS_WINDOW)
	struct MANAGER(Graphics_window) *graphics_window_manager;
#endif /* defined (USE_CMGUI_GRAPHICS_WINDOW) */
	struct MANAGER(Interactive_tool) *interactive_tool_manager;
	struct IO_stream_package *io_stream_package;
	cmzn_lightmodule *lightmodule;
	struct cmzn_light *default_light;
	struct cmzn_materialmodule *materialmodule;
	struct cmzn_scenefiltermodule *filter_module;
	struct cmzn_font *default_font;
	struct cmzn_tessellationmodule *tessellationmodule;
	struct MANAGER(Scene) *scene_manager;
	struct Scene *default_scene;
	struct MANAGER(cmzn_spectrum) *spectrum_manager;
	struct MANAGER(VT_volume_texture) *volume_texture_manager;
	/* global list of selected objects */
	struct Element_point_ranges_selection *element_point_ranges_selection;
	struct cmzn_spectrum *default_spectrum;
	struct Streampoint *streampoint_list;
	struct Time_keeper_app *default_time_keeper_app;
	struct User_interface *user_interface;
#if defined (WX_USER_INTERFACE)
	struct Node_viewer *data_viewer,*node_viewer;
	struct Element_point_viewer *element_point_viewer;
#endif /* defined (WX_USER_INTERFACE) */
#if defined (WX_USER_INTERFACE)
	struct Material_editor *material_editor;
	struct Region_tree_viewer *region_tree_viewer;
	struct Spectrum_editor_dialog *spectrum_editor_dialog;
#endif /* defined (WX_USER_INTERFACE) */
	struct cmzn_graphics_module *graphics_module;
	cmzn_logger_id logger;
	cmzn_loggernotifier_id loggerNotifier;
}; /* struct cmzn_command_data */

typedef struct
/*******************************************************************************
LAST MODIFIED : 12 December 1996+

DESCRIPTION :
==============================================================================*/
{
	char *examples_directory,*help_directory,*help_url,*startup_comfile;
} User_settings;

/*
Module functions
----------------
*/

#if defined (WX_USER_INTERFACE)
static int Graphics_window_update_Interactive_tool(struct Graphics_window *graphics_window,
	void *interactive_tool_void)
/*******************************************************************************
LAST MODIFIED : 27 March 2007

DESCRIPTION :
WX_USER_INTERFACE_ONLY, get the interactive_tool_manager and pass it
to change the interactive tool settings.
==============================================================================*/
{
	char *tool_name;
	struct MANAGER(Interactive_tool) *interactive_tool_manager;
	struct Interactive_tool *global_interactive_tool;
	struct Interactive_tool *wx_interactive_tool;
	global_interactive_tool = (struct Interactive_tool *)interactive_tool_void;
	GET_NAME(Interactive_tool)(global_interactive_tool,&tool_name);
	interactive_tool_manager = Graphics_window_get_interactive_tool_manager(graphics_window);
	if (NULL != (wx_interactive_tool=
		FIND_BY_IDENTIFIER_IN_MANAGER(Interactive_tool,name)(
		(char *)tool_name,interactive_tool_manager)))
	{
		Interactive_tool_copy(wx_interactive_tool,
			global_interactive_tool, (struct MANAGER(Interactive_tool) *)NULL);
	}
	DEALLOCATE(tool_name);
	return 1;
}
#endif /*(WX_USER_INTERFACE)*/

#if defined (WX_USER_INTERFACE) && defined (WIN32_SYSTEM)
/**
 * Set directory for windows drive letter and path separately.
 *
 * @param filename_address  Pointer to filename from which drive letter
 * and path are extracted, and replaced with filename only.
 */
void CMZN_set_directory_and_filename_WIN32(char **filename_address,
	struct cmzn_command_data *command_data)
{
	if ((!filename_address) || (!(*filename_address)) || (!command_data))
	{
		return;
	}
	char *filename_copy = duplicate_string(*filename_address);
	const int total_length = strlen(filename_copy);
	char *path = filename_copy;
	if ((total_length >= 2) && isalpha(path[0]) && (path[1] == ':'))
	{
		char command_string[] = "set dir ?:";
		command_string[8] = path[0];
		Execute_command_execute_string(command_data->execute_command, command_string);
		path += 2;
	}
	char *filename_only = path;
	char *last_backslash = strrchr(path, '\\');
	if (last_backslash)
	{
		filename_only = last_backslash + 1;
		*last_backslash = '\0';
		char *command_string = new char[total_length + 10];
		strcpy(command_string, "set dir ");
		strcat(command_string, path);
		Execute_command_execute_string(command_data->execute_command, command_string);
		delete[] command_string;
	}
	strcpy(*filename_address, filename_only);
	DEALLOCATE(filename_copy);
	return;
}

#endif /*(WX_USER_INTERFACE)*/

static int set_command_prompt(const char *prompt, struct cmzn_command_data *command_data)
/*******************************************************************************
LAST MODIFIED : 26 June 2002

DESCRIPTION :
Changes the command prompt provided to the user.
==============================================================================*/
{
	int return_code = 0;

	ENTER(set_command_prompt);
	if (prompt && command_data)
	{
#if defined (WIN32_USER_INTERFACE) || defined (GTK_USER_INTERFACE) || defined (WX_USER_INTERFACE)
		if (command_data->command_window)
		{
			return_code = Command_window_set_command_prompt(command_data->command_window,
				prompt);
		}
#endif /* defined (WIN32_USER_INTERFACE) || defined (GTK_USER_INTERFACE) */
		if (command_data->command_console)
		{
			return_code = Console_set_command_prompt(command_data->command_console,
				prompt);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_command_prompt.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* set_command_prompt */

static int gfx_change_identifier(struct Parse_state *state,
	void *dummy_to_be_modified, void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 17 January 2003

DESCRIPTION :
==============================================================================*/
{
	char data_flag, element_flag, face_flag, line_flag, node_flag, *sort_by_field_name;
	FE_value time;
	int data_offset, element_offset, face_offset, line_offset, node_offset,
		return_code;
	struct cmzn_command_data *command_data;
	struct Computed_field *sort_by_field;
	struct FE_region *fe_region;
	struct Option_table *option_table;

	ENTER(gfx_change_identifier);
	USE_PARAMETER(dummy_to_be_modified);
	if (state && (command_data = (struct cmzn_command_data *)command_data_void))
	{
		cmzn_region_id region = cmzn_region_access(command_data->root_region);
		cmzn_field_group_id group = 0;
		data_flag = 0;
		data_offset = 0;
		element_flag = 0;
		element_offset = 0;
		face_flag = 0;
		face_offset = 0;
		line_flag = 0;
		line_offset = 0;
		node_flag = 0;
		node_offset = 0;
		sort_by_field_name = NULL;
		sort_by_field = (struct Computed_field *)NULL;
		if (command_data->default_time_keeper_app)
		{
			time = command_data->default_time_keeper_app->getTimeKeeper()->getTime();
		}
		else
		{
			time = 0;
		}

		option_table = CREATE(Option_table)();
		/* data_offset */
		Option_table_add_entry(option_table, "data_offset", &data_offset,
			&data_flag, set_int_and_char_flag);
		/* element_offset */
		Option_table_add_entry(option_table, "element_offset", &element_offset,
			&element_flag, set_int_and_char_flag);
		/* face_offset */
		Option_table_add_entry(option_table, "face_offset", &face_offset,
			&face_flag, set_int_and_char_flag);
		/* group */
		Option_table_add_region_or_group_entry(option_table, "group", &region, &group);
		/* line_offset */
		Option_table_add_entry(option_table, "line_offset", &line_offset,
			&line_flag, set_int_and_char_flag);
		/* node_offset */
		Option_table_add_entry(option_table, "node_offset", &node_offset,
			&node_flag, set_int_and_char_flag);
		/* sort_by */
		Option_table_add_string_entry(option_table, "sort_by", &sort_by_field_name,
			" FIELD_NAME");
		/* time */
		Option_table_add_entry(option_table, "time", &time, NULL, set_FE_value);

		if (0 != (return_code = Option_table_multi_parse(option_table, state)))
		{
			if (region && (fe_region = cmzn_region_get_FE_region(region)))
			{
				if (sort_by_field_name)
				{
					cmzn_fieldmodule_id field_module = cmzn_region_get_fieldmodule(region);
					sort_by_field = cmzn_fieldmodule_find_field_by_name(field_module,
						sort_by_field_name);
					if (sort_by_field)
					{
						if (!Computed_field_has_numerical_components(sort_by_field, NULL))
						{
							cmzn_field_destroy(&sort_by_field);
							return_code = 0;
							display_message(ERROR_MESSAGE,
								"gfx_change_identifier.  Sort by field does not have numerical components");
						}
					}
					else
					{
						return_code = 0;
						display_message(ERROR_MESSAGE,
							"gfx_change_identifier.  Sort by field cannot be found");
					}
					cmzn_fieldmodule_destroy(&field_module);
				}
				if (return_code)
				{
					int highest_dimension = FE_region_get_highest_dimension(fe_region);
					cmzn_fieldmodule_id fieldmodule = cmzn_region_get_fieldmodule(region);
					cmzn_fieldmodule_begin_change(fieldmodule);
					if (element_flag)
					{
						if (highest_dimension > 0)
						{
							cmzn_mesh_id mesh = cmzn_fieldmodule_find_mesh_by_dimension(
								fieldmodule, highest_dimension);
							cmzn_field_element_group_id element_group =
								cmzn_field_group_get_field_element_group(group, mesh);
							if (CMZN_OK != FE_region_change_element_identifiers(fe_region,
								highest_dimension, element_offset, sort_by_field, time,
								element_group))
							{
								return_code = 0;
							}
							cmzn_field_element_group_destroy(&element_group);
							cmzn_mesh_destroy(&mesh);
						}
						else
						{
							display_message(WARNING_MESSAGE,
								"gfx change identifier:  No elements found in region");
						}
					}
					if (face_flag && (highest_dimension > 2))
					{
						cmzn_mesh_id mesh = cmzn_fieldmodule_find_mesh_by_dimension(
							fieldmodule, 2);
						cmzn_field_element_group_id element_group =
							cmzn_field_group_get_field_element_group(group, mesh);
						if (CMZN_OK != FE_region_change_element_identifiers(fe_region,
							/*dimension*/2,	face_offset, sort_by_field, time,
							element_group))
						{
							return_code = 0;
						}
						cmzn_field_element_group_destroy(&element_group);
						cmzn_mesh_destroy(&mesh);
					}
					if (line_flag && (highest_dimension > 1))
					{
						cmzn_mesh_id mesh = cmzn_fieldmodule_find_mesh_by_dimension(
							fieldmodule, 1);
						cmzn_field_element_group_id element_group =
							cmzn_field_group_get_field_element_group(group, mesh);
						if (CMZN_OK != FE_region_change_element_identifiers(fe_region,
							/*dimension*/1, line_offset, sort_by_field, time,
							element_group))
						{
							return_code = 0;
						}
						cmzn_field_element_group_destroy(&element_group);
						cmzn_mesh_destroy(&mesh);
					}
					if (node_flag)
					{
						cmzn_nodeset_id nodeset = cmzn_fieldmodule_find_nodeset_by_field_domain_type(
							fieldmodule, CMZN_FIELD_DOMAIN_TYPE_NODES);
						if (group)
						{						
							cmzn_field_node_group_id node_group = cmzn_field_group_get_field_node_group(group, nodeset);
							cmzn_nodeset_destroy(&nodeset);
							nodeset = cmzn_nodeset_group_base_cast(cmzn_field_node_group_get_nodeset_group(node_group));
							cmzn_field_node_group_destroy(&node_group);
						}
						if (nodeset)
						{
							if (!cmzn_nodeset_change_node_identifiers(nodeset, node_offset, sort_by_field, time))
								return_code = 0;
							cmzn_nodeset_destroy(&nodeset);
						}
					}
					if (data_flag)
					{
						cmzn_nodeset_id nodeset = cmzn_fieldmodule_find_nodeset_by_field_domain_type(
							fieldmodule, CMZN_FIELD_DOMAIN_TYPE_DATAPOINTS);
						if (group)
						{						
							cmzn_field_node_group_id node_group = cmzn_field_group_get_field_node_group(group, nodeset);
							cmzn_nodeset_destroy(&nodeset);
							nodeset = cmzn_nodeset_group_base_cast(cmzn_field_node_group_get_nodeset_group(node_group));
							cmzn_field_node_group_destroy(&node_group);
						}
						if (nodeset)
						{
							if (!cmzn_nodeset_change_node_identifiers(nodeset, data_offset, sort_by_field, time))
								return_code = 0;
							cmzn_nodeset_destroy(&nodeset);
						}
					}
					cmzn_fieldmodule_end_change(fieldmodule);
					cmzn_fieldmodule_destroy(&fieldmodule);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_change_identifier.  Invalid region");
				return_code = 0;
			}
		}
		DESTROY(Option_table)(&option_table);
		if (sort_by_field)
		{
			DEACCESS(Computed_field)(&sort_by_field);
		}
		if (sort_by_field_name)
		{
			DEALLOCATE(sort_by_field_name);
		}
		cmzn_field_group_destroy(&group);
		cmzn_region_destroy(&region);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_change_identifier.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* gfx_change_identifier */

/***************************************************************************//**
 * Dummy modifier function explaining migration from old gfx create axes
 * command.
 */
static int gfx_create_axes(struct Parse_state *state,
	void *dummy_to_be_modified, void *dummy_user_data_void)
{
	USE_PARAMETER(dummy_to_be_modified);
	USE_PARAMETER(dummy_user_data_void);
	display_message(Parse_state_help_mode(state)? INFORMATION_MESSAGE : ERROR_MESSAGE,
		"The 'gfx create axes' command has been removed. These are now drawn as\n"
		"point graphics using built-in axes glyphs. Create these in the scene editor\n"
		"and list commands to reproduce the view with:\n"
		"  gfx list g_element REGION_PATH commands.\n"
		"Alternatively directly add with commands (e.g. for root region):\n"
		"  gfx modify g_element \"/\" point glyph axes_xyz size 1.0;\n");
	if (Parse_state_help_mode(state))
		return 1;
	return 0;
}

/**
 * Executes a GFX CREATE COLOUR_BAR command. Creates a colour bar glyph
 * with tick marks and labels for showing the scale of a spectrum.
 */
static int gfx_create_colour_bar(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
{
	int return_code;
	struct cmzn_command_data *command_data;
	struct Option_table *option_table;

	ENTER(gfx_create_colour_bar);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (NULL != (command_data = (struct cmzn_command_data *)command_data_void))
		{
			/* initialise defaults */
			char *glyph_name = duplicate_string("colour_bar");
			char *number_format = duplicate_string("%+.4e");
			/* must access it now, because we deaccess it later */
			cmzn_material_id label_material = cmzn_materialmodule_get_default_material(command_data->materialmodule);
			cmzn_material_id material = 0;
			cmzn_spectrum_id spectrum = ACCESS(cmzn_spectrum)(command_data->default_spectrum);
			int number_of_components=3;
			double bar_centre[3] = { -0.9, 0.0, 0.5 };
			double bar_axis[3] = { 0.0, 1.0, 0.0 };
			double side_axis[3] = { 1.0, 0.0, 0.0 };
			double bar_length = 1.6;
			double extend_length = 0.06;
			double bar_radius = 0.06;
			double tick_length = 0.04;
			int tick_divisions = 10;
			char *font_name = (char *)NULL;

			option_table=CREATE(Option_table)();
			/* as */
			Option_table_add_entry(option_table,"as",&glyph_name,
				(void *)1,set_name);
			/* axis */
			Option_table_add_entry(option_table,"axis",bar_axis,
				&number_of_components,set_double_vector);
			/* centre */
			Option_table_add_entry(option_table,"centre",bar_centre,
				&number_of_components,set_double_vector);
			/* divisions */
			Option_table_add_entry(option_table,"divisions",&tick_divisions,
				NULL,set_int_non_negative);
			/* extend_length */
			Option_table_add_entry(option_table,"extend_length",&extend_length,
				NULL,set_double);
			/* font */
			Option_table_add_name_entry(option_table, "font",
				&font_name);
			/* label_material */
			Option_table_add_set_Material_entry(option_table, "label_material", &label_material,
				command_data->materialmodule);
			/* length */
			Option_table_add_entry(option_table,"length",&bar_length,
				NULL,set_double);
			/* number_format */
			Option_table_add_entry(option_table,"number_format",&number_format,
				(void *)1,set_name);
			/* material */
			Option_table_add_set_Material_entry(option_table, "material", &material,
				command_data->materialmodule);
			/* radius */
			Option_table_add_entry(option_table,"radius",&bar_radius,
				NULL,set_double);
			/* spectrum */
			Option_table_add_entry(option_table,"spectrum",&spectrum,
				command_data->spectrum_manager,set_Spectrum);
			/* tick_direction */
			Option_table_add_entry(option_table,"tick_direction",side_axis,
				&number_of_components,set_double_vector);
			/* tick_length */
			Option_table_add_entry(option_table,"tick_length",&tick_length,
				NULL,set_double);
			if (0 != (return_code = Option_table_multi_parse(option_table, state)))
			{
				if (material)
				{
					display_message(WARNING_MESSAGE,
						"gfx_create_colour_bar.  material option is not used; material is now taken from g_element graphics");
				}
				cmzn_glyphmodule_begin_change(command_data->glyphmodule);
				cmzn_glyph_id glyph = cmzn_glyphmodule_find_glyph_by_name(command_data->glyphmodule, glyph_name);
				cmzn_glyph_colour_bar_id colour_bar = 0;
				if (glyph)
				{
					colour_bar = cmzn_glyph_cast_colour_bar(glyph);
					if (0 == colour_bar)
					{
						display_message(ERROR_MESSAGE, "Glyph '%s' is not a colour bar", glyph_name);
						return_code = 0;
					}
					cmzn_glyph_destroy(&glyph);
				}
				else
				{
					cmzn_glyph_id colour_bar_glyph = cmzn_glyphmodule_create_glyph_colour_bar(command_data->glyphmodule, spectrum);
					cmzn_glyph_set_name(colour_bar_glyph, glyph_name);
					cmzn_glyph_set_managed(colour_bar_glyph, true);
					colour_bar = cmzn_glyph_cast_colour_bar(colour_bar_glyph);
					cmzn_glyph_destroy(&colour_bar_glyph);
				}
				if (colour_bar)
				{
					double magnitude = sqrt(bar_axis[0]*bar_axis[0]+bar_axis[1]*bar_axis[1]+bar_axis[2]*bar_axis[2]);
					if (magnitude > 0.0)
					{
						for (int i = 0; i < 3; ++i)
						{
							bar_axis[i] *= (bar_length / magnitude);
						}
					}
					magnitude = sqrt(side_axis[0]*side_axis[0]+side_axis[1]*side_axis[1]+side_axis[2]*side_axis[2]);
					if (magnitude > 0.0)
					{
						for (int i = 0; i < 3; ++i)
						{
							side_axis[i] *= (bar_radius * 2.0 / magnitude);
						}
					}
					cmzn_glyph_colour_bar_set_axis(colour_bar, 3, bar_axis);
					cmzn_glyph_colour_bar_set_centre(colour_bar, 3, bar_centre);
					cmzn_glyph_colour_bar_set_extend_length(colour_bar, extend_length);
					cmzn_glyph_colour_bar_set_label_divisions(colour_bar, tick_divisions);
					cmzn_glyph_colour_bar_set_label_material(colour_bar, label_material);
					cmzn_glyph_colour_bar_set_number_format(colour_bar, number_format);
					cmzn_glyph_colour_bar_set_side_axis(colour_bar, 3, side_axis);
					cmzn_glyph_colour_bar_set_tick_length(colour_bar, tick_length);
					cmzn_glyph_colour_bar_destroy(&colour_bar);
				}
				cmzn_glyphmodule_end_change(command_data->glyphmodule);
			} /* parse error, help */
			DESTROY(Option_table)(&option_table);
			cmzn_material_destroy(&label_material);
			cmzn_material_destroy(&material);
			DEACCESS(cmzn_spectrum)(&spectrum);
			DEALLOCATE(glyph_name);
			DEALLOCATE(number_format);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_create_colour_bar.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_create_colour_bar.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
}

#if defined (WX_USER_INTERFACE)
static int gfx_create_element_creator(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 17 January 2003

DESCRIPTION :
Executes a GFX CREATE ELEMENT_CREATOR command.
==============================================================================*/
{
	int return_code;
	USE_PARAMETER(dummy_to_be_modified);
	ENTER(gfx_create_element_creator);
	USE_PARAMETER(state);
	USE_PARAMETER(command_data_void);
	display_message(INFORMATION_MESSAGE,"\ncommand has been removed from the cmgui-wx.\n"
		"please use gfx modify window (NAME) node ? for further instruction for creating elements\n"
		"or directly create new elements using the node tool");
	return_code = 0;
	LEAVE;
	return (return_code);
} /* gfx_create_element_creator */
#endif /* defined (WX_USER_INTERFACE)*/

struct Interpreter_command_element_selection_callback_data
{
	char *perl_action;
	struct Interpreter *interpreter;
}; /* struct Interpreter_command_element_selection_callback_data */

/**
 * Adds or removes elements to/from group.
 */
static int process_modify_element_group(cmzn_field_group_id group,
	cmzn_region_id region, int dimension, char add_flag,
	cmzn_field_id conditional_field, cmzn_field_group_id from_group,
	Multi_range *element_ranges, char selected_flag, FE_value time,
	cmzn_field_group_subelement_handling_mode subelementHandlingMode)
{
	if (!group || !region)
		return 0;
	int return_code = 1;
	cmzn_fieldmodule_id field_module = cmzn_region_get_fieldmodule(region);
	cmzn_mesh_id master_mesh = cmzn_fieldmodule_find_mesh_by_dimension(field_module, dimension);
	cmzn_mesh_group_id selection_mesh_group = 0;
	if (selected_flag)
	{
		cmzn_scene *scene = cmzn_region_get_scene(region);
		cmzn_field_id selection_field = cmzn_scene_get_selection_field(scene);
		cmzn_field_group_id selection_group = cmzn_field_cast_group(selection_field);
		cmzn_field_destroy(&selection_field);
		if (selection_group)
		{
			cmzn_field_element_group_id selection_element_group =
				cmzn_field_group_get_field_element_group(selection_group, master_mesh);
			if (selection_element_group)
			{
				selection_mesh_group = cmzn_field_element_group_get_mesh_group(selection_element_group);
				cmzn_field_element_group_destroy(&selection_element_group);
			}
			cmzn_field_group_destroy(&selection_group);
		}
		cmzn_scene_destroy(&scene);
	}
	cmzn_mesh_group_id from_mesh_group = 0;
	if (from_group)
	{
		cmzn_field_element_group_id from_element_group =
			cmzn_field_group_get_field_element_group(from_group, master_mesh);
		if (from_element_group)
		{
			from_mesh_group = cmzn_field_element_group_get_mesh_group(from_element_group);
			cmzn_field_element_group_destroy(&from_element_group);
		}
	}
	if (((!selected_flag) || selection_mesh_group) && ((!from_group) || from_mesh_group))
	{
		cmzn_fieldmodule_begin_change(field_module);
		cmzn_field_element_group_id modify_element_group = cmzn_field_group_get_field_element_group(group, master_mesh);
		if ((!modify_element_group) && add_flag)
			modify_element_group = cmzn_field_group_create_field_element_group(group, master_mesh);
		cmzn_mesh_group_id modify_mesh_group = cmzn_field_element_group_get_mesh_group(modify_element_group);
		cmzn_field_element_group_destroy(&modify_element_group);
		if (modify_mesh_group)
		{
			cmzn_field_group_subelement_handling_mode oldSubelementHandlingMode = cmzn_field_group_get_subelement_handling_mode(group);
			cmzn_field_group_set_subelement_handling_mode(group, subelementHandlingMode);
			cmzn_fieldcache_id cache = cmzn_fieldmodule_create_fieldcache(field_module);
			cmzn_fieldcache_set_time(cache, time);

			cmzn_mesh_id iteration_mesh = master_mesh;
			cmzn_mesh_id selection_mesh = cmzn_mesh_group_base_cast(selection_mesh_group);
			cmzn_mesh_id from_mesh = cmzn_mesh_group_base_cast(from_mesh_group);
			if (selected_flag && selection_mesh && !cmzn_mesh_match(selection_mesh, cmzn_mesh_group_base_cast(modify_mesh_group)))
			{
				iteration_mesh = selection_mesh;
			}
			if (from_mesh && (!cmzn_mesh_match(from_mesh, cmzn_mesh_group_base_cast(modify_mesh_group))) &&
				(cmzn_mesh_get_size(from_mesh) < cmzn_mesh_get_size(iteration_mesh)))
			{
				iteration_mesh = from_mesh;
			}
			cmzn_elementiterator_id iter = cmzn_mesh_create_elementiterator(iteration_mesh);
			cmzn_element_id element = 0;
			while (NULL != (element = cmzn_elementiterator_next_non_access(iter)))
			{
				if (element_ranges && !Multi_range_is_value_in_range(element_ranges, cmzn_element_get_identifier(element)))
					continue;
				if (selection_mesh && (selection_mesh != iteration_mesh) && !cmzn_mesh_contains_element(selection_mesh, element))
					continue;
				if (from_mesh && (from_mesh != iteration_mesh) && !cmzn_mesh_contains_element(from_mesh, element))
					continue;
				if (conditional_field)
				{
					cmzn_fieldcache_set_element(cache, element);
					if (!cmzn_field_evaluate_boolean(conditional_field, cache))
						continue;
				}
				if (add_flag)
				{
					int result = cmzn_mesh_group_add_element(modify_mesh_group, element);
					if ((CMZN_OK != result) && (CMZN_ERROR_ALREADY_EXISTS != result))
					{
						display_message(ERROR_MESSAGE, "gfx modify egroup:  Adding elements failed");
						return_code = 0;
						break;
					}
				}
				else
				{
					int result = cmzn_mesh_group_remove_element(modify_mesh_group, element);
					if ((CMZN_OK != result) && (CMZN_ERROR_NOT_FOUND != result))
					{
						display_message(ERROR_MESSAGE, "gfx modify egroup:  Removing elements failed");
						return_code = 0;
						break;
					}
				}
			}
			cmzn_elementiterator_destroy(&iter);
			cmzn_fieldcache_destroy(&cache);
			cmzn_field_group_set_subelement_handling_mode(group, oldSubelementHandlingMode);
			cmzn_mesh_group_destroy(&modify_mesh_group);
		}
		cmzn_fieldmodule_end_change(field_module);
	}
	cmzn_mesh_group_destroy(&from_mesh_group);
	cmzn_mesh_group_destroy(&selection_mesh_group);
	cmzn_mesh_destroy(&master_mesh);
	cmzn_fieldmodule_destroy(&field_module);
	return return_code;
}

/***************************************************************************//**
 * Executes a GFX CREATE EGROUP/NGROUP/DGROUP command.
 * <use_object_type> is an integer; 0=elements, 1=nodes, 2=data.
 */
static int gfx_create_group(struct Parse_state *state,
	void *use_object_type, void *root_region_void)
{
	int return_code = 0;
	cmzn_region_id root_region = reinterpret_cast<cmzn_region_id>(root_region_void);
	if (state && root_region)
	{
		int object_type = VOIDPTR2INT(use_object_type);
		char *group_name = 0;
		if (set_name(state, (void *)&group_name, (void *)1))
		{
			cmzn_region_id region = cmzn_region_access(root_region);
			Multi_range *add_ranges = CREATE(Multi_range)();
			char *from_group_name = 0;
			int manage_subobjects = 1;

			Option_table *option_table = CREATE(Option_table)();
			Option_table_add_entry(option_table, "add_ranges", add_ranges, NULL, set_Multi_range);
			Option_table_add_string_entry(option_table, "from", &from_group_name, " GROUP_NAME");
			Option_table_add_switch(option_table, "manage_subobjects", "no_manage_subobjects", &manage_subobjects);
			Option_table_add_set_cmzn_region(option_table, "region", root_region, &region);
			return_code = Option_table_multi_parse(option_table, state);
			DESTROY(Option_table)(&option_table);

			cmzn_field_group_id from_group = 0;
			if (return_code && from_group_name)
			{
				cmzn_field *field =	FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field,name)(
					from_group_name, cmzn_region_get_Computed_field_manager(region));
				from_group = cmzn_field_cast_group(field);
				if (!from_group)
				{
					display_message(ERROR_MESSAGE, "gfx create group: '%s' is not a group.", from_group_name);
					return_code = 0;
				}
			}
			if (return_code)
			{
				cmzn_region_id child_region = cmzn_region_find_child_by_name(region, group_name);
				if (child_region)
				{
					display_message(ERROR_MESSAGE, "Child region with name '%s' already exists.", group_name);
					cmzn_region_destroy(&child_region);
					return_code = 0;
				}
				cmzn_fieldmodule_id field_module = cmzn_region_get_fieldmodule(region);
				cmzn_field_id field = cmzn_fieldmodule_find_field_by_name(field_module, group_name);
				if (field)
				{
					display_message(ERROR_MESSAGE, "Group/field with name '%s' already exists.", group_name);
					cmzn_field_destroy(&field);
					return_code = 0;
				}
				if (return_code)
				{
					cmzn_fieldmodule_begin_change(field_module);
					cmzn_field_id group_field = cmzn_fieldmodule_create_field_group(field_module);
					return_code = cmzn_field_set_name(group_field, group_name) &&
						cmzn_field_set_managed(group_field, true);
					if (Multi_range_get_number_of_ranges(add_ranges) > 0)
					{
						cmzn_field_group_id group = cmzn_field_cast_group(group_field);
						switch (object_type)
						{
							case 0: // elements
							{
								int max_dimension = FE_region_get_highest_dimension(cmzn_region_get_FE_region(region));
								double time = 0;
								return_code = process_modify_element_group(group, region, max_dimension,
									/*add_flag*/1, /*conditional_field*/0, from_group, add_ranges,
									/*selected_flag*/0, time,
									manage_subobjects ? CMZN_FIELD_GROUP_SUBELEMENT_HANDLING_MODE_FULL : CMZN_FIELD_GROUP_SUBELEMENT_HANDLING_MODE_NONE);
							} break;
							case 1: // nodes
							case 2: // data
							{
								cmzn_nodeset_id master_nodeset = cmzn_fieldmodule_find_nodeset_by_field_domain_type(field_module,
									(object_type == 1) ? CMZN_FIELD_DOMAIN_TYPE_NODES : CMZN_FIELD_DOMAIN_TYPE_DATAPOINTS);
								cmzn_field_node_group_id node_group = cmzn_field_group_create_field_node_group(group, master_nodeset);
								cmzn_nodeset_group_id modify_nodeset_group = cmzn_field_node_group_get_nodeset_group(node_group);
								cmzn_nodeiterator_id iter = cmzn_nodeset_create_nodeiterator(master_nodeset);
								cmzn_node_id node = 0;
								while (NULL != (node = cmzn_nodeiterator_next_non_access(iter)))
								{
									if (Multi_range_is_value_in_range(add_ranges, cmzn_node_get_identifier(node)))
									{
										if (!cmzn_nodeset_group_add_node(modify_nodeset_group, node))
										{
											return_code = 0;
											break;
										}
									}
								}
								cmzn_nodeiterator_destroy(&iter);
								cmzn_nodeset_group_destroy(&modify_nodeset_group);
								cmzn_field_node_group_destroy(&node_group);
								cmzn_nodeset_destroy(&master_nodeset);
							} break;
							default:
							{
								return_code = 0;
							} break;
						}
						cmzn_field_group_destroy(&group);
					}
					cmzn_field_destroy(&group_field);
					cmzn_fieldmodule_end_change(field_module);
				}
				cmzn_fieldmodule_destroy(&field_module);
			}
			if (from_group_name)
			{
				DEALLOCATE(from_group_name);
			}
			cmzn_field_group_destroy(&from_group);
			cmzn_region_destroy(&region);
			DESTROY(Multi_range)(&add_ranges);
		}
		if (group_name)
		{
			DEALLOCATE(group_name);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_create_group.  Invalid argument(s)");
		return_code = 0;
	}
	return return_code;
}

static int gfx_create_flow_particles(struct Parse_state *state,
	void *create_more,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 19 May 2003

DESCRIPTION :
Executes a GFX CREATE FLOW_PARTICLES command.
==============================================================================*/
{
	struct cmzn_command_data *command_data = 0;
	int return_code = 1;
	if (state && (command_data = (struct cmzn_command_data *)command_data_void))
	{
		display_message(INFORMATION_MESSAGE,
			" The flow particles feature has been removed.");
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_create_flow_particles.  Invalid argument(s)");
		return_code = 0;
	}

	return (return_code);
} /* gfx_create_flow_particles */

static int gfx_modify_flow_particles(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 18 January 2000

DESCRIPTION :
Executes a GFX MODIFY FLOW_PARTICLES command.
==============================================================================*/
{
	int return_code;
	struct cmzn_command_data *command_data;

	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (NULL != (command_data = (struct cmzn_command_data *)command_data_void))
		{
			display_message(INFORMATION_MESSAGE,
				" The flow particles feature has been removed.");
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_modify_flow_particles.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_modify_flow_particles.  Missing state");
		return_code=0;
	}

	return (return_code);
} /* gfx_modify_flow_particles */

/***************************************************************************//**
 * Creates data points with embedded locations at Gauss points in a mesh.
 */
static int gfx_create_gauss_points(struct Parse_state *state,
	void *dummy_to_be_modified, void *root_region_void)
{
	ENTER(gfx_create_gauss_points);
	int return_code = 0;
	USE_PARAMETER(dummy_to_be_modified);
	cmzn_region *root_region = reinterpret_cast<cmzn_region *>(root_region_void);
	if (state && root_region)
	{
		int first_identifier = 1;
		char *gauss_location_field_name = 0;
		char *gauss_weight_field_name = 0;
		char *gauss_point_nodeset_name = 0;
		char *mesh_name = 0;
		int order = 4;
		cmzn_region_id region = cmzn_region_access(root_region);
		Option_table *option_table = CREATE(Option_table)();
		Option_table_add_help(option_table,
			"Creates points at Gauss point locations in the elements of the mesh. "
			"Nodes are created in the gauss_point_nodeset starting from first_identifier, "
			"and setting the element_xi gauss_location and real gauss_weight fields. "
			"Supports all main element shapes, with polynomial order up to 4. Order gives "
			"the number of Gauss points per element dimension for line/square/cube shapes.");
		Option_table_add_int_non_negative_entry(option_table, "first_identifier",
			&first_identifier);
		Option_table_add_string_entry(option_table, "gauss_location_field",
			&gauss_location_field_name, " FIELD_NAME");
		Option_table_add_string_entry(option_table, "gauss_point_nodeset", &gauss_point_nodeset_name,
			" NODE_GROUP_FIELD_NAME|[GROUP_NAME.]nodes|datapoints|none");
		Option_table_add_string_entry(option_table, "gauss_weight_field",
			&gauss_weight_field_name, " FIELD_NAME");
		Option_table_add_string_entry(option_table, "mesh", &mesh_name,
			" ELEMENT_GROUP_FIELD_NAME|[GROUP_REGION_NAME.]mesh_1d|mesh_2d|mesh_3d");
		Option_table_add_int_positive_entry(option_table, "order", &order);
		Option_table_add_set_cmzn_region(option_table, "region", root_region, &region);
		return_code = Option_table_multi_parse(option_table, state);
		DESTROY(Option_table)(&option_table);
		if (return_code)
		{
			cmzn_fieldmodule_id field_module = cmzn_region_get_fieldmodule(region);
			cmzn_mesh_id mesh = 0;
			if (mesh_name)
			{
				mesh = cmzn_fieldmodule_find_mesh_by_name(field_module, mesh_name);
				if (!mesh)
				{
					display_message(ERROR_MESSAGE, "gfx create gauss_points:  Unknown mesh %s", mesh_name);
					return_code = 0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE, "gfx create gauss_points:  Must specify mesh");
				return_code = 0;
			}
			cmzn_nodeset_id gauss_points_nodeset = 0;
			if (gauss_point_nodeset_name)
			{
				gauss_points_nodeset = cmzn_fieldmodule_find_nodeset_by_name(field_module, gauss_point_nodeset_name);
				if (!gauss_points_nodeset)
				{
					gauss_points_nodeset = cmzn_nodeset_group_base_cast(
						cmzn_fieldmodule_create_nodeset_group_from_name_internal(
							field_module, gauss_point_nodeset_name));
				}
				if (!gauss_points_nodeset)
				{
					display_message(ERROR_MESSAGE,
						"gfx create gauss_points:  Unable to find nodeset %s", gauss_point_nodeset_name);
					return_code = 0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx create gauss_points:  Must specify gauss_point_nodeset");
				return_code = 0;
			}
			cmzn_field_stored_mesh_location_id gauss_location_field = 0;
			if (gauss_location_field_name)
			{
				cmzn_field_id field = cmzn_fieldmodule_find_field_by_name(field_module, gauss_location_field_name);
				if (field)
				{
					gauss_location_field = cmzn_field_cast_stored_mesh_location(field);
					cmzn_field_destroy(&field);
					if (!gauss_location_field)
					{
						display_message(ERROR_MESSAGE, "gfx create gauss_points:  Gauss location field %s is not element_xi valued", gauss_location_field_name);
						return_code = 0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE, "gfx create gauss_points:  No such field %s in region", gauss_location_field_name);
					return_code = 0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE, "gfx create gauss_points:  Must specify gauss_location_field");
				return_code = 0;
			}
			cmzn_field_finite_element_id gauss_weight_field = 0;
			if (gauss_weight_field_name)
			{
				cmzn_field_id field = cmzn_fieldmodule_find_field_by_name(field_module, gauss_weight_field_name);
				if (field)
				{
					gauss_weight_field = cmzn_field_cast_finite_element(field);
					cmzn_field_destroy(&field);
					if (!gauss_weight_field)
					{
						display_message(ERROR_MESSAGE, "gfx create gauss_points:  Gauss weight field %s is not scalar real finite_element", gauss_weight_field_name);
						return_code = 0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE, "gfx create gauss_points:  No such field %s in region", gauss_weight_field_name);
					return_code = 0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE, "gfx create gauss_points:  Must specify gauss_weight_field");
				return_code = 0;
			}
			if (return_code)
			{
				return_code = cmzn_mesh_create_gauss_points(mesh, order, gauss_points_nodeset,
					first_identifier, gauss_location_field, gauss_weight_field);
			}
			cmzn_field_finite_element_destroy(&gauss_weight_field);
			cmzn_field_stored_mesh_location_destroy(&gauss_location_field);
			cmzn_nodeset_destroy(&gauss_points_nodeset);
			cmzn_mesh_destroy(&mesh);
			cmzn_fieldmodule_destroy(&field_module);
		}
		if (gauss_location_field_name)
			DEALLOCATE(gauss_location_field_name);
		if (gauss_weight_field_name)
			DEALLOCATE(gauss_weight_field_name);
		if (gauss_point_nodeset_name)
			DEALLOCATE(gauss_point_nodeset_name);
		if (mesh_name)
			DEALLOCATE(mesh_name);
		cmzn_region_destroy(&region);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_create_gauss_points.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;
	return (return_code);
}

#if defined (WX_USER_INTERFACE)
static int gfx_create_graphical_material_editor(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 14 June 1999

DESCRIPTION :
Executes a GFX CREATE GRAPHICAL_MATERIAL_EDITOR command.
If there is a material editor dialog in existence, then bring it to the front,
otherwise it creates a new one.  Assumes we will only ever want one material
editor at a time.  This implementation may be changed later.
==============================================================================*/
{
	const char *current_token;
	int return_code;
	struct cmzn_command_data *command_data;

	ENTER(gfx_create_graphical_material_editor);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (NULL != (current_token = state->current_token))
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				display_message(ERROR_MESSAGE,"Unknown option: %s",current_token);
				display_parse_state_location(state);
				return_code=0;
			}
			else
			{
				return_code=1;
			}
		}
		else
		{
			if (NULL != (command_data = (struct cmzn_command_data *)command_data_void))
			{
#if defined (WX_USER_INTERFACE)
				return_code=material_editor_bring_up_editor(
					&(command_data->material_editor),
					command_data->root_region,
					command_data->graphics_module,
					command_data->graphics_buffer_package, command_data->user_interface);
#endif /* defined (WX_USER_INTERFACE) */
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_create_graphical_material_editor.  Missing command_data");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_create_graphical_material_editor.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_graphical_material_editor */
#endif /* defined (WX_USER_INTERFACE)*/

static int gfx_create_light(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 14 June 1999

DESCRIPTION :
Executes a GFX CREATE LIGHT command.
==============================================================================*/
{
	const char *current_token;
	int return_code;
	struct cmzn_command_data *command_data;
	struct Modify_light_data modify_light_data;

	ENTER(gfx_create_light);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (NULL != (current_token = state->current_token))
		{
			if (NULL != (command_data = (struct cmzn_command_data *)command_data_void))
			{
				if (strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
				{
					cmzn_lightmodule_begin_change(command_data->lightmodule);
					cmzn_light_id light =
						cmzn_lightmodule_find_light_by_name(command_data->lightmodule, current_token);

					if (!light)
					{
						light = cmzn_lightmodule_create_light(command_data->lightmodule);
						cmzn_light_set_name(light, current_token);
					}
					shift_Parse_state(state,1);
					if (light)
					{
						cmzn_light_set_managed(light, true);
						modify_light_data.default_light=command_data->default_light;
						modify_light_data.lightmodule=command_data->lightmodule;
						return_code=modify_cmzn_light(state,(void *)light,
							(void *)(&modify_light_data));
						if (light)
						{
							cmzn_light_set_managed(light, true);
						}
					}
					cmzn_light_destroy(&light);
					cmzn_lightmodule_end_change(command_data->lightmodule);
				}
				else
				{
					modify_light_data.default_light=command_data->default_light;
					modify_light_data.lightmodule=command_data->lightmodule;
					return_code=modify_cmzn_light(state,(void *)NULL,
						(void *)(&modify_light_data));
					return_code=1;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_create_light.  Missing command_data_void");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Missing light name");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_create_light.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_light */

/**
 * Legacy GFX CREATE|MODIFY LMODEL command. Now only gives guidance for replacement
 * commands.
 */
static int gfx_create_modify_light_model(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
{
	USE_PARAMETER(state);
	USE_PARAMETER(dummy_to_be_modified);
	USE_PARAMETER(command_data_void);
	display_message(INFORMATION_MESSAGE,
		"Light model is no longer supported. Local/infinite viewer and one/two sided "
		"lighting are now set with the 'gfx modify window NAME image ?' command. "
		"Ambient lighting is handled by ambient light type; modify default ambient "
		"light colour with command 'gfx modify light default_ambient colour R G B' "
		"or create a new ambient light and add it to a window.\n");
	return 1;
} /* gfx_create_modify_light_model */

#if defined (WX_USER_INTERFACE)
static int gfx_create_node_viewer(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 6 July 2001

DESCRIPTION :
Executes a GFX CREATE NODE_VIEWER command.
==============================================================================*/
{
	const char *current_token;
	int return_code;
	struct cmzn_command_data *command_data;

	ENTER(gfx_create_node_viewer);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (NULL != (current_token = state->current_token))
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				display_message(ERROR_MESSAGE,"Unknown option: %s",current_token);
				display_parse_state_location(state);
				return_code=0;
			}
			else
			{
				return_code=1;
			}
		}
		else
		{
			if (NULL != (command_data = (struct cmzn_command_data *)command_data_void))
			{
				if (command_data->node_viewer)
				{
					return_code=Node_viewer_bring_window_to_front(
						command_data->node_viewer);
				}
				else
				{
					if (NULL != (command_data->node_viewer = Node_viewer_create(
						&(command_data->node_viewer),
						"Node Viewer",
						command_data->root_region, CMZN_FIELD_DOMAIN_TYPE_NODES,
						command_data->default_time_keeper_app->getTimeKeeper())))
					{
						return_code=1;
					}
					else
					{
						return_code=0;
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_create_node_viewer.  Missing command_data");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_create_node_viewer.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_node_viewer */
#endif /* defined (WX_USER_INTERFACE) */

#if defined (WX_USER_INTERFACE)
static int gfx_create_data_viewer(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 6 July 2001

DESCRIPTION :
Executes a GFX CREATE DATA_VIEWER command.
==============================================================================*/
{
	const char *current_token;
	int return_code;
	struct cmzn_command_data *command_data;

	ENTER(gfx_create_data_viewer);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (NULL != (current_token = state->current_token))
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				display_message(ERROR_MESSAGE,"Unknown option: %s",current_token);
				display_parse_state_location(state);
				return_code=0;
			}
			else
			{
				return_code=1;
			}
		}
		else
		{
			if (NULL != (command_data = (struct cmzn_command_data *)command_data_void))
			{
				if (command_data->data_viewer)
				{
					return_code=Node_viewer_bring_window_to_front(
						command_data->data_viewer);
				}
				else
				{
					if (NULL != (command_data->node_viewer = Node_viewer_create(
						&(command_data->node_viewer),
						"Data Viewer",
						command_data->root_region, CMZN_FIELD_DOMAIN_TYPE_DATAPOINTS,
						command_data->default_time_keeper_app->getTimeKeeper())))
					{
						return_code=1;
					}
					else
					{
						return_code=0;
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_create_data_viewer.  Missing command_data");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_create_data_viewer.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_data_viewer */
#endif /* defined (WX_USER_INTERFACE)*/

#if defined (WX_USER_INTERFACE)
static int gfx_create_element_point_viewer(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 31 May 2000

DESCRIPTION :
Executes a GFX CREATE ELEMENT_POINT_VIEWER command.
==============================================================================*/
{
	const char *current_token;
	int return_code;
	struct cmzn_command_data *command_data;
	struct Time_object *time_object;

	ENTER(gfx_create_element_point_viewer);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (NULL != (current_token = state->current_token))
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				display_message(ERROR_MESSAGE,"Unknown option: %s",current_token);
				display_parse_state_location(state);
				return_code=0;
			}
			else
			{
				return_code=1;
			}
		}
		else
		{
			if (NULL != (command_data = (struct cmzn_command_data *)command_data_void))
			{
				if (command_data->element_point_viewer)
				{
					return_code=Element_point_viewer_bring_window_to_front(
						command_data->element_point_viewer);
				}
				else
				{
					if ((time_object = Time_object_create_regular(
								 /*update_frequency*/10.0, /*time_offset*/0.0))
						&&(command_data->default_time_keeper_app->getTimeKeeper()->addTimeObject(time_object)))
					{
						Time_object_set_name(time_object, "element_point_viewer_time");
						if (NULL != (command_data->element_point_viewer=CREATE(Element_point_viewer)(
									&(command_data->element_point_viewer),
									command_data->root_region,
									command_data->element_point_ranges_selection,
									command_data->computed_field_package,
									time_object,
									command_data->user_interface)))
						{
							return_code=1;
						}
						else
						{
							return_code=0;
						}
						DEACCESS(Time_object)(&time_object);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"gfx_create_element_point_viewer.  Unable to make time object.");
						return_code=0;
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_create_element_point_viewer.  Missing command_data");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_create_element_point_viewer.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_element_point_viewer */
#endif /* defined (WX_USER_INTERFACE) */

/***************************************************************************//**
 * Executes a GFX CREATE REGION command.
 */
static int gfx_create_region(struct Parse_state *state,
	void *dummy, void *root_region_void)
{
	int return_code = 0;

	ENTER(gfx_create_region);
	USE_PARAMETER(dummy);
	cmzn_region *root_region = (struct cmzn_region *)root_region_void;
	if (state && root_region)
	{
		char *region_path = NULL;
		int error_if_exists = 1;
		Option_table *option_table = CREATE(Option_table)();
		Option_table_add_help(option_table,
			"Create a region at the supplied path, with names in the path "
			"separated by slash '/' characters. Use the 'no_error_if_exists' "
			"option to avoid errors if region exists already. ");
		Option_table_add_switch(option_table,
			"error_if_exists", "no_error_if_exists", &error_if_exists);
		Option_table_add_default_string_entry(option_table, &region_path,
			"PATH_TO_REGION");
		return_code = Option_table_multi_parse(option_table, state);
		DESTROY(Option_table)(&option_table);
		if (return_code)
		{
			cmzn_region *region = cmzn_region_find_subregion_at_path(root_region, region_path);
			if (region)
			{
				if (error_if_exists)
				{
					display_message(ERROR_MESSAGE,
						"gfx create region.  Region already exists at path '%s'", region_path);
					return_code = 0;
				}
				cmzn_region_destroy(&region);
			}
			else
			{
				region = cmzn_region_create_subregion(root_region, region_path);
				if (region)
				{
					cmzn_region_destroy(&region);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"gfx create region.  Invalid region path '%s'", region_path);
					return_code = 0;
				}
			}
		}
		DEALLOCATE(region_path);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_create_region.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
}

static int gfx_create_snake(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 3 May 2006

DESCRIPTION :
Executes a GFX CREATE SNAKE command.
==============================================================================*/
{
	char *source_region_path;
	float density_factor, stiffness;
	int i, number_of_elements, number_of_fitting_fields,
		previous_state_index, return_code;
	struct cmzn_command_data *command_data;
	struct cmzn_region *region, *source_region;
	struct Computed_field *coordinate_field, **fitting_fields,
		*weight_field;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_coordinate_field_data,
		set_fitting_field_data, set_weight_field_data;
	struct Set_Computed_field_array_data set_fitting_field_array_data;

	ENTER(gfx_create_snake);
	USE_PARAMETER(dummy_to_be_modified);
	if (state && (command_data = (struct cmzn_command_data *)command_data_void))
	{
		region = cmzn_region_access(command_data->root_region);
		cmzn_field_group_id group = 0;
		source_region_path = (char *)NULL;
		number_of_fitting_fields = 1;
		coordinate_field = (struct Computed_field *)NULL;
		weight_field = (struct Computed_field *)NULL;
		density_factor = 0.0;
		number_of_elements = 1;
		stiffness = 0.0;

		if (strcmp(PARSER_HELP_STRING,state->current_token)&&
			strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token))
		{
			/* Skip this preprocessing if we are just getting the help */
			number_of_fitting_fields = 1;
			previous_state_index = state->current_index;

			option_table = CREATE(Option_table)();
			/* number_of_fitting_fields */
			Option_table_add_entry(option_table, "number_of_fitting_fields",
				&number_of_fitting_fields, NULL, set_int_positive);
			/* absorb everything else */
			Option_table_ignore_all_unmatched_entries(option_table);
			return_code = Option_table_multi_parse(option_table, state);
			DESTROY(Option_table)(&option_table);
			/* Return back to where we were */
			shift_Parse_state(state, previous_state_index - state->current_index);
		}

		if (number_of_fitting_fields)
		{
			ALLOCATE(fitting_fields, struct Computed_field *, number_of_fitting_fields);
			for (i = 0; i < number_of_fitting_fields; i++)
			{
				fitting_fields[i] = (struct Computed_field *)NULL;
			}
		}
		else
		{
			fitting_fields = (struct Computed_field **)NULL;
		}

		option_table = CREATE(Option_table)();
		/* coordinate */
		set_coordinate_field_data.conditional_function =
			Computed_field_has_numerical_components;
		set_coordinate_field_data.conditional_function_user_data = (void *)NULL;
		set_coordinate_field_data.computed_field_manager =
			Computed_field_package_get_computed_field_manager(
				command_data->computed_field_package);
		Option_table_add_entry(option_table, "coordinate",
			&coordinate_field, (void *)&set_coordinate_field_data,
			set_Computed_field_conditional);
		/* density_factor */
		Option_table_add_entry(option_table, "density_factor",
			&density_factor, NULL, set_float_0_to_1_inclusive);
		/* source_group */
		Option_table_add_entry(option_table, "source_group", &source_region_path,
			command_data->root_region, set_cmzn_region_path);
		/* destination_group */
		Option_table_add_region_or_group_entry(option_table, "destination_group",
			&region, &group);
		/* fitting_fields */
		set_fitting_field_data.conditional_function =
			Computed_field_has_numerical_components;
		set_fitting_field_data.conditional_function_user_data = (void *)NULL;
		set_fitting_field_data.computed_field_manager =
			Computed_field_package_get_computed_field_manager(
				command_data->computed_field_package);
		set_fitting_field_array_data.number_of_fields = number_of_fitting_fields;
		set_fitting_field_array_data.conditional_data = &set_fitting_field_data;
		Option_table_add_entry(option_table, "fitting_fields", fitting_fields,
			&set_fitting_field_array_data, set_Computed_field_array);
		/* number_of_fitting_fields */
		Option_table_add_entry(option_table, "number_of_fitting_fields",
			&number_of_fitting_fields, NULL, set_int_positive);
		/* number_of_elements */
		Option_table_add_entry(option_table, "number_of_elements",
			&number_of_elements, NULL, set_int_positive);
		/* stiffness */
		Option_table_add_entry(option_table, "stiffness",
			&stiffness, NULL, set_float_non_negative);
		/* weight_field */
		set_weight_field_data.conditional_function =
			Computed_field_is_scalar;
		set_weight_field_data.conditional_function_user_data = (void *)NULL;
		set_weight_field_data.computed_field_manager =
			Computed_field_package_get_computed_field_manager(
				command_data->computed_field_package);
		Option_table_add_entry(option_table, "weight_field",
			&weight_field, (void *)&set_weight_field_data,
			set_Computed_field_conditional);
		return_code = Option_table_multi_parse(option_table, state);
		DESTROY(Option_table)(&option_table);

		if (return_code)
		{
			if (!coordinate_field)
			{
				display_message(ERROR_MESSAGE, "gfx create snake.  "
					"Must specify a coordinate_field to define on elements in snake");
				return_code = 0;
			}
			if (!(source_region_path &&
				cmzn_region_get_region_from_path_deprecated(command_data->root_region,
					source_region_path, &source_region)))
			{
				source_region = command_data->root_region;
			}
		}
		if (return_code)
		{
			cmzn_fieldmodule_id source_fieldmodule = cmzn_region_get_fieldmodule(source_region);
			cmzn_nodeset_id source_nodeset = cmzn_fieldmodule_find_nodeset_by_field_domain_type(source_fieldmodule,
				CMZN_FIELD_DOMAIN_TYPE_DATAPOINTS);
			cmzn_scene *scene = cmzn_region_get_scene(source_region);
			cmzn_field_id selection_field = cmzn_scene_get_selection_field(scene);
			cmzn_field_group_id selection_group = cmzn_field_cast_group(selection_field);
			if (selection_group)
			{
				// if selection node group is empty => nothing to do
				cmzn_field_node_group_id node_group_field = cmzn_field_group_get_field_node_group(selection_group, source_nodeset);
				cmzn_nodeset_destroy(&source_nodeset);
				source_nodeset = cmzn_nodeset_group_base_cast(cmzn_field_node_group_get_nodeset_group(node_group_field));
				cmzn_field_node_group_destroy(&node_group_field);
			}
			cmzn_field_group_destroy(&selection_group);
			cmzn_field_destroy(&selection_field);
			cmzn_scene_destroy(&scene);
			if ((source_nodeset) && (cmzn_nodeset_get_size(source_nodeset) > 1))
			{
				cmzn_fieldmodule_id target_fieldmodule = cmzn_region_get_fieldmodule(region);
				cmzn_fieldmodule_begin_change(target_fieldmodule);
				FE_region *fe_region = cmzn_region_get_FE_region(region);
				cmzn_nodeset_group_id nodeset_group = 0;
				cmzn_mesh_group_id mesh_group = 0;
				if (group)
				{
					cmzn_nodeset_id master_nodeset = cmzn_fieldmodule_find_nodeset_by_field_domain_type(target_fieldmodule, CMZN_FIELD_DOMAIN_TYPE_NODES);
					cmzn_field_node_group_id node_group = cmzn_field_group_get_field_node_group(group, master_nodeset);
					if (!node_group)
						node_group = cmzn_field_group_create_field_node_group(group, master_nodeset);
					nodeset_group = cmzn_field_node_group_get_nodeset_group(node_group);
					cmzn_field_node_group_destroy(&node_group);
					cmzn_nodeset_destroy(&master_nodeset);

					cmzn_mesh_id master_mesh = cmzn_fieldmodule_find_mesh_by_dimension(target_fieldmodule, 1);
					cmzn_field_element_group_id element_group = cmzn_field_group_get_field_element_group(group, master_mesh);
					if (!element_group)
						element_group = cmzn_field_group_create_field_element_group(group, master_mesh);
					mesh_group = cmzn_field_element_group_get_mesh_group(element_group);
					cmzn_field_element_group_destroy(&element_group);
					cmzn_mesh_destroy(&master_mesh);
				}
				return_code = create_FE_element_snake_from_data_points(
					fe_region, coordinate_field, weight_field,
					number_of_fitting_fields, fitting_fields,
					source_nodeset,
					number_of_elements,
					density_factor,
					stiffness, nodeset_group, mesh_group);
				cmzn_mesh_group_destroy(&mesh_group);
				cmzn_nodeset_group_destroy(&nodeset_group);
				cmzn_fieldmodule_end_change(target_fieldmodule);
				cmzn_fieldmodule_destroy(&target_fieldmodule);
			}
			else
			{
				display_message(ERROR_MESSAGE, "gfx_create_snake.  Not enough selected data points");
				return_code = 0;
			}
			cmzn_nodeset_destroy(&source_nodeset);
			cmzn_fieldmodule_destroy(&source_fieldmodule);
		}
		if (coordinate_field)
		{
			DEACCESS(Computed_field)(&coordinate_field);
		}
		if (weight_field)
		{
			DEACCESS(Computed_field)(&weight_field);
		}
		if (fitting_fields)
		{
			for (i = 0; i < number_of_fitting_fields; i++)
			{
				if (fitting_fields[i])
				{
					DEACCESS(Computed_field)(&fitting_fields[i]);
				}
			}
			DEALLOCATE(fitting_fields);
		}
		if (source_region_path)
		{
			DEALLOCATE(source_region_path);
		}
		cmzn_field_group_destroy(&group);
		cmzn_region_destroy(&region);
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_create_snake.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_snake */

static int gfx_modify_Spectrum(struct Parse_state *state,void *spectrum_void,
	void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 30 April 1999

DESCRIPTION :
Modifier function that parses all the command line options for creating or
modifying a spectrum.
I would put this with the other gfx modify routines but then it can't be
static and referred to by gfx_create_Spectrum
==============================================================================*/
{
	char autorange, blue_to_red, blue_white_red, clear, lg_blue_to_red,
		lg_red_to_blue, overlay_colour, overwrite_colour, red_to_blue;
	const char *current_token;
	int process, return_code;
	struct cmzn_command_data *command_data;
	struct Modify_spectrum_app_data modify_spectrum_data;
	struct Option_table *option_table;
	struct Scene *autorange_scene;
	struct cmzn_spectrum *spectrum_to_be_modified,*spectrum_to_be_modified_copy;


	ENTER(gfx_modify_Spectrum);
	if (state)
	{
		if (NULL != (command_data = (struct cmzn_command_data *)command_data_void))
		{
			if (NULL != (current_token = state->current_token))
			{
				process=0;
				if (NULL != (spectrum_to_be_modified=(struct cmzn_spectrum *)spectrum_void))
				{
					if (IS_MANAGED(cmzn_spectrum)(spectrum_to_be_modified,
						command_data->spectrum_manager))
					{
						spectrum_to_be_modified_copy=cmzn_spectrum_create_private();
						cmzn_spectrum_set_name(spectrum_to_be_modified_copy, "spectrum_modify_temp");
						if (spectrum_to_be_modified_copy)
						{
							MANAGER_COPY_WITHOUT_IDENTIFIER(cmzn_spectrum,name)(
								spectrum_to_be_modified_copy,spectrum_to_be_modified);
							process=1;
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"gfx_modify_Spectrum.  Could not create spectrum copy.");
							return_code=0;
						}
					}
					else
					{
						spectrum_to_be_modified_copy=spectrum_to_be_modified;
						spectrum_to_be_modified=(struct cmzn_spectrum *)NULL;
						process=1;
					}
				}
				else
				{
					if (strcmp(PARSER_HELP_STRING,current_token)&&
						strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
					{
						if (NULL != (spectrum_to_be_modified=FIND_BY_IDENTIFIER_IN_MANAGER(
							cmzn_spectrum,name)(current_token,
							command_data->spectrum_manager)))
						{
							return_code = shift_Parse_state(state, 1);
							if (return_code)
							{
								spectrum_to_be_modified_copy=cmzn_spectrum_create_private();
								cmzn_spectrum_set_name(spectrum_to_be_modified_copy, "spectrum_modify_temp");
								if (spectrum_to_be_modified_copy)
								{
									MANAGER_COPY_WITH_IDENTIFIER(cmzn_spectrum,name)(
										spectrum_to_be_modified_copy,spectrum_to_be_modified);
									process=1;
								}
								else
								{
									display_message(ERROR_MESSAGE,
									"gfx_modify_Spectrum.  Could not create spectrum copy");
									return_code=0;
								}
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,"Unknown spectrum : %s",
								current_token);
							display_parse_state_location(state);
							return_code=0;
						}
					}
					else
					{
						spectrum_to_be_modified_copy=cmzn_spectrum_create_private();
						cmzn_spectrum_set_name(spectrum_to_be_modified_copy, "dummy");
						if (spectrum_to_be_modified_copy)
						{
							option_table=CREATE(Option_table)();
							Option_table_add_entry(option_table,"SPECTRUM_NAME",
								(void *)spectrum_to_be_modified_copy,command_data_void,
								gfx_modify_Spectrum);
							return_code=Option_table_parse(option_table,state);
							DESTROY(Option_table)(&option_table);
							DEACCESS(cmzn_spectrum)(&spectrum_to_be_modified_copy);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"gfx_modify_Spectrum.  Could not create dummy spectrum");
							return_code=0;
						}
					}
				}
				if (process)
				{
					autorange = 0;
					autorange_scene = ACCESS(Scene)(command_data->default_scene);
					blue_to_red = 0;
					clear = 0;
					lg_blue_to_red = 0;
					lg_red_to_blue = 0;
					overlay_colour = 0;
					overwrite_colour = 0;
					red_to_blue = 0;
					blue_white_red = 0;
					modify_spectrum_data.position = 0;
					modify_spectrum_data.component = (struct cmzn_spectrumcomponent *)NULL;
					modify_spectrum_data.spectrum_minimum = cmzn_spectrum_get_minimum(
						spectrum_to_be_modified_copy);
					modify_spectrum_data.spectrum_maximum = cmzn_spectrum_get_maximum(
						spectrum_to_be_modified_copy);
					modify_spectrum_data.computed_field_manager
						= Computed_field_package_get_computed_field_manager(
							command_data->computed_field_package);
					cmzn_scenefilter_id filter =
						cmzn_scenefiltermodule_get_default_scenefilter(command_data->filter_module);
					option_table=CREATE(Option_table)();
					Option_table_add_entry(option_table,"autorange",&autorange,NULL,
						set_char_flag);
					Option_table_add_entry(option_table,"blue_to_red",&blue_to_red,NULL,
						set_char_flag);
					Option_table_add_entry(option_table,"blue_white_red",&blue_white_red,NULL,
						set_char_flag);
					Option_table_add_entry(option_table,"clear",&clear,NULL,
						set_char_flag);
					Option_table_add_entry(option_table,"field",&modify_spectrum_data,
						NULL,gfx_modify_spectrum_settings_field);
					Option_table_add_entry(option_table, "filter", &filter,
						command_data->filter_module, set_cmzn_scenefilter);
					Option_table_add_entry(option_table,"linear",&modify_spectrum_data,
						NULL,gfx_modify_spectrum_settings_linear);
					Option_table_add_entry(option_table,"log",&modify_spectrum_data,
						NULL,gfx_modify_spectrum_settings_log);
					Option_table_add_entry(option_table,"lg_blue_to_red",&lg_blue_to_red,
						NULL,set_char_flag);
					Option_table_add_entry(option_table,"lg_red_to_blue",&lg_red_to_blue,
						NULL,set_char_flag);
					Option_table_add_entry(option_table,"maximum",&spectrum_to_be_modified_copy,
						NULL,set_Spectrum_maximum_command);
					Option_table_add_entry(option_table,"minimum",&spectrum_to_be_modified_copy,
						NULL,set_Spectrum_minimum_command);
					Option_table_add_entry(option_table,"overlay_colour",&overlay_colour,
						NULL,set_char_flag);
					Option_table_add_entry(option_table,"overwrite_colour",&overwrite_colour,
						NULL,set_char_flag);
					Option_table_add_entry(option_table,"scene_for_autorange",&autorange_scene,
						command_data->root_region,set_Scene);
					Option_table_add_entry(option_table,"red_to_blue",&red_to_blue,
						NULL,set_char_flag);
					if (0 != (return_code = Option_table_multi_parse(option_table, state)))
					{
						if (return_code)
						{
							if ( clear )
							{
								cmzn_spectrum_remove_all_spectrumcomponents(spectrum_to_be_modified_copy);
							}
							if (blue_to_red + blue_white_red +red_to_blue + lg_red_to_blue +
								lg_blue_to_red > 1 )
							{
								display_message(ERROR_MESSAGE,
									"gfx_modify_Spectrum.  Specify only one simple spectrum type\n "
									"   (blue_to_red, blue_white_red, red_to_blue, lg_red_to_blue, lg_blue_to_red)");
								return_code=0;
							}
							else if (red_to_blue)
							{
								Spectrum_set_simple_type(spectrum_to_be_modified_copy,
									RED_TO_BLUE_SPECTRUM);
							}
							else if (blue_to_red)
							{
								Spectrum_set_simple_type(spectrum_to_be_modified_copy,
									BLUE_TO_RED_SPECTRUM);
							}
							else if (blue_white_red)
							{
								Spectrum_set_simple_type(spectrum_to_be_modified_copy,
									BLUE_WHITE_RED_SPECTRUM);
							}
							else if (lg_red_to_blue)
							{
								Spectrum_set_simple_type(spectrum_to_be_modified_copy,
									LOG_RED_TO_BLUE_SPECTRUM);
							}
							else if (lg_blue_to_red)
							{
								Spectrum_set_simple_type(spectrum_to_be_modified_copy,
									LOG_BLUE_TO_RED_SPECTRUM);
							}
							if ( modify_spectrum_data.component )
							{
								/* add new settings */
								return_code=Spectrum_add_component(spectrum_to_be_modified_copy,
									modify_spectrum_data.component,
									modify_spectrum_data.position);
							}
							if (overlay_colour && overwrite_colour)
							{
								display_message(ERROR_MESSAGE,
									"gfx_modify_Spectrum.  Specify only one colour mode, overwrite_colour or overlay_colour");
								return_code=0;
							}
							else if (overlay_colour)
							{
								cmzn_spectrum_set_material_overwrite(spectrum_to_be_modified_copy,
									0);
							}
							else if (overwrite_colour)
							{
								cmzn_spectrum_set_material_overwrite(spectrum_to_be_modified_copy,
									1);
							}
							if (autorange)
							{
								double maximum, minimum;
								int maxRanges = cmzn_scene_get_spectrum_data_range(autorange_scene,
									filter, spectrum_to_be_modified
									/* Not spectrum_to_be_modified_copy as this ptr
										identifies the valid graphics objects */,
									/*valuesCount*/1, &minimum, &maximum);
								if ( maxRanges >= 1 )
								{
									Spectrum_set_minimum_and_maximum(spectrum_to_be_modified_copy,
										minimum, maximum );
								}
							}
							if (spectrum_to_be_modified)
							{
								MANAGER_MODIFY_NOT_IDENTIFIER(cmzn_spectrum,name)(
									spectrum_to_be_modified,spectrum_to_be_modified_copy,
									command_data->spectrum_manager);
								DEACCESS(cmzn_spectrum)(&spectrum_to_be_modified_copy);
							}
							else
							{
								spectrum_to_be_modified=spectrum_to_be_modified_copy;
							}
						}
						else
						{
							DEACCESS(cmzn_spectrum)(&spectrum_to_be_modified_copy);
						}
					}
					if(option_table)
					{
						DESTROY(Option_table)(&option_table);
					}
					if ( modify_spectrum_data.component )
					{
						DEACCESS(cmzn_spectrumcomponent)(&(modify_spectrum_data.component));
					}
					DEACCESS(Scene)(&autorange_scene);
					cmzn_scenefilter_destroy(&filter);
				}
			}
			else
			{
				if (spectrum_void)
				{
					display_message(ERROR_MESSAGE,"Missing spectrum modifications");
				}
				else
				{
					display_message(ERROR_MESSAGE,"Missing spectrum name");
				}
				display_parse_state_location(state);
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_modify_Spectrum.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
				"gfx_modify_Spectrum.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_modify_Spectrum */

static int gfx_create_spectrum(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 14 June 1999

DESCRIPTION :
Executes a GFX CREATE SPECTRUM command.
==============================================================================*/
{
	const char *current_token;
	int return_code;
	struct cmzn_command_data *command_data;
	struct cmzn_spectrum *spectrum;

	ENTER(gfx_create_spectrum);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (NULL != (current_token = state->current_token))
		{
			if (NULL != (command_data = (struct cmzn_command_data *)command_data_void))
			{
				if (strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
				{
					if (!FIND_BY_IDENTIFIER_IN_MANAGER(cmzn_spectrum,name)(
						current_token,command_data->spectrum_manager))
					{
						spectrum=cmzn_spectrum_create_private();
						cmzn_spectrum_set_name(spectrum, current_token);
						if (spectrum)
						{
							/*???DB.  Temporary */
							MANAGER_COPY_WITHOUT_IDENTIFIER(cmzn_spectrum,name)(spectrum,
								command_data->default_spectrum);
							shift_Parse_state(state,1);
							if (state->current_token)
							{
								return_code=gfx_modify_Spectrum(state,(void *)spectrum,
									command_data_void);
							}
							else
							{
								return_code=1;
							}
							if (return_code)
							{
								if (ADD_OBJECT_TO_MANAGER(cmzn_spectrum)(spectrum, command_data->spectrum_manager))
									cmzn_spectrum_set_managed(spectrum, true);
								else
									return_code = 0;
							}
							DEACCESS(cmzn_spectrum)(&spectrum);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"gfx_create_spectrum.  Error creating spectrum");
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"Spectrum already exists: %s",
							current_token);
						display_parse_state_location(state);
						return_code=0;
					}
				}
				else
				{
					return_code=gfx_modify_Spectrum(state,(void *)NULL,command_data_void);
					return_code=1;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_create_spectrum.  Missing command_data_void");
				return_code=0;
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
		display_message(ERROR_MESSAGE,"gfx_create_spectrum.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_spectrum */

static int set_Texture_image_from_field(struct Texture *texture,
	struct Computed_field *field,
	struct Computed_field *texture_coordinate_field,
	int propagate_field,
	struct cmzn_spectrum *spectrum,
	cmzn_mesh_id search_mesh,
	enum Texture_storage_type storage,
	int image_width, int image_height, int image_depth,
	int number_of_bytes_per_component,
	struct Graphics_buffer_app_package *graphics_buffer_package,
	cmzn_material *fail_material)
/*******************************************************************************
LAST MODIFIED : 30 June 2006

DESCRIPTION :
Creates the image in the format given by sampling the <field> according to the
reverse mapping of the <texture_coordinate_field>.  The values returned by
field are converted to "colours" by applying the <spectrum>.
Currently limited to 1 byte per component.
@param search_mesh  The mesh to find locations with matching texture coordinates.
==============================================================================*/
{
	char *field_name;
	int bytes_per_pixel, number_of_components, return_code,
		source_dimension, *source_sizes, tex_number_of_components, use_pixel_location = 1;
	struct Computed_field *source_texture_coordinate_field = NULL;

	ENTER(set_Texture_image_from_field);
	if (texture && field && spectrum &&
		(4 >= (number_of_components =
			Texture_storage_type_get_number_of_components(storage))))
	{
		/* Setup sizes */
		if (Computed_field_get_native_resolution(
			field, &source_dimension, &source_sizes,
			&source_texture_coordinate_field))
		{
			if (!texture_coordinate_field)
			{
				texture_coordinate_field =
					source_texture_coordinate_field;
			}
			if (image_width == 0)
			{
				if (source_dimension > 0)
				{
					image_width = source_sizes[0];
				}
				else
				{
					image_width = 1;
				}
			}
			if (image_height == 0)
			{
				if (source_dimension > 1)
				{
					image_height = source_sizes[1];
				}
				else
				{
					image_height = 1;
				}
			}
			if (image_depth == 0)
			{
				if (source_dimension > 2)
				{
					image_depth = source_sizes[2];
				}
				else
				{
					image_depth = 1;
				}
			}
			DEALLOCATE(source_sizes);
		}

		if (texture_coordinate_field &&
			(3 >= (tex_number_of_components =
			cmzn_field_get_number_of_components(texture_coordinate_field))))
		{
			return_code = 1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"set_Texture_image_from_field.  Invalid texture_coordinate field.");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_Texture_image_from_field.  Invalid argument(s)");
		return_code = 0;
	}
	if (return_code)
	{
		if (number_of_bytes_per_component <= 0)
		{
			 number_of_bytes_per_component = 1;
		}
		/* allocate the texture image */
		use_pixel_location = (texture_coordinate_field == source_texture_coordinate_field);
		field_name = (char *)NULL;
		GET_NAME(Computed_field)(field, &field_name);
		if (Texture_allocate_image(texture, image_width, image_height,
			image_depth, storage, number_of_bytes_per_component, field_name))
		{
			bytes_per_pixel = number_of_components*number_of_bytes_per_component;
			double texture_width, texture_height, texture_depth;
			Texture_get_physical_size(texture, &texture_width, &texture_height, &texture_depth);
			Set_cmiss_field_value_to_texture(field, texture_coordinate_field,
				texture, spectrum,	fail_material, image_width, image_height, image_depth,
				bytes_per_pixel, number_of_bytes_per_component, use_pixel_location, texture_width, texture_height, texture_depth,
				storage, propagate_field, Graphics_buffer_package_get_core_package(graphics_buffer_package), search_mesh);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"set_Texture_image_from_field.  Could not allocate image in texture");
			return_code = 0;
		}
		DEALLOCATE(field_name);
	}
	LEAVE;

	return (return_code);
} /* set_Texture_image_from_field */

int set_element_dimension_or_all(struct Parse_state *state,
	void *value_address_void, void *dummy_user_data)
/*******************************************************************************
LAST MODIFIED : 21 June 1999

DESCRIPTION :
Allows either "all" - a return value of zero - or an element dimension up to
MAXIMUM_ELEMENT_XI_DIMENSIONS to be set.
==============================================================================*/
{
	const char *current_token;
	int return_code = 0, value, *value_address;

	ENTER(set_element_dimension_or_all);
	USE_PARAMETER(dummy_user_data);
	if (state)
	{
		if (NULL != (current_token = state->current_token))
		{
			if (strcmp(PARSER_HELP_STRING, current_token) &&
				strcmp(PARSER_RECURSIVE_HELP_STRING, current_token))
			{
				if (NULL != (value_address = (int *)value_address_void))
				{
					if (fuzzy_string_compare_same_length(current_token, "ALL"))
					{
						*value_address = 0;
					}
					else if ((1 == sscanf(current_token, " %d ", &value)) &&
						(0 < value) && (value <= MAXIMUM_ELEMENT_XI_DIMENSIONS))
					{
						*value_address = value;
						return_code = shift_Parse_state(state, 1);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Invalid element dimension: %s\n", current_token);
						display_parse_state_location(state);
						return_code = 0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"set_element_dimension_or_all.  Missing value_address");
					return_code = 0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE, " #|ALL");
				if (NULL != (value_address = (int *)value_address_void))
				{
					if (0 == *value_address)
					{
						display_message(INFORMATION_MESSAGE, "[ALL]");
					}
					else
					{
						display_message(INFORMATION_MESSAGE, "[%d]", *value_address);
					}
				}
				return_code = 1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE, "Missing element dimension or ALL");
			display_parse_state_location(state);
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_element_dimension_or_all.  Missing state");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* set_element_dimension_or_all */

struct Texture_evaluate_image_data
{
	cmzn_region_id region;
	cmzn_field_group_id group;
	char *field_name, *texture_coordinates_field_name;
	int element_dimension; /* where 0 is any dimension */
	int propagate_field;
	struct Computed_field *field, *texture_coordinates_field;
	cmzn_material *fail_material;
	struct cmzn_spectrum *spectrum;
};

static int gfx_modify_Texture_evaluate_image(struct Parse_state *state,
	void *data_void, void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 16 May 2003

DESCRIPTION :
Modifies the properties of a texture.
==============================================================================*/
{
	int return_code;
	struct cmzn_command_data *command_data;
	struct Option_table *option_table;
	struct Texture_evaluate_image_data *data;

	ENTER(gfx_modify_Texture_evaluate_image);
	if (state && (command_data = (struct cmzn_command_data *)command_data_void)
		&& (data = (struct Texture_evaluate_image_data *)data_void))
	{
		if (state->current_token)
		{
			option_table = CREATE(Option_table)();
			/* element_dimension */
			Option_table_add_entry(option_table, "element_dimension",
				&data->element_dimension, NULL, set_element_dimension_or_all);
			/* element_group */
			Option_table_add_region_or_group_entry(option_table, "element_group",
				&data->region, &data->group);
			/* fail_material */
			Option_table_add_set_Material_entry(option_table, "fail_material",
				&data->fail_material, command_data->materialmodule);
			/* field */
			Option_table_add_entry(option_table, "field", &data->field_name,
				(void *)1, set_name);
			/* propagate_field/no_propagate_field */
			Option_table_add_switch(option_table, "propagate_field",
				"no_propagate_field", &data->propagate_field);
			/* spectrum */
			Option_table_add_entry(option_table, "spectrum", &data->spectrum,
				command_data->spectrum_manager, set_Spectrum);
			/* texture_coordinates */
			Option_table_add_entry(option_table, "texture_coordinates",
				&data->texture_coordinates_field_name, (void *)1, set_name);

			return_code = Option_table_multi_parse(option_table, state);
			DESTROY(Option_table)(&option_table);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_modify_Texture_evaluate_image.  Missing evaluate image options");
			display_parse_state_location(state);
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_modify_Texture_evaluate_image.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* gfx_modify_Texture_evaluate_image */

struct Texture_image_data
{
	char *image_file_name;
	int crop_bottom_margin,crop_left_margin,crop_height,crop_width;
};

static int gfx_modify_Texture_image(struct Parse_state *state,
	void *data_void, void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 7 January 2002

DESCRIPTION :
Modifies the properties of a texture.
==============================================================================*/
{
	const char *current_token;
	int return_code;
	struct cmzn_command_data *command_data;
	struct Option_table *option_table;
	struct Texture_image_data *data;

	ENTER(gfx_modify_Texture_image);
	if (state && (data = (struct Texture_image_data *)data_void) &&
		(command_data = (struct cmzn_command_data *)command_data_void))
	{
		return_code=1;
		if (NULL != (current_token = state->current_token))
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				if (fuzzy_string_compare("crop",current_token))
				{
					if (!(shift_Parse_state(state,1)&&
						(current_token=state->current_token)&&
						(1==sscanf(current_token," %d",&(data->crop_left_margin)))&&
						shift_Parse_state(state,1)&&(current_token=state->current_token)&&
						(1==sscanf(current_token," %d",&(data->crop_bottom_margin)))&&
						shift_Parse_state(state,1)&&(current_token=state->current_token)&&
						(1==sscanf(current_token," %d",&(data->crop_width)))&&
						shift_Parse_state(state,1)&&(current_token=state->current_token)&&
						(1==sscanf(current_token," %d",&(data->crop_height)))&&
						shift_Parse_state(state,1)))
					{
						display_message(WARNING_MESSAGE,"Missing/invalid crop value(s)");
						display_parse_state_location(state);
						return_code=0;
					}
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE,
					" <crop LEFT_MARGIN#[0] BOTTOM_MARGIN#[0] WIDTH#[0] HEIGHT#[0]>");
			}
		}
		if (return_code)
		{
			if (NULL != (current_token = state->current_token))
			{
				option_table = CREATE(Option_table)();
				/* example */
				Option_table_add_entry(option_table, CMGUI_EXAMPLE_DIRECTORY_SYMBOL,
					&(data->image_file_name), &(command_data->example_directory),
					set_file_name);
				/* default */
				Option_table_add_entry(option_table, NULL, &(data->image_file_name),
					NULL, set_file_name);
				return_code = Option_table_parse(option_table, state);
				DESTROY(Option_table)(&option_table);
				//				if (data->image_file_name)
				//{
				// DEALLOCATE(data->image_file_name);
				//}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx modify texture image:  Missing image file name");
				display_parse_state_location(state);
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_modify_Texture_image.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_modify_Texture_image */

struct Texture_file_number_series_data
{
	int start, stop, increment;
};

static int gfx_modify_Texture_file_number_series(struct Parse_state *state,
	void *data_void, void *dummy_user_data)
/*******************************************************************************
LAST MODIFIED : 8 February 2002

DESCRIPTION :
Modifies the properties of a texture.
==============================================================================*/
{
	const char *current_token;
	int range, return_code;
	struct Texture_file_number_series_data *data;

	ENTER(gfx_modify_Texture_file_number_series);
	USE_PARAMETER(dummy_user_data);
	if (state && (data = (struct Texture_file_number_series_data *)data_void))
	{
		return_code = 1;
		if (NULL != (current_token = state->current_token))
		{
			if (strcmp(PARSER_HELP_STRING, current_token) &&
				strcmp(PARSER_RECURSIVE_HELP_STRING, current_token))
			{
				if ((1 == sscanf(current_token, " %d", &(data->start))) &&
					shift_Parse_state(state, 1) &&
					(current_token = state->current_token) &&
					(1 == sscanf(current_token, " %d", &(data->stop))) &&
					shift_Parse_state(state, 1) &&
					(current_token = state->current_token) &&
					(1 == sscanf(current_token, " %d", &(data->increment))) &&
					shift_Parse_state(state, 1))
				{
					/* check range proceeds from start to stop with a whole number of
						 increments, and that increment is positive */
					if (!(((0 < data->increment) &&
						(0 <= (range = data->stop - data->start)) &&
						(0 == (range % data->increment))) ||
						((0 > data->increment) &&
							(0 <= (range = data->start - data->stop))
							&& (0 == (range % -data->increment)))))
					{
						display_message(ERROR_MESSAGE,
							"Invalid file number series");
						display_parse_state_location(state);
						return_code = 0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Missing 3-D image series START, STOP or INCREMENT");
					display_parse_state_location(state);
					return_code = 0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE, " START STOP INCREMENT");
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_modify_Texture_file_number_series.  Missing state");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* gfx_modify_Texture_file_number_series */

int gfx_modify_Texture(struct Parse_state *state,void *texture_void,
	void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 28 February 2003

DESCRIPTION :
Modifies the properties of a texture.
==============================================================================*/
{
	char *file_number_pattern, texture_tiling_enabled;
	const char *current_token, *combine_mode_string, *compression_mode_string, *filter_mode_string,
		*raw_image_storage_string, *resize_filter_mode_string, **valid_strings,
		*wrap_mode_string;
	double texture_distortion[3];
	enum Raw_image_storage raw_image_storage;
	enum Texture_combine_mode combine_mode;
	enum Texture_compression_mode compression_mode;
	enum Texture_filter_mode filter_mode;
	enum Texture_resize_filter_mode resize_filter_mode;
	enum Texture_storage_type specify_format;
	enum Texture_wrap_mode wrap_mode;
	double alpha, distortion_centre_x, distortion_centre_y,
		distortion_factor_k1, mipmap_level_of_detail_bias;
	float mipmap_level_of_detail_bias_flt;
	int file_number, i, number_of_file_names, number_of_valid_strings, process,
		return_code, specify_depth, specify_height,
		specify_number_of_bytes_per_component, specify_width, texture_is_managed = 0;
	struct Cmgui_image *cmgui_image;
	struct Cmgui_image_information *cmgui_image_information;
	struct cmzn_command_data *command_data;
	struct Colour colour;
	struct Option_table *option_table;
	struct Texture *texture, *texture_copy;
	struct Texture_image_data image_data;
	struct Texture_file_number_series_data file_number_series_data;
	/* do not make the following static as 'set' flag must start at 0 */
	struct Set_vector_with_help_data texture_distortion_data=
		{3," DISTORTION_CENTRE_X DISTORTION_CENTRE_Y DISTORTION_FACTOR_K1",0};
	struct MANAGER(Computed_field) *field_manager = NULL;
	struct Computed_field *image_field = NULL;
#if defined (SGI_MOVIE_FILE)
	struct Movie_graphics *movie, *old_movie;
	struct X3d_movie *x3d_movie;
#endif /* defined (SGI_MOVIE_FILE) */
	cmzn_field_image_id image = NULL;

	ENTER(gfx_modify_Texture);
	cmgui_image_information = NULL;
	if (state)
	{
		if (NULL != (current_token = state->current_token))
		{
			if (NULL != (command_data = (struct cmzn_command_data *)command_data_void))
			{
				process = 0;
				if (NULL != (texture = (struct Texture *)texture_void))
				{
					process = 1;
				}
				else
				{
					if (strcmp(PARSER_HELP_STRING,current_token)&&
						strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
					{
						struct cmzn_region *region = NULL;
						char *region_path, *field_name;
						if (cmzn_region_get_partial_region_path(command_data->root_region,
							current_token, &region, &region_path, &field_name))
						{
							if (field_name && (strlen(field_name) > 0) &&
								(strchr(field_name, CMZN_REGION_PATH_SEPARATOR_CHAR)	== NULL))
							{
								field_manager = cmzn_region_get_Computed_field_manager(region);
								Computed_field *existing_field =
									FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field,name)(
										field_name, field_manager);
								if (existing_field)
								{
									image = cmzn_field_cast_image(existing_field);
									if (image)
									{
										texture = cmzn_field_image_get_texture(image);
										texture_is_managed = 1;
										image_field = cmzn_field_access(existing_field);
									}
									cmzn_field_destroy(&existing_field);
								}
							}
							else
							{
								if (field_name)
								{
									display_message(ERROR_MESSAGE,
										"gfx_modify_Texture:  Invalid region path or texture field name '%s'", field_name);
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"gfx_modify_Texture:  Missing texture field name or name matches child region '%s'", current_token);
								}
								display_parse_state_location(state);
								return_code = 0;
							}
							if (region_path)
								DEALLOCATE(region_path);
							if (field_name)
								DEALLOCATE(field_name);
						}
						if (texture)
						{
							process = 1;
							return_code = shift_Parse_state(state, 1);
						}
						else
						{
							display_message(ERROR_MESSAGE, "Unknown texture : %s",
								current_token);
							display_parse_state_location(state);
							return_code = 0;
						}
					}
					else
					{
						if (NULL != (texture = CREATE(Texture)((char *)NULL)))
						{
							option_table = CREATE(Option_table)();
							Option_table_add_entry(option_table, "TEXTURE_NAME",
								(void *)texture, command_data_void, gfx_modify_Texture);
							return_code = Option_table_parse(option_table, state);
							/*???DB.  return_code will be 0 ? */
							DESTROY(Option_table)(&option_table);
							DESTROY(Texture)(&texture);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"gfx_modify_Texture.  Could not create dummy texture");
							return_code = 0;
						}
					}
				}
				if (process)
				{
#if defined (SGI_MOVIE_FILE)
					if (x3d_movie=Texture_get_movie(texture))
					{
						if (movie = FIRST_OBJECT_IN_MANAGER_THAT(Movie_graphics)(
							Movie_graphics_has_X3d_movie, (void *)x3d_movie,
							command_data->movie_graphics_manager))
						{
							ACCESS(Movie_graphics)(movie);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"gfx_modify_Texture.  Missing Movie_graphics for X3d_movie");
						}
					}
					else
					{
						movie = (struct Movie_graphics *)NULL;
					}
					old_movie = movie;
#endif /* defined (SGI_MOVIE_FILE) */
					Texture_get_combine_alpha(texture, &alpha);
					Texture_get_combine_colour(texture, &colour);
					double depth = 1.0, height = 1.0, width = 1.0;
					Texture_get_physical_size(texture, &width, &height, &depth);
					Texture_get_distortion_info(texture,
						&distortion_centre_x,&distortion_centre_y,&distortion_factor_k1);
					Texture_get_mipmap_level_of_detail_bias(texture, &mipmap_level_of_detail_bias);
					texture_tiling_enabled = Texture_get_texture_tiling_enabled(texture);
					texture_distortion[0]=(double)distortion_centre_x;
					texture_distortion[1]=(double)distortion_centre_y;
					texture_distortion[2]=(double)distortion_factor_k1;

					specify_format=TEXTURE_RGB;
					specify_width=0;
					specify_height=0;
					specify_depth=0;
					specify_number_of_bytes_per_component=0;

					image_data.image_file_name=(char *)NULL;
					image_data.crop_left_margin=0;
					image_data.crop_bottom_margin=0;
					image_data.crop_width=0;
					image_data.crop_height=0;

					Texture_evaluate_image_data evaluate_data;
					evaluate_data.region = cmzn_region_access(command_data->root_region);
					evaluate_data.group = (cmzn_field_group_id)0;
					evaluate_data.element_dimension = 0; /* dimension == number of texture coordinates components */
					evaluate_data.propagate_field = 1;
					evaluate_data.field = (struct Computed_field *)NULL;
					evaluate_data.texture_coordinates_field =
						(struct Computed_field *)NULL;
					evaluate_data.field_name = (char *)NULL;
					evaluate_data.texture_coordinates_field_name = (char *)NULL;
					/* Try for the special transparent gray material first */
					if (!(evaluate_data.fail_material =
					cmzn_materialmodule_find_material_by_name(command_data->materialmodule,
						"transparent_gray50")))
					{
						/* Just use the default material */
						evaluate_data.fail_material = cmzn_materialmodule_get_default_material(
							command_data->materialmodule);
					}
					evaluate_data.spectrum = (struct cmzn_spectrum *)NULL;

					file_number_pattern = (char *)NULL;
					/* increment must be non-zero for following to be "set" */
					file_number_series_data.start = 0;
					file_number_series_data.stop = 0;
					file_number_series_data.increment = 0;

					option_table = CREATE(Option_table)();
					/* alpha */
					Option_table_add_entry(option_table, "alpha", &alpha,
					  NULL,set_float_0_to_1_inclusive);
					/* blend/decal/modulate */
					combine_mode_string = ENUMERATOR_STRING(Texture_combine_mode)(
						Texture_get_combine_mode(texture));
					valid_strings = ENUMERATOR_GET_VALID_STRINGS(Texture_combine_mode)(
						&number_of_valid_strings,
						(ENUMERATOR_CONDITIONAL_FUNCTION(Texture_combine_mode) *)NULL,
						(void *)NULL);
					Option_table_add_enumerator(option_table, number_of_valid_strings,
						valid_strings, &combine_mode_string);
					DEALLOCATE(valid_strings);
					/* clamp_wrap/repeat_wrap */
					wrap_mode_string = ENUMERATOR_STRING(Texture_wrap_mode)(
						Texture_get_wrap_mode(texture));
					valid_strings = ENUMERATOR_GET_VALID_STRINGS(Texture_wrap_mode)(
						&number_of_valid_strings,
						(ENUMERATOR_CONDITIONAL_FUNCTION(Texture_wrap_mode) *)NULL,
						(void *)NULL);
					Option_table_add_enumerator(option_table,number_of_valid_strings,
						valid_strings,&wrap_mode_string);
					DEALLOCATE(valid_strings);
					/* colour */
					Option_table_add_entry(option_table, "colour", &colour,
					  NULL,set_Colour);
					/* compressed_unspecified/uncompressed */
					compression_mode_string = ENUMERATOR_STRING(Texture_compression_mode)(
						Texture_get_compression_mode(texture));
					valid_strings = ENUMERATOR_GET_VALID_STRINGS(Texture_compression_mode)(
						&number_of_valid_strings,
						(ENUMERATOR_CONDITIONAL_FUNCTION(Texture_compression_mode) *)NULL,
						(void *)NULL);
					Option_table_add_enumerator(option_table, number_of_valid_strings,
						valid_strings, &compression_mode_string);
					DEALLOCATE(valid_strings);
					/* depth */
					Option_table_add_non_negative_double_entry(option_table, "depth", &depth);
					/* distortion */
					Option_table_add_entry(option_table, "distortion",
						&texture_distortion,
					  &texture_distortion_data,set_double_vector_with_help);
					/* height */
					Option_table_add_positive_double_entry(option_table, "height", &height);
					/* image */
					Option_table_add_entry(option_table, "image",
						&image_data, command_data, gfx_modify_Texture_image);
					/* linear_filter/nearest_filter */
					filter_mode_string = ENUMERATOR_STRING(Texture_filter_mode)(
						Texture_get_filter_mode(texture));
					valid_strings = ENUMERATOR_GET_VALID_STRINGS(Texture_filter_mode)(
						&number_of_valid_strings,
						(ENUMERATOR_CONDITIONAL_FUNCTION(Texture_filter_mode) *)NULL,
						(void *)NULL);
					Option_table_add_enumerator(option_table,number_of_valid_strings,
						valid_strings,&filter_mode_string);
					DEALLOCATE(valid_strings);
#if defined (SGI_MOVIE_FILE)
					/* movie */
					Option_table_add_entry(option_table, "movie", &movie,
					  command_data->movie_graphics_manager, set_Movie_graphics);
#endif /* defined (SGI_MOVIE_FILE) */
					/* mipmap_level_of_detail_bias */
					mipmap_level_of_detail_bias_flt = mipmap_level_of_detail_bias;
					Option_table_add_float_entry(option_table, "mipmap_level_of_detail_bias",
						&mipmap_level_of_detail_bias_flt);
					/* number_pattern */
					Option_table_add_entry(option_table, "number_pattern",
						&file_number_pattern, (void *)1, set_name);
					/* number_series */
					Option_table_add_entry(option_table, "number_series",
						&file_number_series_data, NULL,
						gfx_modify_Texture_file_number_series);
					/* raw image storage mode */
					raw_image_storage_string =
						ENUMERATOR_STRING(Raw_image_storage)(RAW_PLANAR_RGB);
					valid_strings = ENUMERATOR_GET_VALID_STRINGS(Raw_image_storage)(
						&number_of_valid_strings,
						(ENUMERATOR_CONDITIONAL_FUNCTION(Raw_image_storage) *)NULL,
						(void *)NULL);
					Option_table_add_enumerator(option_table, number_of_valid_strings,
						valid_strings, &raw_image_storage_string);
					DEALLOCATE(valid_strings);
					/* resize_linear_filter/resize_nearest_filter */
					resize_filter_mode_string =
						ENUMERATOR_STRING(Texture_resize_filter_mode)(
							Texture_get_resize_filter_mode(texture));
					valid_strings =
						ENUMERATOR_GET_VALID_STRINGS(Texture_resize_filter_mode)(
							&number_of_valid_strings, (ENUMERATOR_CONDITIONAL_FUNCTION(
								Texture_resize_filter_mode) *)NULL, (void *)NULL);
					Option_table_add_enumerator(option_table,number_of_valid_strings,
						valid_strings,&resize_filter_mode_string);
					DEALLOCATE(valid_strings);
					/* specify_depth */
					Option_table_add_entry(option_table, "specify_depth",&specify_depth,
					  NULL,set_int_non_negative);
					/* specify_format */
					Option_table_add_entry(option_table, "specify_format", &specify_format,
						NULL, set_Texture_storage);
					/* specify_height */
					Option_table_add_entry(option_table, "specify_height",&specify_height,
					  NULL,set_int_non_negative);
					/* specify_number_of_bytes_per_component */
					Option_table_add_entry(option_table,
						"specify_number_of_bytes_per_component",
						&specify_number_of_bytes_per_component,NULL,set_int_non_negative);
					/* specify_width */
					Option_table_add_entry(option_table, "specify_width",&specify_width,
					  NULL,set_int_non_negative);
					/* texture_tiling */
					Option_table_add_char_flag_entry(option_table,
						"texture_tiling", &texture_tiling_enabled);
					/* no_texture_tiling */
					Option_table_add_unset_char_flag_entry(option_table,
						"no_texture_tiling", &texture_tiling_enabled);
					/* width */
					Option_table_add_positive_double_entry(option_table, "width", &width);
					/* evaluate_image */
					Option_table_add_entry(option_table, "evaluate_image",
					  &evaluate_data, command_data, gfx_modify_Texture_evaluate_image);
					return_code=Option_table_multi_parse(option_table, state);
					if (return_code)
					{
						if (evaluate_data.field_name || evaluate_data.group ||
							evaluate_data.spectrum || evaluate_data.texture_coordinates_field_name)
						{
							if (evaluate_data.field_name && evaluate_data.texture_coordinates_field_name &&
								evaluate_data.spectrum)
							{
								cmzn_fieldmodule_id field_module = cmzn_region_get_fieldmodule(evaluate_data.region);
								evaluate_data.field = cmzn_fieldmodule_find_field_by_name(field_module,
									evaluate_data.field_name);
								evaluate_data.texture_coordinates_field = cmzn_fieldmodule_find_field_by_name(field_module,
									evaluate_data.texture_coordinates_field_name);
								cmzn_fieldmodule_destroy(&field_module);
								if (!evaluate_data.field && !evaluate_data.texture_coordinates_field)
								{
									return_code = 0;
									display_message(ERROR_MESSAGE, "Specified field cannot be found");
								}
							}
							else
							{
								return_code = 0;
							}
							if (!return_code)
							{
								display_message(ERROR_MESSAGE,
									"To evaluate the texture image from a field you must specify\n"
									"a field, element_group (region and optional group), spectrum and texture_coordinates");
								return_code = 0;
							}
						}
					}
					if (return_code)
					{
						if (texture_is_managed)
						{
							MANAGER_BEGIN_CACHE(Computed_field)(field_manager);
							MANAGED_OBJECT_CHANGE(Computed_field)(image_field,
								MANAGER_CHANGE_OBJECT_NOT_IDENTIFIER(Computed_field));
						}
						/* must change filter modes etc. before reading new images since
							 some of them will apply immediately to the new images */
						Texture_set_combine_alpha(texture, alpha);
						Texture_set_combine_colour(texture, &colour);
						if (depth == 0.0)
						{
							display_message(WARNING_MESSAGE, "gfx modify texture:  Changing invalid depth 0 to 1.0");
							depth = 1.0;
						}
						Texture_set_physical_size(texture, width, height, depth);
						Texture_set_texture_tiling_enabled(texture, texture_tiling_enabled);
						Texture_set_mipmap_level_of_detail_bias(texture, mipmap_level_of_detail_bias);

						STRING_TO_ENUMERATOR(Texture_combine_mode)(
							combine_mode_string, &combine_mode);
						Texture_set_combine_mode(texture, combine_mode);

						STRING_TO_ENUMERATOR(Texture_compression_mode)(
							compression_mode_string, &compression_mode);
						Texture_set_compression_mode(texture, compression_mode);

						STRING_TO_ENUMERATOR(Texture_filter_mode)(
							filter_mode_string, &filter_mode);
						Texture_set_filter_mode(texture, filter_mode);

						STRING_TO_ENUMERATOR(Texture_resize_filter_mode)(
							resize_filter_mode_string, &resize_filter_mode);
						Texture_set_resize_filter_mode(texture,
							resize_filter_mode);

						STRING_TO_ENUMERATOR(Texture_wrap_mode)(
							wrap_mode_string, &wrap_mode);
						Texture_set_wrap_mode(texture, wrap_mode);

						if (texture_distortion_data.set)
						{
							distortion_centre_x=(float)texture_distortion[0];
							distortion_centre_y=(float)texture_distortion[1];
							distortion_factor_k1=(float)texture_distortion[2];
							Texture_set_distortion_info(texture,
								distortion_centre_x,distortion_centre_y,distortion_factor_k1);
						}

						if (image_data.image_file_name)
						{
							cmgui_image_information = CREATE(Cmgui_image_information)();
							/* specify file name(s) */
							if (0 != file_number_series_data.increment)
							{
								if (strstr(image_data.image_file_name, file_number_pattern))
								{
									Cmgui_image_information_set_file_name_series(
										cmgui_image_information,
										/*file_name_template*/image_data.image_file_name,
										file_number_pattern,
										file_number_series_data.start,
										file_number_series_data.start,
										/*increment*/1);
								}
								else
								{
									display_message(ERROR_MESSAGE, "gfx modify texture:  "
										"File number pattern \"%s\" not found in file name \"%s\"",
										file_number_pattern, image_data.image_file_name);
									return_code = 0;
								}
							}
							else
							{
								Cmgui_image_information_add_file_name(cmgui_image_information,
									image_data.image_file_name);
							}
							/* specify width and height and raw_image_storage */
							Cmgui_image_information_set_width(cmgui_image_information,
								specify_width);
							Cmgui_image_information_set_height(cmgui_image_information,
								specify_height);
							Cmgui_image_information_set_io_stream_package(cmgui_image_information,
								command_data->io_stream_package);
							STRING_TO_ENUMERATOR(Raw_image_storage)(
								raw_image_storage_string, &raw_image_storage);
							Cmgui_image_information_set_raw_image_storage(
								cmgui_image_information, raw_image_storage);
							switch (specify_format)
							{
								case TEXTURE_LUMINANCE:
								{
									Cmgui_image_information_set_number_of_components(
										cmgui_image_information, 1);
								} break;
								case TEXTURE_LUMINANCE_ALPHA:
								{
									Cmgui_image_information_set_number_of_components(
										cmgui_image_information, 2);
								} break;
								case TEXTURE_RGB:
								case TEXTURE_BGR:
								{
									Cmgui_image_information_set_number_of_components(
										cmgui_image_information, 3);
								} break;
								case TEXTURE_RGBA:
								{
									Cmgui_image_information_set_number_of_components(
										cmgui_image_information, 4);
								} break;
								case TEXTURE_ABGR:
								{
									Cmgui_image_information_set_number_of_components(
										cmgui_image_information, 4);
								} break;
								default:
								{
									display_message(ERROR_MESSAGE,
										"gfx modify texture:  Invalid value for specify_format");
									return_code = 0;
								} break;
							}
							if (specify_number_of_bytes_per_component)
							{
								Cmgui_image_information_set_number_of_bytes_per_component(
									cmgui_image_information, specify_number_of_bytes_per_component);
							}
							if (return_code)
							{
								Image_file_format image_file_format = UNKNOWN_IMAGE_FILE_FORMAT;
								Image_file_format_from_file_name(image_data.image_file_name, &image_file_format);
								cmgui_image = 0;
								if (image_file_format == ANALYZE_FILE_FORMAT)
								{
									cmgui_image = Cmgui_image_read_analyze(cmgui_image_information,
										CMZN_STREAMINFORMATION_DATA_COMPRESSION_TYPE_DEFAULT);
								}
								else if (image_file_format == ANALYZE_OBJECT_MAP_FORMAT)
								{
									cmgui_image = Cmgui_image_read_analyze_object_map(cmgui_image_information,
										CMZN_STREAMINFORMATION_DATA_COMPRESSION_TYPE_DEFAULT);
								}
								else
								{
									cmgui_image = Cmgui_image_read(cmgui_image_information);
								}
								if (cmgui_image != 0)
								{
									char *property, *value;

									return_code = Texture_set_image(texture, cmgui_image,
										image_data.image_file_name, file_number_pattern,
										file_number_series_data.start,
										file_number_series_data.stop,
										file_number_series_data.increment,
										image_data.crop_left_margin, image_data.crop_bottom_margin,
										image_data.crop_width, image_data.crop_height);
									/* Delete any existing properties as we are modifying */
									Texture_clear_all_properties(texture);
									/* Calling get_proprety with wildcard ensures they
										will be available to the iterator, as well as
										any other properties */
									Cmgui_image_get_property(cmgui_image,"exif:*");
									Cmgui_image_reset_property_iterator(cmgui_image);
									while ((property = Cmgui_image_get_next_property(
										cmgui_image)) &&
										(value = Cmgui_image_get_property(cmgui_image,
										property)))
									{
										Texture_set_property(texture, property, value);
										DEALLOCATE(property);
										DEALLOCATE(value);
									}
									DESTROY(Cmgui_image)(&cmgui_image);
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"gfx modify texture:  Could not read image file");
									return_code = 0;
								}
								if (return_code && (0 != file_number_series_data.increment))
								{
									number_of_file_names = 1 + (file_number_series_data.stop -
										file_number_series_data.start) /
										file_number_series_data.increment;
									file_number = file_number_series_data.start +
										file_number_series_data.increment;
									for (i = 1 ; return_code && (i < number_of_file_names) ; i++)
									{
										Cmgui_image_information_set_file_name_series(
											cmgui_image_information,
											/*file_name_template*/image_data.image_file_name,
											file_number_pattern, /*start*/file_number,
											/*end*/file_number, /*increment*/1);
										if (NULL != (cmgui_image = Cmgui_image_read(cmgui_image_information)))
										{
											return_code = Texture_add_image(texture, cmgui_image,
												image_data.crop_left_margin, image_data.crop_bottom_margin,
												image_data.crop_width, image_data.crop_height);
											DESTROY(Cmgui_image)(&cmgui_image);
										}
										else
										{
											display_message(ERROR_MESSAGE,
												"gfx modify texture:  Could not read image file");
											return_code = 0;
										}
										file_number += file_number_series_data.increment;
									}
								}
								if (! return_code)
								{
									/* Set a NULL image into texture so that an incomplete set isn't displayed */
									Texture_allocate_image(texture, /*image_width*/1, /*image_height*/1,
										/*image_depth*/1, TEXTURE_RGB, /*number_of_bytes_per_component*/1,
										"INCOMPLETEDTEXTURE");
									display_message(ERROR_MESSAGE,  "gfx modify texture:  "
										"Unable to read images into texture, setting it to black.");
								}
							}
							DESTROY(Cmgui_image_information)(&cmgui_image_information);
						}
#if defined (SGI_MOVIE_FILE)
						if ( movie != old_movie )
						{
							/* Movie is outside manager copy so that is updates
								the correct texture based on movie events */
							Texture_set_movie(texture,
								Movie_graphics_get_X3d_movie(movie),
								command_data->graphics_buffer_package, "movie");
						}
#endif /* defined (SGI_MOVIE_FILE) */

						if (evaluate_data.field && evaluate_data.spectrum &&
							evaluate_data.texture_coordinates_field)
						{
							if (Computed_field_depends_on_texture(evaluate_data.field,
									texture))
							{
								texture_copy = CREATE(Texture)("temporary");
								Texture_copy_without_identifier(texture, texture_copy);
							}
							else
							{
								texture_copy = texture;
							}

							cmzn_fieldmodule_id field_module = cmzn_region_get_fieldmodule(evaluate_data.region);
							int element_dimension = evaluate_data.element_dimension;
							if (element_dimension == 0)
								element_dimension = cmzn_field_get_number_of_components(evaluate_data.texture_coordinates_field);
							const int highest_dimension = FE_region_get_highest_dimension(cmzn_region_get_FE_region(evaluate_data.region));
							if (element_dimension > highest_dimension)
								element_dimension = highest_dimension;
							cmzn_mesh_id search_mesh = cmzn_fieldmodule_find_mesh_by_dimension(field_module, element_dimension);
							if (evaluate_data.group)
							{
								cmzn_field_element_group_id element_group =
									cmzn_field_group_get_field_element_group(evaluate_data.group, search_mesh);
								cmzn_mesh_destroy(&search_mesh);
								if (element_group)
								{
									search_mesh = cmzn_mesh_group_base_cast(cmzn_field_element_group_get_mesh_group(element_group));
									cmzn_field_element_group_destroy(&element_group);
								}
							}
							set_Texture_image_from_field(texture_copy,
								evaluate_data.field,
								evaluate_data.texture_coordinates_field,
								evaluate_data.propagate_field,
								evaluate_data.spectrum,
								search_mesh,
								specify_format, specify_width,
								specify_height, specify_depth,
								specify_number_of_bytes_per_component,
								command_data->graphics_buffer_package,
								evaluate_data.fail_material);

							if (texture_copy != texture)
							{
								Texture_copy_without_identifier(texture_copy, texture);
								DESTROY(Texture)(&texture_copy);
							}
							cmzn_mesh_destroy(&search_mesh);
							cmzn_fieldmodule_destroy(&field_module);
						}
						if (texture_is_managed)
						{
							if (image && texture)
								cmzn_field_image_set_texture(image, texture);
							MANAGER_END_CACHE(Computed_field)(field_manager);
						}
					}
					if (image_data.image_file_name)
					{
						DEALLOCATE(image_data.image_file_name);
					}
					DESTROY(Option_table)(&option_table);
#if defined (SGI_MOVIE_FILE)
					if (movie)
					{
						DEACCESS(Movie_graphics)(&movie);
					}
#endif /* defined (SGI_MOVIE_FILE) */
					if (evaluate_data.region)
						cmzn_region_destroy(&evaluate_data.region);
					if (evaluate_data.group)
						cmzn_field_group_destroy(&evaluate_data.group);
					if (evaluate_data.fail_material)
					{
						cmzn_material_destroy(&evaluate_data.fail_material);
					}
					if (evaluate_data.spectrum)
					{
						DEACCESS(cmzn_spectrum)(&evaluate_data.spectrum);
					}
					if (evaluate_data.field)
					{
						DEACCESS(Computed_field)(&evaluate_data.field);
					}
					if (evaluate_data.texture_coordinates_field)
					{
						DEACCESS(Computed_field)(&evaluate_data.texture_coordinates_field);
					}
					if (evaluate_data.field_name)
					{
						DEALLOCATE(evaluate_data.field_name);
					}
					if (evaluate_data.texture_coordinates_field_name)
					{
						DEALLOCATE(evaluate_data.texture_coordinates_field_name);
					}

					DEALLOCATE(file_number_pattern);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_modify_Texture.  Missing command_data_void");
				return_code=0;
			}
		}
		else
		{
			if (texture_void)
			{
				display_message(WARNING_MESSAGE,"Missing texture modifications");
			}
			else
			{
				display_message(WARNING_MESSAGE,"Missing texture name");
			}
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_modify_Texture.  Missing state");
		return_code=0;
	}
	if (image_field)
		cmzn_field_destroy(&image_field);
	LEAVE;

	return (return_code);
} /* gfx_modify_Texture */

static int gfx_create_texture(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/******************************************************************************
LAST MODIFIED : 14 June 1999

DESCRIPTION :
Executes a GFX CREATE TEXTURE command.
==============================================================================*/
{
	const char *current_token;
	int return_code = 0;
	struct cmzn_command_data *command_data;
	struct Texture *texture;

	ENTER(gfx_create_texture);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (NULL != (current_token = state->current_token))
		{
			if (NULL != (command_data = (struct cmzn_command_data *)command_data_void))
			{
				if (strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
				{
					struct cmzn_region *region = NULL;
					char *region_path, *field_name;
					if (cmzn_region_get_partial_region_path(command_data->root_region,
						current_token, &region, &region_path, &field_name))
					{
						cmzn_fieldmodule *field_module = cmzn_region_get_fieldmodule(region);
						if (field_name && (strlen(field_name) > 0) &&
							(strchr(field_name, CMZN_REGION_PATH_SEPARATOR_CHAR)	== NULL))
						{
							Computed_field *existing_field =
								FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field,name)(
									field_name, cmzn_region_get_Computed_field_manager(region));
							if (existing_field)
							{
								display_message(ERROR_MESSAGE,
									"gfx_create_texture: Field already exists");
							}
							else
							{
								texture=CREATE(Texture)(field_name);
								if (texture)
								{
									shift_Parse_state(state,1);
									if (state->current_token)
									{
										return_code=gfx_modify_Texture(state,(void *)texture,
											command_data_void);
									}
									else
									{
										return_code=1;
									}
									if (return_code)
									{
										cmzn_fieldmodule_begin_change(field_module);
										cmzn_field_id image_field =	cmzn_fieldmodule_create_field_image(field_module);
										cmzn_field_set_name(image_field, field_name);
										cmzn_field_set_managed(image_field, true);
										cmzn_field_image_id image = cmzn_field_cast_image(image_field);
										cmzn_field_image_set_texture(image, texture);
										cmzn_field_image_destroy(&image);
										cmzn_field_destroy(&image_field);
										cmzn_fieldmodule_end_change(field_module);
									}
									else
									{
										DESTROY(Texture)(&texture);
									}
								}
							}
						}
						else
						{
							if (field_name)
							{
								display_message(ERROR_MESSAGE,
									"gfx_create_texture:  Invalid region path or texture field name '%s'", field_name);
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"gfx_create_texture:  Missing texture field name or name matches child region '%s'", current_token);
							}
							display_parse_state_location(state);
							return_code = 0;
						}
						cmzn_fieldmodule_destroy(&field_module);
						if (region_path)
							DEALLOCATE(region_path);
						if (field_name)
							DEALLOCATE(field_name);
					}
				}
				else
				{
					return_code=gfx_modify_Texture(state,(void *)NULL, command_data_void);
					return_code=1;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_create_texture.  Missing command_data_void");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Missing texture name");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_create_texture.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /*gfx_create_texture */

#if defined (WX_USER_INTERFACE)
static int gfx_create_time_editor(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 15 June 1999

DESCRIPTION :
Executes a GFX CREATE TIME_EDITOR command.
If there is a time editor dialog in existence, then bring it to the front,
otherwise it creates a new one.  Assumes we will only ever want one time
editor at a time.  This implementation may be changed later.
==============================================================================*/
{
	const char *current_token;
	int return_code = 0;
	struct cmzn_command_data *command_data;

	ENTER(gfx_create_graphical_time_editor);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (NULL != (current_token = state->current_token))
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				display_message(ERROR_MESSAGE,"Unknown option: %s",current_token);
				display_parse_state_location(state);
				return_code=0;
			}
			else
			{
				return_code=1;
			}
		}
		else
		{
			if (NULL != (command_data = (struct cmzn_command_data *)command_data_void))
			{
				if (command_data->graphics_window_manager)
				{
					 return_code = FOR_EACH_OBJECT_IN_MANAGER(Graphics_window)(
							Graphics_window_bring_up_time_editor_wx,(void *)NULL,
							command_data->graphics_window_manager);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_create_graphical_time_editor.  Missing command_data");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_create_graphical_time_editor.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_graphical_time_editor */
#endif /* defined (WX_USER_INTERFACE) */

#if defined (USE_CMGUI_GRAPHICS_WINDOW)
static int gfx_create_window(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 16 June 1999

DESCRIPTION :
Executes a GFX CREATE WINDOW command.
==============================================================================*/
{
	char any_buffering_mode_flag, any_stereo_mode_flag, double_buffer_flag,
		*name,mono_buffer_flag,single_buffer_flag,stereo_buffer_flag;
	enum Graphics_window_buffering_mode buffer_mode;
	enum Graphics_window_stereo_mode stereo_mode;
	int minimum_colour_buffer_depth, minimum_depth_buffer_depth,
		minimum_accumulation_buffer_depth, return_code;
	struct cmzn_command_data *command_data;
	struct Graphics_window *window;
	struct Option_table *buffer_option_table, *option_table, *stereo_option_table

	ENTER(gfx_create_window);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (NULL != (command_data = (struct cmzn_command_data *)command_data_void))
		{
			return_code=1;
			/* set_defaults */
			name=Graphics_window_manager_get_new_name(
				command_data->graphics_window_manager);
			buffer_mode = GRAPHICS_WINDOW_DOUBLE_BUFFERING;
			stereo_mode = GRAPHICS_WINDOW_ANY_STEREO_MODE;
			minimum_depth_buffer_depth=8;
			minimum_accumulation_buffer_depth=8;
			minimum_colour_buffer_depth = 8;
			if (state->current_token)
			{
				/* change defaults */
				any_buffering_mode_flag=0;
				single_buffer_flag=0;
				double_buffer_flag=0;
				any_stereo_mode_flag=0;
				mono_buffer_flag=0;
				stereo_buffer_flag=0;

				option_table = CREATE(Option_table)();
				/* accumulation_buffer_depth */
				Option_table_add_entry(option_table, "accumulation_buffer_depth",
					&minimum_accumulation_buffer_depth, NULL, set_int_non_negative);
				/* any_buffer_mode/double_buffer/single_buffer */
				buffer_option_table=CREATE(Option_table)();
				Option_table_add_entry(buffer_option_table,"any_buffer_mode",
					&any_buffering_mode_flag,(void *)NULL,set_char_flag);
				Option_table_add_entry(buffer_option_table,"double_buffer",
					&double_buffer_flag,(void *)NULL,set_char_flag);
				Option_table_add_entry(buffer_option_table,"single_buffer",
					&single_buffer_flag,(void *)NULL,set_char_flag);
				Option_table_add_suboption_table(option_table, buffer_option_table);
				/* any_stereo_mode/mono_buffer/stereo_buffer */
				stereo_option_table=CREATE(Option_table)();
				Option_table_add_entry(stereo_option_table,"any_stereo_mode",
					&any_stereo_mode_flag,(void *)NULL,set_char_flag);
				Option_table_add_entry(stereo_option_table,"mono_buffer",
					&mono_buffer_flag,(void *)NULL,set_char_flag);
				Option_table_add_entry(stereo_option_table,"stereo_buffer",
					&stereo_buffer_flag,(void *)NULL,set_char_flag);
				Option_table_add_suboption_table(option_table, stereo_option_table);
				/* colour_buffer_depth */
				Option_table_add_entry(option_table, "colour_buffer_depth",
					&minimum_colour_buffer_depth, NULL, set_int_non_negative);
				/* depth_buffer_depth */
				Option_table_add_entry(option_table, "depth_buffer_depth",
					&minimum_depth_buffer_depth, NULL, set_int_non_negative);
				/* name */
				Option_table_add_entry(option_table,"name",&name,(void *)1,set_name);
				/* default */
				Option_table_add_entry(option_table,(char *)NULL,&name,(void *)NULL,
					set_name);
				return_code = Option_table_multi_parse(option_table,state);
				DESTROY(Option_table)(&option_table);
				if (return_code)
				{
					if (any_buffering_mode_flag + single_buffer_flag + double_buffer_flag > 1)
					{
						display_message(ERROR_MESSAGE,
							"Only one of any_buffer_mode/single_buffer/double_buffer");
						return_code=0;
					}
					if (any_stereo_mode_flag + mono_buffer_flag + stereo_buffer_flag > 1)
					{
						display_message(ERROR_MESSAGE,
							"Only one of any_stereo_mode/mono_buffer/stereo_buffer");
						return_code=0;
					}
				}
				if (return_code)
				{
					if (any_buffering_mode_flag)
					{
						buffer_mode = GRAPHICS_WINDOW_ANY_BUFFERING_MODE;
					}
					else if (single_buffer_flag)
					{
						buffer_mode = GRAPHICS_WINDOW_SINGLE_BUFFERING;
					}
					else if (double_buffer_flag)
					{
						buffer_mode = GRAPHICS_WINDOW_DOUBLE_BUFFERING;
					}
					if (any_stereo_mode_flag)
					{
						stereo_mode = GRAPHICS_WINDOW_ANY_STEREO_MODE;
					}
					else if (stereo_buffer_flag)
					{
						stereo_mode = GRAPHICS_WINDOW_STEREO;
					}
					else if (mono_buffer_flag)
					{
						stereo_mode = GRAPHICS_WINDOW_MONO;
					}
				}
			}
			if (!name)
			{
				display_message(ERROR_MESSAGE,"gfx_create_window.  Missing name");
				return_code=0;
			}
			if (return_code)
			{
				if (NULL != (window=FIND_BY_IDENTIFIER_IN_MANAGER(Graphics_window,name)(name,
					command_data->graphics_window_manager)))
				{
					display_message(WARNING_MESSAGE,
						"Graphics window '%s' already exists",name);
					return_code=0;
				}
				else
				{
					if (command_data->user_interface)
					{
#if defined (WX_USER_INTERFACE)
						struct MANAGER(Interactive_tool) *interactive_tool_manager;
						struct Interactive_tool *transform_tool;
						interactive_tool_manager = CREATE(MANAGER(Interactive_tool))();
						transform_tool=create_Interactive_tool_transform(
							 command_data->user_interface);
						ADD_OBJECT_TO_MANAGER(Interactive_tool)(transform_tool,
							 interactive_tool_manager);
						cmzn_material_id defaultMaterial =
							cmzn_materialmodule_get_default_material(command_data->materialmodule);
						Node_tool_set_execute_command(CREATE(Node_tool)(
								interactive_tool_manager,
								command_data->root_region, CMZN_FIELD_DOMAIN_TYPE_NODES,
								defaultMaterial,
								command_data->user_interface,
								command_data->default_time_keeper_app),
								command_data->execute_command);
						Node_tool_set_execute_command(CREATE(Node_tool)(
								interactive_tool_manager,
								command_data->root_region, CMZN_FIELD_DOMAIN_TYPE_DATAPOINTS,
								defaultMaterial,
								command_data->user_interface,
								command_data->default_time_keeper_app),
								command_data->execute_command);
						Element_tool_set_execute_command(CREATE(Element_tool)(
								interactive_tool_manager,
								command_data->root_region,
								command_data->element_point_ranges_selection,
								defaultMaterial,
								command_data->user_interface,
								command_data->default_time_keeper_app),
								command_data->execute_command);
#if defined (USE_OPENCASCADE)
						Cad_tool_set_execute_command(CREATE(Cad_tool)(
								interactive_tool_manager,
								command_data->root_region,
								command_data->element_point_ranges_selection,
								defaultMaterial,
								command_data->user_interface,
								command_data->default_time_keeper_app),
								command_data->execute_command);
#endif /* defined (USE_OPENCASCADE) */
						Element_point_tool_set_execute_command(CREATE(Element_point_tool)(
								interactive_tool_manager,
								command_data->root_region,
								command_data->element_point_ranges_selection,
								defaultMaterial,
								command_data->user_interface,
								command_data->default_time_keeper_app),
								command_data->execute_command);
						if (defaultMaterial)
							cmzn_material_destroy(&defaultMaterial);

						if (NULL != (window=CREATE(Graphics_window)(name,buffer_mode,stereo_mode,
							minimum_colour_buffer_depth, minimum_depth_buffer_depth,
							minimum_accumulation_buffer_depth,
							command_data->graphics_buffer_package,
							command_data->filter_module,command_data->default_scene,
							interactive_tool_manager,
							command_data->default_time_keeper_app,
							command_data->user_interface, command_data->root_region,
							command_data->sceneviewermodule->core_sceneviewermodule)))
						{
							if (!ADD_OBJECT_TO_MANAGER(Graphics_window)(window,
							   command_data->graphics_window_manager))
							{
								DESTROY(MANAGER(Interactive_tool))(&interactive_tool_manager);
							   return_code=0;
							}
						   DEACCESS(Graphics_window)(&window);
						}
						else
					   {
						  display_message(ERROR_MESSAGE,
							 "gfx_create_window.  Could not create graphics window");
						  return_code=0;
						}
#else
						window=CREATE(Graphics_window)(name,buffer_mode,stereo_mode,
							minimum_colour_buffer_depth, minimum_depth_buffer_depth,
							minimum_accumulation_buffer_depth,
							command_data->graphics_buffer_package,
							&(command_data->background_colour),
							command_data->lightmodule,command_data->default_light,
							command_data->light_model_module,command_data->default_light_model,
							command_data->scene_manager,command_data->default_scene,
							command_data->interactive_tool_manager,
							command_data->default_time_keeper_app,
							command_data->user_interface, command_data->root_region,
							command_data->sceneviewermodule->core_sceneviewermodule);
					  if (window)
						{
						   if (!ADD_OBJECT_TO_MANAGER(Graphics_window)(window,
							   command_data->graphics_window_manager))
							{
							   return_code=0;
							}
						   DEACCESS(Graphics_window)(&window);
						}
						else
					   {
						  display_message(ERROR_MESSAGE,
							 "gfx_create_window.  Could not create graphics window");
						  return_code=0;
						}
#endif /*(WX_USER_INTERFACE)*/
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"gfx_create_window.  Cannot create a graphics window without a display.");
						return_code=0;
					}
				}
			}
			if (name)
			{
				DEALLOCATE(name);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_create_window.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_create_window.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_window */
#endif /* defined (GTK_USER_INTERFACE)  || defined (WIN32_USER_INTERFACE) || defined (WX_USER_INTERFACE) */

#if defined (SELECT_DESCRIPTORS)
static int execute_command_attach(struct Parse_state *state,
	void *prompt_void,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 11 May 2001

DESCRIPTION :
Executes an ATTACH command.
==============================================================================*/
{
	const char *current_token;
	char end_detection, *perl_action, start_detection;
	int return_code = 0;
	struct Io_device *device;
	static struct Option_table *option_table;
	struct cmzn_command_data *command_data;

	ENTER(execute_command_attach);
	USE_PARAMETER(prompt_void);
	/* check argument */
	if (state && (command_data=(struct cmzn_command_data *)command_data_void))
	{
		device = (struct Io_device *)NULL;
		if (NULL != (current_token = state->current_token))
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				if (command_data->device_list)
				{
					if (NULL != (device=FIND_BY_IDENTIFIER_IN_LIST(Io_device, name)
						(current_token,command_data->device_list)))
					{
						return_code=shift_Parse_state(state,1);
					}
					else
					{
						if (NULL != (device = CREATE(Io_device)(current_token)))
						{
							if (ADD_OBJECT_TO_LIST(Io_device)(device, command_data->device_list))
							{
								return_code=shift_Parse_state(state,1);
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"execute_command_attach.  Unable to create device struture.");
							return_code=0;
						}
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"execute_command_attach.  Missing device list");
					return_code=0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE,
					" DEVICE_NAME");
				return_code = 1;
				/* By not shifting the parse state the rest of the help should come out */
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"execute_command_attach.  Missing device name");
			return_code=0;
		}
		if (return_code)
		{
			end_detection = 0;
			perl_action = (char *)NULL;
			start_detection = 0;

			option_table = CREATE(Option_table)();
			Option_table_add_entry(option_table,"end_detection", &end_detection,
				NULL, set_char_flag);
			Option_table_add_entry(option_table,"perl_action", &perl_action, (void *)1,
				set_name);
			Option_table_add_entry(option_table,"start_detection", &start_detection,
				NULL, set_char_flag);
			return_code = Option_table_multi_parse(option_table,state);
			DESTROY(Option_table)(&option_table);
			if (return_code)
			{
				if (start_detection && end_detection)
				{
					display_message(ERROR_MESSAGE,"execute_command_attach.  "
						"Specify only one of start_detection and end_detection.");
					return_code=0;
				}
			}
			if (return_code)
			{
				if (start_detection)
				{
					Io_device_start_detection(device, command_data->user_interface);
				}
				if (end_detection)
				{
					Io_device_end_detection(device);
				}
#if defined (USE_PERL_INTERPRETER)
				if (perl_action)
				{
					Io_device_set_perl_action(device, command_data->interpreter,
						perl_action);
				}
#endif /* defined (USE_PERL_INTERPRETER) */
			}
			if (perl_action)
			{
				DEALLOCATE(perl_action);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_command_attach.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_attach */

static int execute_command_detach(struct Parse_state *state,
	void *prompt_void,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 11 May 2001

DESCRIPTION :
Executes a DETACH command.
==============================================================================*/
{
	const char *current_token;
	int return_code;
	struct Io_device *device;
	struct cmzn_command_data *command_data;

	ENTER(execute_command_detach);
	USE_PARAMETER(prompt_void);
	/* check argument */
	if (state && (command_data=(struct cmzn_command_data *)command_data_void))
	{
		device = (struct Io_device *)NULL;
		if (NULL != (current_token = state->current_token))
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				if (command_data->device_list)
				{
					if (NULL != (device=FIND_BY_IDENTIFIER_IN_LIST(Io_device, name)
						(current_token,command_data->device_list)))
					{
						if (REMOVE_OBJECT_FROM_LIST(Io_device)(device,
							command_data->device_list))
						{
							if (DESTROY(Io_device)(&device))
							{
								return_code = 1;
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"execute_command_detach.  Unable to destroy device %s.",
									current_token);
								return_code=0;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"execute_command_detach.  Unable to remove device %s.",
								current_token);
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"execute_command_detach.  Io_device %s not found.", current_token);
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"execute_command_detach.  Missing device list");
					return_code=0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE,
					" DEVICE_NAME");
				return_code = 1;
				/* By not shifting the parse state the rest of the help should come out */
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"execute_command_detach.  Missing device name");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_command_detach.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_detach */
#endif /* defined (SELECT_DESCRIPTORS) */

static int gfx_convert_elements(struct Parse_state *state,
	void *dummy_to_be_modified, void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 5 April 2006

DESCRIPTION :
Executes a GFX CONVERT ELEMENTS command.
==============================================================================*/
{
	int i, number_of_fields, previous_state_index, return_code;
	struct cmzn_command_data *command_data;
	struct Computed_field **fields;
	struct Option_table *option_table;
	char **component_names;

	ENTER(gfx_convert_elements);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (NULL != (command_data = (struct cmzn_command_data *)command_data_void))
		{
			cmzn_region *source_region = cmzn_region_access(command_data->root_region);
			cmzn_region *destination_region = NULL;
			fields = (struct Computed_field **)NULL;
			component_names = NULL;
			number_of_fields = 1;
			Convert_finite_elements_mode conversion_mode = CONVERT_TO_FINITE_ELEMENTS_MODE_UNSPECIFIED;
			double tolerance = 1.0E-6;
			Element_discretization element_refinement = { 1, 1, 1 };

			if ((state->current_token) &&
				strcmp(PARSER_HELP_STRING,state->current_token) &&
				strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token))
			{
				/* Skip this preprocessing if we are just getting the help */
				number_of_fields = 1;
				previous_state_index = state->current_index;

				option_table = CREATE(Option_table)();
				/* number_of_fields */
				Option_table_add_entry(option_table, "number_of_fields",
					&number_of_fields, NULL, set_int_positive);
				/* absorb everything else */
				Option_table_ignore_all_unmatched_entries(option_table);
				return_code = Option_table_multi_parse(option_table, state);
				DESTROY(Option_table)(&option_table);
				/* Return back to where we were */
				shift_Parse_state(state, previous_state_index - state->current_index);
			}

			if (number_of_fields)
			{
				ALLOCATE(fields, struct Computed_field *, number_of_fields);
				ALLOCATE(component_names, char *, number_of_fields);
				for (i = 0; i < number_of_fields; i++)
				{
					fields[i] = (struct Computed_field *)NULL;
					component_names[i] = NULL;
				}
			}

			option_table=CREATE(Option_table)();
			Option_table_add_help(option_table,
				"Convert element fields to specified bases on a new mesh in destination_region. "
				"Modes \'convert_trilinear\' and \'convert_triquadratic\' ONLY: "
				"converts element fields on 3-D elements to specified basis. "
				"The first field specified must be a 3-component coordinate field; "
				"nodes with values of this field within the specified "
				"tolerance are merged on the resulting mesh. "
				"Note: field value versions of non-coordinate fields are not handled"
				" - the first processed element's versions are assumed. "
				"Mode \'convert_hermite_2D_product_elements\' ONLY: converts element fields on 2-D elements "
				"into bicubic hermite basis WITHOUT merging nearby nodes.");
			Option_table_add_set_cmzn_region(option_table, "destination_region",
				command_data->root_region, &destination_region);
			Option_table_add_entry(option_table,"fields",component_names,
				&number_of_fields, set_names);
			Option_table_add_entry(option_table, "number_of_fields",
				&number_of_fields, NULL, set_int_positive);
			OPTION_TABLE_ADD_ENUMERATOR(Convert_finite_elements_mode)(option_table,
				&conversion_mode);
			Option_table_add_entry(option_table, "refinement",
				(void *)&element_refinement, (void *)NULL, set_Element_discretization);
			Option_table_add_set_cmzn_region(option_table, "source_region",
				command_data->root_region, &source_region);
			Option_table_add_non_negative_double_entry(option_table, "tolerance", &tolerance);
			return_code=Option_table_multi_parse(option_table,state);
			DESTROY(Option_table)(&option_table);

			if (return_code)
			{
				if (conversion_mode == CONVERT_TO_FINITE_ELEMENTS_MODE_UNSPECIFIED)
				{
					display_message(ERROR_MESSAGE,
						"gfx_convert elements.  Must specify a conversion mode");
					return_code = 0;
				}
				if ((NULL == component_names) || (NULL == component_names[0]))
				{
					display_message(ERROR_MESSAGE,
						"gfx_convert elements.  Must specify which fields to convert");
					return_code = 0;
				}
				if (NULL == destination_region)
				{
					display_message(ERROR_MESSAGE,
						"gfx_convert elements.  Must specify destination region");
					return_code = 0;
				}
				else if (cmzn_region_get_Computed_field_manager(destination_region) ==
					cmzn_region_get_Computed_field_manager(source_region))
				{
					display_message(ERROR_MESSAGE,
						"gfx_convert elements.  Destination and source regions must be different and not share fields");
					return_code = 0;
				}
			}

			if (return_code)
			{
				if (component_names && fields)
				{
					cmzn_fieldmodule_id field_module = cmzn_region_get_fieldmodule(source_region);
					for (i = 0; i < number_of_fields; i++)
					{
						if (component_names[i])
						{
							fields[i] = cmzn_fieldmodule_find_field_by_name(field_module, component_names[i]);
							if (fields[i])
							{
								if (!Computed_field_has_numerical_components(fields[i], NULL))
								{
									return_code = 0;
									display_message(ERROR_MESSAGE,
										"gfx_convert elements.  One of the fields specified does not have numerical components");
									break;
								}
							}
							else
							{
								return_code = 0;
								display_message(ERROR_MESSAGE,
									"gfx_convert elements.  One of the fields specified cannot be found");
								break;
							}
						}
					}
					cmzn_fieldmodule_destroy(&field_module);
				}
				else
				{
					return_code = 0;
				}
				if (return_code)
				{
					Element_refinement refinement;
					refinement.count[0] = element_refinement.number_in_xi1;
					refinement.count[1] = element_refinement.number_in_xi2;
					refinement.count[2] = element_refinement.number_in_xi3;
					return_code = finite_element_conversion(
						source_region, destination_region, conversion_mode,
						number_of_fields, fields, refinement, tolerance);
				}
			}
			if (fields)
			{
				for (i = 0; i < number_of_fields; i++)
				{
					if (fields[i])
					{
						DEACCESS(Computed_field)(&fields[i]);
					}
				}
				DEALLOCATE(fields);
			}
			if (component_names)
			{
				for (i = 0; i < number_of_fields; i++)
				{
					if (component_names[i])
					{
						DEALLOCATE(component_names[i]);
					}
				}
				DEALLOCATE(component_names);
			}
			cmzn_region_destroy(&source_region);
			if (destination_region)
			{
				cmzn_region_destroy(&destination_region);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_convert_elements.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_convert_elements.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_convert_elements */

/***************************************************************************//**
 * Executes a GFX CONVERT GRAPHICS command.
 * Converts graphics to finite elements.
 */
static int gfx_convert_graphics(struct Parse_state *state,
	void *dummy_to_be_modified, void *command_data_void)
{
	int return_code;

	USE_PARAMETER(dummy_to_be_modified);
	cmzn_command_data *command_data = reinterpret_cast<cmzn_command_data *>(command_data_void);
	if (state && command_data)
	{
		cmzn_region_id region = cmzn_region_access(command_data->root_region);
		cmzn_field_group_id group = 0;
		struct cmzn_region *input_region = NULL;
		char *coordinate_field_name = 0;
		cmzn_field_id coordinate_field = 0;
		const char *scene_name = 0;
		const char *graphics_name = NULL;
		cmzn_scene_id scene = 0;
		char *scene_path_name = 0;
		cmzn_scenefilter_id filter =
			cmzn_scenefiltermodule_get_default_scenefilter(command_data->filter_module);
		enum Render_to_finite_elements_mode render_mode = RENDER_TO_FINITE_ELEMENTS_LINEAR_PRODUCT;

		Option_table *option_table=CREATE(Option_table)();
		Option_table_add_help(option_table,
			"Create finite elements or a point cloud of nodes from lines and surfaces graphics, "
			"or create nodes from points graphics. With mode 'render_linear_product_elements' "
			"linear finite elements are made from lines and surfaces. "
			"With mode 'render_surface_node_cloud', nodes are created at points in the lines and "
			"surfaces sampled according to a Poisson distribution with the supplied densities. "
			"The surface_density gives the base expected number of points per unit area, and if the "
			"graphics has a data field, the value of its first component scaled by surface_density_scale_factor "
			"is added to the expected number. Separate values for lines control the expected number per unit length. "
			"With mode 'render_nodes', nodes are created at points.");
		/* coordinate */
		Option_table_add_string_entry(option_table,"coordinate",&coordinate_field_name,
			" FIELD_NAME");
		double line_density = 1.0;
		Option_table_add_double_entry(option_table, "line_density", &line_density);
		double line_density_scale_factor = 0.0;
		Option_table_add_double_entry(option_table, "line_density_scale_factor", &line_density_scale_factor);

		/* render_to_finite_elements_mode */
		OPTION_TABLE_ADD_ENUMERATOR(Render_to_finite_elements_mode)(option_table,
			&render_mode);
		Option_table_add_entry(option_table, "filter", &filter,
			command_data->filter_module, set_cmzn_scenefilter);
		/* region */
		Option_table_add_region_or_group_entry(option_table, "region", &region, &group);
		/* scene */
		Option_table_add_string_entry(option_table, "scene",
			&scene_path_name, " SCENE_NAME[/REGION_PATH][.GRAPHIC_NAME]{default}");
		double surface_density = 1.0;
		Option_table_add_double_entry(option_table, "surface_density", &surface_density);
		double surface_density_scale_factor = 0.0;
		Option_table_add_double_entry(option_table, "surface_density_scale_factor", &surface_density_scale_factor);
		return_code=Option_table_multi_parse(option_table,state);
		DESTROY(Option_table)(&option_table);

		if (return_code)
		{
			if (scene_path_name)
			{
				export_object_name_parser(scene_path_name, &scene_name,
					&graphics_name);
			}
			if (coordinate_field_name &&
				(coordinate_field = FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field,name)(
						coordinate_field_name, cmzn_region_get_Computed_field_manager(region))))
			{
				if (Computed_field_has_up_to_3_numerical_components(coordinate_field, NULL))
				{
					cmzn_field_access(coordinate_field);
				}
				else
				{
					coordinate_field = NULL;
					display_message(ERROR_MESSAGE,
						"gfx_convert_graphics.  "
						"Field specified is not a coordinate field.");
					return_code = 0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_convert_graphics.  "
					"Field %s does not exist.", coordinate_field_name);
			}
			if (scene_name && (0!=strcmp(scene_name,"default")))
			{
				input_region = cmzn_region_find_subregion_at_path(command_data->root_region,
					scene_name);
			}
			else
			{
				input_region = cmzn_region_access(command_data->root_region);
			}
			if (scene_name && !input_region)
			{
				display_message(ERROR_MESSAGE,
					"gfx_convert.  Invalid input_scene");
				return_code = 0;
			}
			if (!coordinate_field)
			{
				display_message(ERROR_MESSAGE,
					"gfx_convert_graphics.  "
					"Must specify a coordinate field to define on the new nodes and elements.");
				return_code = 0;
			}
		}

		if (return_code)
		{
			render_to_finite_elements(input_region, graphics_name, filter, render_mode,
				region, group, coordinate_field, static_cast<cmzn_nodeset_id>(0),
				line_density, line_density_scale_factor, surface_density, surface_density_scale_factor);
		}
		if (scene)
		{
			cmzn_scene_destroy(&scene);
		}
		if (coordinate_field)
		{
			DEACCESS(Computed_field)(&coordinate_field);
		}
		if (coordinate_field_name)
		{
			DEALLOCATE(coordinate_field_name);
		}
		if (scene_name)
		{
			DEALLOCATE(scene_name);
		}
		if (graphics_name)
		{
			DEALLOCATE(graphics_name);
		}
		if (input_region)
		{
			cmzn_region_destroy(&input_region);
		}
		if (filter)
			cmzn_scenefilter_destroy(&filter);
		if (scene_path_name)
		{
			DEALLOCATE(scene_path_name);
		}
		cmzn_field_group_destroy(&group);
		cmzn_region_destroy(&region);
	}
	else
	{
		display_message(ERROR_MESSAGE, "gfx_convert_graphics.  Invalid argument(s)");
		return_code = 0;
	}
	return (return_code);
}

static int gfx_convert(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 5 April 2006

DESCRIPTION :
Executes a GFX CONVERT command.
==============================================================================*/
{
	int return_code;
	struct Option_table *option_table;

	ENTER(gfx_convert);
	USE_PARAMETER(dummy_to_be_modified);
	if (state && command_data_void)
	{
		option_table = CREATE(Option_table)();
		Option_table_add_entry(option_table,"elements",NULL,
			command_data_void, gfx_convert_elements);
		Option_table_add_entry(option_table,"graphics",NULL,
			command_data_void, gfx_convert_graphics);
		return_code = Option_table_parse(option_table,state);
		DESTROY(Option_table)(&option_table);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_convert.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* gfx_convert */

static int execute_command_gfx_create(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 4 May 2004

DESCRIPTION :
Executes a GFX CREATE command.
==============================================================================*/
{
	int return_code;
	struct cmzn_command_data *command_data;
	struct Option_table *option_table;

	ENTER(execute_command_gfx_create);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (NULL != (command_data = (struct cmzn_command_data *)command_data_void))
		{
			if (state->current_token)
			{
				option_table=CREATE(Option_table)();
				Option_table_add_entry(option_table,"axes",NULL,
					command_data_void,gfx_create_axes);
				Option_table_add_entry(option_table,"colour_bar",NULL,
					command_data_void,gfx_create_colour_bar);
#if defined (WX_USER_INTERFACE)
				Option_table_add_entry(option_table,"data_viewer",NULL,
					command_data_void,gfx_create_data_viewer);
#endif /* defined (WX_USER_INTERFACE) */
				Option_table_add_entry(option_table, "dgroup", /*use_object_type*/(void *)2,
					(void *)command_data->root_region, gfx_create_group);
				Option_table_add_entry(option_table, "egroup", /*use_object_type*/(void *)0,
					(void *)command_data->root_region, gfx_create_group);
#if defined (WX_USER_INTERFACE)
				Option_table_add_entry(option_table,"element_creator",NULL,
					command_data_void,gfx_create_element_creator);
#endif /* defined (WX_USER_INTERFACE) */
#if defined (WX_USER_INTERFACE)
				Option_table_add_entry(option_table,"element_point_viewer",NULL,
					command_data_void,gfx_create_element_point_viewer);
#endif /* defined (WX_USER_INTERFACE) */
#if defined (EMOTER_ENABLE)
				struct Create_emoter_slider_data create_emoter_slider_data;
				create_emoter_slider_data.execute_command=command_data->execute_command;
				create_emoter_slider_data.root_region=
					command_data->root_region;
				create_emoter_slider_data.basis_manager=
					command_data->basis_manager;
#if defined (USE_CMGUI_GRAPHICS_WINDOW)
				create_emoter_slider_data.graphics_window_manager=
					command_data->graphics_window_manager;
#endif /* defined (USE_CMGUI_GRAPHICS_WINDOW) */
				create_emoter_slider_data.graphics_buffer_package=
					command_data->graphics_buffer_package;
				create_emoter_slider_data.curve_manager=
					command_data->curve_manager;
				create_emoter_slider_data.io_stream_package =
					command_data->io_stream_package;
				create_emoter_slider_data.viewer_scene=command_data->default_scene;
				create_emoter_slider_data.viewer_background_colour=
					command_data->background_colour;
				create_emoter_slider_data.viewer_light=command_data->default_light;
				create_emoter_slider_data.viewer_light_model=
					command_data->default_light_model;
				create_emoter_slider_data.emoter_dialog_address=
					&(command_data->emoter_slider_dialog);
				create_emoter_slider_data.user_interface=
					command_data->user_interface;
				Option_table_add_entry(option_table,"emoter",NULL,
					(void *)&create_emoter_slider_data,gfx_create_emoter);
#endif
				Option_table_add_entry(option_table, "flow_particles",
					/*create_more*/(void *)0, command_data_void, gfx_create_flow_particles);
#if defined (WX_USER_INTERFACE)
				Option_table_add_entry(option_table,"graphical_material_editor",NULL,
					command_data_void,gfx_create_graphical_material_editor);
#endif /* defined (WX_USER_INTERFACE) */
				Option_table_add_entry(option_table, "gauss_points", NULL,
					(void *)command_data->root_region, gfx_create_gauss_points);
				Option_table_add_entry(option_table,"light",NULL,
					command_data_void,gfx_create_light);
				Option_table_add_entry(option_table,"lmodel",NULL,
					command_data_void, gfx_create_modify_light_model);
				struct Material_module_app materialmodule;
				materialmodule.module = (void *)command_data->materialmodule;
				materialmodule.region = (void *)command_data->root_region;
				cmzn_shadermodule_id shadermodule = cmzn_graphics_module_get_shadermodule(command_data->graphics_module);
				materialmodule.shadermodule = (void *)shadermodule;  // can't be accessed
				cmzn_shadermodule_destroy(&shadermodule);
				Option_table_add_entry(option_table,"material",NULL,
					(void *)(&materialmodule),gfx_create_material);
				Option_table_add_entry(option_table, "more_flow_particles",
					/*create_more*/(void *)1, command_data_void, gfx_create_flow_particles);
				Option_table_add_entry(option_table, "ngroup", /*use_object_type*/(void *)1,
					(void *)command_data->root_region, gfx_create_group);
#if defined (WX_USER_INTERFACE)
				Option_table_add_entry(option_table,"node_viewer",NULL,
					command_data_void,gfx_create_node_viewer);
#endif /* defined (WX_USER_INTERFACE) */
				Option_table_add_entry(option_table, "region", NULL,
					(void *)command_data->root_region, gfx_create_region);
				Option_table_add_entry(option_table, "snake", NULL,
					command_data_void, gfx_create_snake);
				Option_table_add_entry(option_table,"spectrum",NULL,
					command_data_void,gfx_create_spectrum);
				Option_table_add_entry(option_table,"texture",NULL,
					command_data_void,gfx_create_texture);
#if defined (WX_USER_INTERFACE)
				Option_table_add_entry(option_table,"time_editor",NULL,
					command_data_void,gfx_create_time_editor);
#endif /* defined (WX_USER_INTERFACE) */
#if defined (USE_CMGUI_GRAPHICS_WINDOW)
				Option_table_add_entry(option_table,"window",NULL,
					command_data_void,gfx_create_window);
#endif /* defined (USE_CMGUI_GRAPHICS_WINDOW) */
				return_code=Option_table_parse(option_table,state);
				DESTROY(Option_table)(&option_table);
			}
			else
			{
				set_command_prompt("gfx create",command_data);
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"execute_command_gfx_create.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_command_gfx_create.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_gfx_create */

/** Explains migration for removed command 'gfx define curve' */
int gfx_define_Curve(struct Parse_state *state,
	void *dummy_to_be_modified, void *command_data_void)
{
	USE_PARAMETER(dummy_to_be_modified);
	USE_PARAMETER(command_data_void);
	display_message(Parse_state_help_mode(state) ? INFORMATION_MESSAGE : ERROR_MESSAGE,
		"The 'gfx define curve' command has been removed. You can replace a curve with a\n"
		"1-D mesh defining a non-decreasing scalar 'parameter' field, plus a 'value' field\n"
		"to look up for a given parameter value. If your command has 'file' option, you can\n"
		"directly load the exnode/exelem or single exregion file(s) which are defined this way.\n"
		"Use a 'find_mesh_location' field and an embedded field to achieve the effect of the\n"
		"former 'curve_lookup' field type. These currently only work in the same region;\n"
		"talk to the Cmgui developers if you have migration issues.\n");
	if (Parse_state_help_mode(state))
		return 1;
	return 0;
}

/***************************************************************************//**
 * Executes a GFX DEFINE FACES command.
 */
static int gfx_define_faces(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
{
	int return_code;
	USE_PARAMETER(dummy_to_be_modified);
	cmzn_command_data *command_data = reinterpret_cast<cmzn_command_data *>(command_data_void);
	if (state && command_data)
	{
		cmzn_region_id region = cmzn_region_access(command_data->root_region);
		cmzn_field_group_id group = 0;
		Option_table *option_table = CREATE(Option_table)();
		Option_table_add_region_or_group_entry(option_table, "egroup", &region, &group);
		return_code = Option_table_multi_parse(option_table, state);
		DESTROY(Option_table)(&option_table);
		if (return_code)
		{
			cmzn_fieldmodule_id field_module = cmzn_region_get_fieldmodule(region);
			cmzn_fieldmodule_begin_change(field_module);
			FE_region *fe_region = cmzn_region_get_FE_region(region);
			FE_region_begin_define_faces(fe_region);
			for (int dimension = MAXIMUM_ELEMENT_XI_DIMENSIONS; (1 < dimension) && return_code; --dimension)
			{
				cmzn_mesh_id mesh = cmzn_fieldmodule_find_mesh_by_dimension(field_module, dimension);
				FE_mesh *fe_mesh = FE_region_find_FE_mesh_by_dimension(fe_region, dimension);
				if (!fe_mesh)
				{
					return_code = 0;
					break;
				}
				cmzn_mesh_group_id face_mesh_group = 0;
				if (group)
				{
					cmzn_field_element_group_id element_group = cmzn_field_group_get_field_element_group(group, mesh);
					cmzn_mesh_destroy(&mesh);
					mesh = cmzn_mesh_group_base_cast(cmzn_field_element_group_get_mesh_group(element_group));
					cmzn_field_element_group_destroy(&element_group);
				}
				if (mesh && (0 < cmzn_mesh_get_size(mesh)))
				{
					if (group)
					{
						cmzn_mesh_id face_master_mesh = cmzn_fieldmodule_find_mesh_by_dimension(field_module, dimension - 1);
						cmzn_field_element_group_id face_element_group = cmzn_field_group_get_field_element_group(group, face_master_mesh);
						if (!face_element_group)
							face_element_group = cmzn_field_group_create_field_element_group(group, face_master_mesh);
						cmzn_mesh_destroy(&face_master_mesh);
						face_mesh_group = cmzn_field_element_group_get_mesh_group(face_element_group);
						cmzn_field_element_group_destroy(&face_element_group);
					}
					cmzn_elementiterator_id iter = cmzn_mesh_create_elementiterator(mesh);
					cmzn_element_id element = 0;
					while ((0 != (element = cmzn_elementiterator_next_non_access(iter))) && return_code)
					{
						const int result = fe_mesh->defineElementFaces(element->getIndex());
						if (result != CMZN_OK)
						{
							if (result == CMZN_ERROR_NOT_FOUND)
							{
								// no nodes defined, so no faces can be found
								continue;
							}
							return_code = 0;
						}
						if (face_mesh_group)
						{
							FE_element_shape *element_shape = get_FE_element_shape(element);
							int number_of_faces = FE_element_shape_get_number_of_faces(element_shape);
							cmzn_element_id face = 0;
							for (int face_number = 0; face_number < number_of_faces; ++face_number)
							{
								face = get_FE_element_face(element, face_number);
								if (face)
								{
									if (!cmzn_mesh_contains_element(cmzn_mesh_group_base_cast(face_mesh_group), face) &&
										!cmzn_mesh_group_add_element(face_mesh_group, face))
									{
										return_code = 0;
										break;
									}
								}
							}
						}
					}
					cmzn_elementiterator_destroy(&iter);
				}
				cmzn_mesh_group_destroy(&face_mesh_group);
				cmzn_mesh_destroy(&mesh);
			}
			FE_region_end_define_faces(fe_region);
			cmzn_fieldmodule_end_change(field_module);
			cmzn_fieldmodule_destroy(&field_module);
		}
		cmzn_field_group_destroy(&group);
		cmzn_region_destroy(&region);
	}
	else
	{
		display_message(ERROR_MESSAGE, "gfx_define_faces.  Invalid argument(s)");
		return_code = 0;
	}
	return (return_code);
}

static int execute_command_gfx_define(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Executes a GFX DEFINE command.
==============================================================================*/
{
	int return_code;
	struct cmzn_command_data *command_data;
	struct Option_table *option_table;

	ENTER(execute_command_gfx_define);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (NULL != (command_data = (struct cmzn_command_data *)command_data_void))
		{
			if (state->current_token)
			{
				cmzn_fontmodule_id fontmodule = cmzn_graphics_module_get_fontmodule(
					command_data->graphics_module);
				option_table = CREATE(Option_table)();
				/* curve */
				Option_table_add_entry(option_table, "curve", NULL,
					command_data_void, gfx_define_Curve);
				/* faces */
				Option_table_add_entry(option_table, "faces", NULL,
					command_data_void, gfx_define_faces);
				/* field */
				Option_table_add_entry(option_table, "field", command_data->root_region,
					command_data->computed_field_package, define_Computed_field);
				/* font */
				Option_table_add_entry(option_table, "font", NULL,
					fontmodule, gfx_define_font);
				/* glyph */
				Define_glyph_data define_glyph_data =
				{
					command_data->root_region, command_data->glyphmodule
				};
				Option_table_add_entry(option_table, "glyph", NULL,
					&define_glyph_data, gfx_define_glyph);
				/* scene */
				Define_scene_data define_scene_data;
				define_scene_data.root_region = command_data->root_region;
				define_scene_data.graphics_module = command_data->graphics_module;
				Option_table_add_entry(option_table, "scene", NULL,
					(void *)(&define_scene_data), define_Scene);
				/* graphics_filter */
				Option_table_add_entry(option_table, "graphics_filter", command_data->root_region,
					command_data->filter_module, gfx_define_graphics_filter);
				/* tessellation */
				Option_table_add_entry(option_table, "tessellation", NULL,
					command_data->tessellationmodule, gfx_define_tessellation);
				return_code = Option_table_parse(option_table, state);
				DESTROY(Option_table)(&option_table);
				cmzn_fontmodule_destroy(&fontmodule);
			}
			else
			{
				set_command_prompt("gfx define",command_data);
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"execute_command_gfx_define.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_command_gfx_define.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_gfx_define */

/***************************************************************************//**
 * Destroys a group.
 */
static int gfx_destroy_group(struct Parse_state *state,
	void *dummy_to_be_modified, void *root_region_void)
{
	int return_code = 1;
	USE_PARAMETER(dummy_to_be_modified);
	cmzn_region_id root_region = reinterpret_cast<cmzn_region_id>(root_region_void);
	if (state && root_region)
	{
		cmzn_region_id region = cmzn_region_access(root_region);
		cmzn_field_group_id group = 0;
		const char *current_token = state->current_token;
		if (!set_cmzn_region_or_group(state, &region, &group) ||
			Parse_state_help_mode(state))
		{
			// message already output
		}
		else if (!group)
		{
			display_message(ERROR_MESSAGE, "Not a group: %s", current_token);
			return_code = 0;
		}
		else
		{
			if (!MANAGED_OBJECT_NOT_IN_USE(Computed_field)(cmzn_field_group_base_cast(group),
				cmzn_region_get_Computed_field_manager(region)))
			{
				display_message(INFORMATION_MESSAGE, "Group %s marked for destruction when no longer in use.\n",
					current_token);
			}
			return_code = cmzn_field_set_managed(cmzn_field_group_base_cast(group), false);
		}
		cmzn_field_group_destroy(&group);
		cmzn_region_destroy(&region);
	}
	else
	{
		display_message(ERROR_MESSAGE, "gfx_destroy_group.  Invalid argument(s)");
		return_code = 0;
	}
	return return_code;
}

static int gfx_destroy_region(struct Parse_state *state,
	void *dummy_to_be_modified, void *root_region_void)
/*******************************************************************************
LAST MODIFIED : 27 March 2003

DESCRIPTION :
Executes a GFX DESTROY REGION command.
==============================================================================*/
{
	const char *current_token;
	int return_code = 1;
	struct cmzn_region *root_region;

	ENTER(gfx_destroy_region);
	USE_PARAMETER(dummy_to_be_modified);
	if (state && (root_region = (struct cmzn_region *)root_region_void))
	{
		if (NULL != (current_token = state->current_token))
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				struct cmzn_region *region =
					cmzn_region_find_subregion_at_path(root_region, current_token);
				if (region)
				{
					struct cmzn_region *parent_region = cmzn_region_get_parent(region);
					if (parent_region)
					{
						cmzn_region_remove_child(parent_region, region);
						cmzn_region_destroy(&parent_region);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"gfx destroy region:  The root region may not be removed");
						display_parse_state_location(state);
						return_code = 0;
					}
					cmzn_region_destroy(&region);
				}
				else
				{
					display_message(ERROR_MESSAGE, "Unknown region: %s", current_token);
					display_parse_state_location(state);
					return_code = 0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE, " REGION_PATH");
				return_code = 1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE, "Missing region path");
			display_parse_state_location(state);
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE, "gfx_destroy_region.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* gfx_destroy_region */

/***************************************************************************//**
 * Executes a GFX DESTROY ELEMENTS command.
 */
static int gfx_destroy_elements(struct Parse_state *state,
	void *dimension_void, void *command_data_void)
{
	int return_code;
	int dimension = VOIDPTR2INT(dimension_void);
	cmzn_command_data *command_data = reinterpret_cast<cmzn_command_data *>(command_data_void);
	if (state && command_data && (0 < dimension) && (dimension <= 3))
	{
		/* initialise defaults */
		char all_flag = 0;
		cmzn_region_id region = cmzn_region_access(command_data->root_region);
		cmzn_field_group_id group = 0;
		cmzn_field_id conditional_field = 0;
		char *conditional_field_name = 0;
		char selected_flag = 0;
		Multi_range *element_ranges = CREATE(Multi_range)();
		FE_value time;
		if (command_data->default_time_keeper_app)
		{
			time = command_data->default_time_keeper_app->getTimeKeeper()->getTime();
		}
		else
		{
			time = 0.0;
		}

		Option_table *option_table = CREATE(Option_table)();
		/* all */
		Option_table_add_char_flag_entry(option_table, "all", &all_flag);
		/* conditional_field */
		Option_table_add_string_entry(option_table,"conditional_field",
			&conditional_field_name, " FIELD_NAME");
		/* group */
		Option_table_add_region_or_group_entry(option_table, "group", &region, &group);
		/* selected */
		Option_table_add_char_flag_entry(option_table, "selected", &selected_flag);
		/* default option: element number ranges */
		Option_table_add_entry(option_table, (const char *)NULL, (void *)element_ranges,
			NULL, set_Multi_range);
		return_code = Option_table_multi_parse(option_table, state);
		DESTROY(Option_table)(&option_table);

		if (return_code)
		{
			if ((0 == Multi_range_get_number_of_ranges(element_ranges)) && (!selected_flag) &&
				(0 == conditional_field_name) && (!all_flag))
			{
				display_message(ERROR_MESSAGE, "gfx destroy elements:  No elements specified.");
				return_code = 0;
			}
		}
		if (return_code && conditional_field_name)
		{
			cmzn_fieldmodule_id field_module = cmzn_region_get_fieldmodule(region);
			conditional_field = cmzn_fieldmodule_find_field_by_name(field_module, conditional_field_name);
			if (!conditional_field)
			{
				display_message(ERROR_MESSAGE,
					"gfx_destroy_elements:  conditional field cannot be found");
				return_code = 0;
			}
			cmzn_fieldmodule_destroy(&field_module);
		}
		if (return_code)
		{
			cmzn_fieldmodule_id fieldmodule = cmzn_region_get_fieldmodule(region);
			cmzn_fieldmodule_begin_change(fieldmodule);
			cmzn_field_id selection_field = 0;
			if (selected_flag)
			{
				cmzn_scene *scene = cmzn_region_get_scene(region);
				selection_field = cmzn_scene_get_selection_field(scene);
				cmzn_scene_destroy(&scene);
			}
			FE_region *fe_region = cmzn_region_get_FE_region(region);
			int use_dimension = dimension;
			if (dimension == 3)
				use_dimension = FE_region_get_highest_dimension(fe_region);
			cmzn_mesh_id mesh = cmzn_fieldmodule_find_mesh_by_dimension(fieldmodule, use_dimension);
			const int oldSize = cmzn_mesh_get_size(mesh);
			if ((!selected_flag) || (selection_field)) // otherwise empty set
			{
				cmzn_field_id use_conditional_field = cmzn_mesh_create_conditional_field_from_ranges_and_selection(
					mesh, element_ranges, selection_field, cmzn_field_group_base_cast(group), conditional_field, time);
				if (use_conditional_field)
				{
					int result = cmzn_mesh_destroy_elements_conditional(mesh, use_conditional_field);
					if (result != CMZN_OK)
					{
						display_message(INFORMATION_MESSAGE, "gfx destroy elements|faces|lines:  Failed\n");
						return_code = 0;
					}
				}
				else
					return_code = 0;
				cmzn_field_destroy(&use_conditional_field);
			}
			const int newSize = cmzn_mesh_get_size(mesh);
			if (return_code && (newSize == oldSize))
				display_message(INFORMATION_MESSAGE, "gfx destroy elements|faces|lines:  No elements destroyed\n");
			if (selection_field)
			{
				cmzn_field_group_id selection_group = cmzn_field_cast_group(selection_field);
				cmzn_field_group_remove_empty_subgroups(selection_group);
				cmzn_field_group_destroy(&selection_group);
			}
			cmzn_mesh_destroy(&mesh);
			cmzn_field_destroy(&selection_field);
			cmzn_fieldmodule_end_change(fieldmodule);
			cmzn_fieldmodule_destroy(&fieldmodule);
		}
		cmzn_field_destroy(&conditional_field);
		if (conditional_field_name)
			DEALLOCATE(conditional_field_name);
		DESTROY(Multi_range)(&element_ranges);
		cmzn_field_group_destroy(&group);
		cmzn_region_destroy(&region);
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_destroy_elements.  Invalid argument(s)");
		return_code = 0;
	}
	return (return_code);
}

static int gfx_destroy_Computed_field(struct Parse_state *state,
	void *dummy_to_be_modified,void *root_region_void)
/*******************************************************************************
LAST MODIFIED : 24 July 2008

DESCRIPTION :
Executes a GFX DESTROY FIELD command.
==============================================================================*/
{
	const char *current_token;
	char *field_name, *region_path;
	struct Computed_field *field;
	int return_code = 0;
	struct cmzn_region *region, *root_region;

	ENTER(gfx_destroy_Computed_field);
	USE_PARAMETER(dummy_to_be_modified);
	if (state && (root_region=(struct cmzn_region *)root_region_void))
	{
		if (NULL != (current_token = state->current_token))
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				if (cmzn_region_get_partial_region_path(root_region,
					current_token, &region, &region_path, &field_name))
				{
					if (field_name &&
						(field = FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field,name)(
							field_name, cmzn_region_get_Computed_field_manager(region))))
					{
						ACCESS(Computed_field)(field);
						if (!MANAGED_OBJECT_NOT_IN_USE(Computed_field)(field,
							cmzn_region_get_Computed_field_manager(region)))
						{
							display_message(INFORMATION_MESSAGE, "Field %s marked for destruction when no longer in use.\n",
								current_token);
						}
						return_code = cmzn_field_set_managed(field, false);
						if (!return_code)
						{
							display_message(ERROR_MESSAGE, "gfx destroy field.  Failed");
						}
						DEACCESS(Computed_field)(&field);
					}
					else
					{
						display_message(ERROR_MESSAGE,"Field does not exist: %s",
							current_token);
						display_parse_state_location(state);
						return_code=0;
					}
					DEALLOCATE(region_path);
					DEALLOCATE(field_name);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"gfx_destroy_Computed_field.  Failed to get region_path/field_name");
					return_code = 0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE," [REGION_PATH/]FIELD_NAME");
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Missing region_path/field name");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_destroy_Computed_field.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_destroy_Computed_field */

static int gfx_destroy_graphics_object(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 20 November 1998

DESCRIPTION :
Executes a GFX DESTROY GRAPHICS_OBJECT command.
==============================================================================*/
{
	const char *current_token;
	int return_code;
	struct cmzn_command_data *command_data;

	ENTER(gfx_destroy_graphics_object);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (NULL != (command_data = (struct cmzn_command_data *)command_data_void))
		{
			if (NULL != (current_token = state->current_token))
			{
				if (strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
				{
					cmzn_glyph_id glyph = cmzn_glyphmodule_find_glyph_by_name(command_data->glyphmodule, current_token);
					if (glyph)
					{
						cmzn_glyph_set_managed(glyph, false);
						cmzn_glyph_destroy(&glyph);
					}
					else
					{
						display_message(ERROR_MESSAGE,"Graphics object does not exist: %s",
							current_token);
						display_parse_state_location(state);
						return_code=0;
					}
				}
				else
				{
					display_message(INFORMATION_MESSAGE," GRAPHICS_OBJECT_NAME");
					return_code=1;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,"Missing graphics object name");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_destroy_graphics_object.  Missing glyph_manager");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_destroy_graphics_object.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_destroy_graphics_object */

static int gfx_destroy_material(struct Parse_state *state,
	void *dummy_to_be_modified, void *graphical_material_manager_void)
/*******************************************************************************
LAST MODIFIED : 8 May 2002

DESCRIPTION :
Executes a GFX DESTROY MATERIAL command.
==============================================================================*/
{
	const char *current_token;
	cmzn_material *graphical_material;
	int return_code;
	struct MANAGER(cmzn_material) *graphical_material_manager;

	ENTER(gfx_destroy_material);
	USE_PARAMETER(dummy_to_be_modified);
	if (state && (graphical_material_manager =
		(struct MANAGER(cmzn_material) *)graphical_material_manager_void))
	{
		if (NULL != (current_token = state->current_token))
		{
			if (strcmp(PARSER_HELP_STRING, current_token) &&
				strcmp(PARSER_RECURSIVE_HELP_STRING, current_token))
			{
				if (NULL != (graphical_material =
					FIND_BY_IDENTIFIER_IN_MANAGER(cmzn_material, name)(
						current_token, graphical_material_manager)))
				{
					if (REMOVE_OBJECT_FROM_MANAGER(cmzn_material)(graphical_material,
						graphical_material_manager))
					{
						return_code = 1;
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Could not remove material %s from manager", current_token);
						return_code = 0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Unknown material: %s", current_token);
					return_code = 0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE, " MATERIAL_NAME");
				return_code = 1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE, "Missing material name");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_destroy_material.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* gfx_destroy_material */

/***************************************************************************//**
 * Executes a GFX DESTROY NODES/DATA command.
 * If <use_data_flag> is set, work with datapoints, otherwise nodes.
 */
static int gfx_destroy_nodes(struct Parse_state *state,
	void *use_data, void *command_data_void)
{
	int return_code;
	cmzn_command_data *command_data = reinterpret_cast<cmzn_command_data *>(command_data_void);
	if (state && command_data)
	{
		/* initialise defaults */
		char all_flag = 0;
		cmzn_region_id region = cmzn_region_access(command_data->root_region);
		cmzn_field_group_id group = 0;
		cmzn_field_id conditional_field = 0;
		char *conditional_field_name = 0;
		char selected_flag = 0;
		Multi_range *node_ranges = CREATE(Multi_range)();
		FE_value time;
		if (command_data->default_time_keeper_app)
		{
			time = command_data->default_time_keeper_app->getTimeKeeper()->getTime();
		}
		else
		{
			time = 0.0;
		}

		Option_table *option_table = CREATE(Option_table)();
		/* all */
		Option_table_add_char_flag_entry(option_table, "all", &all_flag);
		/* conditional_field */
		Option_table_add_string_entry(option_table,"conditional_field",
			&conditional_field_name, " FIELD_NAME");
		/* group */
		Option_table_add_region_or_group_entry(option_table, "group", &region, &group);
		/* selected */
		Option_table_add_char_flag_entry(option_table, "selected", &selected_flag);
		/* default option: node number ranges */
		Option_table_add_entry(option_table, (const char *)NULL, (void *)node_ranges,
			NULL, set_Multi_range);
		return_code = Option_table_multi_parse(option_table, state);
		DESTROY(Option_table)(&option_table);

		if (return_code)
		{
			if ((0 == Multi_range_get_number_of_ranges(node_ranges)) && (!selected_flag) &&
				(0 == conditional_field_name) && (!all_flag))
			{
				display_message(ERROR_MESSAGE, "gfx destroy nodes:  No nodes specified.");
				return_code = 0;
			}
		}
		cmzn_fieldmodule_id fieldmodule = cmzn_region_get_fieldmodule(region);
		if (return_code && conditional_field_name)
		{
			conditional_field = cmzn_fieldmodule_find_field_by_name(fieldmodule, conditional_field_name);
			if (!conditional_field)
			{
				display_message(ERROR_MESSAGE,
					"gfx destroy nodes:  conditional field cannot be found");
				return_code = 0;
			}
		}
		if (return_code)
		{
			cmzn_fieldmodule_begin_change(fieldmodule);
			cmzn_field_group_id selection_group = 0;
			if (selected_flag)
			{
				cmzn_scene *scene = cmzn_region_get_scene(region);
				cmzn_field_id selection_field = cmzn_scene_get_selection_field(scene);
				selection_group = cmzn_field_cast_group(selection_field);
				cmzn_field_destroy(&selection_field);
				cmzn_scene_destroy(&scene);
			}
			cmzn_nodeset_id master_nodeset = cmzn_fieldmodule_find_nodeset_by_field_domain_type(fieldmodule,
				use_data ? CMZN_FIELD_DOMAIN_TYPE_DATAPOINTS : CMZN_FIELD_DOMAIN_TYPE_NODES);
			const int oldSize = cmzn_nodeset_get_size(master_nodeset);
			bool nodesInElementsNotDestroyed = false;
			if ((!selected_flag) || selection_group) // otherwise empty set
			{
				cmzn_nodeset_id nodeset = 0;
				if (selection_group)
				{
					// if selection node group is empty => nothing to do
					cmzn_field_node_group_id node_group_field = cmzn_field_group_get_field_node_group(selection_group, master_nodeset);
					nodeset = cmzn_nodeset_group_base_cast(cmzn_field_node_group_get_nodeset_group(node_group_field));
					cmzn_field_node_group_destroy(&node_group_field);
				}
				else
					nodeset = cmzn_nodeset_access(master_nodeset);
				if (nodeset)
				{
					cmzn_field_id use_conditional_field = cmzn_nodeset_create_conditional_field_from_ranges_and_selection(
						nodeset, node_ranges, /*selection_field*/0, cmzn_field_group_base_cast(group), conditional_field, time);
					if (use_conditional_field)
					{
						int result = cmzn_nodeset_destroy_nodes_conditional(nodeset, use_conditional_field);
						if (result == CMZN_RESULT_ERROR_IN_USE)
							nodesInElementsNotDestroyed = true;
						else if (result != CMZN_OK)
							return_code = 0;
						if (selection_group)
							cmzn_field_group_remove_empty_subgroups(selection_group);
					}
					else
						return_code = 0;
					cmzn_field_destroy(&use_conditional_field);
					cmzn_nodeset_destroy(&nodeset);
				}
			}
			const int newSize = cmzn_nodeset_get_size(master_nodeset);
			cmzn_nodeset_destroy(&master_nodeset);
			cmzn_field_group_destroy(&selection_group);
			cmzn_fieldmodule_end_change(fieldmodule);
			if (return_code)
			{
				if (nodesInElementsNotDestroyed)
					display_message(INFORMATION_MESSAGE, "gfx destroy nodes|datapoints:  Nodes used by elements were not destroyed\n");
				else if (newSize == oldSize)
					display_message(INFORMATION_MESSAGE, "gfx destroy nodes|datapoints:  No nodes destroyed\n");
			}
			else
			{
				display_message(ERROR_MESSAGE, "gfx destroy nodes|datapoints:  Failed\n");
				return_code = 0;
			}
		}
		cmzn_fieldmodule_destroy(&fieldmodule);
		cmzn_field_destroy(&conditional_field);
		if (conditional_field_name)
			DEALLOCATE(conditional_field_name);
		DESTROY(Multi_range)(&node_ranges);
		cmzn_field_group_destroy(&group);
		cmzn_region_destroy(&region);
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_destroy_nodes.  Invalid argument(s)");
		return_code = 0;
	}
	return (return_code);
}

static int gfx_destroy_Scene(struct Parse_state *state,
	void *dummy_to_be_modified, void *scene_manager_void)
/*******************************************************************************
LAST MODIFIED : 5 December 2001

DESCRIPTION :
Executes a GFX DESTROY SCENE command.
==============================================================================*/
{
	int return_code;

	ENTER(gfx_destroy_Scene);
	USE_PARAMETER(dummy_to_be_modified);
	USE_PARAMETER(scene_manager_void);
	if (state)
	{
		display_message(INFORMATION_MESSAGE, " Scene is now the 1 to 1  graphics properties of a region and "
			"cannot be destroyed");
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE, "gfx_destroy_Scene.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* gfx_destroy_Scene */

static int gfx_destroy_vtextures(struct Parse_state *state,
	void *dummy_to_be_modified,void *volume_texture_manager_void)
/*******************************************************************************
LAST MODIFIED : 7 October 1996

DESCRIPTION :
Executes a GFX DESTROY VTEXTURES command.
???DB.  Could merge with destroy_graphics_objects if graphics_objects used the
	new list structures.
==============================================================================*/
{
	const char *current_token;
	int return_code;
	struct MANAGER(VT_volume_texture) *volume_texture_manager;
	struct VT_volume_texture *volume_texture;

	ENTER(gfx_destroy_vtextures);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (NULL != (volume_texture_manager=
			(struct MANAGER(VT_volume_texture) *)volume_texture_manager_void))
		{
			if (NULL != (current_token = state->current_token))
			{
				if (strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
				{
					if (NULL != (volume_texture=FIND_BY_IDENTIFIER_IN_MANAGER(VT_volume_texture,
						name)(current_token,volume_texture_manager)))
					{
						/* remove object from list (destroys automatically) */
						return_code=REMOVE_OBJECT_FROM_MANAGER(VT_volume_texture)(
							volume_texture,volume_texture_manager);
					}
					else
					{
						display_message(ERROR_MESSAGE,"Volume texture does not exist: %s",
							current_token);
						display_parse_state_location(state);
						return_code=0;
					}
				}
				else
				{
					display_message(INFORMATION_MESSAGE," <NAME[all]>");
					return_code=1;
				}
			}
			else
			{
				/* destroy all objects in list */
				return_code=REMOVE_ALL_OBJECTS_FROM_MANAGER(VT_volume_texture)(
					volume_texture_manager);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_destroy_vtextures.  Missing volume texture manager");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_destroy_vtextures.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_destroy_vtextures */

static int execute_command_gfx_destroy(struct Parse_state *state,
	void *dummy_to_be_modified, void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 3 March 2003

DESCRIPTION :
Executes a GFX DESTROY command.
==============================================================================*/
{
	int return_code;
	struct cmzn_command_data *command_data;
	struct Option_table *option_table;

	ENTER(execute_command_gfx_destroy);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (NULL != (command_data = (struct cmzn_command_data *)command_data_void))
		{
			if (state->current_token)
			{
				option_table = CREATE(Option_table)();
				/* data */
				Option_table_add_entry(option_table, "data", /*use_data*/(void *)1,
					command_data_void, gfx_destroy_nodes);
				/* dgroup */
				Option_table_add_entry(option_table, "dgroup", NULL,
					command_data->root_region, gfx_destroy_group);
				/* egroup */
				Option_table_add_entry(option_table, "egroup", NULL,
					command_data->root_region, gfx_destroy_group);
				/* elements */
				Option_table_add_entry(option_table, "elements", /*dimension*/(void *)3,
					command_data_void, gfx_destroy_elements);
				/* faces */
				Option_table_add_entry(option_table, "faces", /*dimension*/(void *)2,
					command_data_void, gfx_destroy_elements);
				/* field */
				Option_table_add_entry(option_table, "field", NULL,
					(void *)command_data->root_region, gfx_destroy_Computed_field);
				/* graphics_object */
				Option_table_add_entry(option_table, "graphics_object", NULL,
					command_data_void, gfx_destroy_graphics_object);
				/* lines */
				Option_table_add_entry(option_table, "lines", /*dimension*/(void *)1,
					command_data_void, gfx_destroy_elements);
				/* material */
				Option_table_add_entry(option_table, "material", NULL,
					cmzn_materialmodule_get_manager(command_data->materialmodule), gfx_destroy_material);
				/* ngroup */
				Option_table_add_entry(option_table, "ngroup", NULL,
					command_data->root_region, gfx_destroy_group);
				/* nodes */
				Option_table_add_entry(option_table, "nodes", /*use_data*/(void *)0,
					command_data_void, gfx_destroy_nodes);
				/* region */
				Option_table_add_entry(option_table, "region", NULL,
					command_data->root_region, gfx_destroy_region);
				/* scene */
				Option_table_add_entry(option_table, "scene", NULL,
					command_data->scene_manager, gfx_destroy_Scene);
				/* spectrum */
				Option_table_add_entry(option_table, "spectrum", NULL,
					command_data->spectrum_manager, gfx_destroy_spectrum);
				/* tessellation */
				Option_table_add_entry(option_table, "tessellation", NULL,
					command_data->tessellationmodule, gfx_destroy_tessellation);
				/* vtextures */
				Option_table_add_entry(option_table, "vtextures", NULL,
					command_data->volume_texture_manager, gfx_destroy_vtextures);
				return_code = Option_table_parse(option_table, state);
				DESTROY(Option_table)(&option_table);
			}
			else
			{
				set_command_prompt("gfx destroy",command_data);
				return_code = 1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"execute_command_gfx_destroy.  Invalid argument(s)");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"execute_command_gfx_destroy.  Missing state");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_gfx_destroy */

static int execute_command_gfx_draw(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 3 March 2003

DESCRIPTION :
Executes a GFX DRAW command.
==============================================================================*/
{
	int return_code;
	struct cmzn_command_data *command_data;
	struct Option_table *option_table;

	ENTER(execute_command_gfx_draw);
	USE_PARAMETER(dummy_to_be_modified);
	if (state && (command_data=(struct cmzn_command_data *)command_data_void))
	{
		cmzn_glyph *glyph = 0;
		cmzn_scene_id scene = cmzn_scene_access(command_data->default_scene);
		char *graphics_name = 0;
		char *region_path = 0;
		option_table=CREATE(Option_table)();
		/* as */
		Option_table_add_entry(option_table,"as",&graphics_name,
			(void *)1,set_name);
		/* glyph */
		Option_table_add_entry(option_table,"glyph",&glyph,
			command_data->glyphmodule, set_Glyph);
		/* group */
		Option_table_add_entry(option_table, "group", &region_path,
			command_data->root_region, set_cmzn_region_path);
		/* scene */
		Option_table_add_entry(option_table,"scene",&scene,
			command_data->root_region,set_Scene);
		/* default when token omitted (glyph) */
		Option_table_add_entry(option_table,(char *)NULL,&glyph,
			command_data->glyphmodule, set_Glyph);
		return_code = Option_table_multi_parse(option_table,state);
		/* no errors, not asking for help */
		if (return_code)
		{
			if (glyph)
			{
				if (!graphics_name)
				{
					graphics_name = cmzn_glyph_get_name(glyph);
				}
				cmzn_region_id region = cmzn_scene_get_region_internal(scene);
				cmzn_scene_id scene =
					cmzn_region_get_scene(region);
				return_code = cmzn_scene_add_glyph(scene, glyph, graphics_name);
				cmzn_scene_destroy(&scene);
			}
			else if (region_path)
			{
				display_message(ERROR_MESSAGE,
					"execute_command_gfx_draw.  gfx draw group command is no "
					"longer supported, cmgui will always draw graphics for regions");
			}
		} /* parse error,help */
		DESTROY(Option_table)(&option_table);
		cmzn_scene_destroy(&scene);
		if (region_path)
		{
			DEALLOCATE(region_path);
		}
		cmzn_glyph_destroy(&glyph);
		if (graphics_name)
		{
			DEALLOCATE(graphics_name);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"execute_command_gfx_draw.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_gfx_draw */

static int gfx_edit_graphics_object(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 3 March 2003

DESCRIPTION :
Executes a GFX EDIT GRAPHICS_OBJECT command.
==============================================================================*/
{
	int return_code;

	USE_PARAMETER(dummy_to_be_modified);
	struct cmzn_command_data *command_data;
	if (state && (command_data = (struct cmzn_command_data *)command_data_void))
	{
		/* initialize defaults */
		char apply_flag = 0;
		char *region_path = duplicate_string("/");
		char *coordinate_field_name = 0;

		struct Option_table *option_table = CREATE(Option_table)();
		/* apply_transformation */
		Option_table_add_entry(option_table, "apply_transformation", &apply_flag, NULL, set_char_flag);
		/* coordinate field name*/
		Option_table_add_string_entry(option_table, "coordinate_field", &coordinate_field_name, " FIELD_NAME");
		/* region path */
		Option_table_add_string_entry(option_table, "name", &region_path, " PATH_TO_REGION");
		/* default when token omitted (region path) */
		Option_table_add_entry(option_table, (const char *)NULL, &region_path, (void *)"PATH_TO_REGION", set_string);

		if (0 != (return_code = Option_table_multi_parse(option_table, state)))
		{
			struct cmzn_region *region = 0;
			if (!region_path)
			{
				display_message(ERROR_MESSAGE, "gfx edit graphics_object.  Must specify region");
				return_code = 0;
			}
			else if (!(cmzn_region_get_region_from_path_deprecated(command_data->root_region,
				region_path, &region) && (region)))
			{
				display_message(ERROR_MESSAGE, "gfx edit graphics_object.  Could not find region %s", region_path);
				return_code = 0;
			}
			else if (!apply_flag)
			{
				display_message(WARNING_MESSAGE, "gfx edit graphics_object.  Must specify 'apply_transformation' to do anything");
				return_code = 0;
			}
			else
			{
				cmzn_fieldmodule *fieldmodule = cmzn_region_get_fieldmodule(region);
				cmzn_fieldmodule_begin_change(fieldmodule);
				cmzn_scene *scene = cmzn_region_get_scene(region);
				cmzn_field *coordinate_field = 0;
				if (coordinate_field_name)
					coordinate_field = cmzn_fieldmodule_find_field_by_name(fieldmodule, coordinate_field_name);
				else
				{
					coordinate_field = cmzn_scene_guess_coordinate_field(scene, CMZN_FIELD_DOMAIN_TYPE_NODES);
					if (coordinate_field)
					{
						cmzn_field_access(coordinate_field);
						coordinate_field_name = cmzn_field_get_name(coordinate_field);
					}
				}
				cmzn_field *rc_coordinate_field = 0;
				const cmzn_field_coordinate_system_type coordinate_system_type = cmzn_field_get_coordinate_system_type(coordinate_field);
				if (coordinate_system_type == CMZN_FIELD_COORDINATE_SYSTEM_TYPE_RECTANGULAR_CARTESIAN)
				{
					rc_coordinate_field = cmzn_field_access(coordinate_field);
				}
				else
				{
					rc_coordinate_field = cmzn_fieldmodule_create_field_coordinate_transformation(fieldmodule, coordinate_field);
					cmzn_field_set_coordinate_system_type(rc_coordinate_field, CMZN_FIELD_COORDINATE_SYSTEM_TYPE_RECTANGULAR_CARTESIAN);
				}
				double mat[16];
				if (!coordinate_field)
				{
					display_message(ERROR_MESSAGE, "gfx edit graphics_object:  Could not find coordinate field %s",
						coordinate_field_name ? coordinate_field_name : "in region");
					return_code = 0;
				}
				else if (!rc_coordinate_field)
				{
					display_message(ERROR_MESSAGE, "gfx edit graphics_object:  Invalid coordinate field %s", coordinate_field_name);
					return_code = 0;
				}
				else if (!scene)
				{
					display_message(ERROR_MESSAGE, "gfx edit graphics_object:  Missing scene");
					return_code = 0;
				}
				else if (!cmzn_scene_has_transformation(scene))
				{
					return_code = 1; // nothing to do
				}
				else if (CMZN_OK != scene->getTransformationMatrixRowMajor(mat))
				{
					display_message(ERROR_MESSAGE, "gfx edit graphics_object:  Failed to get scene transformation matrix");
					return_code = 0;
				}
				else
				{
					cmzn_field *trans_field = cmzn_fieldmodule_create_field_constant(fieldmodule, 16, mat);
					cmzn_field *transformed_rc_coordinate_field = cmzn_fieldmodule_create_field_projection(fieldmodule, rc_coordinate_field, trans_field);
					cmzn_field_destroy(&trans_field);
					cmzn_field *transformed_coordinate_field = 0;
					if (coordinate_system_type == CMZN_FIELD_COORDINATE_SYSTEM_TYPE_RECTANGULAR_CARTESIAN)
					{
						transformed_coordinate_field = cmzn_field_access(transformed_rc_coordinate_field);
					}
					else
					{
						transformed_coordinate_field = cmzn_fieldmodule_create_field_coordinate_transformation(fieldmodule, transformed_rc_coordinate_field);
						cmzn_field_set_coordinate_system_type(transformed_coordinate_field, coordinate_system_type);
					}
					if (!transformed_coordinate_field)
					{
						display_message(ERROR_MESSAGE, "gfx edit graphics_object:  Failed to create transformed coordinate field");
						return_code = 0;
					}
					else
					{
						cmzn_field_domain_type domainTypes[] = { CMZN_FIELD_DOMAIN_TYPE_NODES, CMZN_FIELD_DOMAIN_TYPE_DATAPOINTS };
						const FE_value time = command_data->default_time_keeper_app->getTimeKeeper()->getTime();
						for (int i = 0; (i < 2) && return_code; ++i)
						{
							cmzn_nodeset_id nodeset = cmzn_fieldmodule_find_nodeset_by_field_domain_type(fieldmodule, domainTypes[i]);
							// Note following will transform value versions and derivatives iff coordinate field is finite element type
							const int result = cmzn_nodeset_assign_field_from_source(nodeset, coordinate_field,
								transformed_coordinate_field, /*conditional_field*/0, time);
							if ((CMZN_RESULT_OK != result) && (CMZN_RESULT_WARNING_PART_DONE != result))
							{
								display_message(ERROR_MESSAGE, "gfx edit graphics_object:  Failed to apply transformation");
								return_code = 0;
							}
							cmzn_nodeset_destroy(&nodeset);
						}
						if (return_code)
							cmzn_scene_clear_transformation(scene);
					}
					cmzn_field_destroy(&transformed_coordinate_field);
					cmzn_field_destroy(&transformed_rc_coordinate_field);
				}
				cmzn_scene_destroy(&scene);
				cmzn_field_destroy(&rc_coordinate_field);
				cmzn_field_destroy(&coordinate_field);
				cmzn_fieldmodule_end_change(fieldmodule);
				cmzn_fieldmodule_destroy(&fieldmodule);
			}
		}
		if (region_path)
			DEALLOCATE(region_path);
		if (coordinate_field_name)
			DEALLOCATE(coordinate_field_name);
		DESTROY(Option_table)(&option_table);
	}
	else
	{
		display_message(ERROR_MESSAGE, "gfx_edit_graphics_object.  Invalid argument(s)");
		return_code = 0;
	}
	return return_code;
}

#if defined (WX_USER_INTERFACE)
static int gfx_edit_scene(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 20 November 2000

DESCRIPTION :

Executes a GFX EDIT_SCENE command.  Brings up the Region_tree_viewer.
==============================================================================*/
{
	char close_flag;
	int return_code;
	struct cmzn_command_data *command_data;
	struct Option_table *option_table;

	ENTER(gfx_edit_scene);
	USE_PARAMETER(dummy_to_be_modified);
	if (state && (command_data = (struct cmzn_command_data *)command_data_void))
	{
		close_flag = 0;

		option_table = CREATE(Option_table)();
		/* close (editor) */
		Option_table_add_entry(option_table, "close", &close_flag,
			NULL, set_char_flag);
		if (0 != (return_code = Option_table_multi_parse(option_table, state)))
		{
			if (command_data->region_tree_viewer)
			{
				if (close_flag)
				{
					DESTROY(Region_tree_viewer)(&(command_data->region_tree_viewer));
				}
				else
				{
					Region_tree_viewer_bring_to_front(command_data->region_tree_viewer);
				}
			}
			else if (close_flag)
			{
				display_message(ERROR_MESSAGE,
					"gfx edit scene:  There is no scene editor to close");
				return_code = 0;
			}
			else
			{
				cmzn_material_id defaultMaterial =
					cmzn_materialmodule_get_default_material(command_data->materialmodule);
				if ((!command_data->user_interface) ||
					(!CREATE(Region_tree_viewer)(	&(command_data->region_tree_viewer),
						command_data->graphics_module,
						command_data->root_region,
						cmzn_materialmodule_get_manager(command_data->materialmodule),
						defaultMaterial,
						command_data->default_font,
						command_data->glyphmodule,
						command_data->spectrum_manager,
						command_data->volume_texture_manager,
						command_data->user_interface)))
				{
					display_message(ERROR_MESSAGE, "gfx_edit_scene.  "
						"Could not create scene editor");
					return_code = 0;
				}
				if (defaultMaterial)
					cmzn_material_destroy(&defaultMaterial);
			}
		} /* parse error, help */
		DESTROY(Option_table)(&option_table);
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_edit_scene.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* gfx_edit_scene */
#endif /* defined (WX_USER_INTERFACE) */

#if defined (WX_USER_INTERFACE)
static int gfx_edit_spectrum(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 13 August 2002

DESCRIPTION :
Executes a GFX EDIT SPECTRUM command.
Invokes the graphical spectrum group editor.
==============================================================================*/
{
	int return_code;
	struct cmzn_command_data *command_data;
	struct cmzn_spectrum *spectrum;
	struct Option_table *option_table;

	ENTER(gfx_edit_spectrum);
	USE_PARAMETER(dummy_to_be_modified);
	if (state && (command_data = (struct cmzn_command_data *)command_data_void))
	{
		/* initialize defaults */
		spectrum = (struct cmzn_spectrum *)NULL;
		option_table = CREATE(Option_table)();
		Option_table_add_entry(option_table, (const char *)NULL, &spectrum,
			command_data->spectrum_manager, set_Spectrum);
		if (0 != (return_code = Option_table_multi_parse(option_table, state)))
		{
			cmzn_region *spectrum_region = cmzn_region_create_region(command_data->root_region);
			return_code = bring_up_spectrum_editor_dialog(
				&(command_data->spectrum_editor_dialog),
				command_data->spectrum_manager, spectrum,
				command_data->default_font,
				command_data->graphics_buffer_package, command_data->user_interface,
				command_data->graphics_module,
				command_data->root_region,
				spectrum_region);
			cmzn_region_destroy(&spectrum_region);
		} /* parse error, help */
		DESTROY(Option_table)(&option_table);
		if (spectrum)
		{
			DEACCESS(cmzn_spectrum)(&spectrum);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_edit_spectrum.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* gfx_edit_spectrum */
#endif /* defined (WX_USER_INTERFACE) */

static int execute_command_gfx_edit(struct Parse_state *state,
	void *dummy_to_be_modified, void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 31 October 2001

DESCRIPTION :
Executes a GFX EDIT command.
==============================================================================*/
{
	int return_code;
	struct cmzn_command_data *command_data;
	struct Option_table *option_table;

	ENTER(execute_command_gfx_edit);
	USE_PARAMETER(dummy_to_be_modified);
	if (state && (command_data = (struct cmzn_command_data *)command_data_void))
	{
		if (state->current_token)
		{
			option_table = CREATE(Option_table)();
			Option_table_add_entry(option_table, "graphics_object", NULL,
				command_data_void, gfx_edit_graphics_object);
#if defined (WX_USER_INTERFACE)
			Option_table_add_entry(option_table, "scene", NULL,
				command_data_void, gfx_edit_scene);
#endif /* if defined (WX_USER_INTERFACE) */
#if defined (WX_USER_INTERFACE)
			Option_table_add_entry(option_table, "spectrum", NULL,
				command_data_void, gfx_edit_spectrum);
#endif /* defined (WX_USER_INTERFACE) */
			return_code = Option_table_parse(option_table, state);
			DESTROY(Option_table)(&option_table);
		}
		else
		{
			set_command_prompt("gfx edit", command_data);
			return_code = 1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"execute_command_gfx_edit.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_gfx_edit */

#if defined (WX_USER_INTERFACE)
static int execute_command_gfx_element_creator(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 3 March 2003

DESCRIPTION :
Executes a GFX ELEMENT_CREATOR command.
==============================================================================*/
{
	int return_code;
	ENTER(execute_command_gfx_element_creator);
	USE_PARAMETER(dummy_to_be_modified);
#if defined (WX_USER_INTERFACE)
	USE_PARAMETER(state);
	USE_PARAMETER(command_data_void);
	display_message(INFORMATION_MESSAGE,
		"\nElement creator has been moved to node tool in the graphics window in cmgui-wx.\n"
		"Please use gfx node_tool command instead.\n");
	return_code = 1;
#endif /*defined (WX_USER_INTERFACE) */
	return (return_code);
} /* execute_command_gfx_element_creator */
#endif /* defined (WX_USER_INTERFACE) */

#if defined (GTK_USER_INTERFACE) || defined (WIN32_USER_INTERFACE) || defined (CARBON_USER_INTERFACE) || defined (WX_USER_INTERFACE)
static int execute_command_gfx_element_point_tool(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 5 July 2002

DESCRIPTION :
Executes a GFX ELEMENT_POINT_TOOL command.
==============================================================================*/
{
	static const char *(dialog_strings[2]) = {"open_dialog", "close_dialog"};
	const char *dialog_string;
	int return_code;
	struct cmzn_command_data *command_data;
	struct Computed_field *command_field;
	struct Element_point_tool *element_point_tool;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_command_field_data;

	ENTER(execute_command_gfx_element_point_tool);
	USE_PARAMETER(dummy_to_be_modified);
	if (state && (command_data = (struct cmzn_command_data *)command_data_void))
	{
		/* initialize defaults */
		if (NULL != (element_point_tool=command_data->element_point_tool))
		{
			command_field = Element_point_tool_get_command_field(element_point_tool);
		}
		else
		{
			command_field = (struct Computed_field *)NULL;
		}
		if (command_field)
		{
			ACCESS(Computed_field)(command_field);
		}
		option_table = CREATE(Option_table)();
		/* open_dialog/close_dialog */
		dialog_string = (char *)NULL;
		Option_table_add_enumerator(option_table, /*number_of_valid_strings*/2,
			dialog_strings, &dialog_string);
		/* command_field */
		set_command_field_data.computed_field_manager=
			Computed_field_package_get_computed_field_manager(
				command_data->computed_field_package);
		set_command_field_data.conditional_function =
			Computed_field_has_string_value_type;
		set_command_field_data.conditional_function_user_data = (void *)NULL;
		Option_table_add_entry(option_table, "command_field", &command_field,
			&set_command_field_data, set_Computed_field_conditional);
		if (0 != (return_code = Option_table_multi_parse(option_table, state)))
		{
			if (element_point_tool)
			{
				Element_point_tool_set_command_field(element_point_tool,command_field);
				if (dialog_string == dialog_strings[0])
				{
					Element_point_tool_pop_up_dialog(element_point_tool, (struct Graphics_window *)NULL);
				}
#if defined (WX_USER_INTERFACE)
			FOR_EACH_OBJECT_IN_MANAGER(Graphics_window)(
				 Graphics_window_update_Interactive_tool,
				 (void *)Element_point_tool_get_interactive_tool(element_point_tool),
				 command_data->graphics_window_manager);
#endif
			cmzn_sceneviewermodule_update_Interactive_tool(
				command_data->sceneviewermodule,
				Element_point_tool_get_interactive_tool(element_point_tool));
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"execute_command_gfx_element_point_tool.  "
					"Missing element_point_tool");
				return_code = 0;
			}
		} /* parse error,help */
		DESTROY(Option_table)(&option_table);
		if (command_field)
		{
			DEACCESS(Computed_field)(&command_field);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"execute_command_gfx_element_point_tool.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_gfx_element_point_tool */
#endif /* defined (GTK_USER_INTERFACE) || defined (WIN32_USER_INTERFACE) || defined (CARBON_USER_INTERFACE) */

#if defined (GTK_USER_INTERFACE) || defined (WIN32_USER_INTERFACE) || defined (CARBON_USER_INTERFACE) || defined (WX_USER_INTERFACE)
static int execute_command_gfx_element_tool(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 5 July 2002

DESCRIPTION :
Executes a GFX ELEMENT_TOOL command.
==============================================================================*/
{
	static const char *(dialog_strings[2]) = {"open_dialog", "close_dialog"};
	const char *dialog_string;
	int select_elements_enabled,select_faces_enabled,select_lines_enabled,
		return_code;
	struct cmzn_command_data *command_data;
	struct Computed_field *command_field;
	struct Element_tool *element_tool;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_command_field_data;

	ENTER(execute_command_gfx_element_tool);
	USE_PARAMETER(dummy_to_be_modified);
	if (state&&(command_data=(struct cmzn_command_data *)command_data_void))
	{
		/* initialize defaults */
		if (NULL != (element_tool = command_data->element_tool))
		{
			select_elements_enabled = Element_tool_get_select_elements_enabled(element_tool);
			select_faces_enabled = Element_tool_get_select_faces_enabled(element_tool);
			select_lines_enabled = Element_tool_get_select_lines_enabled(element_tool);
			command_field=Element_tool_get_command_field(element_tool);
		}
		else
		{
			select_elements_enabled=1;
			select_faces_enabled=1;
			select_lines_enabled=1;
			command_field = (struct Computed_field *)NULL;
		}
		if (command_field)
		{
			ACCESS(Computed_field)(command_field);
		}
		option_table=CREATE(Option_table)();
		/* open_dialog/close_dialog */
		dialog_string = (char *)NULL;
		Option_table_add_enumerator(option_table, /*number_of_valid_strings*/2,
			dialog_strings, &dialog_string);
		/* select_elements/no_select_elements */
		Option_table_add_switch(option_table,"select_elements","no_select_elements",
			&select_elements_enabled);
		/* select_faces/no_select_faces */
		Option_table_add_switch(option_table,"select_faces","no_select_faces",
			&select_faces_enabled);
		/* select_lines/no_select_lines */
		Option_table_add_switch(option_table,"select_lines","no_select_lines",
			&select_lines_enabled);
		/* command_field */
		set_command_field_data.computed_field_manager=
			Computed_field_package_get_computed_field_manager(
				command_data->computed_field_package);
		set_command_field_data.conditional_function =
			Computed_field_has_string_value_type;
		set_command_field_data.conditional_function_user_data=(void *)NULL;
		Option_table_add_entry(option_table,"command_field",&command_field,
			&set_command_field_data,set_Computed_field_conditional);
		if (0 != (return_code = Option_table_multi_parse(option_table, state)))
		{
			if (element_tool)
			{
				Element_tool_set_select_elements_enabled(element_tool, 0 != select_elements_enabled);
				Element_tool_set_select_faces_enabled(element_tool, 0 != select_faces_enabled);
				Element_tool_set_select_lines_enabled(element_tool, 0 != select_lines_enabled);
				Element_tool_set_command_field(element_tool,command_field);
				if (dialog_string == dialog_strings[0])
				{
					Element_tool_pop_up_dialog(element_tool, (struct Graphics_window *)NULL);
				}
#if defined (WX_USER_INTERFACE)
			FOR_EACH_OBJECT_IN_MANAGER(Graphics_window)(
				 Graphics_window_update_Interactive_tool,
				 (void *)Element_tool_get_interactive_tool(element_tool),
				 command_data->graphics_window_manager);
#endif /*(WX_USER_INTERFACE)*/
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"execute_command_gfx_element_tool.  Missing element_tool");
				return_code=0;
			}
		} /* parse error,help */
		DESTROY(Option_table)(&option_table);
		if (command_field)
		{
			DEACCESS(Computed_field)(&command_field);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"execute_command_gfx_element_tool.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_gfx_element_tool */
#endif /* defined (GTK_USER_INTERFACE) || defined (WIN32_USER_INTERFACE) || defined (CARBON_USER_INTERFACE) || defined (WX_USER_INTERFACE) */

static int gfx_export_alias(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 16 June 1999

DESCRIPTION :
Executes a GFX EXPORT ALIAS command.
==============================================================================*/
{
#if defined (NEW_ALIAS)
	char destroy_when_saved,*default_filename="cmgui_wire",*file_name,
		*retrieve_filename,save_now,write_sdl;
	float frame_in,frame_out,view_frame;
#endif /* defined (NEW_ALIAS) */
	int return_code;
	struct cmzn_command_data *command_data;
	struct Scene *scene;
	static struct Modifier_entry option_table[]=
	{
#if defined (NEW_ALIAS)
		{"dont_save_now",NULL,NULL,unset_char_flag},
		{"frame_in",NULL,NULL,set_float},
		{"frame_out",NULL,NULL,set_float},
		{"keep",NULL,NULL,unset_char_flag},
		{"retrieve",NULL,NULL,set_file_name},
#endif /* defined (NEW_ALIAS) */
		{"scene",NULL,NULL,set_Scene},
#if defined (NEW_ALIAS)
		{"sdl",NULL,NULL,set_char_flag},
		{"viewframe",NULL,NULL,set_float},
		{NULL,NULL,NULL,set_file_name}
#else /* defined (NEW_ALIAS) */
		{NULL,NULL,NULL,NULL}
#endif /* defined (NEW_ALIAS) */
	};

	ENTER(gfx_export_alias);
	USE_PARAMETER(dummy_to_be_modified);
	/* check argument */
	if (state)
	{
		if (NULL != (command_data = (struct cmzn_command_data *)command_data_void))
		{
			/* initialize defaults */
#if defined (NEW_ALIAS)
			file_name=(char *)NULL;
			frame_in=0;
			frame_out=0;
			retrieve_filename=(char *)NULL;
			save_now=1;
			view_frame=0;
			write_sdl=0;
			destroy_when_saved=1;
#endif /* defined (NEW_ALIAS) */
			scene=ACCESS(Scene)(command_data->default_scene);
#if defined (NEW_ALIAS)
			(option_table[0]).to_be_modified= &save_now;
			(option_table[1]).to_be_modified= &frame_in;
			(option_table[2]).to_be_modified= &frame_out;
			(option_table[3]).to_be_modified= &destroy_when_saved;
			(option_table[4]).to_be_modified= &retrieve_filename;
			(option_table[5]).to_be_modified= &scene;
			(option_table[5]).user_data=command_data->scene_manager;
			(option_table[6]).to_be_modified= &write_sdl;
			(option_table[7]).to_be_modified= &view_frame;
			(option_table[8]).to_be_modified= &file_name;
#else /* defined (NEW_ALIAS) */
			(option_table[0]).to_be_modified= &scene;
			(option_table[0]).user_data=command_data->scene_manager;
#endif /* defined (NEW_ALIAS) */
			return_code=process_multiple_options(state,option_table);
			/* no errors, not asking for help */
			if (return_code)
			{
				if (scene)
				{
#if defined (NEW_ALIAS)
					if (!file_name)
					{
						file_name=default_filename;
					}
					if (write_sdl)
					{
						export_to_alias_sdl(scene,file_name,retrieve_filename,view_frame);
					}
					else
					{
						export_to_alias_frames(scene,file_name,frame_in,frame_out,save_now,
							destroy_when_saved);
					}
#else /* defined (NEW_ALIAS) */
					display_message(ERROR_MESSAGE,"gfx_export_alias.  The old gfx export alias is superseeded by gfx export wavefront");
					return_code=0;
#endif /* defined (NEW_ALIAS) */
				}
			} /* parse error,help */
			if (scene)
			{
				DEACCESS(Scene)(&scene);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"gfx_export_alias.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_export_alias.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_export_alias */

/**
 * Executes a GFX EXPORT CM command.
 */
static int gfx_export_cm(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
{
	int return_code;
	USE_PARAMETER(dummy_to_be_modified);
	cmzn_command_data *command_data = static_cast<cmzn_command_data *>(command_data_void);
	if (state && command_data)
	{
		char *coordinate_field_name = 0;
		char *ipbase_filename = 0;
		char *ipcoor_filename = 0;
		char *ipelem_filename = 0;
		char *ipmap_filename = 0;
		char *ipnode_filename = 0;
		cmzn_region *region = cmzn_region_access(command_data->root_region);
		cmzn_field_group *group = 0;

		Option_table *option_table = CREATE(Option_table)();
		Option_table_add_help(option_table,
			"Export the definition of a coordinate field from the specified region "
			"(and optional group within the region) to CMISS IP format files. "
			"The ipmap filename is optional, all other filenames are required. "
			"Note this function cannot export all field representations.");
		Option_table_add_string_entry(option_table, "field", &coordinate_field_name, " FIELD_NAME");
		Option_table_add_name_entry(option_table, "ipcoor_filename", &ipcoor_filename);
		Option_table_add_name_entry(option_table, "ipbase_filename", &ipbase_filename);
		Option_table_add_name_entry(option_table, "ipelem_filename", &ipelem_filename);
		Option_table_add_name_entry(option_table, "ipmap_filename", &ipmap_filename);
		Option_table_add_name_entry(option_table, "ipnode_filename", &ipnode_filename);
		Option_table_add_region_or_group_entry(option_table, "region", &region, &group);

		if (0 != (return_code = Option_table_multi_parse(option_table, state)))
		{
			if (!(ipcoor_filename && ipbase_filename && ipnode_filename && ipelem_filename))
			{
				display_message(ERROR_MESSAGE,
					"You must specify all of ipcoor_filename, ipbase_filename, ipnode_filename and ipelem_filename.");
				return_code = 0;
			}
			cmzn_fieldmodule *fieldmodule = cmzn_region_get_fieldmodule(region);
			cmzn_field *field = cmzn_fieldmodule_find_field_by_name(fieldmodule, coordinate_field_name);
			cmzn_field_finite_element *fe_field = cmzn_field_cast_finite_element(field);
			if (fe_field)
				cmzn_field_finite_element_destroy(&fe_field);
			else
			{
				display_message(ERROR_MESSAGE,
					"You must specify a finite element field as the coordinate field to export.");
				return_code = 0;
			}
			cmzn_fieldmodule_destroy(&fieldmodule);

			FILE *ipbase_file, *ipcoor_file, *ipelem_file, *ipmap_file, *ipnode_file;
			if (return_code)
			{
				if (!(ipcoor_file = fopen(ipcoor_filename, "w")))
				{
					display_message(ERROR_MESSAGE,
						"Unable to open ipcoor_filename %s.", ipcoor_filename);
					return_code = 0;
				}
				if (!(ipbase_file = fopen(ipbase_filename, "w")))
				{
					display_message(ERROR_MESSAGE,
						"Unable to open ipbase_filename %s.", ipbase_filename);
					return_code = 0;
				}
				if (!(ipnode_file = fopen(ipnode_filename, "w")))
				{
					display_message(ERROR_MESSAGE,
						"Unable to open ipnode_filename %s.", ipnode_filename);
					return_code = 0;
				}
				if (!(ipelem_file = fopen(ipelem_filename, "w")))
				{
					display_message(ERROR_MESSAGE,
						"Unable to open ipelem_filename %s.", ipelem_filename);
					return_code = 0;
				}
				if (ipmap_filename)
				{
					if (!(ipmap_file = fopen(ipmap_filename, "w")))
					{
						display_message(ERROR_MESSAGE,
							"Unable to open ipmap_filename %s.", ipmap_filename);
						return_code = 0;
					}
				}
				else
				{
					ipmap_file = (FILE *)NULL;
				}
			}
			if (return_code)
			{
				int result = write_cm_files(ipcoor_file, ipbase_file, ipnode_file, ipelem_file,
					ipmap_file, region, group, field);
				if (result != CMZN_OK)
					return_code = 0;
				fclose(ipcoor_file);
				fclose(ipbase_file);
				fclose(ipnode_file);
				fclose(ipelem_file);
				if (ipmap_file)
				{
					fclose(ipmap_file);
				}
			}
			cmzn_field_destroy(&field);
		}
		DESTROY(Option_table)(&option_table);
		if (ipbase_filename)
		{
			DEALLOCATE(ipbase_filename);
		}
		if (ipcoor_filename)
		{
			DEALLOCATE(ipcoor_filename);
		}
		if (ipelem_filename)
		{
			DEALLOCATE(ipelem_filename);
		}
		if (ipmap_filename)
		{
			DEALLOCATE(ipmap_filename);
		}
		if (ipnode_filename)
		{
			DEALLOCATE(ipnode_filename);
		}
		if (coordinate_field_name)
			DEALLOCATE(coordinate_field_name);
		cmzn_field_group_destroy(&group);
		cmzn_region_destroy(&region);
	}
	else
	{
		display_message(ERROR_MESSAGE, "gfx_export_cm.  Invalid argument(s)");
		return_code = 0;
	}

	LEAVE;

	return (return_code);
} /* gfx_export_cm */

static int gfx_export_iges(struct Parse_state *state,void *dummy_to_be_modified,
	void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 4 March 2003

DESCRIPTION :
Executes a GFX EXPORT IGES command.
==============================================================================*/
{
	char *file_name, *region_path;
	int return_code;
	struct Computed_field *coordinate_field;
	struct cmzn_command_data *command_data = (struct cmzn_command_data *)command_data_void;
	struct cmzn_region *region;
	struct Option_table *option_table;

	ENTER(gfx_export_iges);
	USE_PARAMETER(dummy_to_be_modified);
	return_code = 0;
	if (state && (command_data != 0))
	{
		return_code=1;
		/* initialize defaults */
		region_path = cmzn_region_get_root_region_path();
		coordinate_field = (struct Computed_field *)NULL;
		char *coordinate_field_name = 0;
		file_name = (char *)NULL;

		option_table = CREATE(Option_table)();
		/* coordinate_field */
		Option_table_add_entry(option_table,"coordinate_field",&coordinate_field_name,
			(void *)1,set_name);
		/* group */
		Option_table_add_entry(option_table, "group", &region_path,
			command_data->root_region, set_cmzn_region_path);
		/* default option: file name */
		Option_table_add_default_string_entry(option_table, &file_name, "FILE_NAME");

		/* no errors, not asking for help */
		if (0 != (return_code = Option_table_multi_parse(option_table, state)))
		{
			if (cmzn_region_get_region_from_path_deprecated(command_data->root_region,
				region_path, &region))
			{
				if (region && coordinate_field_name &&
					(0 != (coordinate_field = FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field,name)(
							coordinate_field_name, cmzn_region_get_Computed_field_manager(region)))))
				{
					if (Computed_field_has_3_components(coordinate_field, NULL))
					{
						cmzn_field_access(coordinate_field);
					}
					else
					{
						coordinate_field = NULL;
						display_message(ERROR_MESSAGE,
							"gfx_export_iges.  "
							"Field specified is not a coordinate field.");
						return_code = 0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"gfx_export_iges.  "
						"Field %s does not exist.", coordinate_field_name);
						return_code = 0;
				}
				if (return_code)
				{
					if (!file_name)
					{
						file_name = confirmation_get_write_filename(".igs",
							 command_data->user_interface
#if defined (WX_USER_INTERFACE)
							 , command_data->execute_command
#endif /* defined (WX_USER_INTERFACE)*/
);
					}
					if (file_name)
					{
						return_code = check_suffix(&file_name, ".igs");
						if (return_code)
						{
							return_code = export_to_iges(file_name, region, region_path,
								coordinate_field);
						}
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE, "gfx_export_iges.  Invalid region");
			}
		} /* parse error,help */
		if (coordinate_field)
		{
			DEACCESS(Computed_field)(&coordinate_field);
		}
		DESTROY(Option_table)(&option_table);
		DEALLOCATE(coordinate_field_name);
		DEALLOCATE(region_path);
		DEALLOCATE(file_name);
	}
	else
	{
		display_message(ERROR_MESSAGE, "gfx_export_iges.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* gfx_export_iges */

static int gfx_export_stl(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 4 July 2008

DESCRIPTION :
Executes a GFX EXPORT STL command.
==============================================================================*/
{
	char *file_name;
	int return_code;
	struct cmzn_command_data *command_data;
	struct Option_table *option_table;
	struct Scene *scene;

	ENTER(gfx_export_stl);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (NULL != (command_data = (struct cmzn_command_data *)command_data_void))
		{
			file_name = (char *)NULL;
			scene = cmzn_scene_access(command_data->default_scene);
			cmzn_scenefilter_id filter =
				cmzn_scenefiltermodule_get_default_scenefilter(command_data->filter_module);
			option_table = CREATE(Option_table)();
			/* file */
			Option_table_add_entry(option_table, "file", &file_name,
				(void *)1, set_name);
			/* scene */
			Option_table_add_entry(option_table,"scene",&scene,
				command_data->root_region, set_Scene);
			Option_table_add_entry(option_table, "filter", &filter,
				command_data->filter_module, set_cmzn_scenefilter);
			if (0 != (return_code = Option_table_multi_parse(option_table, state)))
			{
				if (scene)
				{
					if (file_name)
					{
						return_code = export_to_stl(file_name, scene, filter);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"gfx export stl.  Must specify file name");
						return_code = 0;
					}
				}
			}
			DESTROY(Option_table)(&option_table);
			if (scene)
				cmzn_scene_destroy(&scene);
			if (filter)
				cmzn_scenefilter_destroy(&filter);
			if (file_name)
			{
				DEALLOCATE(file_name);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_export_stl.  Invalid argument(s)");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_export_stl.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_export_stl */

static int gfx_export_threejs(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
{
	char *file_prefix;
	int return_code;
	struct cmzn_command_data *command_data;
	struct Option_table *option_table;
	struct Scene *scene;

	if (state)
	{
		if (NULL != (command_data = (struct cmzn_command_data *)command_data_void))
		{
			file_prefix = (char *)NULL;
			scene = cmzn_scene_access(command_data->default_scene);
			double begin_time = 0.0, end_time = 0.0;
			int number_of_time_steps = 0;
			enum cmzn_streaminformation_scene_io_data_type export_mode =
				CMZN_STREAMINFORMATION_SCENE_IO_DATA_TYPE_COLOUR;
			char morphVertices = 0,  morphColours = 0, morphNormals = 0;
			cmzn_scenefilter_id filter =
				cmzn_scenefiltermodule_get_default_scenefilter(command_data->filter_module);
			option_table = CREATE(Option_table)();
			Option_table_add_help(option_table,
				"Creates a json file containing data of primitives readable using ThreeJS for "
				"web browser rendering of 3D surfaces. [file_prefix] dictates the export file's name, "
				"name of the export will be in the following format - [file_prefix]_[region]_[graphicsname].json. "
				"[start_time], [end_time] and [number_of_time_steps] must be used together; when specified "
				"animated mesh will be generated. "
				"[data_export_colour] will export the data on per vertices basis as spectrum colours when data and spectrum are specified for surface graphics. "
				"[data_export_per_vertex_value] will export the data for each vertex as hex when data is specified on graphics. "
				"[data_export_per_face_value] will export the data for each face as hex when data is specified on graphics, this should be used with per element constant field (e.g ID). "
				"must be used together; when specified "
				"animated mesh will be generated. "
				"[scene] generates the exports for this scene and its children. "
				"[filter] applies the filter the provided scene."
				"[morph_vertices] determines rather vertices will be output for each time step;"
				"[morph_colours] determines rather colours will be output for each time step; "
				"[morph_normals] determines rather normals will be output for each time step; ");
			/* file */
			Option_table_add_entry(option_table, "file_prefix", &file_prefix,
				(void *)1, set_name);
			Option_table_add_entry(option_table,"start_time",&begin_time,
				NULL,set_double);
			Option_table_add_entry(option_table,"end_time",&end_time,
				NULL,set_double);
			Option_table_add_entry(option_table, "number_of_time_steps", &number_of_time_steps,
				&number_of_time_steps, set_int_non_negative);
			OPTION_TABLE_ADD_ENUMERATOR(cmzn_streaminformation_scene_io_data_type)(option_table,
				&export_mode);
			Option_table_add_char_flag_entry(option_table,
				"morph_vertices", &morphVertices);
			Option_table_add_char_flag_entry(option_table,
				"morph_colours", &morphColours);
			Option_table_add_char_flag_entry(option_table,
				"morph_normals", &morphNormals);
			/* scene */
			Option_table_add_entry(option_table,"scene",&scene,
				command_data->root_region, set_Scene);
			Option_table_add_entry(option_table, "filter", &filter,
				command_data->filter_module, set_cmzn_scenefilter);
			if (0 != (return_code = Option_table_multi_parse(option_table, state)))
			{
				if (scene)
				{
					if (file_prefix)
					{
						return_code = scene_app_export_threejs(scene, filter, file_prefix,
							(int)number_of_time_steps, begin_time, end_time, export_mode,
							morphVertices ? 1 : 0, morphColours ? 1 :0, morphNormals ? 1 : 0);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"gfx export threejs.  Must specify file name");
						return_code = 0;
					}
				}
			}
			DESTROY(Option_table)(&option_table);
			if (scene)
				cmzn_scene_destroy(&scene);
			if (filter)
				cmzn_scenefilter_destroy(&filter);
			if (file_prefix)
			{
				DEALLOCATE(file_prefix);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_export_stl.  Invalid argument(s)");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_export_stl.  Missing state");
		return_code=0;
	}

	return (return_code);
}

static int gfx_export_webgl(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 4 July 2008

DESCRIPTION :
Executes a GFX EXPORT STL command.
==============================================================================*/
{
	char *file_name;
	int return_code;
	struct cmzn_command_data *command_data;
	struct Option_table *option_table;
	struct Scene *scene;

	if (state)
	{
		if (NULL != (command_data = (struct cmzn_command_data *)command_data_void))
		{
			file_name = (char *)NULL;
			scene = cmzn_scene_access(command_data->default_scene);
			cmzn_scenefilter_id filter =
				cmzn_scenefiltermodule_get_default_scenefilter(command_data->filter_module);
			option_table = CREATE(Option_table)();
			/* file */
			Option_table_add_entry(option_table, "file", &file_name,
				(void *)1, set_name);
			/* scene */
			Option_table_add_entry(option_table,"scene",&scene,
				command_data->root_region, set_Scene);
			Option_table_add_entry(option_table, "filter", &filter,
				command_data->filter_module, set_cmzn_scenefilter);
			if (0 != (return_code = Option_table_multi_parse(option_table, state)))
			{
				if (scene)
				{
					if (file_name)
					{
						return_code = Scene_render_webgl(scene, filter, file_name);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"gfx export webgl.  Must specify file name");
						return_code = 0;
					}
				}
			}
			DESTROY(Option_table)(&option_table);
			if (scene)
				cmzn_scene_destroy(&scene);
			if (filter)
				cmzn_scenefilter_destroy(&filter);
			if (file_name)
			{
				DEALLOCATE(file_name);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_export_stl.  Invalid argument(s)");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_export_stl.  Missing state");
		return_code=0;
	}

	return (return_code);
}

static int gfx_export_vrml(struct Parse_state *state,void *dummy_to_be_modified,
	void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 16 June 1999

DESCRIPTION :
Executes a GFX EXPORT VRML command.
==============================================================================*/
{
	char *file_name;
	int return_code;
	struct Option_table *option_table;
	struct cmzn_command_data *command_data;
	struct Scene *scene;

	ENTER(gfx_export_vrml);
	USE_PARAMETER(dummy_to_be_modified);
	/* check argument */
	if (state)
	{
		if (NULL != (command_data = (struct cmzn_command_data *)command_data_void))
		{
			/* initialize defaults */
			file_name=(char *)NULL;
			cmzn_scenefilter_id filter =
				cmzn_scenefiltermodule_get_default_scenefilter(command_data->filter_module);
			scene = cmzn_scene_access(command_data->default_scene);
			option_table = CREATE(Option_table)();
			Option_table_add_entry(option_table, "file", &file_name, (void *)1, set_name);
			Option_table_add_entry(option_table,"scene",&scene,
				command_data->root_region,set_Scene);
			Option_table_add_entry(option_table, "filter", &filter,
				command_data->filter_module, set_cmzn_scenefilter);
			return_code = Option_table_multi_parse(option_table, state);
			DESTROY(Option_table)(&option_table);
			/* no errors, not asking for help */
			if (return_code)
			{
				if (!file_name)
				{
					file_name=confirmation_get_write_filename(".wrl",
						 command_data->user_interface
#if defined (WX_USER_INTERFACE)
							 , command_data->execute_command
#endif /* defined (WX_USER_INTERFACE)*/
																										);
				}
				if (scene)
				{
					if (file_name)
					{
						if (0 != (return_code = check_suffix(&file_name, ".wrl")))
						{
							return_code=export_to_vrml(file_name,scene, filter);
						}
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,"gfx_export_vrml.  Specified scene not found");
					return_code = 0;
				}
			} /* parse error,help */

			cmzn_scene_destroy(&scene);
			if (filter)
				cmzn_scenefilter_destroy(&filter);
			if (file_name)
				DEALLOCATE(file_name);
		}
		else
		{
			display_message(ERROR_MESSAGE,"gfx_export_vrml.  Invalid argument(s)");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_export_vrml.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_export_vrml */

static int gfx_export_wavefront(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 16 June 1999

DESCRIPTION :
Executes a GFX EXPORT WAVEFRONT command.
==============================================================================*/
{
	const char *file_ext = ".obj";
	char *file_name,full_comments;
	int frame_number, number_of_frames, return_code, version;
	struct cmzn_command_data *command_data;
	struct Scene *scene = NULL;
	cmzn_scenefilter_id filter = 0;
	static struct Modifier_entry option_table[]=
	{
		{"file",NULL,(void *)1,set_name},
		{"frame_number",NULL,NULL,set_int_non_negative},
		{"full_comments",NULL,NULL,set_char_flag},
		{"number_of_frames",NULL,NULL,set_int_positive},
		{"scene",NULL,NULL,set_Scene},
		{"version",NULL,NULL,set_int_positive},
		{"filter",NULL,NULL,set_cmzn_scenefilter},
		{NULL,NULL,NULL,NULL}
	};
	ENTER(gfx_export_wavefront);
	USE_PARAMETER(dummy_to_be_modified);
	/* check argument */
	if (state)
	{
		if (NULL != (command_data = (struct cmzn_command_data *)command_data_void))
		{
			/* initialize defaults */
			file_name=(char *)NULL;
			frame_number = 0;
			full_comments=0;
			number_of_frames=100;
			scene=cmzn_scene_access(command_data->default_scene);
			filter=cmzn_scenefiltermodule_get_default_scenefilter(command_data->filter_module);
			version=3;
			(option_table[0]).to_be_modified= &file_name;
			(option_table[1]).to_be_modified= &frame_number;
			(option_table[2]).to_be_modified= &full_comments;
			(option_table[3]).to_be_modified= &number_of_frames;
			(option_table[4]).to_be_modified= &scene;
			(option_table[4]).user_data=command_data->scene_manager;
			(option_table[5]).to_be_modified= &version;
			(option_table[6]).to_be_modified= &filter;
			return_code=process_multiple_options(state,option_table);
			/* no errors, not asking for help */

			if (return_code)
			{
				if (!file_name)
				{
					file_name=confirmation_get_write_filename(".obj",
						 command_data->user_interface
#if defined (WX_USER_INTERFACE)
							 , command_data->execute_command
#endif /* defined (WX_USER_INTERFACE)*/
																										);
				}
				if (file_name)
				{
					return_code = check_suffix(&file_name, file_ext);
					if (!return_code)
					{
						strcat(file_name, file_ext);
					}
					return_code=export_to_wavefront(file_name, scene, filter, full_comments);
				}
			} /* parse error,help */
			cmzn_scene_destroy(&scene);
			cmzn_scenefilter_destroy(&filter);
			if (file_name)
				DEALLOCATE(file_name);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_export_wavefront.  Invalid argument(s)");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_export_wavefront.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_export_wavefront */

#if defined (USE_OPENCASCADE)
static struct cmzn_spectrumcomponent *create_spectrum_component( cmzn_spectrumcomponent_colour_mapping_type colour )
{
	int component = 1;
	struct cmzn_spectrumcomponent *settings = CREATE(cmzn_spectrumcomponent)();
	cmzn_spectrumcomponent_set_scale_type(settings, CMZN_SPECTRUMCOMPONENT_SCALE_TYPE_LINEAR);
	cmzn_spectrumcomponent_set_colour_mapping_type(settings, colour);
	cmzn_spectrumcomponent_set_extend_above(settings, true);
	cmzn_spectrumcomponent_set_extend_below_flag(settings, true);
	cmzn_spectrumcomponent_set_colour_reverse(settings, false);

	if ( colour == CMZN_SPECTRUMCOMPONENT_COLOUR_MAPPING_TYPE_RED )
		component = 1;
	else if ( colour == CMZN_SPECTRUMCOMPONENT_COLOUR_MAPPING_TYPE_GREEN )
		component = 2;
	else
		component = 3;

	cmzn_spectrumcomponent_set_field_component( settings, component );

	return settings;
}

static int create_RGB_spectrum( struct cmzn_spectrum **spectrum, void *command_data_void )
{
	int return_code = 0, number_in_list = 0;
	struct LIST(cmzn_spectrumcomponent) *spectrum_settings_list;
	struct cmzn_spectrumcomponent *red_settings;
	struct cmzn_spectrumcomponent *green_settings;
	struct cmzn_spectrumcomponent *blue_settings;
	struct cmzn_command_data *command_data = (struct cmzn_command_data *)command_data_void;

	if ( command_data && (NULL != ((*spectrum) = cmzn_spectrum_create_private())) &&
		cmzn_spectrum_set_name(spectrum, "RGB"))
	{
		spectrum_settings_list = get_cmzn_spectrumcomponent_list( (*spectrum) );
		number_in_list = NUMBER_IN_LIST(cmzn_spectrumcomponent)(spectrum_settings_list);
		if ( number_in_list > 0 )
		{
			REMOVE_ALL_OBJECTS_FROM_LIST(cmzn_spectrumcomponent)(spectrum_settings_list);
		}
		red_settings = create_spectrum_component( CMZN_SPECTRUMCOMPONENT_COLOUR_MAPPING_TYPE_RED );
		cmzn_spectrumcomponent_add( red_settings, /* end of list = 0 */0,
			spectrum_settings_list );

		green_settings = create_spectrum_component( CMZN_SPECTRUMCOMPONENT_COLOUR_MAPPING_TYPE_GREEN );
		cmzn_spectrumcomponent_add( green_settings, /* end of list = 0 */0,
			spectrum_settings_list );

		blue_settings = create_spectrum_component( CMZN_SPECTRUMCOMPONENT_COLOUR_MAPPING_TYPE_BLUE );
		cmzn_spectrumcomponent_add( blue_settings, /* end of list = 0 */0,
			spectrum_settings_list );

		Spectrum_calculate_range( (*spectrum) );
		Spectrum_calculate_range( (*spectrum) );
		Spectrum_set_minimum_and_maximum( (*spectrum), 0, 1);
		cmzn_spectrum_set_material_overwrite( (*spectrum), 0 );
		if (!ADD_OBJECT_TO_MANAGER(cmzn_spectrum)( (*spectrum),
				command_data->spectrum_manager))
		{
			cmzn_spectrum_destroy(&command_data->spectrum_manager);
		}
		else
		{
			return_code = 1;
		}
	}
	return return_code;
}

static int execute_command_gfx_import(struct Parse_state *state,
	void *dummy_to_be_modified, void *command_data_void )
{
	int return_code;
	struct cmzn_command_data *command_data;

	ENTER(execute_command_gfx_import);

	USE_PARAMETER(dummy_to_be_modified);
	if (state && (command_data = (struct cmzn_command_data *)command_data_void))
	{
		struct cmzn_spectrum *rgb_spectrum = (struct cmzn_spectrum *)NULL;
		cmzn_material *material =
			cmzn_materialmodule_get_default_material(command_data->materialmodule);

		char *graphics_object_name = NULL, *region_path = NULL;
		char *file_name;
		struct cmzn_region *top_region = NULL;
		file_name = duplicate_string( "" );
		float deflection = 0.1;
		struct Option_table *option_table = CREATE(Option_table)();

		rgb_spectrum = FIND_BY_IDENTIFIER_IN_MANAGER(cmzn_spectrum,name)( "RGB", command_data->spectrum_manager );
		if ( !rgb_spectrum )
		{
			if ( !create_RGB_spectrum( &rgb_spectrum, command_data_void ) )
			{
				printf( "Failed to create RGB spectrum\n" );
			}
		}
		else
		{
			ACCESS(cmzn_spectrum)(rgb_spectrum);
		}
		// as
		Option_table_add_entry(option_table, "as", &graphics_object_name,
			(void *)1, set_name );
		// file
		Option_table_add_entry(option_table, "file", &file_name,
			(void *)1, set_name);
		// region
		Option_table_add_entry(option_table,"region",
			&region_path, (void *)1, set_name);
		// spectrum
		Option_table_add_entry(option_table, "spectrum", &rgb_spectrum,
			command_data->spectrum_manager,set_Spectrum);
		// material
		Option_table_add_set_Material_entry(option_table, "material", &material,
			command_data->materialmodule);
		// deflection
		Option_table_add_entry(option_table, "deflection", &deflection, NULL, set_float );

		return_code = Option_table_multi_parse(option_table, state);
		if (file_name == NULL)
		{
			return_code = 0;
		}
		if (return_code)
		{
			clock_t occStart, occEnd;
			occStart = clock();
			printf("region path = '%s'\n", region_path);
			if (region_path)
			{
				top_region = cmzn_region_find_subregion_at_path(
					command_data->root_region, region_path);
				if (NULL == top_region)
				{
					top_region = cmzn_region_create_subregion(
						command_data->root_region, region_path);
					if (NULL == top_region)
					{
						display_message(ERROR_MESSAGE, "execute_command_gfx_import.  "
							"Unable to find or create region '%s'.", region_path);
						return_code = 0;
					}
				}
			}
			else
			{
				top_region = ACCESS(cmzn_region)(command_data->root_region);
			}

			if (return_code)
			{
				if (cmzn_region_import_cad_file(top_region, file_name))
				{
					occEnd = clock();
					//DEBUG_PRINT( "OCC load took %.2lf seconds\n", ( occEnd - occStart ) / double( CLOCKS_PER_SEC ) );
				}
			}
			DEACCESS(cmzn_region)(&top_region);
		}
		DESTROY(Option_table)(&option_table);
		if (region_path)
		{
			DEALLOCATE(region_path);
		}
		if (rgb_spectrum)
		{
			DEACCESS(cmzn_spectrum)(&rgb_spectrum);
		}
		DEALLOCATE(graphics_object_name);
		cmzn_material_destroy(&material);
		DEALLOCATE(file_name);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"execute_command_gfx_import.  Invalid argument(s)\n");
		return_code = 0;
	}
	LEAVE;
	return (return_code);
}

#endif /* USE_OPENCASCADE */

static int execute_command_gfx_export(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 4 March 2003

DESCRIPTION :
Executes a GFX EXPORT command.
==============================================================================*/
{
	int return_code;
	struct Option_table *option_table;

	ENTER(execute_command_gfx_export);
	USE_PARAMETER(dummy_to_be_modified);
	if (state && command_data_void)
	{
		option_table = CREATE(Option_table)();
		Option_table_add_entry(option_table,"alias",NULL,
			command_data_void, gfx_export_alias);
		Option_table_add_entry(option_table,"cm",NULL,
			command_data_void, gfx_export_cm);
		Option_table_add_entry(option_table,"iges",NULL,
			command_data_void, gfx_export_iges);
		Option_table_add_entry(option_table,"stl",NULL,
			command_data_void, gfx_export_stl);
		Option_table_add_entry(option_table,"threejs",NULL,
			command_data_void, gfx_export_threejs);
		Option_table_add_entry(option_table,"vrml",NULL,
			command_data_void, gfx_export_vrml);
		Option_table_add_entry(option_table,"wavefront",NULL,
			command_data_void, gfx_export_wavefront);
		Option_table_add_entry(option_table,"webgl",NULL,
			command_data_void, gfx_export_webgl);
		return_code = Option_table_parse(option_table,state);
		DESTROY(Option_table)(&option_table);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"execute_command_gfx_export.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_gfx_export */

void create_triangle_mesh(struct cmzn_region *region, Triangle_mesh *trimesh)
{
	cmzn_fieldmodule_id fieldmodule = cmzn_region_get_fieldmodule(region);
	cmzn_fieldmodule_begin_change(fieldmodule);

	/* create a 3-D coordinate field */
	cmzn_field_id coordinate_field = cmzn_fieldmodule_find_field_by_name(fieldmodule, "coordinates");
	if (coordinate_field)
	{
		cmzn_field_finite_element_id fe_field = cmzn_field_cast_finite_element(coordinate_field);
		if ((!fe_field) ||
			(3 != cmzn_field_get_number_of_components(coordinate_field)) ||
			(!cmzn_field_is_type_coordinate(coordinate_field)) ||
			(CMZN_FIELD_COORDINATE_SYSTEM_TYPE_RECTANGULAR_CARTESIAN !=
				cmzn_field_get_coordinate_system_type(coordinate_field)))
		{
			cmzn_field_destroy(&coordinate_field);
		}
		cmzn_field_finite_element_destroy(&fe_field);
	}
	if (!coordinate_field)
	{
		coordinate_field = cmzn_fieldmodule_create_field_finite_element(fieldmodule, 3);
		cmzn_field_set_name(coordinate_field, "coordinates");
		cmzn_field_set_component_name(coordinate_field, 1, "x");
		cmzn_field_set_component_name(coordinate_field, 2, "y");
		cmzn_field_set_component_name(coordinate_field, 3, "z");
		cmzn_field_set_managed(coordinate_field, true);
		cmzn_field_set_type_coordinate(coordinate_field, true);
	}

	/* create and fill nodes */
	cmzn_nodeset_id nodeset = cmzn_fieldmodule_find_nodeset_by_field_domain_type(fieldmodule, CMZN_FIELD_DOMAIN_TYPE_NODES);
	cmzn_nodetemplate_id nodetemplate = cmzn_nodeset_create_nodetemplate(nodeset);
	cmzn_nodetemplate_define_field(nodetemplate, coordinate_field);
	cmzn_fieldcache_id cache = cmzn_fieldmodule_create_fieldcache(fieldmodule);
	const Triangle_vertex_set vertex_set = trimesh->get_vertex_set();
	int initial_identifier = cmzn_nodeset_get_FE_nodeset_internal(nodeset)->get_last_FE_node_identifier();
	double coordinates[3];
	for (Triangle_vertex_set_const_iterator vertex_iter = vertex_set.begin(); vertex_iter!=vertex_set.end(); ++vertex_iter)
	{
		int identifier = initial_identifier+(*vertex_iter)->get_identifier();
		cmzn_node_id node = cmzn_nodeset_create_node(nodeset, identifier, nodetemplate);
		cmzn_fieldcache_set_node(cache, node);
		(*vertex_iter)->get_coordinates(coordinates);
		cmzn_field_assign_real(coordinate_field, cache, 3, coordinates);
		cmzn_node_destroy(&node);
	}
	cmzn_fieldcache_destroy(&cache);
	cmzn_nodetemplate_destroy(&nodetemplate);

	// establish mode which enables creation of shared faces with defineElementFaces, below
	FE_region_begin_define_faces(cmzn_region_get_FE_region(region));

	cmzn_mesh_id mesh = cmzn_fieldmodule_find_mesh_by_dimension(fieldmodule, 2);
	FE_mesh *fe_mesh = cmzn_mesh_get_FE_mesh_internal(mesh);
	cmzn_elementtemplate_id elementtemplate = cmzn_mesh_create_elementtemplate(mesh);
	cmzn_elementtemplate_set_element_shape_type(elementtemplate, CMZN_ELEMENT_SHAPE_TYPE_TRIANGLE);
	cmzn_elementtemplate_set_number_of_nodes(elementtemplate, 3);
	cmzn_elementbasis_id elementbasis = cmzn_fieldmodule_create_elementbasis(fieldmodule, 2,
		CMZN_ELEMENTBASIS_FUNCTION_TYPE_LINEAR_SIMPLEX);
	int local_node_indexes[3] = { 1, 2, 3 };
	cmzn_elementtemplate_define_field_simple_nodal(elementtemplate, coordinate_field, /*component_number*/-1,
		elementbasis, 3, local_node_indexes);
	const Triangle_vertex *vertex[3];
	const Mesh_triangle_list triangle_list = trimesh->get_triangle_list();
	for (Mesh_triangle_list_const_iterator triangle_iter = triangle_list.begin(); triangle_iter!=triangle_list.end(); ++triangle_iter)
	{
		(*triangle_iter)->get_vertexes(&(vertex[0]), &(vertex[1]), &(vertex[2]));
		for (int i = 0; i < 3; ++i)
			cmzn_elementtemplate_set_node(elementtemplate, i + 1,
				cmzn_nodeset_find_node_by_identifier(nodeset, initial_identifier + vertex[i]->get_identifier()));
		cmzn_element_id element = cmzn_mesh_create_element(mesh, /*identifier*/-1, elementtemplate);
		fe_mesh->defineElementFaces(element->getIndex());
		cmzn_element_destroy(&element);
	}
	cmzn_elementbasis_destroy(&elementbasis);
	cmzn_elementtemplate_destroy(&elementtemplate);
	cmzn_mesh_destroy(&mesh);

	FE_region_end_define_faces(cmzn_region_get_FE_region(region));

	cmzn_nodeset_destroy(&nodeset);
	cmzn_field_destroy(&coordinate_field);
	cmzn_fieldmodule_end_change(fieldmodule);
	cmzn_fieldmodule_destroy(&fieldmodule);
}

static int gfx_mesh_graphics_tetrahedral(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
{
	int return_code;
	struct cmzn_command_data *command_data;
	char *region_path;

	ENTER(gfx_mesh_graphics_tetrahedral);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		region_path = cmzn_region_get_root_region_path();
		if (NULL != (command_data=(struct cmzn_command_data *)command_data_void))
		{

			double maxh=100000;
			double fineness=0.5;
			int secondorder=0;
			cmzn_scene_id scene;
			char *meshsize_file = NULL;
			struct Option_table *option_table;
			scene = ACCESS(Scene)(command_data->default_scene);
			cmzn_scenefilter_id filter =
				cmzn_scenefiltermodule_get_default_scenefilter(command_data->filter_module);
			option_table = CREATE(Option_table)();
			char clear = 0;
			Option_table_add_entry(option_table, "region", &region_path,
				command_data->root_region, set_cmzn_region_path);
			Option_table_add_entry(option_table, "scene", &scene,
				command_data->root_region, set_Scene);
			Option_table_add_entry(option_table, "filter", &filter,
				command_data->filter_module, set_cmzn_scenefilter);
			Option_table_add_entry(option_table,"clear_region",
				&clear,(void *)NULL,set_char_flag);
			Option_table_add_entry(option_table,"mesh_global_size",
				&maxh,(void *)NULL,set_double);
			Option_table_add_entry(option_table,"fineness",
				&fineness,(void *)NULL,set_double);
			Option_table_add_entry(option_table,"secondorder",
				&secondorder,(void *)NULL,set_int);
			Option_table_add_string_entry(option_table,"meshsize_file",
				&meshsize_file, " FILENAME");

			if ((return_code = Option_table_multi_parse(option_table, state)))
			{
#if defined (ZINC_USE_NETGEN)
				Triangle_mesh *trimesh = NULL;
				if (scene)
				{
					float tolerance = 0.000001;
					build_Scene(scene, filter);
					double min[3] = { 0.0, 0.0, 0.0 };
					double max[3] = { 0.0, 0.0, 0.0 };
					cmzn_scene_get_coordinates_range(scene, filter, min, max);
					const double size_x = max[0] - min[0];
					const double size_y = max[1] - min[1];
					const double size_z = max[2] - min[2];
					if ((size_x != 0.0) || (size_y != 0.0) || (size_z != 0.0))
					{
						tolerance *= static_cast<float>(sqrt(size_x*size_x + size_y*size_y + size_z*size_z));
					}
					Render_graphics_triangularisation renderer(NULL, tolerance);
					if (renderer.Scene_compile(scene, filter))
					{
						return_code = renderer.Scene_tree_execute(scene);
						trimesh = renderer.get_triangle_mesh();
						struct cmzn_region *region = cmzn_region_find_subregion_at_path(
							command_data->root_region, region_path);
						if (clear)
						{
							cmzn_region_clear_finite_elements(region);
						}
						if (trimesh && region)
						{
							struct Generate_netgen_parameters *generate_netgen_para=NULL;
							generate_netgen_para=create_netgen_parameters();
							set_netgen_parameters_maxh(generate_netgen_para,maxh);
							set_netgen_parameters_fineness(generate_netgen_para,fineness);
							set_netgen_parameters_secondorder(generate_netgen_para,secondorder);
							set_netgen_parameters_trimesh(generate_netgen_para, trimesh);
							if (meshsize_file)
								set_netgen_parameters_meshsize_filename(generate_netgen_para, meshsize_file);
							generate_mesh_netgen(region, generate_netgen_para);
							release_netgen_parameters(generate_netgen_para);

						}
						else
						{
							display_message(ERROR_MESSAGE, "gfx_mesh_graphics_tetrahedral."
								"Unknown region: %s", region_path);
						}
						cmzn_region_destroy(&region);
					}
				}
#else
				USE_PARAMETER(scene);
				USE_PARAMETER(region_path);
				display_message(ERROR_MESSAGE,
					"gfx_mesh_graphics. Does not support tetrahedral mesh yet. To use this feature"
					" please compile cmgui with Netgen");
				return_code = 0;
#endif /* defined (ZINC_USE_NETGEN) */
			}
			DEALLOCATE(region_path);
			DESTROY(Option_table)(&option_table);
			cmzn_scene_destroy(&scene);
			cmzn_scenefilter_destroy(&filter);
			if (meshsize_file)
			{
				DEALLOCATE(meshsize_file);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_mesh_graphics_tetrahedral.  Invalid argument(s)");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_mesh_graphics_tetrahedral.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_mesh_graphics */

static int gfx_mesh_graphics_triangle(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
{
	int return_code;
	struct cmzn_command_data *command_data;
	struct Option_table *option_table;
	char *region_path;

	ENTER(gfx_mesh_graphics_triangle);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		region_path = cmzn_region_get_root_region_path();
		if (NULL != (command_data=(struct cmzn_command_data *)command_data_void))
		{
			Triangle_mesh *trimesh = NULL;
			cmzn_scene_id scene = cmzn_scene_access(command_data->default_scene);
			cmzn_scenefilter_id filter =
				cmzn_scenefiltermodule_get_default_scenefilter(command_data->filter_module);
			option_table = CREATE(Option_table)();
			char clear = 0;
			Option_table_add_entry(option_table, "scene", &scene,
				command_data->root_region, set_Scene);
			Option_table_add_entry(option_table, "filter", &filter,
				command_data->filter_module, set_cmzn_scenefilter);
			Option_table_add_entry(option_table, "target_region", &region_path,
				command_data->root_region, set_cmzn_region_path);
			Option_table_add_entry(option_table,"clear_region",
				&clear,(void *)NULL,set_char_flag);
			if ((return_code = Option_table_multi_parse(option_table, state)))
			{
				if (scene)
				{
					float tolerance = 0.000001;
					build_Scene(scene, filter);
					double min[3] = { 0.0, 0.0, 0.0 };
					double max[3] = { 0.0, 0.0, 0.0 };
					cmzn_scene_get_coordinates_range(scene, filter, min, max);
					const double size_x = max[0] - min[0];
					const double size_y = max[1] - min[1];
					const double size_z = max[2] - min[2];
					if ((size_x != 0.0) || (size_y != 0.0) || (size_z != 0.0))
					{
						tolerance *= static_cast<float>(sqrt(size_x*size_x + size_y*size_y + size_z*size_z));
					}
					Render_graphics_triangularisation renderer(NULL, tolerance);
					if (renderer.Scene_compile(scene, filter))
					{
						return_code = renderer.Scene_tree_execute(scene);
						trimesh = renderer.get_triangle_mesh();
						struct cmzn_region *region = cmzn_region_find_subregion_at_path(
							command_data->root_region, region_path);
						if (clear)
						{
							cmzn_region_clear_finite_elements(region);
						}
						if (trimesh && region)
						{
							create_triangle_mesh(region, trimesh);
						}
						else
						{
							display_message(ERROR_MESSAGE, "gfx_mesh_graphics_triangle. Unknown region: %s", region_path);
						}
						cmzn_region_destroy(&region);
					}

				}
			}
			DEALLOCATE(region_path);
			DESTROY(Option_table)(&option_table);
			cmzn_scenefilter_destroy(&filter);
			cmzn_scene_destroy(&scene);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_mesh_graphics_triangle.  Invalid argument(s)");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_mesh_graphics_triangle.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_mesh_graphics */


static int gfx_mesh_graphics(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
{
	int return_code;
	struct cmzn_command_data *command_data;
	struct Option_table *option_table;

	ENTER(gfx_mesh_graphics);
	USE_PARAMETER(dummy_to_be_modified);
	if (state && (command_data=(struct cmzn_command_data *)command_data_void))
	{
		return_code = 1;

		option_table = CREATE(Option_table)();
		Option_table_add_entry(option_table,"tetrahedral", NULL,
			(void *)command_data, gfx_mesh_graphics_tetrahedral);
		Option_table_add_entry(option_table,"triangle", NULL,
			(void *)command_data, gfx_mesh_graphics_triangle);
		return_code = Option_table_parse(option_table, state);

		DESTROY(Option_table)(&option_table);
	}
	else
	{
		display_message(ERROR_MESSAGE,
				"gfx_mesh_graphics.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_mesh_graphics */

static int execute_command_gfx_mesh(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 4 March 2003

DESCRIPTION :
Executes a GFX EXPORT command.
==============================================================================*/
{
	int return_code;
	struct Option_table *option_table;

	ENTER(execute_command_gfx_export);
	USE_PARAMETER(dummy_to_be_modified);
	if (state && command_data_void)
	{
		option_table = CREATE(Option_table)();
		Option_table_add_entry(option_table,"graphics",NULL,
			command_data_void, gfx_mesh_graphics);
		return_code = Option_table_parse(option_table,state);
		DESTROY(Option_table)(&option_table);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"execute_command_gfx_export.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_gfx_export */

/***************************************************************************//**
 * Executes a GFX EVALUATE DGROUP/EGROUP/NGROUP command.
 */
int gfx_evaluate(struct Parse_state *state, void *dummy_to_be_modified,
	void *command_data_void)
{
	int return_code;

	ENTER(gfx_evaluate);
	USE_PARAMETER(dummy_to_be_modified);
	cmzn_command_data *command_data = reinterpret_cast<cmzn_command_data *>(command_data_void);
	if (state && command_data)
	{
		char *data_region_path = 0;
		char *element_region_path = 0;
		char *node_region_path = 0;
		char selected_flag = 0;
		char *source_field_name = 0;
		char *destination_field_name = 0;
		FE_value time = 0;

		Option_table *option_table = CREATE(Option_table)();
		Option_table_add_string_entry(option_table, "destination", &destination_field_name, " FIELD_NAME");
		Option_table_add_string_entry(option_table, "dgroup", &data_region_path, " REGION_PATH/GROUP_NAME");
		Option_table_add_string_entry(option_table, "egroup", &element_region_path, " REGION_PATH/GROUP_NAME");
		Option_table_add_string_entry(option_table, "ngroup", &node_region_path, " REGION_PATH/GROUP_NAME");
		Option_table_add_char_flag_entry(option_table, "selected", &selected_flag);
		Option_table_add_string_entry(option_table, "source", &source_field_name, " FIELD_NAME");

		if (0 != (return_code = Option_table_multi_parse(option_table, state)))
		{
			if (destination_field_name && source_field_name)
			{
				struct Element_point_ranges_selection *element_point_ranges_selection = 0;
				if (selected_flag)
				{
					element_point_ranges_selection =
						command_data->element_point_ranges_selection;
				}
				char *region_or_group_path = 0;
				if (data_region_path && (!element_region_path) && (!node_region_path))
				{
					region_or_group_path = data_region_path;
				}
				else if (element_region_path && (!data_region_path) &&
					(!node_region_path))
				{
					region_or_group_path = element_region_path;
				}
				else if (node_region_path && (!data_region_path) &&
					(!element_region_path))
				{
					region_or_group_path = node_region_path;
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"gfx_evaluate.  Must specify one of dgroup/egroup/ngroup");
					return_code = 0;
				}
				if (region_or_group_path)
				{
					cmzn_region_id region = 0;
					cmzn_field_group_id group = 0;
					int result = cmzn_region_path_to_subregion_and_group(command_data->root_region, region_or_group_path, &region, &group);
					if (CMZN_OK != result)
					{
						display_message(ERROR_MESSAGE, "Invalid region/group path '%s'", region_or_group_path);
						display_parse_state_location(state);
						return_code = 0;
					}
					if (return_code)
					{
						cmzn_fieldmodule_id field_module = cmzn_region_get_fieldmodule(region);
						cmzn_field_id destination_field = cmzn_fieldmodule_find_field_by_name(field_module,
							destination_field_name);
						cmzn_field_id source_field = cmzn_fieldmodule_find_field_by_name(field_module,
							source_field_name);
						cmzn_field_id selection_field = 0;
						if (selected_flag)
						{
							cmzn_scene_id scene = cmzn_region_get_scene(region);
							selection_field = cmzn_scene_get_selection_field(scene);
							cmzn_scene_destroy(&scene);
						}
						if (node_region_path || data_region_path)
						{
							cmzn_nodeset_id nodeset = cmzn_fieldmodule_find_nodeset_by_field_domain_type(
								field_module, node_region_path ? CMZN_FIELD_DOMAIN_TYPE_NODES : CMZN_FIELD_DOMAIN_TYPE_DATAPOINTS);
							if (group)
							{
								cmzn_field_node_group_id node_group = cmzn_field_group_get_field_node_group(group, nodeset);
								cmzn_nodeset_destroy(&nodeset);
								nodeset = cmzn_nodeset_group_base_cast(cmzn_field_node_group_get_nodeset_group(node_group));
								cmzn_field_node_group_destroy(&node_group);
							}
							if (nodeset)
							{
								const int result = cmzn_nodeset_assign_field_from_source(nodeset, destination_field, source_field,
									/*conditional_field*/selection_field, time);
								if ((CMZN_RESULT_OK != result) && (CMZN_RESULT_WARNING_PART_DONE != result))
								{
									return_code = 0;
								}
								cmzn_nodeset_destroy(&nodeset);
							}
						}
						else if (element_region_path)
						{
							int highest_dimension = FE_region_get_highest_dimension(cmzn_region_get_FE_region(region));
							cmzn_mesh_id mesh = cmzn_fieldmodule_find_mesh_by_dimension(field_module, highest_dimension);
							if (group)
							{
								cmzn_field_element_group_id element_group = cmzn_field_group_get_field_element_group(group, mesh);
								cmzn_mesh_destroy(&mesh);
								mesh = cmzn_mesh_group_base_cast(cmzn_field_element_group_get_mesh_group(element_group));
								cmzn_field_element_group_destroy(&element_group);
							}
							if (mesh)
							{
								return_code = cmzn_mesh_assign_grid_field_from_source(mesh, destination_field, source_field,
									/*conditional_field*/selection_field,
									element_point_ranges_selection, time);
								cmzn_mesh_destroy(&mesh);
							}
						}
						cmzn_field_destroy(&selection_field);
						cmzn_field_destroy(&source_field);
						cmzn_field_destroy(&destination_field);
						cmzn_fieldmodule_destroy(&field_module);
					}
					cmzn_field_group_destroy(&group);
					cmzn_region_destroy(&region);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_evaluate.  Must specify destination and source fields");
				return_code = 0;
			}
		}
		DESTROY(Option_table)(&option_table);
		if (data_region_path)
		{
			DEALLOCATE(data_region_path);
		}
		if (element_region_path)
		{
			DEALLOCATE(element_region_path);
		}
		if (node_region_path)
		{
			DEALLOCATE(node_region_path);
		}
		if (source_field_name)
		{
			DEALLOCATE(source_field_name);
		}
		if (destination_field_name)
		{
			DEALLOCATE(destination_field_name);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE, "gfx_evaluate.  Invalid argument(s)");
		return_code = 0;
	}

	LEAVE;

	return (return_code);
}

static int gfx_list_all_commands(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 23 July 2008

DESCRIPTION :
Executes a GFX LIST ALL_COMMANDS.
==============================================================================*/
{
	int return_code = 1;
	static const char	*command_prefix, *current_token;
	struct cmzn_command_data *command_data;
	struct MANAGER(cmzn_material) *graphical_material_manager;

	ENTER(gfx_list_all_commands);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (NULL != (current_token = state->current_token))
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
			}
			else
			{
				display_message(INFORMATION_MESSAGE," [ALL]");
				return_code=1;
			}
		}
		else
		{
			if (NULL != (command_data = (struct cmzn_command_data *)command_data_void))
			{
				cmiss_execute_command("gfx list field commands", command_data_void);
				/* Commands of spectrum */
				if (command_data->spectrum_manager)
				{
					FOR_EACH_OBJECT_IN_MANAGER(cmzn_spectrum)(
						for_each_spectrum_list_or_write_commands, (void *)"false", command_data->spectrum_manager);
				}
				/* Command of graphical_material */
				if (NULL != (graphical_material_manager =
					cmzn_materialmodule_get_manager(command_data->materialmodule)))
				{
					command_prefix="gfx create material ";
					return_code=FOR_EACH_OBJECT_IN_MANAGER(cmzn_material)(
						list_Graphical_material_commands,(void *)command_prefix,
						graphical_material_manager);
				}
				return_code =1;
				/* Command of graphics window */
#if defined (USE_CMGUI_GRAPHICS_WINDOW)
				return_code=FOR_EACH_OBJECT_IN_MANAGER(Graphics_window)(
					 list_Graphics_window_commands,(void *)NULL,
					 command_data->graphics_window_manager);
#endif /*defined (USE_CMGUI_GRAPHICS_WINDOW)*/
			}
		}
	}
	LEAVE;

	return (return_code);
}/* gfx_list_all_commands */

/***************************************************************************//**
 * List statistical information about packing in node/element btree structures.
 */
static int gfx_list_btree_statistics(struct Parse_state *state,
	void *dummy_to_be_modified, void *root_region_void)
{
	int return_code = 0;
	USE_PARAMETER(dummy_to_be_modified);
	cmzn_region *root_region = (struct cmzn_region *)root_region_void;
	if (state && root_region)
	{
		cmzn_region *region = cmzn_region_access(root_region);
		Option_table *option_table = CREATE(Option_table)();
		Option_table_add_help(option_table,
			"List statistics about btree structures storing a region's nodes and elements.");
		Option_table_add_set_cmzn_region(option_table,
			"region", root_region, &region);
		return_code = Option_table_multi_parse(option_table, state);
		DESTROY(Option_table)(&option_table);
		if (return_code)
		{
			cmzn_fieldmodule_id field_module = cmzn_region_get_fieldmodule(region);
			FE_region *fe_region = cmzn_region_get_FE_region(region);
			FE_region_list_btree_statistics(fe_region);
			cmzn_fielditerator_id iterator = cmzn_fieldmodule_create_fielditerator(field_module);
			cmzn_field_id field = 0;
			while (0 != (field = cmzn_fielditerator_next(iterator)))
			{
				cmzn_field_node_group_id node_group = cmzn_field_cast_node_group(field);
				cmzn_field_element_group_id element_group = cmzn_field_cast_element_group(field);
				if (node_group || element_group)
				{
					char *name = cmzn_field_get_name(field);
					display_message(INFORMATION_MESSAGE, "%s group: %s\n", node_group ? "Node" : "Element", name);
					if (node_group)
						cmzn_field_node_group_list_btree_statistics(node_group);
					else
						cmzn_field_element_group_list_btree_statistics(element_group);
					cmzn_field_node_group_destroy(&node_group);
					cmzn_field_element_group_destroy(&element_group);
					DEALLOCATE(name);
				}
				cmzn_field_destroy(&field);
			}
			cmzn_fielditerator_destroy(&iterator);
			cmzn_fieldmodule_destroy(&field_module);
		}
		cmzn_region_destroy(&region);
	}
	return (return_code);
}

static int gfx_list_environment_map(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 29 June 1996

DESCRIPTION :
Executes a GFX LIST ENVIRONMENT_MAP.
==============================================================================*/
{
	const char *current_token;
	int return_code;
	struct cmzn_command_data *command_data;
	struct Environment_map *environment_map;

	ENTER(gfx_list_environment_map);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (NULL != (current_token = state->current_token))
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				if (NULL != (command_data = (struct cmzn_command_data *)command_data_void))
				{
					if (NULL != (environment_map=FIND_BY_IDENTIFIER_IN_MANAGER(Environment_map,
						name)(current_token,command_data->environment_map_manager)))
					{
						return_code=list_Environment_map(environment_map);
					}
					else
					{
						display_message(ERROR_MESSAGE,"Unknown environment map: %s",
							current_token);
						display_parse_state_location(state);
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"gfx_list_environment_map.  Missing command_data");
					return_code=0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE," ENVIRONMENT_MAP_NAME");
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Missing environment map name");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_list_environment_map.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_list_environment_map */

static int gfx_list_Computed_field(struct Parse_state *state,
	void *dummy_to_be_modified,void *root_region_void)
/*******************************************************************************
LAST MODIFIED : 23 July 2008

DESCRIPTION :
Executes a GFX LIST FIELD.
==============================================================================*/
{
	static const char	*command_prefix="gfx define field ";
	char commands_flag, *command_prefix_plus_region_path;
	int path_length, return_code;
	struct cmzn_region *root_region;
	struct cmzn_region_path_and_name region_path_and_name;
	struct Computed_field *field;
	struct List_Computed_field_commands_data list_commands_data;
	struct LIST(Computed_field) *list_of_fields;
	struct Option_table *option_table;

	ENTER(gfx_list_Computed_field);
	USE_PARAMETER(dummy_to_be_modified);
	if (state && (root_region = (struct cmzn_region *)root_region_void))
	{
		commands_flag=0;
		region_path_and_name.region = (struct cmzn_region *)NULL;
		region_path_and_name.region_path = (char *)NULL;
		region_path_and_name.name = (char *)NULL;

		option_table=CREATE(Option_table)();
		/* commands */
		Option_table_add_entry(option_table, "commands", &commands_flag, NULL,
			set_char_flag);
		/* default option: region_path and/or field_name */
		Option_table_add_region_path_and_or_field_name_entry(
			option_table, (char *)NULL, &region_path_and_name, root_region);
		if (0 != (return_code = Option_table_multi_parse(option_table, state)))
		{
			field = (struct Computed_field *)NULL;
			if (region_path_and_name.name)
			{
				/* following accesses the field, if any */
				field = cmzn_region_find_field_by_name(region_path_and_name.region,
					region_path_and_name.name);
				if (!field)
				{
					display_message(ERROR_MESSAGE,
						"gfx list field:  There is no field or child region called %s in region %s",
						region_path_and_name.name, region_path_and_name.region_path);
					return_code = 0;
				}
			}
			if (return_code)
			{
				path_length = 0;
				if (region_path_and_name.region_path)
				{
					path_length = strlen(region_path_and_name.region_path);
					if (path_length > 0)
					{
						path_length += strlen(CMZN_REGION_PATH_SEPARATOR_STRING);
					}
				}
				else
				{
					region_path_and_name.region = ACCESS(cmzn_region)(root_region);
				}
				if (commands_flag)
				{
					if (ALLOCATE(command_prefix_plus_region_path,char,
						strlen(command_prefix)+1+path_length))
					{
						strcpy(command_prefix_plus_region_path,command_prefix);
						if (path_length > 0)
						{
							strcat(command_prefix_plus_region_path,region_path_and_name.region_path);
							strcat(command_prefix_plus_region_path,CMZN_REGION_PATH_SEPARATOR_STRING);
						}
					}
					if (field)
					{
						return_code=list_Computed_field_commands(field,
							(void *)command_prefix_plus_region_path);
					}
					else
					{
						if (NULL != (list_of_fields = CREATE(LIST(Computed_field))()))
						{
							list_commands_data.command_prefix = command_prefix_plus_region_path;
							list_commands_data.listed_fields = 0;
							list_commands_data.computed_field_list = list_of_fields;
							list_commands_data.computed_field_manager =
								cmzn_region_get_Computed_field_manager(region_path_and_name.region);
							while (FOR_EACH_OBJECT_IN_MANAGER(Computed_field)(
								list_Computed_field_commands_if_managed_source_fields_in_list,
								(void *)&list_commands_data, cmzn_region_get_Computed_field_manager(region_path_and_name.region)) &&
								(0 != list_commands_data.listed_fields))
							{
								list_commands_data.listed_fields = 0;
							}
							DESTROY(LIST(Computed_field))(&list_of_fields);
						}
						else
						{
							return_code=0;
						}
					}
					DEALLOCATE(command_prefix_plus_region_path);
				}
				else
				{
					if (field)
					{
						return_code = list_Computed_field(field,(void *)NULL);
					}
					else
					{
						return_code=FOR_EACH_OBJECT_IN_MANAGER(Computed_field)(
							list_Computed_field_name,(void *)NULL,
							cmzn_region_get_Computed_field_manager(region_path_and_name.region));
					}
				}
				if (!return_code)
				{
					display_message(ERROR_MESSAGE,
						"gfx list field.  Failed to list fields");
				}
			}
			if (field)
			{
				DEACCESS(Computed_field)(&field);
			}
		}
		DESTROY(Option_table)(&option_table);
		if (region_path_and_name.region)
		{
			DEACCESS(cmzn_region)(&region_path_and_name.region);
		}
		if (region_path_and_name.region_path)
		{
			DEALLOCATE(region_path_and_name.region_path);
		}
		if (region_path_and_name.name)
		{
			DEALLOCATE(region_path_and_name.name);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_list_Computed_field.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* gfx_list_Computed_field */

/** list element contents by making EX file serialisation */
static bool list_FE_element(cmzn_region_id region, cmzn_fieldmodule_id fieldmodule, cmzn_mesh_id mesh, cmzn_element_id element)
{
	bool result = true;
	cmzn_fieldmodule_begin_change(fieldmodule);
	cmzn_field_id field = cmzn_fieldmodule_create_field_group(fieldmodule);
	cmzn_field_group_id group_field = cmzn_field_cast_group(field);
	cmzn_field_element_group_id element_group_field = cmzn_field_group_create_field_element_group(group_field, mesh);
	cmzn_mesh_group_id mesh_group = cmzn_field_element_group_get_mesh_group(element_group_field);
	cmzn_streaminformation_id si = cmzn_region_create_streaminformation_region(region);
	cmzn_streaminformation_region_id sir = cmzn_streaminformation_cast_region(si);
	cmzn_streamresource_id sr = cmzn_streaminformation_create_streamresource_memory(si);
	cmzn_streamresource_memory_id srm = cmzn_streamresource_cast_memory(sr);
	char *group_name = cmzn_field_get_name(field);
	if ((group_name) && (mesh_group) && (cmzn_streaminformation_region_set_resource_group_name(sir, sr, group_name)))
	{
		const int dimension = cmzn_mesh_get_dimension(mesh);
		cmzn_field_domain_types domainTypes =
			(dimension == 3) ? CMZN_FIELD_DOMAIN_TYPE_MESH3D :
			(dimension == 2) ? CMZN_FIELD_DOMAIN_TYPE_MESH2D : CMZN_FIELD_DOMAIN_TYPE_MESH1D;
		cmzn_streaminformation_region_set_resource_domain_types(sir, sr, domainTypes);
		cmzn_mesh_group_add_element(mesh_group, element);
		const char *buffer;
		unsigned int buffer_size;
		if ((CMZN_OK == cmzn_region_write(region, sir))
			&& (CMZN_OK == cmzn_streamresource_memory_get_buffer(srm, (const void **)&buffer, &buffer_size))
			&& (buffer_size > 0))
		{
			const char *block = buffer;
			unsigned int charCount = 0;
			int lineNumber = 1;
			while (block)
			{
				const char *blockEnd = block;
				const char *nextBlock = 0;
				while (charCount < buffer_size)
				{
					if ((*blockEnd == '\n') || (*blockEnd == '\r'))
					{
						nextBlock = blockEnd + 1;
						++charCount;
						while ((charCount < buffer_size) && ((*nextBlock == '\n') || (*nextBlock == '\r')))
						{
							++nextBlock;
							++charCount;
						}
						if (charCount == buffer_size)
							nextBlock = 0;
						break;
					}
					++blockEnd;
					++charCount;
				}
				// skip first 3 lines which of EX header information
				if (lineNumber > 3)
				{
					int blockLen = static_cast<int>(blockEnd - block);
					display_message(INFORMATION_MESSAGE, "%.*s\n", blockLen, block);
				}
				++lineNumber;
				block = nextBlock;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE, "list_FE_element.  Failed to write element to memory stream");
			result = false;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE, "list_FE_element.  Failed to set up temporary group");
		result = false;
	}
	DEALLOCATE(group_name);
	cmzn_streamresource_memory_destroy(&srm);
	cmzn_streamresource_destroy(&sr);
	cmzn_streaminformation_region_destroy(&sir);
	cmzn_streaminformation_destroy(&si);
	cmzn_mesh_group_destroy(&mesh_group);
	cmzn_field_element_group_destroy(&element_group_field);
	cmzn_field_group_destroy(&group_field);
	cmzn_field_destroy(&field);
	cmzn_fieldmodule_end_change(fieldmodule);
	return result;
}

static int gfx_list_FE_element(struct Parse_state *state,
	void *dimension_void,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 28 March 2003

DESCRIPTION :
Executes a GFX LIST ELEMENT.
==============================================================================*/
{
	int return_code;
	int dimension = VOIDPTR2INT(dimension_void);
	cmzn_command_data *command_data = reinterpret_cast<cmzn_command_data *>(command_data_void);
	if (state && command_data && (0 < dimension) && (dimension <= 3))
	{
		/* initialise defaults */
		char all_flag = 0;
		cmzn_region_id region = cmzn_region_access(command_data->root_region);
		char selected_flag = 0;
		char verbose_flag = 0;
		Multi_range *element_ranges = CREATE(Multi_range)();
		char *conditional_field_name = 0;

		Option_table *option_table = CREATE(Option_table)();
		/* all (redundant option) */
		Option_table_add_entry(option_table, "all", &all_flag, NULL, set_char_flag);
		/* region */
		Option_table_add_set_cmzn_region(option_table, "region",
			command_data->root_region, &region);
		/* selected */
		Option_table_add_entry(option_table, "selected", &selected_flag,
			NULL, set_char_flag);
		/* conditional */
		Option_table_add_string_entry(option_table, "conditional", &conditional_field_name,
			" FIELD_NAME");
		/* verbose */
		Option_table_add_entry(option_table, "verbose", &verbose_flag,
			NULL, set_char_flag);
		/* default option: element number ranges */
		Option_table_add_entry(option_table, (const char *)NULL, (void *)element_ranges,
			NULL, set_Multi_range);
		return_code = Option_table_multi_parse(option_table, state);
		DESTROY(Option_table)(&option_table);

		cmzn_field_id conditional_field = 0;
		if (return_code && conditional_field_name)
		{
			cmzn_fieldmodule_id field_module = cmzn_region_get_fieldmodule(region);
			conditional_field = cmzn_fieldmodule_find_field_by_name(field_module, conditional_field_name);
			if (!conditional_field)
			{
				display_message(ERROR_MESSAGE,
					"gfx list elements:  conditional field cannot be found");
				return_code = 0;
			}
			cmzn_fieldmodule_destroy(&field_module);
		}
		if (return_code)
		{
			int use_dimension = dimension;
			if (dimension == 3)
			{
				use_dimension = FE_region_get_highest_dimension(cmzn_region_get_FE_region(region));
			}
			cmzn_fieldmodule_id field_module = cmzn_region_get_fieldmodule(region);
			cmzn_mesh_id master_mesh = cmzn_fieldmodule_find_mesh_by_dimension(field_module, use_dimension);
			cmzn_mesh_group_id selection_mesh_group = 0;
			if (selected_flag)
			{
				cmzn_scene *scene = cmzn_region_get_scene(region);
				cmzn_field_id selection_field = cmzn_scene_get_selection_field(scene);
				cmzn_field_group_id selection_group = cmzn_field_cast_group(selection_field);
				cmzn_field_destroy(&selection_field);
				if (selection_group)
				{
					cmzn_field_element_group_id selection_element_group =
						cmzn_field_group_get_field_element_group(selection_group, master_mesh);
					if (selection_element_group)
					{
						selection_mesh_group = cmzn_field_element_group_get_mesh_group(selection_element_group);
						cmzn_field_element_group_destroy(&selection_element_group);
					}
				}
				cmzn_field_group_destroy(&selection_group);
				cmzn_scene_destroy(&scene);
			}
			int number_of_elements_listed = 0;
			if ((!selected_flag) || selection_mesh_group)
			{
				cmzn_fieldcache_id cache = cmzn_fieldmodule_create_fieldcache(field_module);
				cmzn_mesh_id iteration_mesh = master_mesh;
				if (selected_flag)
				{
					iteration_mesh = cmzn_mesh_group_base_cast(selection_mesh_group);
				}
				const bool use_element_ranges = Multi_range_get_number_of_ranges(element_ranges) > 0;
				if (Multi_range_get_total_number_in_ranges(element_ranges) == 1)
					verbose_flag = 1;
				Multi_range *output_element_ranges = CREATE(Multi_range)();
				cmzn_elementiterator_id iter = cmzn_mesh_create_elementiterator(iteration_mesh);
				cmzn_element_id element = 0;
				while (NULL != (element = cmzn_elementiterator_next_non_access(iter)))
				{
					if (use_element_ranges && !Multi_range_is_value_in_range(element_ranges, cmzn_element_get_identifier(element)))
						continue;
					if (conditional_field)
					{
						cmzn_fieldcache_set_element(cache, element);
						if (!cmzn_field_evaluate_boolean(conditional_field, cache))
							continue;
					}
					if (verbose_flag)
					{
						if (!list_FE_element(region, field_module, master_mesh, element))
							break;
					}
					else
					{
						int element_identifier = cmzn_element_get_identifier(element);
						Multi_range_add_range(output_element_ranges, element_identifier, element_identifier);
					}
					++number_of_elements_listed;
				}
				cmzn_elementiterator_destroy(&iter);
				if ((!verbose_flag) && number_of_elements_listed)
				{
					if (dimension == 1)
					{
						display_message(INFORMATION_MESSAGE, "Lines:\n");
					}
					else if (dimension == 2)
					{
						display_message(INFORMATION_MESSAGE, "Faces:\n");
					}
					else
					{
						display_message(INFORMATION_MESSAGE, "Elements (dimension %d):\n", use_dimension);
					}
					return_code = Multi_range_display_ranges(output_element_ranges);
					display_message(INFORMATION_MESSAGE, "Total number = %d\n", number_of_elements_listed);
				}
				DESTROY(Multi_range)(&output_element_ranges);
				cmzn_fieldcache_destroy(&cache);
			}
			if (0 == number_of_elements_listed)
			{
				if (dimension == 1)
				{
					display_message(INFORMATION_MESSAGE, "gfx list lines:  No lines specified\n");
				}
				else if (dimension == 2)
				{
					display_message(INFORMATION_MESSAGE, "gfx list faces:  No faces specified\n");
				}
				else
				{
					display_message(INFORMATION_MESSAGE, "gfx list elements:  No elements specified\n");
				}
			}
			cmzn_mesh_group_destroy(&selection_mesh_group);
			cmzn_mesh_destroy(&master_mesh);
			cmzn_fieldmodule_destroy(&field_module);
		}
		DESTROY(Multi_range)(&element_ranges);
		cmzn_field_destroy(&conditional_field);
		if (conditional_field_name)
			DEALLOCATE(conditional_field_name);
		cmzn_region_destroy(&region);
	}
	else
	{
		display_message(ERROR_MESSAGE, "gfx_list_FE_element.  Missing state");
		return_code = 0;
	}
	return return_code;
}

static int gfx_list_FE_node(struct Parse_state *state,
	void *use_data,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 6 May 2003

DESCRIPTION :
Executes a GFX LIST NODES.
If <use_data> is set, use data_manager and data_selection, otherwise
use node_manager and node_selection.
==============================================================================*/
{
	int return_code;
	cmzn_command_data *command_data = reinterpret_cast<cmzn_command_data *>(command_data_void);
	if (state && command_data)
	{
		/* initialise defaults */
		char all_flag = 0;
		cmzn_region_id region = cmzn_region_access(command_data->root_region);
		char selected_flag = 0;
		char verbose_flag = 0;
		Multi_range *node_ranges = CREATE(Multi_range)();
		char *conditional_field_name = 0;

		Option_table *option_table = CREATE(Option_table)();
		/* all (redundant option) */
		Option_table_add_entry(option_table, "all", &all_flag, NULL, set_char_flag);
		/* region */
		Option_table_add_set_cmzn_region(option_table, "region",
			command_data->root_region, &region);
		/* selected */
		Option_table_add_entry(option_table, "selected", &selected_flag,
			NULL, set_char_flag);
		/* conditional */
		Option_table_add_string_entry(option_table, "conditional", &conditional_field_name,
			" FIELD_NAME");
		/* verbose */
		Option_table_add_entry(option_table, "verbose", &verbose_flag,
			NULL, set_char_flag);
		/* default option: node number ranges */
		Option_table_add_entry(option_table, (const char *)NULL, (void *)node_ranges,
			NULL, set_Multi_range);
		return_code = Option_table_multi_parse(option_table, state);
		DESTROY(Option_table)(&option_table);

		cmzn_field_id conditional_field = 0;
		if (return_code && conditional_field_name)
		{
			cmzn_fieldmodule_id field_module = cmzn_region_get_fieldmodule(region);
			conditional_field = cmzn_fieldmodule_find_field_by_name(field_module, conditional_field_name);
			if (!conditional_field)
			{
				display_message(ERROR_MESSAGE,
					"gfx list nodes:  conditional field cannot be found");
				return_code = 0;
			}
			cmzn_fieldmodule_destroy(&field_module);
		}
		if (return_code)
		{
			cmzn_fieldmodule_id field_module = cmzn_region_get_fieldmodule(region);
			cmzn_nodeset_id master_nodeset = cmzn_fieldmodule_find_nodeset_by_field_domain_type(field_module,
				use_data ? CMZN_FIELD_DOMAIN_TYPE_DATAPOINTS : CMZN_FIELD_DOMAIN_TYPE_NODES);
			cmzn_nodeset_group_id selection_nodeset_group = 0;
			if (selected_flag)
			{
				cmzn_scene *scene = cmzn_region_get_scene(region);
				cmzn_field_id selection_field = cmzn_scene_get_selection_field(scene);
				cmzn_field_group_id selection_group = cmzn_field_cast_group(selection_field);
				cmzn_field_destroy(&selection_field);
				if (selection_group)
				{
					cmzn_field_node_group_id selection_node_group =
						cmzn_field_group_get_field_node_group(selection_group, master_nodeset);
					if (selection_node_group)
					{
						selection_nodeset_group = cmzn_field_node_group_get_nodeset_group(selection_node_group);
						cmzn_field_node_group_destroy(&selection_node_group);
					}
				}
				cmzn_field_group_destroy(&selection_group);
				cmzn_scene_destroy(&scene);
			}
			int number_of_nodes_listed = 0;
			if ((!selected_flag) || selection_nodeset_group)
			{
				cmzn_fieldcache_id cache = cmzn_fieldmodule_create_fieldcache(field_module);
				cmzn_nodeset_id iteration_nodeset = master_nodeset;
				if (selected_flag)
				{
					iteration_nodeset = cmzn_nodeset_group_base_cast(selection_nodeset_group);
				}
				const bool use_node_ranges = Multi_range_get_number_of_ranges(node_ranges) > 0;
				if (Multi_range_get_total_number_in_ranges(node_ranges) == 1)
					verbose_flag = 1;
				Multi_range *output_node_ranges = CREATE(Multi_range)();
				cmzn_nodeiterator_id iter = cmzn_nodeset_create_nodeiterator(iteration_nodeset);
				cmzn_node_id node = 0;
				while (NULL != (node = cmzn_nodeiterator_next_non_access(iter)))
				{
					if (use_node_ranges && !Multi_range_is_value_in_range(node_ranges, cmzn_node_get_identifier(node)))
						continue;
					if (conditional_field)
					{
						cmzn_fieldcache_set_node(cache, node);
						if (!cmzn_field_evaluate_boolean(conditional_field, cache))
							continue;
					}
					if (verbose_flag)
					{
						list_FE_node(node);
					}
					else
					{
						int node_identifier = cmzn_node_get_identifier(node);
						Multi_range_add_range(output_node_ranges, node_identifier, node_identifier);
					}
					++number_of_nodes_listed;
				}
				cmzn_nodeiterator_destroy(&iter);
				if ((!verbose_flag) && number_of_nodes_listed)
				{
					display_message(INFORMATION_MESSAGE, use_data ? "Data:\n" : "Nodes:\n");
					return_code = Multi_range_display_ranges(output_node_ranges);
					display_message(INFORMATION_MESSAGE, "Total number = %d\n", number_of_nodes_listed);
				}
				DESTROY(Multi_range)(&output_node_ranges);
				cmzn_fieldcache_destroy(&cache);
			}
			if (0 == number_of_nodes_listed)
			{
				display_message(INFORMATION_MESSAGE,
					use_data ? "gfx list data:  No data specified\n" : "gfx list nodes:  No nodes specified\n");
			}
			cmzn_nodeset_group_destroy(&selection_nodeset_group);
			cmzn_nodeset_destroy(&master_nodeset);
			cmzn_fieldmodule_destroy(&field_module);
		}
		DESTROY(Multi_range)(&node_ranges);
		cmzn_field_destroy(&conditional_field);
		if (conditional_field_name)
			DEALLOCATE(conditional_field_name);
		cmzn_region_destroy(&region);
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_list_FE_node.  Missing state");
		return_code = 0;
	}
	return return_code;
}

static int gfx_list_graphical_material(struct Parse_state *state,
	void *dummy_to_be_modified,void *graphical_material_manager_void)
/*******************************************************************************
LAST MODIFIED : 22 September 1998

DESCRIPTION :
Executes a GFX LIST MATERIAL.
???RC Could be moved to material.c.
==============================================================================*/
{
	static const char	*command_prefix="gfx create material ";
	char commands_flag;
	int return_code;
	static struct Modifier_entry option_table[]=
	{
		{"commands",NULL,NULL,set_char_flag},
		{"name",NULL,NULL,set_Graphical_material},
		{NULL,NULL,NULL,set_Graphical_material}
	};
	cmzn_material *material;
	struct MANAGER(cmzn_material) *graphical_material_manager;

	ENTER(gfx_list_graphical_material);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (NULL != (graphical_material_manager=
			(struct MANAGER(cmzn_material) *)graphical_material_manager_void))
		{
			commands_flag=0;
			/* if no material specified, list all materials */
			material=(cmzn_material *)NULL;
			(option_table[0]).to_be_modified= &commands_flag;
			(option_table[1]).to_be_modified= &material;
			(option_table[1]).user_data= graphical_material_manager_void;
			(option_table[2]).to_be_modified= &material;
			(option_table[2]).user_data= graphical_material_manager_void;
			if (0 != (return_code = process_multiple_options(state,option_table)))
			{
				if (commands_flag)
				{
					if (material)
					{
						return_code=list_Graphical_material_commands(material,
							(void *)command_prefix);
					}
					else
					{
						return_code=FOR_EACH_OBJECT_IN_MANAGER(cmzn_material)(
							list_Graphical_material_commands,(void *)command_prefix,
							graphical_material_manager);
					}
				}
				else
				{
					if (material)
					{
						return_code=list_Graphical_material(material,(void *)NULL);
					}
					else
					{
						return_code=FOR_EACH_OBJECT_IN_MANAGER(cmzn_material)(
							list_Graphical_material,(void *)NULL,
							graphical_material_manager);
					}
				}
			}
			/* must deaccess material since accessed by set_Graphical_material */
			if (material)
			{
				cmzn_material_destroy(&material);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"gfx_list_graphical_material.  "
				"Missing graphical_material_manager_void");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_list_graphical_material.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_list_graphical_material */

/***************************************************************************//**
 * Executes a GFX LIST EGROUP/NGROUP/DGROUP command.
 */
static int gfx_list_group(struct Parse_state *state, void *dummy_to_be_modified,
	void *root_region_void)
{
	int return_code = 1;
	USE_PARAMETER(dummy_to_be_modified);
	cmzn_region_id root_region = reinterpret_cast<cmzn_region_id>(root_region_void);
	if (state && root_region)
	{
		cmzn_region_id region = cmzn_region_access(root_region);
		if ((!state->current_token) || (set_cmzn_region(state, (void *)&region, (void *)root_region) &&
			!Parse_state_help_mode(state)))
		{
			cmzn_fieldmodule_id field_module = cmzn_region_get_fieldmodule(region);
			cmzn_fielditerator_id field_iter = cmzn_fieldmodule_create_fielditerator(field_module);
			cmzn_field_id field = 0;
			char *region_path = cmzn_region_get_path(region);
			display_message(INFORMATION_MESSAGE, "Groups in region %s:\n", region_path);
			DEALLOCATE(region_path);
			while (0 != (field = cmzn_fielditerator_next_non_access(field_iter)))
			{
				cmzn_field_group_id group = cmzn_field_cast_group(field);
				if (group)
				{
					char *group_name = cmzn_field_get_name(field);
					display_message(INFORMATION_MESSAGE, "  %s\n", group_name);
					DEALLOCATE(group_name);
					cmzn_field_group_destroy(&group);
				}
			}
			cmzn_fielditerator_destroy(&field_iter);
			cmzn_fieldmodule_destroy(&field_module);
		}
		cmzn_region_destroy(&region);
	}
	else
	{
		display_message(ERROR_MESSAGE, "gfx_list_group.  Invalid argument(s)");
		return_code = 0;
	}
	return return_code;
}

static int gfx_list_region(struct Parse_state *state,
	void *dummy_to_be_modified, void *root_region_void)
/*******************************************************************************
LAST MODIFIED : 27 March 2003

DESCRIPTION :
Executes a GFX LIST REGION command.
==============================================================================*/
{
	const char *current_token;
	int return_code;
	struct cmzn_region *region, *root_region;

	ENTER(gfx_list_region);
	USE_PARAMETER(dummy_to_be_modified);
	if (state && (root_region = (struct cmzn_region *)root_region_void))
	{
		return_code = 1;
		if ((current_token = state->current_token) &&
			((0 == strcmp(PARSER_HELP_STRING,current_token)) ||
				(0 == strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))))
		{
			/* help */
			display_message(INFORMATION_MESSAGE, " REGION_PATH");
		}
		else
		{
			region = (struct cmzn_region *)NULL;
			if (current_token)
			{
				/* get region to be listed */
				if (cmzn_region_get_region_from_path_deprecated(root_region, current_token,
					&region))
				{
					display_message(INFORMATION_MESSAGE, "Region %s:\n", current_token);
				}
				else
				{
					display_message(ERROR_MESSAGE, "Unknown region: %s", current_token);
					display_parse_state_location(state);
					return_code = 0;
				}
			}
			else
			{
				region = root_region;
				display_message(INFORMATION_MESSAGE,
					"Region " CMZN_REGION_PATH_SEPARATOR_STRING ":\n");
			}
			if (return_code)
			{
				return_code = cmzn_region_list(region, 2, 2);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE, "gfx_list_region.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* gfx_list_region */

int cmzn_region_list_scene(cmzn_region_id region, int commands_flag, int recursive_flag)
{
	if (!region)
		return 0;
	int return_code = 1;
	char *region_path = cmzn_region_get_path(region);
	cmzn_scene_id scene = cmzn_region_get_scene(region);
	if (commands_flag)
	{
		int error = 0;
		char *command_prefix = duplicate_string("gfx modify g_element ");
		make_valid_token(&region_path);
		append_string(&command_prefix, region_path, &error);
		append_string(&command_prefix, " ", &error);
		return_code = cmzn_scene_list_commands(scene, command_prefix, /*command_suffix*/";");
		DEALLOCATE(command_prefix);
	}
	else
	{
		display_message(INFORMATION_MESSAGE,
			"Contents of region %s scene:\n", region_path);
		return_code = cmzn_scene_list_contents(scene);
	}
	cmzn_scene_destroy(&scene);
	DEALLOCATE(region_path);
	if (recursive_flag)
	{
		cmzn_region_id child = cmzn_region_get_first_child(region);
		while (child)
		{
			if (!cmzn_region_list_scene(child, commands_flag, recursive_flag))
			{
				cmzn_region_destroy(&child);
				return_code = 0;
				break;
			}
			cmzn_region_reaccess_next_sibling(&child);
		}
	}
	return return_code;
}

/***************************************************************************//**
 * Executes a GFX LIST G_ELEMENT.
 */
static int gfx_list_g_element(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
{
	int return_code;
	USE_PARAMETER(dummy_to_be_modified);
	cmzn_command_data *command_data = reinterpret_cast<cmzn_command_data *>(command_data_void);
	if (state && command_data)
	{
		int commands_flag = 1;
		int recursive_flag = 1;
		cmzn_region_id region = cmzn_region_access(command_data->root_region);
		Option_table *option_table = CREATE(Option_table)();
		/* commands|description */
		Option_table_add_switch(option_table, "commands", "description", &commands_flag);
		/* recursive|non_recursive */
		Option_table_add_switch(option_table, "recursive", "non_recursive", &recursive_flag);
		/* default option: region */
		Option_table_add_set_cmzn_region(option_table, /*token*/(const char *)0,
			command_data->root_region, &region);
		return_code = Option_table_multi_parse(option_table, state);
		DESTROY(Option_table)(&option_table);
		if (return_code)
		{
			return_code = cmzn_region_list_scene(region, commands_flag, recursive_flag);
		}
		cmzn_region_destroy(&region);
	}
	else
	{
		display_message(ERROR_MESSAGE, "gfx_list_g_element.  Invalid argument(s)");
		return_code = 0;
	}
	return (return_code);
}

static int cmzn_glyph_list_contents(cmzn_glyph *glyph, void *dummy_void)
{
	USE_PARAMETER(dummy_void);
	cmzn_glyph_static *glyphStatic = dynamic_cast<cmzn_glyph_static*>(glyph);
	cmzn_glyph_colour_bar *colourBar = dynamic_cast<cmzn_glyph_colour_bar*>(glyph);
	display_message(INFORMATION_MESSAGE, "%s = %s; access_count = %d\n", glyph->getName(),
		glyphStatic ? "static" : (colourBar ? "colour_bar" : "unknown"), glyph->access_count);
	return 1;
}

static int gfx_list_graphics_object(struct Parse_state *state,
	void *dummy_to_be_modified,void *glyphmodule_void)
/*******************************************************************************
LAST MODIFIED : 26 July 1998

DESCRIPTION :
Executes a GFX LIST GLYPH/GRAPHICS_OBJECT command.
==============================================================================*/
{
	const char *current_token;
	int return_code;

	ENTER(gfx_list_graphics_object);
	USE_PARAMETER(dummy_to_be_modified);
	cmzn_glyphmodule_id glyphmodule = reinterpret_cast<cmzn_glyphmodule *>(glyphmodule_void);
	if (state)
	{
		if (glyphmodule)
		{
			if (NULL != (current_token = state->current_token))
			{
				if (!Parse_state_help_mode(state))
				{
					cmzn_glyph_id glyph = cmzn_glyphmodule_find_glyph_by_name(glyphmodule, current_token);
					if (glyph)
					{
						cmzn_glyph_list_contents(glyph, 0);
						cmzn_glyph_destroy(&glyph);
					}
					else
					{
						display_message(ERROR_MESSAGE,"Could not find glyph '%s'",
							current_token);
						display_parse_state_location(state);
						return_code=0;
					}
				}
				else
				{
					display_message(INFORMATION_MESSAGE," OBJECT_NAME[ALL]");
					return_code=1;
				}
			}
			else
			{
				return_code = FOR_EACH_OBJECT_IN_MANAGER(cmzn_glyph)(
					cmzn_glyph_list_contents, (void *)NULL, cmzn_glyphmodule_get_manager(glyphmodule));
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_list_graphics_object.  Missing glyph module");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_list_graphics_object.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
}

static int gfx_list_grid_points(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 5 March 2003

DESCRIPTION :
Executes a GFX LIST GRID_POINTS.
If <used_data_flag> is set, use data_manager and data_selection, otherwise
use node_manager and node_selection.
==============================================================================*/
{
	char all_flag,ranges_flag,selected_flag;
	int return_code;
	struct cmzn_command_data *command_data;
	struct Element_point_ranges_grid_to_multi_range_data grid_to_multi_range_data;
	struct FE_element_grid_to_multi_range_data element_grid_to_multi_range_data;
	struct FE_field *grid_field;
	struct FE_region *fe_region;
	struct Multi_range *grid_point_ranges,*multi_range;
	struct Option_table *option_table;
	struct Set_FE_field_conditional_FE_region_data set_grid_field_data;

	ENTER(gfx_list_grid_points);
	USE_PARAMETER(dummy_to_be_modified);
	if (state&&(command_data=(struct cmzn_command_data *)command_data_void))
	{
		/* initialise defaults */
		all_flag=0;
		selected_flag=0;
		grid_point_ranges=CREATE(Multi_range)();
		fe_region = cmzn_region_get_FE_region(command_data->root_region);

		if ((grid_field = FE_region_get_FE_field_from_name(fe_region,
			"grid_point_number")) &&
			FE_field_is_1_component_integer(grid_field,(void *)NULL))
		{
			ACCESS(FE_field)(grid_field);
		}
		else
		{
			grid_field=(struct FE_field *)NULL;
		}

		option_table=CREATE(Option_table)();
		/* all */
		Option_table_add_entry(option_table,"all",&all_flag,NULL,set_char_flag);
		/* grid_field */
		set_grid_field_data.conditional_function = FE_field_is_1_component_integer;
		set_grid_field_data.user_data = (void *)NULL;
		set_grid_field_data.fe_region = fe_region;
		Option_table_add_entry(option_table, "grid_field", &grid_field,
			(void *)&set_grid_field_data, set_FE_field_conditional_FE_region);
		/* selected */
		Option_table_add_entry(option_table,"selected",&selected_flag,
			NULL,set_char_flag);
		/* default option: grid point number ranges */
		Option_table_add_entry(option_table,(char *)NULL,(void *)grid_point_ranges,
			NULL,set_Multi_range);
		if (0 != (return_code = Option_table_multi_parse(option_table, state)))
		{
			if (grid_field)
			{
				if (NULL != (multi_range=CREATE(Multi_range)()))
				{
					ranges_flag=(0<Multi_range_get_number_of_ranges(grid_point_ranges));
					if (selected_flag)
					{
						/* fill multi_range with selected grid_point_number ranges */
						grid_to_multi_range_data.grid_fe_field=grid_field;
						grid_to_multi_range_data.multi_range=multi_range;
						grid_to_multi_range_data.all_points_native=1;
						return_code=FOR_EACH_OBJECT_IN_LIST(Element_point_ranges)(
							Element_point_ranges_grid_to_multi_range,
							(void *)&grid_to_multi_range_data,
							Element_point_ranges_selection_get_element_point_ranges_list(
								command_data->element_point_ranges_selection));
					}
					else if (ranges_flag||all_flag)
					{
						/* fill multi_range with all grid_point_number ranges */
						element_grid_to_multi_range_data.grid_fe_field=grid_field;
						element_grid_to_multi_range_data.multi_range=multi_range;
						return_code = FE_region_for_each_FE_element(fe_region,
							FE_element_grid_to_multi_range,
							(void *)&element_grid_to_multi_range_data);
					}
					if (return_code)
					{
						if (ranges_flag)
						{
							/* include in multi_range only values also in grid_point_ranges */
							Multi_range_intersect(multi_range,grid_point_ranges);
						}
						if (0<Multi_range_get_number_of_ranges(multi_range))
						{
							display_message(INFORMATION_MESSAGE,"Grid points:\n");
							return_code=Multi_range_display_ranges(multi_range);
						}
						else
						{
							display_message(WARNING_MESSAGE,
								"gfx list grid_points:  No grid points specified");
						}
					}
					DESTROY(Multi_range)(&multi_range);
				}
				else
				{
					return_code=0;
				}
				if (!return_code)
				{
					display_message(ERROR_MESSAGE,"gfx_list_grid_points.  Failed");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,"To list grid_points, "
					"need integer grid_field (eg. grid_point_number)");
				return_code=0;
			}
		}
		DESTROY(Option_table)(&option_table);
		DESTROY(Multi_range)(&grid_point_ranges);
		if (grid_field)
		{
			DEACCESS(FE_field)(&grid_field);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_list_grid_points.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_list_grid_points */

static int gfx_list_light(struct Parse_state *state,
	void *dummy_to_be_modified,void *light_manager_void)
/*******************************************************************************
LAST MODIFIED : 2 September 1996

DESCRIPTION :
Executes a GFX LIST LIGHT.
==============================================================================*/
{
	const char *current_token;
	int return_code;
	struct cmzn_light *light;
	struct MANAGER(cmzn_light) *light_manager;

	ENTER(gfx_list_light);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (NULL != (light_manager=(struct MANAGER(cmzn_light) *)light_manager_void))
		{
			if (NULL != (current_token = state->current_token))
			{
				if (strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
				{
					if (NULL != (light=FIND_BY_IDENTIFIER_IN_MANAGER(cmzn_light,name)(current_token,
						light_manager)))
					{
						return_code=list_cmzn_light(light,(void *)NULL);
					}
					else
					{
						display_message(ERROR_MESSAGE,"Unknown light: %s",current_token);
						display_parse_state_location(state);
						return_code=0;
					}
				}
				else
				{
					display_message(INFORMATION_MESSAGE," LIGHT_NAME");
					return_code=1;
				}
			}
			else
			{
				return_code=FOR_EACH_OBJECT_IN_MANAGER(cmzn_light)(list_cmzn_light,(void *)NULL,
					light_manager);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_list_light.  Missing light_manager_void");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_list_light.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_list_light */

static int gfx_list_light_model(struct Parse_state *state,
	void *dummy_to_be_modified,void *light_model_manager_void)
/*******************************************************************************
LAST MODIFIED : 3 September 1996

DESCRIPTION :
Executes a GFX LIST LMODEL.
==============================================================================*/
{
	int return_code = 1;

	if (state)
	{
		const char *current_token;
		if (NULL != (current_token = state->current_token))
		{
			display_message(INFORMATION_MESSAGE,
				"gfx_list_light_model.  Light model is no longer supported, it is replaced by light with"
				"ambient type.");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_list_light_model.  Missing state");
		return_code=0;
	}

	return (return_code);
} /* gfx_list_light_model */

/**
 * Executes a GFX LIST SCENE command.
 */
static int gfx_list_scene(struct Parse_state *state,
	void *dummy_to_be_modified, void *root_region_void)
{
	int return_code = 1;
	if (state && root_region_void)
	{
		display_message(WARNING_MESSAGE, "gfx list scene:  Scene is now the direct graphical "
			"representation of a region, please use gfx list g_element instead.");
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_list_scene.  Invalid argument(s)");
		return_code = 0;
	}
	return (return_code);
}

static int gfx_list_spectrum(struct Parse_state *state,
	void *dummy_to_be_modified,void *spectrum_manager_void)
/*******************************************************************************
LAST MODIFIED : 28 November 2000

DESCRIPTION :
Executes a GFX LIST SPECTRUM.
==============================================================================*/
{
	static const char	*command_prefix="gfx modify spectrum";
	char *commands_flag;
	int return_code;
	struct MANAGER(cmzn_spectrum) *spectrum_manager;
	struct cmzn_spectrum *spectrum;
	struct Option_table *option_table;

	ENTER(gfx_list_spectrum);
	USE_PARAMETER(dummy_to_be_modified);
	if (state && (spectrum_manager =
		(struct MANAGER(cmzn_spectrum) *)spectrum_manager_void))
	{
		commands_flag = 0;
		spectrum = (struct cmzn_spectrum *)NULL;

		option_table=CREATE(Option_table)();
		/* commands */
		Option_table_add_entry(option_table, "commands", &commands_flag,
			NULL, set_char_flag);
		/* default option: spectrum name */
		Option_table_add_entry(option_table, (const char *)NULL, &spectrum,
			spectrum_manager_void, set_Spectrum);
		if (0 != (return_code = Option_table_multi_parse(option_table, state)))
		{
			if (commands_flag)
			{
				if (spectrum)
				{
					display_message(INFORMATION_MESSAGE,
						"Commands for reproducing spectrum:\n");
					return_code = Spectrum_list_commands(spectrum,
						command_prefix, (char *)NULL);
				}
				else
				{
					display_message(INFORMATION_MESSAGE," SPECTRUM_NAME\n");
					return_code = 1;
				}
			}
			else
			{
				if (spectrum)
				{
					return_code = Spectrum_list_app_contents(spectrum, (void *)NULL);
				}
				else
				{
					return_code = FOR_EACH_OBJECT_IN_MANAGER(cmzn_spectrum)(
						Spectrum_list_app_contents, (void *)NULL, spectrum_manager);
				}
			}
		}
		DESTROY(Option_table)(&option_table);
		if (spectrum)
		{
			DEACCESS(cmzn_spectrum)(&spectrum);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_list_spectrum.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* gfx_list_spectrum */

static int gfx_list_texture(struct Parse_state *state,
	void *dummy_to_be_modified,void *region_void)
/*******************************************************************************
LAST MODIFIED : 19 May 1999

DESCRIPTION :
Executes a GFX LIST TEXTURE.
???RC Could be moved to texture.c.
==============================================================================*/
{
	static const char	*command_prefix="gfx create texture ";
	char *region_name = NULL, *field_name = NULL;
	char commands_flag;
	int return_code = 0;
	struct cmzn_region *region = NULL, *root_region = NULL;
	struct Option_table *option_table;

	ENTER(gfx_list_texture);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		root_region=(struct cmzn_region *)region_void;
		if (root_region)
		{
			commands_flag=0;
			/* if no texture specified, list all textures */
			option_table=CREATE(Option_table)();
			Option_table_add_entry(option_table,"region",&region_name,
				(void *)1,set_name);
			Option_table_add_entry(option_table, "commands", &commands_flag,
				NULL, set_char_flag);
			Option_table_add_entry(option_table,"field",&field_name,
				(void *)1,set_name);
			return_code=Option_table_multi_parse(option_table,state);
			if (return_code)
			{
				if (region_name)
				{
					region = cmzn_region_find_subregion_at_path(root_region,
						region_name);
					if (!region)
					{
						display_message(ERROR_MESSAGE, "No region named '%s'",region_name);
					}
				}
				else
				{
					region = ACCESS(cmzn_region)(root_region);
					region_name = cmzn_region_get_path(region);
				}
				if (region)
				{
					MANAGER(Computed_field) *field_manager =
						cmzn_region_get_Computed_field_manager(region);
					Computed_field *existing_field = NULL;
					if (field_name)
					{
						existing_field =	FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field,name)(
							(char *)field_name, field_manager);
						if (existing_field)
						{
							if (Computed_field_is_image_type(existing_field, NULL))
							{
								if (commands_flag)
								{
									return_code=list_image_field_commands(existing_field,(void *)command_prefix);
								}
								else
								{
									return_code=list_image_field(existing_field,NULL);
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,"gfx_list_texture.  "
									"Field specified does not contains a texture");
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,"gfx_list_texture.  "
								"Field specified does not exist in region.");
							return_code = 0;
						}
					}
					else
					{
						if (commands_flag)
						{
							return_code=FOR_EACH_OBJECT_IN_MANAGER(Computed_field)(
								list_image_field_commands,(void *)command_prefix,field_manager);
						}
						else
						{
							return_code=FOR_EACH_OBJECT_IN_MANAGER(Computed_field)(
									list_image_field,(void *)NULL,field_manager);
						}
					}
					DEACCESS(cmzn_region)(&region);
				}
			}
			DESTROY(Option_table)(&option_table);
			if (region_name)
				DEALLOCATE(region_name);
			if (field_name)
				DEALLOCATE(field_name);
		}
		else
		{
			display_message(ERROR_MESSAGE,"gfx_list_texture.  "
				"Missing root_region");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_list_texture.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_list_texture */


static int gfx_list_transformation(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 14 March 2001

DESCRIPTION :
Executes a GFX LIST TRANSFORMATION.
==============================================================================*/
{
	char commands_flag,*region_name = NULL;
	int return_code;
	struct cmzn_command_data *command_data;
	struct cmzn_region *region = NULL;
	static struct Modifier_entry option_table[]=
	{
		{"commands",NULL,NULL,set_char_flag},
		{"region",NULL,(void *)1,set_name},
		{NULL,NULL,NULL,NULL}
	};

	ENTER(gfx_list_transformation);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (NULL != (command_data = (struct cmzn_command_data *)command_data_void))
		{
			/* initialise defaults */
			commands_flag=0;
			/* parse the command line */
			(option_table[0]).to_be_modified= &commands_flag;
			(option_table[1]).to_be_modified= &region_name;
			return_code=process_multiple_options(state,option_table);
			/* no errors, not asking for help */
			if (return_code)
			{
				if (region_name)
				{
					region = cmzn_region_find_subregion_at_path(command_data->root_region,
						region_name);
					if (!region)
					{
						display_message(ERROR_MESSAGE, "No region named '%s'",region_name);
					}
				}
				else
				{
					region = ACCESS(cmzn_region)(command_data->root_region);
					region_name = cmzn_region_get_path(region);
				}
				if (region)
				{
					cmzn_scene *scene = cmzn_region_get_scene(region);
					if (scene)
					{
						if (commands_flag)
						{
							/* quote scene name if it contains special characters */
							make_valid_token(&region_name);
							char *command_prefix;
							if (ALLOCATE(command_prefix, char, 40 + strlen(region_name)))
							{
								sprintf(command_prefix, "gfx set transformation name ");
								if (scene)
								{
									return_code = list_cmzn_scene_transformation_commands(
										scene,(void *)command_prefix);
									DEACCESS(cmzn_scene)(&scene);
								}
								DEALLOCATE(command_prefix);
							}
							else
							{
								return_code=0;
							}
						}
						else
						{
							return_code = list_cmzn_scene_transformation(scene);
						}
						cmzn_scene_destroy(&scene);
					}
				}
				else
				{
					return_code=0;
				}
			} /* parse error, help */
			DEACCESS(cmzn_region)(&region);
			if (region_name)
			{
				DEALLOCATE(region_name);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_list_transformation.  Missing command_data_void");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_list_transformation.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_list_transformation */

#if defined (SGI_MOVIE_FILE)
static int gfx_list_movie_graphics(struct Parse_state *state,
	void *dummy_to_be_modified,void *movie_graphics_manager_void)
/*******************************************************************************
LAST MODIFIED : 3 February 2000

DESCRIPTION :
Executes a GFX LIST MOVIE.
==============================================================================*/
{
	const char *current_token;
	int return_code;
	struct Movie_graphics *movie;
	struct MANAGER(Movie_graphics) *movie_graphics_manager;

	ENTER(gfx_list_movie_graphics);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (movie_graphics_manager=
			(struct MANAGER(Movie_graphics) *)movie_graphics_manager_void)
		{
			if (NULL != (current_token = state->current_token))
			{
				if (strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
				{
					if (movie=FIND_BY_IDENTIFIER_IN_MANAGER(Movie_graphics,name)(
						current_token,movie_graphics_manager))
					{
						return_code=list_Movie_graphics(movie,(void *)NULL);
					}
					else
					{
						display_message(ERROR_MESSAGE,"Unknown volume movie: %s",
							current_token);
						return_code=0;
					}
				}
				else
				{
					display_message(INFORMATION_MESSAGE," MOVIE_NAME");
					return_code=1;
				}
			}
			else
			{
				return_code=FOR_EACH_OBJECT_IN_MANAGER(Movie_graphics)(
					list_Movie_graphics,(void *)NULL,movie_graphics_manager);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_list_movie_graphics.  Missing movie_graphics_manager_void");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_list_movie_graphics.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_list_movie_graphics */
#endif /* defined (SGI_MOVIE_FILE) */

#if defined (USE_CMGUI_GRAPHICS_WINDOW)
static int gfx_list_graphics_window(struct Parse_state *state,
	void *dummy_to_be_modified,void *graphics_window_manager_void)
/*******************************************************************************
LAST MODIFIED : 14 October 1998

DESCRIPTION :
Executes a GFX LIST WINDOW.
==============================================================================*/
{
	char commands_flag;
	int return_code;
	static struct Modifier_entry option_table[]=
	{
		{"commands",NULL,NULL,set_char_flag},
		{"name",NULL,NULL,set_Graphics_window},
		{NULL,NULL,NULL,set_Graphics_window}
	};
	struct Graphics_window *window;
	struct MANAGER(Graphics_window) *graphics_window_manager;

	ENTER(gfx_list_graphics_window);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (NULL != (graphics_window_manager=
			(struct MANAGER(Graphics_window) *)graphics_window_manager_void))
		{
			commands_flag=0;
			/* if no window specified, list all windows */
			window=(struct Graphics_window *)NULL;
			(option_table[0]).to_be_modified= &commands_flag;
			(option_table[1]).to_be_modified= &window;
			(option_table[1]).user_data= graphics_window_manager_void;
			(option_table[2]).to_be_modified= &window;
			(option_table[2]).user_data= graphics_window_manager_void;
			if (0 != (return_code = process_multiple_options(state,option_table)))
			{
				if (commands_flag)
				{
					if (window)
					{
						return_code=list_Graphics_window_commands(window,(void *)NULL);
					}
					else
					{
						return_code=FOR_EACH_OBJECT_IN_MANAGER(Graphics_window)(
							list_Graphics_window_commands,(void *)NULL,
							graphics_window_manager);
					}
				}
				else
				{
					if (window)
					{
						return_code=list_Graphics_window(window,(void *)NULL);
					}
					else
					{
						return_code=FOR_EACH_OBJECT_IN_MANAGER(Graphics_window)(
							list_Graphics_window,(void *)NULL,
							graphics_window_manager);
					}
				}
			}
			/* must deaccess window since accessed by set_Graphics_window */
			if (window)
			{
				DEACCESS(Graphics_window)(&window);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"gfx_list_graphics_window.  "
				"Missing graphics_window_manager_void");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_list_graphics_window.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_list_graphics_window */
#endif /* defined (USE_CMGUI_GRAPHICS_WINDOW) */

static int execute_command_gfx_list(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 28 November 2000

DESCRIPTION :
Executes a GFX LIST command.
==============================================================================*/
{
	int return_code;
	struct cmzn_command_data *command_data;
	struct Option_table *option_table;

	ENTER(execute_command_gfx_list);
	USE_PARAMETER(dummy_to_be_modified);
	if (state && (command_data=(struct cmzn_command_data *)command_data_void))
	{
		if (state->current_token)
		{
			option_table = CREATE(Option_table)();
			/* all_commands */
			Option_table_add_entry(option_table, "all_commands", NULL,
				command_data_void, gfx_list_all_commands);
			/* btree_statistics */
			Option_table_add_entry(option_table, "btree_statistics", NULL,
				(void *)command_data->root_region, gfx_list_btree_statistics);
#if defined (USE_OPENCASCADE)
			/* cad */
			Option_table_add_entry(option_table, "cad", NULL,
				(void *)command_data->root_region, gfx_list_cad_entity);
#endif /* defined (USE_OPENCASCADE) */
			/* data */
			Option_table_add_entry(option_table, "data", /*use_data*/(void *)1,
				command_data_void, gfx_list_FE_node);
			/* element */
			Option_table_add_entry(option_table, "elements", /*dimension=highest*/(void *)3,
				command_data_void, gfx_list_FE_element);
			/* environment_map */
			Option_table_add_entry(option_table, "environment_map", NULL,
				command_data_void, gfx_list_environment_map);
			/* faces */
			Option_table_add_entry(option_table, "faces", /*dimension*/(void *)2,
				command_data_void, gfx_list_FE_element);
			/* field */
			Option_table_add_entry(option_table, "field", NULL,
				(void *)command_data->root_region, gfx_list_Computed_field);
			/* g_element */
			Option_table_add_entry(option_table, "g_element", NULL,
				command_data_void, gfx_list_g_element);
			/* glyph */
			Option_table_add_entry(option_table, "glyph", NULL,
				command_data->glyphmodule, gfx_list_graphics_object);
			/* graphics_filter */
			Option_table_add_entry(option_table, "graphics_filter", NULL,
				(void *)command_data->filter_module, gfx_list_graphics_filter);
			/* grid_points */
			Option_table_add_entry(option_table, "grid_points", NULL,
				command_data_void, gfx_list_grid_points);
			/* group */
			Option_table_add_entry(option_table, "group", (void *)0,
				command_data->root_region, gfx_list_group);
			/* light */
			Option_table_add_entry(option_table, "light", NULL,
				cmzn_lightmodule_get_manager(command_data->lightmodule), gfx_list_light);
			/* lines */
			Option_table_add_entry(option_table, "lines", /*dimension*/(void *)1,
				command_data_void, gfx_list_FE_element);
			/* lmodel */
			Option_table_add_entry(option_table, "lmodel", NULL,
				NULL, gfx_list_light_model);
			/* material */
			Option_table_add_entry(option_table, "material", NULL,
				cmzn_materialmodule_get_manager(command_data->materialmodule), gfx_list_graphical_material);
#if defined (SGI_MOVIE_FILE)
			/* movie */
			Option_table_add_entry(option_table, "movie", NULL,
				command_data->movie_graphics_manager, gfx_list_movie_graphics);
#endif /* defined (SGI_MOVIE_FILE) */
			/* nodes */
			Option_table_add_entry(option_table, "nodes", /*use_data*/(void *)0,
				command_data_void, gfx_list_FE_node);
			/* region */
			Option_table_add_entry(option_table, "region", NULL,
				command_data->root_region, gfx_list_region);
			/* scene */
			Option_table_add_entry(option_table, "scene", NULL,
				command_data->root_region, gfx_list_scene);
			/* spectrum */
			Option_table_add_entry(option_table, "spectrum", NULL,
				command_data->spectrum_manager, gfx_list_spectrum);
			/* tessellation */
			Option_table_add_entry(option_table, "tessellation", NULL,
				command_data->tessellationmodule, gfx_list_tessellation);
			/* texture */
			Option_table_add_entry(option_table, "texture", NULL,
					command_data->root_region, gfx_list_texture);
			/* transformation */
			Option_table_add_entry(option_table, "transformation", NULL,
				command_data_void, gfx_list_transformation);
#if defined (USE_CMGUI_GRAPHICS_WINDOW)
			/* graphics window */
			Option_table_add_entry(option_table, "window", NULL,
				command_data->graphics_window_manager, gfx_list_graphics_window);
#endif /* defined (USE_CMGUI_GRAPHICS_WINDOW) */
			return_code = Option_table_parse(option_table, state);
			DESTROY(Option_table)(&option_table);
		}
		else
		{
			set_command_prompt("gfx list", command_data);
			return_code = 1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_command_gfx_list.  Missing state");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_gfx_list */

/***************************************************************************//**
 * Modifies the membership of a group.  Only one of <add> or <remove> can
 * be specified at once.
 */
static int gfx_modify_element_group(struct Parse_state *state,
	void *dummy_to_be_modified, void *command_data_void)
{
	int return_code = 0;
	USE_PARAMETER(dummy_to_be_modified);
	cmzn_command_data *command_data = reinterpret_cast<cmzn_command_data *>(command_data_void);
	if (state && command_data)
	{
		cmzn_region_id region = cmzn_region_access(command_data->root_region);
		cmzn_field_group_id group = 0;
		const char *current_token = state->current_token;
		if (!set_cmzn_region_or_group(state, &region, &group))
		{
			// message already output
		}
		else if (!Parse_state_help_mode(state) && (!group))
		{
			display_message(ERROR_MESSAGE, "Not a group: %s", current_token);
			return_code = 0;
		}
		else
		{
			const char *elements_type_string = "elements";
			const char *faces_type_string = "faces";
			const char *lines_type_string = "lines";
			const char *element_type_strings[3] = { elements_type_string, faces_type_string, lines_type_string };
			/* initialise defaults */
			char add_flag = 0;
			char remove_flag = 0;
			char *conditional_field_name = 0;
			const char *element_type_string = elements_type_string;
			char all_flag = 0;
			char selected_flag = 0;
			Multi_range *element_ranges = CREATE(Multi_range)();
			char *from_group_name = 0;
			double time = 0;
			if (command_data->default_time_keeper_app)
			{
				time = command_data->default_time_keeper_app->getTimeKeeper()->getTime();
			}
			int manage_subobjects = 1; // add faces, lines and nodes with elements, remove if solely in use by removed elements

			Option_table *option_table = CREATE(Option_table)();
			/* add */
			Option_table_add_char_flag_entry(option_table, "add", &add_flag);
			/* all */
			Option_table_add_char_flag_entry(option_table, "all", &all_flag);
			/* conditional_field */
			Option_table_add_string_entry(option_table, "conditional_field", &conditional_field_name,
				" FIELD_NAME");
			/* elements|faces|lines */
			Option_table_add_enumerator(option_table, 3, element_type_strings, &element_type_string);
			/* group */
			Option_table_add_string_entry(option_table, "group", &from_group_name, " GROUP_NAME");
			/* manage_subobjects|no_manage_subobjects */
			Option_table_add_switch(option_table, "manage_subobjects", "no_manage_subobjects", &manage_subobjects);
			/* remove */
			Option_table_add_char_flag_entry(option_table, "remove", &remove_flag);
			/* selected */
			Option_table_add_char_flag_entry(option_table, "selected", &selected_flag);
			/* default option: element number ranges */
			Option_table_add_entry(option_table, (const char *)NULL, (void *)element_ranges,
				NULL, set_Multi_range);
			return_code = Option_table_multi_parse(option_table, state);
			DESTROY(Option_table)(&option_table);

			cmzn_field_id conditional_field = 0;
			if (return_code && conditional_field_name)
			{
				cmzn_fieldmodule_id field_module = cmzn_region_get_fieldmodule(region);
				conditional_field = cmzn_fieldmodule_find_field_by_name(field_module, conditional_field_name);
				if (!conditional_field)
				{
					display_message(ERROR_MESSAGE,
						"gfx modify egroup:  conditional field cannot be found");
					return_code = 0;
				}
				cmzn_fieldmodule_destroy(&field_module);
			}
			cmzn_field_group_id from_group = 0;
			if (return_code)
			{
				if (from_group_name)
				{
					cmzn_field *field =	FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field,name)(
						from_group_name, cmzn_region_get_Computed_field_manager(region));
					from_group = cmzn_field_cast_group(field);
					if (!from_group)
					{
						display_message(ERROR_MESSAGE, "gfx modify egroup: '%s' is not a group.", from_group_name);
						return_code = 0;
					}
				}
				if ((add_flag + remove_flag) != 1)
				{
					display_message(ERROR_MESSAGE, "gfx modify egroup:  Must specify operation 'add' or 'remove'.");
					return_code = 0;
				}
				if ((0 == Multi_range_get_number_of_ranges(element_ranges)) && (!selected_flag) && (0 == conditional_field) && (!all_flag) && (0 == from_group))
				{
					display_message(ERROR_MESSAGE, "gfx modify egroup:  No elements specified.");
					return_code = 0;
				}
			}
			if (return_code)
			{
				int dimension = 0;
				if (element_type_string == lines_type_string)
				{
					dimension = 1;
				}
				else if (element_type_string == faces_type_string)
				{
					dimension = 2;
				}
				else
				{
					dimension = FE_region_get_highest_dimension(cmzn_region_get_FE_region(region));
				}
				// following code
				return_code = process_modify_element_group(group, region, dimension,
					add_flag, conditional_field, from_group,
					Multi_range_get_number_of_ranges(element_ranges) > 0 ? element_ranges : 0,
					selected_flag, time, manage_subobjects ?
						CMZN_FIELD_GROUP_SUBELEMENT_HANDLING_MODE_FULL : CMZN_FIELD_GROUP_SUBELEMENT_HANDLING_MODE_NONE);
			}
			if (from_group_name)
			{
				DEALLOCATE(from_group_name);
			}
			cmzn_field_group_destroy(&from_group);
			DESTROY(Multi_range)(&element_ranges);
			cmzn_field_destroy(&conditional_field);
			if (conditional_field_name)
				DEALLOCATE(conditional_field_name);
		}
		cmzn_field_group_destroy(&group);
		cmzn_region_destroy(&region);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_modify_element_group.  Invalid argument(s)");
		return_code = 0;
	}
	return (return_code);
}

/***************************************************************************//**
 * Renames a field.
 */
static int gfx_modify_field_rename(struct Parse_state *state,
	void *field_void, void *region_void)
{
	int return_code = 1;
	ENTER(gfx_modify_field_rename);
	cmzn_field* field = (cmzn_field*)field_void;
	USE_PARAMETER(region_void);
	if (state)
	{
		if (!state->current_token || (
			strcmp(PARSER_HELP_STRING,state->current_token) &&
			strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
		{
			if (state->current_token)
			{
				return_code = cmzn_field_set_name(field, state->current_token);
			}
			else
			{
				display_message(ERROR_MESSAGE,"Missing new field name");
				display_parse_state_location(state);
				return_code = 0;
			}
		}
		else
		{
			/* Help */
			display_message(INFORMATION_MESSAGE, " NEW_FIELD_NAME");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_modify_field_rename.  Invalid argument(s)");
	}
	LEAVE;
	return (return_code);
}

/***************************************************************************//**
 * Executes GFX MODIFY FIELD subcommands.
 */
static int gfx_modify_field_subcommands(struct Parse_state *state,
	void *field_void, void *region_void)
{
	int return_code = 1;
	ENTER(gfx_modify_field_subcommands);
	if (state)
	{
		Option_table *option_table = CREATE(Option_table)();
		Option_table_add_entry(option_table, "rename",
			field_void, region_void, gfx_modify_field_rename);
		return_code = Option_table_parse(option_table, state);
		DESTROY(Option_table)(&option_table);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_modify_field_subcommands.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;
	return (return_code);
}

/***************************************************************************//**
 * Executes a GFX MODIFY FIELD command.
 */
static int gfx_modify_field(struct Parse_state *state,
	void *dummy_to_be_modified, void *command_data_void)
{
	int return_code = 1;
	ENTER(gfx_modify_field);
	USE_PARAMETER(dummy_to_be_modified);
	cmzn_command_data *command_data = (struct cmzn_command_data *)command_data_void;
	if (state && command_data)
	{
		const char *current_token = state->current_token;
		if (current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				cmzn_field_id field = 0;
				cmzn_region *region = 0;
				char *region_path = NULL;
				char *field_name = NULL;
				if (cmzn_region_get_partial_region_path(command_data->root_region,
					current_token, &region, &region_path, &field_name))
				{
					cmzn_fieldmodule_id field_module = cmzn_region_get_fieldmodule(region);
					field = cmzn_fieldmodule_find_field_by_name(field_module, field_name);
					cmzn_fieldmodule_destroy(&field_module);
				}
				if (field)
				{
					shift_Parse_state(state, 1);
					return_code = gfx_modify_field_subcommands(state,
						(void *)field, (void *)region);
					cmzn_field_destroy(&field);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"gfx modify field:  Invalid region path or field name '%s'", current_token);
					display_parse_state_location(state);
					return_code = 0;
				}
				DEALLOCATE(region_path);
				DEALLOCATE(field_name);
			}
			else
			{
				/* Write out the help */
				Option_table *help_option_table = CREATE(Option_table)();
				Option_table_add_entry(help_option_table,
					"[REGION_PATH" CMZN_REGION_PATH_SEPARATOR_STRING "]FIELD_NAME",
					(void *)NULL, (void *)command_data->root_region, gfx_modify_field_subcommands);
				return_code = Option_table_parse(help_option_table,state);
				DESTROY(Option_table)(&help_option_table);
				return_code = 1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Missing region_path/field_name");
			display_parse_state_location(state);
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_modify_field.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;
	return (return_code);
}

static int gfx_modify_g_element(struct Parse_state *state,
	void *help_mode,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 6 March 2003

DESCRIPTION :
Executes a GFX MODIFY G_ELEMENT command.
Parameter <help_mode> should be NULL when calling this function.
==============================================================================*/
{
	int return_code;
	struct cmzn_command_data *command_data;

	ENTER(gfx_modify_g_element);
	if (state && (command_data = (struct cmzn_command_data *)command_data_void))
	{
		return_code = 1;
		cmzn_region_id region = cmzn_region_access(command_data->root_region);
		cmzn_field_group_id group = 0;
		if (!help_mode)
		{
			struct Option_table *option_table = CREATE(Option_table)();
			if (!state->current_token ||
				(strcmp(PARSER_HELP_STRING, state->current_token) &&
					strcmp(PARSER_RECURSIVE_HELP_STRING, state->current_token)))
			{
				Option_table_add_region_or_group_entry(option_table, /*token*/0, &region, &group);
			}
			else
			{
				/* help: call this function in help_mode */
				Option_table_add_entry(option_table, "REGION_PATH",
					/*help_mode*/(void *)1, command_data_void, gfx_modify_g_element);
			}
			return_code = Option_table_parse(option_table, state);
			DESTROY(Option_table)(&option_table);
		}
		if (return_code)
		{
			struct cmzn_scene *scene = cmzn_region_get_scene(region);
			return_code = cmzn_scene_execute_command_internal(scene, group, state);
			cmzn_scene_destroy(&scene);
		} /* parse error,help */
		cmzn_region_destroy(&region);
		if (group)
		{
			cmzn_field_group_destroy(&group);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_modify_g_element.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* gfx_modify_g_element */

static int gfx_modify_glyph(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 4 June 1999

DESCRIPTION :
Executes a GFX MODIFY GRAPHICS_OBJECT command.
==============================================================================*/
{
	int return_code;
	struct cmzn_command_data *command_data;
	cmzn_material *material;
	struct Option_table *option_table;
	struct cmzn_spectrum *spectrum;

	ENTER(gfx_modify_glyph);
	USE_PARAMETER(dummy_to_be_modified);
	if (state&&(command_data=(struct cmzn_command_data *)command_data_void))
	{
		if ((!state->current_token) || (!Parse_state_help_mode(state)))
		{
			cmzn_glyph_id glyph = cmzn_glyphmodule_find_glyph_by_name(command_data->glyphmodule, state->current_token);
			cmzn_glyph_static *glyphStatic = dynamic_cast<cmzn_glyph_static *>(glyph);
			if (glyphStatic)
			{
				shift_Parse_state(state,1);
				GT_object *graphics_object = glyphStatic->getGraphicsObject();
				/* initialise defaults */
				if (NULL != (material = get_GT_object_default_material(graphics_object)))
				{
					cmzn_material_access(material);
				}
				if (NULL != (spectrum = get_GT_object_spectrum(graphics_object)))
				{
					ACCESS(cmzn_spectrum)(spectrum);
				}
				option_table = CREATE(Option_table)();
				Option_table_add_set_Material_entry(option_table, "material",&material,
					command_data->materialmodule);
				Option_table_add_entry(option_table,"spectrum",&spectrum,
					command_data->spectrum_manager,set_Spectrum);
				return_code = Option_table_multi_parse(option_table, state);
				DESTROY(Option_table)(&option_table);
				if (return_code)
				{
					set_GT_object_default_material(graphics_object, material);
					set_GT_object_Spectrum(graphics_object, spectrum);
					glyph->changed();
				}
				if (material)
				{
					cmzn_material_destroy(&material);
				}
				if (spectrum)
				{
					DEACCESS(cmzn_spectrum)(&spectrum);
				}
				DEACCESS(GT_object)(&graphics_object);
			}
			else
			{
				if (state->current_token)
				{
					if (glyph)
					{
						display_message(ERROR_MESSAGE,"Cannot modify non-static glyph '%s'",
							state->current_token);
					}
					else
					{
						display_message(ERROR_MESSAGE,"Could not find glyph named '%s'",
							state->current_token);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,"Missing glyph name");
				}
				display_parse_state_location(state);
				return_code=0;
			}
			cmzn_glyph_destroy(&glyph);
		}
		else
		{
			/* Help */
			display_message(INFORMATION_MESSAGE,
				"\n      GRAPHICS_OBJECT_NAME");

			spectrum = (struct cmzn_spectrum *)NULL;
			material = (cmzn_material *)NULL;
			option_table=CREATE(Option_table)();
			Option_table_add_set_Material_entry(option_table, "material",&material,
				command_data->materialmodule);
			Option_table_add_entry(option_table,"spectrum",&spectrum,
				command_data->spectrum_manager,set_Spectrum);
			return_code=Option_table_parse(option_table,state);
			DESTROY(Option_table)(&option_table);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_modify_glyph.  Missing command_data");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_modify_glyph */

static int gfx_modify_graphics_object(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 4 June 1999

DESCRIPTION :
Executes a GFX MODIFY GRAPHICS_OBJECT command.
==============================================================================*/
{
	int return_code = 0;

	ENTER(gfx_modify_graphics_object);
	USE_PARAMETER(dummy_to_be_modified);
	USE_PARAMETER(command_data_void);
	if (state)
	{
		display_message(ERROR_MESSAGE,
			"gfx_modify_graphics_object.  This function is no longer supported,"
			" please use gfx modify glyph instead");
	}
	LEAVE;

	return (return_code);
} /* gfx_modify_graphics_object */

/***************************************************************************//**
 * Modifies the membership of a group.  Only one of <add> or <remove> can
 * be specified at once.
 * If <use_data> flag is set, work with data, otherwise nodes.
 */
static int gfx_modify_node_group(struct Parse_state *state,
	void *use_data, void *command_data_void)
{
	int return_code = 0;
	cmzn_command_data *command_data = reinterpret_cast<cmzn_command_data *>(command_data_void);
	if (state && command_data)
	{
		cmzn_region_id region = cmzn_region_access(command_data->root_region);
		cmzn_field_group_id group = 0;
		const char *current_token = state->current_token;
		if (!set_cmzn_region_or_group(state, &region, &group))
		{
			// message already output
		}
		else if (!Parse_state_help_mode(state) && (!group))
		{
			display_message(ERROR_MESSAGE, "Not a group: %s", current_token);
			return_code = 0;
		}
		else
		{
			/* initialise defaults */
			char add_flag = 0;
			char remove_flag = 0;
			char *conditional_field_name = 0;
			char all_flag = 0;
			char selected_flag = 0;
			Multi_range *node_ranges = CREATE(Multi_range)();
			char *from_group_name = 0;
			double time = 0;
			if (command_data->default_time_keeper_app)
			{
				time = command_data->default_time_keeper_app->getTimeKeeper()->getTime();
			}

			Option_table *option_table = CREATE(Option_table)();
			/* add */
			Option_table_add_char_flag_entry(option_table, "add", &add_flag);
			/* all */
			Option_table_add_char_flag_entry(option_table, "all", &all_flag);
			/* conditional_field */
			Option_table_add_string_entry(option_table, "conditional_field", &conditional_field_name,
				" FIELD_NAME");
			/* group */
			Option_table_add_string_entry(option_table, "group", &from_group_name, " GROUP_NAME");
			/* remove */
			Option_table_add_char_flag_entry(option_table, "remove", &remove_flag);
			/* selected */
			Option_table_add_char_flag_entry(option_table, "selected", &selected_flag);
			/* default option: node number ranges */
			Option_table_add_entry(option_table, (const char *)NULL, (void *)node_ranges,
				NULL, set_Multi_range);
			return_code = Option_table_multi_parse(option_table, state);
			DESTROY(Option_table)(&option_table);

			cmzn_field_id conditional_field = 0;
			if (return_code && conditional_field_name)
			{
				cmzn_fieldmodule_id field_module = cmzn_region_get_fieldmodule(region);
				conditional_field = cmzn_fieldmodule_find_field_by_name(field_module, conditional_field_name);
				if (!conditional_field)
				{
					display_message(ERROR_MESSAGE,
						"gfx modify ngroup:  conditional field cannot be found");
					return_code = 0;
				}
				cmzn_fieldmodule_destroy(&field_module);
			}
			cmzn_field_group_id from_group = 0;
			if (return_code)
			{
				if (from_group_name)
				{
					cmzn_field *field =	FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field,name)(
						from_group_name, cmzn_region_get_Computed_field_manager(region));
					from_group = cmzn_field_cast_group(field);
					if (!from_group)
					{
						display_message(ERROR_MESSAGE, "gfx modify ngroup: '%s' is not a group.", from_group_name);
						return_code = 0;
					}
				}
				if ((add_flag + remove_flag) != 1)
				{
					display_message(ERROR_MESSAGE, "gfx modify ngroup:  Must specify operation 'add' or 'remove'.");
					return_code = 0;
				}
				if ((0 == Multi_range_get_number_of_ranges(node_ranges)) && (!selected_flag) && (0 == conditional_field) && (!all_flag) && (0 == from_group))
				{
					display_message(ERROR_MESSAGE, "gfx modify ngroup:  No nodes specified.");
					return_code = 0;
				}
			}
			if (return_code)
			{
				cmzn_fieldmodule_id field_module = cmzn_region_get_fieldmodule(region);
				cmzn_nodeset_id master_nodeset = cmzn_fieldmodule_find_nodeset_by_field_domain_type(field_module,
					use_data ? CMZN_FIELD_DOMAIN_TYPE_DATAPOINTS : CMZN_FIELD_DOMAIN_TYPE_NODES);
				cmzn_nodeset_group_id selection_nodeset_group = 0;
				if (selected_flag)
				{
					cmzn_scene *scene = cmzn_region_get_scene(region);
					cmzn_field_id selection_field = cmzn_scene_get_selection_field(scene);
					cmzn_field_group_id selection_group = cmzn_field_cast_group(selection_field);
					cmzn_field_destroy(&selection_field);
					if (selection_group)
					{
						cmzn_field_node_group_id selection_node_group =
							cmzn_field_group_get_field_node_group(selection_group, master_nodeset);
						if (selection_node_group)
						{
							selection_nodeset_group = cmzn_field_node_group_get_nodeset_group(selection_node_group);
							cmzn_field_node_group_destroy(&selection_node_group);
						}
					}
					cmzn_field_group_destroy(&selection_group);
					cmzn_scene_destroy(&scene);
				}
				cmzn_nodeset_group_id from_nodeset_group = 0;
				if (from_group)
				{
					cmzn_field_node_group_id from_node_group = cmzn_field_group_get_field_node_group(from_group, master_nodeset);
					if (from_node_group)
					{
						from_nodeset_group = cmzn_field_node_group_get_nodeset_group(from_node_group);
						cmzn_field_node_group_destroy(&from_node_group);
					}
				}
				int nodes_processed = 0;
				if (((!selected_flag) || selection_nodeset_group) && ((!from_group) || from_nodeset_group))
				{
					cmzn_fieldmodule_begin_change(field_module);
					cmzn_field_node_group_id modify_node_group = cmzn_field_group_get_field_node_group(group, master_nodeset);
					if (!modify_node_group)
					{
						modify_node_group = cmzn_field_group_create_field_node_group(group, master_nodeset);
					}
					cmzn_nodeset_group_id modify_nodeset_group = cmzn_field_node_group_get_nodeset_group(modify_node_group);
					cmzn_field_node_group_destroy(&modify_node_group);
					cmzn_fieldcache_id cache = cmzn_fieldmodule_create_fieldcache(field_module);
					cmzn_fieldcache_set_time(cache, time);

					cmzn_nodeset_id iteration_nodeset = master_nodeset;
					cmzn_nodeset_id selection_nodeset = cmzn_nodeset_group_base_cast(selection_nodeset_group);
					cmzn_nodeset_id from_nodeset = cmzn_nodeset_group_base_cast(from_nodeset_group);
					if (selected_flag && !cmzn_nodeset_match(selection_nodeset, cmzn_nodeset_group_base_cast(modify_nodeset_group)))
					{
						iteration_nodeset = selection_nodeset;
					}
					if (from_nodeset && (!cmzn_nodeset_match(from_nodeset, cmzn_nodeset_group_base_cast(modify_nodeset_group))) &&
						cmzn_nodeset_get_size(from_nodeset) < cmzn_nodeset_get_size(iteration_nodeset))
					{
						iteration_nodeset = from_nodeset;
					}

					cmzn_nodeiterator_id iter = cmzn_nodeset_create_nodeiterator(iteration_nodeset);
					const bool use_node_ranges = Multi_range_get_number_of_ranges(node_ranges) > 0;
					cmzn_node_id node = 0;
					while (NULL != (node = cmzn_nodeiterator_next_non_access(iter)))
					{
						if (use_node_ranges && !Multi_range_is_value_in_range(node_ranges, cmzn_node_get_identifier(node)))
							continue;
						if (selection_nodeset && (selection_nodeset != iteration_nodeset) && !cmzn_nodeset_contains_node(selection_nodeset, node))
							continue;
						if (from_nodeset && (from_nodeset != iteration_nodeset) && !cmzn_nodeset_contains_node(from_nodeset, node))
							continue;
						if (conditional_field)
						{
							cmzn_fieldcache_set_node(cache, node);
							if (!cmzn_field_evaluate_boolean(conditional_field, cache))
								continue;
						}
						++nodes_processed;
						if (add_flag)
						{
							int result = cmzn_nodeset_group_add_node(modify_nodeset_group, node);
							if ((CMZN_OK != result) && (CMZN_ERROR_ALREADY_EXISTS != result))
							{
								display_message(ERROR_MESSAGE, "gfx modify ngroup:  Adding nodes failed");
								return_code = 0;
								break;
							}
						}
						else
						{
							int result = cmzn_nodeset_group_remove_node(modify_nodeset_group, node);
							if ((CMZN_OK != result) && (CMZN_ERROR_NOT_FOUND != result))
							{
								display_message(ERROR_MESSAGE, "gfx modify ngroup:  Removing nodes failed");
								return_code = 0;
								break;
							}
						}
					}
					cmzn_nodeiterator_destroy(&iter);
					cmzn_fieldcache_destroy(&cache);
					cmzn_nodeset_group_destroy(&modify_nodeset_group);
					cmzn_field_node_group_destroy(&modify_node_group);
					cmzn_fieldmodule_end_change(field_module);
				}
				if (0 == nodes_processed)
					display_message(WARNING_MESSAGE, "gfx modify ngroup:  No nodes processed.");
				cmzn_nodeset_group_destroy(&from_nodeset_group);
				cmzn_nodeset_group_destroy(&selection_nodeset_group);
				cmzn_nodeset_destroy(&master_nodeset);
				cmzn_fieldmodule_destroy(&field_module);
			}
			if (from_group_name)
			{
				DEALLOCATE(from_group_name);
			}
			cmzn_field_group_destroy(&from_group);
			DESTROY(Multi_range)(&node_ranges);
			cmzn_field_destroy(&conditional_field);
			if (conditional_field_name)
				DEALLOCATE(conditional_field_name);
		}
		cmzn_field_group_destroy(&group);
		cmzn_region_destroy(&region);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_modify_node_group.  Invalid argument(s)");
		return_code = 0;
	}
	return (return_code);
}

struct cmzn_nodal_derivatives_data
{
	int number_of_derivatives; // initialise to -1
	enum cmzn_node_value_label *derivatives; // initialise to NULL
};

/***************************************************************************//**
 * Modifier function for entering the derivatives types to be defined in a new
 * node field.
 * Idea: could make it re-enter to be different for different components.
 */
static int set_cmzn_nodal_derivatives(struct Parse_state *state,
	void *derivatives_data_void, void *dummy_void)
{
	int return_code = 1;
	USE_PARAMETER(dummy_void);
	cmzn_nodal_derivatives_data *derivatives_data = reinterpret_cast<cmzn_nodal_derivatives_data *>(derivatives_data_void);
	if (state && derivatives_data && (0 > derivatives_data->number_of_derivatives))
	{
		if (0 == state->current_token)
		{
			display_message(ERROR_MESSAGE, "Missing derivatives");
			display_parse_state_location(state);
			return_code = 0;
		}
		else if (Parse_state_help_mode(state))
		{
			display_message(INFORMATION_MESSAGE, " DERIVATIVE_NAMES(D_DS1 D2_DS1DS2 etc.)|none[none]");
		}
		else if (fuzzy_string_compare("none", state->current_token))
		{
			return_code = shift_Parse_state(state, 1);
			derivatives_data->number_of_derivatives = 0;
		}
		else
		{
			derivatives_data->number_of_derivatives = 0;
			while (state->current_token)
			{
				// stop when derivatives not recognised
				enum cmzn_node_value_label node_value_label =
					cmzn_node_value_label_enum_from_string(state->current_token);
				if (node_value_label != CMZN_NODE_VALUE_LABEL_INVALID)
				{
					enum cmzn_node_value_label *temp;
					if (REALLOCATE(temp, derivatives_data->derivatives, enum cmzn_node_value_label,
						derivatives_data->number_of_derivatives + 1))
					{
						derivatives_data->derivatives = temp;
						derivatives_data->derivatives[derivatives_data->number_of_derivatives] = node_value_label;
						++derivatives_data->number_of_derivatives;
						return_code = shift_Parse_state(state, 1);
					}
					else
					{
						return_code = 0;
						break;
					}
				}
				else
				{
					if (0 == derivatives_data->number_of_derivatives)
					{
						display_message(ERROR_MESSAGE, "Unrecognised derivative %s", state->current_token);
						display_parse_state_location(state);
						return_code = 0;
					}
					break;
				}
			}
		}
	}
	else
	{
		if (derivatives_data && (0 <= derivatives_data->number_of_derivatives))
		{
			display_message(ERROR_MESSAGE, "Derivatives have already been set");
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"set_cmzn_nodal_derivatives.  Invalid argument(s)");
		}
		return_code = 0;
	}
	return (return_code);
}

/***************************************************************************//**
 * Executes a GFX MODIFY NODES/DATA command.
 * If <use_data_flag> is set, use datapoints, otherwise nodes.
 */
static int gfx_modify_nodes(struct Parse_state *state,
	void *use_data, void *command_data_void)
{
	int return_code;
	cmzn_command_data *command_data = reinterpret_cast<cmzn_command_data *>(command_data_void);
	if (state && command_data)
	{
		/* initialise defaults */
		cmzn_region_id region = cmzn_region_access(command_data->root_region);
		cmzn_field_group_id group = 0;
		char all_flag = 0;
		char selected_flag = 0;
		Multi_range *node_ranges = CREATE(Multi_range)();
		char *define_field_name = 0;
		char *undefine_field_name = 0;
		char *conditional_field_name = 0;
		cmzn_nodal_derivatives_data derivatives_data;
		derivatives_data.number_of_derivatives = -1; // not set, = 0
		derivatives_data.derivatives = 0;
		int number_of_versions = 1;
		FE_value time = 0.0;
		if (command_data->default_time_keeper_app)
		{
			time = command_data->default_time_keeper_app->getTimeKeeper()->getTime();
		}

		Option_table *option_table = CREATE(Option_table)();
		/* all */
		Option_table_add_entry(option_table, "all", &all_flag, NULL, set_char_flag);
		/* conditional_field */
		Option_table_add_string_entry(option_table,"conditional_field", &conditional_field_name,
			" FIELD_NAME");
		/* define */
		Option_table_add_string_entry(option_table, "define", &define_field_name,
			" FIELD_NAME");
		/* derivatives */
		Option_table_add_entry(option_table, "derivatives",
			(void *)&derivatives_data, (void *)0, set_cmzn_nodal_derivatives);
		/* group */
		Option_table_add_region_or_group_entry(option_table, "group", &region, &group);
		/* selected */
		Option_table_add_entry(option_table, "selected", &selected_flag,
			NULL, set_char_flag);
		/* undefine */
		Option_table_add_string_entry(option_table, "undefine", &undefine_field_name,
			" FIELD_NAME");
		/* versions */
		Option_table_add_int_positive_entry(option_table, "versions", &number_of_versions);
		/* default option: node number ranges */
		Option_table_add_entry(option_table, (const char *)NULL, (void *)node_ranges,
			NULL, set_Multi_range);
		return_code = Option_table_multi_parse(option_table, state);
		DESTROY(Option_table)(&option_table);

		if (return_code)
		{
			if ((0 == Multi_range_get_number_of_ranges(node_ranges)) && (!selected_flag) &&
				(0 == conditional_field_name) && (!all_flag) && (!group))
			{
				display_message(ERROR_MESSAGE, "gfx modify nodes:  No nodes specified.");
				return_code = 0;
			}
		}
		cmzn_field_id conditional_field = 0;
		if (return_code && conditional_field_name)
		{
			cmzn_fieldmodule_id field_module = cmzn_region_get_fieldmodule(region);
			conditional_field = cmzn_fieldmodule_find_field_by_name(field_module, conditional_field_name);
			if (!conditional_field)
			{
				display_message(ERROR_MESSAGE,
					"gfx modify nodes:  conditional field cannot be found");
				return_code = 0;
			}
			cmzn_fieldmodule_destroy(&field_module);
		}
		if (return_code)
		{
			if (define_field_name && undefine_field_name)
			{
				display_message(WARNING_MESSAGE,
					"gfx modify nodes:  Only specify one of define or undefine field");
				return_code = 0;
			}
			if ((!define_field_name) && (!undefine_field_name))
			{
				display_message(WARNING_MESSAGE,
					"gfx modify nodes:  Must specify define or undefine field");
				return_code = 0;
			}
		}
		if (return_code)
		{
			cmzn_fieldmodule_id field_module = cmzn_region_get_fieldmodule(region);
			cmzn_field_id selection_field = NULL;
			if (selected_flag)
			{
				cmzn_scene *scene = cmzn_region_get_scene(region);
				selection_field = cmzn_scene_get_selection_field(scene);
				cmzn_scene_destroy(&scene);
			}
			const char *field_name = define_field_name ? define_field_name : undefine_field_name;
			cmzn_field_id field = cmzn_fieldmodule_find_field_by_name(field_module, field_name);
			if (!field)
			{
				display_message(ERROR_MESSAGE, "gfx modify nodes:  Cannot find field %s", field_name);
				return_code = 0;
			}
			cmzn_nodeset_id nodeset = cmzn_fieldmodule_find_nodeset_by_field_domain_type(field_module,
				use_data ? CMZN_FIELD_DOMAIN_TYPE_DATAPOINTS : CMZN_FIELD_DOMAIN_TYPE_NODES);
			cmzn_nodetemplate_id node_template = cmzn_nodeset_create_nodetemplate(nodeset);
			if (define_field_name)
			{
				if (CMZN_OK != cmzn_nodetemplate_define_field(node_template, field))
				{
					return_code = 0;
				}
				else
				{
					for (int i = 0; i < derivatives_data.number_of_derivatives; ++i)
					{
						if (CMZN_OK != cmzn_nodetemplate_set_value_number_of_versions(node_template, field,
							/*component_number=all*/-1, derivatives_data.derivatives[i], number_of_versions))
						{
							return_code = 0;
							break;
						}
					}
				}
			}
			else
			{
				if (CMZN_OK != cmzn_nodetemplate_undefine_field(node_template, field))
					return_code = 0;
			}
			if (group)
			{
				cmzn_field_node_group_id node_group = cmzn_field_group_get_field_node_group(group, nodeset);
				cmzn_nodeset_destroy(&nodeset);
				nodeset = cmzn_nodeset_group_base_cast(cmzn_field_node_group_get_nodeset_group(node_group));
				cmzn_field_node_group_destroy(&node_group);
			}
			int nodes_processed = 0;
			if (return_code && nodeset && ((!selected_flag) || selection_field))
			{
				cmzn_fieldmodule_begin_change(field_module);
				cmzn_fieldcache_id cache = cmzn_fieldmodule_create_fieldcache(field_module);
				cmzn_fieldcache_set_time(cache, time);
				cmzn_nodeiterator_id iter = cmzn_nodeset_create_nodeiterator(nodeset);
				const bool use_node_ranges = Multi_range_get_number_of_ranges(node_ranges) > 0;
				cmzn_node_id node = 0;
				while (NULL != (node = cmzn_nodeiterator_next_non_access(iter)))
				{
					if (use_node_ranges && !Multi_range_is_value_in_range(node_ranges, cmzn_node_get_identifier(node)))
						continue;
					if (conditional_field || selection_field)
					{
						cmzn_fieldcache_set_node(cache, node);
					}
					if (conditional_field && !cmzn_field_evaluate_boolean(conditional_field, cache))
						continue;
					if (selection_field && !cmzn_field_evaluate_boolean(selection_field, cache))
						continue;
					if (!cmzn_node_merge(node, node_template))
					{
						display_message(WARNING_MESSAGE, "gfx modify nodes:  Failed to merge node");
						return_code = 0;
						break;
					}
					++nodes_processed;
				}
				cmzn_nodeiterator_destroy(&iter);
				cmzn_fieldcache_destroy(&cache);
				cmzn_fieldmodule_end_change(field_module);
			}
			if (return_code && (0 == nodes_processed))
			{
				if (use_data)
				{
					display_message(WARNING_MESSAGE, "gfx modify data:  No data specified");
				}
				else
				{
					display_message(WARNING_MESSAGE, "gfx modify nodes:  No nodes specified");
				}
			}
			if (node_template)
				cmzn_nodetemplate_destroy(&node_template);
			cmzn_nodeset_destroy(&nodeset);
			cmzn_field_destroy(&field);
			cmzn_field_destroy(&selection_field);
			cmzn_fieldmodule_destroy(&field_module);
		}
		DESTROY(Multi_range)(&node_ranges);
		if (define_field_name)
		{
			DEALLOCATE(define_field_name);
		}
		if (undefine_field_name)
		{
			DEALLOCATE(undefine_field_name);
		}
		if (conditional_field_name)
		{
			DEALLOCATE(conditional_field_name);
		}
		if (derivatives_data.derivatives)
		{
			DEALLOCATE(derivatives_data.derivatives);
		}
		if (conditional_field)
		{
			cmzn_field_destroy(&conditional_field);
		}
		cmzn_field_group_destroy(&group);
		cmzn_region_destroy(&region);
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_modify_nodes.  Invalid argument(s)");
		return_code = 0;
	}
	return (return_code);
}

static int execute_command_gfx_modify(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 26 March 2001

DESCRIPTION :
Executes a GFX MODIFY command.
???DB.  Part of GFX EDIT ?
==============================================================================*/
{
	int return_code;
	struct cmzn_command_data *command_data;
	struct Modify_environment_map_data modify_environment_map_data;
#if defined (USE_CMGUI_GRAPHICS_WINDOW)
	struct Modify_graphics_window_data modify_graphics_window_data;
#endif /* defined (USE_CMGUI_GRAPHICS_WINDOW) */
	struct Option_table *option_table;
	struct Modify_light_data modify_light_data;

	ENTER(execute_command_gfx_modify);
	USE_PARAMETER(dummy_to_be_modified);
	/* check argument */
	if (state)
	{
		if (NULL != (command_data = (struct cmzn_command_data *)command_data_void))
		{
			if (state->current_token)
			{
				option_table=CREATE(Option_table)();
				/* data */
				Option_table_add_entry(option_table, "data", /*use_data*/(void *)1,
					(void *)command_data, gfx_modify_nodes);
				/* dgroup */
				Option_table_add_entry(option_table,"dgroup",(void *)1/*data*/,
					(void *)command_data, gfx_modify_node_group);
				/* egroup */
				Option_table_add_entry(option_table,"egroup",NULL,
					(void *)command_data, gfx_modify_element_group);
#if defined (EMOTER_ENABLE)
				/* emoter */
				Option_table_add_entry(option_table,"emoter",NULL,
					(void *)command_data->emoter_slider_dialog,
					gfx_modify_emoter);
#endif
				/* environment_map */
				modify_environment_map_data.graphical_material_manager=
					cmzn_materialmodule_get_manager(command_data->materialmodule);
				modify_environment_map_data.environment_map_manager=
					command_data->environment_map_manager;
				Option_table_add_entry(option_table,"environment_map",NULL,
					(&modify_environment_map_data),modify_Environment_map);
				/* field */
				Option_table_add_entry(option_table,"field",NULL,
					(void *)command_data, gfx_modify_field);
				/* flow_particles */
				Option_table_add_entry(option_table,"flow_particles",NULL,
					(void *)command_data, gfx_modify_flow_particles);
				/* g_element */
				Option_table_add_entry(option_table,"g_element",NULL,
					(void *)command_data, gfx_modify_g_element);
				/* glyph */
				Option_table_add_entry(option_table,"glyph",NULL,
					(void *)command_data, gfx_modify_glyph);
				/* graphics_object */
				Option_table_add_entry(option_table,"graphics_object",NULL,
					(void *)command_data, gfx_modify_graphics_object);
				/* light */
				modify_light_data.default_light=command_data->default_light;
				modify_light_data.lightmodule=command_data->lightmodule;
				Option_table_add_entry(option_table,"light",NULL,
					(void *)(&modify_light_data), modify_cmzn_light);
				/* lmodel */
				Option_table_add_entry(option_table,"lmodel",NULL,
					NULL, gfx_create_modify_light_model);
				/* material */
				struct Material_module_app materialmodule;
				materialmodule.module = (void *)command_data->materialmodule;
				materialmodule.region = (void *)command_data->root_region;
				Option_table_add_entry(option_table,"material",NULL,
					(void *)(&materialmodule), modify_Graphical_material);
				/* ngroup */
				Option_table_add_entry(option_table,"ngroup",NULL,
					(void *)command_data, gfx_modify_node_group);
				/* nodes */
				Option_table_add_entry(option_table, "nodes", /*use_data*/(void *)0,
					(void *)command_data, gfx_modify_nodes);
				/* scene */
				Define_scene_data define_scene_data;
				define_scene_data.root_region = command_data->root_region;
				define_scene_data.graphics_module = command_data->graphics_module;
				Option_table_add_entry(option_table, "scene", NULL,
					(void *)(&define_scene_data), define_Scene);
				/* spectrum */
				Option_table_add_entry(option_table,"spectrum",NULL,
					(void *)command_data, gfx_modify_Spectrum);
				/* texture */
				Option_table_add_entry(option_table,"texture",NULL,
					(void *)command_data, gfx_modify_Texture);
#if defined (USE_CMGUI_GRAPHICS_WINDOW)
				/* window */
				modify_graphics_window_data.computed_field_package=
					command_data->computed_field_package;
				modify_graphics_window_data.graphics_window_manager=
					command_data->graphics_window_manager;
				modify_graphics_window_data.interactive_tool_manager=
					command_data->interactive_tool_manager;
				modify_graphics_window_data.light_manager=
					cmzn_lightmodule_get_manager(command_data->lightmodule);
				modify_graphics_window_data.root_region=command_data->root_region;
				modify_graphics_window_data.filter_module = command_data->filter_module;
				Option_table_add_entry(option_table,"window",NULL,
					(void *)(&modify_graphics_window_data), modify_Graphics_window);
#endif /* defined (USE_CMGUI_GRAPHICS_WINDOW) */

				return_code=Option_table_parse(option_table,state);
				DESTROY(Option_table)(&option_table);
			}
			else
			{
				set_command_prompt("gfx modify",command_data);
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"execute_command_gfx_modify.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_command_gfx_modify.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_gfx_modify */

#if defined (GTK_USER_INTERFACE) || defined (WIN32_USER_INTERFACE) || defined (CARBON_USER_INTERFACE) || defined (WX_USER_INTERFACE)
static int execute_command_gfx_node_tool(struct Parse_state *state,
	void *data_tool_flag, void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 19 February 2008

DESCRIPTION :
Executes a GFX NODE_TOOL or GFX_DATA_TOOL command. If <data_tool_flag> is set,
then the <data_tool> is being modified, otherwise the <node_tool>.
Which tool that is being modified is passed in <node_tool_void>.
==============================================================================*/
{
	struct cmzn_command_data *command_data;
	struct Node_tool *node_tool;
	int return_code = 0;
	ENTER(execute_command_gfx_node_tool);
	if (state && (command_data=(struct cmzn_command_data *)command_data_void))
	{
		if (data_tool_flag)
		{
			node_tool = command_data->data_tool;
		}
		else
		{
			node_tool = command_data->node_tool;
		}
		return_code = Node_tool_execute_command_with_parse_state(node_tool, state);
		if (node_tool)
		{
#if defined (WX_USER_INTERFACE)
			FOR_EACH_OBJECT_IN_MANAGER(Graphics_window)(
				Graphics_window_update_Interactive_tool,
				(void *)Node_tool_get_interactive_tool(node_tool),
				command_data->graphics_window_manager);
			display_message(WARNING_MESSAGE,
				"This command changes the node tool settings for each window to the global settings. To change node tool settings for individual window, please see the command [gfx modify window <name> nodes ?]. \n");
#endif /*(WX_USER_INTERFACE)*/
			cmzn_sceneviewermodule_update_Interactive_tool(
				command_data->sceneviewermodule,
				Node_tool_get_interactive_tool(node_tool));
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"execute_command_gfx_node_tool.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_gfx_node_tool */
#endif /* defined (GTK_USER_INTERFACE) || defined
			  (WIN32_USER_INTERFACE) || defined (CARBON_USER_INTERFACE) || defined(WX_USER_INTERFACE */

#if defined (GTK_USER_INTERFACE) || defined (WIN32_USER_INTERFACE) || defined (WX_USER_INTERFACE)
static int execute_command_gfx_print(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 23 April 2002

DESCRIPTION :
Executes a GFX PRINT command.
==============================================================================*/
{
	char *file_name, force_onscreen_flag;
	const char*image_file_format_string, **valid_strings;
	enum Image_file_format image_file_format;
	enum Texture_storage_type storage;
	int antialias, height, number_of_valid_strings, return_code,
		transparency_layers, width;
	struct Cmgui_image *cmgui_image;
	struct Cmgui_image_information *cmgui_image_information;
	struct cmzn_command_data *command_data;
	struct Graphics_window *window;
	struct Option_table *option_table;

	ENTER(execute_command_gfx_print);
	USE_PARAMETER(dummy_to_be_modified);
	if (state && (command_data=(struct cmzn_command_data *)command_data_void))
	{
		/* initialize defaults */
		antialias = -1;
		file_name = (char *)NULL;
		height = 0;
		force_onscreen_flag = 0;
		storage = TEXTURE_RGBA;
		transparency_layers = 0;
		width = 0;
		/* default file format is to obtain it from the filename extension */
		image_file_format = UNKNOWN_IMAGE_FILE_FORMAT;
		/* must have at least one graphics_window to print */
		if (NULL != (window = FIRST_OBJECT_IN_MANAGER_THAT(
			Graphics_window)((MANAGER_CONDITIONAL_FUNCTION(Graphics_window) *)NULL,
				(void *)NULL, command_data->graphics_window_manager)))
		{
			ACCESS(Graphics_window)(window);
		}

		option_table = CREATE(Option_table)();
		/* antialias */
		Option_table_add_entry(option_table, "antialias",
			&antialias, NULL, set_int_positive);
		/* image file format */
		image_file_format_string =
			ENUMERATOR_STRING(Image_file_format)(image_file_format);
		valid_strings = ENUMERATOR_GET_VALID_STRINGS(Image_file_format)(
			&number_of_valid_strings,
			(ENUMERATOR_CONDITIONAL_FUNCTION(Image_file_format) *)NULL,
			(void *)NULL);
		Option_table_add_enumerator(option_table, number_of_valid_strings,
			valid_strings, &image_file_format_string);
		DEALLOCATE(valid_strings);
		/* file */
		Option_table_add_entry(option_table, "file", &file_name,
			(void *)1, set_name);
		/* force_onscreen */
		Option_table_add_entry(option_table, "force_onscreen",
			&force_onscreen_flag, NULL, set_char_flag);
		/* format */
		Option_table_add_entry(option_table, "format", &storage,
			NULL, set_Texture_storage);
		/* height */
		Option_table_add_entry(option_table, "height",
			&height, NULL, set_int_non_negative);
		/* transparency_layers */
		Option_table_add_entry(option_table, "transparency_layers",
			&transparency_layers, NULL, set_int_positive);
		/* width */
		Option_table_add_entry(option_table, "width",
			&width, NULL, set_int_non_negative);
		/* window */
		Option_table_add_entry(option_table, "window",
			&window, command_data->graphics_window_manager, set_Graphics_window);

		return_code = Option_table_multi_parse(option_table, state);
		DESTROY(Option_table)(&option_table);
		/* no errors, not asking for help */
		if (return_code)
		{
			if (!file_name)
			{
				if (!(file_name = confirmation_get_write_filename(NULL,
								 command_data->user_interface
#if defined (WX_USER_INTERFACE)
							 , command_data->execute_command
#endif /* defined (WX_USER_INTERFACE)*/
)))
				{
					display_message(ERROR_MESSAGE, "gfx print:  No file name specified");
					return_code = 0;
				}
			}
			if (!window)
			{
				display_message(ERROR_MESSAGE,
					"gfx print:  No graphics windows to print");
				return_code = 0;
			}
		}
		if (return_code)
		{
			cmgui_image_information = CREATE(Cmgui_image_information)();
			if (image_file_format_string)
			{
				STRING_TO_ENUMERATOR(Image_file_format)(
					image_file_format_string, &image_file_format);
			}
			else
			{
				image_file_format = UNKNOWN_IMAGE_FILE_FORMAT;
			}
			Cmgui_image_information_set_image_file_format(
				cmgui_image_information, image_file_format);
			Cmgui_image_information_add_file_name(cmgui_image_information,
				file_name);
			Cmgui_image_information_set_io_stream_package(cmgui_image_information,
				command_data->io_stream_package);
			if (NULL != (cmgui_image = Graphics_window_get_image(window,
				force_onscreen_flag, width, height, antialias,
				transparency_layers, storage)))
			{
				if (!Cmgui_image_write(cmgui_image, cmgui_image_information))
				{
					display_message(ERROR_MESSAGE,
						"gfx print:  Error writing image %s", file_name);
					return_code = 0;
				}
				DESTROY(Cmgui_image)(&cmgui_image);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"execute_command_gfx_print.  Could not get image from window");
				return_code = 0;
			}
			DESTROY(Cmgui_image_information)(&cmgui_image_information);
		}
		if (window)
		{
			DEACCESS(Graphics_window)(&window);
		}
		if (file_name)
		{
			DEALLOCATE(file_name);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"execute_command_gfx_print.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_gfx_print */
#endif

/** Explains migration for removed command 'gfx read curve' */
int gfx_read_Curve(struct Parse_state *state,
	void *dummy_to_be_modified, void *command_data_void)
{
	USE_PARAMETER(dummy_to_be_modified);
	USE_PARAMETER(command_data_void);
	struct Option_table *option_table = CREATE(Option_table)();
	display_message(Parse_state_help_mode(state) ? INFORMATION_MESSAGE : ERROR_MESSAGE,
		"The 'gfx read curve' command has been removed. You can replace a curve with a\n"
		"1-D mesh defining a non-decreasing scalar 'parameter' field, plus a 'value' field\n"
		"to look up for a given parameter value. 'gfx read curve' merely executed a command file\n"
		"with the extension .curve.com, which contained a single 'gfx define curve' command to load\n"
		"the curve from EX files. You can now directly load the exnode/exelem or single exregion\n"
		"file(s) which are defined in the right way. Use a 'find_mesh_location' field and an\n"
		"embedded field to achieve the effect of the former 'curve_lookup' field type. These\n"
		"currently only work in the same region; talk to the Cmgui developers if you have\n"
		"migration issues.\n");
	if (Parse_state_help_mode(state))
		return 1;
	return 0;
}

int offset_region_identifier(cmzn_region_id region, char element_flag, int element_offset,
	char face_flag, int face_offset, char line_flag, int line_offset, char node_flag, int node_offset, int use_data)
{
	int return_code = 1;
	struct FE_region *fe_region = NULL;
	if (region)
		fe_region = cmzn_region_get_FE_region(region);
	/* Offset these nodes and elements before merging */
	if (fe_region)
	{
		cmzn_fieldmodule_id fieldmodule = cmzn_region_get_fieldmodule(region);
		cmzn_fieldmodule_begin_change(fieldmodule);
		int highest_dimension = FE_region_get_highest_dimension(fe_region);
		if (element_flag)
		{
			if (CMZN_OK != FE_region_change_element_identifiers(fe_region,
				highest_dimension, element_offset,
				(struct Computed_field *)NULL, /*time*/0, /*element_group*/ 0))
			{
				return_code = 0;
			}
		}
		if (face_flag && (highest_dimension > 2))
		{
			if (CMZN_OK != FE_region_change_element_identifiers(fe_region,
				/*dimension*/2, face_offset,
				(struct Computed_field *)NULL, /*time*/0, /*element_group*/ 0))
			{
				return_code = 0;
			}
		}
		if (line_flag && (highest_dimension > 1))
		{
			if (CMZN_OK != FE_region_change_element_identifiers(fe_region,
				/*dimension*/1, line_offset,
				(struct Computed_field *)NULL, /*time*/0, /*element_group*/ 0))
			{
				return_code = 0;
			}
		}
		if (node_flag)
		{
			cmzn_nodeset_id nodeset = cmzn_fieldmodule_find_nodeset_by_field_domain_type(fieldmodule,
				use_data ? CMZN_FIELD_DOMAIN_TYPE_DATAPOINTS : CMZN_FIELD_DOMAIN_TYPE_NODES);
			if (!cmzn_nodeset_change_node_identifiers(nodeset,
				node_offset, (struct Computed_field *)NULL, /*time*/0))
			{
				return_code = 0;
			}
		}
		struct cmzn_region *child_region = cmzn_region_get_first_child(region);
		while ((NULL != child_region))
		{
			return_code = offset_region_identifier(child_region, element_flag, element_offset, face_flag,
				face_offset, line_flag, line_offset, node_flag, node_offset, use_data);
			cmzn_region_reaccess_next_sibling(&child_region);
		}
		cmzn_fieldmodule_end_change(fieldmodule);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Unable to get fe_region to offset nodes or elements in file .");
		return_code = 0;
	}
	return return_code;
}

static int gfx_read_elements(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 6 March 2003

DESCRIPTION :
If an element file is not specified a file selection box is presented to the
user, otherwise the elements file is read.
==============================================================================*/
{
	char *file_name, *region_path,
		element_flag, face_flag, line_flag, node_flag;
	int element_offset, face_offset, line_offset, node_offset,
		return_code;
	struct cmzn_command_data *command_data;
	struct cmzn_region *region, *top_region;
	struct IO_stream *input_file;
	struct Option_table *option_table;

	ENTER(gfx_read_elements);
	USE_PARAMETER(dummy_to_be_modified);
	input_file = NULL;
	if (state && (command_data = (struct cmzn_command_data *)command_data_void))
	{
		element_flag = 0;
		element_offset = 0;
		face_flag = 0;
		face_offset = 0;
		line_flag = 0;
		line_offset = 0;
		node_flag = 0;
		node_offset = 0;
		file_name = (char *)NULL;
		region_path = (char *)NULL;
		option_table = CREATE(Option_table)();
		/* element_offset */
		Option_table_add_entry(option_table, "element_offset", &element_offset,
			&element_flag, set_int_and_char_flag);
		/* example */
		Option_table_add_entry(option_table,CMGUI_EXAMPLE_DIRECTORY_SYMBOL,
			&file_name, &(command_data->example_directory), set_file_name);
		/* face_offset */
		Option_table_add_entry(option_table, "face_offset", &face_offset,
			&face_flag, set_int_and_char_flag);
		/* line_offset */
		Option_table_add_entry(option_table, "line_offset", &line_offset,
			&line_flag, set_int_and_char_flag);
		/* node_offset */
		Option_table_add_entry(option_table, "node_offset", &node_offset,
			&node_flag, set_int_and_char_flag);
		/* region */
		Option_table_add_entry(option_table,"region",
			&region_path, (void *)1, set_name);
		/* default */
		Option_table_add_entry(option_table,NULL,&file_name,
			NULL,set_file_name);
		return_code = Option_table_multi_parse(option_table, state);
		if (return_code)
		{
			if (!file_name)
			{
				if (!(file_name = confirmation_get_read_filename(".exelem",
								 command_data->user_interface
#if defined(WX_USER_INTERFACE)
								 , command_data->execute_command
#endif /*defined (WX_USER_INTERFACE) */
		 )))
				{
					return_code = 0;
				}
			}
#if defined (WX_USER_INTERFACE) && defined (WIN32_SYSTEM)
			if (file_name)
			{
				 CMZN_set_directory_and_filename_WIN32(&file_name, command_data);
			}
#endif /* defined (WIN32_SYSTEM)*/

			if (return_code)
			{
				if (!check_suffix(&file_name,".exelem"))
				{
					return_code = 0;
				}
			}
			top_region = (struct cmzn_region *)NULL;
			if (region_path)
			{
				top_region = cmzn_region_find_subregion_at_path(
					command_data->root_region, region_path);
				if (NULL == top_region)
				{
					top_region = cmzn_region_create_subregion(
						command_data->root_region, region_path);
					if (NULL == top_region)
					{
						display_message(ERROR_MESSAGE, "gfx_read_elements.  "
							"Unable to find or create region '%s'.", region_path);
						return_code = 0;
					}
				}
			}
			else
			{
				top_region = ACCESS(cmzn_region)(command_data->root_region);
			}
			if (return_code)
			{
				/* open the file */
				if ((input_file = CREATE(IO_stream)(command_data->io_stream_package))
					&& (IO_stream_open_for_read(input_file, file_name)))
				{
					region = cmzn_region_create_region(top_region);
					if (read_exregion_file(region, input_file,
						(struct FE_import_time_index *)NULL))
					{
						if (element_flag || face_flag || line_flag || node_flag)
						{
							return_code = offset_region_identifier(region, element_flag, element_offset, face_flag,
								face_offset, line_flag, line_offset, node_flag, node_offset, /*use_data*/0);
						}
						if (return_code)
						{
							if (cmzn_region_can_merge(top_region, region))
							{
								if (!cmzn_region_merge(top_region, region))
								{
									display_message(ERROR_MESSAGE,
										"Error merging elements from file: %s", file_name);
									return_code = 0;
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"Contents of file %s not compatible with global objects",
									file_name);
								return_code = 0;
							}
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Error reading element file: %s", file_name);
						return_code = 0;
					}
					DEACCESS(cmzn_region)(&region);
					IO_stream_close(input_file);
					DESTROY(IO_stream)(&input_file);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Could not open element file: %s", file_name);
					return_code = 0;
				}
			}
			DEACCESS(cmzn_region)(&top_region);
		}
		DESTROY(Option_table)(&option_table);
#if defined (WX_USER_INTERFACE)
		if (input_file)
			 DESTROY(IO_stream)(&input_file);
#endif /*defined (WX_USER_INTERFACE)*/
		if (file_name)
		{
			DEALLOCATE(file_name);
		}
		if (region_path)
		{
			DEALLOCATE(region_path);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE, "gfx_read_elements.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* gfx_read_elements */

static int gfx_read_nodes(struct Parse_state *state,
	void *use_data, void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 6 March 2003

DESCRIPTION :
If a nodes file is not specified a file selection box is presented to the user,
otherwise the nodes file is read.
If the <use_data> flag is set, then read data, otherwise nodes.
==============================================================================*/
{
	char *file_name, node_offset_flag, *region_path, time_set_flag;
	double maximum, minimum;
	float time;
	int node_offset, return_code;
	struct cmzn_command_data *command_data;
	struct cmzn_region *region, *top_region;
	struct FE_import_time_index *node_time_index, node_time_index_data;
	struct IO_stream *input_file;
	struct Option_table *option_table;

	ENTER(gfx_read_nodes);
	input_file=NULL;
	if (state)
	{
		if (NULL != (command_data = (struct cmzn_command_data *)command_data_void))
		{
			file_name = (char *)NULL;
			node_offset_flag = 0;
			node_offset = 0;
			region_path = (char *)NULL;
			time = 0;
			time_set_flag = 0;
			node_time_index = (struct FE_import_time_index *)NULL;
			option_table=CREATE(Option_table)();
			/* example */
			Option_table_add_entry(option_table,CMGUI_EXAMPLE_DIRECTORY_SYMBOL,
				&file_name, &(command_data->example_directory), set_file_name);
			if (!use_data)
			{
				/* node_offset */
				Option_table_add_entry(option_table, "node_offset", &node_offset,
					&node_offset_flag, set_int_and_char_flag);
			}
			else
			{
				/* data_offset */
				Option_table_add_entry(option_table, "data_offset", &node_offset,
					&node_offset_flag, set_int_and_char_flag);
			}
			/* region */
			Option_table_add_entry(option_table,"region", &region_path, (void *)1, set_name);
			/* time */
			Option_table_add_entry(option_table,"time",
				&time, &time_set_flag, set_float_and_char_flag);
			/* default */
			Option_table_add_entry(option_table, NULL, &file_name,
				NULL, set_file_name);
			return_code = Option_table_multi_parse(option_table,state);
			if (return_code)
			{
				if (!file_name)
				{
					if (use_data)
					{
						if (!(file_name = confirmation_get_read_filename(".exdata",
										 command_data->user_interface
#if defined(WX_USER_INTERFACE)
								 , command_data->execute_command
#endif /*defined (WX_USER_INTERFACE) */
																														 )))
						{
							return_code = 0;
						}
					}
					else
					{
						if (!(file_name = confirmation_get_read_filename(".exnode",
							command_data->user_interface
#if defined(WX_USER_INTERFACE)
								 , command_data->execute_command
#endif /*defined (WX_USER_INTERFACE) */
																														 )))
						{
							return_code = 0;
						}
					}
				}
				if (time_set_flag)
				{
					node_time_index_data.time = time;
					node_time_index = &node_time_index_data;
				}
				if (return_code)
				{
#if defined (WX_USER_INTERFACE) && defined (WIN32_SYSTEM)
			if (file_name)
			{
				 CMZN_set_directory_and_filename_WIN32(&file_name, command_data);
			}
#endif /* defined (WIN32_SYSTEM)*/
					/* open the file */
					if (use_data)
					{
						return_code = check_suffix(&file_name,".exdata");
					}
					else
					{
						return_code = check_suffix(&file_name,".exnode");
					}
					top_region = (struct cmzn_region *)NULL;
					if (region_path)
					{
						top_region = cmzn_region_find_subregion_at_path(
							command_data->root_region, region_path);
						if (NULL == top_region)
						{
							top_region = cmzn_region_create_subregion(
								command_data->root_region, region_path);
							if (NULL == top_region)
							{
								display_message(ERROR_MESSAGE, "gfx read nodes.  "
									"Unable to find or create region '%s'.", region_path);
								return_code = 0;
							}
						}
					}
					else
					{
						top_region = ACCESS(cmzn_region)(command_data->root_region);
					}
					if (return_code)
					{
						if ((input_file = CREATE(IO_stream)(command_data->io_stream_package))
							&& (IO_stream_open_for_read(input_file, file_name)))
						{
							region = cmzn_region_create_region(top_region);
							if (use_data)
							{
								return_code = read_exdata_file(region, input_file, node_time_index);
							}
							else
							{
								return_code = read_exregion_file(region, input_file, node_time_index);
							}
							if (return_code)
							{
								if (node_offset_flag)
								{
									/* Offset these nodes before merging */
									if (use_data)
									{
										return_code = offset_region_identifier(region, 0, 0, 0,
											0, 0, 0, node_offset_flag, node_offset, /*use_data*/1);
									}
									else
									{
										return_code = offset_region_identifier(region, 0, 0, 0,
											0, 0, 0, node_offset_flag, node_offset, /*use_data*/0);
									}
								}
								if (cmzn_region_can_merge(top_region, region))
								{
									if (!cmzn_region_merge(top_region, region))
									{
										if (use_data)
										{
											display_message(ERROR_MESSAGE,
												"Error merging data from file: %s", file_name);
										}
										else
										{
											display_message(ERROR_MESSAGE,
												"Error merging nodes from file: %s", file_name);
										}
										return_code = 0;
									}
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"Contents of file %s not compatible with global objects",
										file_name);
									return_code = 0;
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"Error reading node file: %s", file_name);
								return_code = 0;
							}
							DEACCESS(cmzn_region)(&region);
							IO_stream_close(input_file);
							DESTROY(IO_stream)(&input_file);
							input_file =NULL;
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"Could not open node file: %s", file_name);
							return_code = 0;
						}
					}
					DEACCESS(cmzn_region)(&top_region);
					if (return_code && time_set_flag)
					{
						/* Increase the range of the default time keepeer and set the
						   minimum and maximum if we set anything */
						maximum = command_data->default_time_keeper_app->getTimeKeeper()->getMaximum();
						minimum = command_data->default_time_keeper_app->getTimeKeeper()->getMinimum();
						if (time < minimum)
						{
							command_data->default_time_keeper_app->setMinimum(time);
							command_data->default_time_keeper_app->setMaximum(maximum);
						}
						if (time > maximum)
						{
							command_data->default_time_keeper_app->setMinimum(minimum);
							command_data->default_time_keeper_app->setMaximum(time);
						}
					}
				}
			}
			DESTROY(Option_table)(&option_table);
#if defined (WX_USER_INTERFACE)
			if (input_file)
				 DESTROY(IO_stream)(&input_file);
			input_file =NULL;
#endif /*defined (WX_USER_INTERFACE)*/
			if (file_name)
			{
				DEALLOCATE(file_name);
			}
			if (region_path)
			{
				DEALLOCATE(region_path);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"gfx_read_nodes.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_read_nodes.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_read_nodes */

static int gfx_read_objects(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 7 January 2003

DESCRIPTION :
If a file is not specified a file selection box is presented to the user,
otherwise the file of graphics objects is read.
==============================================================================*/
{
	char *file_name;
	int return_code;
	struct cmzn_command_data *command_data;
	struct Option_table *option_table;

	ENTER(gfx_read_objects);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (NULL != (command_data = (struct cmzn_command_data *)command_data_void))
		{
			file_name = (char *)NULL;
			option_table=CREATE(Option_table)();
			/* example */
			Option_table_add_entry(option_table,CMGUI_EXAMPLE_DIRECTORY_SYMBOL,
				&file_name, &(command_data->example_directory), set_file_name);
			/* default */
			Option_table_add_entry(option_table,NULL,&file_name,
				NULL,set_file_name);
			return_code = Option_table_multi_parse(option_table,state);
			DESTROY(Option_table)(&option_table);
			if (return_code)
			{
				if (!file_name)
				{
					if (!(file_name = confirmation_get_read_filename(".exgobj",
						command_data->user_interface
#if defined(WX_USER_INTERFACE)
								 , command_data->execute_command
#endif /*defined (WX_USER_INTERFACE) */
																													 )))
					{
						return_code = 0;
					}
				}
				if (return_code)
				{
					/* open the file */
					if (0 != (return_code = check_suffix(&file_name, ".exgobj")))
					{
						return_code=file_read_graphics_objects(file_name, command_data->io_stream_package,
							command_data->materialmodule, command_data->glyphmodule);
					}
				}
			}
			DEALLOCATE(file_name);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_read_objects.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_read_objects.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_read_objects */

/***************************************************************************//**
 * Read regions and fields in formats supported by zinc (EX, FieldML).
 * If filename is not specified a file selection box is presented to the user.
 */
static int gfx_read_region(struct Parse_state *state,
	void *dummy, void *command_data_void)
{
	const char file_ext[] = ".fieldml";
	int return_code = 0;
	USE_PARAMETER(dummy);
	cmzn_command_data *command_data = reinterpret_cast<cmzn_command_data*>(command_data_void);
	if (state && command_data)
	{
		struct FE_field_order_info *field_order_info = (struct FE_field_order_info *)NULL;
		cmzn_region_id region = cmzn_region_access(command_data->root_region);
		char *file_name = (char *)NULL;

		struct Option_table *option_table = CREATE(Option_table)();
		/* fields */
		Option_table_add_entry(option_table, "fields", &field_order_info,
			cmzn_region_get_FE_region(command_data->root_region),
			set_FE_fields_FE_region);
		/* region */
		Option_table_add_set_cmzn_region(option_table, "region",
			command_data->root_region, &region);
		/* default option: file name */
		Option_table_add_default_string_entry(option_table, &file_name, "FILE_NAME");
		return_code = Option_table_multi_parse(option_table, state);
		DESTROY(Option_table)(&option_table);
		if (return_code)
		{
			if (!file_name)
			{
				if (!(file_name = confirmation_get_read_filename(file_ext,
					command_data->user_interface
#if defined(WX_USER_INTERFACE)
					, command_data->execute_command
#endif /*defined (WX_USER_INTERFACE) */
					)))
				{
					return_code = 0;
				}
			}
			if (return_code)
			{
				cmzn_streaminformation_id streaminformation = cmzn_region_create_streaminformation_region(region);
				cmzn_streamresource_id resource = cmzn_streaminformation_create_streamresource_file(
					streaminformation, file_name);
				cmzn_streaminformation_region_id streaminformation_region =
					cmzn_streaminformation_cast_region(streaminformation);
				return_code = cmzn_region_read(region, streaminformation_region);
				cmzn_streamresource_destroy(&resource);
				cmzn_streaminformation_region_destroy(&streaminformation_region);
				cmzn_streaminformation_destroy(&streaminformation);
				if (return_code != CMZN_OK)
				{
					display_message(ERROR_MESSAGE,
						"Error reading region file: %s", file_name);
					return_code = 0;
				}
			}
		}
		if (field_order_info)
		{
			DESTROY(FE_field_order_info)(&field_order_info);
		}
		if (file_name)
		{
			DEALLOCATE(file_name);
		}
		cmzn_region_destroy(&region);
	}
	return return_code;
}

/**
 * If a file is not specified a file selection box is presented to the user,
 * otherwise the wavefront obj file is read.
 */
static int gfx_read_wavefront_obj(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
{
	char *file_name, *graphics_object_name,	*specified_graphics_object_name;
	const char *render_polygon_mode_string, **valid_strings;
	enum cmzn_graphics_render_polygon_mode render_polygon_mode;
	float time;
	int number_of_valid_strings, return_code;
	struct cmzn_command_data *command_data;
	struct Option_table *option_table;

	ENTER(gfx_read_wavefront_obj);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if ((command_data=(struct cmzn_command_data *)command_data_void))
		{
			specified_graphics_object_name=(char *)NULL;
			graphics_object_name=(char *)NULL;
			time = 0;
			file_name=(char *)NULL;

			option_table=CREATE(Option_table)();
			/* example */
			Option_table_add_entry(option_table,CMGUI_EXAMPLE_DIRECTORY_SYMBOL,
			  &file_name, &(command_data->example_directory), set_file_name);
			/* as */
			Option_table_add_entry(option_table,"as",&specified_graphics_object_name,
				(void *)1,set_name);
			/* render_polygon_mode */
			render_polygon_mode = CMZN_GRAPHICS_RENDER_POLYGON_MODE_SHADED;
			render_polygon_mode_string = ENUMERATOR_STRING(cmzn_graphics_render_polygon_mode)(render_polygon_mode);
			valid_strings = ENUMERATOR_GET_VALID_STRINGS(cmzn_graphics_render_polygon_mode)(
				&number_of_valid_strings,
				(ENUMERATOR_CONDITIONAL_FUNCTION(cmzn_graphics_render_polygon_mode) *)NULL, (void *)NULL);
			Option_table_add_enumerator(option_table,number_of_valid_strings,
				valid_strings,&render_polygon_mode_string);
			DEALLOCATE(valid_strings);
			/* time */
			Option_table_add_entry(option_table,"time",&time,NULL,set_float);
			/* default */
			Option_table_add_entry(option_table,NULL,&file_name,
				NULL,set_file_name);

			return_code=Option_table_multi_parse(option_table,state);
			DESTROY(Option_table)(&option_table);
			if (return_code)
			{
				STRING_TO_ENUMERATOR(cmzn_graphics_render_polygon_mode)(render_polygon_mode_string, &render_polygon_mode);
				if (!file_name)
				{
					if (!(file_name = confirmation_get_read_filename(".obj",
						command_data->user_interface
#if defined(WX_USER_INTERFACE)
									 , command_data->execute_command
#endif /*defined (WX_USER_INTERFACE) */
																													 )))
					{
						return_code = 0;
					}
				}
				if (return_code)
				{
					if(specified_graphics_object_name)
					{
						graphics_object_name = specified_graphics_object_name;
					}
					else
					{
						graphics_object_name = file_name;
					}
					/* open the file */
					if (0 != (return_code = check_suffix(&file_name, ".obj")))
					{

						return_code=file_read_surface_graphics_object_from_obj(file_name,
							command_data->io_stream_package,
							graphics_object_name, render_polygon_mode, time,
							command_data->materialmodule,
							command_data->glyphmodule);
					}
				}
			}
			if (file_name)
			{
				DEALLOCATE(file_name);
			}
			if (specified_graphics_object_name)
			{
				DEALLOCATE(specified_graphics_object_name);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_read_wavefront_obj.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_read_wavefront_obj.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_read_wavefront_obj */

static int execute_command_gfx_read(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 7 January 2003

DESCRIPTION :
Executes a GFX READ command.
==============================================================================*/
{
	int return_code;
	struct cmzn_command_data *command_data;
	struct Option_table *option_table;

	ENTER(execute_command_gfx_read);
	USE_PARAMETER(dummy_to_be_modified);
	if (state && (command_data = (struct cmzn_command_data *)command_data_void))
	{
		if (state->current_token)
		{
			option_table = CREATE(Option_table)();
			/* curve */
			Option_table_add_entry(option_table, "curve",
				NULL, command_data_void, gfx_read_Curve);
			/* data */
			Option_table_add_entry(option_table, "data",
				/*use_data*/(void *)1, command_data_void, gfx_read_nodes);
			/* elements */
			Option_table_add_entry(option_table, "elements",
				NULL, command_data_void, gfx_read_elements);
			/* nodes */
			Option_table_add_entry(option_table, "nodes",
				/*use_data*/(void *)0, command_data_void, gfx_read_nodes);
			/* objects */
			Option_table_add_entry(option_table, "objects",
				NULL, command_data_void, gfx_read_objects);
			/* region */
			Option_table_add_entry(option_table, "region",
				NULL, command_data_void, gfx_read_region);
			/* wavefront_obj */
			Option_table_add_entry(option_table, "wavefront_obj",
				NULL, command_data_void, gfx_read_wavefront_obj);
			return_code = Option_table_parse(option_table, state);
			DESTROY(Option_table)(&option_table);
		}
		else
		{
			set_command_prompt("gfx read", command_data);
			return_code = 1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"execute_command_gfx_read.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_gfx_read */

/**
 * Executes a GFX SELECT|UNSELECT command.
 * @param unselect_flag_void  0 to select, non-zero to unselect.
 */
static int execute_command_gfx_select(struct Parse_state *state,
	void *unselect_flag_void,void *command_data_void)
{
	char all_flag,data_flag,elements_flag,faces_flag,grid_points_flag, *conditional_field_name,
		lines_flag,nodes_flag, *region_path, verbose_flag;
	FE_value time;
	int return_code;
	struct Computed_field *conditional_field;
	struct cmzn_command_data *command_data;
	struct cmzn_region *region;
	struct Element_point_ranges *element_point_ranges;
	struct FE_element_grid_to_Element_point_ranges_list_data grid_to_list_data;
	struct FE_field *grid_field;
	struct FE_region *fe_region;
	struct Multi_range *multi_range;
	struct Option_table *option_table;
	struct Set_FE_field_conditional_FE_region_data set_grid_field_data;

	bool unselect = (0 != unselect_flag_void);

	if (state&&(command_data=(struct cmzn_command_data *)command_data_void))
	{
		if (state->current_token)
		{
			region_path = cmzn_region_get_root_region_path();
			fe_region = cmzn_region_get_FE_region(command_data->root_region);
			all_flag = 0;
			conditional_field=(struct Computed_field *)NULL;
			data_flag = 0;
			element_point_ranges=(struct Element_point_ranges *)NULL;
			data_flag = 0;
			elements_flag = 0;
			faces_flag = 0;
			grid_points_flag = 0;
			lines_flag = 0;
			nodes_flag = 0;
			conditional_field_name = NULL;
			/* With the current method the selection is always additive
				and so to set the selected flag makes the command useless */
			multi_range=CREATE(Multi_range)();
			if ((grid_field = FE_region_get_FE_field_from_name(fe_region,
				"grid_point_number")) &&
				FE_field_is_1_component_integer(grid_field, (void *)NULL))
			{
				ACCESS(FE_field)(grid_field);
			}
			else
			{
				grid_field = (struct FE_field *)NULL;
			}
			if (command_data->default_time_keeper_app)
			{
				time = command_data->default_time_keeper_app->getTimeKeeper()->getTime();
			}
			else
			{
				time = 0.0;
			}
			verbose_flag = 0;

			option_table=CREATE(Option_table)();
			/* all */
			Option_table_add_entry(option_table,"all", &all_flag,
				(void *)NULL,set_char_flag);
			/* conditional_field */
			Option_table_add_string_entry(option_table,"conditional_field",
				&conditional_field_name, " FIELD_NAME");
			/* data */
			Option_table_add_entry(option_table,"data", &data_flag,
				(void *)NULL,set_char_flag);
			/* elements */
			Option_table_add_entry(option_table,"elements",&elements_flag,
				(void *)NULL,set_char_flag);
			/* faces */
			Option_table_add_entry(option_table,"faces",&faces_flag,
				(void *)NULL,set_char_flag);
			/* grid_field */
			set_grid_field_data.conditional_function =
				FE_field_is_1_component_integer;
			set_grid_field_data.user_data = (void *)NULL;
			set_grid_field_data.fe_region = fe_region;
			Option_table_add_entry(option_table, "grid_field", &grid_field,
				(void *)&set_grid_field_data, set_FE_field_conditional_FE_region);
			/* grid_points */
			Option_table_add_entry(option_table,"grid_points",&grid_points_flag,
				(void *)NULL,set_char_flag);
			/* group */
			Option_table_add_entry(option_table, "group", &region_path,
				command_data->root_region, set_cmzn_region_path);
			/* lines */
			Option_table_add_entry(option_table,"lines",&lines_flag,
				(void *)NULL,set_char_flag);
			/* nodes */
			Option_table_add_entry(option_table,"nodes",&nodes_flag,
				(void *)NULL,set_char_flag);
			/* points */
			Option_table_add_entry(option_table,"points",&element_point_ranges,
				(void *)fe_region, set_Element_point_ranges);
			/* verbose */
			Option_table_add_char_flag_entry(option_table,"verbose",
				&verbose_flag);
			/* default option: multi range */
			Option_table_add_entry(option_table, (const char *)NULL, (void *)multi_range,
				NULL, set_Multi_range);
			cmzn_fieldmodule_id fieldmodule = 0;
			if (0 != (return_code = Option_table_multi_parse(option_table, state)))
			{
				if ((data_flag + elements_flag + faces_flag + grid_points_flag
					+ lines_flag + nodes_flag) != 1)
				{
					display_message(ERROR_MESSAGE, "gfx (un)select:  "
						"You must specify one and only one of "
						"data/elements/faces/lines/grid_points/nodes.");
					return_code = 0;
				}
				if (region_path &&
					cmzn_region_get_region_from_path_deprecated(command_data->root_region,
						region_path, &region) &&
					(fe_region = cmzn_region_get_FE_region(region)))
				{
					fieldmodule = cmzn_region_get_fieldmodule(region);
				}
				else
				{
					display_message(ERROR_MESSAGE, "gfx (un)select:  Invalid region");
					return_code = 0;
				}
				if (return_code && conditional_field_name)
				{
					conditional_field = cmzn_fieldmodule_find_field_by_name(fieldmodule,
						conditional_field_name);
					if (!conditional_field)
					{
						display_message(ERROR_MESSAGE,
							"gfx (un)select:  conditional field cannot be found");
						return_code = 0;
					}
				}
			}
			if (return_code)
			{
				cmzn_fieldmodule_begin_change(fieldmodule);
				/* datapoints, nodes */
				if (data_flag || nodes_flag)
				{
					cmzn_field_domain_type domain_type = nodes_flag ? CMZN_FIELD_DOMAIN_TYPE_NODES : CMZN_FIELD_DOMAIN_TYPE_DATAPOINTS;
					cmzn_nodeset_id nodeset = cmzn_fieldmodule_find_nodeset_by_field_domain_type(fieldmodule, domain_type);
					cmzn_field_id use_conditional_field = cmzn_nodeset_create_conditional_field_from_ranges_and_selection(
						nodeset, multi_range, /*selection_field*/0, /*groupField*/0, conditional_field, time);
					if (use_conditional_field)
					{
						cmzn_scene *local_scene = cmzn_region_get_scene(region);
						const int result = cmzn_scene_change_node_selection_conditional(
							local_scene, domain_type, use_conditional_field, !unselect);
						if (result != CMZN_OK)
							return_code = 0;
						cmzn_scene_destroy(&local_scene);
					}
					cmzn_field_destroy(&use_conditional_field);
					cmzn_nodeset_destroy(&nodeset);
				}
				/* element_points */
				if (element_point_ranges)
				{
					if (unselect)
						Element_point_ranges_selection_unselect_element_point_ranges(
							command_data->element_point_ranges_selection,element_point_ranges);
					else
						Element_point_ranges_selection_select_element_point_ranges(
							command_data->element_point_ranges_selection,element_point_ranges);
				}
				/* elements */
				const int element_dimension = elements_flag ? FE_region_get_highest_dimension(fe_region) :
					(faces_flag ? 2 : (lines_flag ? 1 : 0));
				if (element_dimension > 0)
				{
					cmzn_mesh_id mesh = cmzn_fieldmodule_find_mesh_by_dimension(fieldmodule, element_dimension);
					cmzn_field_id use_conditional_field = cmzn_mesh_create_conditional_field_from_ranges_and_selection(
						mesh, multi_range, /*selection_field*/0, /*groupField*/0, conditional_field, time);
					if (use_conditional_field)
					{
						cmzn_scene *local_scene = cmzn_region_get_scene(region);
						const int result = cmzn_scene_change_element_selection_conditional(
							local_scene, element_dimension, use_conditional_field, !unselect);
						if (result != CMZN_OK)
							return_code = 0;
						cmzn_scene_destroy(&local_scene);
					}
					cmzn_field_destroy(&use_conditional_field);
					cmzn_mesh_destroy(&mesh);
				}
				/* grid_points */
				if (grid_points_flag)
				{
					if (0<Multi_range_get_total_number_in_ranges(multi_range))
					{
						if (grid_field)
						{
							if (NULL != (grid_to_list_data.element_point_ranges_list=
								CREATE(LIST(Element_point_ranges))()))
							{
								grid_to_list_data.grid_fe_field=grid_field;
								grid_to_list_data.grid_value_ranges=multi_range;
								/* inefficient: go through every element in FE_region */
								FE_region_for_each_FE_element(fe_region,
									FE_element_grid_to_Element_point_ranges_list,
									(void *)&grid_to_list_data);
								if (0 < NUMBER_IN_LIST(Element_point_ranges)(
									grid_to_list_data.element_point_ranges_list))
								{
									Element_point_ranges_selection_begin_cache(
										command_data->element_point_ranges_selection);
									FOR_EACH_OBJECT_IN_LIST(Element_point_ranges)(
										unselect ? Element_point_ranges_unselect : Element_point_ranges_select,
										(void *)command_data->element_point_ranges_selection,
										grid_to_list_data.element_point_ranges_list);
									Element_point_ranges_selection_end_cache(
										command_data->element_point_ranges_selection);
								}
								DESTROY(LIST(Element_point_ranges))(
									&(grid_to_list_data.element_point_ranges_list));
							}
							else
							{
								display_message(ERROR_MESSAGE,"execute_command_gfx_select.  "
									"Could not create grid_point list");
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,"To (un)select grid_points, "
								"need integer grid_field (eg. grid_point_number)");
						}
					}
				}
				cmzn_fieldmodule_end_change(fieldmodule);
			}
			DESTROY(Option_table)(&option_table);
			cmzn_fieldmodule_destroy(&fieldmodule);
			DEALLOCATE(region_path);
			if (conditional_field_name)
				DEALLOCATE(conditional_field_name);
			if (conditional_field)
			{
				DEACCESS(Computed_field)(&conditional_field);
			}
			if (grid_field)
			{
				DEACCESS(FE_field)(&grid_field);
			}
			DESTROY(Multi_range)(&multi_range);
			if (element_point_ranges)
			{
				DESTROY(Element_point_ranges)(&element_point_ranges);
			}
		}
		else
		{
			set_command_prompt(unselect ? "gfx unselect" : "gfx select",command_data);
			return_code = 1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"execute_command_gfx_select.  Invalid argument(s)");
		return_code=0;
	}
	return (return_code);
}

/***************************************************************************//**
 * Sets the order of regions in the region hierarchy.
 */
static int gfx_set_region_order(struct Parse_state *state,
	void *dummy_to_be_modified, void *root_region_void)
{
	int return_code = 0;

	ENTER(gfx_set_region_order);
	USE_PARAMETER(dummy_to_be_modified);
	cmzn_region *root_region = (struct cmzn_region *)root_region_void;
	if (state && root_region)
	{
		char *insert_before_sibling_name = NULL;
		cmzn_region_id region = cmzn_region_access(root_region);
		cmzn_field_group_id group = 0;
		Option_table *option_table = CREATE(Option_table)();
		Option_table_add_help(option_table,
			"Change the order of regions in the region hierarchy. The 'region' "
			"option specifies the current path of the region to be moved. The "
			"'before' option gives the name of an existing sibling region "
			"which the region will be re-inserted before.");
		Option_table_add_string_entry(option_table, "before",
			&insert_before_sibling_name, " SIBLING_REGION_NAME");
		Option_table_add_region_or_group_entry(option_table,
			"region", &region, &group);
		return_code = Option_table_multi_parse(option_table, state);
		DESTROY(Option_table)(&option_table);
		if (group)
		{
			display_message(WARNING_MESSAGE,
				"gfx set order:  Obsolete for subgroups. To set rendering order within a region, create graphics in required order.");
			display_parse_state_location(state);
		}
		else if (return_code)
		{
			if (!region)
			{
				display_message(ERROR_MESSAGE,
					"gfx set order.  Must specify a region");
				return_code = 0;
			}
			if (!insert_before_sibling_name)
			{
				display_message(ERROR_MESSAGE,
					"gfx set order.  Must specify a sibling region name to insert before");
				return_code = 0;
			}
			if (return_code)
			{
				cmzn_region *parent = cmzn_region_get_parent(region);
				cmzn_region *sibling = cmzn_region_find_child_by_name(parent,
					insert_before_sibling_name);
				if (sibling)
				{
					cmzn_region_insert_child_before(parent, region, sibling);
					cmzn_region_destroy(&sibling);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"gfx set order.  Cannot find sibling '%s' to insert before",
						insert_before_sibling_name);
					return_code = 0;
				}
				cmzn_region_destroy(&parent);
			}
		}
		DEALLOCATE(insert_before_sibling_name);
		cmzn_field_group_destroy(&group);
		cmzn_region_destroy(&region);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_set_region_order.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
}

#if defined (WX_USER_INTERFACE)
static int gfx_set_time(struct Parse_state *state,void *dummy_to_be_modified,
	void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 19 October 1998

DESCRIPTION :
Sets the time from the command line.
==============================================================================*/
{
	char *timekeeper_name;
	float time;
	int return_code;
	struct cmzn_command_data *command_data;
	static struct Modifier_entry option_table[]=
	{
		{"timekeeper",NULL,NULL,set_name},
		{NULL,NULL,NULL,set_float}
	};

	ENTER(gfx_set_time);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (NULL != (command_data = (struct cmzn_command_data *)command_data_void))
		{
			ALLOCATE(timekeeper_name,char,10);
			/* there is only a default timekeeper at the moment but I am making the
				commands with a timekeeper manager in mind */
			strcpy(timekeeper_name,"default");
			if (command_data->default_time_keeper_app)
			{
				time=command_data->default_time_keeper_app->getTimeKeeper()->getTime();
			}
			else
			{
				/*This option is used so that help comes out*/
				time = 0;
			}
			(option_table[0]).to_be_modified= &timekeeper_name;
			(option_table[1]).to_be_modified= &time;
			return_code=process_multiple_options(state,option_table);
			/* no errors, not asking for help */
			if (return_code)
			{
				/* the old routine only use to call this if the time wasn't the
					same as the default time, but the timekeeper might not be the
					default */
				command_data->default_time_keeper_app->requestNewTime(time);
			} /* parse error, help */
			DEALLOCATE(timekeeper_name);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_set_time.  Invalid argument(s)");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_set_time.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_set_time */
#endif /* defined (WX_USER_INTERFACE)*/

static int set_transformation_matrix(struct Parse_state *state,
	void *transformation_matrix_void,void *dummy_user_data)
/*******************************************************************************
LAST MODIFIED : 16 June 1999

DESCRIPTION :
Sets a transformation matrix from the command line.
==============================================================================*/
{
	const char *current_token;
	gtMatrix *transformation_matrix;
	int i,j,return_code;

	ENTER(set_transformation_matrix);
	USE_PARAMETER(dummy_user_data);
	if (state)
	{
		if ((current_token=state->current_token)&&(
			!strcmp(PARSER_HELP_STRING,current_token)||
			!strcmp(PARSER_RECURSIVE_HELP_STRING,current_token)))
		{
			display_message(INFORMATION_MESSAGE,"# # # # # # # # # # # # # # # #");
			if (NULL != (transformation_matrix=(gtMatrix *)transformation_matrix_void))
			{
				display_message(INFORMATION_MESSAGE,
					" [%g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g]",
					(*transformation_matrix)[0][0],(*transformation_matrix)[0][1],
					(*transformation_matrix)[0][2],(*transformation_matrix)[0][3],
					(*transformation_matrix)[1][0],(*transformation_matrix)[1][1],
					(*transformation_matrix)[1][2],(*transformation_matrix)[1][3],
					(*transformation_matrix)[2][0],(*transformation_matrix)[2][1],
					(*transformation_matrix)[2][2],(*transformation_matrix)[2][3],
					(*transformation_matrix)[3][0],(*transformation_matrix)[3][1],
					(*transformation_matrix)[3][2],(*transformation_matrix)[3][3]);
			}
			return_code=1;
		}
		else
		{
			if (NULL != (transformation_matrix=(gtMatrix *)transformation_matrix_void))
			{
				return_code=1;
				i=0;
				j=0;
				while ((i<4)&&return_code&&current_token)
				{
					j=0;
					while ((j<4)&&return_code&&current_token)
					{
						if (1==sscanf(current_token,"%lf",&((*transformation_matrix)[i][j])))
						{
							shift_Parse_state(state,1);
							current_token=state->current_token;
						}
						else
						{
							return_code=0;
						}
						j++;
					}
					i++;
				}
				if (!return_code||(i<4)||(j<4))
				{
					if (current_token)
					{
						display_message(ERROR_MESSAGE,
							"Error reading transformation matrix: %s",current_token);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Error reading transformation matrix");
					}
					display_parse_state_location(state);
					return_code=0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"set_transformation_matrix.  Missing transformation_matrix");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_transformation_matrix.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_transformation_matrix */

static int gfx_set_transformation(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
{
	int return_code;

	ENTER(gfx_set_transformation);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		struct cmzn_command_data *command_data = (struct cmzn_command_data *)command_data_void;
		if (command_data)
		{
			 /* initialise defaults */
			cmzn_field_id field = 0;
			cmzn_region *region = cmzn_region_access(command_data->root_region);
			char *field_name = NULL;
			char off_flag = 0;
			gtMatrix transformation_matrix;
			transformation_matrix[0][0]=1;
			transformation_matrix[0][1]=0;
			transformation_matrix[0][2]=0;
			transformation_matrix[0][3]=0;
			transformation_matrix[1][0]=0;
			transformation_matrix[1][1]=1;
			transformation_matrix[1][2]=0;
			transformation_matrix[1][3]=0;
			transformation_matrix[2][0]=0;
			transformation_matrix[2][1]=0;
			transformation_matrix[2][2]=1;
			transformation_matrix[2][3]=0;
			transformation_matrix[3][0]=0;
			transformation_matrix[3][1]=0;
			transformation_matrix[3][2]=0;
			transformation_matrix[3][3]=1;
			/* parse the command line */
			Option_table *option_table = CREATE(Option_table)();
			Option_table_add_set_cmzn_region(option_table,
				"name", command_data->root_region, &region);
			Option_table_add_string_entry(option_table,
				"field", &field_name, " FIELD_NAME");
			Option_table_add_char_flag_entry(option_table,
				"off", &off_flag);
			/* default: set transformation matrix */
			Option_table_add_entry(option_table, (const char *)NULL,
				&transformation_matrix, /*user_data*/(void *)NULL, set_transformation_matrix);
			return_code = Option_table_multi_parse(option_table, state);
			DESTROY(Option_table)(&option_table);
			if (return_code)
			{
				if (field_name)
				{
					cmzn_fieldmodule_id field_module = cmzn_region_get_fieldmodule(region);
					field = cmzn_fieldmodule_find_field_by_name(field_module, field_name);
					cmzn_fieldmodule_destroy(&field_module);
					if (!field)
					{
						display_message(ERROR_MESSAGE,
							"gfx_set_transformation: transformation field cannot be found");
						return_code = 0;
					}
				}
				if (return_code)
				{
					cmzn_scene *scene = cmzn_region_get_scene(region);
					if (scene)
					{
						// Cmgui has always used OpenGL-style column major transformation matrices
						scene->setTransformationMatrixColumnMajor(true);
						if (off_flag)
						{
							cmzn_scene_clear_transformation(scene);
						}
						else if (field)
						{
							cmzn_scene_set_transformation_field(scene, field);
						}
						else
						{
							double mat[16];
							for (int col = 0; col < 4; ++col)
								for (int row = 0; row < 4; ++row)
									mat[col*4 + row] = transformation_matrix[col][row];
							cmzn_scene_set_transformation_matrix(scene, mat);
						}
						DEACCESS(cmzn_scene)(&scene);
					}
					return_code = 1;
				}
				if (field)
					cmzn_field_destroy(&field);
			}
			cmzn_region_destroy(&region);
			if (field_name)
				DEALLOCATE(field_name);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_set_transformation.  Missing command_data_void");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_set_transformation.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_set_transformation */

/***************************************************************************//**
 * Toggles the visibility of graphics objects on scenes from the command line.
 */
static int gfx_set_visibility(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
{
	int return_code;
	USE_PARAMETER(dummy_to_be_modified);
	cmzn_command_data *command_data = reinterpret_cast<cmzn_command_data *>(command_data_void);
	if (state && command_data)
	{
		char on_flag = 0;
		char off_flag = 0;
		char *part_graphics_name = 0;
		char *region_path = 0;
		Option_table *option_table = CREATE(Option_table)();
		Option_table_add_help(option_table,
			"Set the visibility of a whole region's scene or individual graphics in it. "
			"Specify visibility as 'on', 'off' or omit both to toggle. If name option is "
			"specified, sets visibility for all graphics in region whose name contains the string.");
		Option_table_add_char_flag_entry(option_table, "off", &off_flag);
		Option_table_add_char_flag_entry(option_table, "on", &on_flag);
		Option_table_add_string_entry(option_table, "name", &part_graphics_name, " PART_GRAPHIC_NAME");
		Option_table_add_default_string_entry(option_table, &region_path, " REGION_PATH");
		return_code = Option_table_multi_parse(option_table, state);
		DESTROY(Option_table)(&option_table);

		cmzn_region_id region = 0;
		if (return_code)
		{
			if (off_flag && on_flag)
			{
				display_message(ERROR_MESSAGE, "gfx set visibility:  Set only one of off|on");
				return_code = 0;
			}
			if (!region_path)
			{
				display_message(ERROR_MESSAGE, "gfx set visibility:  Must specify region_path");
				return_code = 0;
			}
			else
			{
				region = cmzn_region_find_subregion_at_path(command_data->root_region, region_path);
				if (!region)
				{
					display_message(ERROR_MESSAGE, "gfx set visibility:  Could not find region %s", region_path);
					return_code = 0;
				}
			}
		}
		if (return_code)
		{
			cmzn_scene_id scene = cmzn_region_get_scene(region);
			if (part_graphics_name)
			{
				cmzn_scene_begin_change(scene);
				// support legacy command files by changing visibility of each graphics using group as its subgroup field
				cmzn_graphics_id graphics = cmzn_scene_get_first_graphics(scene);
				int number_matched = 0;
				while (graphics)
				{
					char *graphics_name = cmzn_graphics_get_name_internal(graphics);
					if (strstr(graphics_name, part_graphics_name))
					{
						bool visibility_flag = on_flag ? true : (off_flag ? false : !cmzn_graphics_get_visibility_flag(graphics));
						cmzn_graphics_set_visibility_flag(graphics, visibility_flag);
						++number_matched;
					}
					DEALLOCATE(graphics_name);
					cmzn_graphics_id temp = cmzn_scene_get_next_graphics(scene, graphics);
					cmzn_graphics_destroy(&graphics);
					graphics = temp;
				}
				cmzn_scene_end_change(scene);
				if (0 == number_matched)
				{
					display_message(WARNING_MESSAGE, "gfx set visibility:  No graphics matched name '%s'", part_graphics_name);
				}
			}
			else
			{
				bool visibility_flag = on_flag ? true : (off_flag ? false : !cmzn_scene_get_visibility_flag(scene));
				cmzn_scene_set_visibility_flag(scene, visibility_flag);
			}
			cmzn_scene_destroy(&scene);
		}
		cmzn_region_destroy(&region);
		if (region_path)
			DEALLOCATE(region_path);
		if (part_graphics_name)
			DEALLOCATE(part_graphics_name);
	}
	else
	{
		display_message(ERROR_MESSAGE, "gfx_set_visibility.  Invalid argument(s)");
		return_code = 0;
	}
	return (return_code);
}

static int execute_command_gfx_set(struct Parse_state *state,
	void *dummy_to_be_modified, void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 27 November 2001

DESCRIPTION :
Executes a GFX SET command.
==============================================================================*/
{
	int return_code;
	struct cmzn_command_data *command_data;
	struct Option_table *option_table;

	ENTER(execute_command_gfx_set);
	USE_PARAMETER(dummy_to_be_modified);
	if (state && (command_data = (struct cmzn_command_data *)command_data_void))
	{
		if (state->current_token)
		{
			double point_size = 0.0;
			option_table=CREATE(Option_table)();
			Option_table_add_entry(option_table, "order", NULL,
				(void *)command_data->root_region, gfx_set_region_order);
			Option_table_add_positive_double_entry(option_table, "point_size",
				&point_size);
			Option_table_add_entry(option_table, "transformation", NULL,
				command_data_void, gfx_set_transformation);
#if defined (WX_USER_INTERFACE)
			Option_table_add_entry(option_table, "time", NULL,
				command_data_void, gfx_set_time);
#endif /* defined (WX_USER_INTERFACE)*/
			Option_table_add_entry(option_table, "visibility", NULL,
				command_data_void, gfx_set_visibility);
			return_code = Option_table_parse(option_table, state);
			if (point_size != 0.0)
			{
				display_message(WARNING_MESSAGE, "Set option 'point_size' has been removed; set point_size on individual graphics using gfx modify g_element commands instead");
				display_parse_state_location(state);
			}
			DESTROY(Option_table)(&option_table);
		}
		else
		{
			set_command_prompt("gfx set", command_data);
			return_code = 1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"execute_command_gfx_set.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_gfx_set */

/**
 * Executes a GFX SMOOTH command.
 */
static int execute_command_gfx_smooth(struct Parse_state *state,
	void *dummy_to_be_modified, void *command_data_void)
{
	USE_PARAMETER(dummy_to_be_modified);
	int return_code;
	cmzn_command_data *command_data = reinterpret_cast<cmzn_command_data *>(command_data_void);
	if (state && command_data)
	{
		char *fieldName = 0;
		cmzn_region *region = cmzn_region_access(command_data->root_region);
		FE_value time = 0.0;
		if (command_data->default_time_keeper_app)
			time = command_data->default_time_keeper_app->getTimeKeeper()->getTime();

		Option_table *option_table = CREATE(Option_table)();
		Option_table_add_string_entry(option_table, "field", &fieldName, " FINITE ELEMENT FIELD NAME");
		Option_table_add_set_cmzn_region(option_table, "region", command_data->root_region, &region);
		Option_table_add_entry(option_table, "time", &time, NULL, set_FE_value);
		return_code = Option_table_multi_parse(option_table, state);
		if (return_code)
		{
			if (0 == fieldName)
			{
				display_message(ERROR_MESSAGE, "gfx smooth:  Must specify field to smooth");
				display_parse_state_location(state);
				return_code = 0;
			}
			else
			{
				cmzn_fieldmodule_id fieldModule = cmzn_region_get_fieldmodule(region);
				cmzn_field_id field = cmzn_fieldmodule_find_field_by_name(fieldModule, fieldName);
				if (!field)
				{
					display_message(ERROR_MESSAGE, "gfx smooth:  Could not find field '%s' in region", fieldName);
					display_parse_state_location(state);
					return_code = 0;
				}
				else
				{
					cmzn_field_finite_element_id finite_element_field = cmzn_field_cast_finite_element(field);
					if (!finite_element_field)
					{
						display_message(ERROR_MESSAGE, "gfx smooth:  Field '%s' is not finite element type", fieldName);
						display_parse_state_location(state);
						return_code = 0;
					}
					else
					{
						cmzn_fieldsmoothing_id fieldsmoothing = cmzn_fieldmodule_create_fieldsmoothing(fieldModule);
						cmzn_fieldsmoothing_set_time(fieldsmoothing, time);
						int result = cmzn_field_smooth(field, fieldsmoothing);
						if (result != CMZN_OK)
							return_code = 0;
						cmzn_fieldsmoothing_destroy(&fieldsmoothing);
					}
					cmzn_field_finite_element_destroy(&finite_element_field);
				}
				cmzn_field_destroy(&field);
				cmzn_fieldmodule_destroy(&fieldModule);
			}
		}
		DESTROY(Option_table)(&option_table);
		if (fieldName)
			DEALLOCATE(fieldName);
		cmzn_region_destroy(&region);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"execute_command_gfx_smooth.  Invalid argument(s)");
		return_code = 0;
	}
	return (return_code);
}

int gfx_timekeeper(struct Parse_state *state,void *dummy_to_be_modified,
	void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 16 June 1999

DESCRIPTION :
==============================================================================*/
{
	char every, loop, maximum_flag, minimum_flag, once, play, set_time_flag,
		skip, speed_flag, stop, swing;
	double maximum, minimum, set_time, speed;
	int return_code;
	static struct Modifier_entry option_table[]=
	{
		{"every_frame",NULL,NULL,set_char_flag},
		{"loop",NULL,NULL,set_char_flag},
		{"maximum",NULL,NULL,set_double_and_char_flag},
		{"minimum",NULL,NULL,set_double_and_char_flag},
		{"once",NULL,NULL,set_char_flag},
		{"play",NULL,NULL,set_char_flag},
		{"set_time",NULL,NULL,set_double_and_char_flag},
		{"skip_frames",NULL,NULL,set_char_flag},
		{"speed",NULL,NULL,set_double_and_char_flag},
		{"stop",NULL,NULL,set_char_flag},
		{"swing",NULL,NULL,set_char_flag},
		{NULL,NULL,NULL,NULL}
	};
	struct cmzn_command_data *command_data;
	struct Time_keeper_app *time_keeper_app;

	ENTER(gfx_timekeeper);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (NULL != (command_data = (struct cmzn_command_data *)command_data_void))
		{
			if (state->current_token)
			{
				if (strcmp(PARSER_HELP_STRING,state->current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token))
				{
					if (!strcmp(state->current_token, "default"))
					{
						/* Continue */
						return_code = shift_Parse_state(state,1);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"gfx_timekeeper.  Only a default timekeeper at the moment");
						return_code=0;
					}
				}
				else
				{
					display_message(INFORMATION_MESSAGE,
						"\n      TIMEKEEPER_NAME");
					/* By not shifting the parse state the rest of the help should come out */
					return_code = 1;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_timekeeper.  Missing timekeeper name");
				return_code=0;
			}
			if (return_code)
			{
				/* initialise defaults */
				if (NULL != (time_keeper_app = command_data->default_time_keeper_app))
				{
					maximum = time_keeper_app->getTimeKeeper()->getMaximum();
					minimum = time_keeper_app->getTimeKeeper()->getMinimum();
					set_time = time_keeper_app->getTimeKeeper()->getTime();
					speed = time_keeper_app->getSpeed();
				}
				else
				{
					maximum = 0.0;
					minimum = 0.0;
					set_time = 0.0;
					speed = 30;
				}
				every = 0;
				loop = 0;
				maximum_flag = 0;
				minimum_flag = 0;
				once = 0;
				play = 0;
				set_time_flag = 0;
				skip = 0;
				speed_flag = 0;
				stop = 0;
				swing = 0;

				(option_table[0]).to_be_modified = &every;
				(option_table[1]).to_be_modified = &loop;
				(option_table[2]).to_be_modified = &maximum;
				(option_table[2]).user_data = &maximum_flag;
				(option_table[3]).to_be_modified = &minimum;
				(option_table[3]).user_data = &minimum_flag;
				(option_table[4]).to_be_modified = &once;
				(option_table[5]).to_be_modified = &play;
				(option_table[6]).to_be_modified = &set_time;
				(option_table[6]).user_data = &set_time_flag;
				(option_table[7]).to_be_modified = &skip;
				(option_table[8]).to_be_modified = &speed;
				(option_table[8]).user_data = &speed_flag;
				(option_table[9]).to_be_modified = &stop;
				(option_table[10]).to_be_modified = &swing;
				return_code=process_multiple_options(state,option_table);

				if(return_code)
				{
					if((loop + once + swing) > 1)
					{
						display_message(ERROR_MESSAGE,
							"gfx_timekeeper.  Specify only one of loop, swing or once");
						return_code = 0;
					}
					if(every && skip)
					{
						display_message(ERROR_MESSAGE,
							"gfx_timekeeper.  Specify only one of every_frame or skip_frames");
						return_code = 0;
					}
					if(play && stop)
					{
						display_message(ERROR_MESSAGE,
							"gfx_timekeeper.  Specify only one of play or stop");
						return_code = 0;
					}
				}
				if (return_code)
				{
					if ( time_keeper_app )
					{
						if ( set_time_flag )
						{
							time_keeper_app->requestNewTime(set_time);
						}
						if ( speed_flag )
						{
							time_keeper_app->setSpeed(speed);
						}
						if ( maximum_flag )
						{
							time_keeper_app->setMaximum(maximum);
						}
						if ( minimum_flag )
						{
							time_keeper_app->setMinimum(minimum);
						}
						if ( loop )
						{
							time_keeper_app->setPlayLoop();
						}
						if ( swing )
						{
							time_keeper_app->setPlaySwing();
						}
						if ( once )
						{
							time_keeper_app->setPlayOnce();
						}
						if ( every )
						{
							time_keeper_app->setPlayEveryFrame();
						}
						if ( skip )
						{
							time_keeper_app->setPlaySkipFrames();
						}
						if ( play )
						{
							time_keeper_app->play(CMZN_TIMEKEEPER_PLAY_DIRECTION_FORWARD);
						}
						if ( stop )
						{
							time_keeper_app->stop();
						}
#if defined (WX_USER_INTERFACE)
						if (command_data->graphics_window_manager)
						{
							 FOR_EACH_OBJECT_IN_MANAGER(Graphics_window)(
									Graphics_window_update_time_settings_wx,(void *)NULL,
									command_data->graphics_window_manager);
						}
#endif /*defined (WX_USER_INTERFACE)*/
					}
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_timekeeper.  Missing command data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_timekeeper.  Missing state");
		return_code=0;
	}

	LEAVE;

	return (return_code);
} /* gfx_timekeeper */

static int gfx_transform_tool(struct Parse_state *state,
	void *dummy_user_data,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 6 October 2000

DESCRIPTION :
Executes a GFX TRANSFORM_TOOL command.
==============================================================================*/
{
	int return_code;
	struct cmzn_command_data *command_data;
	struct Interactive_tool *transform_tool;

	ENTER(execute_command_gfx_transform_tool);
	USE_PARAMETER(dummy_user_data);
	if (state&&(command_data=(struct cmzn_command_data *)command_data_void)
		&& (transform_tool=command_data->transform_tool))
	{
		return_code = Transform_tool_execute_command_with_parse_state(transform_tool, state);
		if (return_code)
		{
#if defined (WX_USER_INTERFACE)
			FOR_EACH_OBJECT_IN_MANAGER(Graphics_window)(
				Graphics_window_update_Interactive_tool,(void *)transform_tool,
				command_data->graphics_window_manager);
#endif /*(WX_USER_INTERFACE)*/
			cmzn_sceneviewermodule_update_Interactive_tool(
				command_data->sceneviewermodule,	transform_tool);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"execute_command_gfx_transform_tool.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_gfx_transform_tool */

#if defined (WX_USER_INTERFACE)
static int execute_command_gfx_update(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 28 October 1998

DESCRIPTION :
Executes a GFX UPDATE command.
==============================================================================*/
{
	int return_code;
	struct cmzn_command_data *command_data;
	struct Graphics_window *window;
	static struct Modifier_entry option_table[]=
	{
		{"window",NULL,NULL,set_Graphics_window},/*???DB. "on" ? */
		{NULL,NULL,NULL,NULL}
	};

	ENTER(execute_command_gfx_update);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (NULL != (command_data = (struct cmzn_command_data *)command_data_void))
		{
			/* initialize defaults */
			window=(struct Graphics_window *)NULL;
			(option_table[0]).to_be_modified= &window;
			(option_table[0]).user_data=command_data->graphics_window_manager;
			return_code=process_multiple_options(state,option_table);
			/* no errors, not asking for help */
			if (return_code)
			{
				if (window)
				{
					return_code=Graphics_window_update_now(window);
				}
				else
				{
					return_code=FOR_EACH_OBJECT_IN_MANAGER(Graphics_window)(
						Graphics_window_update_now_iterator,(void *)NULL,
						command_data->graphics_window_manager);
				}
			} /* parse error,help */
			if (window)
			{
				DEACCESS(Graphics_window)(&window);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"execute_command_gfx_update.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_command_gfx_update.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_gfx_update */
#endif /* defined (WX_USER_INTERFACE) */

static int gfx_write_all(struct Parse_state *state,
	 void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 3 August 2007

DESCRIPTION :
If an zip file is not specified a file selection box is presented to the
user, otherwise files are written.
Can also write individual groups with the <group> option.
==============================================================================*/
{
	 FILE *com_file;
	 char *com_file_name, *exfile_name, *file_name;
	 enum FE_write_criterion write_criterion;
	 enum FE_write_recursion write_recursion;
	 int exfile_return_code, return_code, exfile_fd, com_return_code;
	 struct cmzn_command_data *command_data;
	 struct Option_table *option_table;
	 struct MANAGER(cmzn_material) *graphical_material_manager;
	 struct MANAGER(Computed_field) *computed_field_manager;
	 struct LIST(Computed_field) *list_of_fields;
	 struct List_Computed_field_commands_data list_commands_data;
	 static const char	*command_prefix;
	 FE_value time;
#if defined (WX_USER_INTERFACE)
#if defined (WIN32_SYSTEM)
	 char temp_exfile[L_tmpnam];
#else /* (WIN32_SYSTEM) */
	 char temp_exfile[] = "regionXXXXXX";
#endif /* (WIN32_SYSTEM) */
#else /* (WX_USER_INTERFACE) */
	 char *temp_exfile = NULL;
#endif /* (WX_USER_INTERFACE) */

	 ENTER(gfx_write_all);
	 USE_PARAMETER(dummy_to_be_modified);
	 if (state && (command_data=(struct cmzn_command_data *)command_data_void))
	 {
			exfile_fd = 1;
			exfile_return_code = 1;
			com_return_code = 1;
			return_code = 1;
			file_name = 0;
			cmzn_region_id root_region = cmzn_region_access(command_data->root_region);
			char *region_or_group_path = 0;
			exfile_name = (char *)NULL;
			com_file_name = (char *)NULL;
			time = 0.0;
			Multiple_strings field_names;
			write_criterion = FE_WRITE_COMPLETE_GROUP;
			write_recursion = FE_WRITE_RECURSIVE;

			option_table = CREATE(Option_table)();
			/* complete_group|with_all_listed_fields|with_any_listed_fields */
			OPTION_TABLE_ADD_ENUMERATOR(FE_write_criterion)(option_table, &write_criterion);
			/* fields */
			Option_table_add_multiple_strings_entry(option_table, "fields",
				&field_names, "FIELD_NAME [& FIELD_NAME [& ...]]|all|none|[all]");
			/* group */
			Option_table_add_string_entry(option_table, "group", &region_or_group_path,
				" RELATIVE_PATH_TO_REGION");
			/* recursion */
			OPTION_TABLE_ADD_ENUMERATOR(FE_write_recursion)(option_table, &write_recursion);
			/* root_region */
			Option_table_add_set_cmzn_region(option_table, "root",
				command_data->root_region, &root_region);
			/* time */
			Option_table_add_entry(option_table, "time",
				&time, (void *)NULL, set_FE_value);
			/* default option: file name */
			Option_table_add_default_string_entry(option_table, &file_name, "FILE_NAME");

			if (0 != (return_code = Option_table_multi_parse(option_table, state)))
			{
				 if (file_name)
				 {
						int length = strlen(file_name);
						if (ALLOCATE(com_file_name, char, length+6))
						{
							 strcpy(com_file_name, file_name);
							 strcat(com_file_name, ".com");
							 com_file_name[length+5]='\0';
						}
						if (ALLOCATE(exfile_name, char, length+11))
						{
							 strcpy(exfile_name, file_name);
							 strcat(exfile_name, ".exregion");
							 exfile_name[length+10]='\0';
						}
				 }
				enum FE_write_fields_mode write_fields_mode = FE_WRITE_ALL_FIELDS;
				if ((1 == field_names.number_of_strings) && field_names.strings)
				{
					if (fuzzy_string_compare(field_names.strings[0], "all"))
					{
						write_fields_mode = FE_WRITE_ALL_FIELDS;
					}
					else if (fuzzy_string_compare(field_names.strings[0], "none"))
					{
						write_fields_mode = FE_WRITE_NO_FIELDS;
					}
					else
					{
						write_fields_mode = FE_WRITE_LISTED_FIELDS;
					}
				}
				else if (1 < field_names.number_of_strings)
				{
					write_fields_mode = FE_WRITE_LISTED_FIELDS;
				}
				if ((FE_WRITE_LISTED_FIELDS != write_fields_mode) &&
					(FE_WRITE_COMPLETE_GROUP != write_criterion))
				{
					display_message(WARNING_MESSAGE,
						"gfx write all:  Must specify fields to use %s",
						ENUMERATOR_STRING(FE_write_criterion)(write_criterion));
					return_code = 0;
					exfile_return_code = 0;
				}
				cmzn_region_id region = 0;
				char *group_name = 0;
				if (region_or_group_path)
				{
					char *region_path = 0;
					int result = cmzn_region_get_partial_region_path(root_region, region_or_group_path,
						&region, &region_path, &group_name);
					if (CMZN_OK != result)
					{
						display_message(ERROR_MESSAGE, "Invalid region/group path '%s'", region_or_group_path);
						display_parse_state_location(state);
						return_code = 0;
					}
					if (region_path)
						DEALLOCATE(region_path);
				}
				if (region == 0)
					region = root_region;
				if (return_code)
				{
					if (!file_name)
					{
/* 						com_file_name = "temp.com"; */
						if (!(com_file_name = confirmation_get_write_filename(".com",
										 command_data->user_interface
#if defined(WX_USER_INTERFACE)
										 , command_data->execute_command
#endif /*defined (WX_USER_INTERFACE) */
																																	 )))
						{
							 com_return_code = 0;
						}
						if (!(exfile_name = confirmation_get_write_filename(".exregion",
										 command_data->user_interface
#if defined(WX_USER_INTERFACE)
										 , command_data->execute_command
#endif /*defined (WX_USER_INTERFACE) */
																																	 )))
						{
							exfile_return_code = 0;
						}
#if defined(WX_USER_INTERFACE)
						file_name = confirmation_get_write_filename(".zip",
							 command_data->user_interface
							 , command_data->execute_command);
#endif /*defined (WX_USER_INTERFACE) */
					}
#if defined (WX_USER_INTERFACE) && defined (WIN32_SYSTEM)
					if (com_file_name)
					{
						CMZN_set_directory_and_filename_WIN32(&com_file_name, command_data);
					}
					if (exfile_name)
					{
						CMZN_set_directory_and_filename_WIN32(&exfile_name, command_data);
					}
#endif /* defined (WX_USER_INTERFACE) && defined (WIN32_SYSTEM) */
				}
				if (com_return_code)
				{
					com_return_code = check_suffix(&com_file_name,".com");
				}
				if (exfile_return_code)
				{
					exfile_return_code = check_suffix(&exfile_name,".exregion");
				}
#if defined (WX_USER_INTERFACE)
#if defined (WIN32_SYSTEM)
				 /* 	Non MS-windows platform does not have mkstemp implemented,
						therefore tmpnam is used.*/
				 if (exfile_return_code)
				 {
						tmpnam(temp_exfile);
						if (temp_exfile == NULL)
						{
							 exfile_fd = -1;
						}
				 }
#else
				 /* Non MS-windows platform has mkstemp implemented into it*/
				 if (exfile_return_code)
				 {
					 exfile_fd = mkstemp((char *)temp_exfile);
				 }
#endif /* (WIN32_SYSTEM) */
#else /* (WX_USER_INTERFACE) */
				 /* Non wx_user_interface won't be able to stored the file in
						a zip file at the moment */
				 if (exfile_return_code)
				 {
					 temp_exfile = exfile_name;
				 }
#endif /* (WX_USER_INTERFACE) */
				 if (exfile_fd == -1)
				 {
						display_message(ERROR_MESSAGE,
							 "gfx_write_all.  Could not open temporary exregion file");
				 }
				 else
				 {
					 cmzn_streaminformation_region_recursion_mode recursion_mode = CMZN_STREAMINFORMATION_REGION_RECURSION_MODE_ON;
					 if (write_recursion == FE_WRITE_NON_RECURSIVE)
						 recursion_mode = CMZN_STREAMINFORMATION_REGION_RECURSION_MODE_OFF;
					 if (!(exfile_return_code = export_region_file_of_name(temp_exfile,
						 region, group_name, root_region,
						 /*write_elements*/CMZN_FIELD_DOMAIN_TYPE_MESH1D|CMZN_FIELD_DOMAIN_TYPE_MESH2D|
						 CMZN_FIELD_DOMAIN_TYPE_MESH3D|CMZN_FIELD_DOMAIN_TYPE_MESH_HIGHEST_DIMENSION,
						 /*write_nodes*/1, /*write_data*/1,
						 field_names.number_of_strings, field_names.strings,
						 time, recursion_mode,/*isFieldML*/0)))
					 {
						 display_message(ERROR_MESSAGE,
							 "gfx_write_all.  Could not create temporary data file");
					 }
				 }
				 if (com_return_code)
				 {
						if (NULL != (com_file = fopen("temp_file_com.com", "w")))
						{
							 if (exfile_name)
							 {
									fprintf(com_file, "gfx read nodes %s\n",exfile_name);
							 }
							 fclose(com_file);
							 if (command_data->computed_field_package && (computed_field_manager=
										 Computed_field_package_get_computed_field_manager(
												command_data->computed_field_package)))
							 {
									if (NULL != (list_of_fields = CREATE(LIST(Computed_field))()))
									{
										 command_prefix="gfx define field ";
										 list_commands_data.command_prefix = command_prefix;
										 list_commands_data.listed_fields = 0;
										 list_commands_data.computed_field_list = list_of_fields;
										 list_commands_data.computed_field_manager =
												computed_field_manager;
										 while (FOR_EACH_OBJECT_IN_MANAGER(Computed_field)(
															 write_Computed_field_commands_if_managed_source_fields_in_list_to_comfile,
															 (void *)&list_commands_data, computed_field_manager) &&
												(0 != list_commands_data.listed_fields))
										 {
												list_commands_data.listed_fields = 0;
										 }
										 DESTROY(LIST(Computed_field))(&list_of_fields);
									}
									else
									{
										 return_code=0;
									}
									if (!return_code)
									{
										 display_message(ERROR_MESSAGE,
												"gfx_write_all.  Could not list field commands");
									}
							 }

							 if (command_data->spectrum_manager)
							 {
									FOR_EACH_OBJECT_IN_MANAGER(cmzn_spectrum)(
										 for_each_spectrum_list_or_write_commands, (void *)"true", command_data->spectrum_manager);
							 }
							 if (NULL != (graphical_material_manager =
									cmzn_materialmodule_get_manager(command_data->materialmodule)))
							 {
									command_prefix="gfx create material ";
									return_code=FOR_EACH_OBJECT_IN_MANAGER(cmzn_material)(
										 write_Graphical_material_commands_to_comfile,(void *)command_prefix,
										 graphical_material_manager);
							 }
							 return_code =1;
#if defined (USE_CMGUI_GRAPHICS_WINDOW)
							 return_code=FOR_EACH_OBJECT_IN_MANAGER(Graphics_window)(
									write_Graphics_window_commands_to_comfile,(void *)NULL,
									command_data->graphics_window_manager);
#endif /*defined (USE_CMGUI_GRAPHICS_WINDOW)*/
							 rename("temp_file_com.com", com_file_name);
						}
				 }
#if defined (WX_USER_INTERFACE)
				 if (exfile_name)
				 {
						filedir_compressing_process_wx_compress(com_file_name, exfile_name,
							 exfile_return_code, file_name, temp_exfile);
				 }
#if !defined (WIN32_SYSTEM)
				 if (unlink(temp_exfile) == -1)
				 {
						display_message(ERROR_MESSAGE,
							 "gfx_write_all.  Could not unlink temporary exregion file");
				 }
				 if (unlink(com_file_name) == -1)
				 {
						display_message(ERROR_MESSAGE,
							 "compressing_process_wx_compress.  Could not unlink temporary com file");
				 }
#endif /*!definde (WIN32_SYSTEM)*/
#endif /* defined (WX_USER_INTERFACE) */
				 if (com_file_name)
				 {
						DEALLOCATE(com_file_name);
				 }
				 if (exfile_name)
				 {
						DEALLOCATE(exfile_name);
				 }
				if (group_name)
					DEALLOCATE(group_name);
			}
			DESTROY(Option_table)(&option_table);
			if (region_or_group_path)
			{
				 DEALLOCATE(region_or_group_path);
			}
			if (file_name)
			{
				 DEALLOCATE(file_name);
			}
			cmzn_region_destroy(&root_region);
	 }
	 else
	 {
			display_message(ERROR_MESSAGE, "gfx_write_all.  Invalid argument(s)");
			return_code = 0;
	 }
	 LEAVE;

	 return (return_code);
} /* gfx_write_all */

static int gfx_write_elements(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 30 April 2009

DESCRIPTION :
If an element file is not specified a file selection box is presented to the
user, otherwise the element file is written.
Can also write individual element groups with the <group> option.
==============================================================================*/
{
	const char *file_ext = ".exelem";
	enum FE_write_criterion write_criterion;
	enum FE_write_recursion write_recursion;
	int return_code;
	struct cmzn_command_data *command_data;
	struct Option_table *option_table;
	FE_value time;

	ENTER(gfx_write_elements);
	USE_PARAMETER(dummy_to_be_modified);
	if (state && (command_data=(struct cmzn_command_data *)command_data_void))
	{
		return_code = 1;
		cmzn_region_id root_region = cmzn_region_access(command_data->root_region);
		char *region_or_group_path = 0;
		Multiple_strings field_names;
		char *file_name = 0;
		char nodes_flag = 0, data_flag = 0;
		write_criterion = FE_WRITE_COMPLETE_GROUP;
		write_recursion = FE_WRITE_RECURSIVE;
		time = 0.0;
		option_table = CREATE(Option_table)();
		Option_table_add_help(option_table,
			"Write elements and element fields in EX format to FILE_NAME. "
			"Output is restricted to the specified <root> region which forms the "
			"root region in the EX file, and optionally a sub-group or region "
			"specified with the relative <group> path. "
			"Output can be restricted to just <fields> listed in required order, or "
			"\'none\' to list just object identifiers. "
			"Recursion options control whether sub-groups and sub-regions are output "
			"with the chosen root region or group. "
			"Specify <nodes> to include nodes and node fields in the same file. "
			"with the chosen root region or group. "
			"Time option specifies nodes and node fields at time to be output if nodes "
			"or node fields are time dependent. If time is out of range then the nodal "
			"values at the nearest valid time will be output. Time is ignored if node "
			"is not time dependent. ");
		/* complete_group|with_all_listed_fields|with_any_listed_fields */
		OPTION_TABLE_ADD_ENUMERATOR(FE_write_criterion)(option_table, &write_criterion);
		/* fields */
		Option_table_add_multiple_strings_entry(option_table, "fields",
			&field_names, "FIELD_NAME [& FIELD_NAME [& ...]]|all|none|[all]");
		/* group */
		Option_table_add_string_entry(option_table, "group", &region_or_group_path,
			" RELATIVE_PATH_TO_REGION/GROUP");
		/* nodes */
		Option_table_add_char_flag_entry(option_table, "nodes", &nodes_flag);
		/* data */
		Option_table_add_char_flag_entry(option_table, "data", &data_flag);
		/* recursion */
		OPTION_TABLE_ADD_ENUMERATOR(FE_write_recursion)(option_table, &write_recursion);
		/* root_region */
		Option_table_add_set_cmzn_region(option_table, "root",
			command_data->root_region, &root_region);
		/* time */
		Option_table_add_entry(option_table, "time", &time, (void*)NULL, set_FE_value);
		/* default option: file name */
		Option_table_add_default_string_entry(option_table, &file_name, "FILE_NAME");

		if (0 != (return_code = Option_table_multi_parse(option_table, state)))
		{
			enum FE_write_fields_mode write_fields_mode = FE_WRITE_ALL_FIELDS;
			if ((1 == field_names.number_of_strings) && field_names.strings)
			{
				if (fuzzy_string_compare(field_names.strings[0], "all"))
				{
					write_fields_mode = FE_WRITE_ALL_FIELDS;
				}
				else if (fuzzy_string_compare(field_names.strings[0], "none"))
				{
					write_fields_mode = FE_WRITE_NO_FIELDS;
				}
				else
				{
					write_fields_mode = FE_WRITE_LISTED_FIELDS;
				}
			}
			else if (1 < field_names.number_of_strings)
			{
				write_fields_mode = FE_WRITE_LISTED_FIELDS;
			}
			if ((FE_WRITE_LISTED_FIELDS != write_fields_mode) &&
				(FE_WRITE_COMPLETE_GROUP != write_criterion))
			{
				display_message(WARNING_MESSAGE,
					"gfx write elements:  Must list fields to use %s",
					ENUMERATOR_STRING(FE_write_criterion)(write_criterion));
				return_code = 0;

			}
			cmzn_region_id region = 0;
			char *group_name = 0;
			if (region_or_group_path)
			{
				char *region_path = 0;
				int result = cmzn_region_get_partial_region_path(root_region, region_or_group_path,
					&region, &region_path, &group_name);
				if (CMZN_OK != result)
				{
					display_message(ERROR_MESSAGE, "Invalid region/group path '%s'", region_or_group_path);
					display_parse_state_location(state);
					return_code = 0;
				}
				if (region_path)
					DEALLOCATE(region_path);
			}
			if (region == 0)
				region = root_region;

			if (return_code)
			{
				if (!file_name)
				{
					if (!(file_name = confirmation_get_write_filename(file_ext,
						command_data->user_interface
#if defined(WX_USER_INTERFACE)
						, command_data->execute_command
#endif /*defined (WX_USER_INTERFACE) */
						)))
					{
						return_code = 0;
					}
				}
#if defined (WX_USER_INTERFACE) && defined (WIN32_SYSTEM)
				if (file_name)
				{
					CMZN_set_directory_and_filename_WIN32(&file_name, command_data);
				}
#endif /* defined (WX_USER_INTERFACE) && (WIN32_SYSTEM) */
			}
			if (return_code)
			{
				/* open the file */
				if (0 != (return_code = check_suffix(&file_name, ".exelem")))
				{
					cmzn_streaminformation_region_recursion_mode recursion_mode = CMZN_STREAMINFORMATION_REGION_RECURSION_MODE_ON;
					if (write_recursion == FE_WRITE_NON_RECURSIVE)
						recursion_mode = CMZN_STREAMINFORMATION_REGION_RECURSION_MODE_OFF;
					return_code = export_region_file_of_name(file_name, region, group_name, root_region,
						/*write_elements*/CMZN_FIELD_DOMAIN_TYPE_MESH1D|CMZN_FIELD_DOMAIN_TYPE_MESH2D|
						CMZN_FIELD_DOMAIN_TYPE_MESH3D|CMZN_FIELD_DOMAIN_TYPE_MESH_HIGHEST_DIMENSION,
						(int)nodes_flag, /*write_data*/(int)data_flag,
						field_names.number_of_strings, field_names.strings,
						time, recursion_mode, /*isFieldML*/0);
				}
			}
			if (group_name)
				DEALLOCATE(group_name);
		}
		DESTROY(Option_table)(&option_table);
		if (region_or_group_path)
		{
			DEALLOCATE(region_or_group_path);
		}
		if (file_name)
		{
			DEALLOCATE(file_name);
		}
		cmzn_region_destroy(&root_region);
	}
	else
	{
		display_message(ERROR_MESSAGE, "gfx_write_elements.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* gfx_write_elements */

static int gfx_write_region(struct Parse_state *state,
	void *use_data, void *command_data_void)
{
	const char *file_ext = ".fieldml";
	int return_code;
	struct cmzn_command_data *command_data;
	struct Option_table *option_table;

	if (state && (command_data = (struct cmzn_command_data *)command_data_void))
	{
		return_code = 1;
		cmzn_region_id root_region = cmzn_region_access(command_data->root_region);
		char *region_or_group_path = 0;
		Multiple_strings field_names;
		char *file_name = 0;
		cmzn_region_id region = cmzn_region_access(root_region);
		option_table = CREATE(Option_table)();
		Option_table_add_help(option_table,
			"Export fields of the specified region into FieldML format. "
			"Only the specified region will be exported, child regions will not.");
		Option_table_add_set_cmzn_region(option_table, "region", root_region, &region);
		/* default option: file name */
		Option_table_add_default_string_entry(option_table, &file_name, "FILE_NAME");

		if (0 != (return_code = Option_table_multi_parse(option_table, state)))
		{
			if (!file_name)
			{
				if (!(file_name = confirmation_get_write_filename(file_ext,
					command_data->user_interface
#if defined(WX_USER_INTERFACE)
					, command_data->execute_command
#endif /*defined (WX_USER_INTERFACE) */
					)))
				{
					return_code = 0;
				}
			}
#if defined (WX_USER_INTERFACE) && defined (WIN32_SYSTEM)
			if (file_name)
			{
				CMZN_set_directory_and_filename_WIN32(&file_name, command_data);
			}
#endif /* defined (WX_USER_INTERFACE) && (WIN32_SYSTEM) */
			if (return_code)
			{
				cmzn_streaminformation_id streaminformation = cmzn_region_create_streaminformation_region(region);
				cmzn_streamresource_id resource =
					cmzn_streaminformation_create_streamresource_file(streaminformation, file_name);
				if (!resource)
					return_code = 0;
				cmzn_streaminformation_region_id streaminformation_region = cmzn_streaminformation_cast_region(streaminformation);
				cmzn_streaminformation_region_set_file_format(streaminformation_region, CMZN_STREAMINFORMATION_REGION_FILE_FORMAT_FIELDML);
				return_code = cmzn_region_write(region, streaminformation_region);
				cmzn_streamresource_destroy(&resource);
				cmzn_streaminformation_region_destroy(&streaminformation_region);
				cmzn_streaminformation_destroy(&streaminformation);
			}
		}
		DESTROY(Option_table)(&option_table);
		if (file_name)
			DEALLOCATE(file_name);
		cmzn_region_destroy(&root_region);
		cmzn_region_destroy(&region);
	}
	else
	{
		display_message(ERROR_MESSAGE, "gfx_write_region.  Invalid argument(s)");
		return_code = 0;
	}

	return (return_code);
} /* gfx_write_nodes */

static int gfx_write_nodes(struct Parse_state *state,
	void *use_data, void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 30 April 2009

DESCRIPTION :
If a nodes file is not specified a file selection box is presented to the user,
otherwise the nodes file is written.
Can now specify individual node groups to write with the <group> option.
If <use_data> is set, writing data, otherwise writing nodes.
==============================================================================*/
{
	static const char *data_file_ext = ".exdata";
	static const char *node_file_ext = ".exnode";
	const char *file_ext;
	enum FE_write_criterion write_criterion;
	enum FE_write_recursion write_recursion;
	int return_code;
	struct cmzn_command_data *command_data;
	struct Option_table *option_table;
	FE_value time;

	ENTER(gfx_write_nodes);
	if (state && (command_data = (struct cmzn_command_data *)command_data_void))
	{
		return_code = 1;
		time = 0.0;
		cmzn_region_id root_region = cmzn_region_access(command_data->root_region);
		char *region_or_group_path = 0;
		if (use_data)
		{
			file_ext = data_file_ext;
		}
		else
		{
			file_ext = node_file_ext;
		}
		Multiple_strings field_names;
		char *file_name = 0;
		write_criterion = FE_WRITE_COMPLETE_GROUP;
		write_recursion = FE_WRITE_RECURSIVE;

		option_table = CREATE(Option_table)();
		if (use_data)
		{
			Option_table_add_help(option_table, "Write data nodes and data node fields");
		}
		else
		{
			Option_table_add_help(option_table, "Write nodes and node fields");
		}
		Option_table_add_help(option_table,
			" in EX format to FILE_NAME. "
			"Output is restricted to the specified <root> region which forms the "
			"root region in the EX file, and optionally a region or sub-group "
			"specified by the relative path with the <group> option. "
			"Output can be restricted to just <fields> listed in required order, or "
			"\'none\' to list just object identifiers. "
			"Recursion options control whether sub-regions are output "
			"with the chosen root region or group. "
			"Time option allow user to specify at which time of nodes and node fields "
			"to be output if there nodes/node fields are time dependent. If time is out"
			"of range then the nodal values at the nearest valid time will be output. "
			"Time is ignored if node is not time dependent. ");

		/* complete_group|with_all_listed_fields|with_any_listed_fields */
		OPTION_TABLE_ADD_ENUMERATOR(FE_write_criterion)(option_table, &write_criterion);
		/* fields */
		Option_table_add_multiple_strings_entry(option_table, "fields",
			&field_names, "FIELD_NAME [& FIELD_NAME [& ...]]|all|none|[all]");
		/* group */
		Option_table_add_string_entry(option_table, "group", &region_or_group_path,
			" RELATIVE_PATH_TO_REGION/GROUP");
		/* recursion */
		OPTION_TABLE_ADD_ENUMERATOR(FE_write_recursion)(option_table, &write_recursion);
		/* root_region */
		Option_table_add_set_cmzn_region(option_table, "root",
			command_data->root_region, &root_region);
		/* time */
		Option_table_add_entry(option_table, "time", &time, (void*)NULL, set_FE_value);
		/* default option: file name */
		Option_table_add_default_string_entry(option_table, &file_name, "FILE_NAME");

		if (0 != (return_code = Option_table_multi_parse(option_table, state)))
		{
			enum FE_write_fields_mode write_fields_mode = FE_WRITE_ALL_FIELDS;
			if ((1 == field_names.number_of_strings) && field_names.strings)
			{
				if (fuzzy_string_compare_same_length(field_names.strings[0], "all"))
				{
					write_fields_mode = FE_WRITE_ALL_FIELDS;
				}
				else if (fuzzy_string_compare_same_length(field_names.strings[0], "none"))
				{
					write_fields_mode = FE_WRITE_NO_FIELDS;
				}
				else
				{
					write_fields_mode = FE_WRITE_LISTED_FIELDS;
				}
			}
			else if (1 < field_names.number_of_strings)
			{
				write_fields_mode = FE_WRITE_LISTED_FIELDS;
			}
			if ((FE_WRITE_LISTED_FIELDS != write_fields_mode) &&
				(FE_WRITE_COMPLETE_GROUP != write_criterion))
			{
				display_message(WARNING_MESSAGE,
					"gfx write nodes/data:  Must list fields to use %s",
					ENUMERATOR_STRING(FE_write_criterion)(write_criterion));
				return_code = 0;
			}
			cmzn_region_id region = cmzn_region_access(root_region);
			char *group_name = 0;
			if (region_or_group_path)
			{
				char *region_path = 0;
				int result = cmzn_region_get_partial_region_path(root_region, region_or_group_path,
					&region, &region_path, &group_name);
				if (CMZN_OK != result)
				{
					display_message(ERROR_MESSAGE, "Invalid region/group path '%s'", region_or_group_path);
					display_parse_state_location(state);
					return_code = 0;
				}
				if (region_path)
					DEALLOCATE(region_path);
			}
			if (region == 0)
				region = root_region;
			if (return_code)
			{
				if (!file_name)
				{
					if (!(file_name = confirmation_get_write_filename(file_ext,
						command_data->user_interface
#if defined (WX_USER_INTERFACE)
						, command_data->execute_command
#endif /* defined (WX_USER_INTERFACE) */
						)))
					{
						return_code = 0;
					}
				}
#if defined (WX_USER_INTERFACE) && defined (WIN32_SYSTEM)
				if (file_name)
				{
					 CMZN_set_directory_and_filename_WIN32(&file_name, command_data);
				}
#endif /* defined (WX_USER_INTERFACE) && defined (WIN32_SYSTEM) */
			}
			if (return_code)
			{
				cmzn_streaminformation_region_recursion_mode recursion_mode = CMZN_STREAMINFORMATION_REGION_RECURSION_MODE_ON;
				if (write_recursion == FE_WRITE_NON_RECURSIVE)
					recursion_mode = CMZN_STREAMINFORMATION_REGION_RECURSION_MODE_OFF;
				/* open the file */
				if (0 != (return_code = check_suffix(&file_name, file_ext)))
				{
					return_code = export_region_file_of_name(file_name, region, group_name, root_region,
						/*write_elements*/0, /*write_nodes*/!use_data, /*write_data*/(0 != use_data),
						field_names.number_of_strings, field_names.strings, time,
						recursion_mode, /*isFieldML*/0);
				}
			}
			if (group_name)
				DEALLOCATE(group_name);
		}
		DESTROY(Option_table)(&option_table);
		if (region_or_group_path)
		{
			DEALLOCATE(region_or_group_path);
		}
		if (file_name)
		{
			DEALLOCATE(file_name);
		}
		cmzn_region_destroy(&root_region);
	}
	else
	{
		display_message(ERROR_MESSAGE, "gfx_write_nodes.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* gfx_write_nodes */

static int gfx_write_texture(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 12 March 2002

DESCRIPTION :
Executes a GFX WRITE TEXTURE command.
==============================================================================*/
{
	const char *current_token;
	char *file_name, *file_number_pattern;
	const char *image_file_format_string, **valid_strings;
	enum Image_file_format image_file_format;
	int number_of_bytes_per_component, number_of_valid_strings,
		original_depth_texels, original_height_texels, original_width_texels,
		return_code = 0;
	struct Cmgui_image *cmgui_image;
	struct Cmgui_image_information *cmgui_image_information;
	struct cmzn_command_data *command_data;
	struct Option_table *option_table;
	struct Texture *texture;

	ENTER(gfx_write_texture);
	USE_PARAMETER(dummy_to_be_modified);
	if (state && (command_data=(struct cmzn_command_data *)command_data_void))
	{
		texture = (struct Texture *)NULL;
		if (NULL != (current_token = state->current_token))
		{
			if (strcmp(PARSER_HELP_STRING, current_token) &&
				strcmp(PARSER_RECURSIVE_HELP_STRING, current_token))
			{
				if (command_data->root_region)
				{
					struct cmzn_region *region = NULL;
					char *region_path = NULL, *field_name = NULL;
					if (cmzn_region_get_partial_region_path(command_data->root_region,
						current_token, &region, &region_path, &field_name))
					{
						cmzn_fieldmodule *field_module = cmzn_region_get_fieldmodule(region);
						return_code=1;
						if (field_name && (strlen(field_name) > 0) &&
							(strchr(field_name, CMZN_REGION_PATH_SEPARATOR_CHAR)	== NULL))
						{
							struct Computed_field *temp_field =
								cmzn_fieldmodule_find_field_by_name(field_module, field_name);
							if (temp_field)
							{
								if (!Computed_field_is_image_type(temp_field,NULL))
								{
									DEACCESS(Computed_field)(&temp_field);
									display_message(ERROR_MESSAGE,
										"set_image_field.  Field specify does not contain image "
										"information.");
									return_code=0;
								}
								else
								{
									cmzn_field_image_id image_field = cmzn_field_cast_image(temp_field);
									texture = cmzn_field_image_get_texture(image_field);
									cmzn_field_image_destroy(&image_field);
									shift_Parse_state(state, 1);
								}
								DEACCESS(Computed_field)(&temp_field);
							}
						}
						else
						{
							if (field_name)
							{
								display_message(ERROR_MESSAGE,
									"gfx_write_texture:  Invalid region path or texture field name '%s'", field_name);
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"gfx_write_texture:  Missing texture field name or name matches child region '%s'", current_token);
							}
							display_parse_state_location(state);
							return_code = 0;
						}
						cmzn_fieldmodule_destroy(&field_module);
					}
					if (region_path)
						DEALLOCATE(region_path);
					if (field_name)
						DEALLOCATE(field_name);
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx write texture:  Missing texture name");
			return_code = 0;
		}
		if (return_code)
		{
			/* initialize defaults */
			file_name = (char *)NULL;
			file_number_pattern = (char *)NULL;
			/* default file format is to obtain it from the filename extension */
			image_file_format = UNKNOWN_IMAGE_FILE_FORMAT;
			if (texture)
			{
				/* by default, save as much information as there is in the texture */
				number_of_bytes_per_component =
					Texture_get_number_of_bytes_per_component(texture);
			}
			else
			{
				number_of_bytes_per_component = 1;
			}

			option_table = CREATE(Option_table)();
			/* image file format */
			image_file_format_string =
				ENUMERATOR_STRING(Image_file_format)(image_file_format);
			valid_strings = ENUMERATOR_GET_VALID_STRINGS(Image_file_format)(
				&number_of_valid_strings,
				(ENUMERATOR_CONDITIONAL_FUNCTION(Image_file_format) *)NULL,
				(void *)NULL);
			Option_table_add_enumerator(option_table, number_of_valid_strings,
				valid_strings, &image_file_format_string);
			/* bytes_per_component */
			Option_table_add_entry(option_table, "bytes_per_component",
				&number_of_bytes_per_component, (void *)NULL, set_int_positive);
			/* file */
			Option_table_add_entry(option_table, "file", &file_name,
				(void *)1, set_name);
			/* number_pattern */
			Option_table_add_entry(option_table, "number_pattern",
				&file_number_pattern, (void *)1, set_name);
			DEALLOCATE(valid_strings);
			return_code = Option_table_multi_parse(option_table, state);
			/* no errors, not asking for help */
			if (return_code)
			{
				if (!file_name)
				{
					if (!(file_name = confirmation_get_write_filename(NULL,
									 command_data->user_interface
#if defined (WX_USER_INTERFACE)
								 , command_data->execute_command
#endif /* defined (WX_USER_INTERFACE) */
																														)))
					{
						display_message(ERROR_MESSAGE,
							"gfx write texture:  No file name specified");
						return_code = 0;
					}
				}
				if ((1 != number_of_bytes_per_component) &&
					(2 != number_of_bytes_per_component))
				{
					display_message(ERROR_MESSAGE,
						"gfx write texture:  bytes_per_component may be 1 or 2");
					return_code = 0;
				}
			}
			if (return_code)
			{
				cmgui_image_information = CREATE(Cmgui_image_information)();
				if (image_file_format_string)
				{
					STRING_TO_ENUMERATOR(Image_file_format)(
						image_file_format_string, &image_file_format);
				}
				Cmgui_image_information_set_image_file_format(
					cmgui_image_information, image_file_format);
				Cmgui_image_information_set_number_of_bytes_per_component(
					cmgui_image_information, number_of_bytes_per_component);
				Cmgui_image_information_set_io_stream_package(cmgui_image_information,
					command_data->io_stream_package);
				if (file_number_pattern)
				{
					if (strstr(file_name, file_number_pattern))
					{
						/* number images from 1 to the number of depth texels used */
						if (Texture_get_original_size(texture, &original_width_texels,
							&original_height_texels, &original_depth_texels))
						{
							Cmgui_image_information_set_file_name_series(
								cmgui_image_information, file_name, file_number_pattern,
								/*start_file_number*/1,
								/*stop_file_number*/original_depth_texels,
								/*file_number_increment*/1);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE, "gfx write texture:  "
							"File number pattern \"%s\" not found in file name \"%s\"",
							file_number_pattern, file_name);
						return_code = 0;
					}
				}
				else
				{
					Cmgui_image_information_add_file_name(cmgui_image_information,
						file_name);
				}
				if (return_code)
				{
					if (NULL != (cmgui_image = Texture_get_image(texture)))
					{
						if (!Cmgui_image_write(cmgui_image, cmgui_image_information))
						{
							display_message(ERROR_MESSAGE,
								"gfx write texture:  Error writing image %s", file_name);
							return_code = 0;
						}
						DESTROY(Cmgui_image)(&cmgui_image);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"gfx_write_texture.  Could not get image from texture");
						return_code = 0;
					}
				}
				DESTROY(Cmgui_image_information)(&cmgui_image_information);
			}
			DESTROY(Option_table)(&option_table);
			if (file_name)
			{
				DEALLOCATE(file_name);
			}
			if (file_number_pattern)
			{
				DEALLOCATE(file_number_pattern);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE, "gfx_write_texture.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* gfx_write_texture */

static int execute_command_gfx_write(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 7 March 2003

DESCRIPTION :
Executes a GFX WRITE command.
==============================================================================*/
{
	int return_code;
	struct cmzn_command_data *command_data;
	struct Option_table *option_table;

	ENTER(execute_command_gfx_write);
	USE_PARAMETER(dummy_to_be_modified);
	if (state && (command_data = (struct cmzn_command_data *)command_data_void))
	{
		if (state->current_token)
		{
			option_table = CREATE(Option_table)();
			Option_table_add_entry(option_table, "all", NULL,
				command_data_void, gfx_write_all);
			Option_table_add_entry(option_table, "data", /*use_data*/(void *)1,
				command_data_void, gfx_write_nodes);
			Option_table_add_entry(option_table, "elements", NULL,
				command_data_void, gfx_write_elements);
			Option_table_add_entry(option_table, "nodes", /*use_data*/(void *)0,
				command_data_void, gfx_write_nodes);
			Option_table_add_entry(option_table, "region", 0,
				command_data_void, gfx_write_region);
			Option_table_add_entry(option_table, "texture", NULL,
				command_data_void, gfx_write_texture);
			return_code = Option_table_parse(option_table, state);
			DESTROY(Option_table)(&option_table);
		}
		else
		{
			set_command_prompt("gfx write", command_data);
			return_code = 1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"execute_command_gfx_write.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_gfx_write */

static int execute_command_gfx(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 6 March 2003

DESCRIPTION :
Executes a GFX command.
==============================================================================*/
{
	int return_code;
	struct Option_table *option_table;
	struct cmzn_command_data *command_data;

	ENTER(execute_command_gfx);
	USE_PARAMETER(dummy_to_be_modified);
	if (state && (command_data = (struct cmzn_command_data *)command_data_void))
	{
		if (state->current_token)
		{
			option_table=CREATE(Option_table)();
			Option_table_add_entry(option_table, "change_identifier", NULL,
				command_data_void, gfx_change_identifier);
			Option_table_add_entry(option_table, "convert", NULL,
				command_data_void, gfx_convert);
			Option_table_add_entry(option_table, "create", NULL,
				command_data_void, execute_command_gfx_create);
#if defined (GTK_USER_INTERFACE) || defined (WIN32_USER_INTERFACE) || defined (WX_USER_INTERFACE)
			Option_table_add_entry(option_table, "data_tool", /*data_tool*/(void *)1,
			   command_data_void, execute_command_gfx_node_tool);
#endif /* defined (GTK_USER_INTERFACE) || defined (WIN32_USER_INTERFACE) || defined (WX_USER_INTERFACE)*/
			Option_table_add_entry(option_table, "define", NULL,
				command_data_void, execute_command_gfx_define);
			Option_table_add_entry(option_table, "destroy", NULL,
				command_data_void, execute_command_gfx_destroy);
			Option_table_add_entry(option_table, "draw", NULL,
				command_data_void, execute_command_gfx_draw);
			Option_table_add_entry(option_table, "edit", NULL,
				command_data_void, execute_command_gfx_edit);
#if defined (WX_USER_INTERFACE)
			Option_table_add_entry(option_table, "element_creator", NULL,
				command_data_void, execute_command_gfx_element_creator);
#endif /* defined (WX_USER_INTERFACE) */
#if defined (GTK_USER_INTERFACE) || defined (WIN32_USER_INTERFACE) || defined (CARBON_USER_INTERFACE) || defined (WX_USER_INTERFACE)
			Option_table_add_entry(option_table, "element_point_tool", NULL,
				command_data_void, execute_command_gfx_element_point_tool);
#endif /* defined (GTK_USER_INTERFACE) || defined	(WIN32_USER_INTERFACE) || defined (CARBON_USER_INTERFACE)  || defined (WX_USER_INTERFACE)*/
#if defined (GTK_USER_INTERFACE) || defined (WIN32_USER_INTERFACE) || defined (CARBON_USER_INTERFACE) || defined (WX_USER_INTERFACE)
			Option_table_add_entry(option_table, "element_tool", NULL,
				command_data_void, execute_command_gfx_element_tool);
#endif /* defined (GTK_USER_INTERFACE) || defined (WIN32_USER_INTERFACE) || defined (CARBON_USER_INTERFACE) || defined (WX_USER_INTERFACE) */
			Option_table_add_entry(option_table, "evaluate", NULL,
				command_data_void, gfx_evaluate);
			Option_table_add_entry(option_table, "export", NULL,
				command_data_void, execute_command_gfx_export);
#if defined (USE_OPENCASCADE)
			Option_table_add_entry(option_table, "import", NULL,
				command_data_void, execute_command_gfx_import);
#endif /* defined (USE_OPENCASCADE) */
			Option_table_add_entry(option_table, "list", NULL,
				command_data_void, execute_command_gfx_list);
			Option_table_add_entry(option_table, "minimise",
				NULL, (void *)command_data->root_region, gfx_minimise);
			Option_table_add_entry(option_table, "modify", NULL,
				command_data_void, execute_command_gfx_modify);
#if defined (SGI_MOVIE_FILE)
			Option_table_add_entry(option_table, "movie", NULL,
				command_data_void, gfx_movie);
#endif /* defined (SGI_MOVIE_FILE) */
#if defined (GTK_USER_INTERFACE) || defined (WIN32_USER_INTERFACE) || defined (CARBON_USER_INTERFACE) || defined (WX_USER_INTERFACE)
			Option_table_add_entry(option_table, "node_tool", /*data_tool*/(void *)0,
				command_data_void, execute_command_gfx_node_tool);
#endif /* defined (GTK_USER_INTERFACE) || defined	(WIN32_USER_INTERFACE) || defined (CARBON_USER_INTERFACE) || defined (WX_USER_INTERFACE) */
#if defined (GTK_USER_INTERFACE) || defined (WIN32_USER_INTERFACE) || defined (WX_USER_INTERFACE)
			Option_table_add_entry(option_table, "print", NULL,
				command_data_void, execute_command_gfx_print);
#endif
			Option_table_add_entry(option_table, "read", NULL,
				command_data_void, execute_command_gfx_read);
			Option_table_add_entry(option_table, "select", /*unselect*/0,
				command_data_void, execute_command_gfx_select);
			Option_table_add_entry(option_table, "set", NULL,
				command_data_void, execute_command_gfx_set);
			Option_table_add_entry(option_table, "mesh", NULL,
				command_data_void, execute_command_gfx_mesh);
			Option_table_add_entry(option_table, "smooth", NULL,
				command_data_void, execute_command_gfx_smooth);
			Option_table_add_entry(option_table, "timekeeper", NULL,
				command_data_void, gfx_timekeeper);
			Option_table_add_entry(option_table, "transform_tool", NULL,
				command_data_void, gfx_transform_tool);
			Option_table_add_entry(option_table, "unselect", /*unselect*/reinterpret_cast<void *>(1),
				command_data_void, execute_command_gfx_select);
#if defined (WX_USER_INTERFACE)
			Option_table_add_entry(option_table, "update", NULL,
				command_data_void, execute_command_gfx_update);
#endif /* defined (WX_USER_INTERFACE) */
			Option_table_add_entry(option_table, "write", NULL,
				command_data_void, execute_command_gfx_write);
			return_code = Option_table_parse(option_table, state);
			DESTROY(Option_table)(&option_table);
		}
		else
		{
			set_command_prompt("gfx",command_data);
			return_code = 1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE, "execute_command_gfx.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_gfx */

static int execute_command_list_memory(struct Parse_state *state,
	void *dummy_to_be_modified,void *dummy_user_data)
/*******************************************************************************
LAST MODIFIED : 16 June 1999

DESCRIPTION :
Executes a LIST_MEMORY command.
==============================================================================*/
{
	char increment_counter, suppress_pointers;
	int count_number,return_code,set_counter;
	static struct Modifier_entry option_table[]=
	{
		{"increment_counter",NULL,NULL,set_char_flag},
		{"suppress_pointers",NULL,NULL,set_char_flag},
		{NULL,NULL,NULL,set_int}
	};

	ENTER(execute_command_list_memory);
	USE_PARAMETER(dummy_to_be_modified);
	USE_PARAMETER(dummy_user_data);
	if (state)
	{
		count_number=0;
		increment_counter = 0;
		suppress_pointers = 0;
		(option_table[0]).to_be_modified= &increment_counter;
		(option_table[1]).to_be_modified= &suppress_pointers;
		(option_table[2]).to_be_modified= &count_number;
		return_code=process_multiple_options(state,option_table);
		/* no errors, not asking for help */
		if (return_code)
		{
			if (increment_counter)
			{
				set_counter = -1;
			}
			else
			{
				set_counter = 0;
			}
			if (suppress_pointers)
			{
				return_code=list_memory(count_number, /*show_pointers*/0,
					set_counter, /*show_structures*/0);
			}
			else
			{
				return_code=list_memory(count_number, /*show_pointers*/1,
					set_counter, /*show_structures*/1);
			}
		} /* parse error, help */
		else
		{
			/* no help */
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"execute_command_list_memory.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_list_memory */

static int execute_command_read(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 27 April 1999

DESCRIPTION :
Executes a READ command.
==============================================================================*/
{
	int return_code;
	struct cmzn_command_data *command_data;
	struct Open_comfile_data open_comfile_data;
	struct Option_table *option_table;

	ENTER(execute_command_read);
	USE_PARAMETER(dummy_to_be_modified);
	/* check argument */
	if (state)
	{
		if (NULL != (command_data = (struct cmzn_command_data *)command_data_void))
		{
			if (state->current_token)
			{
				option_table = CREATE(Option_table)();
				/* comfile */
				open_comfile_data.file_name=(char *)NULL;
				open_comfile_data.example_flag=0;
				open_comfile_data.execute_count=1;
				open_comfile_data.examples_directory=command_data->example_directory;
				open_comfile_data.example_symbol=CMGUI_EXAMPLE_DIRECTORY_SYMBOL;
				open_comfile_data.execute_command=command_data->execute_command;
				open_comfile_data.set_command=command_data->set_command;
				open_comfile_data.io_stream_package=command_data->io_stream_package;
				open_comfile_data.file_extension=".com";
#if defined (WX_USER_INTERFACE)
				open_comfile_data.comfile_window_manager =
					command_data->comfile_window_manager;
#endif /* defined (WX_USER_INTERFACE)*/
/* #if defined (WX_USER_INTERFACE) */
/* 				change_dir(state,NULL,command_data); */
/* #endif  (WX_USER_INTERFACE)*/
				open_comfile_data.user_interface=command_data->user_interface;
				Option_table_add_entry(option_table, "comfile", NULL,
					(void *)&open_comfile_data, open_comfile);
				return_code=Option_table_parse(option_table, state);
				DESTROY(Option_table)(&option_table);
			}
			else
			{
				set_command_prompt("read",command_data);
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"execute_command_read.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_command_read.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_read */

static int open_example(struct Parse_state *state,void *dummy_to_be_modified,
	void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 12 December 2002

DESCRIPTION :
Opens an example.
==============================================================================*/
{
	char *example, *execute_flag, *found_cm, temp_string[100];
	int return_code;
	struct cmzn_command_data *command_data;
	struct Option_table *option_table;

	ENTER(open_example);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (NULL != (command_data = (struct cmzn_command_data *)command_data_void))
		{
			example = (char *)NULL;
			execute_flag = 0;
			option_table = CREATE(Option_table)();
			/* example */
			Option_table_add_entry(option_table, "example",
				&example, (void *)1, set_name);
			/* execute */
			Option_table_add_entry(option_table, "execute",
				&execute_flag, NULL, set_char_flag);
			/* default */
			Option_table_add_entry(option_table, (const char *)NULL,
				&example, NULL, set_name);
			return_code=Option_table_multi_parse(option_table,state);
			DESTROY(Option_table)(&option_table);
			/* no errors, not asking for help */
			if (return_code)
			{
				if (!example)
				{
					display_message(ERROR_MESSAGE,
						"open_example.  You must specify an example name");
					return_code = 0;
				}
			}
			if (return_code)
			{
				/* set the examples directory */
				sprintf(temp_string,"set dir ");
				strcat(temp_string,CMGUI_EXAMPLE_DIRECTORY_SYMBOL);
				strcat(temp_string," ");
				strcat(temp_string,example);
				Execute_command_execute_string(command_data->execute_command,temp_string);
				/* The example_comfile and example_requirements strings are
					currently set as a sideeffect of "set dir" */
				if (command_data->example_requirements)
				{
					if (NULL != (found_cm = strstr(command_data->example_requirements, "cm")))
					{
						if ((found_cm[2] == 0) || (found_cm[2] == ':') ||
							(found_cm[2] == ','))
						{
							sprintf(temp_string,"create cm");
							Execute_command_execute_string(command_data->execute_command,
								temp_string);
						}
					}
				}
				sprintf(temp_string,"open comfile ");
				if (command_data->example_comfile)
				{
					strcat(temp_string,command_data->example_comfile);
				}
				else
				{
					strcat(temp_string,"example_");
					strcat(temp_string,example);
				}
				strcat(temp_string,";");
				strcat(temp_string,CMGUI_EXAMPLE_DIRECTORY_SYMBOL);
				if (execute_flag)
				{
					strcat(temp_string," execute");
				}
				return_code=Execute_command_execute_string(command_data->execute_command,
					temp_string);
			}
			if (example)
			{
				DEALLOCATE(example);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"open_example.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"open_example.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* open_comfile */

static int execute_command_open(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 18 April 2002

DESCRIPTION :
Executes a OPEN command.
==============================================================================*/
{
	int return_code;
	struct cmzn_command_data *command_data;
	struct Open_comfile_data open_comfile_data;
	struct Option_table *option_table;

	ENTER(execute_command_open);
	USE_PARAMETER(dummy_to_be_modified);
	/* check argument */
	if (state)
	{
		if (NULL != (command_data = (struct cmzn_command_data *)command_data_void))
		{
			if (state->current_token)
			{
				option_table = CREATE(Option_table)();
				/* comfile */
				open_comfile_data.file_name=(char *)NULL;
				open_comfile_data.example_flag=0;
				open_comfile_data.execute_count=0;
				open_comfile_data.examples_directory=command_data->example_directory;
				open_comfile_data.example_symbol=CMGUI_EXAMPLE_DIRECTORY_SYMBOL;
				open_comfile_data.execute_command=command_data->execute_command;
				open_comfile_data.set_command=command_data->set_command;
				open_comfile_data.io_stream_package=command_data->io_stream_package;
				open_comfile_data.file_extension=".com";
#if defined (WX_USER_INTERFACE)
				open_comfile_data.comfile_window_manager =
					command_data->comfile_window_manager;
#endif /* defined (WX_USER_INTERFACE) */
				open_comfile_data.user_interface=command_data->user_interface;
				Option_table_add_entry(option_table, "comfile", NULL,
					(void *)&open_comfile_data, open_comfile);
				Option_table_add_entry(option_table, "example", NULL,
					command_data_void, open_example);
				return_code=Option_table_parse(option_table, state);
/* #if defined (WX_USER_INTERFACE)  */
/*  				change_dir(state,NULL,command_data); */
/* #endif (WX_USER_INTERFACE)*/
				DESTROY(Option_table)(&option_table);
			}
			else
			{
				set_command_prompt("open",command_data);
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"execute_command_open.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_command_open.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_open */

static int execute_command_quit(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 16 June 1999

DESCRIPTION :
Executes a QUIT command.
==============================================================================*/
{
	const char *current_token;
	int return_code;
	struct cmzn_command_data *command_data;

	ENTER(execute_command_quit);
	USE_PARAMETER(dummy_to_be_modified);
	/* check argument */
	if (state)
	{
		if (!((current_token=state->current_token)&&
			!(strcmp(PARSER_HELP_STRING,current_token)&&
			strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))))
		{
			if (NULL != (command_data = (struct cmzn_command_data *)command_data_void))
			{
				User_interface_end_application_loop(command_data->user_interface);
				command_data->start_event_dispatcher = false;
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"execute_command_quit.  Invalid command_data");
				return_code=0;
			}
		}
		else
		{
			/* no help */
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_command_quit.	Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_quit */

static int set_dir(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 17 May 2003

DESCRIPTION :
Executes a SET DIR command.
==============================================================================*/
{
	char *comfile_name, *directory_name, *example_directory, example_flag,
		*example_requirements;
	int file_name_length, return_code;
	struct cmzn_command_data *command_data;
	static struct Modifier_entry option_table[]=
	{
		{CMGUI_EXAMPLE_DIRECTORY_SYMBOL,const_cast<char *>(CMGUI_EXAMPLE_DIRECTORY_SYMBOL),
			NULL,set_char_flag},
		{NULL,NULL,NULL,set_name}
	};

	ENTER(set_dir);
	USE_PARAMETER(dummy_to_be_modified);
	/* check argument */
	if (state)
	{
		if (NULL != (command_data = (struct cmzn_command_data *)command_data_void))
		{
			if (state->current_token)
			{
				directory_name = (char *)NULL;
				example_flag = 0;
				(option_table[0]).to_be_modified = &example_flag;
				(option_table[1]).to_be_modified = &directory_name;
				return_code=process_multiple_options(state,option_table);
				if (return_code)
				{
					if (example_flag)
					{
						if (directory_name)
						{
							/* Lookup the example path */
							if (NULL != (example_directory =
								resolve_example_path(command_data->examples_directory,
								directory_name, &comfile_name, &example_requirements)))
							{
								if (command_data->example_directory)
								{
									DEALLOCATE(command_data->example_directory);
								}
								command_data->example_directory=example_directory;
								if (command_data->example_comfile)
								{
									DEALLOCATE(command_data->example_comfile);
								}
								if (comfile_name)
								{
									command_data->example_comfile = comfile_name;
								}
								else
								{
									command_data->example_comfile = (char *)NULL;
								}
								if (command_data->example_requirements)
								{
									DEALLOCATE(command_data->example_requirements);
								}
								if (example_requirements)
								{
									command_data->example_requirements =
										example_requirements;
								}
								else
								{
									 command_data->example_requirements = (char *)NULL;
								}
#if defined (USE_PERL_INTERPRETER)
								/* Set the interpreter variable */
								interpreter_set_string(command_data->interpreter, "example",
									example_directory, &return_code);
#endif /* defined (USE_PERL_INTERPRETER) */
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"set_dir.  Unable to resolve example path.");
								return_code = 0;
							}
						}
						else
						{
							file_name_length = 1;
							if (command_data->examples_directory)
							{
								file_name_length += strlen(command_data->examples_directory);
							}
							if (ALLOCATE(example_directory,char,file_name_length))
							{
								*example_directory='\0';
								if (command_data->examples_directory)
								{
									strcat(example_directory,command_data->examples_directory);
								}
								DEALLOCATE(command_data->example_directory);
								command_data->example_directory=example_directory;
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"set_dir.  Insufficient memory");
							}
						}
					}
					else
					{
						if(chdir(directory_name))
						{
							display_message(ERROR_MESSAGE,
								"set_dir.  Unable to change to directory %s",
								directory_name);
						}
						return_code = 1;
					}
				}
				if (directory_name)
				{
					DEALLOCATE(directory_name);
				}
			}
			else
			{
				set_command_prompt("set dir",command_data);
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"execute_command_set_dir.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_command_set_dir.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_dir */

static int execute_command_set(struct Parse_state *state,
	void *dummy_to_be_modified,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 16 October 1998

DESCRIPTION :
Executes a SET command.
==============================================================================*/
{
	int return_code;
	struct cmzn_command_data *command_data;
	struct Option_table *option_table;

	ENTER(execute_command_set);
	USE_PARAMETER(dummy_to_be_modified);
	/* check argument */
	if (state)
	{
		if (NULL != (command_data = (struct cmzn_command_data *)command_data_void))
		{
			if (state->current_token)
			{
				option_table = CREATE(Option_table)();
				/* directory */
				Option_table_add_entry(option_table, "directory", NULL,
					command_data_void, set_dir);
				return_code=Option_table_parse(option_table, state);
				DESTROY(Option_table)(&option_table);
			}
			else
			{
				set_command_prompt("set",command_data);
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"execute_command_set.  Missing command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_command_set.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_set */

static int execute_command_system(struct Parse_state *state,
	void *dummy_to_be_modified,void *dummy_user_data)
/*******************************************************************************
LAST MODIFIED : 19 February 1998

DESCRIPTION :
Executes a SET DIR #CMGUI_EXAMPLE_DIRECTORY_SYMBOL command.
???RC Obsolete?
==============================================================================*/
{
	char *command, *system_command;
	const char *current_token;
	int return_code;

	ENTER(execute_command_system);
	USE_PARAMETER(dummy_to_be_modified);
	USE_PARAMETER(dummy_user_data);
	return_code=0;
	/* check argument */
	if (state)
	{
		if (NULL != (current_token = state->current_token))
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				command=strstr(state->command_string,current_token);
				if (ALLOCATE(system_command,char,strlen(command)+1))
				{
					strcpy(system_command,command);
					parse_variable(&system_command);
					//system commands return 0 for no error
					return_code = !system(system_command);
					DEALLOCATE(system_command);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"execute_command_system.  Insufficient memory");
					return_code=0;
				}
			}
			else
			{
				/* no help */
				return_code=1;
			}
		}
		else
		{
			display_message(WARNING_MESSAGE,"Missing graphics object name");
			display_parse_state_location(state);
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"execute_command_system.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_system */

/*
Global functions
----------------
*/
#if defined (F90_INTERPRETER) || defined (USE_PERL_INTERPRETER)
void execute_command(const char *command_string,void *command_data_void, int *quit,
  int *error)
/*******************************************************************************
LAST MODIFIED : 17 July 2002

DESCRIPTION:
==============================================================================*/
{
	char **token;
	int i,return_code = 1;
	struct cmzn_command_data *command_data;
	struct Option_table *option_table;
	struct Parse_state *state;

	ENTER(execute_command);
	USE_PARAMETER(quit);
	if (NULL != (command_data = (struct cmzn_command_data *)command_data_void))
	{
		if (NULL != (state = create_Parse_state(command_string)))
			/*???DB.  create_Parse_state has to be extended */
		{
			i=state->number_of_tokens;
			/* check for comment */
			if (i>0)
			{
				/* check for a "<" as one of the of the tokens */
					/*???DB.  Include for backward compatability.  Remove ? */
				token=state->tokens;
				while ((i>0)&&strcmp(*token,"<"))
				{
					i--;
					token++;
				}
				if (i>0)
				{
					/* return to tree root */
					return_code=set_command_prompt("",command_data);
				}
				else
				{
					option_table = CREATE(Option_table)();
#if defined (SELECT_DESCRIPTORS)
					/* attach */
					Option_table_add_entry(option_table, "attach", NULL, command_data_void,
						execute_command_attach);
#endif /* !defined (SELECT_DESCRIPTORS) */
#if defined (WIN32_USER_INTERFACE) || defined (GTK_USER_INTERFACE)
					/* command_window */
					Option_table_add_entry(option_table, "command_window", NULL, command_data->command_window,
						modify_Command_window);
#endif /* defined (WIN32_USER_INTERFACE) || defined (GTK_USER_INTERFACE) */
#if defined (SELECT_DESCRIPTORS)
					/* detach */
					Option_table_add_entry(option_table, "detach", NULL, command_data_void,
						execute_command_detach);
#endif /* !defined (SELECT_DESCRIPTORS) */
					/* gfx */
					Option_table_add_entry(option_table, "gfx", NULL, command_data_void,
						execute_command_gfx);
					/* open */
					Option_table_add_entry(option_table, "open", NULL, command_data_void,
						execute_command_open);
					/* quit */
					Option_table_add_entry(option_table, "quit", NULL, command_data_void,
						execute_command_quit);
					/* list_memory */
					Option_table_add_entry(option_table, "list_memory", NULL, NULL,
						execute_command_list_memory);
					/* read */
					Option_table_add_entry(option_table, "read", NULL, command_data_void,
						execute_command_read);
					/* set */
					Option_table_add_entry(option_table, "set", NULL, command_data_void,
						execute_command_set);
					/* system */
					Option_table_add_entry(option_table, "system", NULL, command_data_void,
						execute_command_system);
					return_code=Option_table_parse(option_table, state);
					DESTROY(Option_table)(&option_table);
				}
				// Catching case where a fail returned code is returned but we are
				// asking for help, reseting the return code to pass if this is the case.
				if (!return_code && state->current_token &&
				   ((0==strcmp(PARSER_HELP_STRING,state->current_token)) ||
				   (0==strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token))))
				{
					return_code = 1;
				}

			}
			destroy_Parse_state(&state);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"cmiss_execute_command.  Could not create parse state");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmiss_execute_command.  Missing command_data");
		return_code=0;
	}

	*error = return_code;

	LEAVE;

} /* execute_command */

int cmiss_execute_command(const char *command_string,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 28 August 2000

DESCRIPTION:
Takes a <command_string>, processes this through the F90 interpreter
and then executes the returned strings
==============================================================================*/
{
	int quit,return_code;
	struct cmzn_command_data *command_data;

	ENTER(cmiss_execute_command);
	command_data = (struct cmzn_command_data *)NULL;
	if (NULL != (command_data = (struct cmzn_command_data *)command_data_void))
	{
#if defined (WIN32_USER_INTERFACE) || defined (GTK_USER_INTERFACE) || defined (WX_USER_INTERFACE)
		if (command_data->command_window)
		{
			add_to_command_list(command_string,command_data->command_window);
		}
#endif /* defined (WIN32_USER_INTERFACE) || defined (GTK_USER_INTERFACE) */
		quit = 0;

		interpret_command(command_data->interpreter, command_string, (void *)command_data, &quit, &execute_command, &return_code);

#if defined (WIN32_USER_INTERFACE) || defined (GTK_USER_INTERFACE) || defined (WX_USER_INTERFACE)
		if (command_data->command_window)
		{
			reset_command_box(command_data->command_window);
		}
#endif /* defined (WIN32_USER_INTERFACE) || defined (GTK_USER_INTERFACE) */

		if (quit)
		{
			Event_dispatcher_end_main_loop(command_data->event_dispatcher);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmiss_execute_command.  Missing command_data");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* cmiss_execute_command */
#else /* defined (F90_INTERPRETER) || defined (USE_PERL_INTERPRETER) */
int cmiss_execute_command(const char *command_string,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 17 July 2002

DESCRIPTION:
Execute a <command_string>. If there is a command
==============================================================================*/
{
	char **token;
	int i,return_code = 0;
	struct cmzn_command_data *command_data;
	struct Option_table *option_table;
	struct Parse_state *state;

	ENTER(cmiss_execute_command);
	if (NULL != (command_data = (struct cmzn_command_data *)command_data_void))
	{
		if (NULL != (state = create_Parse_state(command_string)))
			/*???DB.  create_Parse_state has to be extended */
		{
			i=state->number_of_tokens;
			/* check for comment */
			if (i>0)
			{
				/* add command to command history */
				/*???RC put out processed tokens instead? */
#if defined (WIN32_USER_INTERFACE) || defined (GTK_USER_INTERFACE) || defined (WX_USER_INTERFACE)
				if (command_data->command_window)
				{
					add_to_command_list(command_string,command_data->command_window);
				}
#endif /* defined (WIN32_USER_INTERFACE) || defined (GTK_USER_INTERFACE) */
				/* check for a "<" as one of the of the tokens */
					/*???DB.  Include for backward compatability.  Remove ? */
				token=state->tokens;
				while ((i>0)&&strcmp(*token,"<"))
				{
					i--;
					token++;
				}
				if (i>0)
				{
					/* return to tree root */
					return_code=set_command_prompt("", command_data);
				}
				else
				{
					option_table = CREATE(Option_table)();
#if defined (SELECT_DESCRIPTORS)
					/* attach */
					Option_table_add_entry(option_table, "attach", NULL, command_data_void,
						execute_command_attach);
#endif /* !defined (SELECT_DESCRIPTORS) */
#if defined (WIN32_USER_INTERFACE) || defined (GTK_USER_INTERFACE)
					/* command_window */
					Option_table_add_entry(option_table, "command_window", NULL, command_data->command_window,
						modify_Command_window);
#endif /* defined (WIN32_USER_INTERFACE) || defined (GTK_USER_INTERFACE) */
#if defined (SELECT_DESCRIPTORS)
					/* detach */
					Option_table_add_entry(option_table, "detach", NULL, command_data_void,
						execute_command_detach);
#endif /* !defined (SELECT_DESCRIPTORS) */
					/* gfx */
					Option_table_add_entry(option_table, "gfx", NULL, command_data_void,
						execute_command_gfx);
					/* open */
					Option_table_add_entry(option_table, "open", NULL, command_data_void,
						execute_command_open);
					/* quit */
					Option_table_add_entry(option_table, "quit", NULL, command_data_void,
						execute_command_quit);
					/* list_memory */
					Option_table_add_entry(option_table, "list_memory", NULL, NULL,
						execute_command_list_memory);
					/* read */
					Option_table_add_entry(option_table, "read", NULL, command_data_void,
						execute_command_read);
					/* set */
					Option_table_add_entry(option_table, "set", NULL, command_data_void,
						execute_command_set);
					/* system */
					Option_table_add_entry(option_table, "system", NULL, command_data_void,
						execute_command_system);
					return_code=Option_table_parse(option_table, state);
					DESTROY(Option_table)(&option_table);
				}
			}
#if defined (WIN32_USER_INTERFACE) || defined (GTK_USER_INTERFACE) || defined (WX_USER_INTERFACE)
			if (command_data->command_window)
			{
				reset_command_box(command_data->command_window);
			}
#endif /* defined (WIN32_USER_INTERFACE) || defined (GTK_USER_INTERFACE) */
			destroy_Parse_state(&state);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"cmiss_execute_command.  Could not create parse state");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmiss_execute_command.  Missing command_data");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* cmiss_execute_command */
#endif  /* defined (F90_INTERPRETER) || defined (USE_PERL_INTERPRETER) */

int cmiss_set_command(const char *command_string,void *command_data_void)
/*******************************************************************************
LAST MODIFIED : 17 May 2003

DESCRIPTION:
Sets the <command_string> in the command box of the cmgui command_window, ready
for editing and entering. If there is no command_window, does nothing.
==============================================================================*/
{
	int return_code;
#if defined (WX_USER_INTERFACE)
	struct cmzn_command_data *command_data;
#endif /* defined (WX_USER_INTERFACE) */

	ENTER(cmiss_set_command);
	if (command_string
#if defined (WX_USER_INTERFACE)
		&& (command_data=(struct cmzn_command_data *)command_data_void)
#endif /* defined (WX_USER_INTERFACE) */
			)
	{
#if defined (WX_USER_INTERFACE)
		if (command_data->command_window)
		{
			return_code=Command_window_set_command_string(
				command_data->command_window,command_string);
		}
#else
		USE_PARAMETER(command_data_void);
#endif /* defined (WX_USER_INTERFACE) */
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"cmiss_set_command.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* cmiss_set_command */

#if defined(USE_CMGUI_COMMAND_WINDOW)

static void display_command_window_message(cmzn_loggerevent_id event,
	void *command_window_void)
{
	if (event)
	{
		char *message = cmzn_loggerevent_get_message_text(event);
		cmzn_logger_message_type message_type = cmzn_loggerevent_get_message_type(event);
		switch (message_type)
		{
			case CMZN_LOGGER_MESSAGE_TYPE_ERROR:
			{
				if (command_window_void)
				{
					write_command_window("ERROR: ",
						(struct Command_window *)command_window_void);
					write_command_window(message,
						(struct Command_window *)command_window_void);
					write_command_window("\n",(struct Command_window *)command_window_void);
				}
				else
				{
					printf("ERROR: %s\n",message);
				}
			} break;
			case CMZN_LOGGER_MESSAGE_TYPE_INFORMATION:
			{
				if (command_window_void)
				{
					write_command_window(message,	(struct Command_window *)command_window_void);
				}
				else
				{
					printf("%s", message);
				}
			} break;
			case CMZN_LOGGER_MESSAGE_TYPE_WARNING:
			{
				if (command_window_void)
				{
					write_command_window("WARNING: ",
						(struct Command_window *)command_window_void);
					write_command_window(message,
						(struct Command_window *)command_window_void);
					write_command_window("\n",(struct Command_window *)command_window_void);
				}
				else
				{
					printf("WARNING: %s\n",message);
				}
			} break;
			default:
			{
			} break;
		}
		if (message)
			DEALLOCATE(message);
	}
}
#endif /* defined(USE_CMGUI_COMMAND_WINDOW) */

static int cmgui_execute_comfile(const char *comfile_name,const char *example_id,
	const char *examples_directory,const char *example_symbol,char **example_comfile_name,
	struct Execute_command *execute_command)
/*******************************************************************************
LAST MODIFIED : 16 October 1998

DESCRIPTION :
Executes the comfile specified on the command line.
==============================================================================*/
{
	int return_code;
	char global_temp_string[1000];

	ENTER(cmgui_execute_comfile);
	return_code=0;
	if ((comfile_name||example_id)&&execute_command)
	{
		if (example_id)
		{
			if (examples_directory&&example_symbol)
			{
				/* set the examples directory */
				sprintf(global_temp_string,"set dir ");
				strcat(global_temp_string,example_symbol);
				strcat(global_temp_string,"=");
				strcat(global_temp_string,example_id);
				Execute_command_execute_string(execute_command,global_temp_string);
				sprintf(global_temp_string,"open comfile ");
				if (comfile_name)
				{
					strcat(global_temp_string,comfile_name);
				}
				else
				{
					if (*example_comfile_name)
					{
						strcat(global_temp_string,*example_comfile_name);
					}
					else
					{
						strcat(global_temp_string,"example_");
						strcat(global_temp_string,example_id);
					}
				}
				strcat(global_temp_string,";");
				strcat(global_temp_string,example_symbol);
				strcat(global_temp_string," execute");
				return_code=Execute_command_execute_string(execute_command,global_temp_string);
			}
			else
			{
				display_message(ERROR_MESSAGE,
				"cmgui_execute_comfile.  Missing examples_directory or example_symbol");
			}
		}
		else
		{
			/* open the command line comfile */
			sprintf(global_temp_string,"open comfile ");
			strcat(global_temp_string,comfile_name);
			strcat(global_temp_string," execute");
			return_code=Execute_command_execute_string(execute_command, global_temp_string);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmgui_execute_comfile.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* cmgui_execute_comfile */

int set_string_no_command_line_option(struct Parse_state *state,
	void *string_address_void, void *string_description_void)
/*******************************************************************************
LAST MODIFIED : 6 August 2002

DESCRIPTION :
Calls set_string unless the first character of the current token is a hyphen.
Used to avoid parsing possible command line switches.
==============================================================================*/
{
	const char *current_token;
	int return_code;

	ENTER(set_string_no_command_line_option);
	if (state)
	{
		if (NULL != (current_token = state->current_token))
		{
			if ('-' != current_token[0])
			{
				return_code = set_string(state, string_address_void,
					string_description_void);
			}
			else
			{
				display_message(ERROR_MESSAGE, "Invalid command line option \"%s\"",
					current_token);
				display_parse_state_location(state);
				return_code = 0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE, "Missing string");
			display_parse_state_location(state);
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_string_no_command_line_option.  Missing state");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* set_string_no_command_line_option */

int ignore_entry_and_next_token(struct Parse_state *state,
	void *dummy_void, void *entry_description_void)
/*******************************************************************************
LAST MODIFIED : 8 August 2002

DESCRIPTION :
Used to consume and write help for command line parameters handled outside
of this parse state routine.
==============================================================================*/
{
	const char *current_token;
	int return_code;

	ENTER(ignore_entry_and_next_token);
	USE_PARAMETER(dummy_void);
	if (state)
	{
		if (NULL != (current_token = state->current_token))
		{
			if (strcmp(PARSER_HELP_STRING, current_token) &&
				strcmp(PARSER_RECURSIVE_HELP_STRING, current_token))
			{
				return_code = shift_Parse_state(state,1);
			}
			else
			{
				display_message(INFORMATION_MESSAGE, (char *)entry_description_void);
				return_code = 1;
			}

		}
		else
		{
			display_message(ERROR_MESSAGE, "Missing string");
			display_parse_state_location(state);
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"ignore_entry_and_next_token.  Missing state");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* ignore_entry_and_next_token */

static int read_cmgui_command_line_options(struct Parse_state *state,
	void *dummy_to_be_modified, void *cmgui_command_line_options_void)
/*******************************************************************************
LAST MODIFIED : 5 August 2002

DESCRIPTION :
Parses command line options from <state>.
==============================================================================*/
{
	int return_code;
	struct Cmgui_command_line_options *command_line_options;
	struct Option_table *option_table;

	ENTER(read_cmgui_command_line_options);
	USE_PARAMETER(dummy_to_be_modified);
	if (state && (command_line_options =
		(struct Cmgui_command_line_options *)cmgui_command_line_options_void))
	{
		option_table = CREATE(Option_table)();
		/* -batch */
		Option_table_add_entry(option_table, "-batch",
			&(command_line_options->batch_mode_flag), NULL, set_char_flag);
		/* -cm */
		Option_table_add_entry(option_table, "-cm",
			&(command_line_options->cm_start_flag), NULL, set_char_flag);
		/* -cm_epath */
		Option_table_add_entry(option_table, "-cm_epath",
			&(command_line_options->cm_epath_directory_name),
			(void *)" PATH_TO_EXAMPLES_DIRECTORY", set_string);
		/* -cm_parameters */
		Option_table_add_entry(option_table, "-cm_parameters",
			&(command_line_options->cm_parameters_file_name),
			(void *)" PARAMETER_FILE_NAME", set_string);
		/* -command_list */
		Option_table_add_entry(option_table, "-command_list",
			&(command_line_options->command_list_flag), NULL, set_char_flag);
		/* -console */
		Option_table_add_entry(option_table, "-console",
			&(command_line_options->console_mode_flag), NULL, set_char_flag);
#if defined (GTK_USER_INTERFACE) || defined (__WXGTK__)
		/* --display, support the gtk convention for this tool */
		Option_table_add_entry(option_table, "--display", NULL,
			(void *)" X11_DISPLAY_NUMBER", ignore_entry_and_next_token);
#endif /* defined (GTK_USER_INTERFACE) || defined (__WXGTK__) */
		/* -epath */
		Option_table_add_entry(option_table, "-epath",
			&(command_line_options->epath_directory_name),
			(void *)" PATH_TO_EXAMPLES_DIRECTORY", set_string);
		/* -example */
		Option_table_add_entry(option_table, "-example",
			&(command_line_options->example_file_name),
			(void *)" EXAMPLE_ID", set_string);
		/* -execute */
		Option_table_add_entry(option_table, "-execute",
			&(command_line_options->execute_string),
			(void *)" EXECUTE_STRING", set_string);
		/* -help */
		Option_table_add_entry(option_table, "-help",
			&(command_line_options->write_help_flag), NULL, set_char_flag);
		/* -id */
		Option_table_add_entry(option_table, "-id",
			&(command_line_options->id_name), (void *)" ID", set_string);
		/* -mycm */
		Option_table_add_entry(option_table, "-mycm",
			&(command_line_options->mycm_start_flag), NULL, set_char_flag);
		/* -no_display */
		Option_table_add_entry(option_table, "-no_display",
			&(command_line_options->no_display_flag), NULL, set_char_flag);
		/* -random */
		Option_table_add_entry(option_table, "-random",
			&(command_line_options->random_number_seed),
			(void *)" NUMBER_SEED", set_int_with_description);
		/* -server */
		Option_table_add_entry(option_table, "-server",
			&(command_line_options->server_mode_flag), NULL, set_char_flag);
#if defined (CARBON_USER_INTERFACE) || (defined (WX_USER_INTERFACE) && defined (DARWIN))
		/* -psn */
		Option_table_add_entry(option_table, "-psn", NULL, NULL, ignore_entry);
#endif
		/* -visual */
		Option_table_add_entry(option_table, "-visual",
			&(command_line_options->visual_id_number),
			(void *)" NUMBER", set_int_with_description);
		/* [default option == command_file_name] */
		Option_table_add_entry(option_table, (const char *)NULL,
			&(command_line_options->command_file_name),
			(void *)"COMMAND_FILE_NAME", set_string_no_command_line_option);
		return_code = Option_table_multi_parse(option_table, state);
		DESTROY(Option_table)(&option_table);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"read_cmgui_command_line_options.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* read_cmgui_command_line_options */

int cmzn_command_data_process_command_line(int argc, char *argv[],
	struct Cmgui_command_line_options *command_line_options)
{
	int return_code = 1;
	struct Option_table *option_table = NULL;
	struct Parse_state *state = NULL;

	/* put command line options into structure for parsing & extract below */
	command_line_options->batch_mode_flag = (char)0;
	command_line_options->cm_start_flag = (char)0;
	command_line_options->cm_epath_directory_name = NULL;
	command_line_options->cm_parameters_file_name = NULL;
	command_line_options->command_list_flag = (char)0;
	command_line_options->console_mode_flag = (char)0;
	command_line_options->epath_directory_name = NULL;
	command_line_options->example_file_name = NULL;
	command_line_options->execute_string = NULL;
	command_line_options->write_help_flag = (char)0;
	command_line_options->id_name = NULL;
	command_line_options->mycm_start_flag = (char)0;
	command_line_options->no_display_flag = (char)0;
	command_line_options->random_number_seed = -1;
	command_line_options->server_mode_flag = (char)0;
	command_line_options->visual_id_number = 0;
	command_line_options->command_file_name = NULL;

	if (argc >  0 && argv != NULL)
	{
		if (NULL != (state = create_Parse_state_from_tokens(argc, argv)))
		{
			option_table = CREATE(Option_table)();
			Option_table_add_entry(option_table, argv[0], NULL,
				(void *)command_line_options, read_cmgui_command_line_options);
			if (!Option_table_parse(option_table, state))
			{
				command_line_options->write_help_flag = (char)1;
				return_code = 0;
			}
			DESTROY(Option_table)(&option_table);
			destroy_Parse_state(&state);
		}
		else
		{
			return_code = 0;
		}
	}

	return return_code;
}

struct cmzn_command_data *CREATE(cmzn_command_data)(struct cmzn_context_app *context,
	struct User_interface_module *UI_module)
/*******************************************************************************
LAST MODIFIED : 3 April 2003

DESCRIPTION :
Initialise all the subcomponents of cmgui and create the cmzn_command_data
==============================================================================*/
{
	char *cm_examples_directory,*cm_parameters_file_name,*comfile_name,
		*example_id,*examples_directory,*examples_environment,*execute_string,
		*version_command_id;
	char global_temp_string[1000];
	int return_code;
	int batch_mode, console_mode, command_list, no_display, non_random,
		server_mode, start_cm, start_mycm, visual_id, write_help;
#if defined (F90_INTERPRETER) || defined (USE_PERL_INTERPRETER)
	int status;
#endif /* defined (F90_INTERPRETER) || defined (USE_PERL_INTERPRETER) */
	struct Cmgui_command_line_options command_line_options;
	struct cmzn_command_data *command_data;
#if defined(USE_CMGUI_COMMAND_WINDOW)
	struct Command_window *command_window;
#endif /* defined(USE_CMGUI_COMMAND_WINDOW) */
	struct Option_table *option_table;
	struct Parse_state *state;
	User_settings user_settings;
#if defined (WIN32_USER_INTERFACE)
	ENTER(WinMain);
#endif /* defined (WIN32_USER_INTERFACE) */
	return_code = 1;

	if (ALLOCATE(command_data, struct cmzn_command_data, 1))
	{
		command_data->access_count = 1;
		// duplicate argument list so it can be modified by User Interface
		/* initialize application specific global variables */
		command_data->execute_command = CREATE(Execute_command)();;
		command_data->set_command = CREATE(Execute_command)();
		command_data->event_dispatcher = (struct Event_dispatcher *)NULL;
		command_data->user_interface= (struct User_interface *)NULL;
		command_data->logger = 0;
		command_data->loggerNotifier = 0;
#if defined (WX_USER_INTERFACE)
		command_data->data_viewer=(struct Node_viewer *)NULL;
		command_data->node_viewer=(struct Node_viewer *)NULL;
		command_data->element_point_viewer=(struct Element_point_viewer *)NULL;
#endif /* defined (WX_USER_INTERFACE) */
#if defined (WX_USER_INTERFACE)
		command_data->material_editor = (struct Material_editor *)NULL;
		command_data->region_tree_viewer = (struct Region_tree_viewer *)NULL;
		command_data->spectrum_editor_dialog = (struct Spectrum_editor_dialog *)NULL;
#endif /*defined (WX_USER_INTERFACE) */
		command_data->command_console = (struct Console *)NULL;
		command_data->example_directory=(char *)NULL;

#if defined (WX_USER_INTERFACE)
		command_data->comfile_window_manager=(struct MANAGER(Comfile_window) *)NULL;
#endif /* defined WX_USER_INTERFACE*/
		command_data->default_light=(struct cmzn_light *)NULL;
		command_data->lightmodule=NULL;
		command_data->environment_map_manager=(struct MANAGER(Environment_map) *)NULL;
		command_data->volume_texture_manager=(struct MANAGER(VT_volume_texture) *)NULL;
		command_data->default_spectrum=(struct cmzn_spectrum *)NULL;
		command_data->spectrum_manager=(struct MANAGER(cmzn_spectrum) *)NULL;
		command_data->graphics_buffer_package=(struct Graphics_buffer_app_package *)NULL;
		command_data->sceneviewermodule=(struct cmzn_sceneviewermodule_app *)NULL;
		command_data->graphics_module = (struct cmzn_graphics_module *)NULL;
#if defined (USE_CMGUI_GRAPHICS_WINDOW)
		command_data->graphics_window_manager=(struct MANAGER(Graphics_window) *)NULL;
#endif /* defined (USE_CMGUI_GRAPHICS_WINDOW) */
		command_data->root_region = (struct cmzn_region *)NULL;
		command_data->basis_manager=(struct MANAGER(FE_basis) *)NULL;
		command_data->streampoint_list=(struct Streampoint *)NULL;
#if defined (SELECT_DESCRIPTORS)
		command_data->device_list=(struct LIST(Io_device) *)NULL;
#endif /* defined (SELECT_DESCRIPTORS) */
		command_data->glyphmodule=(cmzn_glyphmodule_id)0;
		command_data->element_point_ranges_selection=(struct Element_point_ranges_selection *)NULL;
		command_data->interactive_tool_manager=(struct MANAGER(Interactive_tool) *)NULL;
		command_data->io_stream_package = (struct IO_stream_package *)NULL;
		command_data->computed_field_package=(struct Computed_field_package *)NULL;
		command_data->default_scene=(struct Scene *)NULL;
		command_data->scene_manager=(struct MANAGER(Scene) *)NULL;
#if defined (WIN32_USER_INTERFACE) || defined (GTK_USER_INTERFACE) || defined (WX_USER_INTERFACE)
		command_data->command_window=(struct Command_window *)NULL;
#endif /* defined (WIN32_USER_INTERFACE) || defined (GTK_USER_INTERFACE) */
		command_data->transform_tool=(struct Interactive_tool *)NULL;
		command_data->node_tool=(struct Node_tool *)NULL;
		command_data->element_tool=(struct Element_tool *)NULL;
#if defined (USE_OPENCASCADE)
		command_data->cad_tool = (struct Cad_tool *)NULL;
#endif /* defined (USE_OPENCASCADE) */
		command_data->data_tool=(struct Node_tool *)NULL;
		command_data->element_point_tool=(struct Element_point_tool *)NULL;
		command_data->examples_directory=(char *)NULL;
		command_data->example_comfile=(char *)NULL;
		command_data->example_requirements=(char *)NULL;
		command_data->cm_examples_directory=(char *)NULL;
		command_data->cm_parameters_file_name=(char *)NULL;
		command_data->default_time_keeper_app = (struct Time_keeper_app *)NULL;
		command_data->help_directory=(char *)NULL;
		command_data->help_url=(char *)NULL;
		command_data->start_event_dispatcher=true;
#if defined (USE_PERL_INTERPRETER)
		command_data->interpreter = (struct Interpreter *)NULL;
#endif /* defined (USE_PERL_INTERPRETER) */

		/* set default values for command-line modifiable options */
		/* Note User_interface will not be created if command_list selected */
		batch_mode = 0;
		command_list = 0;
		console_mode = 0;
		no_display = 0;
		server_mode = 0;
		visual_id = 0;
		write_help = 0;
		/* flag to say randomise */
		non_random = -1;
		/* flag for starting cm */
		start_cm = 0;
		/* flag for starting mycm */
		start_mycm = 0;
		/* to over-ride all other example directory settings */
		examples_directory = (char *)NULL;
		/* back-end examples directory */
		cm_examples_directory = (char *)NULL;
		/* back-end parameters file */
		cm_parameters_file_name = (char *)NULL;
		/* the comfile is in the examples directory */
		example_id = (char *)NULL;
		/* a string executed by the interpreter before loading any comfiles */
		execute_string = (char *)NULL;
		/* set no command id supplied */
		version_command_id = (char *)NULL;
		/* the name of the comfile to be run on startup */
		comfile_name = (char *)NULL;

		user_settings.examples_directory = (char *)NULL;
		user_settings.help_directory = (char *)NULL;
		user_settings.help_url = (char *)NULL;
		user_settings.startup_comfile = (char *)NULL;

		/* parse commmand line options */

		/* put command line options into structure for parsing & extract below */
		command_line_options.batch_mode_flag = (char)batch_mode;
		command_line_options.cm_start_flag = (char)start_cm;
		command_line_options.cm_epath_directory_name = cm_examples_directory;
		command_line_options.cm_parameters_file_name = cm_parameters_file_name;
		command_line_options.command_list_flag = (char)command_list;
		command_line_options.console_mode_flag = (char)console_mode;
		command_line_options.epath_directory_name = examples_directory;
		command_line_options.example_file_name = example_id;
		command_line_options.execute_string = execute_string;
		command_line_options.write_help_flag = (char)write_help;
		command_line_options.id_name = version_command_id;
		command_line_options.mycm_start_flag = (char)start_mycm;
		command_line_options.no_display_flag = (char)no_display;
		command_line_options.random_number_seed = non_random;
		command_line_options.server_mode_flag = (char)server_mode;
		command_line_options.visual_id_number = visual_id;
		command_line_options.command_file_name = comfile_name;

		if (UI_module->argc > 0 && UI_module->argv)
		{
			if (NULL != (state = create_Parse_state_from_tokens(UI_module->argc, UI_module->argv)))
			{
				option_table = CREATE(Option_table)();
				Option_table_add_entry(option_table, UI_module->argv[0], NULL,
				(void *)&command_line_options, read_cmgui_command_line_options);
				if (!Option_table_parse(option_table, state))
				{
					write_help = 1;
					return_code = 0;
				}
				DESTROY(Option_table)(&option_table);
				destroy_Parse_state(&state);
			}
			else
			{
				return_code = 0;
			}
		}
		/* copy command line options to local vars for use and easy clean-up */
		batch_mode = (int)command_line_options.batch_mode_flag;
		start_cm = command_line_options.cm_start_flag;
		cm_examples_directory = command_line_options.cm_epath_directory_name;
		cm_parameters_file_name = command_line_options.cm_parameters_file_name;
		command_list = command_line_options.command_list_flag;
		console_mode = command_line_options.console_mode_flag;
		examples_directory = command_line_options.epath_directory_name;
		example_id = command_line_options.example_file_name;
		execute_string = command_line_options.execute_string;
		write_help = command_line_options.write_help_flag;
		version_command_id = command_line_options.id_name;
		start_mycm = command_line_options.mycm_start_flag;
		no_display = command_line_options.no_display_flag;
		non_random = command_line_options.random_number_seed;
		server_mode = (int)command_line_options.server_mode_flag;
		visual_id = command_line_options.visual_id_number;
		comfile_name = command_line_options.command_file_name;
		if (write_help)
		{
			char *double_question_mark = new char[3];
			strncpy(double_question_mark, "??", 2);
			double_question_mark[2] = '\0';

			/* write question mark help for command line options */
			state = create_Parse_state_from_tokens(1, &double_question_mark);
			option_table = CREATE(Option_table)();
			Option_table_add_entry(option_table,UI_module->argv[0], NULL,
				(void *)&command_line_options, read_cmgui_command_line_options);
			Option_table_parse(option_table, state);
			DESTROY(Option_table)(&option_table);
			destroy_Parse_state(&state);
			delete double_question_mark;
		}

		command_data->io_stream_package = cmzn_context_get_default_IO_stream_package(cmzn_context_app_get_core_context(context));

#if defined (F90_INTERPRETER) || defined (USE_PERL_INTERPRETER)
		/* SAB I want to do this before CREATEing the User_interface
			as X modifies the argc, argv removing the options it understands
			however I want a full copy for the interpreter so that we can use
			-display for example for both */
		create_interpreter(UI_module->argc, UI_module->unmodified_argv, comfile_name, &command_data->interpreter, &status);

		if (!status)
		{
			return_code=0;
		}

		interpreter_set_display_message_function(command_data->interpreter, display_message, &status);

		/* SAB Set a useful default for the interpreter variable, need to
			specify full name as this function does not run embedded by
			a package directive */
		interpreter_set_string(command_data->interpreter, "cmiss::example", ".", &status);

		/* SAB Set the cmgui command data into the interpreter.  The cmzn package
			is then able to export this when it is called from inside cmgui or
			when called directly from perl to load the appropriate libraries to
			create a cmgui externally. */
		interpreter_set_pointer(command_data->interpreter, "cmzn::cmzn_context",
			"cmzn::cmzn_context", context, &status);

#endif /* defined (F90_INTERPRETER) || defined (USE_PERL_INTERPRETER) */

		if ((!command_list) && (!write_help))
		{
			if (NULL != (command_data->event_dispatcher =
					cmzn_context_app_get_default_event_dispatcher(context)))
			{
				if (!no_display)
				{
					if (NULL == (command_data->user_interface = UI_module->user_interface))
					{
						return_code=0;
					}
				}
			}
			else
			{
				return_code = 0;
			}
		}

		/* use command line options in preference to defaults read from XResources */

		if (examples_directory)
		{
			command_data->examples_directory = examples_directory;
		}
		else if (NULL != (examples_environment = getenv("CMGUI_EXAMPLES")))
		{
			command_data->examples_directory = duplicate_string(examples_environment);
		}
		else
		{
			command_data->examples_directory = (char *)NULL;
		}
#if defined (WIN32_SYSTEM)
		/* We don't know about cygdrive as we are using the win32 api,
		   but try and interpret the variable anyway.  We can't handle
			other cygwin paths unless we call out to cygpath. */
		if (command_data->examples_directory
			&& (strlen(command_data->examples_directory) > 11)
			&& (!strncmp(command_data->examples_directory, "/cygdrive", 9)))
		{
			char *new_examples_string;
			ALLOCATE(new_examples_string, char,
				strlen(command_data->examples_directory) + 10);
			new_examples_string[0] = command_data->examples_directory[10];
		   new_examples_string[1] = ':';
		   new_examples_string[2] = '\\';
		   strcpy(new_examples_string + 3, command_data->examples_directory + 12);
		   DEALLOCATE(command_data->examples_directory);
		   command_data->examples_directory = new_examples_string;
		}
#endif /* defined (WIN32_SYSTEM) */
		command_data->cm_examples_directory = cm_examples_directory;
		command_data->cm_parameters_file_name = cm_parameters_file_name;
		command_data->help_directory = user_settings.help_directory;
		command_data->help_url = user_settings.help_url;

		/* create the managers */

#if defined (WX_USER_INTERFACE)
		/* comfile window manager */
		command_data->comfile_window_manager = UI_module->comfile_window_manager;
#endif /* defined (WX_USER_INTERFACE) */
		command_data->graphics_module =
			cmzn_context_get_graphics_module(cmzn_context_app_get_core_context(context));
		/* light manager */
		command_data->lightmodule=
			cmzn_graphics_module_get_lightmodule(command_data->graphics_module);
		command_data->default_light=
			cmzn_lightmodule_get_default_light(command_data->lightmodule);

		command_data->logger = cmzn_context_get_logger(
			cmzn_context_app_get_core_context(context));

		// ensure we have default tessellations
		command_data->tessellationmodule = cmzn_graphics_module_get_tessellationmodule(command_data->graphics_module);
		cmzn_tessellation_id default_tessellation = cmzn_tessellationmodule_get_default_tessellation(command_data->tessellationmodule);
		cmzn_tessellation_destroy(&default_tessellation);
		cmzn_tessellation_id default_points_tessellation = cmzn_tessellationmodule_get_default_points_tessellation(command_data->tessellationmodule);
		cmzn_tessellation_destroy(&default_points_tessellation);

		/* environment map manager */
		command_data->environment_map_manager=CREATE(MANAGER(Environment_map))();
		/* volume texture manager */
		command_data->volume_texture_manager=CREATE(MANAGER(VT_volume_texture))();
		/* spectrum manager */
		if (NULL != (command_data->spectrum_manager=
				cmzn_graphics_module_get_spectrum_manager(
					command_data->graphics_module)))
		{
			cmzn_spectrummodule_id spectrummodule =
				cmzn_graphics_module_get_spectrummodule(command_data->graphics_module);
			command_data->default_spectrum =
					cmzn_spectrummodule_get_default_spectrum(spectrummodule);
			cmzn_spectrummodule_destroy(&spectrummodule);
		}
		/* create Material module and CMGUI default materials */
		command_data->materialmodule = cmzn_graphics_module_get_materialmodule(command_data->graphics_module);
		if (command_data->materialmodule)
		{
			cmzn_materialmodule_define_standard_materials(
				command_data->materialmodule);
			cmzn_material_id material = cmzn_materialmodule_create_material(
				command_data->materialmodule);
			cmzn_material_set_name(material, "gray50");
			double material_colour[3] = { 0.50, 0.50, 0.50};
			cmzn_material_set_attribute_real3(material,
				CMZN_MATERIAL_ATTRIBUTE_AMBIENT, &material_colour[0]);
			cmzn_material_set_attribute_real3(material,
				CMZN_MATERIAL_ATTRIBUTE_DIFFUSE, &material_colour[0]);
			cmzn_material_set_attribute_real3(material,
				CMZN_MATERIAL_ATTRIBUTE_EMISSION, &material_colour[0]);
			cmzn_material_set_attribute_real3(material,
				CMZN_MATERIAL_ATTRIBUTE_SPECULAR, &material_colour[0]);
			cmzn_material_set_attribute_real(material,
				CMZN_MATERIAL_ATTRIBUTE_ALPHA, 1.0);
			cmzn_material_set_attribute_real(material,
				CMZN_MATERIAL_ATTRIBUTE_SHININESS, 0.2);
			cmzn_material_set_managed(material, true);
			cmzn_material_destroy(&material);
			material = cmzn_materialmodule_create_material(
				command_data->materialmodule);
			cmzn_material_set_name(material, "transparent_gray50");
			cmzn_material_set_attribute_real3(material,
				CMZN_MATERIAL_ATTRIBUTE_AMBIENT, &material_colour[0]);
			cmzn_material_set_attribute_real3(material,
				CMZN_MATERIAL_ATTRIBUTE_DIFFUSE, &material_colour[0]);
			cmzn_material_set_attribute_real3(material,
				CMZN_MATERIAL_ATTRIBUTE_EMISSION, &material_colour[0]);
			cmzn_material_set_attribute_real3(material,
				CMZN_MATERIAL_ATTRIBUTE_SPECULAR, &material_colour[0]);
			cmzn_material_set_attribute_real(material,
				CMZN_MATERIAL_ATTRIBUTE_ALPHA, 0.0);
			cmzn_material_set_attribute_real(material,
				CMZN_MATERIAL_ATTRIBUTE_SHININESS, 0.2);
			cmzn_material_set_managed(material, true);
			cmzn_material_destroy(&material);
		}
		command_data->filter_module =
				cmzn_graphics_module_get_scenefiltermodule(command_data->graphics_module);
		command_data->default_font = cmzn_graphics_module_get_default_font(
			command_data->graphics_module);

		command_data->glyphmodule = cmzn_graphics_module_get_glyphmodule(command_data->graphics_module);
		cmzn_glyphmodule_define_standard_glyphs(command_data->glyphmodule);
		cmzn_glyphmodule_define_standard_cmgui_glyphs(command_data->glyphmodule);

#if defined (USE_CMGUI_GRAPHICS_WINDOW)
		command_data->graphics_buffer_package = UI_module->graphics_buffer_package;
		/* graphics window manager.  Note there is no default window. */
		command_data->graphics_window_manager = UI_module->graphics_window_manager;
#endif /* defined (USE_CMGUI_GRAPHICS_WINDOW) */
		/* FE_element_shape manager */
		/*???DB.  To be done */
		command_data->element_shape_list=CREATE(LIST(FE_element_shape))();

		command_data->basis_manager=CREATE(MANAGER(FE_basis))();

		command_data->root_region = cmzn_context_get_default_region(cmzn_context_app_get_core_context(context));

#if defined (SELECT_DESCRIPTORS)
		/* create device list */
		/*SAB.  Eventually want device manager */
		command_data->device_list=CREATE(LIST(Io_device))();
#endif /* defined (SELECT_DESCRIPTORS) */
		/* global list of selected objects */
		command_data->element_point_ranges_selection =
			cmzn_context_get_element_point_ranges_selection(cmzn_context_app_get_core_context(context));

		/* interactive_tool manager */
		command_data->interactive_tool_manager=UI_module->interactive_tool_manager;
		/* computed field manager and default computed fields zero, xi,
			default_coordinate, etc. */
		/*???RC should the default computed fields be established in
		  CREATE(Computed_field_package)? */

		/*???GRC will eventually remove manager from field package so it is
		  purely type-specific data. Field manager is now owned by region.
		  Temporarily passing it to package to keep existing code running */
		struct MANAGER(Computed_field) *computed_field_manager=
			cmzn_region_get_Computed_field_manager(command_data->root_region);
		command_data->computed_field_package =
			CREATE(Computed_field_package)(computed_field_manager);
		/* Add Computed_fields to the Computed_field_package */
		if (command_data->computed_field_package)
		{
			Computed_field_register_types_coordinate(
				command_data->computed_field_package);
			if (command_data->root_region)
			{
				Computed_field_register_type_alias(
					command_data->computed_field_package,
					command_data->root_region);
			}
			Computed_field_register_types_arithmetic_operators(
				command_data->computed_field_package);
			Computed_field_register_types_trigonometry(
				command_data->computed_field_package);
			Computed_field_register_types_format_output(
				command_data->computed_field_package);
			if (command_data->root_region)
			{
				Computed_field_register_types_compose(
					command_data->computed_field_package,
					command_data->root_region);
			}
			Computed_field_register_types_composite(
				command_data->computed_field_package);
			Computed_field_register_types_conditional(
				command_data->computed_field_package);
#if defined (ZINC_USE_ITK)
			Computed_field_register_types_derivatives(
				command_data->computed_field_package);
#endif /* defined (ZINC_USE_ITK) */
			Computed_field_register_types_fibres(
				command_data->computed_field_package);
			Computed_field_register_types_function(
					command_data->computed_field_package);
			Computed_field_register_types_logical_operators(
				command_data->computed_field_package);
			if (command_data->root_region)
			{
				Computed_field_register_types_lookup(
					command_data->computed_field_package,
					command_data->root_region);
			}
			Computed_field_register_types_matrix_operators(
				command_data->computed_field_package);
			Computed_field_register_types_mesh_operators(
				command_data->computed_field_package);
			Computed_field_register_types_nodeset_operators(
				command_data->computed_field_package);
			Computed_field_register_types_vector_operators(
				command_data->computed_field_package);
#if defined (USE_CMGUI_GRAPHICS_WINDOW)
			if (command_data->graphics_window_manager)
			{
				Computed_field_register_types_sceneviewer_projection(
					command_data->computed_field_package,
					command_data->graphics_window_manager);
			}
#endif /* defined (USE_CMGUI_GRAPHICS_WINDOW) */
			Computed_field_register_types_image(
				command_data->computed_field_package);
			if (command_data->root_region)
			{
				Computed_field_register_types_integration(
					command_data->computed_field_package,
					command_data->root_region);
			}
			Computed_field_register_types_finite_element(
				command_data->computed_field_package);
			Computed_field_register_types_deformation(
				command_data->computed_field_package);
			Computed_field_register_types_string_constant(
				command_data->computed_field_package);

			Computed_field_register_types_image_resample(
				command_data->computed_field_package);
#if defined (ZINC_USE_ITK)
			Computed_field_register_types_threshold_image_filter(
				command_data->computed_field_package);
			Computed_field_register_types_binary_threshold_image_filter(
				command_data->computed_field_package);
			Computed_field_register_types_canny_edge_detection_image_filter(
				command_data->computed_field_package);
			Computed_field_register_types_mean_image_filter(
				command_data->computed_field_package);
			Computed_field_register_types_sigmoid_image_filter(
				command_data->computed_field_package);
			Computed_field_register_types_discrete_gaussian_image_filter(
				command_data->computed_field_package);
			Computed_field_register_types_histogram_image_filter(
				command_data->computed_field_package);
			Computed_field_register_types_curvature_anisotropic_diffusion_image_filter(
				command_data->computed_field_package);
			Computed_field_register_types_derivative_image_filter(
				command_data->computed_field_package);
			Computed_field_register_types_rescale_intensity_image_filter(
				command_data->computed_field_package);
			Computed_field_register_types_connected_threshold_image_filter(
				command_data->computed_field_package);
			Computed_field_register_types_gradient_magnitude_recursive_gaussian_image_filter(
				command_data->computed_field_package);
			Computed_field_register_types_fast_marching_image_filter(
				command_data->computed_field_package);
			Computed_field_register_types_binary_dilate_image_filter(
				command_data->computed_field_package);
			Computed_field_register_types_binary_erode_image_filter(
				command_data->computed_field_package);
#endif /* defined (ZINC_USE_ITK) */
		}
		/* graphics_module */
		command_data->default_time_keeper_app=ACCESS(Time_keeper_app)(UI_module->default_time_keeper_app);

		/* scene manager */
		/*???RC & SAB.   LOTS of managers need to be created before this
		  and the User_interface too */
		command_data->default_scene = cmzn_region_get_scene(command_data->root_region);
		if (command_data->computed_field_package && command_data->default_time_keeper_app)
		{
			Computed_field_register_types_time(command_data->computed_field_package,
				command_data->default_time_keeper_app->getTimeKeeper());
		}

		if (command_data->user_interface)
		{
			command_data->transform_tool=UI_module->transform_tool;
			command_data->node_tool=UI_module->node_tool;
			Node_tool_set_execute_command(command_data->node_tool, command_data->execute_command);
			command_data->data_tool=UI_module->data_tool;
			Node_tool_set_execute_command(command_data->data_tool, command_data->execute_command);
			command_data->element_tool=UI_module->element_tool;
			Element_tool_set_execute_command(command_data->element_tool,
				command_data->execute_command);
#if defined (USE_OPENCASCADE)
			command_data->cad_tool = UI_module->cad_tool;
			Cad_tool_set_execute_command(command_data->cad_tool,
				command_data->execute_command);
#endif /* defined (USE_OPENCASCADE) */
			command_data->element_point_tool=UI_module->element_point_tool;
			Element_point_tool_set_execute_command(command_data->element_point_tool,
				command_data->execute_command);
		}
#if defined (USE_CMGUI_GRAPHICS_WINDOW)
		if (command_data->user_interface)
		{
			command_data->sceneviewermodule = UI_module->sceneviewermodule;
		}
#endif /* defined (USE_CMGUI_GRAPHICS_WINDOW) */

		/* properly set up the Execute_command objects */
		Execute_command_set_command_function(command_data->execute_command,
			cmiss_execute_command, (void *)command_data);
		Execute_command_set_command_function(command_data->set_command,
			cmiss_set_command, (void *)command_data);
		/* initialize random number generator */
		if (-1 == non_random)
		{
			/* randomise */
			srand(time(NULL));
			/*???DB.  time is not ANSI */
		}
		else
		{
			/* randomise using given seed */
			srand(non_random);
		}

		if (return_code && (!command_list) && (!write_help))
		{
			if (!no_display)
			{
				/* create the main window */
				if (!server_mode)
				{
#if defined(USE_CMGUI_COMMAND_WINDOW)
					if (console_mode)
					{
#endif /* defined(USE_CMGUI_COMMAND_WINDOW) */
						if (!(command_data->command_console = CREATE(Console)(
							command_data->execute_command,
							command_data->event_dispatcher, /*stdin*/0)))
						{
							display_message(ERROR_MESSAGE,"main.  "
								"Unable to create console.");
						}
#if defined(USE_CMGUI_COMMAND_WINDOW)
					}
					else if (!UI_module->external)
					{
						if (NULL != (command_window = CREATE(Command_window)(command_data->execute_command,
							command_data->user_interface)))
						{
							command_data->command_window=command_window;
							if (!batch_mode)
							{
								/* set up messages */
								command_data->loggerNotifier = cmzn_logger_create_loggernotifier(
									command_data->logger);
								cmzn_loggernotifier_set_callback(command_data->loggerNotifier,
									display_command_window_message, command_window);
#if defined (USE_PERL_INTERPRETER)
								redirect_interpreter_output(command_data->interpreter, &return_code);
#endif /* defined (USE_PERL_INTERPRETER) */
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,"Unable to create command window");
							return_code=0;
						}
					}
#endif /* defined(USE_CMGUI_COMMAND_WINDOW) */
				}
			}
		}

		if (return_code && (!command_list) && (!write_help))
		{
			if (start_cm||start_mycm)
			{
				sprintf(global_temp_string,"create cm");
				if (start_mycm)
				{
					strcat(global_temp_string," mycm");
				}
				if (cm_parameters_file_name)
				{
					strcat(global_temp_string," parameters ");
					strcat(global_temp_string,cm_parameters_file_name);
				}
				if (cm_examples_directory)
				{
					strcat(global_temp_string," examples_directory ");
					strcat(global_temp_string,cm_examples_directory);
				}
				/* start the back-end */
				cmiss_execute_command(global_temp_string,
					(void *)command_data);
			}
			if (user_settings.startup_comfile)
			{
				/* Can't get the startupComfile name without X at the moment */
				cmgui_execute_comfile(user_settings.startup_comfile, NULL,
					NULL, NULL, (char **)NULL, command_data->execute_command);
			}
			if (execute_string)
			{
				cmiss_execute_command(execute_string,(void *)command_data);
			}
			if (example_id||comfile_name)
			{
				/* open the command line comfile */
				cmgui_execute_comfile(comfile_name,example_id,
					command_data->examples_directory,
					CMGUI_EXAMPLE_DIRECTORY_SYMBOL, &command_data->example_comfile,
					command_data->execute_command);
			}
		}

		if ((!command_list) && (!write_help))
		{
			/* START_ERROR_HANDLING;*/
			switch (signal_code)
			{
				case SIGFPE:
				{
					printf("Floating point exception occurred\n");
					display_message(ERROR_MESSAGE,
						"Floating point exception occurred");
				} break;
				case SIGILL:
				{
					printf("Illegal instruction occurred\n");
					display_message(ERROR_MESSAGE,
						"Illegal instruction occurred");
				} break;
				case SIGSEGV:
				{
					printf("Invalid memory reference occurred\n");
					display_message(ERROR_MESSAGE,
						"Invalid memory reference occurred");
				} break;
			}
		}
		if (command_list)
		{
			cmiss_execute_command("??", (void *)command_data);
		}
		if (example_id)
		{
			DEALLOCATE(example_id);
		}
		if (execute_string)
		{
			DEALLOCATE(execute_string);
		}
		if (version_command_id)
		{
			DEALLOCATE(version_command_id);
		}
		if (comfile_name)
		{
			DEALLOCATE(comfile_name);
		}

		if (command_list || write_help || batch_mode || !return_code)
		{
			cmzn_command_data_destroy(&command_data);
		}
	}
	else
	{
		command_data = (struct cmzn_command_data *)NULL;
	}
	LEAVE;

	return (command_data);
} /* CREATE(cmzn_command_data) */

int DESTROY(cmzn_command_data)(struct cmzn_command_data **command_data_address)
/*******************************************************************************
LAST MODIFIED : 19 December 2002

DESCRIPTION :
Clean up the command_data, deallocating all the associated memory and resources.
NOTE: Do not call this directly: call cmzn_command_data_destroy() to deaccess.
==============================================================================*/
{
	int return_code = 0;
#if defined (F90_INTERPRETER) || defined (USE_PERL_INTERPRETER)
	int status;
#endif /* defined (F90_INTERPRETER) || defined (USE_PERL_INTERPRETER) */
	struct cmzn_command_data *command_data;
	ENTER(DESTROY(cmzn_command_data));

	if (command_data_address && (command_data = *command_data_address))
	{
		if (command_data->access_count != 0)
		{
			display_message(ERROR_MESSAGE,
				"Call to DESTROY(cmzn_command_data) while still in use");
			return 0;
		}
#if defined (WX_USER_INTERFACE)
		/* viewers */
		if (command_data->data_viewer)
		{
			Node_viewer_destroy(&(command_data->data_viewer));
		}
		if (command_data->node_viewer)
		{
			Node_viewer_destroy(&(command_data->node_viewer));
		}
		if (command_data->element_point_viewer)
		{
			DESTROY(Element_point_viewer)(&(command_data->element_point_viewer));
		}
#endif /* defined (WX_USER_INTERFACE) */
#if defined (WX_USER_INTERFACE)
		if (command_data->material_editor)
		{
			DESTROY(Material_editor)(&(command_data->material_editor));
		}
		if (command_data->region_tree_viewer)
		{
			DESTROY(Region_tree_viewer)(&(command_data->region_tree_viewer));
		}
		if (command_data->spectrum_editor_dialog)
		{
			DESTROY(Spectrum_editor_dialog)(&(command_data->spectrum_editor_dialog));
		}
#endif /* defined (WX_USER_INTERFACE) */
		cmzn_loggernotifier_clear_callback(command_data->loggerNotifier);
		cmzn_loggernotifier_destroy(&command_data->loggerNotifier);
		cmzn_logger_destroy(&command_data->logger);
		DEACCESS(Scene)(&command_data->default_scene);
		cmzn_glyphmodule_destroy(&command_data->glyphmodule);
		cmzn_graphics_module_destroy(&command_data->graphics_module);
		DEACCESS(Time_keeper_app)(&command_data->default_time_keeper_app);
		if (command_data->computed_field_package)
		{
			Computed_field_package_remove_types(command_data->computed_field_package);
			DESTROY(Computed_field_package)(&command_data->computed_field_package);
		}
#if defined (SELECT_DESCRIPTORS)
		DESTROY(LIST(Io_device))(&command_data->device_list);
#endif /* defined (SELECT_DESCRIPTORS) */

		DEACCESS(cmzn_region)(&(command_data->root_region));
		DESTROY(MANAGER(FE_basis))(&command_data->basis_manager);
		DESTROY(LIST(FE_element_shape))(&command_data->element_shape_list);

		/* some fields register for changes with the following managers,
			 hence must destroy after regions and their fields */
		DEACCESS(cmzn_spectrum)(&(command_data->default_spectrum));
		command_data->spectrum_manager=NULL;
		cmzn_scenefiltermodule_destroy(&command_data->filter_module);
		cmzn_materialmodule_destroy(&command_data->materialmodule);
		cmzn_tessellationmodule_destroy(&command_data->tessellationmodule);
		DEACCESS(cmzn_font)(&command_data->default_font);
		DESTROY(MANAGER(VT_volume_texture))(&command_data->volume_texture_manager);
		DESTROY(MANAGER(Environment_map))(&command_data->environment_map_manager);
		cmzn_light_destroy(&(command_data->default_light));
		cmzn_lightmodule_destroy(&command_data->lightmodule);
		if (command_data->example_directory)
		{
			DEALLOCATE(command_data->example_directory);
		}
		if (command_data->example_comfile)
		{
			DEALLOCATE(command_data->example_comfile);
		}
		if (command_data->example_requirements)
		{
			DEALLOCATE(command_data->example_requirements);
		}

		Close_image_environment();

		DESTROY(Execute_command)(&command_data->execute_command);
		DESTROY(Execute_command)(&command_data->set_command);

#if defined (F90_INTERPRETER) || defined (USE_PERL_INTERPRETER)
		destroy_interpreter(command_data->interpreter, &status);
#endif /* defined (F90_INTERPRETER) || defined (USE_PERL_INTERPRETER) */

		if (command_data->command_console)
		{
			DESTROY(Console)(&command_data->command_console);
		}
#if defined (WIN32_USER_INTERFACE) || defined (GTK_USER_INTERFACE) || defined (WX_USER_INTERFACE)
		if (command_data->command_window)
		{
			DESTROY(Command_window)(&command_data->command_window);
		}
#endif /* defined (WIN32_USER_INTERFACE) || defined (GTK_USER_INTERFACE) */

		if (command_data->user_interface)
			command_data->user_interface = NULL;
		if (command_data->event_dispatcher)
			command_data->event_dispatcher = NULL;

		/* clean up command-line options */

		if (command_data->examples_directory)
		{
			DEALLOCATE(command_data->examples_directory);
		}
		if (command_data->cm_examples_directory)
		{
			DEALLOCATE(command_data->cm_examples_directory);
		}
		if (command_data->cm_parameters_file_name)
		{
			DEALLOCATE(command_data->cm_parameters_file_name);
		}

		DEALLOCATE(*command_data_address);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"DESTROY(cmzn_command_data).  "
			"Invalid arguments");
	}


	LEAVE;

	return (return_code);
} /* DESTROY(cmzn_command_data) */

struct cmzn_command_data *cmzn_command_data_access(struct cmzn_command_data *command_data)
{
	if (command_data)
	{
		command_data->access_count++;
	}
	return command_data;
}

int cmzn_command_data_destroy(
	struct cmzn_command_data **command_data_address)
{
	int return_code;
	struct cmzn_command_data *command_data;

	ENTER(cmzn_command_data_destroy);
	if (command_data_address && (NULL != (command_data = *command_data_address)))
	{
		command_data->access_count--;
		if (0 == command_data->access_count)
		{
			DESTROY(cmzn_command_data)(command_data_address);
		}
		*command_data_address = NULL;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmzn_command_data_destroy.  Missing command data");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
}

int cmzn_command_data_main_loop(struct cmzn_command_data *command_data)
/*******************************************************************************
LAST MODIFIED : 19 December 2002

DESCRIPTION :
Process events until some events request the program to finish.
==============================================================================*/
{
	int return_code = 0;

	ENTER(cmzn_command_data_main_loop);
	/* main processing / loop */
	if (command_data && command_data->event_dispatcher && command_data->start_event_dispatcher)
	{
		/* user interface loop */
		return_code=Event_dispatcher_main_loop(command_data->event_dispatcher);
	}
	LEAVE;

	return (return_code);
} /* cmzn_command_data_main_loop */

struct cmzn_region *cmzn_command_data_get_root_region(
	struct cmzn_command_data *command_data)
/*******************************************************************************
LAST MODIFIED : 18 April 2003

DESCRIPTION :
Returns the root region from the <command_data>.
==============================================================================*/
{
	struct cmzn_region *root_region;

	ENTER(cmzn_command_data_get_root_region);
	root_region=(struct cmzn_region *)NULL;
	if (command_data)
	{
		/* API functions return accessed values */
		root_region=ACCESS(cmzn_region)(command_data->root_region);
	}
	LEAVE;

	return (root_region);
} /* cmzn_command_data_get_root_region */

struct Execute_command *cmzn_command_data_get_execute_command(
	struct cmzn_command_data *command_data)
/*******************************************************************************
LAST MODIFIED : 28 May 2003

DESCRIPTION :
Returns the execute command structure from the <command_data>, useful for
executing cmiss commands from C.
==============================================================================*/
{
	struct Execute_command *execute_command;

	ENTER(cmzn_command_data_get_execute_command);
	execute_command=(struct Execute_command *)NULL;
	if (command_data)
	{
		execute_command=command_data->execute_command;
	}
	LEAVE;

	return (execute_command);
} /* cmzn_command_data_get_execute_command */

struct IO_stream_package *cmzn_command_data_get_IO_stream_package(
	struct cmzn_command_data *command_data)
/*******************************************************************************
LAST MODIFIED : 16 September 2004

DESCRIPTION :
Returns the io_stream_package structure from the <command_data>
==============================================================================*/
{
	struct IO_stream_package *io_stream_package;

	ENTER(cmzn_command_data_get_io_stream_package);
	io_stream_package=(struct IO_stream_package *)NULL;
	if (command_data)
	{
		io_stream_package = command_data->io_stream_package;
	}
	LEAVE;

	return (io_stream_package);
} /* cmzn_command_data_get_io_stream_package */

struct Fdio_package* cmzn_command_data_get_fdio_package(
	struct cmzn_command_data *command_data)
/*******************************************************************************
LAST MODIFIED : 10 March 2005

DESCRIPTION :
Gets an Fdio_package for this <command_data>
==============================================================================*/
{
	struct Fdio_package *fdio_package;

	ENTER(cmzn_command_data_get_fdio_package);
	fdio_package = (struct Fdio_package *)NULL;
	if (command_data)
	{
		fdio_package = CREATE(Fdio_package)(command_data->event_dispatcher);
	}
	LEAVE;

	return (fdio_package);
}

Idle_package_id cmzn_command_data_get_idle_package(
	struct cmzn_command_data *command_data
)
/*******************************************************************************
LAST MODIFIED : 21 March 2005

DESCRIPTION :
Gets an Idle_package for this <command_data>
==============================================================================*/
{
	struct Idle_package *idle_package;

	ENTER(cmzn_command_data_get_idle_package);
	idle_package = (struct Idle_package *)NULL;
	if (command_data)
	{
		idle_package = CREATE(Idle_package)(command_data->event_dispatcher);
	}
	LEAVE;

	return (idle_package);
}

struct MANAGER(Computed_field) *cmzn_command_data_get_computed_field_manager(
	struct cmzn_command_data *command_data)
/*******************************************************************************
LAST MODIFIED : 16 September 2004

DESCRIPTION :
Returns the root region from the <command_data>.
==============================================================================*/
{
	struct MANAGER(Computed_field) *computed_field_manager;

	ENTER(cmzn_command_data_get_computed_field_manager);
	computed_field_manager=(struct MANAGER(Computed_field) *)NULL;
	if (command_data)
	{
		computed_field_manager = Computed_field_package_get_computed_field_manager(
			command_data->computed_field_package);
	}
	LEAVE;

	return (computed_field_manager);
} /* cmzn_command_data_get_computed_field_manager */

struct User_interface *cmzn_command_data_get_user_interface(
	struct cmzn_command_data *command_data)
/*******************************************************************************
LAST MODIFIED : 25 January 2006

DESCRIPTION :
Gets the user_interface for this <command_data>
==============================================================================*/
{
	struct User_interface *user_interface;

	ENTER(cmzn_command_data_get_user_interface);
	user_interface = (struct User_interface *)NULL;
	if (command_data)
	{
		user_interface = command_data->user_interface;
	}
	LEAVE;

	return (user_interface);
} /* cmzn_command_data_get_user_interface */

struct cmzn_sceneviewermodule_app *cmzn_command_data_get_sceneviewermodule(
	struct cmzn_command_data *command_data)
/*******************************************************************************
LAST MODIFIED : 19 January 2007

DESCRIPTION :
Returns the scene viewer data from the <command_data>.
==============================================================================*/
{
	struct cmzn_sceneviewermodule_app *cmiss_sceneviewermodule;

	ENTER(cmzn_command_package_get_sceneviewermodule);
	cmiss_sceneviewermodule=(struct cmzn_sceneviewermodule_app *)NULL;
	if (command_data)
	{
		cmiss_sceneviewermodule = command_data->sceneviewermodule;
	}
	LEAVE;

	return (cmiss_sceneviewermodule);
}

struct MANAGER(Graphics_window) *cmzn_command_data_get_graphics_window_manager(
	struct cmzn_command_data *command_data)
/*******************************************************************************
LAST MODIFIED : 26 January 2007

DESCRIPTION :
Returns the graphics_window manager from the <command_data>.
==============================================================================*/
{
	struct MANAGER(Graphics_window) *graphics_window_manager;

	ENTER(cmzn_command_data_get_graphics_window_manager);
	graphics_window_manager=(struct MANAGER(Graphics_window) *)NULL;
#if defined (USE_CMGUI_GRAPHICS_WINDOW)
	if (command_data)
	{
		graphics_window_manager = command_data->graphics_window_manager;
	}
#else /* defined (USE_CMGUI_GRAPHICS_WINDOW) */
	USE_PARAMETER(command_data);
#endif /* defined (USE_CMGUI_GRAPHICS_WINDOW) */
	LEAVE;

	return (graphics_window_manager);
} /* cmzn_command_data_get_graphics_window_manager */

int cmzn_command_data_set_cmgui_string(cmzn_command_data *command_data, const char *name_string,
	const char *version_string,const char *date_string, const char *copyright_string,
	const char *build_string, const char *revision_string)
{
	int return_code = 1;
	if (command_data)
	{
#if defined(USE_CMGUI_COMMAND_WINDOW)
		if (command_data->command_window)
		{
			return_code =Command_window_set_cmgui_string(command_data->command_window,
				name_string, version_string, date_string, copyright_string, build_string, revision_string);
		}
		else
		{
			return_code = 0;
		}
#endif
	}

	return return_code;
}
