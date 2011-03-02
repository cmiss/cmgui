/*******************************************************************************
FILE : finite_element_to_graphics_object.h

LAST MODIFIED : 14 March 2003

DESCRIPTION :
The function prototypes for creating graphical objects from finite elements.
==============================================================================*/
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
 * Portions created by the Initial Developer are Copyright (C) 2005
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
#if !defined (FINITE_ELEMENT_TO_GRAPHICAL_OBJECT_H)
#define FINITE_ELEMENT_TO_GRAPHICAL_OBJECT_H

#include "computed_field/computed_field.h"
#include "finite_element/finite_element.h"
#include "general/enumerator.h"
#include "graphics/auxiliary_graphics_types.h"
#include "graphics/graphics_object.h"
#include "graphics/volume_texture.h"

/*
Global types
------------
*/
enum Use_element_type
/*******************************************************************************
LAST MODIFIED : 20 March 2001

DESCRIPTION :
For glyph sets - determines whether they are generated from:
USE_ELEMENTS = CM_ELEMENT or dimension 3
USE_FACES    = CM_FACE or dimension 2
USE_LINES    = CM_LINE or dimension 1
==============================================================================*/
{
	USE_ELEMENT_TYPE_INVALID = 0,
	USE_ELEMENTS = 1,
	USE_FACES = 2,
	USE_LINES = 3
}; /* enum Use_element_type */

struct Element_to_cylinder_data
/*******************************************************************************
LAST MODIFIED : 3 December 2002

DESCRIPTION :
Data for converting a 1-D element into a cylinder.
==============================================================================*/
{
	char exterior;
	int face_number;
	/* radius = constant_radius + scale_factor*radius_field */
	float constant_radius,scale_factor,time;
	int number_of_segments_along,number_of_segments_around;
	struct Computed_field *coordinate_field, *data_field, *radius_field,
		*texture_coordinate_field;
	struct Graphical_material *material;
	struct FE_region *fe_region;
	struct GT_object *graphics_object;
	enum Render_type render_type;
}; /* struct Element_to_cylinder_data */

struct Element_to_polyline_data
/*******************************************************************************
LAST MODIFIED : 3 December 2002

DESCRIPTION :
Data for converting a 1-D element into a polyline.
==============================================================================*/
{
	char exterior;
	float time;
	int face_number,number_of_segments_in_xi1;
	struct Computed_field *coordinate_field,*data_field;
	struct FE_region *fe_region;
	struct GT_object *graphics_object;
}; /* struct Element_to_line_data */

struct Element_to_surface_data
/*******************************************************************************
LAST MODIFIED : 3 December 2002

DESCRIPTION :
Data for converts a 2-D element into a surface.
==============================================================================*/
{
	char exterior,reverse_normals;
	/* Can be either g_SURFACE or g_NURBS */
	enum GT_object_type object_type;
	enum Render_type render_type;
	float time;
	int face_number,number_of_segments_in_xi1,number_of_segments_in_xi2;
	struct Computed_field *coordinate_field,*data_field,*texture_coordinate_field;
	struct FE_region *fe_region;
	struct GT_object *graphics_object;
}; /* struct Element_to_surface_data */

struct Displacement_map
/*******************************************************************************
LAST MODIFIED : 27 April 1998

DESCRIPTION :
Used for displaying voltexes.
???RC Move to texture? volume_texture?
==============================================================================*/
{
	double scale;
	int xi_direction;
	struct Texture *texture;
}; /* struct Displacement_map */

struct Surface_pointset
/*******************************************************************************
LAST MODIFIED : 21 December 1998

DESCRIPTION :
==============================================================================*/
{
	float scale;
	struct GT_pointset *surface_points;
	struct Texture *texture;
}; /* struct Surface_pointset */

struct Element_to_volume_data
/*******************************************************************************
LAST MODIFIED : 4 December 2000

DESCRIPTION :
Data for converting a 3-D element into a volume.
==============================================================================*/
{
	struct Clipping *clipping;
	struct Computed_field *blur_field;
	struct Computed_field *coordinate_field;
	struct Computed_field *data_field;
	struct Computed_field *texture_coordinate_field;
	struct Computed_field *displacement_map_field;
	int displacement_map_xi_direction;
	struct FE_time *fe_time;
	struct GT_object *graphics_object;
	struct FE_region *fe_region;
	enum Render_type render_type;
	float time;
	struct VT_volume_texture *volume_texture;
}; /* struct Element_to_volume_data */

