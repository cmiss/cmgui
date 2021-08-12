/*******************************************************************************
FILE : element_tool.c

LAST MODIFIED : 20 March 2003

DESCRIPTION :
Interactive tool for selecting elements with mouse and other devices.
==============================================================================*/
/* OpenCMISS-Cmgui Application
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#if 1
#include "configure/cmgui_configure.h"
#endif /* defined (1) */
#include "opencmiss/zinc/fieldcache.h"
#include "opencmiss/zinc/glyph.h"
#include "opencmiss/zinc/graphics.h"
#include "opencmiss/zinc/material.h"
#include "opencmiss/zinc/mesh.h"
#include "opencmiss/zinc/scene.h"
#include "opencmiss/zinc/sceneviewer.h"
#include "opencmiss/zinc/scenefilter.h"
#include "command/command.h"
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_finite_element.h"
#include "computed_field/computed_field_group.hpp"
#include "opencmiss/zinc/fieldsubobjectgroup.h"
#include "element/element_operations.h"
#include "element/element_tool.h"
#include "finite_element/finite_element_discretization.h"
#include "finite_element/finite_element_region.h"
#include "general/debug.h"
#include "graphics/glyph.hpp"
#include "graphics/scene.hpp"
#include "interaction/interaction_graphics.h"
#include "interaction/interaction_volume.h"
#include "interaction/interactive_event.h"
#include "graphics/scene.hpp"
#include "graphics/scene_app.h"
#include "graphics/scene_viewer.h"
#include "graphics/graphics.h"
#include "graphics/scene_picker.hpp"
#include "region/cmiss_region.hpp"
#include "time/time_keeper_app.hpp"
#include "general/message.h"
#if defined (WX_USER_INTERFACE)
#include "wx/wx.h"
#include <wx/tglbtn.h>
#include "wx/xrc/xmlres.h"
#include "element/element_tool.xrch"
#include "graphics/graphics_window_private.hpp"
#include "choose/choose_manager_class.hpp"
#endif /* defined (WX_USER_INTERFACE)*/
#include <map>
typedef std::multimap<cmzn_region *, cmzn_element_id> Region_element_map;

/*
Module variables
----------------
*/

static char Interactive_tool_element_type_string[] = "element_tool";


/*
Module types
------------
*/
#if defined (WX_USER_INTERFACE)
class wxElementTool;
#endif /* defined (WX_USER_INTERFACE) */

struct Element_tool
	/*******************************************************************************
	LAST MODIFIED : 20 March 2003

	DESCRIPTION :
	Object storing all the parameters for interactively selecting elements.
	==============================================================================*/
{
	struct Execute_command *execute_command;
	struct MANAGER(Interactive_tool) *interactive_tool_manager;
	struct Interactive_tool *interactive_tool;
	/* needed for destroy button */
	struct cmzn_region *region;
	struct Element_point_ranges_selection *element_point_ranges_selection;
	cmzn_material *rubber_band_material;
	cmzn_glyph *rubber_band_glyph;
	cmzn_graphics *rubber_band_graphics;
	struct Time_keeper_app *time_keeper_app;
	struct User_interface *user_interface;
	/* user-settable flags */
	bool select_elements_enabled, select_faces_enabled, select_lines_enabled;
	struct Computed_field *command_field;
	/* information about picked element */
	bool pickedElementWasSelected;
	int motion_detected;
	cmzn_element *lastPickedElement;
	struct Interaction_volume *last_interaction_volume;
	struct GT_object *rubber_band;

#if defined (WX_USER_INTERFACE)
	wxElementTool *wx_element_tool;
	wxPoint tool_position;
#endif /* defined (WX_USER_INTERFACE) */

	void actionCommandAtElement(cmzn_element *pickedElement);

	cmzn_scenepicker_id createScenepicker(cmzn_scene_id scene,
		cmzn_sceneviewer_id sceneviewer, cmzn_scenefiltermodule_id scenefiltermodule) const;

}; /* struct Element_tool */

/*
Module functions
----------------
*/

struct DestroySelectedElementsData
{
	int dimension; // dimension of elements to destroy;
	int numberDestroyed;
	DestroySelectedElementsData(int dimensionIn) :
		dimension(dimensionIn),
		numberDestroyed(0)
	{
	}
};

static int cmzn_field_group_destroy_all_elements(cmzn_field_group_id group, void *dataVoid)
{
	int return_code = 0;
	DestroySelectedElementsData *data = static_cast<DestroySelectedElementsData*>(dataVoid);
	if (group && data)
	{
		cmzn_fieldmodule_id field_module = cmzn_field_get_fieldmodule(cmzn_field_group_base_cast(group));
		cmzn_mesh_id master_mesh = cmzn_fieldmodule_find_mesh_by_dimension(field_module, data->dimension);
		cmzn_field_element_group_id element_group = cmzn_field_group_get_field_element_group(group, master_mesh);
		cmzn_mesh_destroy(&master_mesh);
		if (element_group)
		{
			cmzn_mesh_group_id mesh_group = cmzn_field_element_group_get_mesh_group(element_group);
			data->numberDestroyed += cmzn_mesh_get_size(cmzn_mesh_group_base_cast(mesh_group));
			cmzn_mesh_destroy_all_elements(cmzn_mesh_group_base_cast(mesh_group));
			cmzn_mesh_group_destroy(&mesh_group);
			cmzn_field_element_group_destroy(&element_group);
		}
		cmzn_fieldmodule_destroy(&field_module);
		return_code = 1;
	}
	return return_code;
}

