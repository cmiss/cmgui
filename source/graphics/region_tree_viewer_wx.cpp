/*******************************************************************************
FILE : region_tree_viewer.cpp

LAST MODIFIED : 26 February 2007

DESCRIPTION :
codes used to build scene editor with wxWidgets.
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

#if 1
#include "configure/cmgui_configure.h"
#endif
#include <cstdio>
#include <string>
#include <vector>

#include "zinc/scene.h"
#include "zinc/graphic.h"
#include "zinc/graphicsmaterial.h"
#include "zinc/graphicsmodule.h"
#include "zinc/spectrum.h"
#include "zinc/status.h"
#include "general/debug.h"
#include "general/indexed_list_private.h"
#include "general/mystring.h"
#include "general/object.h"
#include "graphics/glyph.hpp"
#include "graphics/graphics_object.h"
#include "graphics/scene.h"
#include "graphics/scene_app.h"
#include "general/message.h"
#include "user_interface/user_interface.h"
#include "command/parser.h"
#include "graphics/auxiliary_graphics_types_app.h"

#include "wx/wx.h"
#include <wx/tglbtn.h>
#include <wx/treectrl.h>
#include "wx/xrc/xmlres.h"
#include "choose/choose_manager_class.hpp"
#include "choose/choose_enumerator_class.hpp"
#include "choose/choose_list_class.hpp"
#include "choose/text_choose_from_fe_element.hpp"
#include "dialog/tessellation_dialog.hpp"
#include "transformation/transformation_editor_wx.hpp"
#include <wx/collpane.h>
#include <wx/splitter.h>
#include <wx/imaglist.h>
#include "icon/cmiss_icon.xpm"
#include "icon/tickbox.xpm"
#include "icon/unticked_box.xpm"

#include "computed_field/computed_field_finite_element.h"
#include "computed_field/computed_field.h"
#include "graphics/region_tree_viewer_wx.h"
#include "graphics/font.h"
#include "graphics/scene.h"
#include "graphics/graphic.h"
#include "graphics/graphics_module.h"
#include "graphics/graphics_coordinate_system.hpp"
#include "graphics/tessellation.hpp"
#include "graphics/region_tree_viewer_wx.xrch"

/*
Module types
------------
*/
class wxRegionTreeViewer;

enum
{
	CmguiTree_MenuItem1,
	CmguiTree_MenuItem2,
	CmguiTree_MenuItem3,
	CmguiTree_MenuItem4,
	CmguiTree_Ctrl = 1000
};

class wxCmguiHierachicalTree;

class wxCmguiHierachicalTreeItemData :public wxTreeItemData
{
	struct Cmiss_region *region;
	wxCmguiHierachicalTree *treectrl;

public:
	wxCmguiHierachicalTreeItemData(struct Cmiss_region *input_region,
		wxCmguiHierachicalTree *input_treectrl) : treectrl(input_treectrl)
	{
		region = ACCESS(Cmiss_region)(input_region);
		treectrl = input_treectrl;
		Cmiss_region_add_callback(region,
			propagate_region_change, (void *)this);
	}

	virtual ~wxCmguiHierachicalTreeItemData()
	{
		Cmiss_region_remove_callback(region,
			propagate_region_change, (void *)this);
		DEACCESS(Cmiss_region)(&region);
	}

	Cmiss_region *GetRegion()
	{
		return region;
	}

	static void propagate_region_change(
		struct Cmiss_region *region,struct Cmiss_region_changes *region_changes, void *data_void);
};

class wxCmguiHierachicalTree : public wxTreeCtrl
{
	wxRegionTreeViewer *region_tree_viewer_widget;
public:
	wxCmguiHierachicalTree(wxRegionTreeViewer *region_tree_viewer_widget, wxPanel *parent) :
		wxTreeCtrl(parent, CmguiTree_Ctrl, wxDefaultPosition, wxDefaultSize,
							wxTR_HAS_BUTTONS|wxTR_MULTIPLE), region_tree_viewer_widget(region_tree_viewer_widget)
	{
		wxBoxSizer *sizer = new wxBoxSizer(wxHORIZONTAL);
		sizer->Add(this, wxSizerFlags(1).Align(wxALIGN_CENTER).Expand());
		parent->SetSizer(sizer);
		Show();
		Connect(wxEVT_LEFT_DOWN,
			wxMouseEventHandler(wxCmguiHierachicalTree::SendLeftDownEvent),
			NULL,this);
	}

	wxCmguiHierachicalTree()
	{
	};

	~wxCmguiHierachicalTree()
	{
	};

	void SetTreeIdRegionWithCallback(wxTreeItemId id, Cmiss_region *region)
	{
		wxCmguiHierachicalTreeItemData *data =
			new wxCmguiHierachicalTreeItemData(region, this);
		SetItemData(id, data);
	}

	void remove_child_with_region(wxTreeItemId parent_id, Cmiss_region *child_region)
	{
		wxTreeItemIdValue cookie;
		wxTreeItemId child_id = GetFirstChild(parent_id, cookie);
		Cmiss_region *parent_region = dynamic_cast<wxCmguiHierachicalTreeItemData *>
			(GetItemData(parent_id))->GetRegion();
		while (child_id.IsOk())
		{
			/* if child_region is NULL then find an item with region that does not
				belong to the parent region and remove it from the tree */
			Cmiss_region * current_region = dynamic_cast<wxCmguiHierachicalTreeItemData *>
				(GetItemData(child_id))->GetRegion();
			if ((child_region && child_region == current_region)
				|| (!child_region && !Cmiss_region_contains_subregion(parent_region, current_region)))
			{
				if (IsSelected(child_id))
					SelectItem(child_id, false);
				Delete(child_id);
			}
			child_id = GetNextChild(parent_id, cookie);
		}
	}

	void update_current_tree_item(wxTreeItemId parent_id)
	{
		int i = 0, return_code = 0;
		Cmiss_region *parent_region = dynamic_cast<wxCmguiHierachicalTreeItemData *>
			(GetItemData(parent_id))->GetRegion();
		char *child_name;
		Cmiss_region *child_region = NULL;
		wxTreeItemIdValue cookie;
		child_region = Cmiss_region_get_first_child(parent_region);
		while (child_region)
		{
			return_code = 0;
			child_name = Cmiss_region_get_name(child_region);
			wxTreeItemId child_id = GetFirstChild(parent_id, cookie);
			while (child_id.IsOk() && !return_code)
			{
				Cmiss_region *current_region = dynamic_cast<wxCmguiHierachicalTreeItemData *>
					(GetItemData(child_id))->GetRegion();
				if (current_region == child_region)
				{
					return_code = 1;
				}
				child_id = GetNextChild(parent_id, cookie);
			}
			if (!return_code)
			{
				child_id = InsertItem(parent_id, i, wxString::FromAscii(child_name),0,0);
				SetTreeIdRegionWithCallback(child_id, child_region);
				if (!(this->IsExpanded(parent_id)))
					this->Expand(parent_id);
			}
			if (child_id.IsOk())
			{
				update_current_tree_item(child_id);
			}
			DEALLOCATE(child_name);
			Cmiss_region_reaccess_next_sibling(&child_region);
			i++;
		}
	}

	void region_change(struct Cmiss_region *region,
		struct Cmiss_region_changes *region_changes, wxCmguiHierachicalTreeItemData *data)
	{
		ENTER(region_change);