struct Element_to_iso_scalar_data
/*******************************************************************************
LAST MODIFIED : 4 December 2000

DESCRIPTION :
Data for converting a 3-D element into an iso_surface (via a volume_texture).
==============================================================================*/
{
	char exterior;
	double iso_value;
	enum Use_element_type use_element_type;
	float time;
	int face_number;
	struct Clipping *clipping;
	struct Computed_field *coordinate_field, *data_field, *scalar_field;
	struct Computed_field *texture_coordinate_field;
	int number_in_xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
	struct FE_field *native_discretization_field;
	struct GT_object *graphics_object;
	enum Render_type render_type;
	struct FE_region *fe_region;
	struct FE_time *fe_time;
}; /* struct Element_to_iso_surface_data */

struct Element_to_glyph_set_data
/*******************************************************************************
LAST MODIFIED : 8 May 2001

DESCRIPTION :
Data for converting a finite element into a set of glyphs displaying information
about fields defined over it.
If the <native_discretization_field> is given and uses an element based field in
any element, the native discretization is taken as the number of regular-sized
cells in each xi direction of the element, otherwise, the three values of
number_of_cells_in_xi* are used.
If the <cell_corners_flag> is not set, glyphs are displayed at the cell centres,
otherwise they are displayed at the cell corners and there will be one extra
point in each xi-direction as there are cells.
At each of these points the <glyph> of <glyph_size> with its centre located at
<glyph_centre> is displayed. The optional <orientation_scale_field> can be used
to orient and scale the glyph in a manner depending on the number of components
in the field (see function make_glyph_orientation_scale_axes). The three
<glyph_scale_factors> multiply the scaling effect in each axis taken from the
<orientation_scale_field>.
The optional <data_field> (currently only a scalar) is calculated as data over
the glyph_set, for later colouration by a spectrum.
The optional <label_field> is written beside each glyph in string form.
The <use_element_type> determines the type/dimension of elements in use.
If the dimension is less than 3, <exterior> and <face_number> may be used.
==============================================================================*/
{
	char exterior;
	enum Graphics_select_mode select_mode;
	enum Use_element_type use_element_type;
	enum Xi_discretization_mode xi_discretization_mode;
	FE_value base_size[3], centre[3], scale_factors[3];
	float time;
	int face_number,number_in_xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
	struct Computed_field *coordinate_field, *data_field, *variable_scale_field,
		*label_field, *orientation_scale_field, *xi_point_density_field;
	struct FE_field *native_discretization_field;
	struct FE_region *fe_region;
	struct Graphics_font *font;
	struct GT_object *glyph,*graphics_object;
	FE_value_triple exact_xi;
}; /* struct Element_to_glyph_set_data */

enum Collapsed_element_type
/*******************************************************************************
LAST MODIFIED : 25 July 2001

DESCRIPTION :
How an element is collapsed.
???DB.  Currently only 2D
==============================================================================*/
{
	ELEMENT_NOT_COLLAPSED,
	ELEMENT_COLLAPSED_XI1_0,
	ELEMENT_COLLAPSED_XI1_1,
	ELEMENT_COLLAPSED_XI2_0,
	ELEMENT_COLLAPSED_XI2_1
}; /* enum Collapsed_element_type */

/*
Global functions
----------------
*/

int make_glyph_orientation_scale_axes(
	int number_of_orientation_scale_values, FE_value *orientation_scale_values,
	FE_value *axis1,FE_value *axis2, FE_value *axis3, FE_value *size);