int Element_tool_destroy_selected_elements(struct Element_tool *element_tool)
{
	int return_code = 0;
	if (element_tool->region)
	{
		return_code = 1;
		cmzn_scene *root_scene = cmzn_region_get_scene(element_tool->region);
		cmzn_field_id selection_field = cmzn_scene_get_selection_field(root_scene);
		cmzn_field_group_id selection_group = cmzn_field_cast_group(selection_field);
		cmzn_field_destroy(&selection_field);
		if (selection_group)
		{
			for (int dimension = MAXIMUM_ELEMENT_XI_DIMENSIONS; 0 < dimension; --dimension)
			{
				DestroySelectedElementsData data(dimension);
				return_code = cmzn_field_group_for_each_group_hierarchical(selection_group,
					cmzn_field_group_destroy_all_elements, &data);
				if (data.numberDestroyed > 0)
					break;
			}
			cmzn_scene_flush_tree_selections(root_scene);
			cmzn_field_group_destroy(&selection_group);
		}
		cmzn_scene_destroy(&root_scene);
	}
	return return_code;
}

#if defined (OPENGL_API)
static void Element_tool_reset(void *element_tool_void)
/*******************************************************************************
LAST MODIFIED : 25 February 2008

DESCRIPTION :
Resets current edit. Called on button release or when tool deactivated.
==============================================================================*/
{
	struct Element_tool *element_tool;

	ENTER(Element_tool_reset);
	element_tool = (struct Element_tool *)element_tool_void;
	if (element_tool != 0)
	{
		cmzn_element_destroy(&(element_tool->lastPickedElement));
		if (element_tool->last_interaction_volume)
			DEACCESS(Interaction_volume)(&(element_tool->last_interaction_volume));
	}
	else
	{
		display_message(ERROR_MESSAGE,"Element_tool_reset.  Invalid argument(s)");
	}
	LEAVE;
} /* Element_tool_reset */
#endif /* defined (OPENGL_API) */

void Element_tool::actionCommandAtElement(cmzn_element *pickedElement)
{
	if (this->command_field && pickedElement)
	{
		FE_value time = 0;
		if (this->time_keeper_app)
			time = this->time_keeper_app->getTimeKeeper()->getTime();
		FE_value xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
		/* since we don't really have fields constant over an element, evaluate at its centre */
		int element_dimension = cmzn_element_get_dimension(pickedElement);
		int number_in_xi[MAXIMUM_ELEMENT_XI_DIMENSIONS] = { 1, 1, 1 };
		struct FE_element_shape *element_shape = get_FE_element_shape(pickedElement);
		FE_value_triple *xi_points;
		int number_of_xi_points;
		if (FE_element_shape_get_xi_points_cell_centres(
			element_shape, number_in_xi, &number_of_xi_points, &xi_points))
		{
			for (int i = 0; i < element_dimension; i++)
				xi[i] = xi_points[0][i];
			cmzn_fieldmodule_id field_module = cmzn_field_get_fieldmodule(this->command_field);
			cmzn_fieldcache_id field_cache = cmzn_fieldmodule_create_fieldcache(field_module);
			cmzn_fieldcache_set_time(field_cache, time);
			cmzn_fieldcache_set_mesh_location(field_cache, pickedElement, element_dimension, xi);
			char *command_string = cmzn_field_evaluate_string(this->command_field, field_cache);
			if (command_string)
			{
				Execute_command_execute_string(this->execute_command, command_string);
				DEALLOCATE(command_string);
			}
			cmzn_fieldcache_destroy(&field_cache);
			cmzn_fieldmodule_destroy(&field_module);
			DEALLOCATE(xi_points);
		}
	}
}

cmzn_scenepicker_id Element_tool::createScenepicker(cmzn_scene_id scene,
	cmzn_sceneviewer_id sceneviewer, cmzn_scenefiltermodule_id scenefiltermodule) const
{
	if (!(scene && sceneviewer && scenefiltermodule))
		return 0;
	cmzn_scenepicker_id scenepicker = cmzn_scene_create_scenepicker(scene);
	cmzn_scenefilter_id combined_filter =
		cmzn_scenefiltermodule_create_scenefilter_operator_and(scenefiltermodule);
	cmzn_scenefilter_id or_filter_base =
		cmzn_scenefiltermodule_create_scenefilter_operator_or(scenefiltermodule);
	if (this->select_elements_enabled || this->select_faces_enabled || this->select_lines_enabled)
	{
		cmzn_scenefilter_id element_filter = 0;
		cmzn_scenefilter_operator_id or_filter = cmzn_scenefilter_cast_operator(
			or_filter_base);
		if (this->select_lines_enabled)
		{
			element_filter = cmzn_scenefiltermodule_create_scenefilter_field_domain_type(
				scenefiltermodule, CMZN_FIELD_DOMAIN_TYPE_MESH1D);
			cmzn_scenefilter_operator_append_operand(or_filter, element_filter);
			cmzn_scenefilter_destroy(&element_filter);
		}
		if (this->select_faces_enabled)
		{
			element_filter = cmzn_scenefiltermodule_create_scenefilter_field_domain_type(
				scenefiltermodule, CMZN_FIELD_DOMAIN_TYPE_MESH2D);
			cmzn_scenefilter_operator_append_operand(or_filter, element_filter);
			cmzn_scenefilter_destroy(&element_filter);
		}
		if (this->select_elements_enabled)
		{
			element_filter = cmzn_scenefiltermodule_create_scenefilter_field_domain_type(
				scenefiltermodule, CMZN_FIELD_DOMAIN_TYPE_MESH_HIGHEST_DIMENSION);
			cmzn_scenefilter_operator_append_operand(or_filter, element_filter);
			cmzn_scenefilter_destroy(&element_filter);
			element_filter = cmzn_scenefiltermodule_create_scenefilter_field_domain_type(
				scenefiltermodule, CMZN_FIELD_DOMAIN_TYPE_MESH3D);
			cmzn_scenefilter_operator_append_operand(or_filter, element_filter);
			cmzn_scenefilter_destroy(&element_filter);
		}
		cmzn_scenefilter_operator_destroy(&or_filter);
	}
	else
		cmzn_scenefilter_set_inverse(combined_filter, true);
	cmzn_scenefilter_operator_id and_filter = cmzn_scenefilter_cast_operator(combined_filter);
	cmzn_scenefilter_id sceneviewerFilter = cmzn_sceneviewer_get_scenefilter(sceneviewer);
	cmzn_scenefilter_operator_append_operand(and_filter, sceneviewerFilter);
	cmzn_scenefilter_operator_append_operand(and_filter, or_filter_base);
	// Possible optimisation: don't pick streamlines
	cmzn_scenepicker_set_scenefilter(scenepicker, combined_filter);
	cmzn_scenefilter_destroy(&sceneviewerFilter);
	cmzn_scenefilter_operator_destroy(&and_filter);
	cmzn_scenefilter_destroy(&or_filter_base);
	cmzn_scenefilter_destroy(&combined_filter);
	return scenepicker;
}