		if (region && region_changes && data)
		{
			if (region_changes->children_changed)
			{
				const wxTreeItemId parent_id = data->GetId();
				struct Cmiss_region *child_region = NULL;
				if (region_changes->child_added)
				{
					child_region = region_changes->child_added;
					char *name = Cmiss_region_get_name(child_region);
					wxTreeItemId child_id = AppendItem(parent_id,wxString::FromAscii(name),0,0);
					SetTreeIdRegionWithCallback(child_id, child_region);
					update_current_tree_item(child_id);
					DEALLOCATE(name);
					if (!(this->IsExpanded(parent_id)))
						this->Expand(parent_id);
				}
				else if (region_changes->child_removed)
				{
					child_region = region_changes->child_removed;
					remove_child_with_region(parent_id, child_region);
				}
				else
				{
					remove_child_with_region(parent_id, NULL);
					update_current_tree_item(parent_id);
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Scene_Cmiss_region_change.  Invalid argument(s)");
		}
		LEAVE;
	}

void add_all_child_regions_to_tree_item(wxTreeItemId parent_id)
{
	char *child_name;
	wxTreeItemId child_id;
	Cmiss_region *parent_region, *child_region;

	parent_region = dynamic_cast<wxCmguiHierachicalTreeItemData *>
		(GetItemData(parent_id))->GetRegion();
	child_region = Cmiss_region_get_first_child(parent_region);
	while (child_region)
	{
		child_name = Cmiss_region_get_name(child_region);
		if (child_name)
		{
			child_id = AppendItem(parent_id,wxString::FromAscii(child_name),0,0);
			SetTreeIdRegionWithCallback(child_id, child_region);
			add_all_child_regions_to_tree_item(child_id);
			DEALLOCATE(child_name);
		}
		Cmiss_region_reaccess_next_sibling(&child_region);
	}
	this->ExpandAll();
}

private:
	void SendLeftDownEvent(wxMouseEvent& event);

// 	void SendRightDownEvent(wxMousEvent& event);

	DECLARE_DYNAMIC_CLASS(wxCmguiHierachicalTree);
};

IMPLEMENT_DYNAMIC_CLASS(wxCmguiHierachicalTree, wxFrame)

void wxCmguiHierachicalTreeItemData::propagate_region_change(
	struct Cmiss_region *region,struct Cmiss_region_changes *region_changes, void *data_void)
{
	wxCmguiHierachicalTreeItemData *data =
		static_cast<wxCmguiHierachicalTreeItemData *>(data_void);
	data->treectrl->region_change(region,
		region_changes, data);
}

struct Region_tree_viewer
/*******************************************************************************
LAST MODIFIED : 02 Febuary 2007

DESCRIPTION :
==============================================================================*/
{
	/* if autoapply flag is set, any changes to the currently edited graphical
		element will automatically be applied globally */
	int auto_apply, child_edited, child_expanded,
		transformation_expanded, transformation_callback_flag,
		gt_element_group_callback_flag, scene_callback_flag;
	struct Cmiss_graphics_module *graphics_module;
	Cmiss_tessellation_module_id tessellationModule;
	/* access gt_element_group for current_object if applicable */
	struct Cmiss_scene *scene, *edit_scene;
	/* keep address of pointer to editor so can self-destroy */
	struct Region_tree_viewer **region_tree_viewer_address;
	struct MANAGER(Graphical_material) *graphical_material_manager;
	struct Graphical_material *default_material, *selected_material;
	struct Cmiss_font *default_font;
	struct MANAGER(Scene) *scene_manager;
	struct User_interface *user_interface;
	struct Cmiss_graphic *current_graphic;
	Cmiss_glyph_module *glyphModule;
	struct MANAGER(VT_volume_texture) *volume_texture_manager;
	struct MANAGER(Computed_field) *field_manager;
	struct MANAGER(Cmiss_font) *font_manager;
	struct Cmiss_region *root_region, *current_region;
	enum Graphics_select_mode select_mode;
	enum Cmiss_field_domain_type domain_type;
	enum Cmiss_element_point_sample_mode sample_mode;
	enum Streamline_data_type streamline_data_type;
	struct MANAGER(Spectrum) *spectrum_manager;
	enum Cmiss_graphic_render_polygon_mode render_polygon_mode;
	struct FE_element *fe_element;
#if defined (WX_USER_INTERFACE)
	Transformation_editor *transformation_editor;
	wxRegionTreeViewer *wx_region_tree_viewer;
	wxPanel *top_collpane_panel;
	wxScrolledWindow *sceneediting;
	wxFrame *frame;
	wxCheckListBox *checklistbox;
	wxCheckListBox *graphiclistbox;
	wxCmguiHierachicalTree *testing_tree_ctrl;
	wxImageList *ImageList;
	wxSplitterWindow *lowersplitter;
	wxSplitterWindow *verticalsplitter;
	wxCheckBox *autocheckbox;
	wxButton *applybutton;
	wxButton *revertbutton;
	wxCollapsiblePane *top_collpane;
#endif /*defined (WX_USER_INTERFACE)*/
}; /*struct region_tree_viewer*/

void Region_tree_viewer_wx_transformation_change(struct Cmiss_scene *scene,
	gtMatrix *transformation_matrix, void *region_tree_viewer_void);

/***************************************************************************//**
* Revert changes done on the edit gt element group.
*
*/
int Region_tree_viewer_revert_changes(Region_tree_viewer *region_tree_viewer);

void Region_tree_viewer_set_active_scene(
	struct Region_tree_viewer *region_tree_viewer, struct Cmiss_scene *scene);

// new prototypes
static int Region_tree_viewer_add_graphic_item(
	struct Cmiss_graphic *graphic, void *region_tree_viewer_void);

/***************************************************************************//**
 *Get and set the display of graphic
 */
static int get_and_set_Cmiss_graphic_widgets(void *region_tree_viewer_void);

/***************************************************************************//**
 * Iterator function for Rendition_editor_update_Graphic_item.
 */
static int Region_tree_viewer_add_graphic(
	struct Cmiss_graphic *graphic, void *region_tree_viewer_void);

void Region_tree_viewer_set_graphic_widgets_for_scene(Region_tree_viewer *region_tree_viewer);

/*
Module functions
----------------
*/

/***************************************************************************//**
 * This function will be called whenever there are global changes on scene
 */
static int Region_tree_viewer_wx_scene_change(
	struct Cmiss_scene *scene, void *region_tree_viewer_void);

/***************************************************************************//**
* Callback function in region_tree_viewer_wx when object's transformation has been
* changed
*
*/
void Region_tree_viewer_wx_transformation_change(struct Cmiss_scene *scene,
	gtMatrix *transformation_matrix, void *region_tree_viewer_void)
{
	struct Region_tree_viewer *region_tree_viewer =
		(struct Region_tree_viewer *)region_tree_viewer_void;

	if (region_tree_viewer)
	{
		if (scene == region_tree_viewer->scene)
		{
			/* transformation_matrix can be null here which acutally indicates that
				the scene object has not been transformed. */
			region_tree_viewer->transformation_editor->
			set_transformation(transformation_matrix);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Region_tree_viewer_wx_transformation_change.  Invalid argument(s)");
	}
}

#if defined (WX_USER_INTERFACE)

#if defined (__WXMSW__)
struct Region_tree_viewer_size
{
	int previous_width, previous_height, current_width, current_height;
};
#endif /* defined (__WXMSW__) */

BEGIN_DECLARE_EVENT_TYPES()
DECLARE_EVENT_TYPE(wxEVT_TREE_IMAGE_CLICK_EVENT, -1)
END_DECLARE_EVENT_TYPES()

DEFINE_EVENT_TYPE(wxEVT_TREE_IMAGE_CLICK_EVENT)

void wxCmguiHierachicalTree::SendLeftDownEvent(wxMouseEvent& event)
{
	int flags;
	wxTreeItemId id = HitTest( event.GetPosition(), flags );
	if(flags & wxTREE_HITTEST_ONITEMICON)
	{
		if (this->GetSelection() != id)
		{
			this->SelectItem(id);
		}
		wxTreeEvent this_event(wxEVT_TREE_IMAGE_CLICK_EVENT, this, id);
		this_event.SetEventObject( this );
		// Send it
		GetEventHandler()->ProcessEvent(this_event);
	}
	else
	{
		event.Skip();
	}
};

/**
 * Graphic types that can be added in the scene editor. More than actual number
 * of graphic types as some options allow preset attributes. e.g
 * NODE_POINTS creates a POINTS graphic with DOMAIN_NODES and a default
 * coordinate field.
 * Ensure Add_graphic_type_string has a string for each valid enum.
 */
enum Add_graphic_type
{
	ADD_GRAPHIC_TYPE_INVALID = 0,
	ADD_GRAPHIC_TYPE_POINTS,
	ADD_GRAPHIC_TYPE_NODE_POINTS,
	ADD_GRAPHIC_TYPE_DATA_POINTS,
	ADD_GRAPHIC_TYPE_ELEMENT_POINTS,
	ADD_GRAPHIC_TYPE_LINES,
	ADD_GRAPHIC_TYPE_SURFACES,
	ADD_GRAPHIC_TYPE_CONTOURS,
	ADD_GRAPHIC_TYPE_STREAMLINES
};

const char *Add_graphic_type_string(Add_graphic_type add_graphic_type)
{
	switch (add_graphic_type)
	{
	case ADD_GRAPHIC_TYPE_POINTS:
		return "Points";
		break;
	case ADD_GRAPHIC_TYPE_NODE_POINTS:
		return "Node points";
		break;
	case ADD_GRAPHIC_TYPE_DATA_POINTS:
		return "Data points";
		break;
	case ADD_GRAPHIC_TYPE_ELEMENT_POINTS:
		return "Element points";
		break;
	case ADD_GRAPHIC_TYPE_LINES:
		return "Lines";
		break;
	case ADD_GRAPHIC_TYPE_SURFACES:
		return "Surfaces";
		break;
	case ADD_GRAPHIC_TYPE_CONTOURS:
		return "Contours";
		break;
	case ADD_GRAPHIC_TYPE_STREAMLINES:
		return "Streamlines";
		break;
	}
	return 0;
}

class wxRegionTreeViewer : public wxFrame
{
	Region_tree_viewer *region_tree_viewer;
#if defined (__WXMSW__)
	Region_tree_viewer_size region_tree_viewer_size;
#endif /* defined (__WXMSW__) */
	wxScrolledWindow *sceneediting;
	wxFrame *frame;
	wxSplitterWindow *lowersplitter, *verticalsplitter;
	wxCheckListBox *scenechecklist,*graphicalitemschecklist;
	wxListBox *scenelistbox,*graphicalitemslistbox;
	wxStaticText *currentsceneobjecttext,
		*isoscalartext, *glyphtext, *offsettext, *baseglyphsizetext, *select_mode_text,
		*glyphscalefactorstext, *domaintypetext, *sample_mode_text,
		*tessellationtext, *glyph_repeat_mode_text, *labeloffsettext,
		*sample_density_field_text, *xitext,
		*lineshapetext, *streamlineslengthtext, *streamvectortext,
		*line_width_text, *streamlinedatatypetext, *spectrumtext,
		*point_size_text, *render_polygon_mode_text,
		*staticlabeltext, *fonttext;
	wxButton *sceneupbutton, scenedownbutton, *applybutton, *revertbutton, *tessellationbutton;
	wxCheckBox *autocheckbox, *exteriorcheckbox,*facecheckbox, *seedelementcheckbox;
	wxRadioButton *isovaluelistradiobutton, *isovaluesequenceradiobutton;
	wxPanel *isovalueoptionspane;
	wxTextCtrl *nametextfield, *linescalefactorstextctrl, *isoscalartextctrl, *offsettextctrl,
		*baseglyphsizetextctrl,*glyphscalefactorstextctrl,
		*xitextctrl,*streamlineslengthtextctrl,*linebasesizetextctrl,
		*line_width_text_ctrl,*isovaluesequencenumbertextctrl, *isovaluesequencefirsttextctrl,
		*isovaluesequencelasttextctrl, *labeloffsettextctrl,
		*point_size_text_ctrl, *staticlabeltextctrl[3];
	wxPanel	*coordinate_field_chooser_panel, *coordinate_system_chooser_panel, *data_chooser_panel,
		*line_orientation_scale_field_chooser_panel, *isoscalar_chooser_panel, *glyph_chooser_panel,
		*glyph_repeat_mode_chooser_panel,
		*orientation_scale_field_chooser_panel, *variable_scale_field_chooser_panel,
		*label_chooser_panel, *font_chooser_panel, *select_mode_chooser_panel,
		*domain_type_chooser_panel, *sample_mode_chooser_panel,
		*tessellation_field_chooser_panel, *sample_density_field_chooser_panel,
		*line_shape_chooser_panel, *stream_vector_chooser_panel,
		*streamline_data_type_chooser_panel,
		*streamlines_track_direction_chooser_panel, *spectrum_chooser_panel,
		*texture_coordinates_chooser_panel, *render_polygon_mode_chooser_panel,
		*seed_element_panel, *subgroup_field_chooser_panel, *tessellation_chooser_panel;
	wxWindow *glyphbox,*glyphline;
	wxChoice *facechoice, *add_graphic_choice;
	wxString TempText;
	DEFINE_MANAGER_CLASS(Computed_field);
	Managed_object_chooser<Computed_field,MANAGER_CLASS(Computed_field)>
	*coordinate_field_chooser;
	DEFINE_MANAGER_CLASS(Graphical_material);
	Managed_object_chooser<Graphical_material,MANAGER_CLASS(Graphical_material)>
	*graphical_material_chooser;
	Managed_object_chooser<Graphical_material,MANAGER_CLASS(Graphical_material)>
	*selected_material_chooser;
	/*
	DEFINE_ENUMERATOR_TYPE_CLASS(Cmiss_graphic_type);
	Enumerator_chooser<ENUMERATOR_TYPE_CLASS(Cmiss_graphic_type)>
	*graphic_type_chooser;
	*/
	DEFINE_ENUMERATOR_TYPE_CLASS(Graphics_select_mode);
	Enumerator_chooser<ENUMERATOR_TYPE_CLASS(Graphics_select_mode)>
	*select_mode_chooser;
	Managed_object_chooser<Computed_field,MANAGER_CLASS(Computed_field)>
	*data_field_chooser;
	DEFINE_MANAGER_CLASS(Spectrum);
	Managed_object_chooser<Spectrum,MANAGER_CLASS(Spectrum)>
	*spectrum_chooser;
	Managed_object_chooser<Computed_field,MANAGER_CLASS(Computed_field)>
		*line_orientation_scale_field_chooser;
	Managed_object_chooser<Computed_field,MANAGER_CLASS(Computed_field)>
	*isoscalar_chooser;
	DEFINE_MANAGER_CLASS(Cmiss_glyph);
	Managed_object_chooser<Cmiss_glyph,MANAGER_CLASS(Cmiss_glyph)>
	*glyph_chooser;
	DEFINE_ENUMERATOR_TYPE_CLASS(Cmiss_glyph_repeat_mode);
	Enumerator_chooser<ENUMERATOR_TYPE_CLASS(Cmiss_glyph_repeat_mode)>
		*glyph_repeat_mode_chooser;
	Managed_object_chooser<Computed_field,MANAGER_CLASS(Computed_field)>
	*orientation_scale_field_chooser;
	Managed_object_chooser<Computed_field,MANAGER_CLASS(Computed_field)>
	*variable_scale_field_chooser;
	Managed_object_chooser<Computed_field,MANAGER_CLASS(Computed_field)>
	*label_field_chooser;
	Managed_object_chooser<Computed_field,MANAGER_CLASS(Computed_field)>
	*subgroup_field_chooser;
	DEFINE_MANAGER_CLASS(Cmiss_font);
		 Managed_object_chooser<Cmiss_font,MANAGER_CLASS(Cmiss_font)>
	*font_chooser;
	DEFINE_ENUMERATOR_TYPE_CLASS(Cmiss_field_domain_type);
	Enumerator_chooser<ENUMERATOR_TYPE_CLASS(Cmiss_field_domain_type)>
		*domain_type_chooser;
	DEFINE_ENUMERATOR_TYPE_CLASS(Cmiss_element_point_sample_mode);
	Enumerator_chooser<ENUMERATOR_TYPE_CLASS(Cmiss_element_point_sample_mode)>
		*sample_mode_chooser;
	DEFINE_MANAGER_CLASS(Cmiss_tessellation);
	Managed_object_chooser<Cmiss_tessellation,MANAGER_CLASS(Cmiss_tessellation)>
		*tessellation_chooser;
	Managed_object_chooser<Computed_field,MANAGER_CLASS(Computed_field)>
		*tessellation_field_chooser;
	Managed_object_chooser<Computed_field,MANAGER_CLASS(Computed_field)>
	*sample_density_field_chooser;
	wxFeElementTextChooser *seed_element_chooser;
	DEFINE_ENUMERATOR_TYPE_CLASS(Cmiss_graphic_line_attributes_shape);
	Enumerator_chooser<ENUMERATOR_TYPE_CLASS(Cmiss_graphic_line_attributes_shape)>
		*line_shape_chooser;
	DEFINE_ENUMERATOR_TYPE_CLASS(Cmiss_graphics_coordinate_system);
	Enumerator_chooser<ENUMERATOR_TYPE_CLASS(Cmiss_graphics_coordinate_system)>
	*coordinate_system_chooser;
	Managed_object_chooser<Computed_field,MANAGER_CLASS(Computed_field)>
	*stream_vector_chooser;
	DEFINE_ENUMERATOR_TYPE_CLASS(Streamline_data_type);
	Enumerator_chooser<ENUMERATOR_TYPE_CLASS(Streamline_data_type)>
	*streamline_data_type_chooser;
	Managed_object_chooser<Computed_field,MANAGER_CLASS(Computed_field)>
	*texture_coord_field_chooser;
	DEFINE_ENUMERATOR_TYPE_CLASS(Cmiss_graphic_render_polygon_mode);
	Enumerator_chooser<ENUMERATOR_TYPE_CLASS(Cmiss_graphic_render_polygon_mode)>
	*render_polygon_mode_chooser;
	DEFINE_ENUMERATOR_TYPE_CLASS(Cmiss_graphic_streamlines_track_direction);
	Enumerator_chooser<ENUMERATOR_TYPE_CLASS(Cmiss_graphic_streamlines_track_direction)>
		*streamlines_track_direction_chooser;
	wxWindowID tessellationWindowID;
public:

  wxRegionTreeViewer(Region_tree_viewer *region_tree_viewer) :
	region_tree_viewer(region_tree_viewer)
  {
		wxXmlInit_region_tree_viewer_wx();
		region_tree_viewer->wx_region_tree_viewer = (wxRegionTreeViewer *)NULL;
		wxXmlResource::Get()->LoadFrame(this,
				(wxWindow *)NULL, _T("CmguiRegionTreeViewer"));
		this->SetIcon(cmiss_icon_xpm);

#if defined (__WXMSW__)
		region_tree_viewer_size.previous_width = 0;
		region_tree_viewer_size.previous_height = 0;
		region_tree_viewer_size.current_width = 0;
		region_tree_viewer_size.current_height = 0;
#endif /* defined (__WXMSW__) */


	/* Set the coordinate_field_chooser_panel*/
	coordinate_field_chooser = NULL;
	/* Set the graphical_material_chooser_panel*/
	wxPanel *graphical_material_chooser_panel =
		XRCCTRL(*this, "GraphicalMaterialChooserPanel",wxPanel);
	graphical_material_chooser =
		new Managed_object_chooser<Graphical_material,MANAGER_CLASS(Graphical_material)>
		(graphical_material_chooser_panel, region_tree_viewer->default_material, region_tree_viewer->graphical_material_manager,
				(MANAGER_CONDITIONAL_FUNCTION(Graphical_material) *)NULL, (void *)NULL, region_tree_viewer->user_interface);
	Callback_base< Graphical_material* > *graphical_material_callback =
		new Callback_member_callback< Graphical_material*,
				wxRegionTreeViewer, int (wxRegionTreeViewer::*)(Graphical_material *) >
		(this, &wxRegionTreeViewer::graphical_material_callback);
	graphical_material_chooser->set_callback(graphical_material_callback);
	graphical_material_chooser_panel->Fit();
	/* Set the selected_material_chooser_panel*/
	wxPanel *selected_material_chooser_panel =
		XRCCTRL(*this, "SelectedMaterialChooserPanel",wxPanel);
	selected_material_chooser =
		new Managed_object_chooser<Graphical_material,MANAGER_CLASS(Graphical_material)>
		(selected_material_chooser_panel, region_tree_viewer->selected_material, region_tree_viewer->graphical_material_manager,
				(MANAGER_CONDITIONAL_FUNCTION(Graphical_material) *)NULL, (void *)NULL, region_tree_viewer->user_interface);
	Callback_base< Graphical_material* > *selected_material_callback =
		new Callback_member_callback< Graphical_material*,
				wxRegionTreeViewer, int (wxRegionTreeViewer::*)(Graphical_material *) >
		(this, &wxRegionTreeViewer::selected_material_callback);
	selected_material_chooser->set_callback(selected_material_callback);
	selected_material_chooser_panel->Fit();
	tessellationWindowID = 0;

	wxPanel *add_graphic_chooser_panel = XRCCTRL(*this, "AddGraphicChooserPanel", wxPanel);
	add_graphic_choice = new wxChoice(add_graphic_chooser_panel, /*id*/-1, wxPoint(0,0), wxSize(-1,-1));
	add_graphic_choice->Append(wxString::FromAscii("Add..."), (void *)ADD_GRAPHIC_TYPE_INVALID);
	add_graphic_choice->Append(wxString::FromAscii("---"), (void *)ADD_GRAPHIC_TYPE_INVALID);
	for (size_t i = 1; ; ++i)
	{
		const char *add_graphic_type_string =
			Add_graphic_type_string(static_cast<Add_graphic_type>(i));
		if (!add_graphic_type_string)
			break;
		add_graphic_choice->Append(wxString::FromAscii(add_graphic_type_string), reinterpret_cast<void *>(i));
	}
	add_graphic_choice->SetSelection(0);
	add_graphic_choice->Connect(wxEVT_COMMAND_CHOICE_SELECTED,
		wxCommandEventHandler(wxRegionTreeViewer::AddGraphicChoice), NULL, this);

	data_field_chooser = NULL;
	/* Set the spectrum_chooser*/
	wxPanel *spectrum_chooser_panel =
		XRCCTRL(*this,"SpectrumChooserPanel", wxPanel);
	spectrum_chooser =
		new Managed_object_chooser<Spectrum,MANAGER_CLASS(Spectrum)>
		(spectrum_chooser_panel, /*initial*/static_cast<Cmiss_spectrum*>(0), region_tree_viewer->spectrum_manager,
				(MANAGER_CONDITIONAL_FUNCTION(Spectrum) *)NULL, (void *)NULL, region_tree_viewer->user_interface);
	Callback_base< Spectrum* > *spectrum_callback =
		new Callback_member_callback< Spectrum*,
				wxRegionTreeViewer, int (wxRegionTreeViewer::*)(Spectrum *) >
		(this, &wxRegionTreeViewer::spectrum_callback);
	spectrum_chooser->set_callback(spectrum_callback);
	spectrum_chooser_panel->Fit();

	/* Set the tessellation_chooser*/
	wxPanel *tessellation_chooser_panel =
		 XRCCTRL(*this,"TessellationChooserPanel", wxPanel);
	tessellation_chooser =
		new Managed_object_chooser<Cmiss_tessellation,MANAGER_CLASS(Cmiss_tessellation)>(
			tessellation_chooser_panel, /*initial*/static_cast<Cmiss_tessellation*>(0),
			Cmiss_tessellation_module_get_manager(region_tree_viewer->tessellationModule),
			(MANAGER_CONDITIONAL_FUNCTION(Cmiss_tessellation) *)NULL, (void *)NULL,
			region_tree_viewer->user_interface);
	Callback_base< Cmiss_tessellation* > *tessellation_callback =
		 new Callback_member_callback< Cmiss_tessellation*,
				wxRegionTreeViewer, int (wxRegionTreeViewer::*)(Cmiss_tessellation *) >
		 (this, &wxRegionTreeViewer::tessellation_callback);
	tessellation_chooser->set_callback(tessellation_callback);

	wxPanel *coordinate_system_chooser_panel =
		XRCCTRL(*this, "CoordinateSystemChooserPanel", wxPanel);
	coordinate_system_chooser =
		new Enumerator_chooser<ENUMERATOR_TYPE_CLASS(Cmiss_graphics_coordinate_system)>
		(coordinate_system_chooser_panel,
			CMISS_GRAPHICS_COORDINATE_SYSTEM_LOCAL,
				(ENUMERATOR_CONDITIONAL_FUNCTION(Cmiss_graphics_coordinate_system) *)NULL,
				(void *)NULL, region_tree_viewer->user_interface);
	coordinate_system_chooser_panel->Fit();
	Callback_base< enum Cmiss_graphics_coordinate_system > *coordinate_system_callback =
		new Callback_member_callback< enum Cmiss_graphics_coordinate_system,
				wxRegionTreeViewer, int (wxRegionTreeViewer::*)(enum Cmiss_graphics_coordinate_system) >
		(this, &wxRegionTreeViewer::Region_tree_viewer_coordinate_system_callback);
	coordinate_system_chooser->set_callback(coordinate_system_callback);
	coordinate_system_chooser->set_value(CMISS_GRAPHICS_COORDINATE_SYSTEM_LOCAL);
	coordinate_system_chooser_panel->Fit();

	select_mode_chooser = NULL;
	line_orientation_scale_field_chooser = NULL;
	isoscalar_chooser = NULL;
	glyph_chooser = NULL;
	glyph_repeat_mode_chooser = 0;
	orientation_scale_field_chooser = NULL;
	variable_scale_field_chooser = NULL;
	label_field_chooser= NULL;
	subgroup_field_chooser= NULL;
	font_chooser=NULL;
	domain_type_chooser = NULL;
	sample_mode_chooser = NULL;
	tessellation_field_chooser = NULL;
	sample_density_field_chooser =NULL;
	line_shape_chooser = NULL;
	stream_vector_chooser = NULL;
	streamline_data_type_chooser = NULL;
	texture_coord_field_chooser = NULL;
	render_polygon_mode_chooser = NULL;
	seed_element_chooser = NULL;
	streamlines_track_direction_chooser = 0;
	graphicalitemschecklist = NULL;
	tessellationbutton = NULL;

	XRCCTRL(*this,"NameTextField", wxTextCtrl)->Connect(wxEVT_KILL_FOCUS,
		wxCommandEventHandler(wxRegionTreeViewer::GraphicEditorNameText),
		NULL, this);
	XRCCTRL(*this,"IsoScalarTextCtrl", wxTextCtrl)->Connect(wxEVT_KILL_FOCUS,
		wxCommandEventHandler(wxRegionTreeViewer::EnterIsoScalar),
		NULL, this);
	XRCCTRL(*this,"IsoValueSequenceNumberTextCtrl", wxTextCtrl)->Connect(wxEVT_KILL_FOCUS,
		wxCommandEventHandler(wxRegionTreeViewer::EnterIsoRange),
		NULL, this);
	XRCCTRL(*this,"IsoValueSequenceFirstTextCtrl", wxTextCtrl)->Connect(wxEVT_KILL_FOCUS,
		wxCommandEventHandler(wxRegionTreeViewer::EnterIsoRange),
		NULL, this);
	XRCCTRL(*this,"IsoValueSequenceLastTextCtrl", wxTextCtrl)->Connect(wxEVT_KILL_FOCUS,
		wxCommandEventHandler(wxRegionTreeViewer::EnterIsoRange),
		NULL, this);
	XRCCTRL(*this,"OffsetTextCtrl", wxTextCtrl)->Connect(wxEVT_KILL_FOCUS,
		wxCommandEventHandler(wxRegionTreeViewer::EnterGlyphOffset),
		NULL, this);
	XRCCTRL(*this,"BaseGlyphSizeTextCtrl", wxTextCtrl)->Connect(wxEVT_KILL_FOCUS,
		wxCommandEventHandler(wxRegionTreeViewer::EnterGlyphSize),
		NULL, this);
	XRCCTRL(*this,"GlyphScaleFactorsTextCtrl", wxTextCtrl)->Connect(wxEVT_KILL_FOCUS,
		wxCommandEventHandler(wxRegionTreeViewer::EnterGlyphScale),
		NULL, this);
	XRCCTRL(*this,"LabelOffsetTextCtrl", wxTextCtrl)->Connect(wxEVT_KILL_FOCUS,
		wxCommandEventHandler(wxRegionTreeViewer::EnterLabelOffset),
		NULL, this);
	XRCCTRL(*this,"StaticLabelTextCtrl1", wxTextCtrl)->Connect(wxEVT_KILL_FOCUS,
		wxCommandEventHandler(wxRegionTreeViewer::EnterStaticLabelText),
		NULL, this);
	XRCCTRL(*this,"StaticLabelTextCtrl2", wxTextCtrl)->Connect(wxEVT_KILL_FOCUS,
		wxCommandEventHandler(wxRegionTreeViewer::EnterStaticLabelText),
		NULL, this);
	XRCCTRL(*this,"StaticLabelTextCtrl3", wxTextCtrl)->Connect(wxEVT_KILL_FOCUS,
		wxCommandEventHandler(wxRegionTreeViewer::EnterStaticLabelText),
		NULL, this);
	XRCCTRL(*this,"XiTextCtrl", wxTextCtrl)->Connect(wxEVT_KILL_FOCUS,
		wxCommandEventHandler(wxRegionTreeViewer::EnterSampleLocation),
		NULL, this);
	XRCCTRL(*this,"StreamlinesLengthTextCtrl", wxTextCtrl)->Connect(wxEVT_KILL_FOCUS,
		wxCommandEventHandler(wxRegionTreeViewer::EnterStreamlineLength),
		NULL, this);
	XRCCTRL(*this,"LineBaseSizeTextCtrl", wxTextCtrl)->Connect(wxEVT_KILL_FOCUS,
		wxCommandEventHandler(wxRegionTreeViewer::EnterLineBaseSize),
		NULL, this);
	XRCCTRL(*this,"LineScaleFactorsTextCtrl", wxTextCtrl)->Connect(wxEVT_KILL_FOCUS,
		wxCommandEventHandler(wxRegionTreeViewer::EnterLineScaleFactors),
		NULL, this);
	XRCCTRL(*this,"LineWidthTextCtrl", wxTextCtrl)->Connect(wxEVT_KILL_FOCUS,
		wxCommandEventHandler(wxRegionTreeViewer::EnterLineWidth),
		NULL, this);
	XRCCTRL(*this,"PointSizeTextCtrl", wxTextCtrl)->Connect(wxEVT_KILL_FOCUS,
		wxCommandEventHandler(wxRegionTreeViewer::EnterPointSize),
		NULL, this);
	frame=XRCCTRL(*this, "CmguiRegionTreeViewer", wxFrame);
	currentsceneobjecttext=XRCCTRL(*this,"CurrentSceneObjectText",wxStaticText);
#if defined (__WXMSW__)
	frame->GetSize(&(region_tree_viewer_size.current_width), &(region_tree_viewer_size.current_height));
#endif /*defined (__WXMSW__)*/

	frame->Fit();
	Show();
};

  wxRegionTreeViewer()
  {
  };

  ~wxRegionTreeViewer()
	{
		delete font_chooser;
		delete coordinate_field_chooser;
		delete graphical_material_chooser;
		delete selected_material_chooser;
		delete add_graphic_choice;
		//delete graphic_type_chooser;
		delete select_mode_chooser;
		delete data_field_chooser;
		delete spectrum_chooser;
		delete line_orientation_scale_field_chooser;
		delete isoscalar_chooser;
		delete glyph_chooser;
		delete glyph_repeat_mode_chooser;
		delete orientation_scale_field_chooser;
		delete variable_scale_field_chooser;
		delete label_field_chooser;
		delete subgroup_field_chooser;
		delete domain_type_chooser;
		delete sample_mode_chooser;
		delete tessellation_chooser;
		delete tessellation_field_chooser;
		delete sample_density_field_chooser;
		delete line_shape_chooser;
		delete stream_vector_chooser;
		delete streamline_data_type_chooser;
		delete texture_coord_field_chooser;
		delete render_polygon_mode_chooser;
		delete seed_element_chooser;
		delete coordinate_system_chooser;
		delete streamlines_track_direction_chooser;
	}

/***************************************************************************//**
* Set manager in different field manager object choosers.
*
* @param region_tree_viewer scene editor to be modify
*/
void Region_tree_viewer_wx_set_manager_in_choosers(struct Region_tree_viewer *region_tree_viewer)
{
	if (coordinate_field_chooser != NULL)
			coordinate_field_chooser->set_manager(region_tree_viewer->field_manager);
	if (data_field_chooser != NULL)
			data_field_chooser->set_manager(region_tree_viewer->field_manager);
	if (line_orientation_scale_field_chooser != NULL)
			line_orientation_scale_field_chooser->set_manager(region_tree_viewer->field_manager);
	if (isoscalar_chooser != NULL)
			isoscalar_chooser->set_manager(region_tree_viewer->field_manager);
	if (orientation_scale_field_chooser != NULL)
			orientation_scale_field_chooser->set_manager(region_tree_viewer->field_manager);
	if (variable_scale_field_chooser != NULL)
			variable_scale_field_chooser->set_manager(region_tree_viewer->field_manager);
	if (label_field_chooser != NULL)
			label_field_chooser->set_manager(region_tree_viewer->field_manager);
	if (subgroup_field_chooser != NULL)
		 subgroup_field_chooser->set_manager(region_tree_viewer->field_manager);
	if (tessellation_field_chooser != NULL)
			tessellation_field_chooser->set_manager(region_tree_viewer->field_manager);
	if (sample_density_field_chooser != NULL)
			sample_density_field_chooser->set_manager(region_tree_viewer->field_manager);
	if (stream_vector_chooser != NULL)
			stream_vector_chooser->set_manager(region_tree_viewer->field_manager);
	if (texture_coord_field_chooser != NULL)
			texture_coord_field_chooser->set_manager(region_tree_viewer->field_manager);
	if (seed_element_chooser != NULL)
		seed_element_chooser->set_fe_region(Cmiss_region_get_FE_region(Cmiss_scene_get_region(
			region_tree_viewer->edit_scene)));
}

int coordinate_field_callback(Computed_field *coordinate_field)
/*******************************************************************************
LAST MODIFIED : 19 March 2007

DESCRIPTION :
Callback from wxChooser<Coordinate Field> when choice is made.
==============================================================================*/
	{
		Cmiss_graphic_set_coordinate_field(
			region_tree_viewer->current_graphic, coordinate_field);
		Region_tree_viewer_autoapply(region_tree_viewer->scene,
			region_tree_viewer->edit_scene);
		//Region_tree_viewer_renew_label_on_list(region_tree_viewer->current_graphic);
		return(1);
	}

int Region_tree_viewer_coordinate_system_callback(
	enum Cmiss_graphics_coordinate_system coordinate_system)
/*******************************************************************************
LAST MODIFIED : 19 March 2007

DESCRIPTION :
Callback from wxChooser<Coordinate Field> when choice is made.
==============================================================================*/
	{
		Cmiss_graphic_set_coordinate_system(
			region_tree_viewer->current_graphic, coordinate_system);
		Region_tree_viewer_autoapply(region_tree_viewer->scene,
			region_tree_viewer->edit_scene);
		//Region_tree_viewer_renew_label_on_list(region_tree_viewer->current_graphic);
		return(1);
	}

int graphical_material_callback(Graphical_material *material)
/*******************************************************************************
LAST MODIFIED : 19 March 2007

DESCRIPTION :
Callback from wxChooser<Graphical Material> when choice is made.
==============================================================================*/
	{
		Cmiss_graphic_set_material(region_tree_viewer->current_graphic,
			material);
		Region_tree_viewer_autoapply(region_tree_viewer->scene,
			region_tree_viewer->edit_scene);
		Region_tree_viewer_wx_set_list_string(region_tree_viewer->current_graphic);
		return(1);
	}

int selected_material_callback(Graphical_material *selected_material)
/*******************************************************************************
LAST MODIFIED : 20 March 2007

DESCRIPTION :
Callback from wxChooser<Selected Material> when choice is made.
==============================================================================*/
{
	Cmiss_graphic_set_selected_material(region_tree_viewer->current_graphic,
		selected_material);
	Region_tree_viewer_autoapply(region_tree_viewer->scene,
		region_tree_viewer->edit_scene);
	Region_tree_viewer_wx_set_list_string(region_tree_viewer->current_graphic);
	return(1);
}

int select_mode_callback(enum Graphics_select_mode select_mode)
/*******************************************************************************
LAST MODIFIED : 19 March 2007

DESCRIPTION :
Callback from wxChooser<select mode> when choice is made.
==============================================================================*/
{
	Cmiss_graphic_set_select_mode(
		region_tree_viewer->current_graphic, select_mode);
	Region_tree_viewer_autoapply(region_tree_viewer->scene,
		region_tree_viewer->edit_scene);
	//Region_tree_viewer_renew_label_on_list(region_tree_viewer->current_graphic);
	return 1;
}

/**
 * Callback from wxChooser<line orientation scale field> when choice is made.
 */
int line_orientation_scale_field_callback(Computed_field *orientation_scale_field)
{
	Cmiss_graphic_line_attributes_id line_attributes =
		Cmiss_graphic_get_line_attributes(region_tree_viewer->current_graphic);
	Cmiss_graphic_line_attributes_set_orientation_scale_field(line_attributes, orientation_scale_field);
	Cmiss_graphic_line_attributes_destroy(&line_attributes);
	if (orientation_scale_field)
	{
		linescalefactorstextctrl->Enable();
	}
	else
	{
		linescalefactorstextctrl->Disable();
	}
	Region_tree_viewer_autoapply(region_tree_viewer->scene,
		region_tree_viewer->edit_scene);
	//Region_tree_viewer_renew_label_on_list(region_tree_viewer->current_graphic);
	return 1;
}

/**
 * Callback from wxChooser<Iso Scalar> when choice is made.
 */
int isoscalar_callback(Computed_field *isoscalar_field)
{
	Cmiss_graphic_contours_id contours =
		Cmiss_graphic_cast_contours(region_tree_viewer->current_graphic);
	Cmiss_graphic_contours_set_isoscalar_field(contours, isoscalar_field);
	Cmiss_graphic_contours_destroy(&contours);
	Region_tree_viewer_autoapply(region_tree_viewer->scene,
		region_tree_viewer->edit_scene);
	//Region_tree_viewer_renew_label_on_list(region_tree_viewer->current_graphic);

	return 1;
}

/**
 * Callback from wxChooser<Glyph> when choice is made.
 */
int glyph_callback(Cmiss_glyph *glyph)
{
	Cmiss_graphic_point_attributes_id point_attributes =
		Cmiss_graphic_get_point_attributes(region_tree_viewer->current_graphic);
	Cmiss_graphic_point_attributes_set_glyph(point_attributes, reinterpret_cast<Cmiss_glyph_id>(glyph));
	Cmiss_graphic_point_attributes_destroy(&point_attributes);
	/* inform the client of the change */
	Region_tree_viewer_autoapply(region_tree_viewer->scene,
		region_tree_viewer->edit_scene);
	//Region_tree_viewer_renew_label_on_list(region_tree_viewer->current_graphic);
	return 1;
}

/**
 * Callback from wxChooser<Cmiss_glyph_repeat_mode> when choice is made.
 */
int glyph_repeat_mode_callback(enum Cmiss_glyph_repeat_mode glyph_repeat_mode)
{
	Cmiss_graphic_point_attributes_id point_attributes =
		Cmiss_graphic_get_point_attributes(region_tree_viewer->current_graphic);
	Cmiss_graphic_point_attributes_set_glyph_repeat_mode(point_attributes, glyph_repeat_mode);
	Cmiss_graphic_point_attributes_destroy(&point_attributes);
	/* inform the client of the change */
	Region_tree_viewer_autoapply(region_tree_viewer->scene,
		region_tree_viewer->edit_scene);
	//Region_tree_viewer_renew_label_on_list(region_tree_viewer->current_graphic);
	return 1;
}

/**
 * Callback from wxChooser<Orientation Scale> when choice is made.
 */
int orientation_scale_callback(Computed_field *orientation_scale_field)
{
	Cmiss_graphic_point_attributes_id point_attributes =
		Cmiss_graphic_get_point_attributes(region_tree_viewer->current_graphic);
	Cmiss_graphic_point_attributes_set_orientation_scale_field(point_attributes,
		orientation_scale_field);
	Cmiss_graphic_point_attributes_destroy(&point_attributes);
	glyphscalefactorstext	=XRCCTRL(*this,"GlyphScaleFactorsText",wxStaticText);
	glyphscalefactorstextctrl=XRCCTRL(*this,"GlyphScaleFactorsTextCtrl",wxTextCtrl);
	if (orientation_scale_field)
	{
		glyphscalefactorstext->Enable();
		glyphscalefactorstextctrl->Enable();
	}
	else
	{
		glyphscalefactorstext->Disable();
		glyphscalefactorstextctrl->Disable();
	}
	/* inform the client of the change */
	Region_tree_viewer_autoapply(region_tree_viewer->scene,
		region_tree_viewer->edit_scene);
	//Region_tree_viewer_renew_label_on_list(region_tree_viewer->current_graphic);
	return 1;
}

/**
 * Callback from wxChooser<Variable Scale> when choice is made.
 */
int variable_scale_callback(Computed_field *signed_scale_field)
{
	Cmiss_graphic_point_attributes_id point_attributes =
		Cmiss_graphic_get_point_attributes(region_tree_viewer->current_graphic);
	Cmiss_graphic_point_attributes_set_signed_scale_field(point_attributes,
		signed_scale_field);
	Cmiss_graphic_point_attributes_destroy(&point_attributes);
	Region_tree_viewer_autoapply(region_tree_viewer->scene,
		region_tree_viewer->edit_scene);
	//Region_tree_viewer_renew_label_on_list(region_tree_viewer->current_graphic);
	return 1;
}

/**
 * Callback from wxChooser<label> when choice is made.
 */
int label_callback(Computed_field *label_field)
{
	Cmiss_graphic_point_attributes_id point_attributes =
		Cmiss_graphic_get_point_attributes(region_tree_viewer->current_graphic);
	Cmiss_graphic_point_attributes_set_label_field(point_attributes, label_field);
	Cmiss_graphic_point_attributes_destroy(&point_attributes);
	Region_tree_viewer_autoapply(region_tree_viewer->scene,
	   region_tree_viewer->edit_scene);
	//Region_tree_viewer_renew_label_on_list(region_tree_viewer->current_graphic);
	return 1;
}

int subgroup_field_callback(Computed_field *subgroup_field)
{
	Cmiss_graphic_set_subgroup_field(region_tree_viewer->current_graphic,
		subgroup_field);
	Region_tree_viewer_autoapply(region_tree_viewer->scene,
		region_tree_viewer->edit_scene);
	Region_tree_viewer_renew_label_on_list(region_tree_viewer->current_graphic);

	return 1;
}

/**
 * Callback from wxChooser<font> when choice is made.
 */
int font_callback(Cmiss_font *font)
{
	Cmiss_graphic_point_attributes_id point_attributes =
		Cmiss_graphic_get_point_attributes(region_tree_viewer->current_graphic);
	Cmiss_graphic_point_attributes_set_font(point_attributes, font);
	Cmiss_graphic_point_attributes_destroy(&point_attributes);
	Region_tree_viewer_autoapply(region_tree_viewer->scene,
	 region_tree_viewer->edit_scene);
	//Region_tree_viewer_renew_label_on_list(region_tree_viewer->current_graphic);
	return 1;
}

void show_sampling_widgets()
{
	sample_mode_text = XRCCTRL(*this,"SampleModeText",wxStaticText);
	sample_mode_chooser_panel= XRCCTRL(*this,"SampleModeChooserPanel",wxPanel);
	wxStaticText *tessellation_field_text=XRCCTRL(*this,"TessellationFieldText",wxStaticText);
	tessellation_field_chooser_panel=XRCCTRL(*this, "TessellationFieldChooserPanel",wxPanel);
	sample_density_field_text=XRCCTRL(*this, "DensityFieldText",wxStaticText);
	sample_density_field_chooser_panel=XRCCTRL(*this,"DensityFieldChooserPanel",wxPanel);
	xitext=XRCCTRL(*this,"XiText",wxStaticText);
	xitextctrl=XRCCTRL(*this,"XiTextCtrl",wxTextCtrl);
	const int domain_dimension = Cmiss_graphic_get_domain_dimension(region_tree_viewer->current_graphic);
	Cmiss_graphic_sampling_attributes_id sampling = Cmiss_graphic_get_sampling_attributes(region_tree_viewer->current_graphic);
	Cmiss_element_point_sample_mode sample_mode = sampling ?
		Cmiss_graphic_sampling_attributes_get_mode(sampling) : CMISS_ELEMENT_POINT_SAMPLE_MODE_INVALID;
	if (sampling && (domain_dimension > 0))
	{
		sample_mode_text->Enable();
		sample_mode_chooser_panel->Enable();
	}
	else
	{
		sample_mode_text->Disable();
		sample_mode_chooser_panel->Disable();
	}
	if (sampling && (domain_dimension > 0) &&
		(CMISS_ELEMENT_POINT_SAMPLE_CELL_POISSON == sample_mode))
	{
		sample_density_field_text->Enable();
		sample_density_field_chooser_panel->Enable();
	}
	else
	{
		sample_density_field_text->Disable();
		sample_density_field_chooser_panel->Disable();
	}
	if (sampling && (domain_dimension > 0) &&
		(CMISS_ELEMENT_POINT_SAMPLE_SET_LOCATION == sample_mode))
	{
		xitext->Enable();
		xitextctrl->Enable();
	}
	else
	{
		xitext->Disable();
		xitextctrl->Disable();
	}
	if ((domain_dimension > 0) && (CMISS_ELEMENT_POINT_SAMPLE_SET_LOCATION != sample_mode))
	{
		tessellation_field_text->Enable();
		tessellation_field_chooser_panel->Enable();
	}
	else
	{
		tessellation_field_text->Disable();
		tessellation_field_chooser_panel->Disable();
	}
	Cmiss_graphic_sampling_attributes_destroy(&sampling);
}

/**
 * Callback from wxChooser<Domain Type> when choice is made.
 */
int domain_type_callback(enum Cmiss_field_domain_type domain_type)
 {
	if (CMISS_OK != Cmiss_graphic_set_domain_type(region_tree_viewer->current_graphic,domain_type))
	{
		// user chose an invalid domain type for the graphic
		domain_type_chooser->set_value(Cmiss_graphic_get_domain_type(region_tree_viewer->current_graphic));
		return 1;
	}

	exteriorcheckbox=XRCCTRL(*this,"ExteriorCheckBox",wxCheckBox);
	facecheckbox=XRCCTRL(*this, "FaceCheckBox",wxCheckBox);
	facechoice=XRCCTRL(*this, "FaceChoice",wxChoice);
	if ((CMISS_FIELD_DOMAIN_ELEMENTS_1D == domain_type) ||
		(CMISS_FIELD_DOMAIN_ELEMENTS_2D == domain_type))
	{
		exteriorcheckbox->Enable();
		facecheckbox->Enable();
		Cmiss_graphic_set_exterior(region_tree_viewer->current_graphic,
			exteriorcheckbox->IsChecked());
		Cmiss_element_face_type face;
		if (facecheckbox->IsChecked())
		{
			facechoice->Enable();
			face = static_cast<Cmiss_element_face_type>(facechoice->GetSelection() + CMISS_ELEMENT_FACE_XI1_0);
		}
		else
		{
			facechoice->Disable();
			face= CMISS_ELEMENT_FACE_ALL;
		}
		Cmiss_graphic_set_face(region_tree_viewer->current_graphic,face);
	}
	else
	{
		exteriorcheckbox->Disable();
		facecheckbox->Disable();
		facechoice->Disable();
	}
	show_sampling_widgets();

	Region_tree_viewer_autoapply(region_tree_viewer->scene,
	  region_tree_viewer->edit_scene);
	Region_tree_viewer_renew_label_on_list(region_tree_viewer->current_graphic);
	return 1;
 }

/** Callback from wxChooser<Cmiss_element_point_sample_mode> when choice is made. */
int sample_mode_callback(enum Cmiss_element_point_sample_mode sample_mode)
{
	Cmiss_graphic_sampling_attributes_id sampling = Cmiss_graphic_get_sampling_attributes(region_tree_viewer->current_graphic);
	if (sampling)
	{
		Cmiss_graphic_sampling_attributes_set_mode(sampling, sample_mode);
	}
	Cmiss_graphic_sampling_attributes_destroy(&sampling);
	show_sampling_widgets();

	/* inform the client of the change */
	Region_tree_viewer_autoapply(region_tree_viewer->scene,
		region_tree_viewer->edit_scene);
	//Region_tree_viewer_renew_label_on_list(region_tree_viewer->current_graphic);

	return 1;
}

/** Callback from wxChooser<tessellation field> when choice is made.*/
int tessellation_field_callback(Computed_field *tessellation_field)
{
	Cmiss_graphic_set_tessellation_field(
		region_tree_viewer->current_graphic, tessellation_field);
	Region_tree_viewer_autoapply(region_tree_viewer->scene,
		region_tree_viewer->edit_scene);
	//Region_tree_viewer_renew_label_on_list(region_tree_viewer->current_graphic);
	return 1;
}

/** Callback from wxChooser<xi Point Denstiy Field> when choice is made. */
int sample_density_callback(Computed_field *sample_density_field)
{
	Cmiss_graphic_sampling_attributes_id sampling = Cmiss_graphic_get_sampling_attributes(region_tree_viewer->current_graphic);
	if (sampling)
	{
		Cmiss_graphic_sampling_attributes_set_density_field(sampling, sample_density_field);
	}
	Cmiss_graphic_sampling_attributes_destroy(&sampling);

	Region_tree_viewer_autoapply(region_tree_viewer->scene,
	 region_tree_viewer->edit_scene);
	//Region_tree_viewer_renew_label_on_list(region_tree_viewer->current_graphic);
	return 1;
}

int seed_element_callback(FE_element *seed_element)
/*******************************************************************************
LAST MODIFIED : 30 March 2007

DESCRIPTION :
Callback from wxChooser<Seed Element> when choice is made.
==============================================================================*/
{
	Cmiss_graphic_set_seed_element(region_tree_viewer->current_graphic,
		seed_element);
	Region_tree_viewer_autoapply(region_tree_viewer->scene,
		region_tree_viewer->edit_scene);
	//Region_tree_viewer_renew_label_on_list(region_tree_viewer->current_graphic);
	return 1;
}

/**
 * Callback from wxChooser<Cmiss_graphic_line_attributes_shape> when choice is made.
 */
int line_shape_callback(enum Cmiss_graphic_line_attributes_shape line_shape)
{
	Cmiss_graphic_line_attributes_id line_attributes =
		Cmiss_graphic_get_line_attributes(region_tree_viewer->current_graphic);
	if (CMISS_OK == Cmiss_graphic_line_attributes_set_shape(line_attributes, line_shape))
	{
		Region_tree_viewer_autoapply(region_tree_viewer->scene,
			region_tree_viewer->edit_scene);
		//Region_tree_viewer_renew_label_on_list(region_tree_viewer->current_graphic);
	}
	else
	{
		// restore current shape as some shapes not supported for all graphic types
		line_shape_chooser->set_value(Cmiss_graphic_line_attributes_get_shape(line_attributes));
	}
	Cmiss_graphic_line_attributes_destroy(&line_attributes);
	return 1;
}

void EnterLineBaseSize(wxCommandEvent &event)
{
	USE_PARAMETER(event);
	linebasesizetextctrl = XRCCTRL(*this, "LineBaseSizeTextCtrl", wxTextCtrl);
	wxString wxTextEntry = linebasesizetextctrl->GetValue();
	const char *text_entry = wxTextEntry.mb_str(wxConvUTF8);
	if (text_entry)
	{
		Cmiss_graphic_line_attributes_id line_attributes =
			Cmiss_graphic_get_line_attributes(region_tree_viewer->current_graphic);
		Parse_state *temp_state = create_Parse_state(text_entry);
		double lineBaseSize[2];
		if (set_double_product(temp_state, lineBaseSize, reinterpret_cast<void*>(2)))
		{
			Cmiss_graphic_line_attributes_set_base_size(line_attributes, 2, lineBaseSize);
			Region_tree_viewer_autoapply(region_tree_viewer->scene,
				region_tree_viewer->edit_scene);
			//Region_tree_viewer_renew_label_on_list(region_tree_viewer->current_graphic);
		}
		destroy_Parse_state(&temp_state);
		Cmiss_graphic_line_attributes_get_base_size(line_attributes, 2, lineBaseSize);
		char temp_string[100];
		sprintf(temp_string, "%g*%g", lineBaseSize[0], lineBaseSize[1]);
		linebasesizetextctrl->ChangeValue(wxString::FromAscii(temp_string));
		Cmiss_graphic_line_attributes_destroy(&line_attributes);
	}
	else
	{
		display_message(ERROR_MESSAGE, "EnterLineBaseSize.  Missing text");
	}
}

void EnterLineScaleFactors(wxCommandEvent &event)
{
	USE_PARAMETER(event);
	linescalefactorstextctrl = XRCCTRL(*this, "LineScaleFactorsTextCtrl", wxTextCtrl);
	wxString wxTextEntry = linescalefactorstextctrl->GetValue();
	const char *text_entry = wxTextEntry.mb_str(wxConvUTF8);
	if (text_entry)
	{
		Cmiss_graphic_line_attributes_id line_attributes =
			Cmiss_graphic_get_line_attributes(region_tree_viewer->current_graphic);
		Parse_state *temp_state = create_Parse_state(text_entry);
		double lineScaleFactors[2];
		if (set_double_product(temp_state, lineScaleFactors, reinterpret_cast<void*>(2)))
		{
			Cmiss_graphic_line_attributes_set_scale_factors(line_attributes, 2, lineScaleFactors);
			Region_tree_viewer_autoapply(region_tree_viewer->scene,
				region_tree_viewer->edit_scene);
			//Region_tree_viewer_renew_label_on_list(region_tree_viewer->current_graphic);
		}
		destroy_Parse_state(&temp_state);
		Cmiss_graphic_line_attributes_get_scale_factors(line_attributes, 2, lineScaleFactors);
		char temp_string[100];
		sprintf(temp_string, "%g*%g", lineScaleFactors[0], lineScaleFactors[1]);
		linescalefactorstextctrl->ChangeValue(wxString::FromAscii(temp_string));
		Cmiss_graphic_line_attributes_destroy(&line_attributes);
	}
	else
	{
		display_message(ERROR_MESSAGE, "EnterLineScaleFactors.  Missing text");
	}
}

/**
 * Callback from wxChooser<Stream Vector> when choice is made.
 */
int stream_vector_callback(Computed_field *stream_vector_field)
{
	Cmiss_graphic_streamlines_id streamlines = Cmiss_graphic_cast_streamlines(region_tree_viewer->current_graphic);
	Cmiss_graphic_streamlines_set_stream_vector_field(streamlines, stream_vector_field);
	Cmiss_graphic_streamlines_destroy(&streamlines);
	Region_tree_viewer_autoapply(region_tree_viewer->scene,
		region_tree_viewer->edit_scene);
	//Region_tree_viewer_renew_label_on_list(region_tree_viewer->current_graphic);
	return 1;
}

/**
 * Callback from wxChooser<Stream Data Type> when choice is made.
 */
int streamline_data_type_callback(enum Streamline_data_type streamline_data_type)
{
	enum Streamline_data_type old_streamline_data_type =
		Cmiss_graphic_get_streamline_data_type(region_tree_viewer->current_graphic);
	if (streamline_data_type != old_streamline_data_type)
	{
		Cmiss_graphic_set_streamline_data_type(region_tree_viewer->current_graphic, streamline_data_type);
		Cmiss_spectrum_id spectrum = 0;
		if (STREAM_NO_DATA != streamline_data_type)
		{
			spectrum = spectrum_chooser->get_object();
		}
		Cmiss_graphic_set_spectrum(region_tree_viewer->current_graphic, spectrum);
		data_chooser_panel=XRCCTRL(*this,"DataChooserPanel",wxPanel);
		spectrumtext=XRCCTRL(*this, "SpectrumText", wxStaticText);
		spectrum_chooser_panel=XRCCTRL(*this,"SpectrumChooserPanel", wxPanel);
		if (STREAM_NO_DATA != streamline_data_type)
		{
			spectrumtext->Enable();
			spectrum_chooser_panel->Enable();
		}
		else
		{
			spectrumtext->Disable();
			spectrum_chooser_panel->Disable();
		}
		if (STREAM_FIELD_SCALAR == streamline_data_type)
		{
			data_chooser_panel->Enable();
		}
		else
		{
			data_chooser_panel->Disable();
		}
		/* inform the client of the change */
		Region_tree_viewer_autoapply(region_tree_viewer->scene,
			region_tree_viewer->edit_scene);
		//Region_tree_viewer_renew_label_on_list(region_tree_viewer->current_graphic);
	}
	return 1;
}

/**
 * Callback from wxChooser<Data Field> when choice is made.
 */
int data_field_callback(Computed_field *data_field)
{
	Cmiss_graphic_set_data_field(region_tree_viewer->current_graphic, data_field);
	Cmiss_spectrum_id spectrum = 0;
	if (data_field)
	{
		spectrum = spectrum_chooser->get_object();
	}
	Cmiss_graphic_set_spectrum(region_tree_viewer->current_graphic, spectrum);
	spectrumtext=XRCCTRL(*this, "SpectrumText", wxStaticText);
	spectrum_chooser_panel=XRCCTRL(*this,"SpectrumChooserPanel", wxPanel);
	if (data_field)
	{
		spectrumtext->Enable();
		spectrum_chooser_panel->Enable();
	}
	else
	{
		spectrumtext->Disable();
		spectrum_chooser_panel->Disable();
	}

	Region_tree_viewer_autoapply(region_tree_viewer->scene,
		region_tree_viewer->edit_scene);
	//Region_tree_viewer_renew_label_on_list(region_tree_viewer->current_graphic);
	return 1;
}

/**
 * Callback from wxChooser<Spectrum> when choice is made.
 */
int spectrum_callback(Spectrum *spectrum)
{
	Cmiss_graphic_set_spectrum(region_tree_viewer->current_graphic, spectrum);
	Region_tree_viewer_autoapply(region_tree_viewer->scene,
		region_tree_viewer->edit_scene);
	//Region_tree_viewer_renew_label_on_list(region_tree_viewer->current_graphic);
	return 1;
}

/***************************************************************************//**
 * Callback from wxChooser<Cmiss_tessellation> when choice is made.
 */
int tessellation_callback(Cmiss_tessellation *tessellation)
{
	Cmiss_graphic_set_tessellation(
		region_tree_viewer->current_graphic, tessellation);
	Region_tree_viewer_autoapply(region_tree_viewer->scene,
		region_tree_viewer->edit_scene);
	//Region_tree_viewer_renew_label_on_list(region_tree_viewer->current_graphic);
	return 1;
}

int texture_coord_field_callback(Computed_field *texture_coord_field)
/*******************************************************************************
LAST MODIFIED : 26 March 2007

DESCRIPTION :
Callback from wxChooser<Texture Coord Field> when choice is made.
==============================================================================*/
{
	Cmiss_graphic_set_texture_coordinate_field(
		region_tree_viewer->current_graphic, texture_coord_field);
			/* inform the client of the change */
	Region_tree_viewer_autoapply(region_tree_viewer->scene,
				  region_tree_viewer->edit_scene);
	//Region_tree_viewer_renew_label_on_list(region_tree_viewer->current_graphic);
	return 1;
}

/** Callback from wxChooser<Render Type> when choice is made. */
int render_polygon_mode_callback(enum Cmiss_graphic_render_polygon_mode render_polygon_mode)
{
	Cmiss_graphic_set_render_polygon_mode(
		region_tree_viewer->current_graphic, render_polygon_mode);
	/* inform the client of the change */
	Region_tree_viewer_autoapply(region_tree_viewer->scene,
		region_tree_viewer->edit_scene);
	//Region_tree_viewer_renew_label_on_list(region_tree_viewer->current_graphic);
	return 1;
}

/***************************************************************************//**
*	Refresh the widgets upon collapsible pane change event.
* WXMSW only: Revert the size of the frame as this event will change the size
*             of the window somehow.
*/
void CollapsiblepaneChangedEvent(wxCollapsiblePaneEvent& event)
{
	USE_PARAMETER(event);
	frame=XRCCTRL(*this, "CmguiRegionTreeViewer", wxFrame);
	if (frame)
	{
		frame->Freeze();
		wxPanel *rightpanel=XRCCTRL(*this,"RightPanel", wxPanel);
		rightpanel->Layout();
		verticalsplitter=XRCCTRL(*this,"VerticalSplitter",wxSplitterWindow);
		verticalsplitter->Layout();
		lowersplitter=XRCCTRL(*this,"LowerSplitter",wxSplitterWindow);
		lowersplitter->Layout();
		frame->SetMinSize(wxSize(50,100));
		frame->SetMaxSize(wxSize(2000,2000));
		frame->Layout();
#if defined (__WXMSW__)
		frame->SetSize(region_tree_viewer_size.previous_width, region_tree_viewer_size.previous_height);
		frame->GetSize(&region_tree_viewer_size.current_width, &region_tree_viewer_size.current_height);
#endif /* defined (__WXMSW__) */
		frame->Layout();
		frame->Thaw();
	}
}

#if defined (__WXMSW__)
/***************************************************************************//**
* Get the size of the frame in case the collapsible panes change the size
* of the frame.
*
*/
void FrameGetSize(wxSizeEvent &event)
{
	frame=XRCCTRL(*this, "CmguiRegionTreeViewer", wxFrame);
	if (frame)
	{
		region_tree_viewer_size.previous_width = region_tree_viewer_size.current_width;
		region_tree_viewer_size.previous_height = region_tree_viewer_size.current_height;
		frame->GetSize(&region_tree_viewer_size.current_width, &region_tree_viewer_size.current_height);
		event.Skip();
	}
}
#endif /* defined (__WXMSW__) */


/***************************************************************************//**
 *Check if the auto apply clicked or not, if clicked, apply the current changes
 */
void Region_tree_viewer_autoapply(Cmiss_scene *destination, Cmiss_scene *source)
{
	if(region_tree_viewer->auto_apply)
	{
		if (region_tree_viewer->scene_callback_flag)
		{
			if (Cmiss_scene_remove_callback(region_tree_viewer->scene,
					Region_tree_viewer_wx_scene_change, (void *)region_tree_viewer))
			{
				region_tree_viewer->scene_callback_flag = 0;
			}
		}
		if (!Cmiss_scene_modify(destination,source))
		{
			display_message(ERROR_MESSAGE, "wxRegionTreeViewer::Region_tree_viewer_autoapply"
				"Could not modify scene");
		}
		if (Cmiss_scene_add_callback(region_tree_viewer->scene,
				Region_tree_viewer_wx_scene_change, (void *)region_tree_viewer))
		{
			region_tree_viewer->scene_callback_flag = 1;
		}
	}
	else
	{
		applybutton->Enable();
		revertbutton->Enable();
	}
}

void Region_tree_viewer_wx_set_list_string(Cmiss_graphic *graphic)
{
	unsigned int selection;
	char *graphic_string;
	graphicalitemschecklist=XRCCTRL(*this,"CmissGraphicListBox",wxCheckListBox);
	selection=graphicalitemschecklist->GetSelection();
	bool check = graphicalitemschecklist->IsChecked(selection);
	graphic_string = Cmiss_graphic_get_summary_string(graphic);
	graphicalitemschecklist->SetString(selection, wxString::FromAscii(graphic_string));
	graphicalitemschecklist->Check(selection,check);
	DEALLOCATE(graphic_string);
}

/***************************************************************************//**
 * When changes have been made by the user, renew the label on the list
 */
void Region_tree_viewer_renew_label_on_list(Cmiss_graphic *graphic)
{
	graphicalitemschecklist=XRCCTRL(*this,"CmissGraphicListBox",wxCheckListBox);
	int position = Cmiss_scene_get_graphic_position(
		region_tree_viewer->edit_scene, graphic);
	bool check = graphicalitemschecklist->IsChecked(position - 1);
	char *graphic_string = Cmiss_graphic_get_summary_string(region_tree_viewer->current_graphic);
	graphicalitemschecklist->SetString(position-1, wxString::FromAscii(graphic_string));
	graphicalitemschecklist->Check(position-1, check);
	DEALLOCATE(graphic_string);
}

void ResetWindow(wxSplitterEvent& event)
{
	USE_PARAMETER(event);
	frame =
			XRCCTRL(*this, "CmguiRegionTreeViewer", wxFrame);
	frame->Layout();
	frame->SetMinSize(wxSize(50,100));
	verticalsplitter=XRCCTRL(*this,"VerticalSplitter",wxSplitterWindow);
	verticalsplitter->Layout();
	lowersplitter=XRCCTRL(*this,"LowerSplitter",wxSplitterWindow);
	lowersplitter->Layout();
	sceneediting =
		XRCCTRL(*this, "SceneEditing", wxScrolledWindow);
	sceneediting->Layout();
	sceneediting->SetScrollbars(10,10,40,40);
}

	void AutoChecked(wxCommandEvent &event)
	{
	USE_PARAMETER(event);
		autocheckbox = XRCCTRL(*this, "AutoCheckBox", wxCheckBox);
		applybutton = XRCCTRL(*this, "ApplyButton", wxButton);
		revertbutton = XRCCTRL(*this,"RevertButton", wxButton);
		if(autocheckbox->IsChecked())
		{
			applybutton->Disable();
			revertbutton->Disable();
			region_tree_viewer->auto_apply = 1;
			Region_tree_viewer_autoapply(region_tree_viewer->scene,
				region_tree_viewer->edit_scene);
		}
		else
		{
			applybutton->Disable();
			revertbutton->Disable();
			region_tree_viewer->auto_apply = 0;
		}
	}

void RevertClicked(wxCommandEvent &event)
{
	USE_PARAMETER(event);
	Region_tree_viewer_revert_changes(region_tree_viewer);
}

void ApplyClicked(wxCommandEvent &event)
{
	USE_PARAMETER(event);
	if (region_tree_viewer->scene_callback_flag)
	{
		if (Cmiss_scene_remove_callback(region_tree_viewer->scene,
				Region_tree_viewer_wx_scene_change, (void *)region_tree_viewer))
		{
			region_tree_viewer->scene_callback_flag = 0;
		}
	}
	if (region_tree_viewer->transformation_editor)
	{
		region_tree_viewer->transformation_editor->ApplyTransformation(/*force_apply*/1);
	}
	if (!Cmiss_scene_modify(region_tree_viewer->scene,
			region_tree_viewer->edit_scene))
	{
		display_message(ERROR_MESSAGE, "wxRegionTreeViewer::ApplyClicked.  "
			"Could not modify scene");
	}
	if (Cmiss_scene_add_callback(region_tree_viewer->scene,
			Region_tree_viewer_wx_scene_change, (void *)region_tree_viewer))
	{
		region_tree_viewer->scene_callback_flag = 1;
	}
}

void Region_tree_viewer_wx_update_current_graphic(Cmiss_graphic *graphic)
{
	wxScrolledWindow *sceneeditingpanel= XRCCTRL(*this, "SceneEditing",wxScrolledWindow);
	sceneeditingpanel->Enable();
	sceneeditingpanel->Show();
	graphicalitemschecklist=XRCCTRL(*this,"CmissGraphicListBox",wxCheckListBox);
	REACCESS(Cmiss_graphic)(&region_tree_viewer->current_graphic, graphic);
}

void Region_tree_viewer_wx_update_graphic_widgets()
{
	wxScrolledWindow *sceneeditingpanel = XRCCTRL(*this, "SceneEditing",wxScrolledWindow);
	sceneeditingpanel->Freeze();
	get_and_set_Cmiss_graphic_widgets((void *)region_tree_viewer);
	sceneeditingpanel->Thaw();
	sceneeditingpanel->Layout();
	if (region_tree_viewer->lowersplitter)
	{
			int width, height;
			region_tree_viewer->lowersplitter->GetSize(&width, &height);
			region_tree_viewer->lowersplitter->SetSize(width-1, height-1);
				region_tree_viewer->lowersplitter->SetSize(width+1, height+1);
	}
}

void GraphicListBoxProcessSelection(int selection)
{
	if (-1 != selection)
	{
		Cmiss_graphic *temp_graphic = Cmiss_scene_get_graphic_at_position(
			region_tree_viewer->edit_scene, selection+1);
		Region_tree_viewer_wx_update_current_graphic(temp_graphic);
		Region_tree_viewer_wx_update_graphic_widgets();
		if (temp_graphic)
		{
			Cmiss_graphic_destroy(&temp_graphic);
		}
	}
}

void GraphicListBoxChecked(wxCommandEvent &event)
{
	int selection = event.GetInt();
	graphicalitemschecklist->SetSelection(selection);
	GraphicListBoxProcessSelection(selection);
	Cmiss_graphic *temp_graphic = Cmiss_scene_get_graphic_at_position(
		region_tree_viewer->edit_scene, selection+1);
	Cmiss_graphic_set_visibility_flag(temp_graphic,
		graphicalitemschecklist->IsChecked(selection));
	Region_tree_viewer_autoapply(region_tree_viewer->scene,
		region_tree_viewer->edit_scene);
	Cmiss_graphic_destroy(&temp_graphic);
}

void GraphicListBoxClicked(wxCommandEvent &event)
{
	GraphicListBoxProcessSelection(event.GetInt());
}

/**
 * @param domain_type  If not CMISS_FIELD_DOMAIN_TYPE_INVALID, set for
 * new graphic and also set coordinate field.
 */
void AddGraphic(Cmiss_graphic_type graphic_type,
	Cmiss_field_domain_type domain_type, Cmiss_graphic *graphic_to_copy)
{
	Cmiss_graphic *graphic = Cmiss_scene_create_graphic_app(
		region_tree_viewer->edit_scene, graphic_type, graphic_to_copy);
	if (graphic)
	{
		// make sure new graphic is visible (if copy)
		Cmiss_graphic_set_visibility_flag(graphic, true);
		// set domain and initial coordinate field if appropriate
		if (domain_type != CMISS_FIELD_DOMAIN_TYPE_INVALID)
		{
			Cmiss_graphic_set_domain_type(graphic, domain_type);
			Cmiss_field_id coordinate_field = Cmiss_scene_guess_coordinate_field(
				region_tree_viewer->edit_scene, domain_type);
			if (coordinate_field)
				Cmiss_graphic_set_coordinate_field(graphic, coordinate_field);
		}
		Cmiss_scene_add_graphic(region_tree_viewer->edit_scene, graphic, 0);
		//Update the list of graphic
		wxCheckListBox *graphicalitemschecklist =  XRCCTRL(*this, "CmissGraphicListBox",wxCheckListBox);
		graphicalitemschecklist->SetSelection(wxNOT_FOUND);
		graphicalitemschecklist->Clear();
		for_each_graphic_in_Cmiss_scene(region_tree_viewer->edit_scene,
			Region_tree_viewer_add_graphic_item, (void *)region_tree_viewer);
		graphicalitemschecklist->SetSelection((graphicalitemschecklist->GetCount()-1));
		sceneediting =
			XRCCTRL(*this, "SceneEditing", wxScrolledWindow);
		sceneediting->Freeze();
		Cmiss_graphic *temp_graphic = Cmiss_scene_get_graphic_at_position(
			region_tree_viewer->edit_scene, graphicalitemschecklist->GetCount());
		Region_tree_viewer_wx_update_current_graphic(temp_graphic);
		Cmiss_graphic_destroy(&temp_graphic);
		Region_tree_viewer_wx_update_graphic_widgets();
		Region_tree_viewer_autoapply(region_tree_viewer->scene,
			region_tree_viewer->edit_scene);
		sceneediting->Thaw();
		sceneediting->Layout();
		DEACCESS(Cmiss_graphic)(&graphic);
	}
}

void AddGraphicChoice(wxCommandEvent &event)
{
	wxString wxChoiceLabel = event.GetString();
	const char *choiceLabel = wxChoiceLabel.mb_str(wxConvUTF8);
	size_t tmp = reinterpret_cast<size_t>(event.GetClientData());
	Add_graphic_type add_graphic_type = static_cast<Add_graphic_type>(tmp);
	if (Add_graphic_type_string(add_graphic_type))
	{
		Cmiss_graphic_type graphic_type = CMISS_GRAPHIC_TYPE_INVALID;
		Cmiss_field_domain_type domain_type = CMISS_FIELD_DOMAIN_TYPE_INVALID;
		switch (add_graphic_type)
		{
		case ADD_GRAPHIC_TYPE_POINTS:
			graphic_type = CMISS_GRAPHIC_POINTS;
			break;
		case ADD_GRAPHIC_TYPE_NODE_POINTS:
			domain_type = CMISS_FIELD_DOMAIN_NODES;
			graphic_type = CMISS_GRAPHIC_POINTS;
			break;
		case ADD_GRAPHIC_TYPE_DATA_POINTS:
			domain_type = CMISS_FIELD_DOMAIN_DATA;
			graphic_type = CMISS_GRAPHIC_POINTS;
			break;
		case ADD_GRAPHIC_TYPE_ELEMENT_POINTS:
			domain_type = CMISS_FIELD_DOMAIN_ELEMENTS_HIGHEST_DIMENSION;
			graphic_type = CMISS_GRAPHIC_POINTS;
			break;
		case ADD_GRAPHIC_TYPE_LINES:
			domain_type = CMISS_FIELD_DOMAIN_ELEMENTS_1D;
			graphic_type = CMISS_GRAPHIC_LINES;
			break;
		case ADD_GRAPHIC_TYPE_SURFACES:
			domain_type = CMISS_FIELD_DOMAIN_ELEMENTS_2D;
			graphic_type = CMISS_GRAPHIC_SURFACES;
			break;
		case ADD_GRAPHIC_TYPE_CONTOURS:
			graphic_type = CMISS_GRAPHIC_CONTOURS;
			domain_type = CMISS_FIELD_DOMAIN_ELEMENTS_3D;
			break;
		case ADD_GRAPHIC_TYPE_STREAMLINES:
			domain_type = CMISS_FIELD_DOMAIN_ELEMENTS_HIGHEST_DIMENSION;
			graphic_type = CMISS_GRAPHIC_STREAMLINES;
			break;
		case ADD_GRAPHIC_TYPE_INVALID:
			break;
		}
		AddGraphic(graphic_type, domain_type, (Cmiss_graphic *)0);
	}
	add_graphic_choice->SetSelection(0);
}

void CopyGraphic(wxCommandEvent &event)
{
	USE_PARAMETER(event);
	if (region_tree_viewer->current_graphic)
	{
		AddGraphic(Cmiss_graphic_get_graphic_type(region_tree_viewer->current_graphic),
		 CMISS_FIELD_DOMAIN_TYPE_INVALID, region_tree_viewer->current_graphic);
	}
}

void RemoveFromGraphicList(wxCommandEvent &event)
{
	unsigned int position;

	USE_PARAMETER(event);

	if (region_tree_viewer->edit_scene && region_tree_viewer->current_graphic)
	{
		graphicalitemschecklist=XRCCTRL(*this,"CmissGraphicListBox",wxCheckListBox);
		position = Cmiss_scene_get_graphic_position(
			region_tree_viewer->edit_scene, region_tree_viewer->current_graphic);
		Cmiss_scene_remove_graphic(
			region_tree_viewer->edit_scene, region_tree_viewer->current_graphic);
		graphicalitemschecklist->Clear();
		for_each_graphic_in_Cmiss_scene(region_tree_viewer->edit_scene,
			Region_tree_viewer_add_graphic_item, (void *)region_tree_viewer);
		if (graphicalitemschecklist->GetCount() < position)
		{
			--position;
		}
		Cmiss_graphic *temp_graphic = Cmiss_scene_get_graphic_at_position(
			region_tree_viewer->edit_scene, position);
		Region_tree_viewer_wx_update_current_graphic(temp_graphic);
		Region_tree_viewer_wx_update_graphic_widgets();
		Region_tree_viewer_autoapply(region_tree_viewer->scene,
			region_tree_viewer->edit_scene);
		if (temp_graphic)
		{
			graphicalitemschecklist->SetSelection(position-1);
			Cmiss_graphic_destroy(&temp_graphic);
		}
		else
		{
			graphicalitemschecklist->SetSelection(wxNOT_FOUND);
			graphicalitemschecklist->Clear();
			wxScrolledWindow *sceneeditingpanel= XRCCTRL(*this, "SceneEditing",wxScrolledWindow);
			sceneeditingpanel->Disable();
			sceneeditingpanel->Hide();
		}
	}
}

void MoveUpInGraphicList(wxCommandEvent &event)
{
	USE_PARAMETER(event);
	int position;
	Cmiss_graphic *graphic;
	if (region_tree_viewer->edit_scene)
	{
		if (1 < (position = Cmiss_scene_get_graphic_position(
								region_tree_viewer->edit_scene, region_tree_viewer->current_graphic)))
		{
			graphic = region_tree_viewer->current_graphic;
			ACCESS(Cmiss_graphic)(graphic);
			Cmiss_scene_remove_graphic(region_tree_viewer->edit_scene,
				region_tree_viewer->current_graphic);
			Cmiss_scene_add_graphic(region_tree_viewer->edit_scene,
				region_tree_viewer->current_graphic, position - 1);
			DEACCESS(Cmiss_graphic)(&graphic);
			graphicalitemschecklist=XRCCTRL(*this,"CmissGraphicListBox",wxCheckListBox);
			graphicalitemschecklist->SetSelection(wxNOT_FOUND);
			graphicalitemschecklist->Clear();
			for_each_graphic_in_Cmiss_scene(region_tree_viewer->edit_scene,
				Region_tree_viewer_add_graphic_item, (void *)region_tree_viewer);
			graphicalitemschecklist->SetSelection(position-2);
			Cmiss_graphic *temp_graphic = Cmiss_scene_get_graphic_at_position(
				region_tree_viewer->edit_scene, position-1);
			Region_tree_viewer_wx_update_current_graphic(temp_graphic);
			Region_tree_viewer_wx_update_graphic_widgets();
			Cmiss_graphic_destroy(&temp_graphic);
			Region_tree_viewer_autoapply(region_tree_viewer->scene,
				region_tree_viewer->edit_scene);
			/* By default the graphic name is the position, so it needs to be updated
					even though the graphic hasn't actually changed */
			/* inform the client of the change */
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"MoveUpInGraphicList.  Invalid argument(s)");
	}
}

	void MoveDownInGraphicList(wxCommandEvent &event)
	{

		int position;
		Cmiss_graphic *graphic;

	 USE_PARAMETER(event);

		if 	(region_tree_viewer->edit_scene)
		{
			if (Cmiss_scene_get_number_of_graphics(
						region_tree_viewer->edit_scene) >
				(position = Cmiss_scene_get_graphic_position(
					region_tree_viewer->edit_scene, region_tree_viewer->current_graphic)))
			{
				graphic = region_tree_viewer->current_graphic;
				ACCESS(Cmiss_graphic)(graphic);
				Cmiss_scene_remove_graphic(region_tree_viewer->edit_scene,
					region_tree_viewer->current_graphic);
				Cmiss_scene_add_graphic(region_tree_viewer->edit_scene,
					region_tree_viewer->current_graphic, position + 1);
				DEACCESS(Cmiss_graphic)(&graphic);
				graphicalitemschecklist=XRCCTRL(*this,"CmissGraphicListBox",wxCheckListBox);
				graphicalitemschecklist->SetSelection(wxNOT_FOUND);
				graphicalitemschecklist->Clear();
				for_each_graphic_in_Cmiss_scene(region_tree_viewer->edit_scene,
					Region_tree_viewer_add_graphic_item, (void *)region_tree_viewer);
				graphicalitemschecklist->SetSelection(position);
				Cmiss_graphic *temp_graphic = Cmiss_scene_get_graphic_at_position(
					region_tree_viewer->edit_scene, position+1);
				Region_tree_viewer_wx_update_current_graphic(temp_graphic);
				Region_tree_viewer_wx_update_graphic_widgets();
				Cmiss_graphic_destroy(&temp_graphic);
				Region_tree_viewer_autoapply(region_tree_viewer->scene,
					region_tree_viewer->edit_scene);
				/* By default the graphic name is the position, so it needs to be updated
					even though the graphic hasn't actually changed */
				/* inform the client of the change */
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"MoveDownInGraphicList.  Invalid argument(s)");
		}
	}

	void GraphicEditorNameText(wxCommandEvent &event)
	{
		USE_PARAMETER(event);
		if (region_tree_viewer->current_graphic)
		{
			graphicalitemschecklist=XRCCTRL(*this,"CmissGraphicListBox",wxCheckListBox);
			char *name = Cmiss_graphic_get_name_internal(region_tree_viewer->current_graphic);
			nametextfield = XRCCTRL(*this, "NameTextField", wxTextCtrl);
			wxString wxTextEntry = nametextfield->GetValue();
			const char *text_entry = wxTextEntry.mb_str(wxConvUTF8);
			if (text_entry)
			{
				char *new_name = duplicate_string(text_entry);
				sscanf(text_entry, "%s", new_name);
				if (strcmp(name, new_name))
				{
					Cmiss_graphic_set_name(
						region_tree_viewer->current_graphic, new_name);
						/* inform the client of the change */
					nametextfield->SetValue(wxString::FromAscii(new_name));
					Region_tree_viewer_autoapply(region_tree_viewer->scene,
						region_tree_viewer->edit_scene);
					Region_tree_viewer_renew_label_on_list(region_tree_viewer->current_graphic);
				}
				DEALLOCATE(new_name);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"GraphicEditorNameText.  Missing text");
			}
			DEALLOCATE(name);
		}
		else
		{
			display_message(ERROR_MESSAGE,
			 "GraphicEditorNameText.  Invalid argument(s)");
		}
	}

void EnterIsoScalar(wxCommandEvent &event)
{
	USE_PARAMETER(event);
	if (!(region_tree_viewer && region_tree_viewer->current_graphic))
		return;

	Cmiss_graphic_contours_id contours =
		Cmiss_graphic_cast_contours(region_tree_viewer->current_graphic);

	isovaluelistradiobutton=XRCCTRL(*this,"IsoValueListRadioButton",wxRadioButton);
	isoscalartextctrl=XRCCTRL(*this,"IsoScalarTextCtrl",wxTextCtrl);

	if (isovaluelistradiobutton->GetValue())
	{
		isoscalartextctrl->Enable();
		isovaluesequencenumbertextctrl->Disable();
		isovaluesequencefirsttextctrl->Disable();
		isovaluesequencelasttextctrl->Disable();

		wxString wxTextEntry = isoscalartextctrl->GetValue();
		const char *text_entry = wxTextEntry.mb_str(wxConvUTF8);
		if (text_entry)
		{
			double isovalue;
			std::vector<double> isovalues;
			int valid_value = 1;
			int offset = 0;
			int length = 0;
			while (valid_value)
			{
				if (0 < sscanf(text_entry + offset, "%lg%n", &isovalue, &length))
				{
					offset += length;
					isovalues.push_back(isovalue);
				}
				else
				{
					valid_value = 0;
				}
			}
			Cmiss_graphic_contours_set_list_isovalues(contours, static_cast<int>(isovalues.size()), isovalues.data());
			Region_tree_viewer_autoapply(region_tree_viewer->scene,
				region_tree_viewer->edit_scene);
			//Region_tree_viewer_renew_label_on_list(region_tree_viewer->current_graphic);		/* inform the client of the change */
		}
		else
		{
			display_message(ERROR_MESSAGE, "EnterIsoScalar.  Missing text");
		}

		// always redisplay current values
		int number_of_isovalues = Cmiss_graphic_contours_get_list_isovalues(contours, 0, 0);
		if (number_of_isovalues)
		{
			char *vector_temp_string = 0;
			char temp_string[50];
			double *isovalues = new double[number_of_isovalues];
			Cmiss_graphic_contours_get_list_isovalues(contours, number_of_isovalues, isovalues);
			int error = 0;
			for (int i = 0 ; !error && (i < number_of_isovalues) ; i++)
			{
				sprintf(temp_string, "%g ", isovalues[i]);
				append_string(&vector_temp_string, temp_string, &error);
			}
			if (vector_temp_string)
			{
				isoscalartextctrl->SetValue(wxString::FromAscii(vector_temp_string));
				DEALLOCATE(vector_temp_string);
			}
			delete[] isovalues;
		}
		else
		{
			isoscalartextctrl->SetValue(wxString());
		}
	}
	Cmiss_graphic_contours_destroy(&contours);
}

void EnterIsoRange(wxCommandEvent &event)
{
	USE_PARAMETER(event);
	if (!(region_tree_viewer && region_tree_viewer->current_graphic))
		return;

	Cmiss_graphic_contours_id contours =
		Cmiss_graphic_cast_contours(region_tree_viewer->current_graphic);

	isovaluesequenceradiobutton=XRCCTRL(*this,"IsoValueSequenceRadioButton",wxRadioButton);
	isovaluesequencenumbertextctrl=XRCCTRL(*this,"IsoValueSequenceNumberTextCtrl",wxTextCtrl);
	isovaluesequencefirsttextctrl=XRCCTRL(*this,"IsoValueSequenceFirstTextCtrl",wxTextCtrl);
	isovaluesequencelasttextctrl=XRCCTRL(*this,"IsoValueSequenceLastTextCtrl",wxTextCtrl);

	if (isovaluesequenceradiobutton->GetValue())
	{
		int number_of_isovalues = 0;
		double first_isovalue = 0.0;
		double last_isovalue = 0.0;

		isoscalartextctrl->Disable();
		isovaluesequencenumbertextctrl->Enable();
		isovaluesequencefirsttextctrl->Enable();
		isovaluesequencelasttextctrl->Enable();
		wxString wxTextEntry = isovaluesequencenumbertextctrl->GetValue();
		const char *text_entry = wxTextEntry.mb_str(wxConvUTF8);
		if (text_entry)
		{
			sscanf(text_entry, "%d", &number_of_isovalues);
		}
		else
		{
			display_message(ERROR_MESSAGE, "EnterIsoRange.  Missing text");
		}

		wxTextEntry = isovaluesequencefirsttextctrl->GetValue();
		text_entry = wxTextEntry.mb_str(wxConvUTF8);
		if (text_entry)
		{
			sscanf(text_entry, "%lg", &first_isovalue);
		}
		else
		{
			display_message(ERROR_MESSAGE, "EnterIsoRange.  Missing text");
		}

		wxTextEntry = isovaluesequencelasttextctrl->GetValue();
		text_entry = wxTextEntry.mb_str(wxConvUTF8);
		if (text_entry)
		{
			sscanf(text_entry, "%lg", &last_isovalue);
		}
		else
		{
			display_message(ERROR_MESSAGE, "EnterIsoRange.  Missing text");
		}

		Cmiss_graphic_contours_set_range_isovalues(contours,
			number_of_isovalues, first_isovalue, last_isovalue);

		/* always restore strings to actual value in use */
		char temp_string[50];
		sprintf(temp_string, "%d", number_of_isovalues);
		isovaluesequencenumbertextctrl->SetValue(wxString::FromAscii(temp_string));
		sprintf(temp_string, "%g", first_isovalue);
		isovaluesequencefirsttextctrl->SetValue(wxString::FromAscii(temp_string));
		sprintf(temp_string, "%g", last_isovalue);
		isovaluesequencelasttextctrl->SetValue(wxString::FromAscii(temp_string));

		/* inform the client of the change */
		Region_tree_viewer_autoapply(region_tree_viewer->scene,
			region_tree_viewer->edit_scene);
		//Region_tree_viewer_renew_label_on_list(region_tree_viewer->current_graphic);
	}
	Cmiss_graphic_contours_destroy(&contours);
}

void EnterGlyphOffset(wxCommandEvent &event)
{
	USE_PARAMETER(event);
	offsettextctrl=XRCCTRL(*this,"OffsetTextCtrl",wxTextCtrl);
	wxString wxTextEntry = offsettextctrl->GetValue();
	const char *text_entry = wxTextEntry.mb_str(wxConvUTF8);
	if (text_entry)
	{
		Cmiss_graphic_point_attributes_id point_attributes =
			Cmiss_graphic_get_point_attributes(region_tree_viewer->current_graphic);
		Parse_state *temp_state = create_Parse_state(text_entry);
		const int number_of_components = 3;
		double glyph_offset[3] = { 0.0, 0.0, 0.0 };
		set_double_vector(temp_state, glyph_offset, (void *)&number_of_components);
		Cmiss_graphic_point_attributes_set_glyph_offset(point_attributes, number_of_components, glyph_offset);
		Region_tree_viewer_autoapply(region_tree_viewer->scene,
			region_tree_viewer->edit_scene);
		//Region_tree_viewer_renew_label_on_list(region_tree_viewer->current_graphic);
		destroy_Parse_state(&temp_state);
		Cmiss_graphic_point_attributes_get_glyph_offset(point_attributes, number_of_components, glyph_offset);
		char temp_string[100];
		sprintf(temp_string, "%g,%g,%g", glyph_offset[0], glyph_offset[1], glyph_offset[2]);
		offsettextctrl->ChangeValue(wxString::FromAscii(temp_string));
		Cmiss_graphic_point_attributes_destroy(&point_attributes);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"settings_editor_glyph_offset_text_CB.  Missing text");
	}
}

void EnterLabelOffset(wxCommandEvent &event)
{
	USE_PARAMETER(event);
	labeloffsettextctrl=XRCCTRL(*this,"LabelOffsetTextCtrl",wxTextCtrl);
	wxString wxTextEntry = labeloffsettextctrl->GetValue();
	const char *text_entry = wxTextEntry.mb_str(wxConvUTF8);
	if (text_entry)
	{
		Cmiss_graphic_point_attributes_id point_attributes =
			Cmiss_graphic_get_point_attributes(region_tree_viewer->current_graphic);
		Parse_state *temp_state = create_Parse_state(text_entry);
		const int number_of_components = 3;
		double label_offset[3] = { 0.0, 0.0, 0.0 };
		set_double_vector(temp_state, label_offset, (void *)&number_of_components);
		Cmiss_graphic_point_attributes_set_label_offset(point_attributes, number_of_components, label_offset);
		Region_tree_viewer_autoapply(region_tree_viewer->scene,
			region_tree_viewer->edit_scene);
		//Region_tree_viewer_renew_label_on_list(region_tree_viewer->current_graphic);
		destroy_Parse_state(&temp_state);
		Cmiss_graphic_point_attributes_get_label_offset(point_attributes, number_of_components, label_offset);
		char temp_string[100];
		sprintf(temp_string, "%g,%g,%g", label_offset[0], label_offset[1], label_offset[2]);
		labeloffsettextctrl->ChangeValue(wxString::FromAscii(temp_string));
		Cmiss_graphic_point_attributes_destroy(&point_attributes);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"settings_editor_glyph_offset_text_CB.  Missing text");
	}
}

void EnterStaticLabelText(wxCommandEvent &event)
{
	staticlabeltextctrl[0]=XRCCTRL(*this,"StaticLabelTextCtrl1",wxTextCtrl);
	staticlabeltextctrl[1]=XRCCTRL(*this,"StaticLabelTextCtrl2",wxTextCtrl);
	staticlabeltextctrl[2]=XRCCTRL(*this,"StaticLabelTextCtrl3",wxTextCtrl);
	wxObject *object = event.GetEventObject();
	int i = 0;
	for (i = 0; i < 3; ++i)
	{
		if (object == static_cast<wxObject*>(staticlabeltextctrl[i]))
			break;
	}
	wxString wxTextEntry = staticlabeltextctrl[i]->GetValue();
	const char *text_entry = wxTextEntry.mb_str(wxConvUTF8);
	if (text_entry)
	{
		Cmiss_graphic_point_attributes_id point_attributes =
			Cmiss_graphic_get_point_attributes(region_tree_viewer->current_graphic);
		if (0 == strlen(text_entry))
		{
			text_entry = 0;
		}
		Cmiss_graphic_point_attributes_set_label_text(point_attributes, i + 1, text_entry);
		Region_tree_viewer_autoapply(region_tree_viewer->scene,
			region_tree_viewer->edit_scene);
		//Region_tree_viewer_renew_label_on_list(region_tree_viewer->current_graphic);
		Cmiss_graphic_point_attributes_destroy(&point_attributes);
	}
}

void EnterGlyphSize(wxCommandEvent &event)
{
	USE_PARAMETER(event);
	baseglyphsizetextctrl=XRCCTRL(*this,"BaseGlyphSizeTextCtrl",wxTextCtrl);
	wxString wxTextEntry = baseglyphsizetextctrl->GetValue();
	const char *text_entry = wxTextEntry.mb_str(wxConvUTF8);
	if (text_entry)
	{
		Cmiss_graphic_point_attributes_id point_attributes =
			Cmiss_graphic_get_point_attributes(region_tree_viewer->current_graphic);
		Parse_state *temp_state = create_Parse_state(text_entry);
		const int number_of_components = 3;
		double point_base_size[3];
		if (set_double_product(temp_state, point_base_size, reinterpret_cast<void *>(3)))
		{
			Cmiss_graphic_point_attributes_set_base_size(point_attributes, number_of_components, point_base_size);
			Region_tree_viewer_autoapply(region_tree_viewer->scene,
				region_tree_viewer->edit_scene);
			//Region_tree_viewer_renew_label_on_list(region_tree_viewer->current_graphic);
		}
		destroy_Parse_state(&temp_state);
		Cmiss_graphic_point_attributes_get_base_size(point_attributes, number_of_components, point_base_size);
		char temp_string[100];
		sprintf(temp_string, "%g*%g*%g", point_base_size[0], point_base_size[1], point_base_size[2]);
		baseglyphsizetextctrl->ChangeValue(wxString::FromAscii(temp_string));
		Cmiss_graphic_point_attributes_destroy(&point_attributes);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"settings_editor_glyph_size_text_CB.  Missing text");
	}
}

