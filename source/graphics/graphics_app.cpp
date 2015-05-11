/* OpenCMISS-Cmgui Application
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#if 1
#include "configure/cmgui_configure.h"
#endif /* defined (BUILD_WITH_CMAKE) */

#include "zinc/fieldmodule.h"
#include "zinc/graphics.h"
#include "zinc/material.h"
#include "zinc/font.h"
#include "zinc/region.h"
#include "zinc/spectrum.h"
#include "zinc/tessellation.h"
#include "finite_element/finite_element_app.h"
#include "general/debug.h"
#include "general/enumerator.h"
#include "general/enumerator_private.hpp"
#include "general/message.h"
#include "command/parser.h"
#include "graphics/glyph.hpp"
#include "graphics/graphics.h"
#include "graphics/graphics_app.h"
#include "graphics/graphics_module.h"
#include "graphics/scene.h"
#include "graphics/scene_coordinate_system.hpp"
#include "graphics/tessellation.hpp"
#include "computed_field/computed_field_finite_element.h"
#include "graphics/auxiliary_graphics_types_app.h"
#include "computed_field/computed_field_set.h"
#include "computed_field/computed_field_set_app.h"
#include "graphics/spectrum_app.h"
#include "graphics/font.h"
#include "graphics/scene_app.h"
#include "graphics/material_app.h"
#include "finite_element/finite_element_region_app.h"
#include "graphics/tessellation_app.hpp"

enum Legacy_graphics_type
{
	LEGACY_GRAPHIC_NONE,
	LEGACY_GRAPHIC_POINT,
	LEGACY_GRAPHIC_NODE_POINTS,
	LEGACY_GRAPHIC_DATA_POINTS,
	LEGACY_GRAPHIC_ELEMENT_POINTS,
	LEGACY_GRAPHIC_CYLINDERS,
	LEGACY_GRAPHIC_ISO_SURFACES
};

int gfx_modify_scene_graphics(struct Parse_state *state,
	enum cmzn_graphics_type graphics_type,
	enum Legacy_graphics_type legacy_graphics_type, const char *help_text,
	struct Modify_scene_data *modify_scene_data,
	struct Scene_command_data *scene_command_data)
{
	if (!(state && scene_command_data && modify_scene_data))
	{
		display_message(ERROR_MESSAGE,
			"gfx_modify_scene_graphics.  Invalid argument(s)");
		return 0;
	}
	cmzn_graphics_id graphics = 0;
	if (modify_scene_data->modify_this_graphics)
	{
		graphics = cmzn_graphics_access(modify_scene_data->graphics);
		graphics_type = cmzn_graphics_get_type(graphics);
	}
	else
	{
		graphics = cmzn_scene_create_graphics_app(scene_command_data->scene,
			graphics_type, modify_scene_data->graphics);
		switch (legacy_graphics_type)
		{
		case LEGACY_GRAPHIC_NODE_POINTS:
			cmzn_graphics_set_field_domain_type(graphics, CMZN_FIELD_DOMAIN_TYPE_NODES);
			break;
		case LEGACY_GRAPHIC_DATA_POINTS:
			cmzn_graphics_set_field_domain_type(graphics, CMZN_FIELD_DOMAIN_TYPE_DATAPOINTS);
			break;
		case LEGACY_GRAPHIC_ELEMENT_POINTS:
			cmzn_graphics_set_field_domain_type(graphics, CMZN_FIELD_DOMAIN_TYPE_MESH_HIGHEST_DIMENSION);
			break;
		default:
			// do nothing
			break;
		}
		if (legacy_graphics_type == LEGACY_GRAPHIC_CYLINDERS)
		{
			cmzn_graphicslineattributes_id line_attributes = cmzn_graphics_get_graphicslineattributes(graphics);
			cmzn_graphicslineattributes_set_shape_type(line_attributes, CMZN_GRAPHICSLINEATTRIBUTES_SHAPE_TYPE_CIRCLE_EXTRUSION);
			const double two = 2;
			// default scale factor is 2.0 for radius to diameter conversion
			cmzn_graphicslineattributes_set_scale_factors(line_attributes, 1, &two);
			cmzn_graphicslineattributes_destroy(&line_attributes);
		}
		cmzn_scene_set_graphics_defaults_gfx_modify(scene_command_data->scene, graphics);
		if (modify_scene_data->group)
		{
			cmzn_field_id subgroup_field = cmzn_graphics_get_subgroup_field(graphics);
			if (subgroup_field)
			{
				cmzn_field_destroy(&subgroup_field);
			}
			else
			{
				cmzn_graphics_set_subgroup_field(graphics,
					cmzn_field_group_base_cast(modify_scene_data->group));
			}
		}
	}
	if (!graphics)
	{
		display_message(ERROR_MESSAGE,
			"gfx_modify_scene_graphics.  Could not create graphics");
		return 0;
	}
	REACCESS(cmzn_graphics)(&(modify_scene_data->graphics), graphics);

	int return_code = 1;

	int number_of_valid_strings;
	const char **valid_strings;
	char *seed_nodeset_name = 0;
	if (graphics->seed_nodeset)
	{
		seed_nodeset_name = cmzn_nodeset_get_name(graphics->seed_nodeset);
	}
	cmzn_graphics_contours_id contours = cmzn_graphics_cast_contours(graphics);
	cmzn_graphics_streamlines_id streamlines = cmzn_graphics_cast_streamlines(graphics);
	cmzn_graphicslineattributes_id line_attributes = cmzn_graphics_get_graphicslineattributes(graphics);
	cmzn_field_id line_orientation_scale_field = 0;
	cmzn_graphicspointattributes_id point_attributes = cmzn_graphics_get_graphicspointattributes(graphics);
	cmzn_graphicssamplingattributes_id sampling =
		((legacy_graphics_type == LEGACY_GRAPHIC_POINT) ||
		(legacy_graphics_type == LEGACY_GRAPHIC_NODE_POINTS) ||
		(legacy_graphics_type == LEGACY_GRAPHIC_DATA_POINTS)) ?
		0 : cmzn_graphics_get_graphicssamplingattributes(graphics);

	cmzn_field_id isoscalar_field = 0;
	int number_of_isovalues = 0;
	double *isovalues = 0;
	double last_isovalue = 0;
	double first_isovalue = 0;
	int range_number_of_isovalues = 0;
	double decimation_threshold = 0;
	if (contours)
	{
		isoscalar_field = cmzn_graphics_contours_get_isoscalar_field(contours);
		number_of_isovalues = cmzn_graphics_contours_get_list_isovalues(contours, 0, 0);
		if (number_of_isovalues)
		{
			ALLOCATE(isovalues, double, number_of_isovalues);
			cmzn_graphics_contours_get_list_isovalues(contours, number_of_isovalues, isovalues);
		}
		range_number_of_isovalues = cmzn_graphics_contours_get_range_number_of_isovalues(contours);
		first_isovalue = cmzn_graphics_contours_get_range_first_isovalue(contours);
		last_isovalue = cmzn_graphics_contours_get_range_last_isovalue(contours);
		decimation_threshold = cmzn_graphics_contours_get_decimation_threshold(contours);
	}

	Option_table *option_table = CREATE(Option_table)();
	if (help_text)
	{
		Option_table_add_help(option_table, help_text);
	}

	/* as */
	char *name = cmzn_graphics_get_name(graphics);
	Option_table_add_entry(option_table,"as", &name,
		(void *)1,set_name);

	/* element points sample mode: cell_centres/cell_corners/cell_density/set_location */
	const char *sampling_mode_string = 0;
	if (sampling)
	{
		cmzn_element_point_sampling_mode sampling_mode = cmzn_graphicssamplingattributes_get_element_point_sampling_mode(sampling);
		sampling_mode_string = ENUMERATOR_STRING(cmzn_element_point_sampling_mode)(sampling_mode);
		valid_strings = ENUMERATOR_GET_VALID_STRINGS(cmzn_element_point_sampling_mode)(
			&number_of_valid_strings,
			(ENUMERATOR_CONDITIONAL_FUNCTION(cmzn_element_point_sampling_mode) *)NULL,
			(void *)NULL);
		Option_table_add_enumerator(option_table,number_of_valid_strings,
			valid_strings,&sampling_mode_string);
		DEALLOCATE(valid_strings);
	}