#if defined (OPENGL_API)
/**
 * Input handler for input from devices. <device_id> is a unique address enabling
 * the editor to handle input from more than one device at a time. The <event>
 * describes the type of event, button numbers and key modifiers, and the volume
 * of space affected by the interaction. Main events are button press, movement and
 * release.
 */
static void Element_tool_interactive_event_handler(void *device_id,
	struct Interactive_event *event,void *element_tool_void,
	struct cmzn_sceneviewer *scene_viewer)
{
	Element_tool *element_tool = static_cast<Element_tool *>(element_tool_void);
	if (device_id && event && element_tool && scene_viewer)
	{
		cmzn_region_begin_hierarchical_change(element_tool->region);
		cmzn_scene_id rootScene = cmzn_region_get_scene(element_tool->region);
		cmzn_field_id selectionField = cmzn_scene_get_selection_field(rootScene);
		cmzn_field_group_id rootSelectionGroup = cmzn_field_cast_group(selectionField);
		cmzn_field_destroy(&selectionField);
		// cache scenefilter changes to avoid notifying about temporary filters
		cmzn_scenefiltermodule_id scenefiltermodule = cmzn_scene_get_scenefiltermodule(rootScene);
		cmzn_scenefiltermodule_begin_change(scenefiltermodule);

		Interaction_volume *interaction_volume = Interactive_event_get_interaction_volume(event);
		cmzn_scene_id eventScene = Interactive_event_get_scene(event);
		if (eventScene != 0)
		{
			Interactive_event_type event_type = Interactive_event_get_type(event);
			int input_modifier = Interactive_event_get_input_modifier(event);
			bool incrementalEdit = (0 != (INTERACTIVE_EVENT_MODIFIER_SHIFT & input_modifier));
			cmzn_scenepicker_id scenepicker = 0;
			switch (event_type)
			{
				case INTERACTIVE_EVENT_BUTTON_PRESS:
				{
					if (Interactive_event_get_button_number(event) == 1)
					{
						REACCESS(Interaction_volume)(&(element_tool->last_interaction_volume), interaction_volume);
						element_tool->pickedElementWasSelected = false;
						cmzn_element_destroy(&(element_tool->lastPickedElement));
						scenepicker = element_tool->createScenepicker(eventScene, scene_viewer, scenefiltermodule);
						cmzn_scenepicker_set_interaction_volume(scenepicker, interaction_volume);
						element_tool->lastPickedElement = cmzn_scenepicker_get_nearest_element(scenepicker);
						if (element_tool->lastPickedElement)
						{
							if (!rootSelectionGroup)
								rootSelectionGroup = cmzn_scene_get_or_create_selection_group(rootScene);
							if (!incrementalEdit)
								cmzn_field_group_clear(rootSelectionGroup);
							cmzn_mesh_id masterMesh = cmzn_element_get_mesh(element_tool->lastPickedElement);
							cmzn_field_element_group_id elementGroup =
								cmzn_field_group_get_field_element_group(rootSelectionGroup, masterMesh);
							if (!elementGroup)
								elementGroup = cmzn_field_group_create_field_element_group(rootSelectionGroup, masterMesh);
							cmzn_mesh_group_id meshGroup = cmzn_field_element_group_get_mesh_group(elementGroup);
							int status = cmzn_mesh_group_add_element(meshGroup, element_tool->lastPickedElement);
							if (CMZN_ERROR_ALREADY_EXISTS == status)
								element_tool->pickedElementWasSelected = true;
							cmzn_mesh_group_destroy(&meshGroup);
							cmzn_field_element_group_destroy(&elementGroup);
							cmzn_mesh_destroy(&masterMesh);

							if (element_tool->command_field)
								element_tool->actionCommandAtElement(element_tool->lastPickedElement);
						}
						else if (rootSelectionGroup && (!incrementalEdit))
							cmzn_field_group_clear(rootSelectionGroup);
					}
					element_tool->motion_detected = 0;
				} break;
				case INTERACTIVE_EVENT_MOTION_NOTIFY:
				case INTERACTIVE_EVENT_BUTTON_RELEASE:
				{
					if (element_tool->last_interaction_volume&&
						((INTERACTIVE_EVENT_MOTION_NOTIFY==event_type) ||
						(1==Interactive_event_get_button_number(event))))
					{
						if (INTERACTIVE_EVENT_MOTION_NOTIFY==event_type)
						{
							element_tool->motion_detected=1;
						}
						if (element_tool->lastPickedElement)
						{
							/* unselect lastPickedElement if not just added */
							if ((INTERACTIVE_EVENT_BUTTON_RELEASE == event_type) &&
								incrementalEdit && element_tool->pickedElementWasSelected)
							{
								cmzn_mesh_id masterMesh = cmzn_element_get_mesh(element_tool->lastPickedElement);
								cmzn_field_element_group_id elementGroup =
									cmzn_field_group_get_field_element_group(rootSelectionGroup, masterMesh);
								cmzn_mesh_group_id meshGroup = cmzn_field_element_group_get_mesh_group(elementGroup);
								cmzn_mesh_group_remove_element(meshGroup, element_tool->lastPickedElement);
								cmzn_mesh_group_destroy(&meshGroup);
								cmzn_field_element_group_destroy(&elementGroup);
								cmzn_mesh_destroy(&masterMesh);
							}
						}
						else if (element_tool->motion_detected)
						{
							/* rubber band select */
							Interaction_volume *temp_interaction_volume =
								create_Interaction_volume_bounding_box(
									element_tool->last_interaction_volume, interaction_volume);
							if (temp_interaction_volume != 0)
							{
								if (INTERACTIVE_EVENT_MOTION_NOTIFY==event_type)
								{
									cmzn_scene_begin_change(eventScene);
									if (!element_tool->rubber_band)
									{
										/* create rubber_band object and put in scene */
										element_tool->rubber_band=CREATE(GT_object)(
											"element_tool_rubber_band",g_POLYLINE_VERTEX_BUFFERS,
											element_tool->rubber_band_material);
										cmzn_glyphmodule_id glyphmodule = cmzn_scene_get_glyphmodule(eventScene);
										element_tool->rubber_band_glyph = cmzn_glyphmodule_create_glyph_static(
											glyphmodule, element_tool->rubber_band);
										cmzn_glyph_set_name(element_tool->rubber_band_glyph, "temp_rubber_band");
										element_tool->rubber_band_graphics = cmzn_scene_create_graphics_points(
											eventScene);
										cmzn_graphics_set_scenecoordinatesystem(element_tool->rubber_band_graphics,
											CMZN_SCENECOORDINATESYSTEM_WORLD);
										cmzn_graphicspointattributes_id point_attributes = cmzn_graphics_get_graphicspointattributes(
											element_tool->rubber_band_graphics);
										cmzn_graphicspointattributes_set_glyph(point_attributes,
											element_tool->rubber_band_glyph);
										double base_size = 1.0;
										cmzn_graphicspointattributes_set_base_size(
											point_attributes, 1, &base_size);
										cmzn_graphicspointattributes_destroy(&point_attributes);
										cmzn_glyphmodule_destroy(&glyphmodule);
									}
									Interaction_volume_make_polyline_extents(
										temp_interaction_volume,element_tool->rubber_band);
									cmzn_graphics_flag_glyph_has_changed(element_tool->rubber_band_graphics);
									cmzn_scene_end_change(eventScene);
								}
								else
								{
									cmzn_scene_remove_graphics(eventScene, element_tool->rubber_band_graphics);
									cmzn_graphics_destroy(&element_tool->rubber_band_graphics);
									cmzn_glyph_destroy(&element_tool->rubber_band_glyph);
									DEACCESS(GT_object)(&(element_tool->rubber_band));
								}

								// remove the following line for live graphics update on picking
								if (INTERACTIVE_EVENT_BUTTON_RELEASE==event_type)
								{
									scenepicker = element_tool->createScenepicker(eventScene, scene_viewer, scenefiltermodule);
									cmzn_scenepicker_set_interaction_volume(scenepicker, temp_interaction_volume);
									if (!rootSelectionGroup)
										rootSelectionGroup = cmzn_scene_get_or_create_selection_group(rootScene);
									cmzn_scenepicker_add_picked_elements_to_field_group(scenepicker, rootSelectionGroup);
								}
								DEACCESS(Interaction_volume)(&temp_interaction_volume);
							}
						}
						if (INTERACTIVE_EVENT_BUTTON_RELEASE==event_type)
							Element_tool_reset((void *)element_tool);
					}
				} break;
				default:
				{
					display_message(ERROR_MESSAGE,
						"Element_tool_interactive_event_handler.  Unknown event type");
				} break;
			}
			if (scenepicker)
			{
				cmzn_scenepicker_set_scenefilter(scenepicker, static_cast<cmzn_scenefilter_id>(0));
				cmzn_scenepicker_destroy(&scenepicker);
			}
		}
		cmzn_scenefiltermodule_end_change(scenefiltermodule);
		cmzn_scenefiltermodule_destroy(&scenefiltermodule);
		if (rootScene)
			cmzn_scene_flush_tree_selections(rootScene);
		cmzn_field_group_destroy(&rootSelectionGroup);
		cmzn_scene_destroy(&rootScene);
		cmzn_region_end_hierarchical_change(element_tool->region);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_tool_interactive_event_handler.  Invalid argument(s)");
	}
}
#endif /* defined (OPENGL_API) */