void EnterGlyphScale(wxCommandEvent &event)
{
	USE_PARAMETER(event);
	glyphscalefactorstextctrl=XRCCTRL(*this,"GlyphScaleFactorsTextCtrl",wxTextCtrl);
	wxString wxTextEntry = glyphscalefactorstextctrl->GetValue();
	const char *text_entry = wxTextEntry.mb_str(wxConvUTF8);
	if (text_entry)
	{
		Cmiss_graphic_point_attributes_id point_attributes =
			Cmiss_graphic_get_point_attributes(region_tree_viewer->current_graphic);
		Parse_state *temp_state = create_Parse_state(text_entry);
		const int number_of_components = 3;
		double point_scale_factors[3];
		if (set_double_product(temp_state, point_scale_factors, reinterpret_cast<void *>(3)))
		{
			Cmiss_graphic_point_attributes_set_scale_factors(point_attributes, number_of_components, point_scale_factors);
			Region_tree_viewer_autoapply(region_tree_viewer->scene,
				region_tree_viewer->edit_scene);
			//Region_tree_viewer_renew_label_on_list(region_tree_viewer->current_graphic);
		}
		destroy_Parse_state(&temp_state);
		Cmiss_graphic_point_attributes_get_scale_factors(point_attributes, number_of_components, point_scale_factors);
		char temp_string[100];
		sprintf(temp_string, "%g*%g*%g", point_scale_factors[0], point_scale_factors[1], point_scale_factors[2]);
		glyphscalefactorstextctrl->ChangeValue(wxString::FromAscii(temp_string));
		Cmiss_graphic_point_attributes_destroy(&point_attributes);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"settings_editor_glyph_scale_factors_text_CB.  Missing text");
	}
}

	/*Overlay disabled
   void OverlayChecked(wxCommandEvent &event)
	{
		wxTextCtrl *overlay_textctrl = XRCCTRL(*this, "OverlayTextCtrl", wxTextCtrl);
		char temp_string[50];
		Cmiss_graphic_enable_overlay(region_tree_viewer->current_graphic,event.IsChecked());
		Region_tree_viewer_autoapply(region_tree_viewer->scene,
			region_tree_viewer->edit_scene);
		if (event.IsChecked())
		{
			overlay_textctrl->Enable();
			sprintf(temp_string,"%d ",Cmiss_graphic_get_overlay_order(
								region_tree_viewer->current_graphic));
			overlay_textctrl->SetValue(temp_string);
		}
		else
		{
			overlay_textctrl->Disable();
		}
	}

  void OverlayEntered(wxCommandEvent &event)
  {
		wxTextCtrl *overlay_textctrl = XRCCTRL(*this, "OverlayTextCtrl", wxTextCtrl);
		int new_overlay_order;

		USE_PARAMETER(event);
		const char *text_entry=const_cast<char *>(overlay_textctrl->GetValue().mb_str(wxConvUTF8));
		sscanf(text_entry,"%d",&new_overlay_order);
		Cmiss_graphic_set_overlay_order(region_tree_viewer->current_graphic, new_overlay_order);
		Region_tree_viewer_autoapply(region_tree_viewer->scene,
			region_tree_viewer->edit_scene);
	}
*/