	/* deprecated sample modes:
	   cell_density, cell_random -> cell_poisson (with warnings)
	   exact_xi -> set_location */
	const char *old_sampling_mode_strings[] = { "cell_density", "cell_random", "exact_xi" };
	const enum cmzn_element_point_sampling_mode old_to_new_sampling_mode[] =
	{
		CMZN_ELEMENT_POINT_SAMPLING_MODE_CELL_POISSON,
		CMZN_ELEMENT_POINT_SAMPLING_MODE_CELL_POISSON,
		CMZN_ELEMENT_POINT_SAMPLING_MODE_SET_LOCATION
	};
	const int old_sampling_mode_strings_count =
		sizeof(old_to_new_sampling_mode) / sizeof(cmzn_element_point_sampling_mode);
	const char *old_sampling_mode_string = 0;
	if (sampling)
	{
		Option_table_add_enumerator(option_table, old_sampling_mode_strings_count,
			old_sampling_mode_strings, &old_sampling_mode_string);
	}

	int three = 3;
	double glyph_offset[3];
	if (point_attributes)
	{
		cmzn_graphicspointattributes_get_glyph_offset(point_attributes, 3, glyph_offset);
	}

	/* glyph centre: note equal to negative of point offset! */
	double glyph_centre[3];
	if (point_attributes && (legacy_graphics_type != LEGACY_GRAPHIC_NONE))
	{
		for (int i = 0; i < 3; i++)
		{
			glyph_centre[i] = (glyph_offset[i] != 0.0) ? -glyph_offset[i] : 0.0;
		}
		Option_table_add_double_vector_entry(option_table, "centre", glyph_centre, &three);
	}

	/* circle_discretization */
	int circle_discretization = 0;
	if (legacy_graphics_type == LEGACY_GRAPHIC_CYLINDERS)
	{
		Option_table_add_entry(option_table, "circle_discretization",
			(void *)&circle_discretization, (void *)NULL,
			set_circle_divisions);
	}

	/* constant_radius */
	double constant_radius = 0;
	if (legacy_graphics_type == LEGACY_GRAPHIC_CYLINDERS)
	{
		cmzn_graphicslineattributes_get_base_size(line_attributes, 1, &constant_radius);
		constant_radius *= 0.5; // convert from diameter
		Option_table_add_double_entry(option_table, "constant_radius", &constant_radius);
	}

	/* coordinate */
	cmzn_field_id coordinate_field = cmzn_graphics_get_coordinate_field(graphics);
	Set_Computed_field_conditional_data set_coordinate_field_data;
	set_coordinate_field_data.computed_field_manager = scene_command_data->computed_field_manager;
	set_coordinate_field_data.conditional_function = Computed_field_has_up_to_3_numerical_components;
	set_coordinate_field_data.conditional_function_user_data = (void *)NULL;
	Option_table_add_Computed_field_conditional_entry(option_table, "coordinate",
		&coordinate_field, &set_coordinate_field_data);

	/* coordinate system */
	cmzn_scenecoordinatesystem coordinate_system = cmzn_graphics_get_scenecoordinatesystem(graphics);
	const char *coordinate_system_string =
		ENUMERATOR_STRING(cmzn_scenecoordinatesystem)(coordinate_system);
	valid_strings = ENUMERATOR_GET_VALID_STRINGS(cmzn_scenecoordinatesystem)(
		&number_of_valid_strings,
		(ENUMERATOR_CONDITIONAL_FUNCTION(cmzn_scenecoordinatesystem) *)NULL,
		(void *)NULL);
	Option_table_add_enumerator(option_table, number_of_valid_strings,
		valid_strings, &coordinate_system_string);
	DEALLOCATE(valid_strings);

	/* data */
	cmzn_field_id data_field = cmzn_graphics_get_data_field(graphics);
	Set_Computed_field_conditional_data set_data_field_data;
	set_data_field_data.computed_field_manager = scene_command_data->computed_field_manager;
	set_data_field_data.conditional_function = Computed_field_has_numerical_components;
	set_data_field_data.conditional_function_user_data = (void *)NULL;
	Option_table_add_Computed_field_conditional_entry(option_table, "data",
		&data_field, &set_data_field_data);

	/* decimation_threshold */
	if (contours)
	{
		Option_table_add_double_entry(option_table, "decimation_threshold",
			&decimation_threshold);
	}

	/* delete */
	Option_table_add_entry(option_table,"delete",
		&(modify_scene_data->delete_flag),NULL,set_char_flag);

	/* sample density */
	cmzn_field_id sample_density_field = 0;
	Set_Computed_field_conditional_data set_sample_density_field_data;
	set_sample_density_field_data.computed_field_manager = scene_command_data->computed_field_manager;
	set_sample_density_field_data.conditional_function = Computed_field_is_scalar;
	set_sample_density_field_data.conditional_function_user_data = (void *)NULL;
	if (sampling)
	{
		sample_density_field = cmzn_graphicssamplingattributes_get_density_field(sampling);
		Option_table_add_Computed_field_conditional_entry(option_table, "density",
			&sample_density_field, &set_sample_density_field_data);
	}

	/* discretization */
	int element_divisions_size = 0;
	int *element_divisions = 0;
	if ((legacy_graphics_type == LEGACY_GRAPHIC_ELEMENT_POINTS) ||
		(graphics_type == CMZN_GRAPHICS_TYPE_STREAMLINES))
	{
		Option_table_add_divisions_entry(option_table, "discretization",
			&element_divisions, &element_divisions_size);
	}

	/* domain type */
	cmzn_field_domain_type domain_type = cmzn_graphics_get_field_domain_type(graphics);
	const char *domain_type_string = ENUMERATOR_STRING(cmzn_field_domain_type)(domain_type);
	if ((legacy_graphics_type != LEGACY_GRAPHIC_POINT) &&
		(legacy_graphics_type != LEGACY_GRAPHIC_NODE_POINTS) &&
		(legacy_graphics_type != LEGACY_GRAPHIC_DATA_POINTS) &&
		(legacy_graphics_type != LEGACY_GRAPHIC_ELEMENT_POINTS) &&
		(legacy_graphics_type != LEGACY_GRAPHIC_ISO_SURFACES))
	{
		valid_strings = ENUMERATOR_GET_VALID_STRINGS(cmzn_field_domain_type)(
			&number_of_valid_strings,
			(ENUMERATOR_CONDITIONAL_FUNCTION(cmzn_field_domain_type) *)NULL,
			(void *)NULL);
		Option_table_add_enumerator(option_table, number_of_valid_strings,
			valid_strings, &domain_type_string);
		DEALLOCATE(valid_strings);
	}

	/* deprecated streamline_type ellipse/line/rectangle/ribbon/cylinder
	 * (replaced with line_shape, some with same token so not migrated) */
	const char *streamline_type_strings[3] = { "ellipse", "rectangle", "cylinder" };
	const struct { enum cmzn_graphicslineattributes_shape_type shape_type; FE_value thickness_to_width_ratio; }
		streamline_type_to_line_shape_type[] =
		{
			{ CMZN_GRAPHICSLINEATTRIBUTES_SHAPE_TYPE_CIRCLE_EXTRUSION, 0.2 },
			{ CMZN_GRAPHICSLINEATTRIBUTES_SHAPE_TYPE_SQUARE_EXTRUSION, 0.2 },
			{ CMZN_GRAPHICSLINEATTRIBUTES_SHAPE_TYPE_CIRCLE_EXTRUSION, 1.0 }
		};
	const char *streamline_type_string = 0;
	if (streamlines)
	{
		Option_table_add_enumerator(option_table, 3, streamline_type_strings, &streamline_type_string);
	}

	/* exterior */
	char exterior_flag = static_cast<char>(cmzn_graphics_is_exterior(graphics));
	if ((legacy_graphics_type != LEGACY_GRAPHIC_POINT) &&
		(legacy_graphics_type != LEGACY_GRAPHIC_NODE_POINTS) &&
		(legacy_graphics_type != LEGACY_GRAPHIC_DATA_POINTS))
	{
		Option_table_add_entry(option_table, "exterior", &exterior_flag,
			NULL, set_char_flag);
	}

