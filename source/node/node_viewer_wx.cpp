/***************************************************************************//**
 * FILE : node_viewer_wx.cpp
 *
 * Dialog for selecting nodes and viewing and/or editing field values. Works
 * with selection to display the last selected node, or set it if entered in
 * this dialog.
 */
/* OpenCMISS-Cmgui Application
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#include "configure/cmgui_configure.h"

#include "opencmiss/zinc/core.h"
#include "opencmiss/zinc/field.h"
#include "opencmiss/zinc/fieldcache.h"
#include "opencmiss/zinc/fieldfiniteelement.h"
#include "opencmiss/zinc/fieldgroup.h"
#include "opencmiss/zinc/fieldmodule.h"
#include "opencmiss/zinc/fieldsubobjectgroup.h"
#include "opencmiss/zinc/node.h"
#include "opencmiss/zinc/nodeset.h"
#include "opencmiss/zinc/nodetemplate.h"
#include "opencmiss/zinc/region.h"
#include "opencmiss/zinc/scene.h"
#include "opencmiss/zinc/status.h"
#include "opencmiss/zinc/timenotifier.h"
#include "opencmiss/zinc/timekeeper.h"
#include "opencmiss/zinc/timesequence.h"
#include "general/debug.h"
#include "general/mystring.h"
#include "node/node_viewer_wx.h"
#include "general/message.h"
#if defined (WX_USER_INTERFACE)
#include <wx/collpane.h>
#include "wx/wx.h"
#include "wx/xrc/xmlres.h"
#include "node/node_viewer_wx.xrch"
#include "region/cmiss_region_chooser_wx.hpp"
#include "choose/text_FE_choose_class.hpp"
#include "icon/cmiss_icon.xpm"
#endif /*defined (WX_USER_INTERFACE)*/

/*
Module variables
----------------
*/

#if defined (WX_USER_INTERFACE)
class wxNodeViewer;
#endif /* defined (WX_USER_INTERFACE) */

/***************************************************************************//**
 * Contains all the information carried by the node_viewer widget.
 */
struct Node_viewer
{
	struct Node_viewer **node_viewer_address;
	cmzn_node_id current_node;
	struct cmzn_region *region;
	cmzn_field_domain_type domain_type;
	cmzn_fieldmodulenotifier_id fieldmodulenotifier;
	cmzn_timekeeper_id timekeeper;
	cmzn_timenotifier_id timenotifier;
#if defined (WX_USER_INTERFACE)
	wxNodeViewer *wx_node_viewer;
	wxScrolledWindow *collpane;
	wxFrame *frame;
	int init_width, init_height, frame_width, frame_height;
#endif /* (WX_USER_INTERFACE) */
}; /* node_viewer_struct */

/*
Prototype
------------
*/
static int Node_viewer_set_cmzn_region(struct Node_viewer *node_viewer,
	struct cmzn_region *region);

static int Node_viewer_set_viewer_node(struct Node_viewer *node_viewer);
/*******************************************************************************
LAST MODIFIED : 24 April 2007

DESCRIPTION :
Gets the current node from the select widget, makes a copy of it if not NULL,
and passes it to the node_viewer.
==============================================================================*/

int Node_viewer_update_collpane(struct Node_viewer *node_viewer, cmzn_fieldmoduleevent_id event = 0);
char *node_viewer_get_component_value_string(struct Node_viewer *node_viewer, cmzn_field_id field, int component_number, enum cmzn_node_value_label node_value_label, int version);

static int node_viewer_setup_components(struct Node_viewer *node_viewer, wxWindow *parentWin,
	cmzn_node_id node, cmzn_field_id field, bool &time_varying_field, bool& refit);

/*
Module constants
----------------
*/

/* following must be big enough to hold an element_xi value */
#define VALUE_STRING_SIZE 100

/*
Module functions
----------------
*/

class wxNodeViewer : public wxFrame
{
	 Node_viewer *node_viewer;
	 wxPanel *node_text_panel;
	 wxScrolledWindow *variable_viewer_panel;
	 FE_object_text_chooser< cmzn_node > *node_text_chooser;
	 wxFrame *frame;
	 wxRegionChooser *region_chooser;
public:

	 wxNodeViewer(Node_viewer *node_viewer):
			node_viewer(node_viewer)
	 {
			wxXmlInit_node_viewer_wx();
			node_viewer->wx_node_viewer = this;
			wxXmlResource::Get()->LoadFrame(this,
				(wxWindow *)NULL, _T("CmguiNodeViewer"));
			this->SetIcon(cmiss_icon_xpm);
			wxPanel *node_text_panel =
				XRCCTRL(*this, "NodeTextPanel",wxPanel);
			node_text_chooser = new FE_object_text_chooser< cmzn_node >(node_text_panel,
				node_viewer->region, node_viewer->domain_type, node_viewer->current_node,
				static_cast<FE_object_text_chooser<cmzn_node>::conditional_function_type *>(0), /*user_data*/0);
			Callback_base< cmzn_node* > *node_text_callback =
				(new Callback_member_callback< cmzn_node*,
				wxNodeViewer, int (wxNodeViewer::*)(cmzn_node *) >
				(this, &wxNodeViewer::node_text_callback));
			node_text_chooser->set_callback(node_text_callback);
			wxPanel *region_chooser_panel =
				 XRCCTRL(*this, "RegionChooserPanel", wxPanel);
			region_chooser = new wxRegionChooser(region_chooser_panel,
				node_viewer->region, /*initial_path*/"/");
			Callback_base<cmzn_region* > *Node_viewer_wx_region_callback =
				new Callback_member_callback< cmzn_region*,
				wxNodeViewer, int (wxNodeViewer::*)(cmzn_region *) >
				(this, &wxNodeViewer::Node_viewer_wx_region_callback);
			region_chooser->set_callback(Node_viewer_wx_region_callback);
			Show();
			frame = XRCCTRL(*this, "CmguiNodeViewer",wxFrame);
			frame->GetSize(&(node_viewer->init_width), &(node_viewer->init_height));
			frame->SetSize(node_viewer->frame_width,node_viewer->frame_height+100);
			frame->GetSize(&(node_viewer->frame_width), &(node_viewer->frame_height));
			frame->Layout();
	 };