/*******************************************************************************
LAST MODIFIED : 16 November 2000

DESCRIPTION :
Computes the three glyph orientation axes from the <orientation_scale_values>.

The orientation is understood from the number_of_orientation_scale_values as:
0 = zero scalar (no vector/default orientation);
1 = scalar (no vector/default orientation);
2 = 1 2-D vector (2nd glyph axis is normal in plane, 3rd is out of 2-D plane);
3 = 1 3-D vector (orthogonal 2nd and 3rd glyph axes are arbitrarily chosen);
4 = 2 2-D vectors (3rd glyph axis taken as out of 2-D plane);
6 = 2 3-D vectors (3rd glyph axis found from cross product);
9 = 3 3-D vectors = complete definition of glyph axes;

The scaling behaviour depends on the number of vectors interpreted above, where:
0 = isotropic scaling on all three axes by scalar;
1 = isotropic scaling on all three axes by magnitude of vector;
2 = scaling in direction of 2 vectors, ie. they keep their current length, unit
    vector in 3rd axis;
3 = scaling in direction of 3 vectors - ie. they keep their current length.

Function returns the axes as unit vectors with their magnitudes in the <size>
array. This is always possible if there is a scalar (or zero scalar), but where
zero vectors are either read or calculated from the <orientation_scale_values>,
these are simply returned, since no valid direction can be produced.
==============================================================================*/

PROTOTYPE_ENUMERATOR_FUNCTIONS(Use_element_type);

/***************************************************************************//**
 * Returns the dimension expected for the <use_element_type>. If <fe_region> is
 * supplied, USE_ELEMENT returns the highest dimension present in fe_region.
 */
int Use_element_type_dimension(enum Use_element_type use_element_type,
	struct FE_region *fe_region);

struct GT_glyph_set *create_GT_glyph_set_from_FE_element(
	struct FE_element *element, struct FE_element *top_level_element,
	struct Computed_field *coordinate_field,
	int number_of_xi_points, FE_value_triple *xi_points, struct GT_object *glyph,
	FE_value *base_size, FE_value *centre, FE_value *scale_factors,
	struct Computed_field *orientation_scale_field,
	struct Computed_field *variable_scale_field,
	struct Computed_field *data_field, 
	struct Graphics_font *font, struct Computed_field *label_field,
	enum Graphics_select_mode select_mode, int element_selected,
	struct Multi_range *selected_ranges, int *point_numbers, FE_value time);
/*******************************************************************************
LAST MODIFIED : 18 November 2005

DESCRIPTION :
Converts a finite element into a set of glyphs displaying information
about fields defined over it.
At each of the <number_of_xi_points> <xi_points> the <glyph> of at least
<base_size> with the given glyph <centre> is displayed.
The optional <orientation_scale_field> can be used to orient and scale the
glyph in a manner depending on the number of components in the field. The
optional <variable_scale_field> can provide signed scaling independently of the
glyph axis directions. See function make_glyph_orientation_scale_axes for
details. The combined scale from the above 2 fields is multiplied in each axis
by the <scale_factors> then added to the base_size.
The optional <data_field> (currently only a scalar) is calculated as data over
the glyph_set, for later colouration by a spectrum.
The optional <label_field> is written beside each glyph in string form.
The optional <top_level_element> may be provided as a clue to Computed_fields
to say which parent element they should be evaluated on as necessary.
<select_mode> is used in combination with the <element_selected> and
<selected_ranges> to draw only those points with numbers in or out of the given
ranges when given value GRAPHICS_DRAW_SELECTED or GRAPHICS_DRAW_UNSELECTED.
If <element_selected> is true, all points are selected, otherwise selection is
determined from the <selected_ranges>, and if <selected_ranges> is NULL, no
numbers are selected.
If <point_numbers> are supplied then points numbers for OpenGL picking are taken
from this array, otherwise they are sequential, starting at 0.
Note:
- the coordinate and orientation fields are assumed to be rectangular cartesian.
- the coordinate system of the variable_scale_field is ignored/not used.
==============================================================================*/

struct GT_glyph_set *create_GT_glyph_set_from_FE_region_nodes(
	struct FE_region *fe_region,
	struct Computed_field *coordinate_field, struct GT_object *glyph,
	FE_value *base_size, FE_value *centre, FE_value *scale_factors,
	FE_value time, struct Computed_field *orientation_scale_field,
	struct Computed_field *variable_scale_field,
	struct Computed_field *data_field,
	struct Graphics_font *font, struct Computed_field *label_field,
	struct Computed_field *label_density_field,
	struct Computed_field *visibility_field, enum Graphics_select_mode select_mode,
	struct Computed_field *group_field);