	/* face {xi1_0|xi1_1|xi2_0|...} */
	cmzn_element_face_type face_type = CMZN_ELEMENT_FACE_TYPE_INVALID;
	if ((legacy_graphics_type != LEGACY_GRAPHIC_POINT) &&
		(legacy_graphics_type != LEGACY_GRAPHIC_NODE_POINTS) &&
		(legacy_graphics_type != LEGACY_GRAPHIC_DATA_POINTS))
	{
		face_type = cmzn_graphics_get_element_face_type(graphics);
		Option_table_add_entry(option_table,"face", &face_type, const_cast<char *>("element face"),
			setEnum<cmzn_element_face_type, /*firstEnum*/CMZN_ELEMENT_FACE_TYPE_ALL, cmzn_element_face_type_to_string>);
	}

	/* first_iso_value */
	if (contours)
	{
		Option_table_add_double_entry(option_table,"first_iso_value",
			&first_isovalue);
	}

	/* font */
	char *font_name = (char *)NULL;
	if (point_attributes)
	{
		Option_table_add_name_entry(option_table, "font", &font_name);
	}

	/* glyph */
	char *glyph_name = 0;
	if (point_attributes)
	{
		cmzn_glyph_id glyph = cmzn_graphicspointattributes_get_glyph(point_attributes);
		if (glyph)
		{
			glyph_name = cmzn_glyph_get_name(glyph);
			cmzn_glyph_destroy(&glyph);
		}
		Option_table_add_string_entry(option_table, "glyph", &glyph_name, " GLYPH_NAME|none");
	}

	/* deprecated: glyph scaling mode constant/scalar/vector/axes/general (redundant) */
	const char *deprecated_glyph_scaling_mode_strings[] = { "constant", "scalar", "vector", "axes", "general" };
	const char *glyph_scaling_mode_string = 0;
	if ((legacy_graphics_type != LEGACY_GRAPHIC_NONE) && point_attributes)
	{
		Option_table_add_enumerator(option_table, /*number_of_valid_strings*/5,
			deprecated_glyph_scaling_mode_strings, &glyph_scaling_mode_string);
	}

	/* iso_scalar */
	Set_Computed_field_conditional_data set_isoscalar_field_data;
	set_isoscalar_field_data.computed_field_manager = scene_command_data->computed_field_manager;
	set_isoscalar_field_data.conditional_function = Computed_field_is_scalar;
	set_isoscalar_field_data.conditional_function_user_data = (void *)NULL;
	if (contours)
	{
		Option_table_add_Computed_field_conditional_entry(option_table, "iso_scalar",
			&isoscalar_field, &set_isoscalar_field_data);
	}

	/* iso_values */
	if (contours)
	{
		Option_table_add_variable_length_double_vector_entry(option_table,
			"iso_values", &number_of_isovalues, &isovalues);
	}

	/* last_iso_value */
	if (contours)
	{
		Option_table_add_double_entry(option_table,"last_iso_value",
			&last_isovalue);
	}

	/* label */
	cmzn_field_id label_field = 0;
	Set_Computed_field_conditional_data set_label_field_data;
	set_label_field_data.computed_field_manager = scene_command_data->computed_field_manager;
	set_label_field_data.conditional_function = (MANAGER_CONDITIONAL_FUNCTION(Computed_field) *)NULL;
	set_label_field_data.conditional_function_user_data = (void *)NULL;
	if (point_attributes)
	{
		label_field = cmzn_graphicspointattributes_get_label_field(point_attributes);
		Option_table_add_Computed_field_conditional_entry(option_table, "label",
			&label_field, &set_label_field_data);
	}

	/* label_offset */
	double label_offset[3];
	if (point_attributes)
	{
		cmzn_graphicspointattributes_get_label_offset(point_attributes, 3, label_offset);
		Option_table_add_double_vector_entry(option_table, "label_offset", label_offset, &three);
	}

	/* label_text */
	Multiple_strings label_strings;
	if (point_attributes)
	{
		Option_table_add_multiple_strings_entry(option_table, "label_text",
			&label_strings, " LABEL_STRING [& LABEL_STRING [& ...]]");
	}

	/* ldensity */
	Set_Computed_field_conditional_data set_label_density_field_data;
	set_label_density_field_data.computed_field_manager = scene_command_data->computed_field_manager;
	set_label_density_field_data.conditional_function = Computed_field_has_up_to_3_numerical_components;
	set_label_density_field_data.conditional_function_user_data = (void *)NULL;
	if (point_attributes)
	{
		Option_table_add_Computed_field_conditional_entry(option_table, "ldensity",
			&(graphics->label_density_field), &set_label_density_field_data);
	}

	/* streamline length */
	double streamline_length = 0;
	if (streamlines)
	{
		streamline_length = cmzn_graphics_streamlines_get_track_length(streamlines);
		Option_table_add_non_negative_double_entry(option_table,"length", &streamline_length);
	}

	/* line shape: line/ribbon/circle_extrusion/square_extrusion */
	const char *line_shape_string = 0;
	if (line_attributes)
	{
		line_shape_string = ENUMERATOR_STRING(cmzn_graphicslineattributes_shape_type)(
			cmzn_graphicslineattributes_get_shape_type(line_attributes));
		valid_strings = ENUMERATOR_GET_VALID_STRINGS(cmzn_graphicslineattributes_shape_type)(
			&number_of_valid_strings,
			(ENUMERATOR_CONDITIONAL_FUNCTION(cmzn_graphicslineattributes_shape_type) *)0,
			(void *)NULL);
		Option_table_add_enumerator(option_table, number_of_valid_strings,
			valid_strings, &line_shape_string);
		DEALLOCATE(valid_strings);
	}

	/* line_base_size */
	const int line_base_size_count = 2;
	double line_base_size[2] = { 0.0, 0.0 };
	if (line_attributes && (legacy_graphics_type != LEGACY_GRAPHIC_CYLINDERS))
	{
		cmzn_graphicslineattributes_get_base_size(line_attributes, 2, line_base_size);
		Option_table_add_double_product_entry(option_table, "line_base_size",
			line_base_size_count, line_base_size);
	}

	/* line_orientation_scale */
	Set_Computed_field_conditional_data set_line_orientation_scale_field_data;
	set_line_orientation_scale_field_data.computed_field_manager = scene_command_data->computed_field_manager;
	set_line_orientation_scale_field_data.conditional_function = Computed_field_is_scalar;
	set_line_orientation_scale_field_data.conditional_function_user_data = (void *)NULL;
	if (line_attributes && (legacy_graphics_type != LEGACY_GRAPHIC_CYLINDERS))
	{
		line_orientation_scale_field =
			cmzn_graphicslineattributes_get_orientation_scale_field(line_attributes);
		Option_table_add_Computed_field_conditional_entry(option_table, "line_orientation_scale",
			&line_orientation_scale_field, &set_line_orientation_scale_field_data);
	}

	/* line_scale_factors */
	const int line_scale_factors_count = 2;
	double line_scale_factors[2] = { 1.0, 1.0 };
	if (line_attributes && (legacy_graphics_type != LEGACY_GRAPHIC_CYLINDERS))
	{
		cmzn_graphicslineattributes_get_scale_factors(line_attributes, 2, line_scale_factors);
		Option_table_add_double_product_entry(option_table, "line_scale_factors",
			line_scale_factors_count, line_scale_factors);
	}

	/* line_width */
	double line_width = cmzn_graphics_get_render_line_width(graphics);
	Option_table_add_positive_double_entry(option_table, "line_width", &line_width);

	/* material */
	cmzn_material_id material = cmzn_graphics_get_material(graphics);
	Option_table_add_set_Material_entry(option_table, "material", &material,
		scene_command_data->materialmodule);

	/* glyph repeat mode REPEAT_MODE_NONE|REPEAT_MODE_AXES_2D|REPEAT_MODE_AXES_3D|REPEAT_MODE_MIRROR */
	const char *glyph_repeat_mode_string = 0;
	if (point_attributes)
	{
		glyph_repeat_mode_string = ENUMERATOR_STRING(cmzn_glyph_repeat_mode)(
			cmzn_graphicspointattributes_get_glyph_repeat_mode(point_attributes));
		valid_strings = ENUMERATOR_GET_VALID_STRINGS(cmzn_glyph_repeat_mode)(
			&number_of_valid_strings,
			(ENUMERATOR_CONDITIONAL_FUNCTION(cmzn_glyph_repeat_mode) *)NULL,
			(void *)NULL);
		Option_table_add_enumerator(option_table, number_of_valid_strings,
			valid_strings, &glyph_repeat_mode_string);
		DEALLOCATE(valid_strings);
	}