void SeedElementChecked(wxCommandEvent &event)
{
	seed_element_panel = XRCCTRL(*this, "SeedElementPanel", wxPanel);
	seedelementcheckbox = XRCCTRL(*this, "SeedElementCheckBox", wxCheckBox);

	USE_PARAMETER(event);
	if (seedelementcheckbox->IsChecked())
	{
			Cmiss_graphic_set_seed_element(region_tree_viewer->current_graphic, seed_element_chooser->get_object());
			seed_element_panel->Enable();
	}
	else
	{
			Cmiss_graphic_set_seed_element(region_tree_viewer->current_graphic,(FE_element*)NULL);
			seed_element_panel->Disable();
	}
	Region_tree_viewer_autoapply(region_tree_viewer->scene,
		region_tree_viewer->edit_scene);
	//Region_tree_viewer_renew_label_on_list(region_tree_viewer->current_graphic);

}

void EnterSampleLocation(wxCommandEvent &event)
{
	USE_PARAMETER(event);
	Cmiss_graphic_sampling_attributes_id sampling =
		Cmiss_graphic_get_sampling_attributes(region_tree_viewer->current_graphic);
	xitext=XRCCTRL(*this,"XiText",wxStaticText);
	xitextctrl=XRCCTRL(*this,"XiTextCtrl",wxTextCtrl);
	char temp_string[100];
	const int number_of_components=3;
	struct Parse_state *temp_state;
	double sample_location[number_of_components];
	if (CMISS_OK == Cmiss_graphic_sampling_attributes_get_location(sampling, number_of_components, sample_location))
	{
		/* Get the text string */
		wxString wxTextEntry = xitextctrl->GetValue();
		const char *text_entry = wxTextEntry.mb_str(wxConvUTF8);
		if (text_entry)
		{
			/* clean up spaces? */
			temp_state=create_Parse_state(text_entry);
			if (temp_state)
			{
				set_double_vector(temp_state, sample_location, (void *)&number_of_components);
				Cmiss_graphic_sampling_attributes_set_location(sampling, number_of_components, sample_location);
				/* inform the client of the change */
				Region_tree_viewer_autoapply(region_tree_viewer->scene,
					region_tree_viewer->edit_scene);
				//Region_tree_viewer_renew_label_on_list(region_tree_viewer->current_graphic);
				destroy_Parse_state(&temp_state);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"settings_editor_seed_xi_text_CB.  Missing text");
		}
		/* always re-display the values actually set */
		sprintf(temp_string,"%g,%g,%g", sample_location[0], sample_location[1], sample_location[2]);
		xitextctrl->ChangeValue(wxString::FromAscii(temp_string));
	}
	Cmiss_graphic_sampling_attributes_destroy(&sampling);
}

