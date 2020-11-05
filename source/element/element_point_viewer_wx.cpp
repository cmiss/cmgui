/*******************************************************************************
FILE : element_point_viewer_wx.cpp

LAST MODIFIED : 6 June 2007

DESCRIPTION :
For wxWidgets only, Dialog for selecting an element point, viewing and editing its fields and
applying changes. Works with Element_point_ranges_selection to display the last
selected element point, or set it if entered in this dialog.
==============================================================================*/
/* OpenCMISS-Cmgui Application
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#if 1
#include "configure/cmgui_configure.h"
#endif
#include "opencmiss/zinc/elementfieldtemplate.h"
#include "opencmiss/zinc/fieldcache.h"
#include "opencmiss/zinc/fieldmodule.h"
#include "opencmiss/zinc/mesh.h"
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_app.h"
#include "computed_field/computed_field_finite_element.h"
#include "computed_field/computed_field_value_index_ranges.h"
#include "element/element_point_viewer_wx.h"
#include "finite_element/finite_element_app.h"
#include "finite_element/finite_element_discretization.h"
#include "finite_element/finite_element_mesh.hpp"
#include "finite_element/finite_element_region_private.h"
#include "finite_element/finite_element_to_graphics_object.h"
#include "general/debug.h"
#include "general/message.h"
#include "user_interface/user_interface.h"
#include "command/parser.h"
#include "graphics/auxiliary_graphics_types_app.h"
#if defined (WX_USER_INTERFACE)
#include <wx/collpane.h>
#include "wx/wx.h"
#include "wx/xrc/xmlres.h"
#include "choose/choose_manager_class.hpp"
#include "choose/choose_enumerator_class.hpp"
#include "element/element_point_viewer_wx.xrch"
#include "choose/text_choose_from_fe_element.hpp"
#include "icon/cmiss_icon.xpm"
#endif /*defined (WX_USER_INTERFACE)*/

/*
Module variables
----------------
*/
#if defined (WX_USER_INTERFACE)
class wxElementPointViewer;
#endif /* defined (WX_USER_INTERFACE) */

struct Element_point_viewer
/*******************************************************************************
LAST MODIFIED : 19 March 2003

DESCRIPTION :
Contains all the information carried by the element_point_viewer widget.
==============================================================================*/
{
	/* global data */
	struct Computed_field_package *computed_field_package;
	struct Element_point_viewer **element_point_viewer_address;
	struct cmzn_region *region;
	cmzn_fieldmodulenotifier_id fieldmodulenotifier;
	struct Element_point_ranges_selection *element_point_ranges_selection;
	struct Time_object *time_object;
	struct User_interface *user_interface;
	/* information about the element point being edited; note element in
		 identifier is not accessed */
	struct Element_point_ranges_identifier element_point_identifier;
	 int element_point_number, number_of_components;
	struct Computed_field *current_field;
	FE_value xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
	/* field components whose values have been modified stored in following */
	struct LIST(Field_value_index_ranges) *modified_field_components;
	/* apply will set same values to all grid points with this field the same,
		 if non-NULL */
	 struct Computed_field *match_grid_field;
	 int time_object_callback;
	struct MANAGER(Computed_field) *computed_field_manager;
#if defined (WX_USER_INTERFACE)
	 wxElementPointViewer *wx_element_point_viewer;
	 wxScrolledWindow *variable_viewer_panel;
	 wxScrolledWindow *collpane;
	 wxWindow *element_point_win;
	 wxBoxSizer *paneSz;
	 wxFrame *frame;
	 wxGridSizer *element_point_grid_field;
	 wxSize frame_size;
	 wxTextCtrl *xitextctrl, *pointtextctrl, *discretizationtextctrl, *gridvaluetextctrl;
	 wxPanel *element_sample_mode_chooser_panel;
	 int init_width, init_height, frame_width, frame_height;
	 //  wxBoxSizer *sizer_1;
#endif /* (WX_USER_INTERFACE) */

}; /* element_point_viewer_struct */


/*
Prototype
----------------
*/
static int element_point_viewer_setup_components(
	 struct Element_point_viewer *element_point_viewer);
/*******************************************************************************
LAST MODIFIED : 10 October 2003

DESCRIPTION :
Creates the array of cells containing field component names and values.
==============================================================================*/

int element_point_viewer_set_element_point_field(
  void *element_point_viewer_void,
	struct Element_point_ranges_identifier *element_point_identifier,
	int element_point_number,struct Computed_field *field);
/*******************************************************************************
LAST MODIFIED : 6 Jube 2007

DESCRIPTION :
Sets the element_point/field being edited in the
<element_point_field_viewer_widget>.
Note that the viewer works on the element itself, not a local copy. Hence, only
pass unmanaged elements in the element_point_identifier to this widget.
==============================================================================*/

static int Element_point_viewer_refresh_chooser(
	 struct Element_point_viewer *element_point_viewer);
/*******************************************************************************
LAST MODIFIED :7 June 2007

DESCRIPTION :
Updates the element shown in the chooser to match that for the current point.
==============================================================================*/

int element_point_viewer_widget_set_element_point(
	Element_point_viewer *element_point_viewer,
	struct Element_point_ranges_identifier *element_point_identifier,
	int element_point_number);
/*******************************************************************************
LAST MODIFIED : 6 June 2007

DESCRIPTION :
Sets the element point being edited in the <element_point_viewer_widget>. Note
that the viewer works on the element itself, not a local copy. Hence, only pass
unmanaged elements in the identifier to this widget.
==============================================================================*/

static int element_point_viewer_add_collpane(struct Computed_field *current_field,
	 void *element_point_viewer_void);

/*
Module functions
----------------
*/
static int Element_point_viewer_refresh_sample_mode(
	 struct Element_point_viewer *element_point_viewer);
/*******************************************************************************
LAST MODIFIED : 13 June 2007

DESCRIPTION :
Updates the cmzn_element_point_sampling_mode shown in the chooser to match that for the
current point.
==============================================================================*/


static int Element_point_viewer_refresh_discretization_text(
	struct Element_point_viewer *element_point_viewer)