#if defined (OPENGL_API)
static int Element_tool_bring_up_interactive_tool_dialog(
	void *element_tool_void,struct Graphics_window *graphics_window)
/*******************************************************************************
LAST MODIFIED : 20 July 2000

DESCRIPTION :
Brings up a dialog for editing settings of the Element_tool - in a standard
format for passing to an Interactive_toolbar.
==============================================================================*/
{
	int return_code;

	ENTER(Element_tool_bring_up_interactive_tool_dialog);
	return_code =
		Element_tool_pop_up_dialog((struct Element_tool *)element_tool_void,graphics_window);
	LEAVE;

	return (return_code);
} /* Element_tool_bring_up_interactive_tool_dialog */
#endif /* defined (OPENGL_API) */

#if defined (OPENGL_API)
static int Element_tool_destroy_element_tool(void **element_tool_void)
/*******************************************************************************
LAST MODIFIED : 6 July 2007

DESCRIPTION :
Function to call DESTROY
==============================================================================*/
{
	ENTER(element_point_tool_destroy_element_point_tool);
	Element_tool *element_tool;
	int return_code;
	return_code=0;

	element_tool = (struct Element_tool *)*element_tool_void;
	if (element_tool != 0)
	{
		return_code = DESTROY(Element_tool)(&element_tool);
	}
	LEAVE;
	return (return_code);
}
#endif