void EnterStreamlineLength(wxCommandEvent &event)
{
	Cmiss_graphic_streamlines_id streamlines = Cmiss_graphic_cast_streamlines(region_tree_viewer->current_graphic);
	streamlineslengthtextctrl = XRCCTRL(*this, "StreamlinesLengthTextCtrl", wxTextCtrl);
	wxString wxTextEntry = streamlineslengthtextctrl->GetValue();
	const char *text_entry = wxTextEntry.mb_str(wxConvUTF8);
	if (text_entry)
	{
		double streamline_length = 0.0;
		sscanf(text_entry, "%lg", &streamline_length);
		if (Cmiss_graphic_streamlines_set_track_length(streamlines, streamline_length))
		{
			/* inform the client of the change */
			Region_tree_viewer_autoapply(region_tree_viewer->scene,
				region_tree_viewer->edit_scene);
			//Region_tree_viewer_renew_label_on_list(region_tree_viewer->current_graphic);
		}
	}
	char temp_string[50];
	double streamline_length = Cmiss_graphic_streamlines_get_track_length(streamlines);
	sprintf(temp_string, "%g", streamline_length);
	streamlineslengthtextctrl->SetValue(wxString::FromAscii(temp_string));
	Cmiss_graphic_streamlines_destroy(&streamlines);
}

/**
 * Callback from wxChooser<Cmiss_graphic_streamlines_track_direction> when choice is made.
 */
int streamlines_track_direction_callback(enum Cmiss_graphic_streamlines_track_direction track_direction)
{
	Cmiss_graphic_streamlines_id streamlines = Cmiss_graphic_cast_streamlines(region_tree_viewer->current_graphic);
	Cmiss_graphic_streamlines_set_track_direction(streamlines, track_direction);
	Cmiss_graphic_streamlines_destroy(&streamlines);
	Region_tree_viewer_autoapply(region_tree_viewer->scene,
		region_tree_viewer->edit_scene);
	//Region_tree_viewer_renew_label_on_list(region_tree_viewer->current_graphic);
	return 1;
}

void EnterLineWidth(wxCommandEvent &event)
{
	USE_PARAMETER(event);
	line_width_text_ctrl = XRCCTRL(*this, "LineWidthTextCtrl", wxTextCtrl);
	double lineWidth;
	wxString wxTextEntry = line_width_text_ctrl->GetValue();
	const char *text_entry = wxTextEntry.mb_str(wxConvUTF8);
	if (text_entry)
	{
		sscanf(text_entry, "%lf", &lineWidth);
		Cmiss_graphic_set_render_line_width(region_tree_viewer->current_graphic, lineWidth);
		Region_tree_viewer_autoapply(region_tree_viewer->scene,
			region_tree_viewer->edit_scene);
	}
	/* redisplay actual value in use */
	lineWidth = Cmiss_graphic_get_render_line_width(region_tree_viewer->current_graphic);
	char temp_string[50];
	sprintf(temp_string, "%g", lineWidth);
	line_width_text_ctrl->SetValue(wxString::FromAscii(temp_string));
}

void EnterPointSize(wxCommandEvent &event)
{
	USE_PARAMETER(event);
	point_size_text_ctrl = XRCCTRL(*this, "PointSizeTextCtrl", wxTextCtrl);
	double pointSize;
	wxString wxTextEntry = point_size_text_ctrl->GetValue();
	const char *text_entry = wxTextEntry.mb_str(wxConvUTF8);
	if (text_entry)
	{
		sscanf(text_entry, "%lf", &pointSize);
		Cmiss_graphic_set_render_point_size(region_tree_viewer->current_graphic, pointSize);
		Region_tree_viewer_autoapply(region_tree_viewer->scene,
			region_tree_viewer->edit_scene);
	}
	/* redisplay actual value in use */
	pointSize = Cmiss_graphic_get_render_point_size(region_tree_viewer->current_graphic);
	char temp_string[50];
	sprintf(temp_string, "%g", pointSize);
	point_size_text_ctrl->SetValue(wxString::FromAscii(temp_string));
}

void ExteriorChecked(wxCommandEvent &event)
{
	USE_PARAMETER(event);
	exteriorcheckbox=XRCCTRL(*this,"ExteriorCheckBox",wxCheckBox);
	Cmiss_graphic_set_exterior(region_tree_viewer->current_graphic,
		exteriorcheckbox->IsChecked());
		/* inform the client of the change */
	Region_tree_viewer_autoapply(region_tree_viewer->scene,
		region_tree_viewer->edit_scene);
	//Region_tree_viewer_renew_label_on_list(region_tree_viewer->current_graphic);
}

void FaceChecked(wxCommandEvent &event)
{
	Cmiss_element_face_type face;
	USE_PARAMETER(event);
	facecheckbox=XRCCTRL(*this, "FaceCheckBox",wxCheckBox);
	facechoice=XRCCTRL(*this, "FaceChoice",wxChoice);
	if (facecheckbox->IsChecked())
	{
		facechoice->Enable();
		face = static_cast<Cmiss_element_face_type>(facechoice->GetSelection() + CMISS_ELEMENT_FACE_XI1_0);
	}
	else
	{
		facechoice->Disable();
		face = CMISS_ELEMENT_FACE_ALL;
	}
	Cmiss_graphic_set_face(region_tree_viewer->current_graphic,face);
	Region_tree_viewer_autoapply(region_tree_viewer->scene,
		region_tree_viewer->edit_scene);
	//Region_tree_viewer_renew_label_on_list(region_tree_viewer->current_graphic);
 }

void FaceChosen(wxCommandEvent &event)
{
	Cmiss_element_face_type face;
	USE_PARAMETER(event);
	facechoice=XRCCTRL(*this, "FaceChoice",wxChoice);
	face = static_cast<Cmiss_element_face_type>(facechoice->GetSelection() + CMISS_ELEMENT_FACE_XI1_0);
	Cmiss_graphic_set_face(region_tree_viewer->current_graphic, face);
	Region_tree_viewer_autoapply(region_tree_viewer->scene,
		region_tree_viewer->edit_scene);
	//Region_tree_viewer_renew_label_on_list(region_tree_viewer->current_graphic);
}

void SetBothMaterialChooser(Cmiss_graphic *graphic)
{
	Cmiss_graphics_material_id material = Cmiss_graphic_get_material(graphic);
	graphical_material_chooser->set_object(material);
	Cmiss_graphics_material_destroy(&material);
	Cmiss_graphics_material_id selected_material = Cmiss_graphic_get_selected_material(graphic);
	selected_material_chooser->set_object(selected_material);
	Cmiss_graphics_material_destroy(&selected_material);
}