	 wxNodeViewer()
	 {
	 };

  ~wxNodeViewer()
	 {
			delete node_text_chooser;
			delete region_chooser;
	 }

	int Node_viewer_wx_region_callback(cmzn_region *region)
/*******************************************************************************
LAST MODIFIED : 9 February 2007

DESCRIPTION :
Callback from region chooser when choice is made.
==============================================================================*/
	{
		USE_PARAMETER(region);
		if (region)
			node_text_chooser->set_region(region);
		if (Node_viewer_set_cmzn_region(node_viewer, region) &&
			node_viewer->wx_node_viewer && node_viewer->collpane)
		{
			Node_viewer_update_collpane(node_viewer);
		}
		return 1;
	}

	/*******************************************************************************
	 * Callback from wxTextChooser when text is entered.
	 */
	int node_text_callback(cmzn_node *node)
	{
		if (node_viewer)
		{
			if (node)
			{
				if (node_viewer->current_node)
					cmzn_node_destroy(&node_viewer->current_node);
				node_viewer->current_node = cmzn_node_access(node);
				Node_viewer_update_collpane(node_viewer);
			}
			else
			{
				Node_viewer_set_viewer_node(node_viewer);
				if (node_viewer->wx_node_viewer && node_viewer->collpane)
				{
					Node_viewer_update_collpane(node_viewer);
				}
			}
		}
		else
		{
			display_message(WARNING_MESSAGE,
				"Node_text_callback.  Invalid argument(s)");
		}
		LEAVE;
		return 1;
	}

	void RenewNodeViewer(wxCollapsiblePaneEvent& event)
	{
		USE_PARAMETER(event);
		wxScrolledWindow *VariableViewer = XRCCTRL(*this,"VariableViewerPanel",wxScrolledWindow);
		VariableViewer->Layout();
		frame = XRCCTRL(*this, "CmguiNodeViewer", wxFrame);
		frame->SetMinSize(wxSize(50,100));
		frame->SetMaxSize(wxSize(2000,2000));
	}

	void OnClosepressed(wxCommandEvent &event)
	{
		USE_PARAMETER(event);
		Node_viewer_destroy(node_viewer->node_viewer_address);
	}

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
			frame = XRCCTRL(*this, "CmguiNodeViewer",wxFrame);
			frame->Freeze();
			frame->GetSize(&temp_width, &temp_height);
			if (temp_height !=node_viewer->frame_height || temp_width !=node_viewer->frame_width)
			{
				 if (temp_width>node_viewer->init_width || temp_height>node_viewer->init_height)
				 {
						node_viewer->frame_width = temp_width;
						node_viewer->frame_height = temp_height;
				 }
				 else
				 {
						frame->SetSize(node_viewer->frame_width,node_viewer->frame_height);
				 }
			}
			frame->Thaw();
			frame->Layout();
	 }

	 void Terminate(wxCloseEvent& event)
	 {
		 USE_PARAMETER(event);
		 Node_viewer_destroy(node_viewer->node_viewer_address);
	 }

	void NodeViewerTextEntered(wxTextCtrl *textctrl, Node_viewer *node_viewer,
		cmzn_field_id field, int component_number, enum cmzn_node_value_label node_value_label, int version)
	{
		if (textctrl && node_viewer && field && node_viewer->current_node)
		{
			wxString wxValueString = textctrl->GetValue();
			const char *value_string = wxValueString.mb_str(wxConvUTF8);
			if (value_string != NULL)
			{
				int result = CMZN_ERROR_GENERAL;
				cmzn_fieldmodule_id field_module = cmzn_field_get_fieldmodule(field);
				cmzn_fieldmodule_begin_change(field_module);
				cmzn_fieldcache_id field_cache = cmzn_fieldmodule_create_fieldcache(field_module);
				double time = cmzn_timenotifier_get_time(node_viewer->timenotifier);
				cmzn_fieldcache_set_time(field_cache, time);
				cmzn_fieldcache_set_node(field_cache, node_viewer->current_node);
				enum cmzn_field_value_type valueType = cmzn_field_get_value_type(field);
				switch (valueType)
				{
				case CMZN_FIELD_VALUE_TYPE_REAL:
					{
						cmzn_field_id assignField = 0;
						if ((node_value_label != CMZN_NODE_VALUE_LABEL_VALUE) || (version != 1))
						{
							assignField = cmzn_fieldmodule_create_field_node_value(field_module, field, node_value_label, version);
						}
						else
						{
							assignField = cmzn_field_access(field);
						}
						const int numberOfComponents = cmzn_field_get_number_of_components(field);
						double *values = new double[numberOfComponents];
						if (CMZN_OK == cmzn_field_evaluate_real(assignField, field_cache, numberOfComponents, values))
						{
							sscanf(value_string, FE_VALUE_INPUT_STRING, &values[component_number - 1]);
							result = (CMZN_OK == cmzn_field_assign_real(assignField, field_cache, numberOfComponents, values));
						}
						delete[] values;
						cmzn_field_destroy(&assignField);
					} break;
				case CMZN_FIELD_VALUE_TYPE_STRING:
					{
						result = (CMZN_OK == cmzn_field_assign_string(field, field_cache, value_string));
					} break;
				default:
					{
					} break;
				}
				cmzn_fieldcache_destroy(&field_cache);
				cmzn_fieldmodule_end_change(field_module);
				cmzn_fieldmodule_destroy(&field_module);

				if (result != CMZN_OK)
				{
					display_message(ERROR_MESSAGE, "Cannot set this field's value");
				}
				/* refresh value shown in the text field widgets */
				char *temp_string = node_viewer_get_component_value_string(node_viewer, field, component_number, node_value_label, version);
				if (temp_string != NULL)
				{
					textctrl->SetValue(wxString::FromAscii(temp_string));
				}
			}
		}
	}

	 struct cmzn_node  *get_selected_node()
	 {
			return node_text_chooser->get_object();
	 }

	 void set_selected_node(cmzn_node *new_node)
	 {
		  node_text_chooser->set_object(new_node);
	 }

  DECLARE_DYNAMIC_CLASS(wxNodeViewer);
  DECLARE_EVENT_TABLE();
};