#if defined (WX_USER_INTERFACE)
class wxElementTool : public wxPanel
{
	Element_tool *element_tool;
	wxCheckBox *button_element;
	wxCheckBox *button_face;
	wxCheckBox *button_line;
	wxCheckBox *elementcommandfieldcheckbox;
	wxPanel *element_command_field_chooser_panel;

	wxButton *button_destroy;
	struct MANAGER(Computed_field) *computed_field_manager;
	struct FE_region *fe_region;
	struct Computed_field *command_field;
	DEFINE_MANAGER_CLASS(Computed_field);
	Managed_object_chooser<Computed_field,MANAGER_CLASS(Computed_field)>
		*element_command_field_chooser;

public:

	wxElementTool(Element_tool *element_tool, wxPanel *parent)
		: element_tool(element_tool)
	{
		wxXmlInit_element_tool();
		wxXmlResource::Get()->LoadPanel(this,parent,_T("CmguiElementTool"));
		elementcommandfieldcheckbox = XRCCTRL(*this, "ElementCommandFieldCheckBox",wxCheckBox);
		element_command_field_chooser_panel = XRCCTRL(*this, "ElementCommandFieldChooserPanel", wxPanel);
		if (element_tool->region)
		{
			computed_field_manager=
				cmzn_region_get_Computed_field_manager(element_tool->region);
		}
		else
		{
			computed_field_manager=NULL;
		}
		element_command_field_chooser =
			new Managed_object_chooser<Computed_field,MANAGER_CLASS(Computed_field)>
			(element_command_field_chooser_panel, element_tool->command_field, computed_field_manager,
			Computed_field_has_string_value_type, (void *)NULL, element_tool->user_interface);
		Callback_base< Computed_field* > *element_command_field_callback =
			new Callback_member_callback< Computed_field*,
			wxElementTool, int (wxElementTool::*)(Computed_field *) >
			(this, &wxElementTool::element_command_field_callback);
		element_command_field_chooser->set_callback(element_command_field_callback);
		if (element_tool != NULL)
		{
			command_field = Element_tool_get_command_field(element_tool);
			element_command_field_chooser->set_object(command_field);
			if (command_field == NULL)
				{
					elementcommandfieldcheckbox->SetValue(0);
					element_command_field_chooser_panel->Disable();
				}
			else
				{
					elementcommandfieldcheckbox->SetValue(1);
					element_command_field_chooser_panel->Enable();
				}
		}
		button_face = XRCCTRL(*this, "ButtonFace", wxCheckBox);
		button_element = XRCCTRL(*this, "ButtonElement", wxCheckBox);
		button_line = XRCCTRL(*this, "ButtonLine", wxCheckBox);
		button_element->SetValue(element_tool->select_elements_enabled);
		button_face->SetValue(element_tool->select_faces_enabled);
		button_line->SetValue(element_tool->select_lines_enabled);
	}

	wxElementTool()
	{
	}

	~ wxElementTool()
	{
		if (element_command_field_chooser)
			delete element_command_field_chooser;
	}

	int element_command_field_callback(Computed_field *command_field)
	{
		Element_tool_set_command_field(element_tool, command_field);
		return 1;
	}

	void OnButtonElementpressed(wxCommandEvent& event)
	{
		USE_PARAMETER(event);
		button_element = XRCCTRL(*this, "ButtonElement", wxCheckBox);
		Element_tool_set_select_elements_enabled(element_tool,
			button_element->IsChecked());
	}

	void OnButtonFacepressed(wxCommandEvent& event)
	{
		USE_PARAMETER(event);
		button_face = XRCCTRL(*this, "ButtonFace", wxCheckBox);
		Element_tool_set_select_faces_enabled(element_tool,
			button_face->IsChecked());
	}

