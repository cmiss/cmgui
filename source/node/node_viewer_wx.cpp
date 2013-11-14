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
#if 1
#include "configure/cmgui_configure.h"
#endif /* defined (1) */

#include "zinc/core.h"
#include "zinc/field.h"
#include "zinc/fieldcache.h"
#include "zinc/fieldfiniteelement.h"
#include "zinc/fieldgroup.h"
#include "zinc/fieldmodule.h"
#include "zinc/fieldsubobjectgroup.h"
#include "zinc/region.h"
#include "zinc/scene.h"
#include "zinc/status.h"
#include "zinc/timenotifier.h"
#include "zinc/timekeeper.h"
#include "zinc/timesequence.h"
// GRC want to eliminate following 2 includes
#include "computed_field/computed_field.h"
#include "finite_element/finite_element.h"
#include "finite_element/finite_element_region.h"
#include "general/debug.h"
#include "general/mystring.h"
#include "node/node_viewer_wx.h"
#include "general/message.h"
#include "time/time_keeper_app.hpp"
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
	struct FE_region *fe_region;
	struct MANAGER(Computed_field) *computed_field_manager;
	void *computed_field_manager_callback_id;
	struct Time_keeper_app *time_keeper_app;
	cmzn_timenotifier_id timenotifier;
	int timenotifier_callback;
#if defined (WX_USER_INTERFACE)
	wxNodeViewer *wx_node_viewer;
	wxScrolledWindow *collpane;
	wxWindow *win;
	wxFrame *frame;
	wxGridSizer *grid_field;
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

static void Node_viewer_FE_region_change(struct FE_region *fe_region,
	 struct FE_region_changes *changes, void *node_viewer_void);
/*******************************************************************************
LAST MODIFIED : 24 April 2007

DESCRIPTION :
Note that we do not have to handle add, delete and identifier change messages
here as the select widget does this for us. Only changes to the content of the
object cause updates.
==============================================================================*/

int Node_viewer_update_collpane(struct Node_viewer *node_viewer);
char *node_viewer_get_component_value_string(struct Node_viewer *node_viewer, cmzn_field_id field, int component_number, enum cmzn_node_value_label node_value_label, int version);