IMPLEMENT_DYNAMIC_CLASS(wxNodeViewer, wxFrame)
BEGIN_EVENT_TABLE(wxNodeViewer, wxFrame)
	 EVT_COLLAPSIBLEPANE_CHANGED(wxID_ANY,wxNodeViewer::RenewNodeViewer)
	 EVT_BUTTON(XRCID("CloseButton"),wxNodeViewer::OnClosepressed)
#if !defined (__WXGTK__)
	 EVT_SIZE(wxNodeViewer::FrameSetSize)
#endif /*!defined (__WXGTK__)*/
	 EVT_CLOSE(wxNodeViewer::Terminate)
END_EVENT_TABLE()

class wxNodeViewerTextCtrl : public wxTextCtrl
{
	 Node_viewer *node_viewer;
	 cmzn_field_id field;
	 int component_number;
	 enum cmzn_node_value_label node_value_label;
	 int version;

public:

  wxNodeViewerTextCtrl(Node_viewer *node_viewer,
		 cmzn_field_id field,
		 int component_number,
		 enum cmzn_node_value_label node_value_label,
		 int version) :
		 node_viewer(node_viewer), field(field), component_number(component_number),
		 node_value_label(node_value_label), version(version)
  {
  }

  ~wxNodeViewerTextCtrl()
  {
  }

  void OnNodeViewerTextCtrlEntered(wxCommandEvent& event)
  {
	USE_PARAMETER(event);
		 node_viewer->wx_node_viewer->NodeViewerTextEntered
				(this, node_viewer, field, component_number, node_value_label, version);
   }

};

/** @param time_varying_field  Initialise to false before calling. Set to true if any field is time varying on node */
static int node_viewer_add_collpane(Node_viewer *node_viewer,
	cmzn_fieldcache_id field_cache, cmzn_field_id field, bool &time_varying_field, bool& refit)
{
	char *field_name = cmzn_field_get_name(field);
	wxScrolledWindow *panel = node_viewer->collpane;

	// identifier is the name of the panel in the collapsible pane
	// field_name is the name of the CollapsiblePane
	wxWindow *wind = 0;
	wxWindowList list = node_viewer->collpane->GetChildren();
	wxWindowList::iterator iter;
	int insertIndex = 0;
	for (iter = list.begin(); iter != list.end(); ++iter)
	{
		wxCollapsiblePane *current = wxDynamicCast(*iter, wxCollapsiblePane);
		if (current)
		{
			wxWindow *child = current->GetPane();
			wxString tmpstr = child->GetName().GetData();
			const char *window_name = tmpstr.mb_str(wxConvUTF8);
			int comparison = strcmp(window_name, field_name);
			if (0 == comparison)
			{
				wind = child;
				break;
			}
			if (0 > comparison)
				++insertIndex;
		}
	}
	if (node_viewer->current_node && cmzn_field_is_defined_at_location(field, field_cache))
	{
		if (0 == wind)
		{
			wxCollapsiblePane *collapsiblepane = new wxCollapsiblePane(panel, /*id*/-1, wxString::FromAscii(field_name));
			wxSizer *sizer = panel->GetSizer();
			sizer->Insert(insertIndex, collapsiblepane, 0, wxALL, 5);
			wind = collapsiblepane->GetPane();
			wind->SetName(wxString::FromAscii(field_name));
			refit = true;
		}
		node_viewer_setup_components(node_viewer, wind, node_viewer->current_node,
			field, time_varying_field, refit);
	}
	else if (0 != wind)
	{
		wxString noName;
		wind->SetName(noName);
		wind->DestroyChildren();
		refit = true;
	}
	cmzn_deallocate(field_name);
	return 1;
}