	/* native_discretization (tessellation_field) */
	cmzn_field_id tessellation_field = cmzn_graphics_get_tessellation_field(graphics);
	Set_Computed_field_conditional_data set_tessellation_field_data;
	set_tessellation_field_data.computed_field_manager = scene_command_data->computed_field_manager;
	set_tessellation_field_data.conditional_function = (MANAGER_CONDITIONAL_FUNCTION(Computed_field) *)NULL;
	set_tessellation_field_data.conditional_function_user_data = (void *)NULL;
	if ((legacy_graphics_type != LEGACY_GRAPHIC_POINT) &&
		(legacy_graphics_type != LEGACY_GRAPHIC_NODE_POINTS) &&
		(legacy_graphics_type != LEGACY_GRAPHIC_DATA_POINTS))
	{
		tessellation_field = cmzn_graphics_get_tessellation_field(graphics);
		Option_table_add_Computed_field_conditional_entry(option_table, "native_discretization",
			&tessellation_field, &set_tessellation_field_data);
	}

	/* no_data/field_scalar/magnitude_scalar/travel_scalar */
	cmzn_graphics_streamlines_colour_data_type streamlines_colour_data_type = CMZN_GRAPHICS_STREAMLINES_COLOUR_DATA_TYPE_FIELD;
	const char *streamlines_colour_data_type_string =
		ENUMERATOR_STRING(cmzn_graphics_streamlines_colour_data_type)(streamlines_colour_data_type);
	const char *no_streamlines_colour_data_type_string = "no_data";
	if (graphics_type == CMZN_GRAPHICS_TYPE_STREAMLINES)
	{
		const char *colour_data_strings[4] =
		{
			no_streamlines_colour_data_type_string, // option removed; interpreted as COLOUR_DATA_TYPE_FIELD
			ENUMERATOR_STRING(cmzn_graphics_streamlines_colour_data_type)(CMZN_GRAPHICS_STREAMLINES_COLOUR_DATA_TYPE_FIELD),
			ENUMERATOR_STRING(cmzn_graphics_streamlines_colour_data_type)(CMZN_GRAPHICS_STREAMLINES_COLOUR_DATA_TYPE_MAGNITUDE),
			ENUMERATOR_STRING(cmzn_graphics_streamlines_colour_data_type)(CMZN_GRAPHICS_STREAMLINES_COLOUR_DATA_TYPE_TRAVEL_TIME)
		};
		Option_table_add_enumerator(option_table, 4,
			colour_data_strings, &streamlines_colour_data_type_string);
	}

	/* glyph offset */
	if (point_attributes && (legacy_graphics_type == LEGACY_GRAPHIC_NONE))
	{
		Option_table_add_double_vector_entry(option_table, "offset", glyph_offset, &three);
	}

	/* orientation */
	cmzn_field_id orientation_scale_field = 0;
	Set_Computed_field_conditional_data set_orientation_scale_field_data;
	set_orientation_scale_field_data.computed_field_manager = scene_command_data->computed_field_manager;
	set_orientation_scale_field_data.conditional_function = Computed_field_is_orientation_scale_capable;
	set_orientation_scale_field_data.conditional_function_user_data = (void *)NULL;
	if (point_attributes)
	{
		orientation_scale_field = cmzn_graphicspointattributes_get_orientation_scale_field(point_attributes);
		Option_table_add_Computed_field_conditional_entry(option_table, "orientation",
			&orientation_scale_field, &set_orientation_scale_field_data);
	}

	/* point_size */
	double point_size = cmzn_graphics_get_render_point_size(graphics);
	Option_table_add_positive_double_entry(option_table, "point_size", &point_size);

	/* position */
	Option_table_add_entry(option_table,"position",
		&(modify_scene_data->position),NULL,set_int_non_negative);

	/* radius_scalar */
	Set_Computed_field_conditional_data set_radius_scalar_field_data;
	set_radius_scalar_field_data.computed_field_manager = scene_command_data->computed_field_manager;
	set_radius_scalar_field_data.conditional_function = Computed_field_is_scalar;
	set_radius_scalar_field_data.conditional_function_user_data = (void *)NULL;
	if (legacy_graphics_type == LEGACY_GRAPHIC_CYLINDERS)
	{
		line_orientation_scale_field =
			cmzn_graphicslineattributes_get_orientation_scale_field(line_attributes);
		Option_table_add_Computed_field_conditional_entry(option_table, "radius_scalar",
			&line_orientation_scale_field, &set_radius_scalar_field_data);
	}

	/* range_number_of_iso_values */
	if (contours)
	{
		Option_table_add_int_positive_entry(option_table,
			"range_number_of_iso_values", &range_number_of_isovalues);
	}

	/* render_polygon_mode: render_shaded|render_wireframe */
	const char *render_polygon_mode_string = 0;
	cmzn_graphics_render_polygon_mode render_polygon_mode =  cmzn_graphics_get_render_polygon_mode(graphics);
	render_polygon_mode_string = ENUMERATOR_STRING(cmzn_graphics_render_polygon_mode)(render_polygon_mode);
	valid_strings = ENUMERATOR_GET_VALID_STRINGS(cmzn_graphics_render_polygon_mode)(
		&number_of_valid_strings,
		(ENUMERATOR_CONDITIONAL_FUNCTION(cmzn_graphics_render_polygon_mode) *)NULL, (void *)NULL);
	Option_table_add_enumerator(option_table,number_of_valid_strings,
		valid_strings,&render_polygon_mode_string);
	DEALLOCATE(valid_strings);

	/* forward_track|reverse_track */
	const char *streamlines_track_direction_string = 0;
	if (streamlines)
	{
		streamlines_track_direction_string = ENUMERATOR_STRING(cmzn_graphics_streamlines_track_direction)(
			cmzn_graphics_streamlines_get_track_direction(streamlines));
		valid_strings = ENUMERATOR_GET_VALID_STRINGS(cmzn_graphics_streamlines_track_direction)(
			&number_of_valid_strings,
			(ENUMERATOR_CONDITIONAL_FUNCTION(cmzn_graphics_streamlines_track_direction) *)0, (void *)0);
		Option_table_add_enumerator(option_table, number_of_valid_strings,
			valid_strings, &streamlines_track_direction_string);
		DEALLOCATE(valid_strings);
	}

	/* deprecated cylinder radius scale_factor (replaced with line_scale_factors) */
	double radius_scale_factor = 0;
	if (legacy_graphics_type == LEGACY_GRAPHIC_CYLINDERS)
	{
		cmzn_graphicslineattributes_get_scale_factors(line_attributes, 1, &radius_scale_factor);
		radius_scale_factor *= 0.5; // convert from diameter
		Option_table_add_entry(option_table, "scale_factor",
			&radius_scale_factor, NULL, set_double);
	}

	/* glyph scale_factors */
	double glyph_scale_factors[3];
	if (point_attributes)
	{
		cmzn_graphicspointattributes_get_scale_factors(point_attributes, 3, glyph_scale_factors);
		Option_table_add_double_product_entry(option_table, "scale_factors", 3, glyph_scale_factors);
	}

	/* secondary_material (was: multipass_pass1_material) */
	if (graphics_type == CMZN_GRAPHICS_TYPE_LINES)
	{
		Option_table_add_set_Material_entry(option_table, "secondary_material", &(graphics->secondary_material),
			scene_command_data->materialmodule);
	}

	/* seed_element */
	if (graphics_type == CMZN_GRAPHICS_TYPE_STREAMLINES)
	{
		Option_table_add_entry(option_table, "seed_element",
			&(graphics->seed_element), cmzn_region_get_FE_region(scene_command_data->region),
			set_FE_element_top_level_FE_region);
	}

