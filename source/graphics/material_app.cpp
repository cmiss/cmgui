/* OpenCMISS-Cmgui Application
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */


#include "opencmiss/zinc/zincconfigure.h"
#include "opencmiss/zinc/material.h"
#include "opencmiss/zinc/region.h"
#if 1
#include "configure/cmgui_configure.h"
#endif /* defined (BUILD_WITH_CMAKE) */
#include "command/parser.h"
#include "three_d_drawing/graphics_buffer.h"
#include "general/message.h"
#include "computed_field/computed_field_image.h"
#include "graphics/graphics_module.hpp"
#include "graphics/render_gl.h"
#include "graphics/material.hpp"
#include "graphics/material_app.h"
#include "graphics/shader_program.hpp"


//struct Material_program_uniform *CREATE(Material_program_uniform)(char *name)

//static struct Material_program_uniform *CREATE(Material_program_uniform)(char *name)

int gfx_create_material(struct Parse_state *state,
	void *dummy_to_be_modified, void *materialmodule_void)
/*******************************************************************************
LAST MODIFIED : 20 November 2003

DESCRIPTION :
Shifted from command/cmiss.c now that there is a material module.
If the material already exists, then behaves like gfx modify material.
==============================================================================*/
{
	const char *current_token;
	int material_is_new,return_code;
	cmzn_material *material;
	struct Material_module_app *app_module = 0;
	struct cmzn_materialmodule *materialmodule = 0;

	ENTER(gfx_create_material);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		current_token=state->current_token;
		if (current_token)
		{
			app_module=(struct Material_module_app *)materialmodule_void;
			if (app_module && (NULL != (materialmodule =
				(struct cmzn_materialmodule *)app_module->module)))
			{
				if (strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
				{
					/* if there is an existing material of that name, just modify it */
					material=cmzn_materialmodule_find_material_by_name(materialmodule, current_token);
					if (!material)
					{
						material = cmzn_material_create_private();
						cmzn_material_set_name(material, current_token);
						if (material)
						{
							/*???DB.  Temporary */
							cmzn_material_id defaultMaterial =
								cmzn_materialmodule_get_default_material(materialmodule);
							MANAGER_COPY_WITHOUT_IDENTIFIER(cmzn_material,name)(material,
								defaultMaterial);
							if (defaultMaterial)
								cmzn_material_destroy(&defaultMaterial);
						}
						material_is_new=1;
					}
					else
					{
						material_is_new=0;
					}
					if (material)
					{
						shift_Parse_state(state,1);
						if (state->current_token)
						{
							return_code=modify_Graphical_material(state,(void *)material,
								materialmodule_void);
						}
						else
						{
							return_code=1;
						}
						if (material_is_new)
						{
							cmzn_material_set_managed(material, true);
							ADD_OBJECT_TO_MANAGER(cmzn_material)(material,
								materialmodule->getManager());
						}
						cmzn_material_destroy(&material);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"gfx_create_material.  Error creating material");
						return_code=0;
					}
				}
				else
				{
					return_code=modify_Graphical_material(state,(void *)NULL,
						materialmodule_void);
					return_code=1;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_create_material.  Missing materialmodule_void");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Missing material name");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_create_material.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_create_material */


#if defined (WX_USER_INTERFACE)
#endif /* (WX_USER_INTERFACE) */

cmzn_field_image_id Material_image_texture_get_field(struct Material_image_texture *image_texture);

int Material_image_texture_set_field(struct Material_image_texture *image_texture,
	cmzn_field_image_id field);

int set_Material_image_texture(struct Parse_state *state,void *material_image_texture_void,
		void *root_region_void)
{
	const char *current_token;
	int return_code;
	struct cmzn_region *root_region = NULL;
	struct Computed_field *temp_field = NULL;
	struct Material_image_texture *image_texture;

	ENTER(set_Material_image_field);
	if (state)
	{
		current_token=state->current_token;
		if (current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				image_texture=(struct Material_image_texture *)material_image_texture_void;
				root_region = (struct cmzn_region *)root_region_void;
				if (image_texture	&& root_region)
				{
					if (fuzzy_string_compare(current_token,"NONE"))
					{
						return_code = Material_image_texture_set_field(image_texture, 0);;
					}
					else
					{
						struct cmzn_region *region = NULL;
						char *region_path = NULL, *field_name = NULL;
						if (cmzn_region_get_partial_region_path(root_region,
							current_token, &region, &region_path, &field_name))
						{
							cmzn_fieldmodule *field_module = cmzn_region_get_fieldmodule(region);
							if (field_name && (strlen(field_name) > 0) &&
								(strchr(field_name, CMZN_REGION_PATH_SEPARATOR_CHAR)	== NULL))
							{
								temp_field = cmzn_fieldmodule_find_field_by_name(field_module,
									field_name);
								if (temp_field &&
										!Computed_field_is_image_type(temp_field,0))
								{
									DEACCESS(Computed_field)(&temp_field);
									display_message(ERROR_MESSAGE,
										"set_Material_image_field.  Field specify does not contain image "
										"information.");
									return_code=0;
								}
							}
							else
							{
								if (field_name)
								{
									display_message(ERROR_MESSAGE,
										"set_Material_image_texture:  Invalid region path or texture field name '%s'", field_name);
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"set_Material_image_texture:  Missing texture field name or name matches child region '%s'", current_token);
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
						if (temp_field)
						{
							cmzn_field_image_id image_field = cmzn_field_cast_image(temp_field);
							Material_image_texture_set_field(image_texture,	image_field);
							cmzn_field_image_destroy(&image_field);
							return_code=1;
							DEACCESS(Computed_field)(&temp_field);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"set_Material_image_field.  Image field does not exist");
							return_code=0;
						}
					}
					shift_Parse_state(state,1);
				}
				else
				{
					display_message(ERROR_MESSAGE,"set_Material_image_field.  Invalid argument(s)");
					return_code=0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE," IMAGE_NAME|none");
				image_texture=(struct Material_image_texture *)material_image_texture_void;
				if (image_texture)
				{
					temp_field = cmzn_field_image_base_cast(image_texture->field);
					if (temp_field)
					{
						char *temp_name = cmzn_field_get_name(temp_field);
						display_message(INFORMATION_MESSAGE,"[%s]",temp_name);
						DEALLOCATE(temp_name);
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
			display_message(ERROR_MESSAGE,"Missing field name");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_Material_image_field.  Missing state");
		return_code=0;
	}

	LEAVE;

	return (return_code);
} /* set_Material_image_field */

#include "graphics/colour_app.h"
#include "graphics/spectrum_app.h"
#include "graphics/graphics_library.h"

int Material_program_uniform_set_float_vector(struct Material_program_uniform *uniform,
	unsigned int number_of_values, double *values);

int modify_Graphical_material(struct Parse_state *state,void *material_void,
	void *materialmodule_void)
/*******************************************************************************
LAST MODIFIED : 4 July 2007

DESCRIPTION :
==============================================================================*/
{
	const char *current_token;
	char bump_mapping_flag, colour_lookup_red_flag, colour_lookup_green_flag,
		colour_lookup_blue_flag, colour_lookup_alpha_flag,
		lit_volume_finite_difference_normal_flag,
		lit_volume_intensity_normal_texture_flag, lit_volume_scale_alpha_flag,
		normal_mode_flag, per_pixel_mode_flag,
		*fragment_program_string, *vertex_program_string, *geometry_program_string,
		*uniform_name;
	/*	enum Spectrum_colour_components spectrum_colour_components; */
	int /*dimension,*/ lit_volume_normal_scaling_number,
		number_of_spectrum_components, number_of_uniform_values,
		process, return_code;
	cmzn_material *material_to_be_modified,
		*material_to_be_modified_copy;
	struct cmzn_materialmodule *materialmodule = 0;
	struct Material_module_app *app_module = 0;
	struct Option_table *help_option_table, *option_table, *mode_option_table;
	double *uniform_values;

	ENTER(modify_Graphical_material);
	if (state)
	{
		app_module=(struct Material_module_app *)materialmodule_void;
		if (app_module && (NULL != (materialmodule =
			(struct cmzn_materialmodule *)app_module->module)))
		{
			current_token=state->current_token;
			if (current_token)
			{
				process=0;
				material_to_be_modified=(cmzn_material *)material_void;
				if (material_to_be_modified)
				{
					if (IS_MANAGED(cmzn_material)(material_to_be_modified,
						materialmodule->getManager()))
					{
						material_to_be_modified_copy = cmzn_material_create_private();
						cmzn_material_set_name(material_to_be_modified_copy, "copy");
						if (material_to_be_modified_copy)
						{
							MANAGER_COPY_WITHOUT_IDENTIFIER(cmzn_material,name)(
								material_to_be_modified_copy,material_to_be_modified);
							process=1;
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"modify_Graphical_material.  Could not create material copy");
							return_code=0;
						}
					}
					else
					{
						material_to_be_modified_copy=material_to_be_modified;
						material_to_be_modified=(cmzn_material *)NULL;
						process=1;
					}
				}
				else
				{
					if (strcmp(PARSER_HELP_STRING,current_token)&&
						strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
					{
						material_to_be_modified=FIND_BY_IDENTIFIER_IN_MANAGER(cmzn_material,name)(current_token,
							materialmodule->getManager());
						if (material_to_be_modified)
						{
							return_code=shift_Parse_state(state,1);
							if (return_code)
							{
								material_to_be_modified_copy = cmzn_material_create_private();
								cmzn_material_set_name(material_to_be_modified_copy, "copy");
								if (material_to_be_modified_copy)
								{
									MANAGER_COPY_WITHOUT_IDENTIFIER(cmzn_material,name)(
										material_to_be_modified_copy,material_to_be_modified);
									process=1;
								}
								else
								{
									display_message(ERROR_MESSAGE,
									"modify_Graphical_material.  Could not create material copy");
									return_code=0;
								}
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,"Unknown material : %s",
								current_token);
							display_parse_state_location(state);
							return_code=0;
						}
					}
					else
					{
						material_to_be_modified = cmzn_material_create_private();
						cmzn_material_set_name(material_to_be_modified, "help");
						if (material_to_be_modified)
						{
							cmzn_material *default_material =
								cmzn_materialmodule_get_default_material(materialmodule);
							if (default_material)
							{
								MANAGER_COPY_WITHOUT_IDENTIFIER(cmzn_material,name)(
									material_to_be_modified,default_material);
								cmzn_material_destroy(&default_material);
							}
							help_option_table = CREATE(Option_table)();
							Option_table_add_entry(help_option_table, "MATERIAL_NAME",
								material_to_be_modified, materialmodule_void,
								modify_Graphical_material);
							return_code=Option_table_parse(help_option_table, state);
							DESTROY(Option_table)(&help_option_table);
							cmzn_material_destroy(&material_to_be_modified);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"modify_Graphical_material.  Could not create dummy material");
							return_code=0;
						}
					}
				}
				if (process)
				{
					float material_alpha = (float)material_to_be_modified_copy->alpha;
					float material_shininess = (float)material_to_be_modified_copy->shininess;
					bump_mapping_flag = 0;
					normal_mode_flag = 0;
					per_pixel_mode_flag = 0;
					colour_lookup_red_flag = 0;
					colour_lookup_green_flag = 0;
					colour_lookup_blue_flag = 0;
					colour_lookup_alpha_flag = 0;
					lit_volume_intensity_normal_texture_flag = 0;
					lit_volume_finite_difference_normal_flag = 0;
					lit_volume_scale_alpha_flag = 0;
					vertex_program_string = (char *)NULL;
					geometry_program_string = (char *)NULL;
					fragment_program_string = (char *)NULL;
					uniform_name = (char *)NULL;
					number_of_uniform_values = 0;
					uniform_values = (double *)NULL;
					option_table = CREATE(Option_table)();
					Option_table_add_help(option_table,
						"The material controls how pixels will be rendered on the "
						"screen.  The initial mode of operation reflects the "
						"standard gouraud shading model usual within OpenGL, "
						"this is called <normal_mode>. "
						"This has rgb colour values for <diffuse>, <ambient> "
						"<emission> and <specular> colours.  The <ambient> "
						"colour is the unlit colour, the <diffuse> colour "
						"interacts with the light and is the lit surface, "
						"the <specular> colour is used for the glossy "
						"highlights, the spread of which is specified by the"
						"<shininess>.  The <alpha> controls the transparency "
						"of the material.  See a/a1."
						"If a <texture> is specified then this is combined with "
						"the calculated gouraud colour according to the textures "
						"rendering mode and the texture coordinates.  "
						"Additional support is provided for more accurate "
						"rendering using phong shading calculated at every pixel, "
						"called <per_pixel_lighting>.  This is implemented using "
						"the ARB_vertex_program and ARB_fragment_program OpenGL "
						"extensions.  The program only implements a single point "
						"light so you should set the default light to a point light "
						"at the position you want.  See a/per_pixel_lighting.  "
						"<bump_mapping> uses a <secondary_texture> to specify "
						"a perturbation to the normal, giving a smooth model "
						"a much more detailed surface appearance.  "
						"Also demonstrated in a/per_pixel_lighting.  "
						"A <colour_lookup_spectrum> takes the calculated colour values "
						"and further modifies them by using the specified subset "
						"of colour values specified by <colour_lookup_red>, "
						"<colour_lookup_green>, "
						"<colour_lookup_blue> and <colour_lookup_alpha>, as inputs "
						"to a 1, 2 or 3 component spectrum.  Depending on the "
						"spectrum this will override either the rgb colour, "
						"the alpha value or both rgb and alpha.  "
						"If the number of input components used in the spectrum "
						"matches the number of components specified then a texture "
						"of this dimension will be used and evaluated for each tensor "
						"product combination.  If only a 1 component spectrum is "
						"specified then it will be applied independently to each input "
						"component specified.  This spectrum "
						"can be modified to quickly change the appearance of a "
						"large volume dataset, see example a/indexed_volume.  "
						"A lit volume uses a per voxel normal to calculate the "
						"phong lighting model at each pixel.  The normal can "
						"be specified by encoding it into the blue, green and "
						"alpha channels of an input texture (the red channel "
						"being the intensity) <lit_volume_intensity_normal_texture>."
						"Alternatively it can be estimated on the fly by applying "
						"a finite difference operator to the pixel intensities. "
						"<lit_volume_finite_difference_normal>."
						"A <lit_volume_normal_scaling> can be applied, modifying "
						"the estimated normal by scaling it.  The normal is used "
						"as if it is a coordinate normal and so the texture "
						"coordinates must line up with the geometrical coordinates. "
						"The <lit_volume_normal_scaling> can be used to account for "
						"when the texture coordinates are not equally matched to "
						"the geometrical coordinates. "
						"The magnitude of the <lit_volume_normal_scaling> will only "
						"affect the optional following parameter, "
						"<lit_volume_scale_alpha>, scaling the alpha attenuation. "
						"Optionally with either normal, the magnitude of that normal"
						"can multiply the calculated alpha value, "
						"<lit_volume_scale_alpha> making those pixels with small "
						"gradients more transparent.  See a/volume_render. "
						"<secondary_texture>, <third_texture> and <fourth_texture> will "
						"be blended with the first <texture> according to the multitexture "
						"rules controlled by each textures combine mode (such as modulate or add). "
						"Specifying a <vertex_program_string> and <fragment_program_string> allows "
						"any arbitrary program to be loaded, overriding the one that is generated automatically, "
						"no checks for consistency with textures or inputs are made.  Both must be specified together. "
						"Optional <geometry_program_string> is also available, which is only available on "
						"relatively newer hardware, it must be used with <vertex_program_string> and "
						"<fragment_program_string> and only works with surface at the moment. Specifying "
						"a <uniform_name> and <uniform_value> allows any uniform qualified variables in"
						"any arbitrary program to be set. At the moment only float type is supported.");

					Option_table_add_entry(option_table, "alpha",
						&(material_alpha), NULL,
						set_float_0_to_1_inclusive);
					Option_table_add_entry(option_table, "ambient",
						&(material_to_be_modified_copy->ambient), NULL,
						set_Colour);
					Option_table_add_char_flag_entry(option_table,
						"bump_mapping", &bump_mapping_flag);
					Option_table_add_char_flag_entry(option_table,
						"colour_lookup_alpha", &colour_lookup_alpha_flag);
					Option_table_add_char_flag_entry(option_table,
						"colour_lookup_blue", &colour_lookup_blue_flag);
					Option_table_add_char_flag_entry(option_table,
						"colour_lookup_green", &colour_lookup_green_flag);
					Option_table_add_char_flag_entry(option_table,
						"colour_lookup_red", &colour_lookup_red_flag);
					Option_table_add_entry(option_table, "colour_lookup_spectrum",
						&(material_to_be_modified_copy->spectrum),
						materialmodule->getSpectrumManager(),
						set_Spectrum);
					Option_table_add_entry(option_table, "diffuse",
						&(material_to_be_modified_copy->diffuse), NULL,
						set_Colour);
					Option_table_add_entry(option_table, "emission",
						&(material_to_be_modified_copy->emission), NULL,
						set_Colour);
					Option_table_add_entry(option_table, "fourth_texture",
						&(material_to_be_modified_copy->fourth_image_texture),
						(cmzn_region *)app_module->region,
						set_Material_image_texture);
					Option_table_add_name_entry(option_table, "fragment_program_string",
						&fragment_program_string);
					Option_table_add_char_flag_entry(option_table,
						"lit_volume_intensity_normal_texture",
						&lit_volume_intensity_normal_texture_flag);
					Option_table_add_char_flag_entry(option_table,
						"lit_volume_finite_difference_normal",
						&lit_volume_finite_difference_normal_flag);
					lit_volume_normal_scaling_number = 3;
					Option_table_add_double_vector_entry(option_table,
						"lit_volume_normal_scaling",
						material_to_be_modified_copy->lit_volume_normal_scaling,
						&lit_volume_normal_scaling_number);
					Option_table_add_char_flag_entry(option_table,
						"lit_volume_scale_alpha",
						&lit_volume_scale_alpha_flag);
					mode_option_table = CREATE(Option_table)();
					Option_table_add_char_flag_entry(mode_option_table,
						"normal_mode", &normal_mode_flag);
					Option_table_add_char_flag_entry(mode_option_table,
						"per_pixel_mode", &per_pixel_mode_flag);
					Option_table_add_suboption_table(option_table, mode_option_table);
					Option_table_add_entry(option_table, "secondary_texture",
						&(material_to_be_modified_copy->second_image_texture),
						(cmzn_region *)app_module->region,
						set_Material_image_texture);
					Option_table_add_entry(option_table, "shininess",
						&(material_shininess), NULL,
						set_float_0_to_1_inclusive);
					Option_table_add_entry(option_table, "specular",
						&(material_to_be_modified_copy->specular), NULL,
						set_Colour);
					Option_table_add_entry(option_table, "texture",
						&(material_to_be_modified_copy->image_texture),
						(cmzn_region *)app_module->region,
						set_Material_image_texture);
					Option_table_add_entry(option_table, "third_texture",
						&(material_to_be_modified_copy->third_image_texture),
						(cmzn_region *)app_module->region,
						set_Material_image_texture);
					Option_table_add_name_entry(option_table, "vertex_program_string",
						&vertex_program_string);
					Option_table_add_name_entry(option_table, "geometry_program_string",
						&geometry_program_string);
					Option_table_add_name_entry(option_table, "uniform_name",
						&uniform_name);
					Option_table_add_variable_length_double_vector_entry(option_table,
						"uniform_values", &number_of_uniform_values, &uniform_values);
					return_code=Option_table_multi_parse(option_table, state);
					if (return_code)
					{
						material_to_be_modified_copy->alpha = material_alpha;
						material_to_be_modified_copy->shininess = material_shininess;
						if (normal_mode_flag + per_pixel_mode_flag > 1)
						{
							display_message(ERROR_MESSAGE,
								"Specify only one of normal_mode/per_pixel_mode.");
							return_code = 0;
						}
						if (fragment_program_string || vertex_program_string)
						{
							if (normal_mode_flag)
							{
								display_message(ERROR_MESSAGE,
									"vertex_program_string and fragment_program_string"
									"imply per_pixel_mode and cannot be used with normal_mode.");
								return_code = 0;
							}
							per_pixel_mode_flag = 1;
						}
						enum cmzn_shaderprogram_type shaderprogram_type = cmzn_shaderprogram_get_type(material_to_be_modified_copy->program);
						if ((!material_to_be_modified_copy->program ||
							(0 != shaderprogram_type))
							&& ((fragment_program_string && !vertex_program_string) ||
							(vertex_program_string && !fragment_program_string)))
						{
							display_message(ERROR_MESSAGE,
								"If you specify one of vertex_program_string or "
								"fragment_program_string you must specify both.");
							return_code = 0;
						}
						if ((colour_lookup_alpha_flag + colour_lookup_blue_flag +
								colour_lookup_green_flag + colour_lookup_red_flag) > 0)
						{
							if (!material_to_be_modified_copy->spectrum)
							{
								display_message(ERROR_MESSAGE,
									"If you specify a colour lookup colour you must also specify a colour_lookup_spectrum.");
								return_code = 0;
							}
							else
							{
								number_of_spectrum_components =
									Spectrum_get_number_of_data_components(material_to_be_modified_copy->spectrum);
								if ((colour_lookup_alpha_flag + colour_lookup_blue_flag +
										colour_lookup_green_flag + colour_lookup_red_flag) ==
									number_of_spectrum_components)
								{
									/* OK */
								}
								else if (1 == number_of_spectrum_components)
								{
									/* Lookup each component specified in the 1D spectra only */
									/* Also OK */
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"Either your spectrum should have 1 component, or the number of components must match the number of colour lookups specfied (colour_lookup_alpha, colour_lookup_blue, colour_lookup_green_flag or colour_lookup_red_flag).");
									return_code = 0;
								}
							}
							if ((colour_lookup_alpha_flag + colour_lookup_blue_flag +
									colour_lookup_green_flag + colour_lookup_red_flag) > 3)
							{
								display_message(ERROR_MESSAGE,
									"A maximum of three colours (3 of colour_lookup_alpha, colour_lookup_blue, colour_lookup_green_flag or colour_lookup_red_flag) can be used as input to a colour_lookup_spectrum.");
								return_code = 0;
							}
						}
						else if (material_to_be_modified_copy->spectrum &&
							(!(material_to_be_modified_copy->program) ||
							!(shaderprogram_type & SHADER_PROGRAM_CLASS_DEPENDENT_TEXTURE_INPUTS)))
						{
							display_message(ERROR_MESSAGE,
								"If you specify a colour_lookup_spectrum you must also specify the input colours. (1 to 3 of colour_lookup_alpha, colour_lookup_blue, colour_lookup_green_flag and colour_lookup_red_flag)");
							return_code = 0;
						}
						/* Don't check run time availability yet as we may
							not have initialised any openGL display yet */
						if (material_to_be_modified_copy->second_image_texture.texture)
						{
#if defined (GL_VERSION_1_3)
							if (!Graphics_library_tentative_check_extension(GL_VERSION_1_3))
							{
								display_message(ERROR_MESSAGE,
									"Multitexture requires OpenGL version 1.3 or better which is "
									"not available on this display.");
								DEACCESS(Texture)(&material_to_be_modified_copy->second_image_texture.texture);
								return_code = 0;
							}
#else /* defined (GL_VERSION_1_3) */
							display_message(ERROR_MESSAGE,
								"Multitexture requires OpenGL version 1.3 or better which was "
								"not compiled into this executable.");
							DEACCESS(Texture)(&material_to_be_modified_copy->second_image_texture.texture);
							return_code = 0;
#endif /* defined (GL_VERSION_1_3) */
						}
						if (material_to_be_modified_copy->spectrum)
						{
#if defined (GL_VERSION_1_3)
							if (!Graphics_library_tentative_check_extension(GL_VERSION_1_3))
							{
								display_message(ERROR_MESSAGE,
									"A colour lookup spectrum requires OpenGL version 1.3 or better which is "
									"not available on this display.");
								DEACCESS(cmzn_spectrum)(&material_to_be_modified_copy->spectrum);
								return_code = 0;
							}
#else /* defined (GL_VERSION_1_3) */
							display_message(ERROR_MESSAGE,
								"A colour lookup spectrum requires OpenGL version 1.3 or better which was "
								"not compiled into this executable.");
							DEACCESS(cmzn_spectrum)(&material_to_be_modified_copy->spectrum);
							return_code = 0;
#endif /* defined (GL_VERSION_1_3) */
						}
						cmzn_shadermodule_id shadermodule = static_cast<cmzn_shadermodule_id>(app_module->shadermodule);
						if (normal_mode_flag)
						{
							material_deaccess_shader_program(material_to_be_modified_copy);
						}
						else if (per_pixel_mode_flag || material_to_be_modified_copy->program)
						{
#if defined GL_ARB_vertex_program && defined GL_ARB_fragment_program || defined GL_VERSION_2_0
							return_code = 0;
#if defined GL_ARB_fragment_program && defined GL_ARB_vertex_program
							if (Graphics_library_tentative_check_extension(GL_ARB_fragment_program) &&
									Graphics_library_tentative_check_extension(GL_ARB_vertex_program))
							{
								return_code = 1;
							}
#endif /* defined GL_ARB_vertex_program && defined GL_ARB_fragment_program */
#if defined (GL_VERSION_2_0)
							if (Graphics_library_tentative_check_extension(GL_shading_language))
							{
								return_code = 1;
							}
#endif // defined (GL_VERSION_2_0)
							if (return_code)
							{
								if (vertex_program_string && fragment_program_string)
								{
									return_code = Material_set_shader_program_strings(shadermodule,
										material_to_be_modified_copy, vertex_program_string,
										fragment_program_string, geometry_program_string);
								}
								else if (material_to_be_modified_copy->program &&
										(0 == shaderprogram_type))
								{
									/* Do nothing as we just keep the existing program */
								}
								else
								{
									return_code = set_shader_program_type(shadermodule, material_to_be_modified_copy,
										bump_mapping_flag, colour_lookup_red_flag,colour_lookup_green_flag,
										colour_lookup_blue_flag, colour_lookup_alpha_flag,
										lit_volume_intensity_normal_texture_flag, lit_volume_finite_difference_normal_flag,
										lit_volume_scale_alpha_flag, return_code);
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"Program or shader based materials require GL_ARB_vertex_program"
									" and GL_ARB_fragment_program extensions or OpenGL 2.0 shading language support"
									" which is not available on this display.");
							}
#else // defined GL_ARB_vertex_program && defined GL_ARB_fragment_program || defined GL_VERSION_2_0
							display_message(ERROR_MESSAGE,
								"Program or shader based materials require GL_ARB_vertex_program"
								" and GL_ARB_fragment_program extensions or OpenGL 2.0 shading language support"
								" which was not compiled into this program.");
							return_code = 0;
#endif // defined GL_ARB_vertex_program && defined GL_ARB_fragment_program || defined GL_VERSION_2_0
						}
						if (uniform_name && number_of_uniform_values && uniform_values)
						{
							cmzn_shaderuniforms_id shaderuniforms = cmzn_material_get_shaderuniforms(material_to_be_modified_copy);
							const bool newShaderuniforms = (!shaderuniforms);
							if (newShaderuniforms)
							{
								shaderuniforms = cmzn_shadermodule_create_shaderuniforms(shadermodule);
								cmzn_shaderuniforms_set_name(shaderuniforms, material_to_be_modified_copy->name);  // set material name to help identify
							}
							else
							{
								cmzn_shaderuniforms_remove_uniform(shaderuniforms, uniform_name);
							}
							cmzn_shaderuniforms_add_uniform_real(shaderuniforms, uniform_name, number_of_uniform_values, uniform_values);
							if (newShaderuniforms)
							{
								cmzn_material_set_shaderuniforms(material_to_be_modified_copy, shaderuniforms);
							}
							cmzn_shaderuniforms_destroy(&shaderuniforms);
						}
						if (return_code)
						{
							if (material_to_be_modified)
							{
								MANAGER_MODIFY_NOT_IDENTIFIER(cmzn_material,name)(
									material_to_be_modified,material_to_be_modified_copy,
									materialmodule->getManager());
								material_copy_bump_mapping_and_per_pixel_lighting_flag(
									 material_to_be_modified_copy, material_to_be_modified);
								cmzn_material_destroy(&material_to_be_modified_copy);
							}
							else
							{
								material_to_be_modified=material_to_be_modified_copy;
							}
						}
						else
						{
							if (material_to_be_modified)
							{
								cmzn_material_destroy(&material_to_be_modified_copy);
							}
						}
					}
					if (vertex_program_string)
					{
						DEALLOCATE(vertex_program_string);
					}
					if (fragment_program_string)
					{
						DEALLOCATE(fragment_program_string);
					}
					if (geometry_program_string)
					{
						DEALLOCATE(geometry_program_string);
					}
					if (uniform_name)
					{
						DEALLOCATE(uniform_name);
					}
					if (uniform_values)
					{
						DEALLOCATE(uniform_values);
					}
					DESTROY(Option_table)(&option_table);
				}
			}
			else
			{
				if (material_void)
				{
					display_message(ERROR_MESSAGE,"Missing material modifications");
				}
				else
				{
					display_message(ERROR_MESSAGE,"Missing material name");
				}
				display_parse_state_location(state);
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
		"modify_Graphical_material.  Missing modify_graphical_material_data_void");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"modify_Graphical_material.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* modify_Graphical_material */

int set_Graphical_material(struct Parse_state *state,
	void *material_address_void,void *graphical_material_manager_void)
/*******************************************************************************
LAST MODIFIED : 22 June 1999

DESCRIPTION :
Modifier function to set the material from a command.
???RC set_Object routines could become a macro.
==============================================================================*/
{
	const char *current_token;
	int return_code;
	cmzn_material *temp_material,**material_address;
	struct MANAGER(cmzn_material) *graphical_material_manager;

	ENTER(set_Graphical_material);
	if (state)
	{
		current_token=state->current_token;
		if (current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				material_address=	(cmzn_material **)material_address_void;
				graphical_material_manager=(struct MANAGER(cmzn_material) *)graphical_material_manager_void;
				if (material_address && graphical_material_manager)
				{
					if (fuzzy_string_compare(current_token,"NONE"))
					{
						if (*material_address)
						{
							cmzn_material_destroy(material_address);
							*material_address=(cmzn_material *)NULL;
						}
						return_code=1;
					}
					else
					{
						temp_material=FIND_BY_IDENTIFIER_IN_MANAGER(cmzn_material,name)
								(current_token,graphical_material_manager);
						if (temp_material)
						{
							if (*material_address!=temp_material)
							{
								cmzn_material_destroy(material_address);
								*material_address=cmzn_material_access(temp_material);
							}
							return_code=1;
						}
						else
						{
							display_message(ERROR_MESSAGE,"Unknown material : %s",
								current_token);
							return_code=0;
						}
					}
					shift_Parse_state(state,1);
				}
				else
				{
					display_message(ERROR_MESSAGE,
"set_Graphical_material.  Invalid argument(s).  material_address %p.  material_manager %p",
						material_address_void,graphical_material_manager_void);
					return_code=0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE," MATERIAL_NAME|none");
				/* if possible, then write the name */
				material_address=(cmzn_material **)material_address_void;
				if (material_address)
				{
					temp_material= *material_address;
					if (temp_material)
					{
						display_message(INFORMATION_MESSAGE,"[%s]",Graphical_material_name(temp_material));
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
		display_message(ERROR_MESSAGE,"set_Graphical_material.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_Graphical_material */

int Option_table_add_set_Material_entry(
	struct Option_table *option_table, const char *token,
	cmzn_material **material, struct cmzn_materialmodule *materialmodule)
/*******************************************************************************
LAST MODIFIED : 20 November 2003

DESCRIPTION :
Adds the given <token> to the <option_table>.  The <material> is selected from
the <materialmodule> by name.
==============================================================================*/
{
	int return_code;

	ENTER(Option_table_add_double_vector_with_help_entry);
	if (option_table && token)
	{
		return_code = Option_table_add_entry(option_table, token, (void *)material,
			(void *)materialmodule->getManager(), set_Graphical_material);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Option_table_add_double_vector_with_help_entry.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Option_table_add_double_vector_with_help_entry */

#include "graphics/graphics_module.hpp"
#include "general/manager_private.h"