/*******************************************************************************
LAST MODIFIED : 8 June 2000

DESCRIPTION :
Updates the discretization text field. If there is a current element point,
writes the discretization in use, otherwise N/A.
==============================================================================*/
{
	char temp_string[60];
	int dimension,is_editable,is_sensitive,*number_in_xi,return_code;
	struct FE_element *element;

	ENTER(Element_point_viewer_refresh_discretization_text);
	if (element_point_viewer)
	{
		return_code=1;
		wxString discretizationtextctrl_string = element_point_viewer->discretizationtextctrl->GetValue();
		const char *value_string = discretizationtextctrl_string.mb_str(wxConvUTF8);
		if (value_string)
		{
			if ((element=element_point_viewer->element_point_identifier.element)&&
				(dimension=get_FE_element_dimension(element))&&
				(number_in_xi=
					element_point_viewer->element_point_identifier.number_in_xi))
			{
				switch (dimension)
				{
					case 1:
					{
						sprintf(temp_string,"%d",number_in_xi[0]);
					} break;
					case 2:
					{
						sprintf(temp_string,"%d*%d",number_in_xi[0],number_in_xi[1]);
					} break;
					default:
					{
						sprintf(temp_string,"%d*%d*%d",number_in_xi[0],number_in_xi[1],
							number_in_xi[2]);
					} break;
				}
				is_sensitive=is_editable=
					(CMZN_ELEMENT_POINT_SAMPLING_MODE_SET_LOCATION != element_point_viewer->
						element_point_identifier.sampling_mode);
				}
				else
				{
					sprintf(temp_string,"N/A");
					is_editable=0;
					is_sensitive=0;
				}
				/* only set string if different from that shown */
				if (strcmp(temp_string,value_string))
				{
					element_point_viewer->discretizationtextctrl->SetValue(wxString::FromAscii(temp_string));
				}
				element_point_viewer->discretizationtextctrl->Enable(is_sensitive);
		}
	}
	else
	{
		 display_message(ERROR_MESSAGE,
				"Element_point_viewer_refresh_discretization_text.  Invalid argument(s)");
		 return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Element_point_viewer_refresh_discretization_text */

static int Element_point_viewer_refresh_point_number_text(
	struct Element_point_viewer *element_point_viewer)
/*******************************************************************************
LAST MODIFIED : 8 June 2000

DESCRIPTION :
Updates the point_number text field. If there is a current element point,
writes its number, otherwise N/A.
==============================================================================*/
{
	char temp_string[20];
	const char *value_string;
	int return_code,is_editable,is_sensitive;

	ENTER(Element_point_viewer_refresh_point_number_text);
	if (element_point_viewer)
	{
		return_code=1;
		wxString point_string = element_point_viewer->pointtextctrl->GetValue();
		value_string = point_string.mb_str(wxConvUTF8);
		if (value_string)
		{
			if (element_point_viewer->element_point_identifier.element)
			{
				sprintf(temp_string,"%d",element_point_viewer->element_point_number);
				is_sensitive=is_editable=
					(CMZN_ELEMENT_POINT_SAMPLING_MODE_SET_LOCATION != element_point_viewer->
						element_point_identifier.sampling_mode);
			}
			else
			{
				sprintf(temp_string,"N/A");
				is_editable=0;
				is_sensitive=0;
			}
			/* only set string if different from that shown */
			if (strcmp(temp_string,value_string))
			{
				 element_point_viewer->pointtextctrl->SetValue(wxString::FromAscii(temp_string));
			}
			element_point_viewer->pointtextctrl->Enable(is_sensitive);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_point_viewer_refresh_point_number_text.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Element_point_viewer_refresh_point_number_text */

static int Element_point_viewer_refresh_xi_text(
	struct Element_point_viewer *element_point_viewer)
/*******************************************************************************
LAST MODIFIED : 13 June 2007

DESCRIPTION :
Updates the xi text field. If there is a current element point, writes its xi
value otherwise N/A.
==============================================================================*/
{
	char temp_string[120];
	const char *value_string;
	FE_value *xi;
	int dimension,is_editable,is_sensitive,return_code;
	struct FE_element *element;

	ENTER(Element_point_viewer_refresh_xi_text);
	if (element_point_viewer)
	{
		return_code=1;
		wxString xi_string = element_point_viewer->xitextctrl->GetValue();
		value_string = xi_string.mb_str(wxConvUTF8);
		if (value_string)
		{
			 if (NULL != (element=element_point_viewer->element_point_identifier.element)&&
					 0 != (dimension=get_FE_element_dimension(element))&&
					 NULL!= (xi=element_point_viewer->xi))
			{
				switch (dimension)
				{
					case 1:
					{
						sprintf(temp_string,"%g",xi[0]);
					} break;
					case 2:
					{
						sprintf(temp_string,"%g, %g",xi[0],xi[1]);
					} break;
					default:
					{
						sprintf(temp_string,"%g, %g, %g",xi[0],xi[1],xi[2]);
					} break;
				}
				is_editable=(CMZN_ELEMENT_POINT_SAMPLING_MODE_SET_LOCATION == element_point_viewer->
					element_point_identifier.sampling_mode);
				is_sensitive=1;
			}
			else
			{
				sprintf(temp_string,"N/A");
				is_editable=0;
				is_sensitive=0;
			}
			/* only set string if different from that shown */
			if (strcmp(temp_string,value_string))
			{
				 element_point_viewer->xitextctrl->SetValue(wxString::FromAscii(temp_string));
			}
			element_point_viewer->xitextctrl->Enable(is_sensitive);
			element_point_viewer->xitextctrl->SetEditable(is_editable);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_point_viewer_refresh_xi_text.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Element_point_viewer_refresh_xi_text */

static int Element_point_viewer_refresh_grid_value_text(
	 struct Element_point_viewer *element_point_viewer);
/*******************************************************************************
LAST MODIFIED : 13 June 2007

DESCRIPTION :
Updates the grid_value text field. If there is a current element point, writes
the field value, otherwise N/A.
==============================================================================*/

static int element_point_field_is_editable(
	struct Element_point_ranges_identifier *element_point_identifier,
	struct Computed_field *field,int *number_in_xi)
/*******************************************************************************
LAST MODIFIED : 6 June 2000

DESCRIPTION :
Returns true if <field> is editable at <element_point_identifier>, and if so
returns the <number_in_xi>. Returns 0 with no error if no field or no element
is supplied.
==============================================================================*/
{
	int dimension,i,return_code;

	ENTER(element_point_field_is_editable);
	if (element_point_identifier&&number_in_xi)
	{
		if (element_point_identifier->element&&field&&
			(CMZN_ELEMENT_POINT_SAMPLING_MODE_CELL_CORNERS==
				element_point_identifier->sampling_mode)&&
			Computed_field_get_native_discretization_in_element(field,
				element_point_identifier->element,number_in_xi))
		{
			return_code=1;
			/* check matching discretization */
			dimension=get_FE_element_dimension(element_point_identifier->element);
			for (i=0;(i<dimension)&&return_code;i++)
			{
				if (element_point_identifier->number_in_xi[i] != number_in_xi[i])
				{
					return_code=0;
				}
			}
		}
		else
		{
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"element_point_field_is_editable.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
}

static int Element_point_viewer_set_viewer_element_point(
	struct Element_point_viewer *element_point_viewer)
/*******************************************************************************
LAST MODIFIED : 19 March 2003

DESCRIPTION :
Gets the current element_point, makes a copy of its element if not NULL,
and passes it to the element_point_viewer_widget.
==============================================================================*/
{
	int temp_element_point_number,return_code;
	struct Element_point_ranges_identifier temp_element_point_identifier;

	ENTER(Element_point_viewer_set_viewer_element_point);
	if (element_point_viewer)
	{
		 temp_element_point_number=element_point_viewer->element_point_number;
		 COPY(Element_point_ranges_identifier)(&temp_element_point_identifier,
				&(element_point_viewer->element_point_identifier));
		if (temp_element_point_identifier.element)
		{
			if (Element_point_ranges_identifier_is_valid(
				&temp_element_point_identifier))
			{
				Element_point_make_top_level(&temp_element_point_identifier,
					&temp_element_point_number);
			}
		}
		/* clear modified_components */
		REMOVE_ALL_OBJECTS_FROM_LIST(Field_value_index_ranges)(
			element_point_viewer->modified_field_components);
		element_point_viewer_widget_set_element_point(
			element_point_viewer,
			&temp_element_point_identifier,temp_element_point_number);
		return_code=1;
	}
	else
	{
		 display_message(ERROR_MESSAGE,
				"Element_point_viewer_set_viewer_element_point.  Invalid argument(s)");
		 return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Element_point_viewer_set_viewer_element_point */

static int Element_point_viewer_calculate_xi(
	struct Element_point_viewer *element_point_viewer)
/*******************************************************************************
LAST MODIFIED : 23 April 2001

DESCRIPTION :
Ensures xi is correct for the currently selected element point, if any.
==============================================================================*/
{
	int return_code;

	ENTER(Element_point_viewer_calculate_xi);
	if (element_point_viewer)
	{
		if (element_point_viewer->element_point_identifier.element)
		{
			return_code = FE_element_get_numbered_xi_point(
				element_point_viewer->element_point_identifier.element,
				element_point_viewer->element_point_identifier.sampling_mode,
				element_point_viewer->element_point_identifier.number_in_xi,
				element_point_viewer->element_point_identifier.exact_xi,
				(cmzn_fieldcache_id)0,
				/*coordinate_field*/(struct Computed_field *)NULL,
				/*density_field*/(struct Computed_field *)NULL,
				element_point_viewer->element_point_number,
				element_point_viewer->xi);
		}
		else
		{
			return_code = 1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_point_viewer_calculate_xi.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Element_point_viewer_calculate_xi */

static void Element_point_viewer_element_point_ranges_selection_change(
	struct Element_point_ranges_selection *element_point_ranges_selection,
	struct Element_point_ranges_selection_changes *changes,
	void *element_point_viewer_void)
/*******************************************************************************
LAST MODIFIED : 25 May 2000

DESCRIPTION :
Callback for change in the global element_point selection.
==============================================================================*/
{
	int start,stop;
	struct Element_point_ranges *element_point_ranges;
	struct Element_point_viewer *element_point_viewer;
	struct Multi_range *ranges;

	ENTER(Element_point_viewer_element_point_ranges_selection_change);
	if (element_point_ranges_selection&&changes&&(element_point_viewer=
		(struct Element_point_viewer *)element_point_viewer_void))
	{
		/* get the last selected element_point and put it in the viewer */
		if ((element_point_ranges=
			FIRST_OBJECT_IN_LIST_THAT(Element_point_ranges)(
				(LIST_CONDITIONAL_FUNCTION(Element_point_ranges) *)NULL,(void *)NULL,
				changes->newly_selected_element_point_ranges_list))||
			(element_point_ranges=
				FIND_BY_IDENTIFIER_IN_LIST(Element_point_ranges,identifier)(
					&(element_point_viewer->element_point_identifier),
					Element_point_ranges_selection_get_element_point_ranges_list(
						element_point_ranges_selection)))||
			(element_point_ranges=
				FIRST_OBJECT_IN_LIST_THAT(Element_point_ranges)(
					(LIST_CONDITIONAL_FUNCTION(Element_point_ranges) *)NULL,(void *)NULL,
					Element_point_ranges_selection_get_element_point_ranges_list(
						element_point_ranges_selection))))
		{
			Element_point_ranges_get_identifier(element_point_ranges,
				&(element_point_viewer->element_point_identifier));
			if ((ranges=Element_point_ranges_get_ranges(element_point_ranges))&&
				Multi_range_get_range(ranges,0,&start,&stop))
			{
				element_point_viewer->element_point_number=start;
			}
			else
			{
				element_point_viewer->element_point_number=0;
			}
			Element_point_viewer_calculate_xi(element_point_viewer);
			Element_point_viewer_refresh_chooser(element_point_viewer);
			Element_point_viewer_set_viewer_element_point(element_point_viewer);
			if (element_point_viewer->wx_element_point_viewer && element_point_viewer->collpane)
			{
				 FOR_EACH_OBJECT_IN_MANAGER(Computed_field)(
						element_point_viewer_add_collpane,
						(void *)element_point_viewer,
						element_point_viewer->computed_field_manager);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_point_viewer_element_point_ranges_selection_change.  "
			"Invalid argument(s)");
	}
	LEAVE;
} /* Element_point_viewer_element_point_ranges_selection_change */

static int Element_point_viewer_select_current_point(
	struct Element_point_viewer *element_point_viewer)
/*******************************************************************************
LAST MODIFIED : 30 May 2000

DESCRIPTION :
Makes the currently described element point the only one in the global
selection. Does nothing if no current element point.
==============================================================================*/
{
	int return_code;
	struct Element_point_ranges *element_point_ranges;

	ENTER(Element_point_viewer_select_current_point);
	if (element_point_viewer)
	{
		if (element_point_viewer->element_point_identifier.element)
		{
			element_point_ranges=CREATE(Element_point_ranges)(
				&(element_point_viewer->element_point_identifier));
			if (element_point_ranges)
			{
				Element_point_ranges_add_range(element_point_ranges,
					element_point_viewer->element_point_number,
					element_point_viewer->element_point_number);
				Element_point_ranges_selection_begin_cache(
					element_point_viewer->element_point_ranges_selection);
				Element_point_ranges_selection_clear(
					element_point_viewer->element_point_ranges_selection);
				return_code=
					Element_point_ranges_selection_select_element_point_ranges(
						element_point_viewer->element_point_ranges_selection,
						element_point_ranges);
				Element_point_ranges_selection_end_cache(
					element_point_viewer->element_point_ranges_selection);
				DESTROY(Element_point_ranges)(&element_point_ranges);
			}
			else
			{
				return_code=0;
			}
			if (!return_code)
			{
				display_message(ERROR_MESSAGE,
					"Element_point_viewer_select_current_point.  Failed");
			}
		}
		else
		{
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_point_viewer_select_current_point.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Element_point_viewer_select_current_point */

static int Element_point_viewer_get_grid(
	struct Element_point_viewer *element_point_viewer)
/*******************************************************************************
LAST MODIFIED : 30 May 2000

DESCRIPTION :
If there is a grid field defined for the element, gets its discretization and
sets the cmzn_element_point_sampling_mode to CMZN_ELEMENT_POINT_SAMPLING_MODE_CELL_CORNERS, otherwise
leaves the current discretization/mode intact.
==============================================================================*/
{
	int return_code;
	struct Computed_field *grid_field;
	struct FE_element *element;
	struct FE_field *grid_fe_field;

	ENTER(Element_point_viewer_get_grid);
	if (element_point_viewer)
	{
		return_code=1;
		element=element_point_viewer->element_point_identifier.element;
		if (element&&(grid_field=FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
			Computed_field_is_scalar_integer_grid_in_element,(void *)element,
			Computed_field_package_get_computed_field_manager(
				element_point_viewer->computed_field_package)))&&
			Computed_field_get_type_finite_element(grid_field, &grid_fe_field))
		{
			/* only checks first component */
			cmzn_elementfieldtemplate_id eft = cmzn_element_get_elementfieldtemplate(element, grid_field, 1);
			if (eft)
			{
				const int *numberInXi = eft->getLegacyGridNumberInXi();
				if (numberInXi)
				{
					const int dimension = element->getDimension();
					for (int d = 0; d < dimension; ++d)
						element_point_viewer->element_point_identifier.number_in_xi[d] = numberInXi[d];
					element_point_viewer->element_point_identifier.sampling_mode =
						CMZN_ELEMENT_POINT_SAMPLING_MODE_CELL_CORNERS;
				}
				cmzn_elementfieldtemplate_destroy(&eft);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_point_viewer_get_grid.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Element_point_viewer_get_grid */

/**
 * Callback for changes from the fieldmodule. If the element currently being
 * viewed has changed, re-send to viewer.
 * Note that we do not have to handle add, delete and identifier change messages
 * here as the select widget does this for us. Only changes to the content of the
 * object cause updates.
 */
static void cmzn_fieldmoduleevent_to_element_point_viewer(
	cmzn_fieldmoduleevent_id event, void *element_point_viewer_void)
{
	struct Element_point_viewer *element_point_viewer =
		static_cast<struct Element_point_viewer *>(element_point_viewer_void);
	if (event && element_point_viewer)
	{
		cmzn_fieldmodule_id fieldmodule = cmzn_region_get_fieldmodule(element_point_viewer->region);
		if (element_point_viewer->element_point_identifier.element)
		{
			const int dimension = get_FE_element_dimension(element_point_viewer->element_point_identifier.element);
			cmzn_mesh_id mesh = cmzn_fieldmodule_find_mesh_by_dimension(fieldmodule, dimension);
			cmzn_meshchanges_id meshchanges = cmzn_fieldmoduleevent_get_meshchanges(event, mesh);
			if (meshchanges)
			{
				int element_change = cmzn_meshchanges_get_element_change_flags(meshchanges,
					element_point_viewer->element_point_identifier.element);
				if (element_change & CMZN_ELEMENT_CHANGE_FLAG_FIELD)
				{
					/* update grid_text in case number changed */
					 //				Element_point_viewer_refresh_grid_value_text(element_point_viewer);
					Element_point_viewer_set_viewer_element_point(element_point_viewer);
				}
			}
			cmzn_meshchanges_destroy(&meshchanges);
			cmzn_mesh_destroy(&mesh);
		}
		cmzn_fieldmodule_destroy(&fieldmodule);
	}
}

static void element_point_viewer_time_change_callback(
	cmzn_timenotifierevent_id timenotifierevent,	void *element_point_field_viewer_void)
/*******************************************************************************
LAST MODIFIED : 6 June 2007

DESCRIPTION :
==============================================================================*/
{
	struct Element_point_viewer *element_point_viewer;

	ENTER(element_point_viewer_time_change_callback);
	USE_PARAMETER(timenotifierevent);
	if ((element_point_viewer =
		(struct Element_point_viewer *)element_point_field_viewer_void))
	{
		 //		element_point_field_viewer_widget_update_values(element_point_field_viewer);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"element_point_viewer_time_change_callback.  "
			"Invalid argument(s)");
	}

} /* element_point_viewer_time_change_callback */

/***************************************************************************//**
 * Convenience function for evaluating field or field component as string.
 * @param component_number  From 0 to field->number_of_components or -1 for all.
 */
static char *element_point_viewer_get_field_string(struct Element_point_viewer *element_point_viewer,
	 Computed_field *field, int component_number)
{
	char *value_string = 0;
	FE_value time, *xi;
	struct FE_element *element,*top_level_element;

	int number_of_components = cmzn_field_get_number_of_components(field);
	if (element_point_viewer &&
		(element = element_point_viewer->element_point_identifier.element) &&
		(top_level_element = element_point_viewer->element_point_identifier.top_level_element) &&
		(xi = element_point_viewer->xi) &&
		(component_number < number_of_components))
	{
		cmzn_fieldmodule_id field_module = cmzn_field_get_fieldmodule(field);
		cmzn_fieldcache_id field_cache = cmzn_fieldmodule_create_fieldcache(field_module);
		time = cmzn_timenotifier_get_time(element_point_viewer->time_object);
		cmzn_fieldcache_set_time(field_cache, time);
		int element_dimension = cmzn_element_get_dimension(element);
		cmzn_fieldcache_set_mesh_location_with_parent(field_cache, element, element_dimension, xi, top_level_element);
		if ((component_number < 0) || (1 == number_of_components))
		{
			value_string = cmzn_field_evaluate_string(field, field_cache);
		}
		else
		{
			FE_value *values = new FE_value[number_of_components];
			if (CMZN_OK == cmzn_field_evaluate_real(field, field_cache, number_of_components, values))
			{
				char tmp[50];
				sprintf(tmp, "%g", values[component_number]);
				value_string = duplicate_string(tmp);
			}
		}
		cmzn_fieldcache_destroy(&field_cache);
		cmzn_fieldmodule_destroy(&field_module);
		if (!value_string)
		{
			display_message(ERROR_MESSAGE,
				"element_point_viewer_get_field_string.  Could not get component as string");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"element_point_viewer_get_field_string.  Invalid argument(s)");
	}
	return (value_string);
}

class wxElementPointViewer : public wxFrame
{
	 Element_point_viewer *element_point_viewer;
	 wxPanel *node_text_panel, *element_text_panel, *top_element_text_panel,
			*grid_field_chooser_panel;
	 // 	 wxScrolledWindow *variable_viewer_panel;
	 wxFeElementTextChooser *element_text_chooser;
	 wxFeElementTextChooser *top_level_element_text_chooser;
	 wxFrame *frame;
	 wxTextCtrl *discretizationtextctrl, *pointtextctrl;
	 DEFINE_ENUMERATOR_TYPE_CLASS(cmzn_element_point_sampling_mode);
	 Enumerator_chooser<ENUMERATOR_TYPE_CLASS(cmzn_element_point_sampling_mode)>
	 *element_sample_mode_chooser;
	 DEFINE_MANAGER_CLASS(Computed_field);
	 Managed_object_chooser<Computed_field,MANAGER_CLASS(Computed_field)>
		*grid_field_chooser;
public:

	 wxElementPointViewer(Element_point_viewer *element_point_viewer):
			element_point_viewer(element_point_viewer)
	 {
			wxXmlInit_element_point_viewer_wx();
			element_point_viewer->wx_element_point_viewer = (wxElementPointViewer *)NULL;
			wxXmlResource::Get()->LoadFrame(this,
				 (wxWindow *)NULL, _T("CmguiElementPointViewer"));
			this->SetIcon(cmiss_icon_xpm);

			element_text_panel =
				 XRCCTRL(*this, "ElementTextPanel",wxPanel);
			element_text_chooser =
				 new wxFeElementTextChooser(element_text_panel,
						element_point_viewer->element_point_identifier.element,
						element_point_viewer->region,
						(LIST_CONDITIONAL_FUNCTION(FE_element) *)NULL,(void *)NULL);
			Callback_base<FE_element *> *element_text_callback =
				 new Callback_member_callback< FE_element*,
				 wxElementPointViewer, int (wxElementPointViewer::*)(FE_element *) >
				 (this, &wxElementPointViewer::element_text_callback);
			element_text_chooser->set_callback(element_text_callback);

			top_element_text_panel =
				 XRCCTRL(*this, "TopElementTextPanel",wxPanel);
			top_level_element_text_chooser =
				 new wxFeElementTextChooser(top_element_text_panel,
						element_point_viewer->element_point_identifier.top_level_element,
						element_point_viewer->region,
						FE_element_is_top_level,(void *)NULL);
			Callback_base<FE_element *> *top_element_text_callback =
				 new Callback_member_callback< FE_element*,
				 wxElementPointViewer, int (wxElementPointViewer::*)(FE_element *) >
				 (this, &wxElementPointViewer::top_element_text_callback);
			top_level_element_text_chooser->set_callback(top_element_text_callback);

			element_point_viewer->element_sample_mode_chooser_panel=XRCCTRL(
				 *this,"XiDiscretizationModePanel",wxPanel);
			element_sample_mode_chooser = new Enumerator_chooser<ENUMERATOR_TYPE_CLASS(cmzn_element_point_sampling_mode)>
				 (element_point_viewer->element_sample_mode_chooser_panel,
						element_point_viewer->element_point_identifier.sampling_mode,
						(ENUMERATOR_CONDITIONAL_FUNCTION(cmzn_element_point_sampling_mode) *)NULL,
						(void *)NULL, element_point_viewer->user_interface);
			element_point_viewer->element_sample_mode_chooser_panel->Fit();
			Callback_base< enum cmzn_element_point_sampling_mode > *element_sample_mode_callback =
				 new Callback_member_callback<enum cmzn_element_point_sampling_mode,
				 wxElementPointViewer, int(wxElementPointViewer::*)(enum cmzn_element_point_sampling_mode)>
				 (this, &wxElementPointViewer::element_sample_mode_callback);
			element_sample_mode_chooser->set_callback(element_sample_mode_callback);
			if (element_point_viewer->element_point_identifier.element)
			{
				 element_point_viewer->element_sample_mode_chooser_panel->Enable(1);
			}
			else
			{
				 element_point_viewer->element_sample_mode_chooser_panel->Enable(0);
			}

			element_point_viewer->discretizationtextctrl = XRCCTRL(*this,"DiscretizationTextCtrl",wxTextCtrl);
			Element_point_viewer_refresh_discretization_text(element_point_viewer);
			element_point_viewer->pointtextctrl = XRCCTRL(*this, "PointTextCtrl", wxTextCtrl);
			Element_point_viewer_refresh_point_number_text(element_point_viewer);
			element_point_viewer->xitextctrl =
				 XRCCTRL(*this,"XiTextCtrl", wxTextCtrl);
			Element_point_viewer_refresh_xi_text(element_point_viewer);

			struct Computed_field *grid_field = 0;
			struct MANAGER(Computed_field) *computed_field_manager;
			computed_field_manager = Computed_field_package_get_computed_field_manager(
				element_point_viewer->computed_field_package);
			if (computed_field_manager)
			{
				 if (!(grid_field=
							 FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field,name)(
									"grid_point_number",computed_field_manager)))
				 {
						grid_field=FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
							 Computed_field_is_scalar_integer,(void *)NULL,
							 computed_field_manager);
				 }
			}
			grid_field_chooser_panel=XRCCTRL(
				 *this,"GridFieldPanel",wxPanel);
			grid_field_chooser =
				 new Managed_object_chooser<Computed_field,MANAGER_CLASS(Computed_field)>
				 (grid_field_chooser_panel, grid_field, computed_field_manager,
						Computed_field_is_scalar_integer, (void *)NULL, element_point_viewer->user_interface);
			Callback_base< Computed_field* > *grid_field_callback =
				 new Callback_member_callback< Computed_field*,
				 wxElementPointViewer, int (wxElementPointViewer::*)(Computed_field *) >
				 (this, &wxElementPointViewer::grid_field_callback);
			grid_field_chooser->set_callback(grid_field_callback);

			element_point_viewer->gridvaluetextctrl=XRCCTRL(*this, "GridValueTextCtrl", wxTextCtrl);

			XRCCTRL(*this, "DiscretizationTextCtrl", wxTextCtrl)->Connect(wxEVT_KILL_FOCUS,
				 wxCommandEventHandler(wxElementPointViewer::OnDiscretizationValueEntered),
				 NULL, this);
			XRCCTRL(*this, "PointTextCtrl", wxTextCtrl)->Connect(wxEVT_KILL_FOCUS,
				 wxCommandEventHandler(wxElementPointViewer::OnPointValueEntered),
				 NULL, this);
			XRCCTRL(*this, "XiTextCtrl", wxTextCtrl)->Connect(wxEVT_KILL_FOCUS,
				 wxCommandEventHandler(wxElementPointViewer::OnXiValueEntered),
				 NULL, this);
			XRCCTRL(*this, "GridValueTextCtrl", wxTextCtrl)->Connect(wxEVT_KILL_FOCUS,
				 wxCommandEventHandler(wxElementPointViewer::OnGridValueEntered),
				 NULL, this);
			Show();
			frame = XRCCTRL(*this, "CmguiElementPointViewer",wxFrame);
			frame->GetSize(&(element_point_viewer->init_width), &(element_point_viewer->init_height));
			frame->SetSize(element_point_viewer->frame_width,element_point_viewer->frame_height+100);
			frame->GetSize(&(element_point_viewer->frame_width), &(element_point_viewer->frame_height));
			frame->Layout();
	 };

	 wxElementPointViewer()
	 {
	 };

  ~wxElementPointViewer()
	 {
			delete element_text_chooser;
			delete top_level_element_text_chooser;
			delete element_sample_mode_chooser;
			delete grid_field_chooser;
	 }

int element_text_callback(FE_element *element)
/*******************************************************************************
LAST MODIFIED :7 June 2007

DESCRIPTION :
Callback from wxTextChooser when text is entered.
==============================================================================*/
{
	 FE_value element_to_top_level[9];

	 if (element_point_viewer)
	 {
			/* don't select elements until there is a top_level_element */
			if (element_point_viewer->element_point_identifier.top_level_element&&
				 (element))
			{
				 element_point_viewer->element_point_identifier.element=element;
				 if (element)
				 {
						/* get top_level_element, keeping existing one if possible */
						element_point_viewer->element_point_identifier.top_level_element=
							 FE_element_get_top_level_element_conversion(
									element_point_viewer->element_point_identifier.element,
									element_point_viewer->element_point_identifier.top_level_element,
									/*face*/CMZN_ELEMENT_FACE_TYPE_ALL, element_to_top_level);
						top_level_element_text_chooser->set_object(
							 element_point_viewer->element_point_identifier.top_level_element);
						//				top_element_text_callback(
						// 					 element_point_viewer->element_point_identifier.top_level_element);
						element_point_viewer->element_point_number=0;
						if (CMZN_ELEMENT_POINT_SAMPLING_MODE_CELL_CORNERS==
							 element_point_viewer->element_point_identifier.sampling_mode)
						{
							 Element_point_viewer_get_grid(element_point_viewer);
						}
						Element_point_viewer_calculate_xi(element_point_viewer);
						Element_point_viewer_select_current_point(element_point_viewer);
				 }
			}
	 }
	 else
	 {
			display_message(ERROR_MESSAGE,
				 "Element_point_viewer_update_element.  Invalid argument(s)");
	 }
	 return 1;
}

int top_element_text_callback(FE_element *top_level_element)
/*******************************************************************************
LAST MODIFIED :7 June 2007

DESCRIPTION :
Callback from wxTextChooser when text is entered.
==============================================================================*/
{
	 int i;

	 if (element_point_viewer)
	 {
			if (top_level_element)
			{
				 if (element_point_viewer->element_point_identifier.element)
				 {
						if (top_level_element&&
							FE_element_is_top_level_parent_of_element(top_level_element,
								element_point_viewer->element_point_identifier.element))
						{
							element_point_viewer->element_point_identifier.top_level_element=
								top_level_element;
						}
						else
						{
							element_point_viewer->element_point_identifier.top_level_element =
								cmzn_element_get_first_top_level_ancestor(element_point_viewer->element_point_identifier.element);
							top_level_element_text_chooser->set_object(
									element_point_viewer->element_point_identifier.top_level_element);
						}
				 }
				 else
				 {
						/* use the top_level_element for the element too */
						element_point_viewer->element_point_identifier.element=
							 top_level_element;
						element_text_chooser->set_object(
							 element_point_viewer->element_point_identifier.element);
						element_point_viewer->element_point_identifier.top_level_element=
							 top_level_element;
						/* get the element point at the centre of top_level_element */
						element_point_viewer->element_point_identifier.sampling_mode=
							 CMZN_ELEMENT_POINT_SAMPLING_MODE_SET_LOCATION;
						for (i=0;i<MAXIMUM_ELEMENT_XI_DIMENSIONS;i++)
						{
							 element_point_viewer->element_point_identifier.number_in_xi[i]=1;
							 element_point_viewer->xi[i]=
									element_point_viewer->element_point_identifier.exact_xi[i]=0.5;
						}
						element_point_viewer->element_point_number=0;
						/* try to replace with a grid point, if possible */
						Element_point_viewer_get_grid(element_point_viewer);
				 }
				 Element_point_viewer_select_current_point(element_point_viewer);
			}
			else
			{
				 /* must be no element if no top_level_element */
				 element_point_viewer->element_point_identifier.element =
						(struct FE_element *)NULL;
				 Element_point_viewer_refresh_sample_mode(element_point_viewer);
				 Element_point_viewer_refresh_discretization_text(element_point_viewer);
				 Element_point_viewer_refresh_point_number_text(element_point_viewer);
				 Element_point_viewer_refresh_xi_text(element_point_viewer);
				 Element_point_viewer_refresh_grid_value_text(element_point_viewer);
				 Element_point_viewer_set_viewer_element_point(element_point_viewer);
			}
	 }
	 else
	 {
			display_message(ERROR_MESSAGE,
				 "Element_point_viewer_update_top_level_element.  Invalid argument(s)");
	 }

			return 1;
}

int element_sample_mode_callback(enum cmzn_element_point_sampling_mode sampling_mode)
/*******************************************************************************
LAST MODIFIED : 22 March 2007

DESCRIPTION :
Callback from wxChooser<xi Discretization Mode> when choice is made.
==============================================================================*/
{
	 int i, is_editable;
	struct Element_point_ranges_identifier temp_element_point_identifier;
	is_editable = 0;
	if (element_point_viewer)
	{
		/* store old identifier unless in case new one is invalid */
		COPY(Element_point_ranges_identifier)(&temp_element_point_identifier,
			&(element_point_viewer->element_point_identifier));
		if (sampling_mode)
		{
			element_point_viewer->element_point_identifier.sampling_mode=
				sampling_mode;
			if (CMZN_ELEMENT_POINT_SAMPLING_MODE_CELL_CORNERS==sampling_mode)
			{
				Element_point_viewer_get_grid(element_point_viewer);
			}
			else if (CMZN_ELEMENT_POINT_SAMPLING_MODE_SET_LOCATION==sampling_mode)
			{
				for (i=0;i<MAXIMUM_ELEMENT_XI_DIMENSIONS;i++)
				{
					element_point_viewer->element_point_identifier.number_in_xi[i]=1;
					is_editable = 1;
				}
			}
			if (element_point_viewer->xitextctrl)
			{
				 element_point_viewer->xitextctrl->SetEditable(is_editable);
			}
			element_point_viewer->element_point_number=0;
			if (Element_point_ranges_identifier_is_valid(
				&(element_point_viewer->element_point_identifier)))
			{
				Element_point_viewer_calculate_xi(element_point_viewer);
				Element_point_viewer_select_current_point(element_point_viewer);
			}
			else
			{
				COPY(Element_point_ranges_identifier)(
					&(element_point_viewer->element_point_identifier),
					&temp_element_point_identifier);
				/* always restore mode to actual value in use */
				Element_point_viewer_refresh_sample_mode(
					element_point_viewer);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_point_viewer_update_sample_mode.  "
			"Invalid argument(s)");
	}
	 return 1;
}

int grid_field_callback(Computed_field *grid_field)
/*******************************************************************************
LAST MODIFIED :7 June 2007

DESCRIPTION :
Callback from wxTextChooser when text is entered.
==============================================================================*/
{
	USE_PARAMETER(grid_field);
	 if (element_point_viewer)
	 {
			Element_point_viewer_refresh_grid_value_text(element_point_viewer);
	 }
	 else
	 {
			display_message(ERROR_MESSAGE,
				 "Element_point_viewer_update_grid_field.  Invalid argument(s)");
	 }

	 return 1;
}

void RenewElementPointViewer(wxCollapsiblePaneEvent& event)
/*******************************************************************************
LAST MODIFIED : 14 June 2007

DESCRIPTION :
==============================================================================*/
 {
	USE_PARAMETER(event);
		wxScrolledWindow *VariableViewer = XRCCTRL(
			 *this,"ElementVariableViewerPanel",wxScrolledWindow);
		VariableViewer->Layout();
		frame = XRCCTRL(*this, "CmguiElementPointViewer", wxFrame);
		frame->SetMinSize(wxSize(50,100));
		frame->SetMaxSize(wxSize(2000,2000));
 }

void OnPointValueEntered(wxCommandEvent &event)
/*******************************************************************************
LAST MODIFIED : 14 June 2007

DESCRIPTION :
Called when entry is made into the point_number_text field.
==============================================================================*/
{
	const char *value_string;
	int element_point_number;

	USE_PARAMETER(event);
	if (element_point_viewer)
	{
		/* Get the text string */
		wxString point_string = element_point_viewer->pointtextctrl->GetValue();
		value_string = point_string.mb_str(wxConvUTF8);
		if (value_string)
		{
			if ((1==sscanf(value_string,"%d",&element_point_number))&&
				Element_point_ranges_identifier_element_point_number_is_valid(
					&(element_point_viewer->element_point_identifier),
					element_point_number))
			{
				element_point_viewer->element_point_number=element_point_number;
				Element_point_viewer_select_current_point(element_point_viewer);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Element_point_viewer_point_number_text_CB.  Invalid point number");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Element_point_viewer_point_number_text_CB.  Missing text");
		}
		/* always restore point_number_text to actual value stored */
		Element_point_viewer_refresh_point_number_text(element_point_viewer);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_point_viewer_point_number_text_CB.  Invalid argument(s)");
	}
} /* Element_point_viewer_point_number_text_CB */

void OnDiscretizationValueEntered(wxCommandEvent &event)
/*******************************************************************************
LAST MODIFIED : 14 June 2007

DESCRIPTION :
Called when entry is made into the discretization_text field.
==============================================================================*/
{
	const char *value_string;
	struct Element_discretization discretization,temp_discretization;
	struct Parse_state *temp_state;

	USE_PARAMETER(event);
	if (element_point_viewer)
	{
		/* Get the text string */
		wxString discretization_string = element_point_viewer->discretizationtextctrl->GetValue();
		value_string = discretization_string.mb_str(wxConvUTF8);
		if (value_string)
		{
			temp_state=create_Parse_state(value_string);
			if (temp_state)
			{
				temp_discretization.number_in_xi1=
					element_point_viewer->element_point_identifier.number_in_xi[0];
				temp_discretization.number_in_xi2=
					element_point_viewer->element_point_identifier.number_in_xi[1];
				temp_discretization.number_in_xi3=
					element_point_viewer->element_point_identifier.number_in_xi[2];
				if (set_Element_discretization(temp_state,(void *)&discretization,
							(void *)element_point_viewer->user_interface))
				{
					element_point_viewer->element_point_identifier.number_in_xi[0]=
						discretization.number_in_xi1;
					element_point_viewer->element_point_identifier.number_in_xi[1]=
						discretization.number_in_xi2;
					element_point_viewer->element_point_identifier.number_in_xi[2]=
						discretization.number_in_xi3;
					if (Element_point_ranges_identifier_is_valid(
						&(element_point_viewer->element_point_identifier)))
					{
						element_point_viewer->element_point_number=0;
						Element_point_viewer_select_current_point(element_point_viewer);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Element_point_viewer_discretization_text_CB.  "
							"Invalid element point");
						element_point_viewer->element_point_identifier.number_in_xi[0]=
							temp_discretization.number_in_xi1;
						element_point_viewer->element_point_identifier.number_in_xi[1]=
							temp_discretization.number_in_xi2;
						element_point_viewer->element_point_identifier.number_in_xi[2]=
							temp_discretization.number_in_xi3;
					}
				}
				destroy_Parse_state(&temp_state);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Element_point_viewer_discretization_text_CB.  "
					"Could not create parse state");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Element_point_viewer_discretization_text_CB.  Missing text");
		}

		/* always restore discretization_text to actual value stored */
		Element_point_viewer_refresh_discretization_text(element_point_viewer);
	}
	else
	{
		 display_message(ERROR_MESSAGE,
				"Element_point_viewer_discretization_text_CB.  Invalid argument(s)");
	}
} /* Element_point_viewer_discretization_text_CB */

void OnXiValueEntered(wxCommandEvent &event)
/*******************************************************************************
LAST MODIFIED : 14 June 2007

DESCRIPTION :
Called when entry is made into the xi_text field.
==============================================================================*/
{
	const char *value_string;
	float xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
	int dimension,i;
	struct FE_element *element;
	struct Parse_state *temp_state;

	USE_PARAMETER(event);
	if (element_point_viewer)
	{
		if ((NULL!= (element = element_point_viewer->element_point_identifier.element)) &&
			(0 != (dimension = get_FE_element_dimension(element))))
		{
			/* Get the text string */
			wxString xi_string = element_point_viewer->xitextctrl->GetValue();
			value_string = xi_string.mb_str(wxConvUTF8);
			if (value_string)
			{
				/* clean up spaces? */
				temp_state=create_Parse_state(value_string);
					if (temp_state)
					{
						if (set_float_vector(temp_state,xi,(void *)&dimension))
						{
							for (i=0;i<dimension;i++)
							{
								element_point_viewer->element_point_identifier.exact_xi[i] = xi[i];
							}
							Element_point_viewer_select_current_point(element_point_viewer);
					}
					destroy_Parse_state(&temp_state);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Element_point_viewer_xi_text_CB.  Missing text");
			}
		}
		/* always restore xi_text to actual value stored */
		Element_point_viewer_refresh_xi_text(element_point_viewer);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			 "Element_point_viewer_xi_text_CB.  Invalid argument(s)");
	}
} /* Element_point_viewer_xi_text_CB */

void OnGridValueEntered(wxCommandEvent &event)
{
	const char *value_string;
	int grid_value;
	struct Computed_field *grid_field;
	struct FE_element_grid_to_Element_point_ranges_list_data grid_to_list_data;
	struct FE_field *grid_fe_field;
	USE_PARAMETER(event);
	if (element_point_viewer)
	{
		/* Get the text string */
		wxString grid_value_string = element_point_viewer->gridvaluetextctrl->GetValue();
		value_string= grid_value_string.mb_str(wxConvUTF8);
		if (value_string)
		{
			grid_field=get_grid_field();
				if (grid_field && Computed_field_get_type_finite_element(grid_field,&grid_fe_field))
				{
					if (1==sscanf(value_string,"%d",&grid_value))
					{
						grid_to_list_data.grid_value_ranges=CREATE(Multi_range)();
							if (grid_to_list_data.grid_value_ranges &&
								Multi_range_add_range(grid_to_list_data.grid_value_ranges,
									grid_value,grid_value))
							{
								grid_to_list_data.element_point_ranges_list=CREATE(LIST(Element_point_ranges))();
								if (grid_to_list_data.element_point_ranges_list)
								{
									grid_to_list_data.grid_fe_field=grid_fe_field;
									/* inefficient: go through every element in FE_region */
									FE_region_for_each_FE_element(cmzn_region_get_FE_region(element_point_viewer->region),
										FE_element_grid_to_Element_point_ranges_list,
										(void *)&grid_to_list_data);
									if (0<NUMBER_IN_LIST(Element_point_ranges)(
										grid_to_list_data.element_point_ranges_list))
									{
										Element_point_ranges_selection_begin_cache(
											element_point_viewer->element_point_ranges_selection);
										Element_point_ranges_selection_clear(
											element_point_viewer->element_point_ranges_selection);
										FOR_EACH_OBJECT_IN_LIST(Element_point_ranges)(
											Element_point_ranges_select,
											(void *)element_point_viewer->element_point_ranges_selection,
											grid_to_list_data.element_point_ranges_list);
										Element_point_ranges_selection_end_cache(
											element_point_viewer->element_point_ranges_selection);
									}
									DESTROY(LIST(Element_point_ranges))(
										&(grid_to_list_data.element_point_ranges_list));
							}
							DESTROY(Multi_range)(&(grid_to_list_data.grid_value_ranges));
						}
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Element_point_viewer_grid_value_text_CB.  Missing text");
		}
		/* always restore grid_value_text to actual value stored */
		Element_point_viewer_refresh_grid_value_text(element_point_viewer);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_point_viewer_grid_value_text_CB.  Invalid argument(s)");
	}
}

void OnApplypressed(wxCommandEvent &event)
{
	USE_PARAMETER(event);
	//Element_point_viewer_apply_changes(element_point_viewer,/*apply_all*/0);
}

void OnApplyAllpressed(wxCommandEvent &event)
{
	USE_PARAMETER(event);
	//Element_point_viewer_apply_changes(element_point_viewer,/*apply_all*/1);
}

void OnRevertpressed(wxCommandEvent &event)
/*******************************************************************************
LAST MODIFIED : 15 June 2007

DESCRIPTION :
Callback for Revert button. Sends global element point values back to the
editor widget, undoing any modifications.
==============================================================================*/
{
	USE_PARAMETER(event);
	if (element_point_viewer)
	{
		Element_point_viewer_set_viewer_element_point(element_point_viewer);
		if (element_point_viewer->wx_element_point_viewer && element_point_viewer->collpane)
		{
			 FOR_EACH_OBJECT_IN_MANAGER(Computed_field)(
					element_point_viewer_add_collpane,
					(void *)element_point_viewer,
					element_point_viewer->computed_field_manager);
		}
	}
}

void OnCancelpressed(wxCommandEvent &event)
/*******************************************************************************
LAST MODIFIED : 16 June 2007

DESCRIPTION :
Callback from the Close button.
Also called when "close" is selected from the window menu.
==============================================================================*/
{
	USE_PARAMETER(event);
	if (element_point_viewer)
	{
		DESTROY(Element_point_viewer)(
			element_point_viewer->element_point_viewer_address);
	}
	else
	{
		display_message(WARNING_MESSAGE,
			"Element_point_viewer_close_CB.  Invalid argument(s)");
	}
} /* Element_point_viewer_close_CB */


void FrameSetSize(wxSizeEvent &event)
/*******************************************************************************
LAST MODIFIED : 19 June 2007

DESCRIPTION :
Callback of size event to prevent minising the size of the windows
after a collapsible pane is opened/closed.
==============================================================================*/
{
	 int temp_width, temp_height;

	USE_PARAMETER(event);
	 frame = XRCCTRL(*this, "CmguiElementPointViewer",wxFrame);
	 frame->Freeze();
	 frame->GetSize(&temp_width, &temp_height);
	 if (temp_height !=element_point_viewer->frame_height || temp_width !=element_point_viewer->frame_width)
	 {
			if (temp_width>element_point_viewer->init_width || temp_height>element_point_viewer->init_height)
			{
				 element_point_viewer->frame_width = temp_width;
				 element_point_viewer->frame_height = temp_height;
			}
			else
			{
				 frame->SetSize(element_point_viewer->frame_width,element_point_viewer->frame_height);
			}
	 }
	 frame->Thaw();
	 frame->Layout();
}

void Terminate(wxCloseEvent& event)
{
	USE_PARAMETER(event);
	 if (element_point_viewer)
	 {
			DESTROY(Element_point_viewer)(
				 element_point_viewer->element_point_viewer_address);
	 }
}

void ElementPointViewerTextEntered(wxTextCtrl *textctrl ,
	 Element_point_viewer *element_point_field_viewer, Computed_field *field, int component_number)
/*******************************************************************************
LAST MODIFIED : 14 June 2007

DESCRIPTION :
Called when the user has changed the data in the text widget.  Processes the
data, and then changes the correct value in the array structure.
==============================================================================*/
{
	char *field_value_string;
	const char*value_string;
	FE_value time,value,*values,*xi;
	int dimension,element_point_number,i,int_value,
		number_in_xi[MAXIMUM_ELEMENT_XI_DIMENSIONS],
		number_of_grid_values,return_code;
	struct FE_element *element,*top_level_element;
	struct FE_field *fe_field;

	int number_of_components = cmzn_field_get_number_of_components(field);
	if ((element_point_field_viewer) &&
		(element=element_point_field_viewer->element_point_identifier.element)&&
		(top_level_element=
			element_point_field_viewer->element_point_identifier.top_level_element)&&
			(xi=element_point_field_viewer->xi)&&
			(field)&&
			(0 <= component_number) && (component_number < number_of_components) &&
			(element_point_viewer->current_field=field))
	{
		time = cmzn_timenotifier_get_time(element_point_field_viewer->time_object);
		/* get old_value_string to prevent needless updating and preserve text
				 selections for cut-and-paste */
		wxString tmpstr = textctrl->GetValue();
		value_string=tmpstr.mb_str(wxConvUTF8);
		if (value_string)
		{
			if ((CMZN_ELEMENT_POINT_SAMPLING_MODE_CELL_CORNERS==element_point_field_viewer->
				element_point_identifier.sampling_mode)&&
				Computed_field_get_native_discretization_in_element(field,element,
					number_in_xi))
			{
				/* get the number of values that are stored in the grid */
				dimension=get_FE_element_dimension(element);
				number_of_grid_values=1;
				for (i=0;i<dimension;i++)
				{
					number_of_grid_values *= (number_in_xi[i]+1);
				}
				element_point_number=element_point_field_viewer->element_point_number;
				/* check the element_point_number is valid */
				if ((0<=element_point_number)&&
					(element_point_number<number_of_grid_values))
				{
					return_code=0;
					if (Computed_field_is_type_finite_element(field)&&
						Computed_field_get_type_finite_element(field,&fe_field)&&
						(INT_VALUE==get_FE_field_value_type(fe_field)))
					{
						/* handle integer value_type separately to avoid inaccuracies of
										 real->integer conversion */
						if (1==sscanf(value_string,"%d",&int_value))
						{
							FE_mesh_field_data *meshFieldData;
							FE_mesh_field_template *mft;
							FE_element_field_template *eft;
							if ((meshFieldData = fe_field->getMeshFieldData(element->getMesh()))
								&& (mft = meshFieldData->getComponentMeshfieldtemplate(component_number))
								&& (eft = mft->getElementfieldtemplate(element->getIndex())))
							{
								const int numberOfElementDOFs = eft->getNumberOfElementDOFs();
								auto component = static_cast<FE_mesh_field_data::Component<int>*>(meshFieldData->getComponentBase(component_number));
								if ((component) && (numberOfElementDOFs > 0) && (element_point_number < numberOfElementDOFs))
								{
									int *int_values = component->getElementValues(element->getIndex(), numberOfElementDOFs);
									// change the value for this element point
									// GRC there should be convenience methods to do this and notify; but grid fields are internal only.
									int_values[element_point_number] = int_value;
									element->getMesh()->get_FE_region()->FE_field_change(fe_field, CHANGE_LOG_RELATED_OBJECT_CHANGED(FE_field));
									element->getMesh()->elementChange(element->getIndex(), DS_LABEL_CHANGE_TYPE_RELATED);
								}
							}
						}
					}
					else
					{
						if (1==sscanf(value_string,FE_VALUE_INPUT_STRING,&value))
						{
							if (ALLOCATE(values, FE_value, number_of_components))
							{
								cmzn_fieldmodule_id field_module = cmzn_field_get_fieldmodule(field);
								cmzn_fieldcache_id field_cache = cmzn_fieldmodule_create_fieldcache(field_module);
								if ((CMZN_OK == cmzn_fieldcache_set_time(field_cache, time)) &&
									(CMZN_OK == cmzn_fieldcache_set_mesh_location(field_cache, element, MAXIMUM_ELEMENT_XI_DIMENSIONS, xi)) &&
									(CMZN_OK == cmzn_field_evaluate_real(field, field_cache, number_of_components, values)))
								{
									values[component_number] = value;
									return_code = (CMZN_OK == cmzn_field_assign_real(field, field_cache, number_of_components, values));
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"ElementPointViewerTextEntered.  "
										"Unable to evaluate field component values");
								}
								DEALLOCATE(values);
								cmzn_fieldcache_destroy(&field_cache);
								cmzn_fieldmodule_destroy(&field_module);
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"ElementPointViewerTextEntered.  "
									"Unable to allocate temporary storage.");
							}
						}
					}
					if (return_code)
					{
						/* add this field component to the modified list */
						Field_value_index_ranges_list_add_field_value_index(
							element_point_field_viewer->modified_field_components,
							field,component_number);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"ElementPointViewerTextEntered.  "
						"element_point_number out of range");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,"ElementPointViewerTextEntered.  "
					"Cannot set values for non-grid-based field");
			}
		}
		/* redisplay the actual value for the field component */
		field_value_string = element_point_viewer_get_field_string(
			element_point_viewer, field, component_number);
		if (field_value_string)
		{
			/* only set string from field if different from that shown */
			if (strcmp(field_value_string,value_string))
			{
				textctrl->SetValue(wxString::FromAscii(field_value_string));
			}
			DEALLOCATE(field_value_string);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"ElementPointViewerTextEntered.  Could not get component as string");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"ElementPointViewerTextEntered.  Invalid argument(s)");
	}
}

int set_element_selection(Element_point_viewer *element_point_viewer)
{
	 return element_text_chooser->set_object(element_point_viewer->element_point_identifier.element);
}

int set_top_level_element_selection(Element_point_viewer *element_point_viewer)
{
	 return top_level_element_text_chooser->set_object(
			element_point_viewer->element_point_identifier.top_level_element);
}

struct Computed_field *get_grid_field()
{
	 Computed_field *grid_field;
	 if (grid_field_chooser)
	 {
			grid_field = grid_field_chooser->get_object();
	 }
	 else
	 {
			grid_field = (Computed_field *)NULL;
	 }
	 return (grid_field);
}

 void set_element_sample_mode_chooser_value(Element_point_viewer *element_point_viewer)
{
	 if (element_sample_mode_chooser)
	 {
			element_sample_mode_chooser->set_value(element_point_viewer->
				 element_point_identifier.sampling_mode);
	 }
}

  DECLARE_DYNAMIC_CLASS(wxElementPointViewer);
  DECLARE_EVENT_TABLE();
};

IMPLEMENT_DYNAMIC_CLASS(wxElementPointViewer, wxFrame)
BEGIN_EVENT_TABLE(wxElementPointViewer, wxFrame)
	 EVT_COLLAPSIBLEPANE_CHANGED(wxID_ANY,wxElementPointViewer::RenewElementPointViewer)
	 EVT_TEXT_ENTER(XRCID("DiscretizationTextCtrl"),wxElementPointViewer::OnDiscretizationValueEntered)
	 EVT_TEXT_ENTER(XRCID("PointTextCtrl"),wxElementPointViewer::OnPointValueEntered)
	 EVT_TEXT_ENTER(XRCID("XiTextCtrl"),wxElementPointViewer::OnXiValueEntered)
	 EVT_TEXT_ENTER(XRCID("GridValueTextCtrl"),wxElementPointViewer::OnGridValueEntered)
	 EVT_BUTTON(XRCID("ApplyButton"),wxElementPointViewer::OnApplypressed)
	 EVT_BUTTON(XRCID("ApplyAllButton"),wxElementPointViewer::OnApplyAllpressed)
	 EVT_BUTTON(XRCID("RevertButton"),wxElementPointViewer::OnRevertpressed)
	 EVT_BUTTON(XRCID("CancelButton"),wxElementPointViewer::OnCancelpressed)
#if !defined (__WXGTK__)
	 EVT_SIZE(wxElementPointViewer::FrameSetSize)
#endif /*!defined (__WXGTK__)*/
	 EVT_CLOSE(wxElementPointViewer::Terminate)
END_EVENT_TABLE()

class wxElementPointViewerTextCtrl : public wxTextCtrl
{
	 Element_point_viewer *element_point_viewer;
	 struct Computed_field *field;
	 int component_number;

public:

  wxElementPointViewerTextCtrl(Element_point_viewer *element_point_viewer,
		 Computed_field *field,
		 int component_number) :
		 element_point_viewer(element_point_viewer), field(field),
		 component_number(component_number)
  {
  }

  ~wxElementPointViewerTextCtrl()
  {
  }

  void OnElementPointViewerTextCtrlEntered(wxCommandEvent& event)
  {
		USE_PARAMETER(event);
		 element_point_viewer->wx_element_point_viewer->ElementPointViewerTextEntered
				(this, element_point_viewer, field, component_number);
	}

};

int element_point_viewer_add_textctrl(int editable, struct Element_point_viewer *element_point_viewer,
	 Computed_field *field, int component_number)
/*******************************************************************************
LAST MODIFIED : 12 June 2007

DESCRIPTION :
Add textctrl box onto the viewer.
==============================================================================*/
{
	 char *temp_string;
	 wxElementPointViewerTextCtrl *element_point_viewer_text =
			new wxElementPointViewerTextCtrl(element_point_viewer, field, component_number);
	 temp_string = element_point_viewer_get_field_string(element_point_viewer, field, component_number);
	 if (temp_string)
	 {
			if (editable)
			{
				 element_point_viewer_text ->Create(element_point_viewer->element_point_win,
						-1, wxString::FromAscii(temp_string), wxPoint(100,component_number * 35),wxDefaultSize,wxTE_PROCESS_ENTER);
			}
			else
			{
				 element_point_viewer_text ->Create (element_point_viewer->element_point_win, -1,
						wxString::FromAscii(temp_string) ,wxPoint(100,component_number * 35),wxDefaultSize,wxTE_READONLY);
				 element_point_viewer_text->SetBackgroundColour(wxColour(231, 231, 231, 255));
			}
			DEALLOCATE(temp_string);
	 }
	 else
	 {
			element_point_viewer_text->Create (element_point_viewer->element_point_win, -1, wxString::FromAscii("ERROR")
				 , wxPoint(100,component_number * 35), wxDefaultSize,wxTE_PROCESS_ENTER);
	 }
	 if (editable)
	 {
			element_point_viewer_text->Connect(wxEVT_COMMAND_TEXT_ENTER,
				 wxCommandEventHandler(wxElementPointViewerTextCtrl::
						OnElementPointViewerTextCtrlEntered));
			element_point_viewer_text->Connect(wxEVT_KILL_FOCUS,
				 wxCommandEventHandler(wxElementPointViewerTextCtrl::OnElementPointViewerTextCtrlEntered));
	 }
	 element_point_viewer->element_point_grid_field->Add(element_point_viewer_text, 0,
            wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL|wxFIXED_MINSIZE, 0);
	 return (1);
}

static int element_point_viewer_add_collpane(struct Computed_field *current_field,
	 void *element_point_viewer_void)
{
	 char *field_name;
	struct Element_point_ranges_identifier temp_element_point_identifier;
	int temp_element_point_number;
	 Element_point_viewer *element_point_viewer = (struct Element_point_viewer *)element_point_viewer_void;
	 field_name = (char *)NULL;
	 GET_NAME(Computed_field)(current_field, &field_name);
	 element_point_viewer->current_field = current_field;
	 int condition;
	 if (element_point_viewer->element_point_identifier.element)
	 {
				 condition = Computed_field_is_defined_in_element_conditional(
						current_field, (void *)element_point_viewer->element_point_identifier.element);
	 }
	 else
	 {
			condition = 0;
	 }
	 if (condition)
	 {
			wxScrolledWindow *panel = element_point_viewer->collpane;
			char *identifier;
			identifier = (char *)NULL;
			const size_t length = strlen(field_name);
			COPY(Element_point_ranges_identifier)(
				 &temp_element_point_identifier,
				 &(element_point_viewer->element_point_identifier));
			temp_element_point_number=
				 element_point_viewer->element_point_number;
			/* Check the collapsible panel is created, if yes, renew the
				 content, if collapsible panel does not exist, create a new one */
			if (ALLOCATE(identifier,char,length+length+length+2))
			{
				 strcpy(identifier, field_name);
				 strcat(identifier, field_name);
				 strcat(identifier, field_name);
				 identifier[length+length+length+1]='\0';
			}
			element_point_viewer->element_point_win = panel->FindWindowByName(wxString::FromAscii(identifier));
			if (element_point_viewer->element_point_win)
			{
				 element_point_viewer->element_point_win->DestroyChildren();
				 element_point_viewer->paneSz = new wxBoxSizer(wxVERTICAL);
			}
			else
			{
				 wxCollapsiblePane *collapsiblepane = new wxCollapsiblePane;
				 wxSizer *sizer = panel->GetSizer();
				 collapsiblepane->Create(panel, /*id*/-1, wxString::FromAscii(field_name));
				 sizer->Add(collapsiblepane, 0,wxALL, 5);
				 element_point_viewer->element_point_win = collapsiblepane->GetPane();
				 element_point_viewer->element_point_win->SetName(wxString::FromAscii(identifier));
				 element_point_viewer->paneSz = new wxBoxSizer(wxVERTICAL);
			}
			if (identifier)
			{
				 DEALLOCATE(identifier);
			}
			if (element_point_viewer_set_element_point_field((void *)element_point_viewer,
						&temp_element_point_identifier,
						temp_element_point_number, element_point_viewer->current_field))
			{
				 if (element_point_viewer->element_point_grid_field != NULL)
				 {
						element_point_viewer->element_point_win->SetSizer(
							 element_point_viewer->element_point_grid_field);
						element_point_viewer->element_point_grid_field->SetSizeHints(
							 element_point_viewer->element_point_win);
						element_point_viewer->element_point_grid_field->Layout();
						element_point_viewer->element_point_win->Layout();
				 }
			}
			element_point_viewer->collpane->Layout();
			panel->FitInside();
			panel->SetScrollbars(20, 20, 50, 50);
			panel->Layout();
			wxFrame *frame;
			frame =
				 XRCCTRL(*element_point_viewer->wx_element_point_viewer, "CmguiElementPointViewer", wxFrame);
			frame->Layout();
			frame->SetMinSize(wxSize(50,100));
	 }
	 if (field_name)
	 {
			DEALLOCATE(field_name);
	 }
	 return 1;
}

/*
Global functions
----------------
*/
struct Element_point_viewer *CREATE(Element_point_viewer)(
	struct Element_point_viewer **element_point_viewer_address,
	struct cmzn_region *region,
	struct Element_point_ranges_selection *element_point_ranges_selection,
	struct Computed_field_package *computed_field_package,
	struct Time_object *time_object,
	struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 6 June 2007

DESCRIPTION :
Creates a dialog for choosing element points and displaying and editing their
fields.
==============================================================================*/
{
	int i, start, stop, temp_element_point_number;
	struct MANAGER(Computed_field) *computed_field_manager;
	struct Element_point_ranges *element_point_ranges;
	struct Element_point_ranges_identifier temp_element_point_identifier;
	struct Element_point_viewer *element_point_viewer;
	struct Multi_range *ranges;

	ENTER(CREATE(Element_point_viewer));
	element_point_viewer=(struct Element_point_viewer *)NULL;
	if (element_point_viewer_address && region &&
		element_point_ranges_selection&&computed_field_package &&
		(computed_field_manager = Computed_field_package_get_computed_field_manager(
			computed_field_package)) && user_interface)
	{
		 /* allocate memory */
		 if (ALLOCATE(element_point_viewer,struct Element_point_viewer,1)&&
				(element_point_viewer->modified_field_components=
					 CREATE(LIST(Field_value_index_ranges))()))
		 {
				element_point_viewer->computed_field_manager=computed_field_manager;
				element_point_viewer->computed_field_package=computed_field_package;
				element_point_viewer->element_point_viewer_address=
					 element_point_viewer_address;
				element_point_viewer->region = region;
				cmzn_fieldmodule_id fieldmodule = cmzn_region_get_fieldmodule(region);
				element_point_viewer->fieldmodulenotifier = cmzn_fieldmodule_create_fieldmodulenotifier(fieldmodule);
				cmzn_fieldmodulenotifier_set_callback(element_point_viewer->fieldmodulenotifier,
					cmzn_fieldmoduleevent_to_element_point_viewer, (void *)element_point_viewer);
				cmzn_fieldmodule_destroy(&fieldmodule);
				element_point_viewer->element_point_ranges_selection=
					 element_point_ranges_selection;
				element_point_viewer->user_interface=user_interface;
				element_point_viewer->time_object = ACCESS(Time_object)(
					 time_object);
				element_point_viewer->element_point_identifier.element=
					 (struct FE_element *)NULL;
				element_point_viewer->element_point_identifier.top_level_element=
					 (struct FE_element *)NULL;
				element_point_viewer->element_point_identifier.sampling_mode=
					 CMZN_ELEMENT_POINT_SAMPLING_MODE_SET_LOCATION;
				//				element_point_viewer->time_object_callback = 0;
				element_point_viewer->number_of_components=-1;
				for (i=0;i<MAXIMUM_ELEMENT_XI_DIMENSIONS;i++)
				{
					 element_point_viewer->element_point_identifier.number_in_xi[i]=1;
					 element_point_viewer->xi[i]=
							element_point_viewer->element_point_identifier.exact_xi[i]=0.5;
				}
				element_point_viewer->element_point_number=0;
				element_point_viewer->match_grid_field=
					 FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field,name)(
							"grid_point_number",computed_field_manager);
				element_point_viewer->current_field=(struct Computed_field *)NULL;
				/* initialise widgets */
#if defined (WX_USER_INTERFACE)
				element_point_viewer->wx_element_point_viewer = (wxElementPointViewer *)NULL;
				element_point_viewer->element_point_grid_field = NULL;
#endif /* defined (WX_USER_INTERFACE) */
				/* initialise the structure */
				element_point_ranges=FIRST_OBJECT_IN_LIST_THAT(Element_point_ranges)
					((LIST_CONDITIONAL_FUNCTION(Element_point_ranges) *)NULL,(void *)NULL,
						Element_point_ranges_selection_get_element_point_ranges_list(
							element_point_ranges_selection));
				if (element_point_ranges)
				{
					 Element_point_ranges_get_identifier(element_point_ranges,
							&(element_point_viewer->element_point_identifier));
					 if ((ranges=Element_point_ranges_get_ranges(element_point_ranges))&&
							Multi_range_get_range(ranges,0,&start,&stop))
					 {
							element_point_viewer->element_point_number=start;
					 }
				}
				else
				{
					element_point_viewer->element_point_identifier.element =
						FE_region_get_first_FE_element_that(cmzn_region_get_FE_region(region),
							FE_element_is_top_level, (void *)NULL);
					 if (element_point_viewer->element_point_identifier.element)
					 {
							element_point_viewer->element_point_identifier.top_level_element=
								 element_point_viewer->element_point_identifier.element;
					 }
					 else
					 {
							element_point_viewer->element_point_identifier.top_level_element=
								 (struct FE_element *)NULL;
					 }
					 Element_point_viewer_get_grid(element_point_viewer);
					 Element_point_viewer_select_current_point(element_point_viewer);
				}
				Element_point_viewer_calculate_xi(element_point_viewer);
				/* get callbacks from global element_point selection */
				Element_point_ranges_selection_add_callback(
					 element_point_ranges_selection,
					 Element_point_viewer_element_point_ranges_selection_change,
					 (void *)element_point_viewer);
				/* new code */
				COPY(Element_point_ranges_identifier)(
					 &temp_element_point_identifier,
					 &(element_point_viewer->element_point_identifier));
				temp_element_point_number=
							element_point_viewer->element_point_number;
				if (temp_element_point_identifier.element)
				{
					 Element_point_make_top_level(&temp_element_point_identifier,
							&temp_element_point_number);
				}
				/* pass identifier with copy_element to viewer widget */
				temp_element_point_identifier.top_level_element = temp_element_point_identifier.element = 0;
				/* make the dialog shell */
#if defined (WX_USER_INTERFACE)
				element_point_viewer->frame_width = 0;
				element_point_viewer->frame_height = 0;
				element_point_viewer->init_width = 0;
				element_point_viewer->init_height=0;
				wxLogNull logNo;
				element_point_viewer->wx_element_point_viewer = new wxElementPointViewer(element_point_viewer);
				Element_point_viewer_refresh_grid_value_text(element_point_viewer);
				element_point_viewer->collpane =
					 XRCCTRL(*element_point_viewer->wx_element_point_viewer,"ElementVariableViewerPanel", wxScrolledWindow);
				element_point_viewer->element_point_win=NULL;
				element_point_viewer->paneSz = NULL;
				wxBoxSizer *Collpane_sizer = new wxBoxSizer( wxVERTICAL );
				element_point_viewer->collpane->SetSizer(Collpane_sizer);
				FOR_EACH_OBJECT_IN_MANAGER(Computed_field)(
					 element_point_viewer_add_collpane,
					 (void *)element_point_viewer,
					 computed_field_manager);
				element_point_viewer->frame=
					 XRCCTRL(*element_point_viewer->wx_element_point_viewer,
							"CmguiElementPointViewer", wxFrame);
				element_point_viewer->frame->Layout();
				element_point_viewer->frame->SetMinSize(wxSize(50,200));
				element_point_viewer->collpane->Layout();
#endif /* defined (WX_USER_INTERFACE) */
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"CREATE(Element_point_viewer).  Could not open hierarchy");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"CREATE(Element_point_viewer).  Invalid argument(s)");
	}
	if (element_point_viewer_address)
	{
		*element_point_viewer_address=element_point_viewer;
	}
	LEAVE;

	return (element_point_viewer);
} /* CREATE(Element_point_viewer) */

int DESTROY(Element_point_viewer)(
	struct Element_point_viewer **element_point_viewer_address)
/*******************************************************************************
LAST MODIFIED : 15 June 2000

DESCRIPTION:
Destroys the Element_point_viewer. See also Element_point_viewer_close_CB.
==============================================================================*/
{
	int return_code;
	struct Element_point_viewer *element_point_viewer;

	ENTER(DESTROY(element_point_viewer));
	if (element_point_viewer_address&&
		(element_point_viewer= *element_point_viewer_address))
	{
		DESTROY(LIST(Field_value_index_ranges))(
			&(element_point_viewer->modified_field_components));
		cmzn_fieldmodulenotifier_destroy(&element_point_viewer->fieldmodulenotifier);
		DEACCESS(Time_object)(&(element_point_viewer->time_object));
		/* end callbacks from global element_point selection */
		Element_point_ranges_selection_remove_callback(
			element_point_viewer->element_point_ranges_selection,
			Element_point_viewer_element_point_ranges_selection_change,
			(void *)element_point_viewer);
		// Use Destroy method not C++ delete for safety:
		element_point_viewer->wx_element_point_viewer->Destroy();
		DEALLOCATE(*element_point_viewer_address);
		element_point_viewer_address = 0;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(Element_point_viewer).  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Element_point_viewer) */

static int Element_point_viewer_refresh_elements(
	struct Element_point_viewer *element_point_viewer)
/*******************************************************************************
LAST MODIFIED :7 June 2007

DESCRIPTION :
Updates the element shown in the chooser to match that for the current point.
==============================================================================*/
{
	int return_code;

	ENTER(Element_point_viewer_refresh_elements);
	if (element_point_viewer->wx_element_point_viewer)
	{
		return_code=1;
		element_point_viewer->wx_element_point_viewer->set_element_selection(element_point_viewer);
		element_point_viewer->wx_element_point_viewer->set_top_level_element_selection(element_point_viewer);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_point_viewer_refresh_elements.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Element_point_viewer_refresh_elements */

static int Element_point_viewer_refresh_chooser(
	struct Element_point_viewer *element_point_viewer)
/*******************************************************************************
LAST MODIFIED : 7 June 2007

DESCRIPTION :
Fills the widgets for choosing the element point with the current values.
==============================================================================*/
{
	int return_code;

	ENTER(Element_point_viewer_refresh_chooser);
	if (element_point_viewer)
	{
		return_code=1;
		Element_point_viewer_refresh_elements(element_point_viewer);
		Element_point_viewer_refresh_sample_mode(element_point_viewer);
		Element_point_viewer_refresh_discretization_text(element_point_viewer);
		Element_point_viewer_refresh_point_number_text(element_point_viewer);
		Element_point_viewer_refresh_xi_text(element_point_viewer);
		Element_point_viewer_refresh_grid_value_text(element_point_viewer);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_point_viewer_refresh_chooser_widgets.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Element_point_viewer_refresh_chooser_widgets */

int Element_point_viewer_bring_window_to_front(struct Element_point_viewer *element_point_viewer)
/*******************************************************************************
LAST MODIFIED : 11 May 2000

DESCRIPTION :
Pops the window for <element_poin_viewer> to the front of those visible.
??? De-iconify as well?
==============================================================================*/
{
	int return_code;

	ENTER(Element_point_viewer_bring_window_to_front);
	if (element_point_viewer->wx_element_point_viewer)
	{
		 element_point_viewer->wx_element_point_viewer->Raise();
		 return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_point_viewer_bring_window_to_front.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Element_point_viewer_bring_window_to_front */

int element_point_viewer_set_element_point_field(
  void *element_point_viewer_void,
	struct Element_point_ranges_identifier *element_point_identifier,
	int element_point_number,struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 6 Jube 2007

DESCRIPTION :
Sets the element_point/field being edited in the
<element_point_field_viewer_widget>.
Note that the viewer works on the element itself, not a local copy. Hence, only
pass unmanaged elements in the element_point_identifier to this widget.
==============================================================================*/
{
	 int return_code, setup_components;
	struct Element_point_viewer *element_point_viewer;

	ENTER(element_point_viewer_set_element_point_field);
	if ((element_point_viewer = static_cast<Element_point_viewer*>(element_point_viewer_void))&&
		(((struct FE_element *)NULL==element_point_identifier->element)||
			(Element_point_ranges_identifier_element_point_number_is_valid(
				element_point_identifier,element_point_number)&&field&&
				Computed_field_is_defined_in_element(field,
					element_point_identifier->element))))
	{
		 /* rebuild viewer widgets if nature of element_point or field changed */
// 		 old_editable=element_point_field_is_editable(
// 				&(element_point_viewer->element_point_identifier),
// 				element_point_viewer->current_field,number_in_xi);
// 		 new_editable=element_point_field_is_editable(
// 				element_point_identifier,field,number_in_xi);
// 		 if ((!element_point_identifier->element)||
// 				(!(element_point_viewer->element_point_identifier.element))||
// 				(field != element_point_viewer->current_field)||
// 				(field&&((element_point_viewer->number_of_components !=
// 							cmzn_field_get_number_of_components(field))||
// 					 (new_editable != old_editable))))
// 		 {
				setup_components=1;
// 		 }
// 		 else
// 		 {
// 				setup_components=0;
// 		 }
		 if (element_point_identifier->element&&field)
		 {
				COPY(Element_point_ranges_identifier)(
					 &(element_point_viewer->element_point_identifier),
					 element_point_identifier);
				element_point_viewer->element_point_number=element_point_number;
				element_point_viewer->current_field=field;
				FE_element_get_numbered_xi_point(
					 element_point_identifier->element,
					 element_point_identifier->sampling_mode,
					 element_point_identifier->number_in_xi,
					 element_point_identifier->exact_xi,
					 (cmzn_fieldcache_id)0,
					 /*coordinate_field*/(struct Computed_field *)NULL,
					 /*density_field*/(struct Computed_field *)NULL,
					 element_point_number, element_point_viewer->xi);
		 }
		 else
		 {
				element_point_viewer->element_point_identifier.element=
					 (struct FE_element *)NULL;
				element_point_viewer->current_field=(struct Computed_field *)NULL;
		 }
		 if (setup_components)
		 {
				element_point_viewer_setup_components(
					 element_point_viewer);
		 }
		 if (element_point_identifier->element&&field)
		 {
				if (Computed_field_has_multiple_times(field))
				{
					 if (!element_point_viewer->time_object_callback)
					 {
							if (CMZN_OK == cmzn_timenotifier_set_callback(element_point_viewer->time_object,
										element_point_viewer_time_change_callback,
										(void *)element_point_viewer))
							{
								element_point_viewer->time_object_callback = 1;
							}
					 }
				}
				else
				{
					 if (element_point_viewer->time_object_callback)
					 {
							cmzn_timenotifier_clear_callback(element_point_viewer->time_object);
							element_point_viewer->time_object_callback = 0;
					 }
				}
// 				element_point_field_viewer_widget_update_values(
// 					element_point_field_viewer);
// 				XtManageChild(element_point_field_viewer->widget);
		 }
// 		 else
// 		 {
// 				 				XtUnmanageChild(element_point_field_viewer->widget);
//
// 				 			return_code = 1;
// 		 }
// 		 else
// 		 {
// 				display_message(ERROR_MESSAGE,
// 					 "element_point_field_viewer_widget_set_element_point_field.  "
// 					 "Missing widget data");
// 				return_code = 0;
// 		 }
		 return_code = 1;
	}
	else
	{
		 display_message(ERROR_MESSAGE,
				"element_point_viewer_set_element_point_field.  "
				"Invalid argument(s)");
		 return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* element_point_field_viewer_widget_set_element_point_field */

static int element_point_viewer_setup_components(
	struct Element_point_viewer *element_point_viewer)
/*******************************************************************************
LAST MODIFIED : 10 October 2003

DESCRIPTION :
Creates the array of cells containing field component names and values.
==============================================================================*/
{
	 char *component_name, *new_string;
	 int return_code, number_of_components, editable;
	struct Computed_field *field;
	struct FE_element *element;
	int number_in_xi[MAXIMUM_ELEMENT_XI_DIMENSIONS], tmp, comp_no;
	wxString tmp_string;

	ENTER(element_point_field_viewer_setup_components);
	if (element_point_viewer)
	{
		 return_code=1;
		 element_point_viewer->number_of_components=-1;
		 tmp = 0;
		 if ((element=element_point_viewer->element_point_identifier.element)&&
				(field=element_point_viewer->current_field)&&
				Computed_field_is_defined_in_element(field,element))
		 {
				editable = element_point_field_is_editable(
					 &(element_point_viewer->element_point_identifier),
					 field,number_in_xi);
				number_of_components=cmzn_field_get_number_of_components(field);
				element_point_viewer->number_of_components=number_of_components;
				for (comp_no=0;(comp_no<number_of_components)&&return_code;comp_no++)
				{
					/* component name label */
					component_name = cmzn_field_get_component_name(field, comp_no + 1);
					if (component_name)
					{
						new_string = component_name;
						tmp_string = wxString::FromAscii(new_string);
						if (tmp == 0)
						{
							tmp = 1;
							element_point_viewer->element_point_grid_field = new wxGridSizer(2,2,1,1);
						}
						element_point_viewer->element_point_grid_field->Add(new wxStaticText(
							element_point_viewer->element_point_win, -1, tmp_string,
							/* the default position is not nice in the optimised
							   version, so manually setting up the position*/
							wxPoint(0,comp_no*35 + 5),wxDefaultSize),1,
							wxALIGN_CENTER_VERTICAL|
                            wxALIGN_CENTER_HORIZONTAL|wxFIXED_MINSIZE, 0);
						element_point_viewer_add_textctrl(editable,element_point_viewer, field, comp_no);
						DEALLOCATE(component_name);
					}
				}
		 }
		 else
		 {
				display_message(ERROR_MESSAGE,
					 "element_point_viewer_widget_setup_components.  "
					 "Invalid argument(s)");
				return_code=0;
		 }
		 LEAVE;
	}

	return (return_code);
} /* element_point_field_viewer_widget_setup_components */

int element_point_viewer_widget_set_element_point(
	Element_point_viewer *element_point_viewer,
	struct Element_point_ranges_identifier *element_point_identifier,
	int element_point_number)
/*******************************************************************************
LAST MODIFIED : 6 June 2007

DESCRIPTION :
Sets the element point being edited in the <element_point_viewer_widget>. Note
that the viewer works on the element itself, not a local copy. Hence, only pass
unmanaged elements in the identifier to this widget.
==============================================================================*/
{
	int return_code;

	ENTER(element_point_viewer_widget_set_element_point);
	if (element_point_viewer && element_point_identifier&&
		(((struct FE_element *)NULL==element_point_identifier->element)||
				Element_point_ranges_identifier_element_point_number_is_valid(
					element_point_identifier,element_point_number)))
	{
		COPY(Element_point_ranges_identifier)(
			&(element_point_viewer->element_point_identifier),
			element_point_identifier);
		element_point_viewer->element_point_number=element_point_number;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
				"element_point_viewer_widget_set_element_point.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* element_point_viewer_widget_set_element_point */

static int Element_point_viewer_refresh_grid_value_text(
	 struct Element_point_viewer *element_point_viewer)
/*******************************************************************************
LAST MODIFIED : 13 June 2007

DESCRIPTION :
Updates the grid_value text field. If there is a current element point, writes
the field value, otherwise N/A.
==============================================================================*/
{
	char *field_value_string;
	const char *value_string;
	int is_sensitive,return_code;
	struct Computed_field *grid_field;
	struct FE_element *element,*top_level_element;

	ENTER(Element_point_viewer_refresh_grid_value_text);
	if (element_point_viewer)
	{
		return_code=1;
		/* Get the text string */
		wxString grid_value_string = element_point_viewer->gridvaluetextctrl->GetValue();
		value_string = grid_value_string.mb_str(wxConvUTF8);
		if (value_string)
		{
			element = element_point_viewer->element_point_identifier.element;
			top_level_element = element_point_viewer->element_point_identifier.top_level_element;
			grid_field = element_point_viewer->wx_element_point_viewer->get_grid_field();
			if (element && top_level_element && grid_field &&
				Computed_field_is_defined_in_element(grid_field,element))
			{
				field_value_string = element_point_viewer_get_field_string(element_point_viewer, grid_field, /*component_number*/-1);
				if (field_value_string)
				{
					/* only set string from field if different from that shown */
					if (strcmp(field_value_string,value_string))
					{
						element_point_viewer->gridvaluetextctrl->SetValue(wxString::FromAscii(field_value_string));
					}
					DEALLOCATE(field_value_string);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Element_point_viewer_refresh_grid_value_text.  "
						"Could not evaluate field");
					element_point_viewer->gridvaluetextctrl->SetValue(wxString::FromAscii("ERROR"));
				}
				is_sensitive=1;
			}
			else
			{
				/* only set string if different from that shown */
				if (strcmp(value_string,"N/A"))
				{
					 element_point_viewer->gridvaluetextctrl->SetValue(wxString::FromAscii("N/A"));
				}
				is_sensitive=0;
			}
			element_point_viewer->gridvaluetextctrl->Enable(is_sensitive);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_point_viewer_refresh_grid_value_text.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Element_point_viewer_refresh_grid_value_text */


static int Element_point_viewer_refresh_sample_mode(
	 struct Element_point_viewer *element_point_viewer)
/*******************************************************************************
LAST MODIFIED : 19 March 2001

DESCRIPTION :
Updates the cmzn_element_point_sampling_mode shown in the chooser to match that for the
current point.
==============================================================================*/
{
	int is_sensitive,return_code;

	ENTER(Element_point_viewer_refresh_sample_mode);
	if (element_point_viewer)
	{
		return_code=1;
		if (element_point_viewer->element_point_identifier.element)
		{
			 element_point_viewer->wx_element_point_viewer->
					set_element_sample_mode_chooser_value(element_point_viewer);
			is_sensitive=1;
		}
		else
		{
			is_sensitive=0;
		}
		element_point_viewer->element_sample_mode_chooser_panel->Enable(is_sensitive);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_point_viewer_refresh_sample_mode.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Element_point_viewer_refresh_sample_mode */