	// seed_node_mesh_location_field
	Set_Computed_field_conditional_data set_seed_mesh_location_field_data;
	set_seed_mesh_location_field_data.conditional_function = Computed_field_has_value_type_mesh_location;
	set_seed_mesh_location_field_data.conditional_function_user_data = (void *)NULL;
	set_seed_mesh_location_field_data.computed_field_manager = scene_command_data->computed_field_manager;
	if (graphics_type == CMZN_GRAPHICS_TYPE_STREAMLINES)
	{
		Option_table_add_Computed_field_conditional_entry(option_table, "seed_node_mesh_location_field",
			&(graphics->seed_node_mesh_location_field), &set_seed_mesh_location_field_data);
	}

	// seed_nodeset
	if (graphics_type == CMZN_GRAPHICS_TYPE_STREAMLINES)
	{
		Option_table_add_string_entry(option_table, "seed_nodeset", &seed_nodeset_name,
			" NODE_GROUP_FIELD_NAME|[GROUP_NAME.]nodes|datapoints|none");
	}

	/* select_mode */
	enum cmzn_graphics_select_mode select_mode = cmzn_graphics_get_select_mode(graphics);
	const char *select_mode_string = ENUMERATOR_STRING(cmzn_graphics_select_mode)(select_mode);
	valid_strings = ENUMERATOR_GET_VALID_STRINGS(cmzn_graphics_select_mode)(
		&number_of_valid_strings,
		(ENUMERATOR_CONDITIONAL_FUNCTION(cmzn_graphics_select_mode) *)NULL,
		(void *)NULL);
	Option_table_add_enumerator(option_table,number_of_valid_strings,
		valid_strings,&select_mode_string);
	DEALLOCATE(valid_strings);

	/* selected_material */
	cmzn_material_id selected_material = cmzn_graphics_get_selected_material(graphics);
	Option_table_add_set_Material_entry(option_table, "selected_material", &selected_material,
		scene_command_data->materialmodule);

	/* glyph base size */
	double glyph_base_size[3];
	if (point_attributes)
	{
		cmzn_graphicspointattributes_get_base_size(point_attributes, 3, glyph_base_size);
		Option_table_add_double_product_entry(option_table, "size", 3, glyph_base_size);
	}

	/* spectrum */
	cmzn_spectrum_id spectrum = cmzn_graphics_get_spectrum(graphics);
	Option_table_add_entry(option_table,"spectrum",
		&spectrum, scene_command_data->spectrum_manager,
		set_Spectrum);

	/* subgroup field */
	cmzn_field_id subgroup_field = cmzn_graphics_get_subgroup_field(graphics);
	Set_Computed_field_conditional_data set_subgroup_field_data;
	set_subgroup_field_data.computed_field_manager = scene_command_data->computed_field_manager;
	set_subgroup_field_data.conditional_function = Computed_field_is_scalar;
	set_subgroup_field_data.conditional_function_user_data = (void *)NULL;
	Option_table_add_Computed_field_conditional_entry(option_table, "subgroup",
		&subgroup_field, &set_subgroup_field_data);

	/* tessellation */
	cmzn_tessellation_id tessellation = cmzn_graphics_get_tessellation(graphics);
	if ((legacy_graphics_type != LEGACY_GRAPHIC_POINT) &&
		(legacy_graphics_type != LEGACY_GRAPHIC_NODE_POINTS) &&
		(legacy_graphics_type != LEGACY_GRAPHIC_DATA_POINTS))
	{
		Option_table_add_cmzn_tessellation_entry(option_table, "tessellation",
			scene_command_data->tessellationmodule, &tessellation);
	}

	/* texture_coordinates */
	cmzn_field_id texture_coordinate_field = cmzn_graphics_get_texture_coordinate_field(graphics);
	Set_Computed_field_conditional_data set_texture_coordinate_field_data;
	set_texture_coordinate_field_data.computed_field_manager = scene_command_data->computed_field_manager;
	set_texture_coordinate_field_data.conditional_function = Computed_field_has_up_to_3_numerical_components;
	set_texture_coordinate_field_data.conditional_function_user_data = (void *)NULL;
	if ((graphics_type == CMZN_GRAPHICS_TYPE_SURFACES) ||
		(graphics_type == CMZN_GRAPHICS_TYPE_CONTOURS) ||
		(graphics_type == CMZN_GRAPHICS_TYPE_LINES))
	{
		Option_table_add_Computed_field_conditional_entry(option_table, "texture_coordinates",
			&texture_coordinate_field, &set_texture_coordinate_field_data);
	}

	/* deprecated use_elements/use_faces/use_lines (translated into domain type) */
	const char *use_element_type_strings[] = { "use_elements", "use_faces", "use_lines" };
	const enum cmzn_field_domain_type use_element_type_to_domain_type[] =
	{
		CMZN_FIELD_DOMAIN_TYPE_MESH_HIGHEST_DIMENSION,
		CMZN_FIELD_DOMAIN_TYPE_MESH2D,
		CMZN_FIELD_DOMAIN_TYPE_MESH1D
	};
	const char *use_element_type_string = 0;
	if ((legacy_graphics_type == LEGACY_GRAPHIC_ISO_SURFACES) ||
		(legacy_graphics_type == LEGACY_GRAPHIC_ELEMENT_POINTS))
	{
		Option_table_add_enumerator(option_table, 3, use_element_type_strings, &use_element_type_string);
	}

	/* variable_scale */
	cmzn_field_id signed_scale_field = 0;
	Set_Computed_field_conditional_data set_signed_scale_field_data;
	set_signed_scale_field_data.computed_field_manager = scene_command_data->computed_field_manager;
	set_signed_scale_field_data.conditional_function = Computed_field_has_up_to_3_numerical_components;
	set_signed_scale_field_data.conditional_function_user_data = (void *)NULL;
	if (point_attributes)
	{
		signed_scale_field = cmzn_graphicspointattributes_get_signed_scale_field(point_attributes);
		Option_table_add_Computed_field_conditional_entry(option_table, "variable_scale",
			&signed_scale_field, &set_signed_scale_field_data);
	}

	/* vector */
	cmzn_field_id stream_vector_field = 0;
	Set_Computed_field_conditional_data set_stream_vector_field_data;
	set_stream_vector_field_data.computed_field_manager = scene_command_data->computed_field_manager;
	set_stream_vector_field_data.conditional_function = Computed_field_is_stream_vector_capable;
	set_stream_vector_field_data.conditional_function_user_data = (void *)NULL;
	if (streamlines)
	{
		stream_vector_field = cmzn_graphics_streamlines_get_stream_vector_field(streamlines);
		Option_table_add_Computed_field_conditional_entry(option_table, "vector",
			&stream_vector_field, &set_stream_vector_field_data);
	}

	/* visible/invisible */
	int visibility_flag = static_cast<int>(cmzn_graphics_get_visibility_flag(graphics));
	Option_table_add_switch(option_table, "visible", "invisible", &visibility_flag);

	/* deprecated: streamline width (replaced with line_base_size) */
	double streamline_width = 0.0;
	if (graphics_type == CMZN_GRAPHICS_TYPE_STREAMLINES)
	{
		Option_table_add_double_entry(option_table, "width", &streamline_width);
	}

	/* xi (sample location) */
	double *sample_location = 0;
	int sample_location_components = 0;
	if (sampling)
	{
		sample_location_components = 3;
		ALLOCATE(sample_location, double, sample_location_components);
		cmzn_graphicssamplingattributes_get_location(sampling, sample_location_components, sample_location);
		Option_table_add_variable_length_double_vector_entry(option_table, "xi",
			&sample_location_components, &sample_location);
	}