	void OnButtonLinepressed(wxCommandEvent& event)
	{
		USE_PARAMETER(event);
		button_line = XRCCTRL(*this, "ButtonLine", wxCheckBox);
		Element_tool_set_select_lines_enabled(element_tool,
			button_line->IsChecked());
	}

	void OnButtonDestroypressed(wxCommandEvent& event)
	{
		USE_PARAMETER(event);
		Element_tool_destroy_selected_elements(element_tool);
	}

	void ElementToolInterfaceRenew(Element_tool *destination_element_tool)
	{
		button_element = XRCCTRL(*this, "ButtonElement", wxCheckBox);
		button_face = XRCCTRL(*this, "ButtonFace", wxCheckBox);
		button_line = XRCCTRL(*this, "ButtonLine", wxCheckBox);
		button_element->SetValue(destination_element_tool->select_elements_enabled);
		button_face->SetValue(destination_element_tool->select_faces_enabled);
		button_line->SetValue(destination_element_tool->select_lines_enabled);
	}

	void ElementCommandFieldChecked(wxCommandEvent &event)
	{
		USE_PARAMETER(event);
		struct Computed_field *command_field;
		elementcommandfieldcheckbox = XRCCTRL(*this, "ElementCommandFieldCheckBox",wxCheckBox);
		element_command_field_chooser_panel = XRCCTRL(*this, "ElementCommandFieldChooserPanel", wxPanel);
		if (elementcommandfieldcheckbox->IsChecked())
		{
			if (element_tool)
			{
				if (Element_tool_get_command_field(element_tool))
				{
					Element_tool_set_command_field(element_tool, (struct Computed_field *)NULL);
					element_command_field_chooser_panel->Enable();
				}
				else
				{
					/* get label field from widget */
					if (element_command_field_chooser->get_number_of_object() > 0)
					{
						command_field = element_command_field_chooser->get_object();
						if (command_field)
							 {
								 Element_tool_set_command_field(element_tool, command_field);
							 }
					}
					else
					{
						elementcommandfieldcheckbox->SetValue(0);
						element_command_field_chooser_panel->Disable();
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Elementt_tool_command_field_button_CB.  Invalid argument(s)");
			}
		}
		else
		{
			Element_tool_set_command_field(element_tool, (struct Computed_field *)NULL);
			elementcommandfieldcheckbox->SetValue(0);
			element_command_field_chooser_panel->Disable();
		}
	}

	DECLARE_DYNAMIC_CLASS(wxElementTool);
	DECLARE_EVENT_TABLE();
};

IMPLEMENT_DYNAMIC_CLASS(wxElementTool, wxPanel)

BEGIN_EVENT_TABLE(wxElementTool, wxPanel)
	EVT_CHECKBOX(XRCID("ButtonElement"),wxElementTool::OnButtonElementpressed)
	EVT_CHECKBOX(XRCID("ButtonFace"),wxElementTool::OnButtonFacepressed)
	EVT_CHECKBOX(XRCID("ButtonLine"),wxElementTool::OnButtonLinepressed)
	EVT_CHECKBOX(XRCID("ElementCommandFieldCheckBox"),wxElementTool::ElementCommandFieldChecked)
	EVT_BUTTON(XRCID("ButtonDestroy"),wxElementTool::OnButtonDestroypressed)
END_EVENT_TABLE()


#endif /* defined (WX_USER_INTERFACE) */

/*
Global functions
----------------
*/
static int Element_tool_copy_function(
	void *destination_tool_void, void *source_tool_void,
	struct MANAGER(Interactive_tool) *destination_tool_manager)
/*******************************************************************************
LAST MODIFIED : 29 March 2007

DESCRIPTION :
Copies the state of one element tool to another.WX only
==============================================================================*/
{
	int return_code;
	struct Element_tool *destination_element_tool, *source_element_tool;
	ENTER(Element_tool_copy_function);
	if ((destination_tool_void || destination_tool_manager) &&
		(source_element_tool=(struct Element_tool *)source_tool_void))
	{
		if (destination_tool_void)
		{
			destination_element_tool = (struct Element_tool *)destination_tool_void;
		}
		else
		{
			destination_element_tool = CREATE(Element_tool)
				(destination_tool_manager,
				source_element_tool->region,
				source_element_tool->element_point_ranges_selection,
				source_element_tool->rubber_band_material,
				source_element_tool->user_interface,
				source_element_tool->time_keeper_app);
			Element_tool_set_execute_command(destination_element_tool,
				source_element_tool->execute_command);
		}
		if (destination_element_tool)
		{
			destination_element_tool->select_elements_enabled = source_element_tool->select_elements_enabled;
			destination_element_tool->select_faces_enabled = source_element_tool->select_faces_enabled;
			destination_element_tool->select_lines_enabled = source_element_tool->select_lines_enabled;
			destination_element_tool->command_field = source_element_tool->command_field;
#if defined (WX_USER_INTERFACE)
			if (destination_element_tool->wx_element_tool != (wxElementTool *) NULL)
			{
				destination_element_tool->wx_element_tool->ElementToolInterfaceRenew(destination_element_tool);
			}
#endif /*defined (WX_USER_INTERFACE)*/
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Element_tool_copy_function.  Could not create copy.");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_tool_copy_function.  Invalid argument(s)");
		return_code=0;
	}
	return (return_code);
}

struct Element_tool *CREATE(Element_tool)(
	struct MANAGER(Interactive_tool) *interactive_tool_manager,
	struct cmzn_region *region,
	struct Element_point_ranges_selection *element_point_ranges_selection,
	cmzn_material *rubber_band_material,
	struct User_interface *user_interface,
	struct Time_keeper_app *time_keeper_app)
/*******************************************************************************
LAST MODIFIED : 20 March 2003

DESCRIPTION :
Creates an Element_tool with Interactive_tool in <interactive_tool_manager>.
Selects elements in <element_selection> in response to interactive_events.
==============================================================================*/
{
	struct Element_tool *element_tool;
	struct MANAGER(Computed_field) *computed_field_manager;

	ENTER(CREATE(Element_tool));
	element_tool=(struct Element_tool *)NULL;
	if (interactive_tool_manager && region &&(NULL != (computed_field_manager=
		cmzn_region_get_Computed_field_manager(region)))
		&&rubber_band_material&&user_interface)
	{
		if (ALLOCATE(element_tool,struct Element_tool,1))
		{
			element_tool->execute_command=NULL;
			element_tool->interactive_tool_manager=interactive_tool_manager;
			element_tool->region = region;
			element_tool->element_point_ranges_selection=
				element_point_ranges_selection;
			element_tool->rubber_band_material=
				cmzn_material_access(rubber_band_material);
			element_tool->user_interface=user_interface;
			element_tool->time_keeper_app = (struct Time_keeper_app *)NULL;
			if (time_keeper_app)
			{
				element_tool->time_keeper_app = ACCESS(Time_keeper_app)(time_keeper_app);
			}
			/* user-settable flags */
			element_tool->select_elements_enabled = true;
			element_tool->select_faces_enabled = true;
			element_tool->select_lines_enabled = true;
			element_tool->command_field = (struct Computed_field *)NULL;
			/* interactive_tool */
#if defined (OPENGL_API)
			element_tool->interactive_tool=CREATE(Interactive_tool)(
				"element_tool","Element tool",
				Interactive_tool_element_type_string,
				Element_tool_interactive_event_handler,
				Element_tool_bring_up_interactive_tool_dialog,
				Element_tool_reset,
				Element_tool_destroy_element_tool,
				Element_tool_copy_function,
				(void *)element_tool);
#else /* defined (OPENGL_API) */
			element_tool->interactive_tool=CREATE(Interactive_tool)(
				"element_tool","Element tool",
				Interactive_tool_element_type_string,
				(Interactive_event_handler*)NULL,
				(Interactive_tool_bring_up_dialog_function*)NULL,
				(Interactive_tool_reset_function*)NULL,
				(Interactive_tool_destroy_tool_data_function *)NULL,
				Element_tool_copy_function,
				(void *)element_tool);
#endif /* defined (OPENGL_API) */
			ADD_OBJECT_TO_MANAGER(Interactive_tool)(
				element_tool->interactive_tool,
				element_tool->interactive_tool_manager);
			element_tool->lastPickedElement=(struct FE_element *)NULL;
			element_tool->last_interaction_volume=(struct Interaction_volume *)NULL;
			element_tool->rubber_band=(struct GT_object *)NULL;
			element_tool->rubber_band_glyph = 0;
			element_tool->rubber_band_graphics = 0;
#if defined (WX_USER_INTERFACE) /* switch (USER_INTERFACE) */
			element_tool->wx_element_tool=(wxElementTool *)NULL;
#endif /* defined (WX_USER_INTERFACE) */
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"CREATE(Element_tool).  Not enough memory");
			DEALLOCATE(element_tool);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"CREATE(Element_tool).  Invalid argument(s)");
	}
	LEAVE;

	return (element_tool);
} /* CREATE(Element_tool) */

int DESTROY(Element_tool)(struct Element_tool **element_tool_address)
/*******************************************************************************
LAST MODIFIED : 5 July 2002

DESCRIPTION :
Frees and deaccesses objects in the <element_tool> and deallocates the
structure itself.
==============================================================================*/
{
	struct Element_tool *element_tool;
	int return_code;

	ENTER(DESTROY(Element_tool));
	if (element_tool_address&&(element_tool= *element_tool_address))
	{
		cmzn_element_destroy(&(element_tool->lastPickedElement));
		REACCESS(Interaction_volume)(&(element_tool->last_interaction_volume),
			(struct Interaction_volume *)NULL);
		cmzn_graphics_destroy(&element_tool->rubber_band_graphics);
		cmzn_glyph_destroy(&element_tool->rubber_band_glyph);
		REACCESS(GT_object)(&(element_tool->rubber_band),(struct GT_object *)NULL);
		cmzn_material_destroy(&(element_tool->rubber_band_material));
		if (element_tool->time_keeper_app)
		{
			DEACCESS(Time_keeper_app)(&(element_tool->time_keeper_app));
		}
#if defined (WX_USER_INTERFACE)
		if (element_tool->wx_element_tool)
			element_tool->wx_element_tool->Destroy();
#endif /*(WX_USER_INTERFACE)*/
		DEALLOCATE(*element_tool_address);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(Element_tool).  Invalid argument(s)");
		return_code=0;
	}

	return (return_code);
} /* DESTROY(Element_tool) */

int Element_tool_pop_up_dialog(struct Element_tool *element_tool, struct Graphics_window *graphics_window)
/*******************************************************************************
LAST MODIFIED : 20 June 2001

DESCRIPTION :
Pops up a dialog for editing settings of the Element_tool.
==============================================================================*/
{
	int return_code;

	ENTER(Element_tool_pop_up_dialog);
	if (element_tool)
	{
#if defined (WX_USER_INTERFACE) /* switch (USER_INTERFACE) */
		wxPanel *pane;
		if (!element_tool->wx_element_tool)
		{
			element_tool->wx_element_tool = new wxElementTool(element_tool,
				Graphics_window_get_interactive_tool_panel(graphics_window));
			pane = XRCCTRL(*element_tool->wx_element_tool, "CmguiElementTool", wxPanel);
			element_tool->tool_position = pane->GetPosition();
			element_tool->wx_element_tool->Show();
		}
		else
		{
			pane = XRCCTRL(*element_tool->wx_element_tool, "CmguiElementTool", wxPanel);
			pane->SetPosition(element_tool->tool_position);
			element_tool->wx_element_tool->Show();
		}
#else /* switch (USER_INTERFACE) */
		USE_PARAMETER(graphics_window);
		display_message(ERROR_MESSAGE, "Element_tool_pop_up_dialog.  "
			"No dialog implemented for this User Interface");
#endif /*  switch (USER_INTERFACE) */
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_tool_pop_up_dialog.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Element_tool_pop_up_dialog */

bool Element_tool_get_select_elements_enabled(struct Element_tool *element_tool)
{
	if (element_tool)
		return element_tool->select_elements_enabled;
	display_message(ERROR_MESSAGE,
		"Element_tool_get_select_elements_enabled.  Invalid argument(s)");
	return false;
}

int Element_tool_set_select_elements_enabled(struct Element_tool *element_tool,
	bool select_elements_enabled)
{
	if (element_tool)
	{
		element_tool->select_elements_enabled = select_elements_enabled;
		return 1;
	}
	display_message(ERROR_MESSAGE,
		"Element_tool_set_select_elements_enabled.  Invalid argument(s)");
	return 0;
}

bool Element_tool_get_select_faces_enabled(struct Element_tool *element_tool)
{
	if (element_tool)
		return element_tool->select_faces_enabled;
	display_message(ERROR_MESSAGE,
		"Element_tool_get_select_faces_enabled.  Invalid argument(s)");
	return false;
}

int Element_tool_set_select_faces_enabled(struct Element_tool *element_tool,
	bool select_faces_enabled)
{
	if (element_tool)
	{
		element_tool->select_faces_enabled = select_faces_enabled;
		return 1;
	}
	display_message(ERROR_MESSAGE,
		"Element_tool_set_select_faces_enabled.  Invalid argument(s)");
	return 0;
}

bool Element_tool_get_select_lines_enabled(struct Element_tool *element_tool)
{
	if (element_tool)
		return element_tool->select_lines_enabled;
	display_message(ERROR_MESSAGE,
		"Element_tool_get_select_lines_enabled.  Invalid argument(s)");
	return false;
}

int Element_tool_set_select_lines_enabled(struct Element_tool *element_tool,
	bool select_lines_enabled)
{
	if (element_tool)
	{
		element_tool->select_lines_enabled=select_lines_enabled;
		return 1;
	}
	display_message(ERROR_MESSAGE,
		"Element_tool_set_select_lines_enabled.  Invalid argument(s)");
	return 0;
}

struct Computed_field *Element_tool_get_command_field(
struct Element_tool *element_tool)
	/*******************************************************************************
	LAST MODIFIED : 5 July 2002

	DESCRIPTION :
	Returns the command_field to be looked up in a web browser when the element is
	clicked on in the <element_tool>.
	==============================================================================*/
{
	struct Computed_field *command_field;

	ENTER(Element_tool_get_command_field);
	if (element_tool)
	{
		command_field=element_tool->command_field;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_tool_get_command_field.  Invalid argument(s)");
		command_field=(struct Computed_field *)NULL;
	}
	LEAVE;

	return (command_field);
} /* Element_tool_get_command_field */

int Element_tool_set_command_field(struct Element_tool *element_tool,
struct Computed_field *command_field)
	/*******************************************************************************
	LAST MODIFIED : 5 July 2002

	DESCRIPTION :
	Sets the command_field to be looked up in a web browser when the element is clicked
	on in the <element_tool>.
	==============================================================================*/
{
	int return_code;

	ENTER(Element_tool_set_command_field);
	if (element_tool && ((!command_field) ||
		Computed_field_has_string_value_type(command_field, (void *)NULL)))
	{
		return_code = 1;
		if (command_field != element_tool->command_field)
		{
			element_tool->command_field = command_field;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_tool_set_command_field.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Element_tool_set_command_field */

struct Interactive_tool *Element_tool_get_interactive_tool(
struct Element_tool *element_tool)
	/*******************************************************************************
	LAST MODIFIED : 29 March 2007

	DESCRIPTION :
	Returns the generic interactive_tool the represents the <element_tool>.
	==============================================================================*/
{
	struct Interactive_tool *interactive_tool;

	ENTER(Element_tool_get_interactive_tool);
	if (element_tool)
	{
		interactive_tool=element_tool->interactive_tool;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_tool_get_interactive_tool.  Invalid argument(s)");
		interactive_tool=(struct Interactive_tool *)NULL;
	}
	LEAVE;

	return (interactive_tool);
} /* Element_tool_get_interactive_tool */

int Element_tool_set_execute_command(struct Element_tool *element_tool,
struct Execute_command *execute_command)
{
	int return_code = 0;
	if (element_tool)
	{
		element_tool->execute_command = execute_command;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_tool_set_execute_command.  Invalid argument(s)");
	}

	return return_code;
}