static int node_viewer_setup_components(
	struct Node_viewer *node_viewer, cmzn_node_id node, cmzn_field_id field, bool &time_varying_field);

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
	 DEFINE_FE_region_FE_object_method_class(node);
	 FE_object_text_chooser< FE_node, FE_region_FE_object_method_class(node) > *node_text_chooser;
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
			node_text_chooser =
				new FE_object_text_chooser< FE_node, FE_region_FE_object_method_class(node) >(node_text_panel, node_viewer->current_node, node_viewer->fe_region, (LIST_CONDITIONAL_FUNCTION(FE_node) *)NULL, NULL);
			Callback_base< FE_node* > *node_text_callback =
				(new Callback_member_callback< FE_node*,
				wxNodeViewer, int (wxNodeViewer::*)(FE_node *) >
				(this, &wxNodeViewer::node_text_callback));
			node_text_chooser->set_callback(node_text_callback);
		wxPanel *region_chooser_panel =
			 XRCCTRL(*this, "RegionChooserPanel", wxPanel);
		char *initial_path;
		initial_path = cmzn_region_get_root_region_path();
		region_chooser = new wxRegionChooser(region_chooser_panel,
			node_viewer->region, initial_path);
		DEALLOCATE(initial_path);
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
Callback from wxChooser<Computed_field> when choice is made.
==============================================================================*/
	{
		USE_PARAMETER(region);
		if (region)
		{
			FE_region *fe_region = cmzn_region_get_FE_region(region);
			node_text_chooser->set_region(fe_region);
		}
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
	int node_text_callback(FE_node *node)
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
				int result = !CMZN_OK;
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
							result = cmzn_field_assign_real(field, field_cache, numberOfComponents, values);
						}
						delete[] values;
						cmzn_field_destroy(&assignField);
					} break;
				case CMZN_FIELD_VALUE_TYPE_STRING:
					{
						result = cmzn_field_assign_string(field, field_cache, value_string);
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

	 struct FE_node  *get_selected_node()
	 {
			return node_text_chooser->get_object();
	 }

	 int set_selected_node(FE_node *new_node)
	 {
		  return node_text_chooser->set_object(new_node);
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
	cmzn_fieldcache_id field_cache, cmzn_field_id field, bool &time_varying_field)
{
	char *field_name = cmzn_field_get_name(field);
	wxScrolledWindow *panel = node_viewer->collpane;

	// identifier is the name of the panel in the collapsible pane
	// field_name is the name of the CollapsiblePane
	node_viewer->win = panel->FindWindowByName(wxString::FromAscii(field_name));
	if (node_viewer->win != NULL)
	{
		node_viewer->win->DestroyChildren();
	}
	else
	{
		wxCollapsiblePane *collapsiblepane = new wxCollapsiblePane(panel, /*id*/-1, wxString::FromAscii(field_name));
		wxSizer *sizer = panel->GetSizer();
		sizer->Add(collapsiblepane, 0,wxALL, 5);
		node_viewer->win = collapsiblepane->GetPane();
		node_viewer->win->SetName(wxString::FromAscii(field_name));
	}

	if (node_viewer->current_node && cmzn_field_is_defined_at_location(field, field_cache))
	{
		node_viewer_setup_components(node_viewer, node_viewer->current_node, field, time_varying_field);
		if (node_viewer->grid_field != NULL)
		{
			node_viewer->win->SetSizer(node_viewer->grid_field);
			node_viewer->grid_field->SetSizeHints(node_viewer->win);
			node_viewer->grid_field->Layout();
			node_viewer->win->Layout();
		}
	}

	panel->FitInside();
	panel->SetScrollbars(20, 20, 50, 50);
	panel->Layout();
	wxFrame *frame;
	frame = XRCCTRL(*node_viewer->wx_node_viewer, "CmguiNodeViewer", wxFrame);
	frame->Layout();
	frame->SetMinSize(wxSize(50,100));
	cmzn_deallocate(field_name);
	return 1;
}

int Node_viewer_remove_unused_collpane(struct Node_viewer *node_viewer)
{
	int return_code = 0;
	if (node_viewer)
	{
		wxWindowList list = node_viewer->collpane->GetChildren();
		wxWindowList::iterator iter;
		for (iter = list.begin(); iter != list.end(); ++iter)
		{
			wxWindow *current = *iter;
			wxWindow *child = ((wxCollapsiblePane *)current)->GetPane();
			wxString tmpstr = child->GetName().GetData();
			const char *field_name = tmpstr.mb_str(wxConvUTF8);
			cmzn_fieldmodule_id fieldModule = cmzn_region_get_fieldmodule(node_viewer->region);
			cmzn_field_id field = cmzn_fieldmodule_find_field_by_name(fieldModule, field_name);
			if (!field)
			{
				current->Destroy();
			}
			cmzn_fieldmodule_destroy(&fieldModule);
		}
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

int Node_viewer_update_collpane(struct Node_viewer *node_viewer)
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
		while ((0 != (field = cmzn_fielditerator_next(iter))))
		{
			node_viewer_add_collpane(node_viewer, field_cache, field, time_varying);
			cmzn_field_destroy(&field);
		}
		cmzn_fielditerator_destroy(&iter);
		cmzn_fieldcache_destroy(&field_cache);
		cmzn_fieldmodule_destroy(&field_module);
		Node_viewer_remove_unused_collpane(node_viewer);
		if (time_varying)
		{
			if (!node_viewer->timenotifier_callback)
			{
				if (CMZN_OK == cmzn_timenotifier_set_callback(node_viewer->timenotifier,
					node_field_time_change_callback, (void *)node_viewer))
				{
					node_viewer->timenotifier_callback = 1;
				}
			}
		}
		else
		{
			if (node_viewer->timenotifier_callback)
			{
				cmzn_timenotifier_clear_callback(node_viewer->timenotifier);
				node_viewer->timenotifier_callback = 0;
			}
		}
	}
	return return_code;
}

static void Node_viewer_Computed_field_change(
	struct MANAGER_MESSAGE(Computed_field) *message,void *node_viewer_void)
{
	struct Node_viewer *node_viewer;
	struct LIST(Computed_field) *changed_field_list=NULL;

	ENTER(Node_viewer_Computed_field_change);
	node_viewer = (struct Node_viewer *)node_viewer_void;
	if (message && node_viewer)
	{
		cmzn_scene_id scene = cmzn_region_get_scene(node_viewer->region);
		if (scene)
		{
			cmzn_field_group_id selection_group = cmzn_scene_get_selection_group(scene);
			changed_field_list =
				MANAGER_MESSAGE_GET_CHANGE_LIST(Computed_field)(message,
					MANAGER_CHANGE_RESULT(Computed_field));
			if (selection_group && changed_field_list && Computed_field_or_ancestor_satisfies_condition(
				cmzn_field_group_base_cast(selection_group), Computed_field_is_in_list, (void *)changed_field_list))
			{
				cmzn_fieldmodule_id field_module = cmzn_region_get_fieldmodule(node_viewer->region);
				cmzn_nodeset_id master_nodeset = cmzn_fieldmodule_find_nodeset_by_domain_type(
					field_module, node_viewer->domain_type);
				cmzn_fieldmodule_destroy(&field_module);
				cmzn_field_node_group_id node_group = cmzn_field_group_get_node_group(selection_group, master_nodeset);
				cmzn_nodeset_destroy(&master_nodeset);
				cmzn_nodeset_group_id nodeset_group = cmzn_field_node_group_get_nodeset(node_group);
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
						{
							Node_viewer_update_collpane(node_viewer);
						}
					}
					cmzn_node_destroy(&node);
				}
				cmzn_nodeset_group_destroy(&nodeset_group);
			}
			if (changed_field_list)
				DESTROY(LIST(Computed_field))(&changed_field_list);
			cmzn_scene_destroy(&scene);
			if (selection_group)
			{
				cmzn_field_group_destroy(&selection_group);
			}
		}
	}
}