	if ((return_code=Option_table_multi_parse(option_table,state)))
	{
		if (name)
		{
			cmzn_graphics_set_name(graphics, name);
		}
		cmzn_graphics_set_subgroup_field(graphics, subgroup_field);
		cmzn_graphics_set_coordinate_field(graphics, coordinate_field);
		cmzn_graphics_set_data_field(graphics, data_field);
		bool use_spectrum = (0 != data_field);
		cmzn_graphics_set_exterior(graphics, (0 != exterior_flag));
		cmzn_graphics_set_element_face_type(graphics, face_type);
		cmzn_graphics_set_tessellation(graphics, tessellation);
		cmzn_graphics_set_tessellation_field(graphics, tessellation_field);
		if ((graphics_type == CMZN_GRAPHICS_TYPE_SURFACES) ||
			(graphics_type == CMZN_GRAPHICS_TYPE_CONTOURS) ||
			(graphics_type == CMZN_GRAPHICS_TYPE_LINES))
		{
			cmzn_graphics_set_texture_coordinate_field(graphics, texture_coordinate_field);
		}
		cmzn_graphics_set_material(graphics, material);
		cmzn_graphics_set_render_line_width(graphics, line_width);
		cmzn_graphics_set_render_point_size(graphics, point_size);
		cmzn_graphics_set_selected_material(graphics, selected_material);

		if (contours)
		{
			if (!isoscalar_field)
			{
				display_message(ERROR_MESSAGE,
					"gfx_modify_scene_iso_surfaces.  Missing iso_scalar field");
				return_code=0;
			}
			cmzn_graphics_contours_set_isoscalar_field(contours, isoscalar_field);
			if (((0 < range_number_of_isovalues) && isovalues) ||
				((0 >= range_number_of_isovalues) && (0 == isovalues)))
			{
				display_message(ERROR_MESSAGE,
					"Must specify either <iso_values> OR <range_number_of_iso_values>, <first_iso_value> and <last_iso_value>.");
				return_code = 0;
			}
			else if (range_number_of_isovalues)
			{
				cmzn_graphics_contours_set_range_isovalues(contours, range_number_of_isovalues, first_isovalue, last_isovalue);
			}
			else
			{
				cmzn_graphics_contours_set_list_isovalues(contours, number_of_isovalues, isovalues);
			}
			cmzn_graphics_contours_set_decimation_threshold(contours, decimation_threshold);
		}

		cmzn_graphics_set_visibility_flag(graphics, 0 != visibility_flag);

		if (sampling)
		{
			cmzn_element_point_sampling_mode sampling_mode;
			STRING_TO_ENUMERATOR(cmzn_element_point_sampling_mode)(
				sampling_mode_string, &sampling_mode);
			if (old_sampling_mode_string)
			{
				for (int i = 0; i < old_sampling_mode_strings_count; ++i)
				{
					if (old_sampling_mode_string == old_sampling_mode_strings[i])
					{
						sampling_mode = old_to_new_sampling_mode[i];
					}
				}
				if (CMZN_ELEMENT_POINT_SAMPLING_MODE_CELL_POISSON == sampling_mode)
				{
					display_message(WARNING_MESSAGE, "Migrating obsolete sampling mode '%s' to cell_poisson", old_sampling_mode_string);
				}
			}
			if ((CMZN_ELEMENT_POINT_SAMPLING_MODE_CELL_POISSON == sampling_mode) &&
				(0 == sample_density_field))
			{
				display_message(ERROR_MESSAGE,
					"No density field specified for sample mode 'cell_poisson'");
				return_code = 0;
			}
			cmzn_graphicssamplingattributes_set_element_point_sampling_mode(sampling, sampling_mode);
			cmzn_graphicssamplingattributes_set_density_field(sampling, sample_density_field);
			cmzn_graphicssamplingattributes_set_location(sampling, sample_location_components, sample_location);
		}

		if ((graphics_type != CMZN_GRAPHICS_TYPE_LINES) &&
			(graphics_type != CMZN_GRAPHICS_TYPE_SURFACES))
		{
			STRING_TO_ENUMERATOR(cmzn_field_domain_type)(domain_type_string, &domain_type);
			cmzn_graphics_set_field_domain_type(graphics, domain_type);
		}
		// translate legacy use_element_type to domain_type
		if (use_element_type_string)
		{
			for (int i = 0; i < 3; i++)
			{
				if (fuzzy_string_compare_same_length(use_element_type_string, use_element_type_strings[i]))
				{
					domain_type = use_element_type_to_domain_type[i];
					cmzn_graphics_set_field_domain_type(graphics, domain_type);
					break;
				}
			}
		}

		if (point_attributes)
		{
			cmzn_glyph_repeat_mode glyph_repeat_mode = CMZN_GLYPH_REPEAT_MODE_NONE;
			STRING_TO_ENUMERATOR(cmzn_glyph_repeat_mode)(glyph_repeat_mode_string, &glyph_repeat_mode);
			if (legacy_graphics_type == LEGACY_GRAPHIC_POINT)
			{
				// scale factors and orientation were never used, and offset was in model units.
				// now the offset is in multiples of glyph axes, i.e. need divide by the base size
				// to work as before
				for (int i = 0; i < 3; i++)
				{
					if (glyph_base_size[i] != 0.0)
					{
						glyph_centre[i] /= glyph_base_size[i];
					}
				}
			}

			cmzn_glyph_id glyph = 0;
			if (glyph_name)
			{
				glyph = cmzn_glyphmodule_find_glyph_by_name(scene_command_data->glyphmodule, glyph_name);
			}
			if (glyph_name && (!glyph) && (0 != strcmp(glyph_name, "none")))
			{
				if ((0 == strcmp(glyph_name, "mirror_arrow_solid")) ||
					(0 == strcmp(glyph_name, "mirror_cone")) ||
					(0 == strcmp(glyph_name, "mirror_line")))
				{
					glyph = cmzn_glyphmodule_find_glyph_by_name(scene_command_data->glyphmodule, glyph_name + 7);
					glyph_repeat_mode = CMZN_GLYPH_REPEAT_MODE_MIRROR;
				}
				else if ((0 == strcmp(glyph_name, "arrow_line")) ||
					(0 == strcmp(glyph_name, "mirror_arrow_line")))
				{
					glyph = cmzn_glyphmodule_find_glyph_by_name(scene_command_data->glyphmodule, "arrow");
					if (glyph_name[0] == 'm')
					{
						glyph_repeat_mode = CMZN_GLYPH_REPEAT_MODE_MIRROR;
					}
					// fix lateral scaling of old arrow_line glyph; now unit width arrow
					glyph_base_size[1] *= 0.25;
					glyph_base_size[2] *= 0.25;
					glyph_scale_factors[1] *= 0.25;
					glyph_scale_factors[2] *= 0.25;
				}
				else if (0 == strcmp(glyph_name, "cylinder6"))
				{
					// just using normal cylinder, default circle divisions
					glyph = cmzn_glyphmodule_find_glyph_by_name(scene_command_data->glyphmodule, "cylinder");
				}
				else if (0 == strcmp(glyph_name, "cylinder_hires"))
				{
					glyph = cmzn_glyphmodule_find_glyph_by_name(scene_command_data->glyphmodule, "cylinder");
					circle_discretization = 48;
				}
				else if (0 == strcmp(glyph_name, "cylinder_solid_hires"))
				{
					glyph = cmzn_glyphmodule_find_glyph_by_name(scene_command_data->glyphmodule, "cylinder_solid");
					circle_discretization = 48;
				}
				else if (0 == strcmp(glyph_name, "sphere_hires"))
				{
					glyph = cmzn_glyphmodule_find_glyph_by_name(scene_command_data->glyphmodule, "sphere");
					circle_discretization = 48;
				}
				if (!glyph)
				{
					display_message(ERROR_MESSAGE, "Unknown glyph: ", glyph_name);
					return_code = 0;
				}
			}
			cmzn_graphicspointattributes_set_glyph(point_attributes, glyph);
			cmzn_glyph_destroy(&glyph);
			cmzn_graphicspointattributes_set_glyph_repeat_mode(point_attributes, glyph_repeat_mode);
			cmzn_graphicspointattributes_set_orientation_scale_field(point_attributes, orientation_scale_field);

			if (legacy_graphics_type != LEGACY_GRAPHIC_NONE)
			{
				// reverse centre to get offset:
				for (int i = 0; i < 3; i++)
				{
					glyph_offset[i] = (glyph_centre[i] != 0.0) ? -glyph_centre[i] : 0.0;
				}
			}
			cmzn_graphicspointattributes_set_glyph_offset(point_attributes, 3, glyph_offset);
			cmzn_graphicspointattributes_set_base_size(point_attributes, 3, glyph_base_size);
			cmzn_graphicspointattributes_set_scale_factors(point_attributes, 3, glyph_scale_factors);
			cmzn_graphicspointattributes_set_signed_scale_field(point_attributes, signed_scale_field);
			cmzn_graphicspointattributes_set_label_field(point_attributes, label_field);
			cmzn_graphicspointattributes_set_label_offset(point_attributes, 3, label_offset);
			if (0 < label_strings.number_of_strings)
			{
				for (int i = 0; i < label_strings.number_of_strings; ++i)
				{
					cmzn_graphicspointattributes_set_label_text(point_attributes, i + 1, label_strings.strings[i]);
				}
			}
			if (font_name)
			{
				cmzn_fontmodule_id fontmodule = cmzn_graphics_module_get_fontmodule(
					scene_command_data->graphics_module);
				cmzn_font *new_font = cmzn_fontmodule_find_font_by_name(
					fontmodule, font_name);
				cmzn_fontmodule_destroy(&fontmodule);
				if (new_font)
				{
					cmzn_graphicspointattributes_set_font(point_attributes, new_font);
					cmzn_font_destroy(&new_font);
				}
				else
				{
					display_message(WARNING_MESSAGE, "Unknown font: %s", font_name);
				}
			}
		}

		STRING_TO_ENUMERATOR(cmzn_scenecoordinatesystem)(
			coordinate_system_string, &coordinate_system);
		cmzn_graphics_set_scenecoordinatesystem(graphics, coordinate_system);

		cmzn_graphics_render_polygon_mode render_polygon_mode;
		STRING_TO_ENUMERATOR(cmzn_graphics_render_polygon_mode)(render_polygon_mode_string, &render_polygon_mode);
		cmzn_graphics_set_render_polygon_mode(graphics, render_polygon_mode);

		STRING_TO_ENUMERATOR(cmzn_graphics_select_mode)(select_mode_string, &select_mode);
		cmzn_graphics_set_select_mode(graphics, select_mode);

		if ((0 != element_divisions_size) || (0 != circle_discretization))
		{
			cmzn_tessellationmodule_id tessellationmodule =
				cmzn_graphics_module_get_tessellationmodule(scene_command_data->graphics_module);
			cmzn_tessellation_id fixedTessellation =
				cmzn_tessellationmodule_find_or_create_fixed_tessellation(tessellationmodule,
					element_divisions_size, element_divisions, circle_discretization, tessellation);
			cmzn_graphics_set_tessellation(graphics, fixedTessellation);
			cmzn_tessellation_destroy(&fixedTessellation);
			cmzn_tessellationmodule_destroy(&tessellationmodule);
		}

		if (legacy_graphics_type == LEGACY_GRAPHIC_CYLINDERS)
		{
			// convert radius to diameter
			line_base_size[1] = line_base_size[0] = 2.0*constant_radius;
			line_scale_factors[1] = line_scale_factors[0] = 2.0*radius_scale_factor;
		}

		if (streamlines)
		{
			if (!stream_vector_field)
			{
				display_message(INFORMATION_MESSAGE,"Must specify a vector before any streamlines can be created");
			}
			if (return_code && seed_nodeset_name)
			{
				cmzn_fieldmodule_id field_module = cmzn_region_get_fieldmodule(scene_command_data->region);
				cmzn_nodeset_id seed_nodeset =
					cmzn_fieldmodule_find_nodeset_by_name(field_module, seed_nodeset_name);
				if (seed_nodeset || (fuzzy_string_compare(seed_nodeset_name, "none")))
				{
					if (graphics->seed_nodeset)
					{
						cmzn_nodeset_destroy(&graphics->seed_nodeset);
					}
					// take over reference:
					graphics->seed_nodeset = seed_nodeset;
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Unknown seed_nodeset %s", seed_nodeset_name);
					return_code = 0;
				}
				cmzn_fieldmodule_destroy(&field_module);
			}
			if ((graphics->seed_node_mesh_location_field && (!graphics->seed_nodeset)) ||
				((!graphics->seed_node_mesh_location_field) && graphics->seed_nodeset))
			{
				display_message(ERROR_MESSAGE,
					"Must specify both seed_nodeset and seed_node_mesh_location_field, or neither");
				return_code = 0;
			}
			if (return_code)
			{
				cmzn_graphics_streamlines_set_stream_vector_field(streamlines, stream_vector_field);
				cmzn_graphics_streamlines_set_track_length(streamlines, streamline_length);
				cmzn_graphics_streamlines_track_direction streamlines_track_direction;
				STRING_TO_ENUMERATOR(cmzn_graphics_streamlines_track_direction)(
					streamlines_track_direction_string, &streamlines_track_direction);
				cmzn_graphics_streamlines_set_track_direction(streamlines, streamlines_track_direction);
				// translate deprecated streamline_type to line_shape, and width to line_base_size[2] depending on type
				if (streamline_type_string)
				{
					for (int i = 0; i < 3; i++)
					{
						if (fuzzy_string_compare_same_length(streamline_type_string, streamline_type_strings[i]))
						{
							// set line_shape_string for processing below!
							line_shape_string = ENUMERATOR_STRING(cmzn_graphicslineattributes_shape_type)(streamline_type_to_line_shape_type[i].shape_type);
							line_base_size[0] = streamline_width;
							line_base_size[1] = streamline_width*streamline_type_to_line_shape_type[i].thickness_to_width_ratio;
							break;
						}
					}
				}
				else if (streamline_width != 0.0)
				{
					// handle width of legacy ribbon shape
					line_base_size[1] = line_base_size[0] = streamline_width;
				}
				if (streamlines_colour_data_type_string == no_streamlines_colour_data_type_string)
				{
					// now same as COLOUR_DATA_TYPE_FIELD: just has no field
					streamlines_colour_data_type = CMZN_GRAPHICS_STREAMLINES_COLOUR_DATA_TYPE_FIELD;
				}
				else
				{
					STRING_TO_ENUMERATOR(cmzn_graphics_streamlines_colour_data_type)(streamlines_colour_data_type_string, &streamlines_colour_data_type);
				}
				use_spectrum = (CMZN_GRAPHICS_STREAMLINES_COLOUR_DATA_TYPE_FIELD != streamlines_colour_data_type) || (data_field);
				cmzn_graphics_streamlines_set_colour_data_type(streamlines, streamlines_colour_data_type);
			}
		}
		if (use_spectrum)
		{
			if (spectrum)
			{
				cmzn_graphics_set_spectrum(graphics, spectrum);
			}
			else
			{
				cmzn_graphics_set_spectrum(graphics, scene_command_data->default_spectrum);
			}
		}
		else
		{
			cmzn_graphics_set_spectrum(graphics, static_cast<cmzn_spectrum *>(0));
		}

		if (line_attributes)
		{
			cmzn_graphicslineattributes_shape_type line_shape;
			STRING_TO_ENUMERATOR(cmzn_graphicslineattributes_shape_type)(line_shape_string, &line_shape);
			cmzn_graphicslineattributes_set_shape_type(line_attributes, line_shape);
			cmzn_graphicslineattributes_set_base_size(line_attributes, 2, line_base_size);
			cmzn_graphicslineattributes_set_scale_factors(line_attributes, 2, line_scale_factors);
			cmzn_graphicslineattributes_set_orientation_scale_field(line_attributes, line_orientation_scale_field);
		}
	}
	DESTROY(Option_table)(&option_table);
	if (!return_code)
	{
		/* parse error, help */
		cmzn_graphics_destroy(&(modify_scene_data->graphics));
	}
	if (glyph_name)
	{
		DEALLOCATE(glyph_name);
	}
	cmzn_field_destroy(&coordinate_field);
	cmzn_field_destroy(&data_field);
	cmzn_field_destroy(&isoscalar_field);
	cmzn_field_destroy(&line_orientation_scale_field);
	cmzn_field_destroy(&label_field);
	cmzn_field_destroy(&orientation_scale_field);
	cmzn_field_destroy(&stream_vector_field);
	cmzn_field_destroy(&subgroup_field);
	cmzn_field_destroy(&signed_scale_field);
	cmzn_field_destroy(&tessellation_field);
	cmzn_field_destroy(&texture_coordinate_field);
	cmzn_field_destroy(&sample_density_field);
	if (sample_location)
	{
		DEALLOCATE(sample_location);
	}
	if (font_name)
	{
		DEALLOCATE(font_name);
	}
	if (seed_nodeset_name)
	{
		DEALLOCATE(seed_nodeset_name);
	}
	cmzn_graphics_destroy(&graphics);
	if (isovalues)
		DEALLOCATE(isovalues);
	cmzn_graphics_contours_destroy(&contours);
	cmzn_graphics_streamlines_destroy(&streamlines);
	cmzn_graphicslineattributes_destroy(&line_attributes);
	cmzn_graphicspointattributes_destroy(&point_attributes);
	cmzn_graphicssamplingattributes_destroy(&sampling);
	cmzn_spectrum_destroy(&spectrum);
	cmzn_tessellation_destroy(&tessellation);
	cmzn_material_destroy(&material);
	cmzn_material_destroy(&selected_material);
	DEALLOCATE(name);
	return return_code;
}