void SetGraphic(Cmiss_graphic *graphic)
{
	int error;
	Cmiss_element_face_type face = CMISS_ELEMENT_FACE_INVALID;
	char temp_string[100], *vector_temp_string;
	enum Cmiss_graphic_render_polygon_mode render_polygon_mode;
	struct FE_element *seed_element;
	struct FE_region *fe_region;

	const Cmiss_graphic_type graphic_type = Cmiss_graphic_get_graphic_type(graphic);
	const int domain_dimension = Cmiss_graphic_get_domain_dimension(graphic);
	Cmiss_graphic_line_attributes_id line_attributes =
		Cmiss_graphic_get_line_attributes(graphic);
	Cmiss_graphic_point_attributes_id point_attributes =
		Cmiss_graphic_get_point_attributes(graphic);
	Cmiss_graphic_contours_id contours = Cmiss_graphic_cast_contours(graphic);

	coordinate_field_chooser_panel =
		XRCCTRL(*this, "CoordinateFieldChooserPanel",wxPanel);
	wxStaticText *coordinatefieldstatictext=
		XRCCTRL(*this, "CoordinateFieldStaticText",wxStaticText);

	Cmiss_field_id temp_coordinate_field = 0;
	if (0 != region_tree_viewer->current_graphic)
	{
		temp_coordinate_field =
			Cmiss_graphic_get_coordinate_field(region_tree_viewer->current_graphic);
	}
	if (coordinate_field_chooser ==NULL)
	{
		coordinate_field_chooser =
			new Managed_object_chooser<Computed_field,MANAGER_CLASS(Computed_field)>
			(coordinate_field_chooser_panel, temp_coordinate_field, region_tree_viewer->field_manager,
				(MANAGER_CONDITIONAL_FUNCTION(Computed_field) *)NULL, (void *)NULL, region_tree_viewer->user_interface);
		Callback_base< Computed_field* > *coordinate_field_callback =
			new Callback_member_callback< Computed_field*,
			wxRegionTreeViewer, int (wxRegionTreeViewer::*)(Computed_field *) >
			(this, &wxRegionTreeViewer::coordinate_field_callback);
		coordinate_field_chooser->set_callback(coordinate_field_callback);
		coordinate_field_chooser_panel->Fit();
		coordinate_field_chooser->include_null_item(true);
	}
	coordinate_field_chooser->set_object(temp_coordinate_field);
	coordinate_field_chooser_panel->Show();
	coordinatefieldstatictext->Show();
	Cmiss_field_destroy(&temp_coordinate_field);

	enum Cmiss_graphics_coordinate_system coordinate_system;
	coordinate_system = Cmiss_graphic_get_coordinate_system(
		region_tree_viewer->current_graphic);
	coordinate_system_chooser->set_value(coordinate_system);

	lineshapetext = XRCCTRL(*this, "LineShapeText", wxStaticText);
	line_shape_chooser_panel = XRCCTRL(*this, "LineShapeChooserPanel", wxPanel);
	wxStaticText *linebasesizetext = XRCCTRL(*this, "LineBaseSizeText", wxStaticText);
	linebasesizetextctrl = XRCCTRL(*this,"LineBaseSizeTextCtrl", wxTextCtrl);
	wxStaticText *linescalefactorstext = XRCCTRL(*this, "LineScaleFactorsText", wxStaticText);
	linescalefactorstextctrl = XRCCTRL(*this,"LineScaleFactorsTextCtrl", wxTextCtrl);
	wxStaticText *lineorientationscalefieldtext = XRCCTRL(*this, "LineOrientationScaleFieldText", wxStaticText);
	line_orientation_scale_field_chooser_panel = XRCCTRL(*this, "LineOrientationScaleFieldChooserPanel", wxPanel);
	if (line_attributes)
	{
		Cmiss_graphic_line_attributes_shape line_shape = Cmiss_graphic_line_attributes_get_shape(line_attributes);
		if (0 == line_shape_chooser)
		{
			line_shape_chooser = new Enumerator_chooser<ENUMERATOR_TYPE_CLASS(Cmiss_graphic_line_attributes_shape)>(
				line_shape_chooser_panel, line_shape,
				(ENUMERATOR_CONDITIONAL_FUNCTION(Cmiss_graphic_line_attributes_shape) *)NULL,
				(void *)NULL, region_tree_viewer->user_interface);
			line_shape_chooser_panel->Fit();
			Callback_base< enum Cmiss_graphic_line_attributes_shape > *line_shape_callback =
				new Callback_member_callback< enum Cmiss_graphic_line_attributes_shape,
					wxRegionTreeViewer, int (wxRegionTreeViewer::*)(enum Cmiss_graphic_line_attributes_shape) >(
						this, &wxRegionTreeViewer::line_shape_callback);
			line_shape_chooser->set_callback(line_shape_callback);
		}
		line_shape_chooser->set_value(line_shape);
		lineshapetext->Show();
		line_shape_chooser_panel->Show();

		double lineBaseSize[2];
		Cmiss_graphic_line_attributes_get_base_size(line_attributes, 2, lineBaseSize);
		sprintf(temp_string, "%g*%g", lineBaseSize[0], lineBaseSize[1]);
		linebasesizetextctrl->SetValue(wxString::FromAscii(temp_string));
		linebasesizetext->Show();
		linebasesizetextctrl->Show();

		double lineScaleFactors[2];
		Cmiss_graphic_line_attributes_get_scale_factors(line_attributes, 2, lineScaleFactors);
		sprintf(temp_string, "%g*%g", lineScaleFactors[0], lineScaleFactors[1]);
		linescalefactorstextctrl->SetValue(wxString::FromAscii(temp_string));
		linescalefactorstext->Show();
		linescalefactorstextctrl->Show();

		Cmiss_field_id orientationScaleField =
			Cmiss_graphic_line_attributes_get_orientation_scale_field(line_attributes);
		if (line_orientation_scale_field_chooser == NULL)
		{
			line_orientation_scale_field_chooser = new Managed_object_chooser<Computed_field,MANAGER_CLASS(Computed_field)>(
				line_orientation_scale_field_chooser_panel, orientationScaleField, region_tree_viewer->field_manager,
				Computed_field_is_scalar, (void *)NULL, region_tree_viewer->user_interface);
			Callback_base< Computed_field* > *line_orientation_scale_field_callback =
				new Callback_member_callback< Computed_field*,
			wxRegionTreeViewer, int (wxRegionTreeViewer::*)(Computed_field *) >
				(this, &wxRegionTreeViewer::line_orientation_scale_field_callback);
			line_orientation_scale_field_chooser->set_callback(line_orientation_scale_field_callback);
			line_orientation_scale_field_chooser_panel->Fit();
			line_orientation_scale_field_chooser->include_null_item(true);
		}
		line_orientation_scale_field_chooser->set_object(orientationScaleField);
		lineorientationscalefieldtext->Show();
		line_orientation_scale_field_chooser_panel->Show();
		if (0 != orientationScaleField)
		{
			linescalefactorstextctrl->Enable();
		}
		else
		{
			linescalefactorstextctrl->Disable();
		}
		Cmiss_field_destroy(&orientationScaleField);
	}
	else
	{
		lineshapetext->Hide();
		line_shape_chooser_panel->Hide();
		linebasesizetext->Hide();
		linebasesizetextctrl->Hide();
		linescalefactorstext->Hide();
		linescalefactorstextctrl->Hide();
		lineorientationscalefieldtext->Hide();
		line_orientation_scale_field_chooser_panel->Hide();
	}

	// iso-surface
	isoscalar_chooser_panel=XRCCTRL(*this, "IsoScalarChooserPanel",wxPanel);
	isoscalartextctrl = XRCCTRL(*this, "IsoScalarTextCtrl",wxTextCtrl);
	isoscalartext=XRCCTRL(*this,"IsoScalarText",wxStaticText);
	isovalueoptionspane=XRCCTRL(*this,"IsoValueOptions",wxPanel);

	isovaluelistradiobutton=XRCCTRL(*this,"IsoValueListRadioButton",wxRadioButton);
	isovaluesequenceradiobutton=XRCCTRL(*this,"IsoValueSequenceRadioButton",wxRadioButton);

	isoscalartextctrl=XRCCTRL(*this,"IsoScalarTextCtrl",wxTextCtrl);
	isovaluesequencenumbertextctrl=XRCCTRL(*this,"IsoValueSequenceNumberTextCtrl",wxTextCtrl);
	isovaluesequencefirsttextctrl=XRCCTRL(*this,"IsoValueSequenceFirstTextCtrl",wxTextCtrl);
	isovaluesequencelasttextctrl=XRCCTRL(*this,"IsoValueSequenceLastTextCtrl",wxTextCtrl);

	if (contours)
	{
		Cmiss_field_id isoscalar_field =
			Cmiss_graphic_contours_get_isoscalar_field(contours);
		int number_of_isovalues = Cmiss_graphic_contours_get_list_isovalues(contours, 0, 0);
		double *isovalues = 0;
		double first_isovalue, last_isovalue;
		if (number_of_isovalues)
		{
			isovalues = new double[number_of_isovalues];
			Cmiss_graphic_contours_get_list_isovalues(contours, number_of_isovalues, isovalues);
		}
		else
		{
			number_of_isovalues = Cmiss_graphic_contours_get_range_number_of_isovalues(contours);
			first_isovalue = Cmiss_graphic_contours_get_range_first_isovalue(contours);
			last_isovalue = Cmiss_graphic_contours_get_range_last_isovalue(contours);
		}
		double decimation_threshold = Cmiss_graphic_contours_get_decimation_threshold(contours);

		isoscalar_chooser_panel->Show();
		isoscalartextctrl->Show();
		isoscalartext->Show();
		isovalueoptionspane->Show();
		if (0 == isoscalar_chooser)
		{
			isoscalar_chooser =
					new Managed_object_chooser<Computed_field,MANAGER_CLASS(Computed_field)>
					(isoscalar_chooser_panel, isoscalar_field, region_tree_viewer->field_manager,
						Computed_field_is_scalar, (void *)NULL, region_tree_viewer->user_interface);
			Callback_base< Computed_field* > *isoscalar_callback =
					new Callback_member_callback< Computed_field*,
					wxRegionTreeViewer, int (wxRegionTreeViewer::*)(Computed_field *) >
					(this, &wxRegionTreeViewer::isoscalar_callback);
			isoscalar_chooser->set_callback(isoscalar_callback);
			isoscalar_chooser_panel->Fit();
			isoscalar_chooser->include_null_item(true);
		}
		if (isovalues)
		{
			isovaluelistradiobutton->SetValue(true);
			isoscalartextctrl->Enable();
			isovaluesequencenumbertextctrl->Disable();
			isovaluesequencefirsttextctrl->Disable();
			isovaluesequencelasttextctrl->Disable();
			vector_temp_string = (char *)NULL;
			error = 0;
			for (int i = 0 ; !error && (i < number_of_isovalues) ; i++)
			{
				sprintf(temp_string,"%g ",isovalues[i]);
				append_string(&vector_temp_string, temp_string, &error);
			}
			if (vector_temp_string)
			{
				isoscalartextctrl ->SetValue(wxString::FromAscii(vector_temp_string));
				DEALLOCATE(vector_temp_string);
			}
		}
		else
		{
			isoscalartextctrl->Disable();
			isovaluesequencenumbertextctrl->Enable();
			isovaluesequencefirsttextctrl->Enable();
			isovaluesequencelasttextctrl->Enable();
			isovaluesequenceradiobutton->SetValue(true);
			sprintf(temp_string,"%d", number_of_isovalues);
			isovaluesequencenumbertextctrl->SetValue(wxString::FromAscii(temp_string));
			sprintf(temp_string,"%g", first_isovalue);
			isovaluesequencefirsttextctrl->SetValue(wxString::FromAscii(temp_string));
			sprintf(temp_string,"%g", last_isovalue);
			isovaluesequencelasttextctrl->SetValue(wxString::FromAscii(temp_string));
		}
		isoscalar_chooser->set_object(isoscalar_field);
		delete[] isovalues;
		Cmiss_field_destroy(&isoscalar_field);
	}
	else
	{
		isoscalar_chooser_panel->Hide();
		isoscalartextctrl->Hide();
		isoscalartext->Hide();
		isovalueoptionspane->Hide();
	}

		/* node_points, data_points, element_points */
		/* glyphs */
		glyph_chooser_panel=XRCCTRL(*this,"GlyphChooserPanel",wxPanel);
		glyph_repeat_mode_text=XRCCTRL(*this,"GlyphRepeatModeText",wxStaticText);
		glyph_repeat_mode_chooser_panel=XRCCTRL(*this,"GlyphRepeatModeChooserPanel",wxPanel);
		orientation_scale_field_chooser_panel=XRCCTRL(*this,	"OrientationScaleChooserPanel",wxPanel);
		variable_scale_field_chooser_panel=XRCCTRL(*this,"VariableScaleChooserPanel",wxPanel);
		glyphtext=XRCCTRL(*this,"GlyphText",wxStaticText);
		offsettext=XRCCTRL(*this,"OffsetText",wxStaticText);
		offsettextctrl=XRCCTRL(*this,"OffsetTextCtrl",wxTextCtrl);
		baseglyphsizetext=XRCCTRL(*this,"BaseGlyphSizeText",wxStaticText);
		baseglyphsizetextctrl=XRCCTRL(*this,"BaseGlyphSizeTextCtrl",wxTextCtrl);
		wxStaticText *orientationscaletext=XRCCTRL(*this,"OrientationScaleText",wxStaticText);
		glyphscalefactorstext	=XRCCTRL(*this,"GlyphScaleFactorsText",wxStaticText);
		glyphscalefactorstextctrl=XRCCTRL(*this,"GlyphScaleFactorsTextCtrl",wxTextCtrl);
		wxStaticText *variablescaletext=XRCCTRL(*this,"VariableScaleText",wxStaticText);
		glyphbox=XRCCTRL(*this,"GlyphBox",wxWindow);
		glyphline=XRCCTRL(*this,"GlyphLine",wxWindow);

		/* overlay disabled
		wxCheckBox *overlay_checkbox = XRCCTRL(*this, "OverlayCheckBox", wxCheckBox);
		wxTextCtrl *overlay_textctrl = XRCCTRL(*this, "OverlayTextCtrl", wxTextCtrl);
		if (CMISS_GRAPHIC_STATIC == graphic_type)
		{
			overlay_checkbox->SetValue(Cmiss_graphic_is_overlay(graphic));
			overlay_checkbox->Show();
			if (Cmiss_graphic_is_overlay(graphic))
			{
				overlay_textctrl->Enable();
			}
			else
			{
				overlay_textctrl->Disable();
			}
			sprintf(temp_string,"%d ",Cmiss_graphic_get_overlay_order(graphic));
			overlay_textctrl->SetValue(temp_string);
			overlay_textctrl->Show();
		}
		else
		{
			overlay_textctrl->Hide();
			overlay_textctrl->Disable();
			overlay_checkbox->Hide();
		}
		*/
		if (point_attributes)
		{
			Cmiss_glyph_id glyph = Cmiss_graphic_point_attributes_get_glyph(point_attributes);
			Cmiss_field_id orientation_scale_field =
				Cmiss_graphic_point_attributes_get_orientation_scale_field(point_attributes);
			Cmiss_field_id signed_scale_field =
				Cmiss_graphic_point_attributes_get_signed_scale_field(point_attributes);
			double point_base_size[3], glyph_offset[3], point_scale_factors[3];
			Cmiss_graphic_point_attributes_get_base_size(point_attributes, 3, point_base_size);
			Cmiss_graphic_point_attributes_get_glyph_offset(point_attributes, 3, glyph_offset);
			Cmiss_graphic_point_attributes_get_scale_factors(point_attributes, 3, point_scale_factors);
			Cmiss_glyph_repeat_mode glyph_repeat_mode =
				Cmiss_graphic_point_attributes_get_glyph_repeat_mode(point_attributes);

			/* turn on callbacks */
			glyph_chooser_panel->Show();
			glyph_repeat_mode_text->Show();
			glyph_repeat_mode_chooser_panel->Show();
			orientation_scale_field_chooser_panel->Show();
			variable_scale_field_chooser_panel->Show();
			glyphtext->Show();
			offsettext->Show();
			offsettextctrl->Show();
			baseglyphsizetext->Show();
			baseglyphsizetextctrl->Show();
			orientationscaletext->Show();
			glyphscalefactorstext->Show();
			glyphscalefactorstextctrl->Show();
			variablescaletext->Show();
			glyphbox->Show();
			glyphline->Show();
			if (glyph_chooser == NULL)
			{
				glyph_chooser =
						new Managed_object_chooser<Cmiss_glyph, MANAGER_CLASS(Cmiss_glyph)>
						(glyph_chooser_panel, glyph, Cmiss_glyph_module_get_manager(region_tree_viewer->glyphModule),
							(LIST_CONDITIONAL_FUNCTION(Cmiss_glyph) *)NULL, (void *)NULL,
							region_tree_viewer->user_interface);
				Callback_base< Cmiss_glyph* > *glyph_callback =
						new Callback_member_callback< Cmiss_glyph*,
						wxRegionTreeViewer, int (wxRegionTreeViewer::*)(Cmiss_glyph *) >
						(this, &wxRegionTreeViewer::glyph_callback);
				glyph_chooser->include_null_item(true);
				glyph_chooser->set_callback(glyph_callback);
				glyph_chooser_panel->Fit();
			}
			glyph_chooser->set_object(glyph);

			if (glyph_repeat_mode_chooser == 0)
			{
				glyph_repeat_mode_chooser =
					new Enumerator_chooser<ENUMERATOR_TYPE_CLASS(Cmiss_glyph_repeat_mode)>(
						glyph_repeat_mode_chooser_panel, glyph_repeat_mode,
							(ENUMERATOR_CONDITIONAL_FUNCTION(Cmiss_glyph_repeat_mode) *)0,
							(void *)NULL, region_tree_viewer->user_interface);
				glyph_repeat_mode_chooser_panel->Fit();
				Callback_base< enum Cmiss_glyph_repeat_mode > *glyph_repeat_mode_callback =
						new Callback_member_callback< enum Cmiss_glyph_repeat_mode,
						wxRegionTreeViewer, int (wxRegionTreeViewer::*)(enum Cmiss_glyph_repeat_mode) >
						(this, &wxRegionTreeViewer::glyph_repeat_mode_callback);
				glyph_repeat_mode_chooser->set_callback(glyph_repeat_mode_callback);
			}
			glyph_repeat_mode_chooser->set_value(glyph_repeat_mode);

			if (orientation_scale_field_chooser == NULL)
			{
				orientation_scale_field_chooser=
						new Managed_object_chooser<Computed_field,MANAGER_CLASS(Computed_field)>
						(orientation_scale_field_chooser_panel, orientation_scale_field, region_tree_viewer->field_manager,
							Computed_field_is_orientation_scale_capable, NULL,
							region_tree_viewer->user_interface);
				Callback_base< Computed_field* > *orientation_scale_callback =
						new Callback_member_callback< Computed_field*,
						wxRegionTreeViewer, int (wxRegionTreeViewer::*)(Computed_field *) >
						(this, &wxRegionTreeViewer::orientation_scale_callback);
				orientation_scale_field_chooser->set_callback(orientation_scale_callback);
				orientation_scale_field_chooser_panel->Fit();
				orientation_scale_field_chooser->include_null_item(true);
			}
			if (variable_scale_field_chooser  ==NULL)
			{
				variable_scale_field_chooser=
						new Managed_object_chooser<Computed_field,MANAGER_CLASS(Computed_field)>
						(variable_scale_field_chooser_panel, signed_scale_field, region_tree_viewer->field_manager,
							Computed_field_has_up_to_3_numerical_components, (void *)NULL,
							region_tree_viewer->user_interface);
				Callback_base< Computed_field* > *variable_scale_callback =
						new Callback_member_callback< Computed_field*,
						wxRegionTreeViewer, int (wxRegionTreeViewer::*)(Computed_field *) >
						(this, &wxRegionTreeViewer::variable_scale_callback);
				variable_scale_field_chooser->set_callback(variable_scale_callback);
				variable_scale_field_chooser_panel->Fit();
				variable_scale_field_chooser->include_null_item(true);
			}

			sprintf(temp_string,"%g,%g,%g",
				glyph_offset[0], glyph_offset[1], glyph_offset[2]);
			offsettextctrl->SetValue(wxString::FromAscii(temp_string));
			sprintf(temp_string,"%g*%g*%g",
				point_base_size[0], point_base_size[1], point_base_size[2]);
			baseglyphsizetextctrl->SetValue(wxString::FromAscii(temp_string));
			sprintf(temp_string,"%g*%g*%g", point_scale_factors[0],
				point_scale_factors[1], point_scale_factors[2]);
			glyphscalefactorstextctrl->SetValue(wxString::FromAscii(temp_string));

				orientation_scale_field_chooser->set_object(orientation_scale_field);
			if ((struct Computed_field *)NULL!=orientation_scale_field)
			{
				glyphscalefactorstextctrl->Enable();
				glyphscalefactorstext->Enable();
			}
			else
			{
				glyphscalefactorstextctrl->Disable();
				glyphscalefactorstext->Disable();
			}
			variable_scale_field_chooser->set_object(signed_scale_field);

			Cmiss_field_destroy(&orientation_scale_field);
			Cmiss_field_destroy(&signed_scale_field);
			Cmiss_glyph_destroy(&glyph);
		}
		else
		{
			glyph_chooser_panel->Hide();
			glyph_repeat_mode_text->Hide();
			glyph_repeat_mode_chooser_panel->Hide();
			orientation_scale_field_chooser_panel->Hide();
			variable_scale_field_chooser_panel->Hide();
			glyphtext->Hide();
			offsettext->Hide();
			offsettextctrl->Hide();
			baseglyphsizetext->Hide();
			baseglyphsizetextctrl->Hide();
			orientationscaletext->Hide();
			glyphscalefactorstext->Hide();
			glyphscalefactorstextctrl->Hide();
			variablescaletext->Hide();
			glyphbox->Hide();
			glyphline->Hide();
		}

		/* label field */
		label_chooser_panel = XRCCTRL(*this,"LabelChooserPanel",wxPanel);
		labeloffsettext = XRCCTRL(*this,"LabelOffsetText",wxStaticText);
		labeloffsettextctrl = XRCCTRL(*this,"LabelOffsetTextCtrl",wxTextCtrl);
		fonttext=XRCCTRL(*this,"FontText",wxStaticText);
		font_chooser_panel = XRCCTRL(*this,"FontChooserPanel",wxPanel);
		wxStaticText *labeltext = XRCCTRL(*this,"LabelText",wxStaticText);
		staticlabeltext = XRCCTRL(*this,"StaticLabelText",wxStaticText);
		staticlabeltextctrl[0] = XRCCTRL(*this,"StaticLabelTextCtrl1",wxTextCtrl);
		staticlabeltextctrl[1] = XRCCTRL(*this,"StaticLabelTextCtrl2",wxTextCtrl);
		staticlabeltextctrl[2] = XRCCTRL(*this,"StaticLabelTextCtrl3",wxTextCtrl);
		if (point_attributes)
		{
			Cmiss_field_id label_field = Cmiss_graphic_point_attributes_get_label_field(point_attributes);
			if (label_field_chooser == NULL)
			{
					label_field_chooser =
						new Managed_object_chooser<Computed_field,MANAGER_CLASS(Computed_field)>
						(label_chooser_panel, label_field, region_tree_viewer->field_manager,
								(MANAGER_CONDITIONAL_FUNCTION(Computed_field) *)NULL , (void *)NULL,
								region_tree_viewer->user_interface);
					Callback_base< Computed_field* > *label_callback =
						new Callback_member_callback< Computed_field*,
						wxRegionTreeViewer, int (wxRegionTreeViewer::*)(Computed_field *) >
						(this, &wxRegionTreeViewer::label_callback);
					label_field_chooser->set_callback(label_callback);
					label_chooser_panel->Fit();
					label_field_chooser->include_null_item(true);
			}
			label_field_chooser->set_object(label_field);
			Cmiss_field_destroy(&label_field);
			labeltext->Show();
			label_chooser_panel->Show();

			double label_offset[3];
			Cmiss_graphic_point_attributes_get_label_offset(point_attributes, 3, label_offset);
			sprintf(temp_string,"%g,%g,%g",
				label_offset[0], label_offset[1], label_offset[2]);
			labeloffsettextctrl->SetValue(wxString::FromAscii(temp_string));
			labeloffsettext->Show();
			labeloffsettextctrl->Show();
			staticlabeltext->Show();
			for (int i = 0; i < 3; ++i)
			{
				char *label_text = Cmiss_graphic_point_attributes_get_label_text(point_attributes, i + 1);
				staticlabeltextctrl[i]->SetValue(wxString::FromAscii(label_text ? label_text : ""));
				if (label_text)
				{
					DEALLOCATE(label_text);
				}
				staticlabeltextctrl[i]->Show();
			}

			Cmiss_font_id font = Cmiss_graphic_point_attributes_get_font(point_attributes);
			if (font_chooser == NULL)
			{
					font_chooser =
						new Managed_object_chooser<Cmiss_font,MANAGER_CLASS(Cmiss_font)>
						(font_chooser_panel, font, region_tree_viewer->font_manager,
								(MANAGER_CONDITIONAL_FUNCTION(Cmiss_font) *)NULL , (void *)NULL,
								region_tree_viewer->user_interface);
					Callback_base< Cmiss_font* > *font_callback =
						new Callback_member_callback< Cmiss_font*,
						wxRegionTreeViewer, int (wxRegionTreeViewer::*)(Cmiss_font *) >
						(this, &wxRegionTreeViewer::font_callback);
					font_chooser->set_callback(font_callback);
					font_chooser_panel->Fit();
			}
			fonttext->Show();
			font_chooser_panel->Show();
			if (0 != font)
			{
					font_chooser->set_object(font);
					font_chooser_panel->Enable();
			}
			else
			{
					font_chooser_panel->Disable();
			}
			Cmiss_font_destroy(&font);
		}
		else
		{
			labeltext->Hide();
			label_chooser_panel->Hide();
			labeloffsettext->Hide();
			labeloffsettextctrl->Hide();
			staticlabeltext->Hide();
			for (int i = 0; i < 3; ++i)
			{
				staticlabeltextctrl[i]->Hide();
			}
			font_chooser_panel->Hide();
			fonttext->Hide();
		}

		/* Subgroup field */
		wxStaticText *subgroup_field_text=XRCCTRL(*this,"SubgroupFieldText",wxStaticText);
		subgroup_field_chooser_panel = XRCCTRL(*this,"SubgroupFieldChooserPanel",wxPanel);

		Cmiss_field_id subgroup_field = Cmiss_graphic_get_subgroup_field(graphic);
		if (subgroup_field_chooser == NULL)
		{
			subgroup_field_chooser =
				new Managed_object_chooser<Computed_field,MANAGER_CLASS(Computed_field)>
				(subgroup_field_chooser_panel, subgroup_field, region_tree_viewer->field_manager,
					Computed_field_is_scalar, (void *)NULL,
					region_tree_viewer->user_interface);
			Callback_base< Computed_field* > *subgroup_field_callback =
				new Callback_member_callback< Computed_field*,
				wxRegionTreeViewer, int (wxRegionTreeViewer::*)(Computed_field *) >
				(this, &wxRegionTreeViewer::subgroup_field_callback);
			subgroup_field_chooser->set_callback(subgroup_field_callback);
			subgroup_field_chooser_panel->Fit();
			subgroup_field_chooser->include_null_item(true);
		}
		subgroup_field_text->Show();
		subgroup_field_chooser->set_object(subgroup_field);
		subgroup_field_chooser_panel->Show();
		Cmiss_field_destroy(&subgroup_field);

		/* Set the select_mode_chooser_panel*/
		select_mode_chooser_panel =
			XRCCTRL(*this, "SelectModeChooserPanel", wxPanel);
		select_mode_text=XRCCTRL(*this,"SelectModeText",wxStaticText);
		if (select_mode_chooser == NULL)
		{
			select_mode_chooser =
				new Enumerator_chooser<ENUMERATOR_TYPE_CLASS(Graphics_select_mode)>
				(select_mode_chooser_panel,
					region_tree_viewer->select_mode,
					(ENUMERATOR_CONDITIONAL_FUNCTION(Graphics_select_mode) *)NULL,
					(void *)NULL, region_tree_viewer->user_interface);
			select_mode_chooser_panel->Fit();
			Callback_base< enum Graphics_select_mode > *select_mode_callback =
				new Callback_member_callback< enum Graphics_select_mode,
				wxRegionTreeViewer, int (wxRegionTreeViewer::*)(enum Graphics_select_mode) >
				(this, &wxRegionTreeViewer::select_mode_callback);
			select_mode_chooser->set_callback(select_mode_callback);
		}
		select_mode_chooser_panel->Show();
		select_mode_text->Show();
		select_mode_chooser->set_value(Cmiss_graphic_get_select_mode(graphic));

		domain_type_chooser_panel =
			XRCCTRL(*this, "DomainTypeChooserPanel", wxPanel);
		domaintypetext=XRCCTRL(*this,"DomainTypeText",wxStaticText);
		if (domain_type_chooser == NULL)
		{
			domain_type_chooser =
				new Enumerator_chooser<ENUMERATOR_TYPE_CLASS(Cmiss_field_domain_type)>
				(domain_type_chooser_panel,
					region_tree_viewer->domain_type,
					(ENUMERATOR_CONDITIONAL_FUNCTION(Cmiss_field_domain_type) *)NULL,
					(void *)NULL, region_tree_viewer->user_interface);
			domain_type_chooser_panel->Fit();
			Callback_base< enum Cmiss_field_domain_type > *domain_type_callback =
				new Callback_member_callback< enum Cmiss_field_domain_type,
				wxRegionTreeViewer, int (wxRegionTreeViewer::*)(enum Cmiss_field_domain_type) >
				(this, &wxRegionTreeViewer::domain_type_callback);
			domain_type_chooser->set_callback(domain_type_callback);
		}
		domain_type_chooser_panel->Show();
		domaintypetext->Show();
		domain_type_chooser->set_value(Cmiss_graphic_get_domain_type(graphic));

		tessellationtext=XRCCTRL(*this,"TessellationStaticText",wxStaticText);
		tessellation_chooser_panel=XRCCTRL(*this, "TessellationChooserPanel",wxPanel);
		tessellationbutton=XRCCTRL(*this,"TessellationButton",wxButton);
		Cmiss_tessellation *tessellation = Cmiss_graphic_get_tessellation(graphic);
		tessellation_chooser->set_object(tessellation);
		tessellation_chooser_panel->Show();
		tessellationtext->Show();
		tessellationbutton->Show();
		if (tessellation)
		{
			DEACCESS(Cmiss_tessellation)(&tessellation);
		}

		wxStaticText *tessellation_field_text = XRCCTRL(*this, "TessellationFieldText", wxStaticText);
		tessellation_field_chooser_panel = XRCCTRL(*this, "TessellationFieldChooserPanel", wxPanel);

		Cmiss_field_id tessellation_field = Cmiss_graphic_get_tessellation_field(graphic);
		if (tessellation_field_chooser == NULL)
		{
			tessellation_field_chooser =
				new Managed_object_chooser<Computed_field,MANAGER_CLASS(Computed_field)>
				(tessellation_field_chooser_panel,(Computed_field*)NULL,
					region_tree_viewer->field_manager,
					(MANAGER_CONDITIONAL_FUNCTION(Computed_field) *)NULL , (void *)NULL,
					region_tree_viewer->user_interface);
			Callback_base< Computed_field* > *tessellation_field_callback =
				new Callback_member_callback< Computed_field*,
				wxRegionTreeViewer, int (wxRegionTreeViewer::*)(Computed_field *) >
				(this, &wxRegionTreeViewer::tessellation_field_callback);
			tessellation_field_chooser->set_callback(tessellation_field_callback);
			tessellation_field_chooser_panel->Fit();
			tessellation_field_chooser->include_null_item(true);
		}
		tessellation_field_text->Show();
		tessellation_field_chooser_panel->Show();
		tessellation_field_chooser->set_object(tessellation_field);
		Cmiss_field_destroy(&tessellation_field);

		sample_mode_text = XRCCTRL(*this,"SampleModeText",wxStaticText);
		sample_mode_chooser_panel= XRCCTRL(*this,"SampleModeChooserPanel",wxPanel);
		sample_density_field_text=XRCCTRL(*this, "DensityFieldText",wxStaticText);
		sample_density_field_chooser_panel=XRCCTRL(*this,"DensityFieldChooserPanel",wxPanel);
		xitext=XRCCTRL(*this,"XiText",wxStaticText);
		xitextctrl=XRCCTRL(*this,"XiTextCtrl",wxTextCtrl);
		Cmiss_graphic_sampling_attributes_id sampling = Cmiss_graphic_get_sampling_attributes(graphic);
		if (sampling)
		{
			Cmiss_element_point_sample_mode sample_mode = Cmiss_graphic_sampling_attributes_get_mode(sampling);
			if (0 == sample_mode_chooser)
			{
				sample_mode_chooser =
					new Enumerator_chooser<ENUMERATOR_TYPE_CLASS(Cmiss_element_point_sample_mode)>
					(sample_mode_chooser_panel, sample_mode,
						(ENUMERATOR_CONDITIONAL_FUNCTION(Cmiss_element_point_sample_mode) *)NULL,
						(void *)NULL, region_tree_viewer->user_interface);
				sample_mode_chooser_panel->Fit();
				Callback_base< enum Cmiss_element_point_sample_mode > *sample_mode_callback =
					new Callback_member_callback< enum Cmiss_element_point_sample_mode,
					wxRegionTreeViewer, int (wxRegionTreeViewer::*)(enum Cmiss_element_point_sample_mode) >
					(this, &wxRegionTreeViewer::sample_mode_callback);
				sample_mode_chooser->set_callback(sample_mode_callback);
			}
			sample_mode_chooser_panel->Show();
			sample_mode_text->Show();
			sample_mode_chooser->set_value(sample_mode);

			Cmiss_field_id sample_density_field = Cmiss_graphic_sampling_attributes_get_density_field(sampling);
			if (sample_density_field_chooser == NULL)
			{
				sample_density_field_chooser =
					new Managed_object_chooser<Computed_field,MANAGER_CLASS(Computed_field)>
					(sample_density_field_chooser_panel, sample_density_field, region_tree_viewer->field_manager,
						Computed_field_is_scalar, (void *)NULL,
						region_tree_viewer->user_interface);
				sample_density_field_chooser->include_null_item(true);
				Callback_base< Computed_field* > *sample_density_callback =
					new Callback_member_callback< Computed_field*,
					wxRegionTreeViewer, int (wxRegionTreeViewer::*)(Computed_field *) >
					(this, &wxRegionTreeViewer::sample_density_callback);
				sample_density_field_chooser->set_callback(sample_density_callback);
				sample_density_field_chooser_panel->Fit();
			}
			sample_density_field_text->Show();
			sample_density_field_chooser_panel->Show();
			sample_density_field_chooser->set_object(sample_density_field);

			double sample_location[3];
			Cmiss_graphic_sampling_attributes_get_location(sampling, 3, sample_location);
			sprintf(temp_string,"%g,%g,%g",sample_location[0],sample_location[1],sample_location[2]);
			xitextctrl->SetValue(wxString::FromAscii(temp_string));
		}
		Cmiss_graphic_sampling_attributes_destroy(&sampling);
		show_sampling_widgets();

		/* seed element */
		seed_element_panel = XRCCTRL(*this, "SeedElementPanel", wxPanel);
		seedelementcheckbox = XRCCTRL(*this, "SeedElementCheckBox", wxCheckBox);

		if (CMISS_GRAPHIC_STREAMLINES==graphic_type)
		{
			seed_element_panel->Show();
			seedelementcheckbox->Show();
			fe_region = Cmiss_region_get_FE_region(Cmiss_scene_get_region(
				region_tree_viewer->edit_scene));
			seed_element =
				Cmiss_graphic_get_seed_element(graphic);
			if (seed_element_chooser == NULL)
			{
				seed_element_chooser = new wxFeElementTextChooser(seed_element_panel,
						seed_element, fe_region, FE_element_is_top_level,(void *)NULL);
				Callback_base<FE_element *> *seed_element_callback =
						new Callback_member_callback< FE_element*,
						wxRegionTreeViewer, int (wxRegionTreeViewer::*)(FE_element *) >
						(this, &wxRegionTreeViewer::seed_element_callback);
				seed_element_chooser->set_callback(seed_element_callback);
				seed_element_panel->Fit();
			}
			if (NULL != seed_element)
			{
				seedelementcheckbox->SetValue(1);
				seed_element_panel->Enable();
			}
			else
			{
				seedelementcheckbox->SetValue(0);
				seed_element_panel->Disable();
			}
			seed_element_chooser->set_object(seed_element);
		}
		else
		{
			seed_element_panel->Hide();
			seedelementcheckbox->Hide();
		}

		streamvectortext = XRCCTRL(*this, "StreamVectorText",wxStaticText);
		stream_vector_chooser_panel = XRCCTRL(*this, "StreamVectorChooserPanel",wxPanel);
		streamlines_track_direction_chooser_panel = XRCCTRL(*this, "StreamlinesTrackDirectionChooserPanel",wxPanel);
		streamlineslengthtext = XRCCTRL(*this, "StreamlinesLengthText",wxStaticText);
		streamlineslengthtextctrl = XRCCTRL(*this,"StreamlinesLengthTextCtrl",wxTextCtrl);
		Cmiss_graphic_streamlines_id streamlines = Cmiss_graphic_cast_streamlines(graphic);
		if (streamlines)
		{
			Cmiss_field_id stream_vector_field = Cmiss_graphic_streamlines_get_stream_vector_field(streamlines);
			if (stream_vector_chooser == NULL)
			{
				stream_vector_chooser =
						new Managed_object_chooser<Computed_field,MANAGER_CLASS(Computed_field)>
						(stream_vector_chooser_panel, stream_vector_field, region_tree_viewer->field_manager,
							Computed_field_is_stream_vector_capable, (void *)NULL,
							region_tree_viewer->user_interface);
				Callback_base< Computed_field* > *stream_vector_callback =
						new Callback_member_callback< Computed_field*,
						wxRegionTreeViewer, int (wxRegionTreeViewer::*)(Computed_field *) >
						(this, &wxRegionTreeViewer::stream_vector_callback);
				stream_vector_chooser->set_callback(stream_vector_callback);
				stream_vector_chooser_panel->Fit();
				stream_vector_chooser->include_null_item(true);
			}
			stream_vector_chooser->set_object(stream_vector_field);
			Cmiss_field_destroy(&stream_vector_field);

			Cmiss_graphic_streamlines_track_direction streamlines_track_direction =
				Cmiss_graphic_streamlines_get_track_direction(streamlines);
			if (0 == streamlines_track_direction_chooser)
			{
				streamlines_track_direction_chooser = new Enumerator_chooser<ENUMERATOR_TYPE_CLASS(Cmiss_graphic_streamlines_track_direction)>(
					streamlines_track_direction_chooser_panel, streamlines_track_direction,
					(ENUMERATOR_CONDITIONAL_FUNCTION(Cmiss_graphic_streamlines_track_direction) *)NULL,
					(void *)NULL, region_tree_viewer->user_interface);
				streamlines_track_direction_chooser_panel->Fit();
				Callback_base< enum Cmiss_graphic_streamlines_track_direction > *streamlines_track_direction_callback =
					new Callback_member_callback< enum Cmiss_graphic_streamlines_track_direction,
						wxRegionTreeViewer, int (wxRegionTreeViewer::*)(enum Cmiss_graphic_streamlines_track_direction) >(
							this, &wxRegionTreeViewer::streamlines_track_direction_callback);
				streamlines_track_direction_chooser->set_callback(streamlines_track_direction_callback);
			}
			streamlines_track_direction_chooser->set_value(streamlines_track_direction);

			double streamline_length = Cmiss_graphic_streamlines_get_track_length(streamlines);
			sprintf(temp_string,"%g",streamline_length);
			streamlineslengthtextctrl->SetValue(wxString::FromAscii(temp_string));

			streamvectortext->Show();
			stream_vector_chooser_panel->Show();
			streamlines_track_direction_chooser_panel->Show();
			streamlineslengthtext->Show();
			streamlineslengthtextctrl->Show();
		}
		else
		{
			streamvectortext->Hide();
			stream_vector_chooser_panel->Hide();
			streamlines_track_direction_chooser_panel->Hide();
			streamlineslengthtext->Hide();
			streamlineslengthtextctrl->Hide();
		}
		Cmiss_graphic_streamlines_destroy(&streamlines);

		line_width_text=XRCCTRL(*this, "LineWidthText", wxStaticText);
		line_width_text_ctrl=XRCCTRL(*this, "LineWidthTextCtrl", wxTextCtrl);
		line_width_text->Show();
		line_width_text_ctrl->Show();
		double line_width = Cmiss_graphic_get_render_line_width(graphic);
		sprintf(temp_string, "%g", line_width);
		line_width_text_ctrl->SetValue(wxString::FromAscii(temp_string));
		line_width_text_ctrl->Enable();

		point_size_text=XRCCTRL(*this, "PointSizeText", wxStaticText);
		point_size_text_ctrl=XRCCTRL(*this, "PointSizeTextCtrl", wxTextCtrl);
		point_size_text->Show();
		point_size_text_ctrl->Show();
		double point_size = Cmiss_graphic_get_render_point_size(graphic);
		sprintf(temp_string, "%g", point_size);
		point_size_text_ctrl->SetValue(wxString::FromAscii(temp_string));
		point_size_text_ctrl->Enable();

		streamlinedatatypetext=XRCCTRL(*this, "StreamlineDataTypeText", wxStaticText);
		streamline_data_type_chooser_panel = XRCCTRL(*this,"StreamlineDataTypeChooserPanel",wxPanel);
		spectrumtext=XRCCTRL(*this, "SpectrumText", wxStaticText);
		spectrum_chooser_panel=XRCCTRL(*this,"SpectrumChooserPanel", wxPanel);
		wxStaticText *datatext=XRCCTRL(*this, "DataText", wxStaticText);
		data_chooser_panel=XRCCTRL(*this,"DataChooserPanel",wxPanel);

		if (data_field_chooser == NULL)
		{
			Cmiss_field_id temp_data_field = 0;
			if (region_tree_viewer->current_graphic != NULL)
			{
				temp_data_field = Cmiss_graphic_get_data_field(region_tree_viewer->current_graphic);
			}
			data_field_chooser =
				new Managed_object_chooser<Computed_field,MANAGER_CLASS(Computed_field)>
				(data_chooser_panel,temp_data_field, region_tree_viewer->field_manager,
					Computed_field_has_numerical_components, (void *)NULL, region_tree_viewer->user_interface);
			Callback_base< Computed_field* > *data_field_callback =
				new Callback_member_callback< Computed_field*,
				wxRegionTreeViewer, int (wxRegionTreeViewer::*)(Computed_field *) >
				(this, &wxRegionTreeViewer::data_field_callback);
			data_field_chooser->set_callback(data_field_callback);
			data_chooser_panel->Fit();
			data_field_chooser->include_null_item(true);
			Cmiss_field_destroy(&temp_data_field);
		}
		datatext->Show();
		data_chooser_panel->Show();

		Cmiss_field_id data_field = Cmiss_graphic_get_data_field(graphic);
		data_field_chooser->set_object(data_field);
		if (data_field)
		{
			Cmiss_spectrum_id spectrum = Cmiss_graphic_get_spectrum(graphic);
			spectrum_chooser->set_object(spectrum);
			data_chooser_panel->Enable();
			spectrumtext->Enable();
			spectrum_chooser_panel->Enable();
			Cmiss_spectrum_destroy(&spectrum);
		}

		if (CMISS_GRAPHIC_STREAMLINES==graphic_type)
		{
			streamlinedatatypetext->Show();
			streamline_data_type_chooser_panel->Show();
			enum Streamline_data_type streamline_data_type =
				Cmiss_graphic_get_streamline_data_type(graphic);
			if (streamline_data_type_chooser == NULL)
			{
				streamline_data_type_chooser=
					new Enumerator_chooser<ENUMERATOR_TYPE_CLASS(Streamline_data_type)>
					(streamline_data_type_chooser_panel, streamline_data_type,
						(ENUMERATOR_CONDITIONAL_FUNCTION(Streamline_data_type) *)NULL,
						(void *)NULL, region_tree_viewer->user_interface);
				Callback_base< enum Streamline_data_type > *streamline_data_type_callback =
					new Callback_member_callback< enum Streamline_data_type,
					wxRegionTreeViewer, int (wxRegionTreeViewer::*)(enum Streamline_data_type) >
					(this, &wxRegionTreeViewer::streamline_data_type_callback);
				streamline_data_type_chooser->set_callback(streamline_data_type_callback);
				streamline_data_type_chooser_panel->Fit();
			}
			streamline_data_type_chooser->set_value(streamline_data_type);
		}
		else
		{
			/* set scalar data field & spectrum */
			streamlinedatatypetext->Hide();
			streamline_data_type_chooser_panel->Hide();
		}

		wxStaticText *texturecoordinatestext = XRCCTRL(*this, "TextureCoordinatesText", wxStaticText);
		texture_coordinates_chooser_panel = XRCCTRL(*this, "TextureCoordinatesChooserPanel", wxPanel);
		if ((graphic_type == CMISS_GRAPHIC_SURFACES) ||
			(graphic_type == CMISS_GRAPHIC_CONTOURS) ||
			(graphic_type == CMISS_GRAPHIC_LINES))
		{
			texture_coordinates_chooser_panel->Show();
			texturecoordinatestext->Show();
			Cmiss_field_id texture_coordinate_field = Cmiss_graphic_get_texture_coordinate_field(graphic);
			if (texture_coord_field_chooser == NULL)
			{
				texture_coord_field_chooser =
						new Managed_object_chooser<Computed_field,MANAGER_CLASS(Computed_field)>
				   (texture_coordinates_chooser_panel,texture_coordinate_field, region_tree_viewer->field_manager,
							Computed_field_has_up_to_3_numerical_components, (void *)NULL,
							region_tree_viewer->user_interface);
				Callback_base< Computed_field* > *texture_coord_field_callback =
						new Callback_member_callback< Computed_field*,
					wxRegionTreeViewer, int (wxRegionTreeViewer::*)(Computed_field *) >
						(this, &wxRegionTreeViewer::texture_coord_field_callback);
				texture_coord_field_chooser->set_callback(texture_coord_field_callback);
				texture_coordinates_chooser_panel->Fit();
				texture_coord_field_chooser->include_null_item(true);
			}
			texture_coord_field_chooser->set_object(texture_coordinate_field);
			Cmiss_field_destroy(&texture_coordinate_field);
		}
		else
		{
			texture_coordinates_chooser_panel->Hide();
			texturecoordinatestext->Hide();
		}

		/* render_polygon_mode */
		render_polygon_mode_text=XRCCTRL(*this, "RenderPolygonModeText",wxStaticText);
		render_polygon_mode_chooser_panel=XRCCTRL(*this,"RenderPolygonModeChooserPanel",wxPanel);
		render_polygon_mode_text->Show();
		render_polygon_mode_chooser_panel->Show();
		render_polygon_mode=Cmiss_graphic_get_render_polygon_mode(graphic);
		if (render_polygon_mode_chooser == NULL)
		{
			render_polygon_mode_chooser =
					new Enumerator_chooser<ENUMERATOR_TYPE_CLASS(Cmiss_graphic_render_polygon_mode)>
					(render_polygon_mode_chooser_panel, render_polygon_mode,
						(ENUMERATOR_CONDITIONAL_FUNCTION(Cmiss_graphic_render_polygon_mode) *)NULL,
						(void *)NULL, region_tree_viewer->user_interface);
			render_polygon_mode_chooser_panel->Fit();
			Callback_base< enum Cmiss_graphic_render_polygon_mode > *render_polygon_mode_callback =
					new Callback_member_callback< enum Cmiss_graphic_render_polygon_mode,
					wxRegionTreeViewer, int (wxRegionTreeViewer::*)(enum Cmiss_graphic_render_polygon_mode) >
					(this, &wxRegionTreeViewer::render_polygon_mode_callback);
			render_polygon_mode_chooser->set_callback(render_polygon_mode_callback);
		}
		render_polygon_mode_chooser->set_value(render_polygon_mode);

		exteriorcheckbox=XRCCTRL(*this,"ExteriorCheckBox",wxCheckBox);
		facecheckbox=XRCCTRL(*this, "FaceCheckBox",wxCheckBox);
		facechoice=XRCCTRL(*this, "FaceChoice",wxChoice);
		Cmiss_field_domain_type domain_type = Cmiss_graphic_get_domain_type(graphic);
		exteriorcheckbox->Show();
		facecheckbox->Show();
		facechoice->Show();
		if ((CMISS_FIELD_DOMAIN_ELEMENTS_1D == domain_type) ||
			(CMISS_FIELD_DOMAIN_ELEMENTS_2D == domain_type))
		{
			exteriorcheckbox->Enable();
			facecheckbox->Enable();
		}
		else
		{
			exteriorcheckbox->Disable();
			facecheckbox->Disable();
			facechoice->Disable();
		}
		if (Cmiss_graphic_get_exterior(graphic))
		{
			exteriorcheckbox->SetValue(1);
		}
		else
		{
			exteriorcheckbox->SetValue(0);
		}
		face = Cmiss_graphic_get_face(graphic);
		if (face >= CMISS_ELEMENT_FACE_XI1_0)
		{
			facecheckbox->SetValue(1);
			facechoice->Enable();
			facechoice->SetSelection(static_cast<int>(face) - CMISS_ELEMENT_FACE_XI1_0);
		}
		else
		{
			facecheckbox->SetValue(0);
			facechoice->Disable();
		}
		Cmiss_graphic_contours_destroy(&contours);
		Cmiss_graphic_line_attributes_destroy(&line_attributes);
		Cmiss_graphic_point_attributes_destroy(&point_attributes);
} /*SetGraphic*/