/*******************************************************************************
Creates a GT_glyph_set displaying a <glyph> of at least <base_size>, with the
given glyph <centre> at each node in <fe_region>.
The optional <orientation_scale_field> can be used to orient and scale the
glyph in a manner depending on the number of components in the field. The
optional <variable_scale_field> can provide signed scaling independently of the
glyph axis directions. See function make_glyph_orientation_scale_axes for
details. The combined scale from the above 2 fields is multiplied in each axis
by the <scale_factors> then added to the base_size.
The optional <data_field> is calculated as data over the glyph_set, for later
colouration by a spectrum.
The optional <label_field> is written beside each glyph in string form.
The optional <label_density_field> controls the number of times the label
field is rendered for glyphs that can render the label many times, such as axes
and graph grids.
The <select_mode> controls whether node cmiss numbers are output as integer
names with the glyph_set. If <select_mode> is DRAW_SELECTED or DRAW_UNSELECTED,
only nodes in (or not in) the <selected_node_list> are rendered.
Notes:
- the coordinate and orientation fields are assumed to be rectangular cartesian.
- the coordinate system of the variable_scale_field is ignored/not used.
==============================================================================*/

struct GT_polyline *create_GT_polyline_from_FE_element(
	struct FE_element *element,struct Computed_field *coordinate_field,
	struct Computed_field *data_field,int number_of_segments,
	struct FE_element *top_level_element, FE_value time,
	int line_width);
/*******************************************************************************
LAST MODIFIED : 22 April 2004

DESCRIPTION :
Creates a <GT_polyline> from the <coordinate_field> for the 1-D finite <element>
using <number_of_segments> spaced equally in xi.
The optional <data_field> (currently only a scalar) is calculated as data over
the polyline, for later colouration by a spectrum.
The optional <top_level_element> may be provided as a clue to Computed_fields
to say which parent element they should be evaluated on as necessary.
If the <line_width> is non zero then it will override the default line width.
Notes:
- the coordinate field is assumed to be rectangular cartesian.
==============================================================================*/

/***************************************************************************//**
 * Adds vertex values to the supplied vertex array to create a line representing
 * the 1-D finite element.
 * 
 * @param element The element that primitives will be generated for.
 * @param vertex_array The array which this primitive will be added to.
 * @param coordinate_field The required position coordinate field.  It is assumed
 * that this field is rectangular cartesian (already wrapped if the original coordinates were not).
 * @param data_field Optional data field, if not wanted set to NULL.
 * @param number_of_data_values The number of components in the data_field and
 * the allocated size of the data_buffer.
 * @param data_buffer A buffer large enough to evaluate the data_field into.
 * @param texture_coordinate_field Optional field usually used for positioning textures.
 * @param number_of_segments The supplied element is broken into this many linear
 * segments in the created primitive.
 * @param top_level_element Optional element may be provided as a clue to Computed_fields
 * to say which parent element they should be evaluated on as necessary.
 * @param time The time used for evaluating the various fields.
 */
int FE_element_add_line_to_vertex_array(
	struct FE_element *element, struct Graphics_vertex_array *vertex_array,
	struct Computed_field *coordinate_field, struct Computed_field *data_field,
	int number_of_data_values, FE_value *data_buffer,
	struct Computed_field *texture_coordinate_field,
	unsigned int number_of_segments, struct FE_element *top_level_element, FE_value time);

struct GT_surface *create_cylinder_from_FE_element(struct FE_element *element,
	struct Computed_field *coordinate_field,struct Computed_field *data_field,
	float constant_radius,float scale_factor,struct Computed_field *radius_field,
	int number_of_segments_along,int number_of_segments_around,
	struct Computed_field *texture_coordinate_field,
	struct FE_element *top_level_element, enum Render_type render_type,
	FE_value time);