int gfx_modify_scene_contours(struct Parse_state *state,
	void *modify_scene_data_void,void *scene_command_data_void)
{
	const char *help_text =
		"Create contours i.e. isosurfaces in volume elements <domain_elements_3d> or isolines in face or surface elements "
		"<domain_elements_2d>.  "
		"The isosurface will be generated at the values in the elements where <iso_scalar> equals the iso values specified.  "
		"The iso values can be specified either as a list with the <iso_values> option "
		"or by specifying <range_number_of_iso_values>, <first_iso_value> and <last_iso_value>.  "
		"The <as> parameter allows a name to be specified for this setting.  "
		"The <coordinate> parameter optionally overrides the groups default coordinate field.  "
		"If a <data> field is specified then the <spectrum> is used to render the data values as colour on the generated isosurface.  "
		"If a <decimation_threshold> is specified then the resulting iso_surface will be decimated according to the threshold.  "
		"If <delete> is specified then if the graphics matches an existing setting (either by parameters or name) then it will be removed.  "
		"If <exterior> is specified then only faces with one parent will be selected when <use_faces> is specified.  "
		"If <face> is specified then only that face will be selected when <use_faces> is specified.  "
		"The <material> is used to render the surface.  "
		"You can specify the <position> the graphics has in the graphics list.  "
		"You can specify the <line_width>, this option only applies when <use_faces> is specified.  "
		"You can render a mesh as solid <render_shaded> or as a wireframe <render_wireframe>.  "
		"If <select_on> is active then the element tool will select the elements the iso_surface was generated from.  "
		"If <no_select> is active then the iso_surface cannot be selected.  "
		"If <draw_selected> is active then iso_surfaces will only be generated in elements that are selected.  "
		"Conversely, if <draw_unselected> is active then iso_surfaces will only be generated in elements that are not selected.  "
		"The <texture_coordinates> are used to lay out a texture if the <material> contains a texture.  "
		"A graphics can be made <visible> or <invisible>.  ";

	return gfx_modify_scene_graphics(state, CMZN_GRAPHICS_TYPE_CONTOURS,
		LEGACY_GRAPHIC_NONE, help_text,
		reinterpret_cast<Modify_scene_data *>(modify_scene_data_void),
		reinterpret_cast<Scene_command_data *>(scene_command_data_void));
}