const char *GetRegionPathFromTreeItemId(wxTreeItemId current_item_id)
{
	wxTreeItemId parent_id, root_item_id;
	wxString path_name, current_region_name;

	path_name = region_tree_viewer->testing_tree_ctrl->
		GetItemText(current_item_id);
	root_item_id =
		region_tree_viewer->testing_tree_ctrl->GetRootItem();
	if (current_item_id != root_item_id)
	{
		parent_id =
			region_tree_viewer->testing_tree_ctrl->GetItemParent(current_item_id);
		while (parent_id != root_item_id)
		{
			current_region_name = region_tree_viewer->testing_tree_ctrl->
				GetItemText(parent_id);
			path_name = wxString::FromAscii("/") + path_name;
			path_name = current_region_name + path_name;
			parent_id =
				region_tree_viewer->testing_tree_ctrl->GetItemParent(parent_id);
		}
		path_name = region_tree_viewer->testing_tree_ctrl->
			GetItemText(root_item_id) + path_name;
	}
	return duplicate_string(path_name.mb_str(wxConvUTF8));
}

void TreeControlSelectionChanged(wxTreeEvent &event)
{
	ENTER(TreeControlSelectionChanged);

	struct Cmiss_region *region;
	struct Cmiss_scene *scene;
	int width, height;
	wxTreeItemId new_id= event.GetItem();
	wxCmguiHierachicalTreeItemData* data = NULL;
	wxArrayTreeItemIds array;

	if (new_id.IsOk())
	{
		data = dynamic_cast<wxCmguiHierachicalTreeItemData*>(
			region_tree_viewer->testing_tree_ctrl->GetItemData(new_id));
	}
	if (region_tree_viewer->testing_tree_ctrl->GetSelections(array)
		&& data && (region = data->GetRegion()))
	{
		scene = Cmiss_region_get_scene_internal(region);
		if (scene)
		{
			Region_tree_viewer_set_active_scene(region_tree_viewer, scene);
			region_tree_viewer->lowersplitter->Enable();
			region_tree_viewer->lowersplitter->Show();
			Region_tree_viewer_set_graphic_widgets_for_scene(region_tree_viewer);
			if (region_tree_viewer->sceneediting)
			{
				if (!Cmiss_scene_get_number_of_graphics(scene))
				{
					region_tree_viewer->sceneediting->Hide();
				}
				else
				{
					region_tree_viewer->sceneediting->Show();
				}
			}
			DEACCESS(Cmiss_scene)(&scene);
		}
		wxPanel *rightpanel=XRCCTRL(*this,"RightPanel", wxPanel);
		rightpanel->Layout();
	}
	else
	{
		Region_tree_viewer_set_active_scene(
				region_tree_viewer, NULL);
		if (!region_tree_viewer->graphiclistbox)
			region_tree_viewer->graphiclistbox = XRCCTRL(
				*this, "CmissGraphicListBox",wxCheckListBox);
		region_tree_viewer->graphiclistbox->SetSelection(wxNOT_FOUND);
		region_tree_viewer->graphiclistbox->Clear();
		region_tree_viewer->lowersplitter->Disable();
		region_tree_viewer->lowersplitter->Hide();
		region_tree_viewer->sceneediting->Hide();
		Region_tree_viewer_set_graphic_widgets_for_scene(region_tree_viewer);
	}

	if (region_tree_viewer->lowersplitter)
	{
		region_tree_viewer->lowersplitter->GetSize(&width, &height);
		region_tree_viewer->lowersplitter->SetSize(width-1, height-1);
		region_tree_viewer->lowersplitter->SetSize(width+1, height+1);
	}
	wxPanel *lowest_panel = XRCCTRL(*this, "LowestPanel",wxPanel);
	if (lowest_panel)
	{
		lowest_panel->GetSize(&width, &height);
		lowest_panel->SetSize(width-1, height-1);
		lowest_panel->SetSize(width+1, height+1);
	}

	LEAVE;
}

void SetTreeItemImage(wxTreeItemId current_item_id, int id)
{
	region_tree_viewer->testing_tree_ctrl->SetItemImage(
		current_item_id,id,
		wxTreeItemIcon_Normal);
	region_tree_viewer->testing_tree_ctrl->SetItemImage(
		current_item_id,id,
		wxTreeItemIcon_Selected);
	region_tree_viewer->testing_tree_ctrl->SetItemImage(
		current_item_id,id,
		wxTreeItemIcon_Expanded);
	region_tree_viewer->testing_tree_ctrl->SetItemImage(
		current_item_id,id,
		wxTreeItemIcon_SelectedExpanded);
}

void ShowMenu(wxTreeItemId id, const wxPoint& pt)
{
	const char *full_path = GetRegionPathFromTreeItemId(
		id);
	wxMenu menu(wxString::FromAscii(full_path));
	menu.Append(CmguiTree_MenuItem1, wxT("Turn on tree visibililty"));
	menu.Append(CmguiTree_MenuItem2, wxT("Turn off tree visibililty"));
	PopupMenu(&menu, pt);
	DEALLOCATE(full_path);
}

void OnItemMenu(wxTreeEvent &event)
{
	wxTreeItemId itemId = event.GetItem();
	wxPoint clientpt = event.GetPoint();

	ShowMenu(itemId, clientpt);
	event.Skip();
}

void SetVisibilityOfTreeId(wxTreeItemId current_item_id, bool flag)
{
	struct Cmiss_region *region;
	struct Cmiss_scene *scene;

	wxCmguiHierachicalTreeItemData* data =
		dynamic_cast<wxCmguiHierachicalTreeItemData*>(
			region_tree_viewer->testing_tree_ctrl->GetItemData(current_item_id));
	if ((region = data->GetRegion()))
	{
		scene = Cmiss_region_get_scene_internal(region);
		if (scene)
		{
			Cmiss_scene_set_visibility_flag(scene, flag);
			SetTreeItemImage(current_item_id, !flag);
			DEACCESS(Cmiss_scene)(&scene);
			if (region_tree_viewer->edit_scene)
			{
				Cmiss_scene_set_visibility_flag(
					region_tree_viewer->edit_scene, flag);
			}
		}
	}
}

void PropagateChanges(wxTreeItemId current_item_id, bool flag)
{
	wxTreeItemIdValue cookie;
	wxTreeItemId child_id = region_tree_viewer->testing_tree_ctrl->GetFirstChild(
		current_item_id, cookie);
	while (child_id.IsOk())
	{
			SetVisibilityOfTreeId(child_id, flag);
			PropagateChanges(child_id, flag);
			child_id = region_tree_viewer->testing_tree_ctrl->GetNextChild(
				current_item_id, cookie);
	}
}

void TreeControlImageClicked(wxEvent &event)
{
	wxTreeEvent *tree_event = (wxTreeEvent *)&event;
	SetVisibilityOfTreeId(tree_event->GetItem(),
		!Cmiss_scene_get_visibility_flag(region_tree_viewer->edit_scene));
}

void OnMenuSelectionOn(wxCommandEvent &event)
{
	wxArrayTreeItemIds array;

	USE_PARAMETER(event);

	int num = static_cast<int>(region_tree_viewer->testing_tree_ctrl->GetSelections(array));
	for (int i = 0; i < num; i ++)
	{
		SetVisibilityOfTreeId(array[i], true);
		PropagateChanges(array[i], true);
	}
}

void OnMenuSelectionOff(wxCommandEvent &event)
{
	wxArrayTreeItemIds array;

	USE_PARAMETER(event);

	int num = static_cast<int>(region_tree_viewer->testing_tree_ctrl->GetSelections(array));
	for (int i = 0; i < num; i ++)
	{
		SetVisibilityOfTreeId(array[i], false);
		PropagateChanges(array[i], false);
	}
}

void CloseRegionTreeViewer(wxCloseEvent &event)
{
	ENTER(CloseRegionTreeViewer);
	USE_PARAMETER(event);
	DESTROY(Region_tree_viewer)(region_tree_viewer->region_tree_viewer_address);
	LEAVE;
}

void EditTessellation(wxCommandEvent &event)
{
	USE_PARAMETER(event);
	wxWindow *window = FindWindowById(tessellationWindowID, this);
	if (!window)
	{
		TessellationDialog *dlg = new TessellationDialog(
			region_tree_viewer->tessellationModule, this, wxID_ANY);
		tessellationWindowID = dlg->GetId();
		dlg->Show();
	}
	else
	{
		window->Show();
		window->Raise();
	}
}

DECLARE_DYNAMIC_CLASS(wxRegionTreeViewer);
  DECLARE_EVENT_TABLE();
};

IMPLEMENT_DYNAMIC_CLASS(wxRegionTreeViewer, wxFrame)

BEGIN_EVENT_TABLE(wxRegionTreeViewer, wxFrame)
	EVT_SPLITTER_SASH_POS_CHANGED(XRCID("LowerSplitter"),wxRegionTreeViewer::ResetWindow)
	EVT_COLLAPSIBLEPANE_CHANGED(XRCID("RegionTreeViewerTopCollapsiblePane"), wxRegionTreeViewer::CollapsiblepaneChangedEvent)
	EVT_CHECKBOX(XRCID("AutoCheckBox"),wxRegionTreeViewer::AutoChecked)
	EVT_BUTTON(XRCID("ApplyButton"),wxRegionTreeViewer::ApplyClicked)
	EVT_BUTTON(XRCID("RevertButton"),wxRegionTreeViewer::RevertClicked)
	EVT_CHECKLISTBOX(XRCID("CmissGraphicListBox"), wxRegionTreeViewer::GraphicListBoxChecked)
	EVT_TEXT_ENTER(XRCID("NameTextField"),wxRegionTreeViewer::GraphicEditorNameText)
	EVT_LISTBOX(XRCID("CmissGraphicListBox"), wxRegionTreeViewer::GraphicListBoxClicked)
	EVT_BUTTON(XRCID("CopyButton"),wxRegionTreeViewer::CopyGraphic)
	EVT_BUTTON(XRCID("DeleteButton"),wxRegionTreeViewer::RemoveFromGraphicList)
	EVT_BUTTON(XRCID("UpButton"),wxRegionTreeViewer::MoveUpInGraphicList)
	EVT_BUTTON(XRCID("DownButton"),wxRegionTreeViewer::MoveDownInGraphicList)
	EVT_RADIOBUTTON(XRCID("IsoValueListRadioButton"), wxRegionTreeViewer::EnterIsoScalar)
	EVT_RADIOBUTTON(XRCID("IsoValueSequenceRadioButton"), wxRegionTreeViewer::EnterIsoRange)
	EVT_TEXT_ENTER(XRCID("IsoScalarTextCtrl"),wxRegionTreeViewer::EnterIsoScalar)
	EVT_TEXT_ENTER(XRCID("IsoValueSequenceNumberTextCtrl"),wxRegionTreeViewer::EnterIsoRange)
	EVT_TEXT_ENTER(XRCID("IsoValueSequenceFirstTextCtrl"),wxRegionTreeViewer::EnterIsoRange)
	EVT_TEXT_ENTER(XRCID("IsoValueSequenceLastTextCtrl"),wxRegionTreeViewer::EnterIsoRange)
	EVT_TEXT_ENTER(XRCID("OffsetTextCtrl"),wxRegionTreeViewer::EnterGlyphOffset)
	EVT_TEXT_ENTER(XRCID("BaseGlyphSizeTextCtrl"),wxRegionTreeViewer::EnterGlyphSize)
	EVT_TEXT_ENTER(XRCID("GlyphScaleFactorsTextCtrl"),wxRegionTreeViewer::EnterGlyphScale)
	EVT_TEXT_ENTER(XRCID("LabelOffsetTextCtrl"),wxRegionTreeViewer::EnterLabelOffset)
	EVT_CHECKBOX(XRCID("OrientationScaleCheckBox"),wxRegionTreeViewer::EnterGlyphScale)
	/*overlay disabled
	EVT_CHECKBOX(XRCID("OverlayCheckBox"),wxRegionTreeViewer::OverlayChecked)
	EVT_TEXT_ENTER(XRCID("OverlayTextCtrl"),wxRegionTreeViewer::OverlayEntered)
	*/
	EVT_CHECKBOX(XRCID("SeedElementCheckBox"),wxRegionTreeViewer::SeedElementChecked)
	EVT_TEXT_ENTER(XRCID("XiTextCtrl"),wxRegionTreeViewer::EnterSampleLocation)
	EVT_TEXT_ENTER(XRCID("StreamlinesLengthTextCtrl"),wxRegionTreeViewer::EnterStreamlineLength)
	EVT_TEXT_ENTER(XRCID("LineBaseSizeTextCtrl"),wxRegionTreeViewer::EnterLineBaseSize)
	EVT_TEXT_ENTER(XRCID("LineScaleFactorsTextCtrl"), wxRegionTreeViewer::EnterLineScaleFactors)
	EVT_TEXT_ENTER(XRCID("LineWidthTextCtrl"),wxRegionTreeViewer::EnterLineWidth)
	EVT_TEXT_ENTER(XRCID("PointSizeTextCtrl"),wxRegionTreeViewer::EnterPointSize)
	EVT_CHECKBOX(XRCID("ExteriorCheckBox"),wxRegionTreeViewer::ExteriorChecked)
	EVT_CHECKBOX(XRCID("FaceCheckBox"),wxRegionTreeViewer::FaceChecked)
	EVT_CHOICE(XRCID("FaceChoice"),wxRegionTreeViewer::FaceChosen)
	EVT_TREE_SEL_CHANGED(wxID_ANY, wxRegionTreeViewer::TreeControlSelectionChanged)
	EVT_CUSTOM(wxEVT_TREE_IMAGE_CLICK_EVENT, wxID_ANY, wxRegionTreeViewer::TreeControlImageClicked)
	EVT_TREE_ITEM_MENU(CmguiTree_Ctrl, wxRegionTreeViewer::OnItemMenu)
	EVT_BUTTON(XRCID("TessellationButton"),wxRegionTreeViewer::EditTessellation)
	EVT_MENU(CmguiTree_MenuItem1, wxRegionTreeViewer::OnMenuSelectionOn)
	EVT_MENU(CmguiTree_MenuItem2, wxRegionTreeViewer::OnMenuSelectionOff)
#if defined (__WXMSW__)
	EVT_SIZE(wxRegionTreeViewer::FrameGetSize)
#endif /*!defined (__WXMSW__)*/
	EVT_CLOSE(wxRegionTreeViewer::CloseRegionTreeViewer)
END_EVENT_TABLE()