/*******************************************************************************
LAST MODIFIED : 11 October 2002

DESCRIPTION :
Creates a <GT_surface> from the <coordinate_field> and the radius for the 1-D
finite <element> using a grid of points.  The cylinder is built out of an array
of rectangles <number_of_segments_along> by <number_of_segments_around> the
cylinder.The actual radius is calculated as:
radius = constant_radius + scale_factor*radius_field(a scalar field)
The optional <data_field> (currently only a scalar) is calculated as data over
the length of the cylinder, for later colouration by a spectrum.
The optional <top_level_element> may be provided as a clue to Computed_fields
to say which parent element they should be evaluated on as necessary.
The first component of <texture_coordinate_field> is used to control the
texture coordinates along the element. If not supplied it will match Xi. The
texture coordinate around the cylinders is always from 0 to 1.
Notes:
- the coordinate field is assumed to be rectangular cartesian.
==============================================================================*/

struct GT_nurbs *create_GT_nurb_from_FE_element(struct FE_element *element,
	struct Computed_field *coordinate_field,
	struct Computed_field *texture_coordinate_field,
	struct FE_element *top_level_element, FE_value time);
/*******************************************************************************
LAST MODIFIED : 3 December 2001

DESCRIPTION :
The optional <top_level_element> may be provided as a clue to Computed_fields
to say which parent element they should be evaluated on as necessary.
==============================================================================*/

/****************************************************************************//**
 * Sorts out how standard, polygon and simplex elements are segmented, based on
 * numbers of segments requested for "square" elements.
 */
int get_surface_element_segmentation(struct FE_element *element,
	int number_of_segments_in_xi1_requested,
	int number_of_segments_in_xi2_requested,
	int *number_of_points_in_xi1,int *number_of_points_in_xi2,
	int *number_of_points,int *number_of_polygon_vertices,
	gtPolygonType *polygon_type,enum Collapsed_element_type *collapsed_element,
	enum FE_element_shape_type *shape_type_address);

struct GT_surface *create_GT_surface_from_FE_element(
	struct FE_element *element,struct Computed_field *coordinate_field,
	struct Computed_field *texture_coordinate_field,
	struct Computed_field *data_field,int number_of_segments_in_xi1_requested,
	int number_of_segments_in_xi2_requested,char reverse_normals,
	struct FE_element *top_level_element, enum Render_type render_type,
	FE_value time);
/*******************************************************************************
LAST MODIFIED : 2 May 2000

DESCRIPTION :
Creates a <GT_surface> from the <coordinate_field> for the 2-D finite <element>
using an array of <number_of_segments_in_xi1> by <number_of_segments_in_xi2>
rectangles in xi space.  The spacing is constant in each of xi1 and xi2.
The optional <texture_coordinate_field> is evaluated at each vertex for the
corresonding texture coordinates.  If none is supplied then a length based
algorithm is used instead.
The optional <data_field> (currently only a scalar) is calculated as data over
the surface, for later colouration by a spectrum.
The optional <top_level_element> may be provided as a clue to Computed_fields
to say which parent element they should be evaluated on as necessary.
Notes:
- the coordinate field is assumed to be rectangular cartesian.
???DB.  Check for collapsed elements (repeated nodes) and treat accordingly -
	discretise with triangles ?  Have done for one special case - bi-quadratic
	with collapse along xi2=0.  Extend ?
???DB.  Always calculates texture coordinates.  Calculate as required ?  Finite
	element graphical object ?
???DB.  18 September 1993.  Only the side with the normal pointing out is
	textured correctly (even in TWOSIDE mode).  For 2-D finite elements which
	are faces of 3-D finite elements make the normals point out of the element.
==============================================================================*/

struct GT_voltex *create_GT_voltex_from_FE_element(struct FE_element *element,
	struct Computed_field *coordinate_field,struct Computed_field *data_field,
	struct VT_volume_texture *vtexture, enum Render_type render_type,
	struct Computed_field *displacement_field, int displacement_map_xi_direction,
	struct Computed_field *texture_coordinate_field, FE_value time);