int Node_viewer_remove_unused_collpane(struct Node_viewer *node_viewer, bool& refit)
{
	int return_code = 0;
	if (node_viewer)
	{
		cmzn_fieldmodule_id fieldModule = cmzn_region_get_fieldmodule(node_viewer->region);
		wxWindowList list = node_viewer->collpane->GetChildren();
		wxWindowList::iterator iter;
		for (iter = list.begin(); iter != list.end(); ++iter)
		{
			wxWindow *current = *iter;
			wxCollapsiblePane *currentPane = wxDynamicCast(current, wxCollapsiblePane);
			if (currentPane)
			{
				wxWindow *child = currentPane->GetPane();
				wxString tmpstr = child->GetName().GetData();
				const char *field_name = tmpstr.mb_str(wxConvUTF8);
				cmzn_field_id field = cmzn_fieldmodule_find_field_by_name(fieldModule, field_name);
				if (field)
					cmzn_field_destroy(&field);
				else
				{
					current->Destroy();
					refit = true;
				}
			}
		}
		cmzn_fieldmodule_destroy(&fieldModule);
	}
	return return_code;
}

static void node_field_time_change_callback(
	cmzn_timenotifierevent_id timenotifierevent, void *node_viewer_void)
/*******************************************************************************
LAST MODIFIED : 5 December 2001

DESCRIPTION :
==============================================================================*/
{
	struct Node_viewer *node_viewer;

	if((node_viewer =	(struct Node_viewer *)node_viewer_void))
	{
// 		node_viewer_widget_update_values(node_viewer);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"node_field_viewer_widget_time_change_callback.  Invalid argument(s)");
	}
} /* node_field_viewer_widget_time_change_callback */

/**
 * @event  Optional field module event; if supplied only updates panes for modified fields.
 */
int Node_viewer_update_collpane(struct Node_viewer *node_viewer, cmzn_fieldmoduleevent_id event)
{
	int return_code = 0;
	if (node_viewer)
	{
		cmzn_fieldmodule_id field_module = cmzn_region_get_fieldmodule(node_viewer->region);
		cmzn_fieldcache_id field_cache = cmzn_fieldmodule_create_fieldcache(field_module);
		cmzn_fieldcache_set_time(field_cache, cmzn_timenotifier_get_time(node_viewer->timenotifier));
		cmzn_fieldcache_set_node(field_cache, node_viewer->current_node);
		cmzn_fielditerator_id iter = cmzn_fieldmodule_create_fielditerator(field_module);
		cmzn_field_id field = 0;
		bool time_varying = false;
		cmzn_field_change_flags field_change;
		bool refit = false;
		while ((0 != (field = cmzn_fielditerator_next(iter))))
		{
			if ((!event) || ((field_change = cmzn_fieldmoduleevent_get_field_change_flags(event, field))
					& (CMZN_FIELD_CHANGE_FLAG_ADD | CMZN_FIELD_CHANGE_FLAG_IDENTIFIER | CMZN_FIELD_CHANGE_FLAG_RESULT)))
				node_viewer_add_collpane(node_viewer, field_cache, field, time_varying, refit);
			cmzn_field_destroy(&field);
		}
		cmzn_fielditerator_destroy(&iter);
		cmzn_fieldcache_destroy(&field_cache);
		cmzn_fieldmodule_destroy(&field_module);
		Node_viewer_remove_unused_collpane(node_viewer, refit);
		if (time_varying)
			cmzn_timenotifier_set_callback(node_viewer->timenotifier,
				node_field_time_change_callback, (void *)node_viewer);
		else
			cmzn_timenotifier_clear_callback(node_viewer->timenotifier);
		if (refit)
		{
			wxScrolledWindow *panel = node_viewer->collpane;
			panel->FitInside();
			panel->SetScrollbars(20, 20, 50, 50);
			panel->Layout();
			node_viewer->frame->Layout();
			node_viewer->frame->SetMinSize(wxSize(50,100));
		}
	}
	return return_code;
}