int Region_tree_viewer_revert_changes(Region_tree_viewer *region_tree_viewer)
{
	int return_code = 0;
	gtMatrix transformation_matrix;
	if (region_tree_viewer && region_tree_viewer->wx_region_tree_viewer)
	{
		return_code = 1;

		if (region_tree_viewer->scene)
		{
			if (!region_tree_viewer->graphiclistbox)
			{
				region_tree_viewer->graphiclistbox =  XRCCTRL(*region_tree_viewer->wx_region_tree_viewer,
					"CmissGraphicListBox",wxCheckListBox);
			}

			int selection=	region_tree_viewer->graphiclistbox->GetSelection();
			region_tree_viewer->graphiclistbox->SetSelection(wxNOT_FOUND);
			region_tree_viewer->graphiclistbox->Clear();
			for_each_graphic_in_Cmiss_scene(region_tree_viewer->scene,
				Region_tree_viewer_add_graphic_item, (void *)region_tree_viewer);
			DEACCESS(Cmiss_scene)(&region_tree_viewer->edit_scene);
			region_tree_viewer->edit_scene = create_editor_copy_Cmiss_scene(region_tree_viewer->scene);
			int num = 	region_tree_viewer->graphiclistbox->GetCount();
			if (selection >= num)
			{
				selection = 0;
			}
			region_tree_viewer->graphiclistbox->SetSelection(selection);
			region_tree_viewer->lowersplitter->Show();
			region_tree_viewer->wx_region_tree_viewer->
				Region_tree_viewer_wx_set_manager_in_choosers(region_tree_viewer);
			Cmiss_graphic *temp_graphic = Cmiss_scene_get_graphic_at_position(
				region_tree_viewer->edit_scene, selection+1);
			region_tree_viewer->wx_region_tree_viewer->Region_tree_viewer_wx_update_current_graphic(temp_graphic);
			region_tree_viewer->wx_region_tree_viewer->Region_tree_viewer_wx_update_graphic_widgets();
			Cmiss_graphic_destroy(&temp_graphic);
			if (region_tree_viewer->transformation_editor)
			{
				Cmiss_scene_get_transformation(region_tree_viewer->scene,
					&transformation_matrix);
				region_tree_viewer->transformation_editor->set_transformation(&transformation_matrix);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Region_tree_viewer_revert_changes.  Missing scene");
			return_code = 0;
		}

		if (return_code)
		{
			wxButton *applybutton;
			wxButton *revertbutton;
			region_tree_viewer->child_edited = 0;
			applybutton = XRCCTRL(*region_tree_viewer->wx_region_tree_viewer, "ApplyButton", wxButton);
			revertbutton = XRCCTRL(*region_tree_viewer->wx_region_tree_viewer,"RevertButton", wxButton);
			applybutton->Disable();
			revertbutton->Disable();
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene Editor Revert Changes.  Invalid argument(s)");
		return_code = 0;
	}
	return (return_code);
}

/***************************************************************************//**
*AW: Reaccess both pointers of region_tree_viewer->scene and
*region_tree_viewer->edit_scene to scene if scene object is
*this also change the current field manager in scene editor.
*
* @param region_tree_viewer scene editor to be modified
* @param scene Currently active scene
*/

void Region_tree_viewer_set_active_scene(
	struct Region_tree_viewer *region_tree_viewer, struct Cmiss_scene *scene)
{
	if (region_tree_viewer->scene)
	{
		if (region_tree_viewer->transformation_callback_flag)
		{
			region_tree_viewer->transformation_editor->set_scene(NULL);
			if (Cmiss_scene_remove_transformation_callback(
					region_tree_viewer->scene,
					Region_tree_viewer_wx_transformation_change,
					(void *) region_tree_viewer))
			{
				region_tree_viewer->transformation_callback_flag = 0;
			}
		}
		if (region_tree_viewer->scene_callback_flag)
		{
			if (Cmiss_scene_remove_callback(region_tree_viewer->scene,
					Region_tree_viewer_wx_scene_change, (void *) region_tree_viewer))
			{
				region_tree_viewer->scene_callback_flag = 0;
			}
		}
	}
	if (region_tree_viewer && scene)
	{
		int previous_selection;
		if (region_tree_viewer->edit_scene
			&& region_tree_viewer->current_graphic)
		{
			previous_selection = Cmiss_scene_get_graphic_position(
				region_tree_viewer->edit_scene,
				region_tree_viewer->current_graphic);
		}
		else
		{
			previous_selection = 0;
		}
		REACCESS(Cmiss_scene)(&region_tree_viewer->scene, scene);
		Cmiss_scene *edit_scene;
		if (region_tree_viewer->scene)
		{
			edit_scene = create_editor_copy_Cmiss_scene(
				region_tree_viewer->scene);
			if (!edit_scene)
			{
				display_message(ERROR_MESSAGE, "Rendition_editor_set_scene.  "
					"Could not copy scene");
			}
		}
		else
		{
			edit_scene = (struct Cmiss_scene *) NULL;
		}
		REACCESS(Cmiss_scene)(&(region_tree_viewer->edit_scene),
			edit_scene);
		DEACCESS(Cmiss_scene)(&edit_scene);
		if (previous_selection == 0)
		{
			previous_selection = 1;
		}
		if (!region_tree_viewer->graphiclistbox)
		{
			region_tree_viewer->graphiclistbox
				= XRCCTRL(*region_tree_viewer->wx_region_tree_viewer, "CmissGraphicListBox",wxCheckListBox);
		}
		region_tree_viewer->graphiclistbox->SetSelection(wxNOT_FOUND);
		region_tree_viewer->graphiclistbox->Clear();
		if (region_tree_viewer->edit_scene)
		{
			if (Cmiss_scene_get_number_of_graphics(
				region_tree_viewer->edit_scene) > 0)
			{
				for_each_graphic_in_Cmiss_scene(region_tree_viewer->edit_scene,
					Region_tree_viewer_add_graphic, (void *) region_tree_viewer);
				int num = Cmiss_scene_get_number_of_graphics(
					region_tree_viewer->edit_scene);
				Cmiss_graphic *temp_graphic = NULL;
				if (num > previous_selection)
				{
					num = previous_selection;
				}
				temp_graphic = Cmiss_scene_get_graphic_at_position(
					region_tree_viewer->edit_scene, num);
				region_tree_viewer->wx_region_tree_viewer->Region_tree_viewer_wx_update_current_graphic(
					temp_graphic);
				region_tree_viewer->graphiclistbox->SetSelection(num - 1);
				REACCESS(Cmiss_graphic)(&region_tree_viewer->current_graphic,
					temp_graphic);
				if (temp_graphic)
					Cmiss_graphic_destroy(&temp_graphic);
				region_tree_viewer->lowersplitter->Enable();
				region_tree_viewer->lowersplitter->Show();
			}
			else
			{
				if (region_tree_viewer->current_graphic)
				{
					DEACCESS(Cmiss_graphic)(&region_tree_viewer->current_graphic);
				}
				region_tree_viewer->lowersplitter->Enable();
				region_tree_viewer->lowersplitter->Show();
				region_tree_viewer->wx_region_tree_viewer->Region_tree_viewer_wx_update_current_graphic(NULL);
			}
			region_tree_viewer->field_manager
				= Cmiss_region_get_Computed_field_manager(Cmiss_scene_get_region(
					region_tree_viewer->edit_scene));
		}
		if (region_tree_viewer->scene)
		{
			gtMatrix transformation_matrix;
			Cmiss_scene_get_transformation(region_tree_viewer->scene,
				&transformation_matrix);
			region_tree_viewer->transformation_editor->set_scene(
				region_tree_viewer->scene);
			region_tree_viewer->transformation_editor->set_transformation(
				&transformation_matrix);
			if (Cmiss_scene_add_transformation_callback(
				region_tree_viewer->scene,
				Region_tree_viewer_wx_transformation_change,
				(void *) region_tree_viewer))
			{
				region_tree_viewer->transformation_callback_flag = 1;
			}
			if (Cmiss_scene_add_callback(region_tree_viewer->scene,
				Region_tree_viewer_wx_scene_change, (void *) region_tree_viewer))
			{
				region_tree_viewer->scene_callback_flag = 1;
			}
		}
	}
	else if (region_tree_viewer)
	{
		REACCESS(Cmiss_scene)(&region_tree_viewer->scene, NULL);
		REACCESS(Cmiss_scene)(&region_tree_viewer->edit_scene, NULL);
		REACCESS(Cmiss_graphic)(&region_tree_viewer->current_graphic, NULL);
		region_tree_viewer->field_manager = NULL;
		region_tree_viewer->wx_region_tree_viewer->Region_tree_viewer_wx_set_manager_in_choosers(region_tree_viewer);
	}
}

/***************************************************************************//**
 *Get and set the display of graphic
 */
static int get_and_set_Cmiss_graphic_widgets(void *region_tree_viewer_void)
{
	Region_tree_viewer *region_tree_viewer = static_cast<Region_tree_viewer*>(region_tree_viewer_void);

	/*for the text field*/
	if (region_tree_viewer->current_graphic)
	{
		char *name = Cmiss_graphic_get_name_internal(region_tree_viewer->current_graphic);
		wxTextCtrl *nametextfield = XRCCTRL(*region_tree_viewer->wx_region_tree_viewer, "NameTextField", wxTextCtrl);
		nametextfield->SetValue(wxString::FromAscii(name));
		DEALLOCATE(name);

		/*for the selected and material chooser*/
		region_tree_viewer->wx_region_tree_viewer->SetBothMaterialChooser(region_tree_viewer->current_graphic);
		region_tree_viewer->wx_region_tree_viewer->SetGraphic(region_tree_viewer->current_graphic);
	}
	wxFrame *frame=XRCCTRL(*region_tree_viewer->wx_region_tree_viewer, "CmguiRegionTreeViewer", wxFrame);
	frame->Layout();
	return 1;
}

/***************************************************************************//**
 * Iterator function for scene_editor_update_Graphic_item.
 */
static int Region_tree_viewer_add_graphic_item(
	struct Cmiss_graphic *graphic, void *region_tree_viewer_void)
{
	char *graphic_string;
	int return_code;
	struct Region_tree_viewer *region_tree_viewer;
	ENTER(Region_tree_viewer_add_graphic_item);
	if (graphic && (region_tree_viewer = static_cast<Region_tree_viewer*>(region_tree_viewer_void)))
	{
		graphic_string = Cmiss_graphic_get_summary_string(graphic);
		wxCheckListBox *graphicalitemschecklist =  XRCCTRL(*region_tree_viewer->wx_region_tree_viewer, "CmissGraphicListBox",wxCheckListBox);
		graphicalitemschecklist->Append(wxString::FromAscii(graphic_string));
		if (Cmiss_graphic_get_visibility_flag(graphic))
		{
				graphicalitemschecklist->Check((graphicalitemschecklist->GetCount()-1), true);
		}
		DEALLOCATE(graphic_string);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Region_tree_viewer_add_graphic_item.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Region_tree_viewer_add_graphic_item */

#endif /* defined (WX_USER_INTERFACE) */

void Region_tree_viewer_region_tree_region_add_child(Region_tree_viewer *region_tree_viewer, Cmiss_region *parent_region,
	   wxTreeItemId parent)
{
	char *child_name;
	wxTreeItemId current;
	Cmiss_region *current_region;

	current_region = Cmiss_region_get_first_child(parent_region);
	while (current_region)
	{
		child_name = Cmiss_region_get_name(current_region);
		if (child_name)
		{
			current = region_tree_viewer->testing_tree_ctrl->AppendItem(parent,wxString::FromAscii(child_name),0,0);
			Region_tree_viewer_region_tree_region_add_child(region_tree_viewer,current_region,current);
			DEALLOCATE(child_name);
		}
		Cmiss_region_reaccess_next_sibling(&current_region);
	}

	LEAVE;
}

void Region_tree_viewer_setup_region_tree(Region_tree_viewer *region_tree_viewer)
{
	wxTreeItemId current;
	char *root_region_path;
	struct Cmiss_scene *scene;

	ENTER(Region_tree_viewer_setup_region_tree);

	root_region_path = Cmiss_region_get_root_region_path();
	if (root_region_path)
	{
		current = region_tree_viewer->testing_tree_ctrl->AddRoot(wxString::FromAscii(root_region_path),0,0);
		region_tree_viewer->testing_tree_ctrl->SetTreeIdRegionWithCallback(
			current, region_tree_viewer->root_region);
		region_tree_viewer->testing_tree_ctrl->add_all_child_regions_to_tree_item(current);
		scene = Cmiss_region_get_scene_internal(region_tree_viewer->root_region);
		REACCESS(Cmiss_scene)(&region_tree_viewer->scene,
			scene);
		DEACCESS(Cmiss_scene)(&scene);
		DEALLOCATE(root_region_path);
	}

	LEAVE;
}

/*
Global functions
----------------
*/

struct Region_tree_viewer *CREATE(Region_tree_viewer)(
	struct Region_tree_viewer **region_tree_viewer_address,
	struct Cmiss_graphics_module *graphics_module,
	struct Cmiss_region *root_region,
	struct MANAGER(Graphical_material) *graphical_material_manager,
	struct Graphical_material *default_material,
	struct Cmiss_font *default_font,
	Cmiss_glyph_module *glyphModule,
	struct MANAGER(Spectrum) *spectrum_manager,
	struct MANAGER(VT_volume_texture) *volume_texture_manager,
	struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 2 Febuary 2007

DESCRIPTION :
==============================================================================*/
{
	struct Region_tree_viewer *region_tree_viewer;

	ENTER(CREATE(Region_tree_viewer));
	region_tree_viewer = (struct Region_tree_viewer *)NULL;
	if (graphics_module && root_region &&
		graphical_material_manager && default_material &&
		glyphModule && spectrum_manager &&
		volume_texture_manager && user_interface)
	{
		if (ALLOCATE(region_tree_viewer,struct Region_tree_viewer,1))
		{
			region_tree_viewer->auto_apply = 1;
			region_tree_viewer->child_edited =1;
			region_tree_viewer->child_expanded=1;
			region_tree_viewer->transformation_expanded=1;
			region_tree_viewer->transformation_callback_flag = 0;
			region_tree_viewer->gt_element_group_callback_flag = 0;
			region_tree_viewer->scene_callback_flag = 0;
			region_tree_viewer->graphics_module = graphics_module;
			region_tree_viewer->tessellationModule = Cmiss_graphics_module_get_tessellation_module(graphics_module);
			region_tree_viewer->graphical_material_manager = graphical_material_manager;
			region_tree_viewer->region_tree_viewer_address = (struct Region_tree_viewer **)NULL;
			region_tree_viewer->default_material=default_material;
			region_tree_viewer->selected_material=default_material;
			region_tree_viewer->default_font=default_font;
			region_tree_viewer->glyphModule = Cmiss_glyph_module_access(glyphModule);
			region_tree_viewer->user_interface=user_interface;
			region_tree_viewer->current_graphic = (Cmiss_graphic *)NULL;
			region_tree_viewer->volume_texture_manager=volume_texture_manager;
			region_tree_viewer->field_manager=(MANAGER(Computed_field)*)NULL;
			region_tree_viewer->font_manager=Cmiss_graphics_module_get_font_manager(graphics_module);
			region_tree_viewer->select_mode=(Graphics_select_mode)NULL;
			region_tree_viewer->domain_type = CMISS_FIELD_DOMAIN_TYPE_INVALID;
			region_tree_viewer->sample_mode= (Cmiss_element_point_sample_mode)NULL;
			region_tree_viewer->streamline_data_type=(Streamline_data_type)NULL;
			region_tree_viewer->spectrum_manager=spectrum_manager;
			region_tree_viewer->render_polygon_mode =(Cmiss_graphic_render_polygon_mode)NULL;
			region_tree_viewer->fe_element =(FE_element *)NULL;
			region_tree_viewer->root_region = root_region;
			region_tree_viewer->current_region = NULL;
			region_tree_viewer->wx_region_tree_viewer = (wxRegionTreeViewer *)NULL;
			region_tree_viewer->graphiclistbox = (wxCheckListBox *)NULL;
			region_tree_viewer->scene = (Cmiss_scene *)NULL;
			region_tree_viewer->edit_scene = (Cmiss_scene *)NULL;
			wxLogNull logNo;
			region_tree_viewer->wx_region_tree_viewer = new
				wxRegionTreeViewer(region_tree_viewer);
			region_tree_viewer->frame=
				XRCCTRL(*region_tree_viewer->wx_region_tree_viewer, "CmguiRegionTreeViewer", wxFrame);
			region_tree_viewer->frame->SetMinSize(wxSize(50,100));
			region_tree_viewer->frame->SetSize(wxSize(600,800));
			region_tree_viewer->frame->SetMaxSize(wxSize(2000,2000));
			region_tree_viewer->lowersplitter=XRCCTRL(*region_tree_viewer->wx_region_tree_viewer,"LowerSplitter",wxSplitterWindow);
			region_tree_viewer->top_collpane = XRCCTRL(*region_tree_viewer->wx_region_tree_viewer,
				"RegionTreeViewerTopCollapsiblePane", wxCollapsiblePane);
			wxWindow *top_collpane_win = region_tree_viewer->top_collpane->GetPane();
			region_tree_viewer->top_collpane_panel = new wxPanel(top_collpane_win);
			region_tree_viewer->transformation_editor = new Transformation_editor(
				region_tree_viewer->top_collpane_panel, "transformation_editor", NULL,
				&region_tree_viewer->auto_apply);
			wxSizer *top_collpane_sizer = new wxBoxSizer(wxVERTICAL);
			top_collpane_sizer->Add(region_tree_viewer->top_collpane_panel, 1, wxEXPAND|wxALL, 2);
			top_collpane_win->SetSizer(top_collpane_sizer);
			top_collpane_sizer->SetSizeHints(top_collpane_win);
			top_collpane_sizer->Layout();
			top_collpane_win->Layout();
			region_tree_viewer->top_collpane->Collapse(1);
			region_tree_viewer->autocheckbox =
				XRCCTRL(*region_tree_viewer->wx_region_tree_viewer, "AutoCheckBox", wxCheckBox);
			region_tree_viewer->autocheckbox->SetValue(true);
			region_tree_viewer->applybutton =
				XRCCTRL(*region_tree_viewer->wx_region_tree_viewer, "ApplyButton", wxButton);
			region_tree_viewer->applybutton->Disable();
			region_tree_viewer->revertbutton =
					XRCCTRL(*region_tree_viewer->wx_region_tree_viewer, "RevertButton", wxButton);
			region_tree_viewer->revertbutton->Disable();
			region_tree_viewer->frame->Layout();
			region_tree_viewer->sceneediting =
				XRCCTRL(*region_tree_viewer->wx_region_tree_viewer, "SceneEditing", wxScrolledWindow);
			region_tree_viewer->graphiclistbox =
				XRCCTRL(*region_tree_viewer->wx_region_tree_viewer, "CmissGraphicListBox",wxCheckListBox);
			region_tree_viewer->sceneediting->Layout();
			region_tree_viewer->sceneediting->SetScrollbars(10,10,40,40);
			region_tree_viewer->verticalsplitter=XRCCTRL(*region_tree_viewer->wx_region_tree_viewer,"VerticalSplitter",wxSplitterWindow);
			region_tree_viewer->verticalsplitter->SetSashPosition(150);
			region_tree_viewer->lowersplitter->SetSashPosition(160);
			region_tree_viewer->lowersplitter->Layout();
			region_tree_viewer->lowersplitter->Hide();
			region_tree_viewer->verticalsplitter->Layout();
			region_tree_viewer->lowersplitter->SetSashPosition(100);
			wxPanel *lowest_panel = XRCCTRL(*region_tree_viewer->wx_region_tree_viewer,
				"LowestPanel",wxPanel);
			lowest_panel->SetMinSize(wxSize(-1,150));
			region_tree_viewer->lowersplitter->Layout();
			region_tree_viewer->lowersplitter->Hide();
			region_tree_viewer->verticalsplitter->Layout();
			wxPanel *rightpanel=XRCCTRL(*region_tree_viewer->wx_region_tree_viewer, "RightPanel", wxPanel);
			rightpanel->Layout();
			wxPanel *tree_control_panel =
				XRCCTRL(*region_tree_viewer->wx_region_tree_viewer,"TreeControlPanel",wxPanel);
			region_tree_viewer->testing_tree_ctrl =
				new wxCmguiHierachicalTree(region_tree_viewer->wx_region_tree_viewer, tree_control_panel);
			region_tree_viewer->ImageList = new wxImageList(13,13);
			region_tree_viewer->ImageList->Add(wxIcon(tickbox_xpm));
			region_tree_viewer->ImageList->Add(wxIcon(unticked_box_xpm));
			region_tree_viewer->testing_tree_ctrl->AssignImageList(region_tree_viewer->ImageList);
			Region_tree_viewer_setup_region_tree(region_tree_viewer);
			region_tree_viewer->testing_tree_ctrl->ExpandAll();
			tree_control_panel->Layout();
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"CREATE(Region_tree_viewer).  Could not allocate editor structure");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"CREATE(Region_tree_viewer).  Invalid argument(s)");
	}
	if (region_tree_viewer_address && region_tree_viewer)
	{
		region_tree_viewer->region_tree_viewer_address = region_tree_viewer_address;
		*region_tree_viewer_address = region_tree_viewer;
	}
	LEAVE;

	return (region_tree_viewer);
} /* CREATE(Region_tree_viewer_wx) */

int DESTROY(Region_tree_viewer)(struct Region_tree_viewer **region_tree_viewer_address)
/*******************************************************************************
LAST MODIFIED : 16 November 2001

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	struct Region_tree_viewer *region_tree_viewer;
	ENTER(DESTROY(Region_tree_viewer));
	if (region_tree_viewer_address && (region_tree_viewer = *region_tree_viewer_address) &&
		(region_tree_viewer->region_tree_viewer_address == region_tree_viewer_address))
	{
		if (region_tree_viewer->current_graphic)
			DEACCESS(Cmiss_graphic)(&region_tree_viewer->current_graphic);
		if (region_tree_viewer->edit_scene)
			DEACCESS(Cmiss_scene)(&region_tree_viewer->edit_scene);
		if (region_tree_viewer->scene)
		{
			region_tree_viewer->transformation_editor->set_scene(NULL);
			if (region_tree_viewer->transformation_callback_flag &&
				Cmiss_scene_remove_transformation_callback(region_tree_viewer->scene,
					Region_tree_viewer_wx_transformation_change, (void *)region_tree_viewer))
			{
				region_tree_viewer->transformation_callback_flag = 0;
			}
			if (region_tree_viewer->scene_callback_flag)
			{
				if (Cmiss_scene_remove_callback(region_tree_viewer->scene,
						Region_tree_viewer_wx_scene_change, (void *)region_tree_viewer))
				{
					region_tree_viewer->scene_callback_flag = 0;
				}
			}
		}
		if (region_tree_viewer->scene)
			DEACCESS(Cmiss_scene)(&region_tree_viewer->scene);
		Cmiss_glyph_module_destroy(&region_tree_viewer->glyphModule);
		delete region_tree_viewer->transformation_editor;
		delete region_tree_viewer->wx_region_tree_viewer;
		Cmiss_tessellation_module_destroy(&region_tree_viewer->tessellationModule);
		DEALLOCATE(*region_tree_viewer_address);
		*region_tree_viewer_address = NULL;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(Region_tree_viewer).  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Region_tree_viewer) */

int Region_tree_viewer_bring_to_front(struct Region_tree_viewer *region_tree_viewer)
/*******************************************************************************
LAST MODIFIED : 31 October 2001

DESCRIPTION :
De-iconifies and brings the scene editor to the front.
==============================================================================*/
{
	int return_code;

	ENTER(Region_tree_viewer_bring_to_front);
	if (region_tree_viewer)
	{
		/* bring up the dialog */
		region_tree_viewer->wx_region_tree_viewer->Raise();
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Region_tree_viewer_bring_to_front.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Region_tree_viewer_bring_to_front */

/***************************************************************************//**
* Setup the graphic widgets.
*
* @param region_tree_viewer scene editor to be be modified
*/
void Region_tree_viewer_set_graphic_widgets_for_scene(Region_tree_viewer *region_tree_viewer)
{
	ENTER(Region_tree_viewer_set_graphic_widgets_for_scene);
	if (region_tree_viewer && region_tree_viewer->scene)
	{
		region_tree_viewer->wx_region_tree_viewer->
			Region_tree_viewer_wx_set_manager_in_choosers(region_tree_viewer);
		get_and_set_Cmiss_graphic_widgets((void *)region_tree_viewer);
		region_tree_viewer->lowersplitter->Enable();
		region_tree_viewer->lowersplitter->Show();
	}
	LEAVE;
}

static int Region_tree_viewer_wx_scene_change(
	struct Cmiss_scene *scene, void *region_tree_viewer_void)
{
	Region_tree_viewer *region_tree_viewer;
	int return_code;
	//	wxCheckListBox *cmiss_graphic_checklist;

	if (scene &&
		(region_tree_viewer = (struct Region_tree_viewer *)region_tree_viewer_void))
	{
		return_code = 1;
		if (!Cmiss_scenes_match(
					scene, region_tree_viewer->edit_scene))
		{
			if (region_tree_viewer->auto_apply)
			{
				Region_tree_viewer_revert_changes(region_tree_viewer);
			}
			else
			{
				wxButton *applybutton;
				wxButton *revertbutton;
				region_tree_viewer->child_edited = 0;
				applybutton = XRCCTRL(*region_tree_viewer->wx_region_tree_viewer, "ApplyButton", wxButton);
				revertbutton = XRCCTRL(*region_tree_viewer->wx_region_tree_viewer,"RevertButton", wxButton);
				applybutton->Enable();
				revertbutton->Enable();
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Region_tree_viewer_wx_scene_change.  Invalid argument(s)");
		return_code = 0;
	}
	return return_code;
}

static int Region_tree_viewer_add_graphic(
	struct Cmiss_graphic *graphic, void *region_tree_viewer_void)
{
	char *graphic_string;
	int return_code;
	struct Region_tree_viewer *region_tree_viewer;
	ENTER(Region_tree_viewer_add_graphic);
	if (graphic && (region_tree_viewer = static_cast<Region_tree_viewer*>(region_tree_viewer_void)))
	{
		graphic_string = Cmiss_graphic_get_summary_string(graphic);
		wxCheckListBox *graphicchecklist =  XRCCTRL(*region_tree_viewer->wx_region_tree_viewer, "CmissGraphicListBox",wxCheckListBox);
		graphicchecklist->Append(wxString::FromAscii(graphic_string));
		if (Cmiss_graphic_get_visibility_flag(graphic))
		{
			graphicchecklist->Check((graphicchecklist->GetCount()-1), true);
		}
		DEALLOCATE(graphic_string);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Region_tree_viewer_add_graphic.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Region_tree_viewer_add_element_graphic_item */