cmzn_node_id Node_viewer_get_first_node(Node_viewer *node_viewer)
{
	cmzn_node_id node = 0;
	if (node_viewer)
	{
		cmzn_fieldmodule_id field_module = cmzn_region_get_fieldmodule(node_viewer->region);
		cmzn_nodeset_id nodeset = cmzn_fieldmodule_find_nodeset_by_domain_type(field_module,
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
	struct Time_keeper_app *time_keeper_app)
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
			node_viewer->computed_field_manager =
				cmzn_region_get_Computed_field_manager(node_viewer->region);
			node_viewer->node_viewer_address = node_viewer_address;
			node_viewer->domain_type = domain_type;
			node_viewer->fe_region = cmzn_region_get_FE_region(root_region);
			if (domain_type == CMZN_FIELD_DOMAIN_TYPE_DATAPOINTS)
			{
				node_viewer->fe_region = FE_region_get_data_FE_region(node_viewer->fe_region);
			}
			node_viewer->collpane = NULL;
			node_viewer->time_keeper_app = time_keeper_app->access();
			node_viewer->timenotifier = cmzn_timekeeper_create_timenotifier_regular(
				time_keeper_app->getTimeKeeper(), /*update_frequency*/10.0, /*time_offset*/0.0);
			node_viewer->timenotifier_callback = 0;
			node_viewer->grid_field = NULL;
#if defined (WX_USER_INTERFACE)
			node_viewer->grid_field = NULL;
			node_viewer->wx_node_viewer = (wxNodeViewer *)NULL;
#endif /* defined (WX_USER_INTERFACE) */
			node_viewer->current_node = Node_viewer_get_first_node(node_viewer);
			node_viewer->computed_field_manager_callback_id =
				MANAGER_REGISTER(Computed_field)(Node_viewer_Computed_field_change,
				(void *)node_viewer,	node_viewer->computed_field_manager);
			FE_region_add_callback(node_viewer->fe_region,
				Node_viewer_FE_region_change, (void *)node_viewer);
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
			node_viewer->win=NULL;
			wxBoxSizer *Collpane_sizer = new wxBoxSizer( wxVERTICAL );
			node_viewer->collpane->SetSizer(Collpane_sizer);
			Node_viewer_update_collpane(node_viewer);
			node_viewer->frame=
				XRCCTRL(*node_viewer->wx_node_viewer, "CmguiNodeViewer", wxFrame);
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
		/* end callback from region */
		FE_region_remove_callback(node_viewer->fe_region,
			Node_viewer_FE_region_change, (void *)node_viewer);
		if (node_viewer->computed_field_manager_callback_id)
		{
			MANAGER_DEREGISTER(Computed_field)(
					node_viewer->computed_field_manager_callback_id,
					node_viewer->computed_field_manager);
			node_viewer->computed_field_manager_callback_id = NULL;
		}
		if (node_viewer->wx_node_viewer)
		{
			 delete node_viewer->wx_node_viewer;
		}
		cmzn_timenotifier_destroy(&(node_viewer->timenotifier));
		DEACCESS(Time_keeper_app)(&(node_viewer->time_keeper_app));
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

static void Node_viewer_FE_region_change(struct FE_region *fe_region,
	struct FE_region_changes *changes, void *node_viewer_void)
/*******************************************************************************
LAST MODIFIED : 24 April 2007

DESCRIPTION :
Note that we do not have to handle add, delete and identifier change messages
here as the select widget does this for us. Only changes to the content of the
object cause updates.
==============================================================================*/
{
	int fe_node_change;
	struct Node_viewer *node_viewer;

	ENTER(Node_viewer_cmzn_region_change);
	if (fe_region && changes &&
		(node_viewer = (struct Node_viewer *)node_viewer_void))
	{
		 if (node_viewer->wx_node_viewer)
		 {
				if (CHANGE_LOG_QUERY(FE_node)(changes->fe_node_changes,
							node_viewer->wx_node_viewer->get_selected_node(),
							&fe_node_change))
				{
					 if (fe_node_change | CHANGE_LOG_OBJECT_CHANGED(FE_node))
					 {
							Node_viewer_set_viewer_node(node_viewer);
					 }
				}
		 }
	}
	else
	{
		 display_message(ERROR_MESSAGE,
				"Node_viewer_cmzn_region_change.  Invalid argument(s)");
	}
	LEAVE;
} /* Node_viewer_cmzn_region_change */

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


int node_viewer_add_textctrl(Node_viewer *node_viewer, cmzn_field_id field,
	int component_number, cmzn_node_value_label node_value_label, int version)
/*******************************************************************************
LAST MODIFIED : 24 April 2007

DESCRIPTION :
Add textctrl box onto the viewer.
==============================================================================*/
{
	char *temp_string;
	wxNodeViewerTextCtrl *node_viewer_text =
		new wxNodeViewerTextCtrl(node_viewer, field, component_number, node_value_label, version);
	temp_string = node_viewer_get_component_value_string(
		node_viewer, field, component_number, node_value_label, version);
	if (temp_string != NULL)
	{
		node_viewer_text ->Create(node_viewer->win, -1, wxString::FromAscii(temp_string),wxDefaultPosition,
			wxDefaultSize,wxTE_PROCESS_ENTER);
		DEALLOCATE(temp_string);
	}
	else
	{
		node_viewer_text->Create (node_viewer->win, -1, wxT("ERROR"),wxDefaultPosition,
			wxDefaultSize,wxTE_PROCESS_ENTER);
	}
	node_viewer_text->Connect(wxEVT_COMMAND_TEXT_ENTER,
		wxCommandEventHandler(wxNodeViewerTextCtrl::OnNodeViewerTextCtrlEntered));
	node_viewer_text->Connect(wxEVT_KILL_FOCUS,
		wxCommandEventHandler(wxNodeViewerTextCtrl::OnNodeViewerTextCtrlEntered));
	node_viewer->grid_field->Add(node_viewer_text, 0,
		wxALIGN_CENTER_VERTICAL|wxALIGN_CENTER_HORIZONTAL|wxADJUST_MINSIZE, 0);
	return (1);
}

/***************************************************************************//**
 * Creates the array of cells containing field component values and derivatives
 * and their labels.
 * Assumes field is defined at node.
 */
static int node_viewer_setup_components(
	struct Node_viewer *node_viewer, cmzn_node_id node, cmzn_field_id field, bool &time_varying_field)
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
			cmzn_nodeset_id nodeset = cmzn_fieldmodule_find_nodeset_by_domain_type(field_module,
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

		// first row is blank cell followed by nodal value type labels
		node_viewer->grid_field = new wxGridSizer(
			number_of_components + 1, number_of_node_value_labels + 1, 1, 1);
		node_viewer->grid_field->Add(new wxStaticText(
			node_viewer->win, -1, wxT("")), 1, wxEXPAND|wxADJUST_MINSIZE, 0);
		for (int nodal_value_no = 0; nodal_value_no < number_of_node_value_labels; ++nodal_value_no)
		{
			tmp_string = wxString::FromAscii(nodal_value_labels[nodal_value_no]);
			node_viewer->grid_field->Add(new wxStaticText(node_viewer->win, -1, tmp_string),1,
				wxALIGN_CENTER_VERTICAL|
				wxALIGN_CENTER_HORIZONTAL|wxADJUST_MINSIZE, 0);
		}

		for (int comp_no = 1; comp_no <= number_of_components; ++comp_no)
		{
			// first column is component label */
			char *new_string = cmzn_field_get_component_name(field, comp_no);
			tmp_string = wxString::FromAscii(new_string);
			node_viewer->grid_field->Add(new wxStaticText(node_viewer->win, -1, tmp_string),1,
				wxALIGN_CENTER_VERTICAL|
				wxALIGN_CENTER_HORIZONTAL|wxADJUST_MINSIZE, 0);
			cmzn_deallocate(new_string);

			for (int nodal_value_no = 0; nodal_value_no < number_of_node_value_labels; ++nodal_value_no)
			{
				enum cmzn_node_value_label node_value_label = node_value_labels[nodal_value_no];
				if (!feField || (0 < cmzn_nodetemplate_get_value_number_of_versions(
					nodeTemplate, field, comp_no, node_value_label)))
				{
					node_viewer_add_textctrl(node_viewer, field, comp_no, node_value_label, 1);
				}
				else
				{
					node_viewer->grid_field->Add(new wxStaticText(node_viewer->win, -1, wxT("")),1,
						wxEXPAND|wxALIGN_CENTER_VERTICAL|
						wxALIGN_CENTER_HORIZONTAL|wxADJUST_MINSIZE, 0);
				}
			}
		}
		cmzn_timesequence_id timeSequence = cmzn_nodetemplate_get_timesequence(nodeTemplate, field);
		if (timeSequence)
		{
			time_varying_field = true;
			cmzn_timesequence_destroy(&timeSequence);
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
			node_viewer->region = region;
			FE_region_remove_callback(node_viewer->fe_region,
				Node_viewer_FE_region_change, (void *)node_viewer);
			if (region)
			{
				node_viewer->fe_region = cmzn_region_get_FE_region(region);

				if (node_viewer->domain_type == CMZN_FIELD_DOMAIN_TYPE_DATAPOINTS)
				{
					node_viewer->fe_region = FE_region_get_data_FE_region(
						node_viewer->fe_region);
				}
				FE_region_add_callback(node_viewer->fe_region,
						Node_viewer_FE_region_change, (void *) node_viewer);
				node_viewer->current_node = Node_viewer_get_first_node(node_viewer);
				if (node_viewer->current_node)
				{
					return_code = node_viewer->wx_node_viewer->set_selected_node(node_viewer->current_node);
					if (node_viewer->wx_node_viewer && node_viewer->collpane)
					{
						Node_viewer_update_collpane(node_viewer);
					}
				}
				if (node_viewer->computed_field_manager_callback_id)
				{
					MANAGER_DEREGISTER(Computed_field)(
							node_viewer->computed_field_manager_callback_id,
							node_viewer->computed_field_manager);
					node_viewer->computed_field_manager_callback_id = NULL;
				}
				node_viewer->computed_field_manager	=
					cmzn_region_get_Computed_field_manager(region);
				if (node_viewer->computed_field_manager)
				{
					node_viewer->computed_field_manager_callback_id =
						MANAGER_REGISTER(Computed_field)(Node_viewer_Computed_field_change,
							(void *)node_viewer,	node_viewer->computed_field_manager);
				}
			}
			else
			{
				return_code=0;
				node_viewer->current_node=NULL;
				node_viewer->region = NULL;
				node_viewer->fe_region = (struct FE_region *)NULL;
			}
		}
	}
	else
	{
		return_code=0;
	}
	return (return_code);
}