/** callback from field module for changes to fields, nodes etc. */
static void cmzn_fieldmoduleevent_to_Node_viewer(
	cmzn_fieldmoduleevent_id event, void *node_viewer_void)
{
	struct Node_viewer *node_viewer = static_cast<struct Node_viewer *>(node_viewer_void);
	if (event && node_viewer)
	{
		cmzn_fieldmodule_id field_module = cmzn_region_get_fieldmodule(node_viewer->region);
		cmzn_nodeset_id master_nodeset = cmzn_fieldmodule_find_nodeset_by_field_domain_type(
			field_module, node_viewer->domain_type);

		// get the selected node, if selection has changed
		cmzn_field_change_flags field_change_summary = cmzn_fieldmoduleevent_get_summary_field_change_flags(event);
		bool updateCollPane = false;
		cmzn_scene_id scene = cmzn_region_get_scene(node_viewer->region);
		cmzn_field_id selection_field = cmzn_scene_get_selection_field(scene);
		if (!selection_field)
			selection_field = cmzn_fieldmodule_find_field_by_name(field_module, "cmiss_selection");
		if (selection_field && (0 != (CMZN_FIELD_CHANGE_FLAG_RESULT &
			cmzn_fieldmoduleevent_get_field_change_flags(event, selection_field))))
		{
			cmzn_field_group_id selection_group = cmzn_field_cast_group(selection_field);
			cmzn_field_node_group_id node_group = cmzn_field_group_get_field_node_group(selection_group, master_nodeset);
			cmzn_nodeset_group_id nodeset_group = cmzn_field_node_group_get_nodeset_group(node_group);
			cmzn_field_node_group_destroy(&node_group);
			/* make sure there is only one node selected in group */
			if (1 == cmzn_nodeset_get_size(cmzn_nodeset_group_base_cast(nodeset_group)))
			{
				cmzn_nodeiterator_id iterator = cmzn_nodeset_create_nodeiterator(
					cmzn_nodeset_group_base_cast(nodeset_group));
				cmzn_node_id node = cmzn_nodeiterator_next(iterator);
				cmzn_nodeiterator_destroy(&iterator);
				if (node != node_viewer->current_node)
				{
					if (node_viewer->wx_node_viewer)
					{
						node_viewer->wx_node_viewer->set_selected_node(node);
						if (node_viewer->current_node)
							cmzn_node_destroy(&node_viewer->current_node);
						node_viewer->current_node = cmzn_node_access(node);
					}
					if (node_viewer->wx_node_viewer && node_viewer->collpane)
						updateCollPane = true;
				}
				cmzn_node_destroy(&node);
			}
			cmzn_nodeset_group_destroy(&nodeset_group);
			cmzn_field_group_destroy(&selection_group);
		}
		cmzn_field_destroy(&selection_field);
		cmzn_scene_destroy(&scene);

		// re-show any fields that have changed at currently selected node
		cmzn_nodesetchanges_id nodesetchanges = cmzn_fieldmoduleevent_get_nodesetchanges(event, master_nodeset);
		cmzn_node_change_flags node_change = cmzn_nodesetchanges_get_node_change_flags(
			nodesetchanges, node_viewer->wx_node_viewer->get_selected_node());
		if (node_change & (~CMZN_NODE_CHANGE_FLAG_FIELD))
		{
			Node_viewer_set_viewer_node(node_viewer);
			updateCollPane = true;
		}
		if (updateCollPane)
			Node_viewer_update_collpane(node_viewer);
		else if ((0 != (node_change & CMZN_NODE_CHANGE_FLAG_FIELD)) ||
			(0 != (field_change_summary & (CMZN_FIELD_CHANGE_FLAG_ADD | CMZN_FIELD_CHANGE_FLAG_REMOVE |
				CMZN_FIELD_CHANGE_FLAG_IDENTIFIER | CMZN_FIELD_CHANGE_FLAG_FULL_RESULT))))
			Node_viewer_update_collpane(node_viewer, event);
		cmzn_nodesetchanges_destroy(&nodesetchanges);

		cmzn_nodeset_destroy(&master_nodeset);
		cmzn_fieldmodule_destroy(&field_module);
	}
}

cmzn_node_id Node_viewer_get_first_node(Node_viewer *node_viewer)
{
	cmzn_node_id node = 0;
	if (node_viewer)
	{
		cmzn_fieldmodule_id field_module = cmzn_region_get_fieldmodule(node_viewer->region);
		cmzn_nodeset_id nodeset = cmzn_fieldmodule_find_nodeset_by_field_domain_type(field_module,
			node_viewer->domain_type);
		cmzn_nodeiterator_id iter = cmzn_nodeset_create_nodeiterator(nodeset);
		node = cmzn_nodeiterator_next(iter);
		cmzn_nodeiterator_destroy(&iter);
		cmzn_nodeset_destroy(&nodeset);
		cmzn_fieldmodule_destroy(&field_module);
	}
	return node;
}