int gfx_modify_scene_cylinders(struct Parse_state *state,
	void *modify_scene_data_void,void *scene_command_data_void)
{
	return gfx_modify_scene_graphics(state, CMZN_GRAPHICS_TYPE_LINES,
		LEGACY_GRAPHIC_CYLINDERS, /*help_text*/(const char *)0,
		reinterpret_cast<Modify_scene_data *>(modify_scene_data_void),
		reinterpret_cast<Scene_command_data *>(scene_command_data_void));
}

int gfx_modify_scene_data_points(struct Parse_state *state,
	void *modify_scene_data_void,void *scene_command_data_void)
{
	const char *help_text = "Deprecated; use points with domain_data option instead.";
	return gfx_modify_scene_graphics(state, CMZN_GRAPHICS_TYPE_POINTS,
		LEGACY_GRAPHIC_DATA_POINTS, help_text,
		reinterpret_cast<Modify_scene_data *>(modify_scene_data_void),
		reinterpret_cast<Scene_command_data *>(scene_command_data_void));
}

int gfx_modify_scene_element_points(struct Parse_state *state,
	void *modify_scene_data_void,void *scene_command_data_void)
{
	const char *help_text = "Deprecated; use points with domain_elements* option instead.";
	return gfx_modify_scene_graphics(state, CMZN_GRAPHICS_TYPE_POINTS,
		LEGACY_GRAPHIC_ELEMENT_POINTS, help_text,
		reinterpret_cast<Modify_scene_data *>(modify_scene_data_void),
		reinterpret_cast<Scene_command_data *>(scene_command_data_void));
}

int gfx_modify_scene_iso_surfaces(struct Parse_state *state,
	void *modify_scene_data_void,void *scene_command_data_void)
{
	const char *help_text = "Deprecated; use contours with domain_elements* option instead.";
	return gfx_modify_scene_graphics(state, CMZN_GRAPHICS_TYPE_CONTOURS,
		LEGACY_GRAPHIC_ISO_SURFACES, help_text,
		reinterpret_cast<Modify_scene_data *>(modify_scene_data_void),
		reinterpret_cast<Scene_command_data *>(scene_command_data_void));
}

int gfx_modify_scene_lines(struct Parse_state *state,
	void *modify_scene_data_void,void *scene_command_data_void)
{
	return gfx_modify_scene_graphics(state, CMZN_GRAPHICS_TYPE_LINES,
		LEGACY_GRAPHIC_NONE, /*help_text*/(const char *)0,
		reinterpret_cast<Modify_scene_data *>(modify_scene_data_void),
		reinterpret_cast<Scene_command_data *>(scene_command_data_void));
}

int gfx_modify_scene_node_points(struct Parse_state *state,
	void *modify_scene_data_void,void *scene_command_data_void)
{
	const char *help_text = "Deprecated; use points with domain_nodes option instead.";
	return gfx_modify_scene_graphics(state, CMZN_GRAPHICS_TYPE_POINTS,
		LEGACY_GRAPHIC_NODE_POINTS, help_text,
		reinterpret_cast<Modify_scene_data *>(modify_scene_data_void),
		reinterpret_cast<Scene_command_data *>(scene_command_data_void));
}

int gfx_modify_scene_point(struct Parse_state *state,
	void *modify_scene_data_void,void *scene_command_data_void)
{
	const char *help_text = "Deprecated; use points with domain_point option instead.";
	return gfx_modify_scene_graphics(state, CMZN_GRAPHICS_TYPE_POINTS,
		LEGACY_GRAPHIC_POINT, help_text,
		reinterpret_cast<Modify_scene_data *>(modify_scene_data_void),
		reinterpret_cast<Scene_command_data *>(scene_command_data_void));
}

int gfx_modify_scene_streamlines(struct Parse_state *state,
	void *modify_scene_data_void,void *scene_command_data_void)
{
	return gfx_modify_scene_graphics(state, CMZN_GRAPHICS_TYPE_STREAMLINES,
		LEGACY_GRAPHIC_NONE, /*help_text*/(const char *)0,
		reinterpret_cast<Modify_scene_data *>(modify_scene_data_void),
		reinterpret_cast<Scene_command_data *>(scene_command_data_void));
}

int gfx_modify_scene_surfaces(struct Parse_state *state,
	void *modify_scene_data_void,void *scene_command_data_void)
{
	return gfx_modify_scene_graphics(state, CMZN_GRAPHICS_TYPE_SURFACES,
		LEGACY_GRAPHIC_NONE, /*help_text*/(const char *)0,
		reinterpret_cast<Modify_scene_data *>(modify_scene_data_void),
		reinterpret_cast<Scene_command_data *>(scene_command_data_void));
}

int gfx_modify_scene_points(struct Parse_state *state,
	void *modify_scene_data_void,void *scene_command_data_void)
{
	return gfx_modify_scene_graphics(state, CMZN_GRAPHICS_TYPE_POINTS,
		LEGACY_GRAPHIC_NONE, /*help_text*/(const char *)0,
		reinterpret_cast<Modify_scene_data *>(modify_scene_data_void),
		reinterpret_cast<Scene_command_data *>(scene_command_data_void));
}