/*******************************************************************************
LAST MODIFIED : 10 November 2005

DESCRIPTION :
Creates a <GT_voltex> from a 3-D finite <element> <block> and volume texture
The volume texture contains a list of coloured triangular polygons representing
an isosurface calculated from the volumetric data. These polygons are stored in
local xi1,2,3 space and undergo a free form deformation (FFD) when mapped onto
the finite <element> <block>. The output contains the deformed polygon & normal
list.  Only the vertices are transformed. The polygons are then generated from a
list of pointers to the vertices.  Normals are calculated from the resulting
deformed vertices by taking the average of the cross products of the surrounding
faces.
Added a surface_points set and a controlling texture_map and scale which list
seed points distributed on the surface randomly but with density due to the 
texture_map.
==============================================================================*/

struct VT_vector_field *interpolate_vector_field_on_FE_element(double ximax[3],
	struct FE_element *element,struct Computed_field *coordinate_field,
	struct VT_vector_field *vector_field, FE_value time);
/*******************************************************************************
LAST MODIFIED : 3 December 2001

DESCRIPTION :
Interpolates xi points (triples in vector field) over the finite <element>
<block>.  Outputs the updated field.
==============================================================================*/

struct GT_voltex *generate_clipped_GT_voltex_from_FE_element(
	struct Clipping *clipping,struct FE_element *element,
	struct Computed_field *coordinate_field,struct Computed_field *field_scalar,
	struct VT_volume_texture *texture, enum Render_type render_type,
	struct Computed_field *displacement_map_field, int displacement_map_xi_direction,
	struct Computed_field *texture_coordinate_field, FE_value time);
/*******************************************************************************
LAST MODIFIED : 10 November 2005

DESCRIPTION :
Generates clipped voltex from <volume texture> and <clip_function> over
<element><block>
==============================================================================*/

int element_to_cylinder(struct FE_element *element,
	void *void_element_to_cylinder_data);
/*******************************************************************************
LAST MODIFIED : 14 February 1999

DESCRIPTION :
Converts a finite element into a cylinder.
==============================================================================*/

int element_to_polyline(struct FE_element *element,
	void *element_to_polyline_data_void);
/*******************************************************************************
LAST MODIFIED : 12 March 1999

DESCRIPTION :
Converts a finite element into a polyline and adds it to a graphics_object.
==============================================================================*/

int element_to_nurbs(struct FE_element *element,
	void *void_element_to_nurbs_data);
/*******************************************************************************
LAST MODIFIED : 12 March 1999

DESCRIPTION :
Converts a finite element into a nurbs surface.
==============================================================================*/

int element_to_surface(struct FE_element *element,
	void *void_element_to_surface_data);
/*******************************************************************************
LAST MODIFIED : 12 March 1999

DESCRIPTION :
Converts a finite element into a surface.
==============================================================================*/

int element_to_glyph_set(struct FE_element *element,
	void *new_element_to_glyph_set_data_void);
/*******************************************************************************
LAST MODIFIED : 4 January 1999

DESCRIPTION :
Converts a finite element into a set of glyphs displaying information about the
fields defined over it.
==============================================================================*/

int element_to_volume(struct FE_element *element,
	void *void_element_to_volume_data);
/*******************************************************************************
LAST MODIFIED : 16 May 1998

DESCRIPTION :
Converts a 3-D element into a volume.
==============================================================================*/

int element_to_iso_scalar(struct FE_element *element,
	void *element_to_iso_scalar_data_void);
/*******************************************************************************
LAST MODIFIED : 28 January 2000

DESCRIPTION :
Computes iso-surfaces/lines/points graphics from <element>.
==============================================================================*/

int create_iso_surfaces_from_FE_element(struct FE_element *element,
	double iso_value, FE_value time,struct Clipping *clipping,
	struct Computed_field *coordinate_field,
	struct Computed_field *data_field,struct Computed_field *scalar_field,
	struct Computed_field *texture_coordinate_field,
	int *number_in_xi, double decimation_threshold,
	struct GT_object *graphics_object,enum Render_type render_type);
/*******************************************************************************
LAST MODIFIED : 8 December 2005

DESCRIPTION :
Converts a 3-D element into an iso_surface (via a volume_texture).
==============================================================================*/

#endif /* !defined (FINITE_ELEMENT_TO_GRAPHICAL_OBJECT_H) */