struct Node_viewer *Node_viewer_create(
	struct Node_viewer **node_viewer_address,
	const char *dialog_title,
	cmzn_region_id root_region, cmzn_field_domain_type domain_type,
	cmzn_timekeeper_id timekeeper)
{
	struct Node_viewer *node_viewer;
	ENTER(CREATE(Node_viewer));
	node_viewer = (struct Node_viewer *)NULL;
	if (node_viewer_address && dialog_title && root_region)
	{
		/* allocate memory */
		if (ALLOCATE(node_viewer,struct Node_viewer,1))
		{
			node_viewer->region = root_region;
			cmzn_fieldmodule_id fieldmodule = cmzn_region_get_fieldmodule(node_viewer->region);
			node_viewer->fieldmodulenotifier = cmzn_fieldmodule_create_fieldmodulenotifier(fieldmodule);
			cmzn_fieldmodulenotifier_set_callback(node_viewer->fieldmodulenotifier,
				cmzn_fieldmoduleevent_to_Node_viewer, static_cast<void *>(node_viewer));
			cmzn_fieldmodule_destroy(&fieldmodule);
			node_viewer->node_viewer_address = node_viewer_address;
			node_viewer->domain_type = domain_type;
			node_viewer->collpane = NULL;
			node_viewer->timekeeper = cmzn_timekeeper_access(timekeeper);
			node_viewer->timenotifier = cmzn_timekeeper_create_timenotifier_regular(
				timekeeper, /*update_frequency*/10.0, /*time_offset*/0.0);
#if defined (WX_USER_INTERFACE)
			node_viewer->wx_node_viewer = (wxNodeViewer *)NULL;
#endif /* defined (WX_USER_INTERFACE) */
			node_viewer->current_node = Node_viewer_get_first_node(node_viewer);
			/* make the dialog shell */
#if defined (WX_USER_INTERFACE)
			node_viewer->frame_width = 0;
			node_viewer->frame_height = 0;
			node_viewer->init_width = 0;
			node_viewer->init_height=0;
			wxLogNull logNo;
			node_viewer->wx_node_viewer = new wxNodeViewer(node_viewer);
			node_viewer->collpane =
				XRCCTRL(*node_viewer->wx_node_viewer, "VariableViewerPanel", wxScrolledWindow);
			wxBoxSizer *Collpane_sizer = new wxBoxSizer( wxVERTICAL );
			node_viewer->collpane->SetSizer(Collpane_sizer);
			node_viewer->frame=
				XRCCTRL(*node_viewer->wx_node_viewer, "CmguiNodeViewer", wxFrame);
			Node_viewer_update_collpane(node_viewer);
			node_viewer->frame->SetTitle(wxString::FromAscii(dialog_title));
			node_viewer->frame->Layout();
			node_viewer->frame->SetMinSize(wxSize(50,100));
			node_viewer->collpane->Layout();
#endif /* defined (WX_USER_INTERFACE) */
			if (node_viewer->current_node != 0)
			{
				/* select the node to be displayed in dialog; note this is ok
				here as we are not receiving selection callbacks yet */
				node_viewer->wx_node_viewer->set_selected_node(node_viewer->current_node);
				if (node_viewer->wx_node_viewer && node_viewer->collpane)
				{
					Node_viewer_update_collpane(node_viewer);
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"CREATE(Node_viewer).  Could not open hierarchy");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"CREATE(Node_viewer).  Invalid argument(s)");
	}
	if (node_viewer_address)
	{
		*node_viewer_address=node_viewer;
	}
	LEAVE;

	return (node_viewer);
}

int Node_viewer_destroy(struct Node_viewer **node_viewer_address)
{
	int return_code;
	struct Node_viewer *node_viewer;
	if (node_viewer_address &&
		(node_viewer= *node_viewer_address))
	{
		cmzn_fieldmodulenotifier_destroy(&node_viewer->fieldmodulenotifier);
		if (node_viewer->wx_node_viewer)
			 delete node_viewer->wx_node_viewer;
		cmzn_timenotifier_destroy(&(node_viewer->timenotifier));
		cmzn_timekeeper_destroy(&(node_viewer->timekeeper));
		DEALLOCATE(*node_viewer_address);
		return_code=1;
	}
	else
	{
		return_code=0;
	}
	return (return_code);
}

int Node_viewer_bring_window_to_front(struct Node_viewer *node_viewer)
/*******************************************************************************
LAST MODIFIED : 11 May 2000

DESCRIPTION :
Pops the window for <node_viewer> to the front of those visible.
??? De-iconify as well?
==============================================================================*/
{
	int return_code;

	ENTER(Node_viewer_bring_window_to_front);
	if (node_viewer->wx_node_viewer)
	{
		 node_viewer->wx_node_viewer->Raise();
		 return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Node_viewer_bring_window_to_front.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Node_viewer_bring_window_to_front */

/***************************************************************************//**
 * Gets the current node from the select widget, makes a copy of it if not NULL,
 * and passes it to the node_viewer.
 */
static int Node_viewer_set_viewer_node(struct Node_viewer *node_viewer)
{
	int return_code = 1;
	if (node_viewer)
	{
		if (node_viewer->current_node)
			cmzn_node_destroy(&node_viewer->current_node);
		if (node_viewer->wx_node_viewer != 0)
		{
			node_viewer->current_node = cmzn_node_access(node_viewer->wx_node_viewer->get_selected_node());
		}
		return_code=1;
	}
	return (return_code);
}

/*******************************************************************************
 * Get field component value as string
 */
char *node_viewer_get_component_value_string(Node_viewer *node_viewer, cmzn_field_id field,
	int component_number, enum cmzn_node_value_label node_value_label, int version)
{
	char *new_value_string = 0;
	if (node_viewer && field && node_viewer->current_node)
	{
		const int numberOfComponents = cmzn_field_get_number_of_components(field);
		cmzn_fieldmodule_id field_module = cmzn_field_get_fieldmodule(field);
		cmzn_fieldcache_id field_cache = cmzn_fieldmodule_create_fieldcache(field_module);
		double time = cmzn_timenotifier_get_time(node_viewer->timenotifier);
		cmzn_fieldcache_set_time(field_cache, time);
		cmzn_fieldcache_set_node(field_cache, node_viewer->current_node);
		if (1 == numberOfComponents)
		{
			new_value_string = cmzn_field_evaluate_string(field, field_cache);
		}
		else
		{
			// must be numeric
			cmzn_fieldmodule_begin_change(field_module);
			cmzn_field_id useField = 0;
			if ((node_value_label != CMZN_NODE_VALUE_LABEL_VALUE) || (version != 1))
			{
				useField = cmzn_fieldmodule_create_field_node_value(field_module, field, node_value_label, version);
			}
			else
			{
				useField = cmzn_field_access(field);
			}
			const int numberOfComponents = cmzn_field_get_number_of_components(field);
			double *values = new double[numberOfComponents];
			if (CMZN_OK == cmzn_field_evaluate_real(useField, field_cache, numberOfComponents, values))
			{
				char temp_string[VALUE_STRING_SIZE];
				sprintf(temp_string, FE_VALUE_INPUT_STRING, values[component_number-1]);
				new_value_string = duplicate_string(temp_string);
			}
			else
			{
				new_value_string = duplicate_string("nan");
			}
			delete[] values;
			cmzn_field_destroy(&useField);
			cmzn_fieldmodule_end_change(field_module);
		}
		cmzn_fieldcache_destroy(&field_cache);
		cmzn_fieldmodule_destroy(&field_module);
	}
	return new_value_string;
}

/**
 * Add textctrl box onto the viewer.
 */
void Node_viewer_updateTextCtrl(Node_viewer *node_viewer, wxWindow *parentWin,
	wxGridSizer *gridSizer, int index, cmzn_field_id field, int component_number,
	cmzn_node_value_label node_value_label, int version, bool& refit)
{
	wxSizerItem *item = gridSizer->GetItem(index);
	wxNodeViewerTextCtrl *textCtrl = 0;
	wxWindow *window = 0;
	if (item)
	{
		window = item->GetWindow();
		textCtrl = dynamic_cast<wxNodeViewerTextCtrl *>(window);
	}
	char *valueString = node_viewer_get_component_value_string(
		node_viewer, field, component_number, node_value_label, version);
	if (!valueString)
		valueString = duplicate_string("ERROR");
	if (textCtrl)
	{
		wxString oldValue = textCtrl->GetValue();
		wxString newValue(valueString);
		if (newValue != oldValue)
			textCtrl->SetValue(newValue);
	}
	else
	{
		refit = true;
		textCtrl = new wxNodeViewerTextCtrl(node_viewer, field, component_number, node_value_label, version);
		textCtrl->Create(parentWin, -1, wxString::FromAscii(valueString),wxDefaultPosition,
			wxDefaultSize,wxTE_PROCESS_ENTER);
		textCtrl->Connect(wxEVT_COMMAND_TEXT_ENTER,
			wxCommandEventHandler(wxNodeViewerTextCtrl::OnNodeViewerTextCtrlEntered));
		textCtrl->Connect(wxEVT_KILL_FOCUS,
			wxCommandEventHandler(wxNodeViewerTextCtrl::OnNodeViewerTextCtrlEntered));
		if (window)
		{
			gridSizer->Replace(window, textCtrl);
			window->Destroy();
		}
		else
			gridSizer->Insert(index, textCtrl, 1, wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL|wxADJUST_MINSIZE, 0);
	}
	DEALLOCATE(valueString);
}

void gridSizer_updateStaticText(wxWindow *parentWin, wxGridSizer *gridSizer, int index, const char *text, int flag, bool& refit)
{
	wxSizerItem *item = gridSizer->GetItem(index);
	wxStaticText *staticText = 0;
	wxWindow *window = 0;
	if (item)
	{
		window = item->GetWindow();
		staticText = dynamic_cast<wxStaticText *>(window);
	}
	if (staticText)
	{
		wxString oldLabel = staticText->GetLabel();
		wxString newLabel(text);
		if (newLabel != oldLabel)
			staticText->SetLabel(newLabel);
	}
	else
	{
		refit = true;
		staticText = new wxStaticText(parentWin, -1, wxString::FromAscii(text));
		if (window)
		{
			gridSizer->Replace(window, staticText);
			window->Destroy();
		}
		else
			gridSizer->Insert(index, staticText, 1, flag, 0);
	}
}

/**
 * Creates or redisplays the array of cells containing field component values
 * and derivatives and their labels.
 * Assumes field is defined at node.
 * Creates if parentWin is empty, otherwise assumes only fields need redisplaying.
 */
static int node_viewer_setup_components(struct Node_viewer *node_viewer,
	wxWindow *parentWin, cmzn_node_id node, cmzn_field_id field, bool &time_varying_field, bool& refit)
{
	struct cmzn_node_value_label_label
	{
		enum cmzn_node_value_label type;
		const char *label;
	};
	const cmzn_node_value_label_label all_node_value_labels[] =
	{
		{ CMZN_NODE_VALUE_LABEL_VALUE, "value" },
		{ CMZN_NODE_VALUE_LABEL_D_DS1, "d/ds1" },
		{ CMZN_NODE_VALUE_LABEL_D_DS2, "d/ds2" },
		{ CMZN_NODE_VALUE_LABEL_D_DS3, "d/ds3" },
		{ CMZN_NODE_VALUE_LABEL_D2_DS1DS2, "d2/ds1ds2" },
		{ CMZN_NODE_VALUE_LABEL_D2_DS1DS3, "d2/ds1ds3" },
		{ CMZN_NODE_VALUE_LABEL_D2_DS2DS3, "d2/ds2ds3" },
		{ CMZN_NODE_VALUE_LABEL_D3_DS1DS2DS3, "d3/ds1ds2ds3" }
	};
	const int all_node_value_labels_count = sizeof(all_node_value_labels) / sizeof(cmzn_node_value_label_label);
	int return_code = 0;
	wxString tmp_string;
	if (node_viewer && node && field)
	{
		return_code = 1;
		const int number_of_components = cmzn_field_get_number_of_components(field);
		cmzn_field_finite_element_id feField = cmzn_field_cast_finite_element(field);
		cmzn_nodetemplate_id nodeTemplate = 0;
		enum cmzn_node_value_label node_value_labels[8];
		const char *nodal_value_labels[8];
		node_value_labels[0] = CMZN_NODE_VALUE_LABEL_VALUE;
		nodal_value_labels[0] = "value";
		int number_of_node_value_labels = 1;
		if (feField)
		{
			cmzn_fieldmodule_id field_module = cmzn_region_get_fieldmodule(node_viewer->region);
			cmzn_nodeset_id nodeset = cmzn_fieldmodule_find_nodeset_by_field_domain_type(field_module,
				node_viewer->domain_type);
			nodeTemplate = cmzn_nodeset_create_nodetemplate(nodeset);
			cmzn_nodetemplate_define_field_from_node(nodeTemplate, field, node);
			cmzn_nodeset_destroy(&nodeset);
			cmzn_fieldmodule_destroy(&field_module);
			for (int i = 1; i < all_node_value_labels_count; ++i)
			{
				enum cmzn_node_value_label node_value_label = all_node_value_labels[i].type;
				if (0 < cmzn_nodetemplate_get_value_number_of_versions(nodeTemplate,
					field, /*component_number*/-1, node_value_label))
				{
					node_value_labels[number_of_node_value_labels] = node_value_label;
					nodal_value_labels[number_of_node_value_labels] = all_node_value_labels[i].label;
					++number_of_node_value_labels;
				}
			}
		}
		wxGridSizer *gridSizer = dynamic_cast<wxGridSizer *>(parentWin->GetSizer());
		if (gridSizer)
		{
			if ((gridSizer->GetRows() != (number_of_components + 1)) ||
				(gridSizer->GetCols() != number_of_node_value_labels + 1))
			{
				parentWin->DestroyChildren();
				gridSizer = 0;
				refit = true;
			}
		}
		if (!gridSizer)
			gridSizer = new wxGridSizer(number_of_components + 1, number_of_node_value_labels + 1, 1, 1);
		int index = 0;
		// first row is blank cell followed by nodal value type labels
		gridSizer_updateStaticText(parentWin, gridSizer, index++, "", wxEXPAND|wxADJUST_MINSIZE, refit);
		for (int nodal_value_no = 0; nodal_value_no < number_of_node_value_labels; ++nodal_value_no)
			gridSizer_updateStaticText(parentWin, gridSizer, index++, nodal_value_labels[nodal_value_no],
				wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL|wxADJUST_MINSIZE, refit);
		for (int comp_no = 1; comp_no <= number_of_components; ++comp_no)
		{
			// first column is component label */
			char *name = cmzn_field_get_component_name(field, comp_no);
			gridSizer_updateStaticText(parentWin, gridSizer, index++, name,
				wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL|wxADJUST_MINSIZE, refit);
			cmzn_deallocate(name);

			for (int nodal_value_no = 0; nodal_value_no < number_of_node_value_labels; ++nodal_value_no)
			{
				enum cmzn_node_value_label node_value_label = node_value_labels[nodal_value_no];
				if (!feField || (0 < cmzn_nodetemplate_get_value_number_of_versions(
					nodeTemplate, field, comp_no, node_value_label)))
				{
					Node_viewer_updateTextCtrl(node_viewer, parentWin, gridSizer, index++, field, comp_no, node_value_label, 1, refit);
				}
				else
					gridSizer_updateStaticText(parentWin, gridSizer, index++, "",
						wxEXPAND|wxALIGN_CENTER_VERTICAL|	wxALIGN_CENTER_HORIZONTAL|wxADJUST_MINSIZE, refit);
			}
		}
		cmzn_timesequence_id timeSequence = cmzn_nodetemplate_get_timesequence(nodeTemplate, field);
		if (timeSequence)
		{
			time_varying_field = true;
			cmzn_timesequence_destroy(&timeSequence);
		}
		if (refit)
		{
			parentWin->SetSizer(gridSizer);
			gridSizer->SetSizeHints(parentWin);
			gridSizer->Layout();
			parentWin->Layout();
		}

		cmzn_nodetemplate_destroy(&nodeTemplate);
		cmzn_field_finite_element_destroy(&feField);
	}
	return (return_code);
}

static int Node_viewer_set_cmzn_region(struct Node_viewer *node_viewer,
	struct cmzn_region *region)
/*******************************************************************************
LAST MODIFIED : 8 September 2008

DESCRIPTION :
Sets the <region> used by <node_viewer> and update the chooser to include fields
in this region only.
==============================================================================*/
{
	int return_code = 1;
	if (node_viewer)
	{
		if (region != node_viewer->region)
		{
			cmzn_fieldmodulenotifier_destroy(&node_viewer->fieldmodulenotifier);
			node_viewer->region = region;
			if (region)
			{
				node_viewer->current_node = Node_viewer_get_first_node(node_viewer);
				if (node_viewer->current_node)
				{
					node_viewer->wx_node_viewer->set_selected_node(node_viewer->current_node);
					if (node_viewer->wx_node_viewer && node_viewer->collpane)
						Node_viewer_update_collpane(node_viewer);
				}
				cmzn_fieldmodule_id fieldmodule = cmzn_region_get_fieldmodule(node_viewer->region);
				node_viewer->fieldmodulenotifier = cmzn_fieldmodule_create_fieldmodulenotifier(fieldmodule);
				cmzn_fieldmodulenotifier_set_callback(node_viewer->fieldmodulenotifier,
					cmzn_fieldmoduleevent_to_Node_viewer, static_cast<void *>(node_viewer));
				cmzn_fieldmodule_destroy(&fieldmodule);
			}
			else
			{
				return_code=0;
				node_viewer->current_node=NULL;
			}
		}
	}
	else
	{
		return_code=0;
	}
	return (return_code);
}


