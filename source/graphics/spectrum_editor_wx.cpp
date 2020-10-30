/* OpenCMISS-Cmgui Application
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
/*******************************************************************************
FILE : spectrum_editor_wx.cpp

LAST MODIFIED : 22 Aug 2007
DESCRIPTION:
Provides the wxWidgets interface to manipulate spectrum settings.
==============================================================================*/

#if 1
#include "configure/cmgui_configure.h"
#endif /* defined (1) */

#include <stdio.h>
#include <math.h>

#include "opencmiss/zinc/glyph.h"
#include "opencmiss/zinc/scene.h"
#include "opencmiss/zinc/material.h"
#include "opencmiss/zinc/scene.h"
#include "command/parser.h"
#include "general/debug.h"
#include "graphics/glyph_colour_bar.hpp"
#include "graphics/graphics.h"
#include "graphics/graphics_module.h"
#include "graphics/scene.h"
#include "graphics/scene_app.h"
#include "graphics/scene_viewer.h"
#include "graphics/scenefilter.hpp"
#include "graphics/spectrum.h"
#include "graphics/spectrum_component.h"
#include "graphics/spectrum_component_app.h"
#include "graphics/spectrum_editor_wx.h"
#include "graphics/spectrum_editor_dialog_wx.h"
#include "region/cmiss_region.hpp"
#include "three_d_drawing/graphics_buffer.h"
#include "general/message.h"
#include "user_interface/user_interface.h"
// insert app headers here
#include "three_d_drawing/graphics_buffer_app.h"
#include "graphics/scene_viewer_app.h"

/* SAB Trying to hide the guts of GT_object and its primitives,
	however the spectrum editor is modifying it's primitives quite a bit . */
#include "region/cmiss_region_chooser_wx.hpp"
#include "graphics/graphics_object_private.hpp"
#include "wx/wx.h"
#include <wx/splitter.h>
#include "wx/xrc/xmlres.h"
#include "choose/choose_manager_listbox_class.hpp"
#include "graphics/spectrum_editor_wx.xrch"
#include "icon/cmiss_icon.xpm"
#include "choose/choose_manager_class.hpp"

class wxSpectrumEditor;

struct Spectrum_editor
/*******************************************************************************
LAST MODIFIED : 22 August 2007

DESCRIPTION :
Contains all the information carried by the graphical element editor widget.
==============================================================================*/
{
	/* This editorMaterial is used when displaying the spectrum in the
		3d widget */
	 cmzn_material *editorMaterial, *labelMaterial;
	 struct cmzn_spectrumcomponent *current_settings;
	 struct cmzn_spectrum *edit_spectrum, *current_spectrum;
	 struct MANAGER(cmzn_spectrum) *spectrum_manager;
	 struct User_interface *user_interface;
	 struct Spectrum_editor_dialog **spectrum_editor_dialog_address;
	 cmzn_scene *spectrum_editor_scene, *autorange_scene;
	 cmzn_region_id root_region;
	 struct Scene_viewer_app *spectrum_editor_scene_viewer;
	 cmzn_glyph_colour_bar *colourBar;
	 int viewer_type;
	 struct cmzn_region *private_region;
	 wxSpectrumEditor *wx_spectrum_editor;
	 wxPanel *spectrum_panel, *spectrum_listbox_panel;
	 wxFrame *spectrum_editor_frame;
	 wxTextCtrl *spectrum_range_min_text, *spectrum_range_max_text,
			*spectrum_exaggeration_text, *spectrum_data_component_text,
			*spectrum_number_of_bands_text, *spectrum_ratio_of_black_bands_text,
			*spectrum_editor_step_value_text, *spectrum_normalised_colour_range_min_text,
			*spectrum_normalised_colour_range_max_text;
	 wxCheckListBox *spectrum_settings_checklist;
	 wxButton *spectrum_settings_add_button, *spectrum_settings_del_button,
			*spectrum_settings_up_button, *spectrum_settings_dn_button;
	 wxCheckBox *spectrum_reverse_checkbox, *spectrum_extend_below_check,
			*spectrum_extend_above_check, *spectrum_fix_minimum_check,
			*spectrum_fix_maximum_check;
	 wxChoice *spectrum_colour_mapping_type_choice, *spectrum_type_choice;
	 wxRadioBox *spectrum_overwrite_colour_radiobox, *spectrum_left_right_radio_box;
	 wxScrolledWindow *spectrum_higher_panel, *spectrum_lower_panel;
	 void *material_manager_callback_id;
	 void *spectrum_manager_callback_id;
	 cmzn_scenefiltermodule_id filter_module;
};


/* prototype */
static int make_current_spectrum(
	struct Spectrum_editor *spectrum_editor,
	struct cmzn_spectrum *spectrum);
/******************************************************************************
LAST MODIFIED : 7 January 2008

DESCRIPTION :
Destroys the edit_spectrum member of <spectrum_editor> and rebuilds it as
a complete copy of <Spectrum>.
==============================================================================*/

/**
 * Updates the scene with a colour bar matching the current edit_spectrum.
 */
static int make_colour_bar(struct Spectrum_editor *spectrum_editor)
{
	int return_code = 0;
	if (spectrum_editor && spectrum_editor->spectrum_editor_scene && spectrum_editor->edit_spectrum)
	{
		cmzn_scene_begin_change(spectrum_editor->spectrum_editor_scene);
		if (spectrum_editor->colourBar)
		{
			cmzn_glyph_colour_bar_destroy(&spectrum_editor->colourBar);
		}
		spectrum_editor->colourBar = cmzn_glyph_colour_bar::create(spectrum_editor->edit_spectrum);
		spectrum_editor->colourBar->setName("Spectrum Editor Colour Bar");
		spectrum_editor->colourBar->setNumberFormat("%.4g");
		const double axis[3] = { 1.0, 0.0, 0.0 };
		spectrum_editor->colourBar->setAxis(3, axis);
		const double sideAxis[3] = { 0.0, -0.2, 0.0 };
		spectrum_editor->colourBar->setSideAxis(3, sideAxis);
		//spectrum_editor->colourBar->setExtendLength(0.3);
		//spectrum_editor->colourBar->setTickLength(0.3);
		int labelDivisions = 4;
		switch (spectrum_editor->viewer_type % 3)
		{
			case 0:
			{
				labelDivisions = 4;
			} break;
			case 1:
			{
				labelDivisions = 10;
			} break;
			default:
			{
				labelDivisions = 2;
			} break;
		}
		if (spectrum_editor->viewer_type == 0)
		{
			struct Colour white = {1.0, 1.0, 1.0};
			Graphical_material_set_ambient(spectrum_editor->labelMaterial, &white );
			Graphical_material_set_diffuse(spectrum_editor->labelMaterial, &white );
			cmzn_sceneviewer_set_background_colour_component_rgb(
				spectrum_editor->spectrum_editor_scene_viewer->core_scene_viewer, 0.0, 0.0, 0.0);
		}
		else if (spectrum_editor->viewer_type == 3)
		{
			struct Colour black = {0, 0, 0};
			Graphical_material_set_ambient(spectrum_editor->labelMaterial, &black );
			Graphical_material_set_diffuse(spectrum_editor->labelMaterial, &black );
			cmzn_sceneviewer_set_background_colour_component_rgb(
				spectrum_editor->spectrum_editor_scene_viewer->core_scene_viewer, 1.0, 1.0, 1.0);
		}
		spectrum_editor->colourBar->setLabelDivisions(labelDivisions);
		spectrum_editor->colourBar->setLabelMaterial(spectrum_editor->labelMaterial);

		cmzn_graphics *graphics = cmzn_scene_get_first_graphics(spectrum_editor->spectrum_editor_scene);
		if (!graphics)
		{
			graphics = cmzn_scene_create_graphics_points(spectrum_editor->spectrum_editor_scene);
			cmzn_graphics_set_name(graphics, "Spectrum Editor Colour Bar");
			cmzn_graphics_set_material(graphics, spectrum_editor->editorMaterial);
		}
		cmzn_graphicspointattributes_id pointAttr = cmzn_graphics_get_graphicspointattributes(graphics);
		cmzn_graphicspointattributes_set_glyph(pointAttr, spectrum_editor->colourBar);
		const double one = 1.0;
		cmzn_graphicspointattributes_set_base_size(pointAttr, 1, &one);
		cmzn_graphicspointattributes_destroy(&pointAttr);
		cmzn_graphics_destroy(&graphics);
		cmzn_scene_end_change(spectrum_editor->spectrum_editor_scene);
		Scene_viewer_app_redraw(spectrum_editor->spectrum_editor_scene_viewer);
		if (spectrum_editor->spectrum_panel)
		{
			spectrum_editor->spectrum_panel->Update();
		}
	}
	return return_code;
}

static int make_edit_spectrum(
	struct Spectrum_editor *spectrum_editor,
	struct cmzn_spectrum *spectrum)
/*******************************************************************************
LAST MODIFIED : 10 March 1998

DESCRIPTION :
Destroys the edit_spectrum member of <spectrum_editor> and rebuilds it as
a complete copy of <Spectrum>.
==============================================================================*/
{
	int return_code;

	ENTER(make_edit_spectrum);
	/* check arguments */
	if (spectrum_editor&&spectrum)
	{
		/* destroy current edit_spectrum */
		if (spectrum_editor->edit_spectrum)
		{
			DEACCESS(cmzn_spectrum)(&(spectrum_editor->edit_spectrum));
		}
		/* make an empty spectrum */
		spectrum_editor->edit_spectrum = cmzn_spectrum_create_private();
		cmzn_spectrum_set_name(spectrum_editor->edit_spectrum, "copy");
		if (spectrum_editor->edit_spectrum)
		{
			/* copy general settings into new object */
			MANAGER_COPY_WITHOUT_IDENTIFIER(cmzn_spectrum,name)
				(spectrum_editor->edit_spectrum,spectrum);

			spectrum_editor->spectrum_overwrite_colour_radiobox->SetSelection(
				 cmzn_spectrum_is_material_overwrite(spectrum_editor->edit_spectrum));

			make_colour_bar(spectrum_editor);

			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"make_edit_spectrum.  Could not make copy of spectrum");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"make_edit_spectrum.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* make_edit_spectrum */

static int create_settings_item_wx(struct cmzn_spectrumcomponent *settings,
	void *spectrum_editor_void)
/*******************************************************************************
LAST MODIFIED : 30 August 2007

DESCRIPTION :
Clears then fills the settings list RowCol with descriptions of the settings
of the type spectrum_editor->settings_type.
==============================================================================*/
{
	 int num_children,return_code;
	 struct Spectrum_editor *spectrum_editor;
	 char *settings_string;

	 ENTER(create_settings_item_wx);
	 if (settings&&(spectrum_editor=
				 (struct Spectrum_editor *)spectrum_editor_void))
	 {
			settings_string=cmzn_spectrumcomponent_string(settings,
						SPECTRUM_COMPONENT_STRING_COMPLETE_PLUS);
			if (settings_string != 0)
			{
				 if (spectrum_editor->spectrum_settings_checklist)
				 {
						spectrum_editor->spectrum_settings_checklist->Append(wxString::FromAscii(settings_string));
						num_children = spectrum_editor->spectrum_settings_checklist->GetCount();
						if (cmzn_spectrumcomponent_is_active(settings))
						{
							 spectrum_editor->spectrum_settings_checklist->Check(num_children-1, true);
						}
						else
						{
							 spectrum_editor->spectrum_settings_checklist->Check(num_children-1, false);
						}
						return_code = 1;
				 }
				 else
				 {
						display_message(ERROR_MESSAGE,
							 "create_settings_item_wx. Invalid checklist");
						return_code = 0;
				 }
			}
			else
			{
				 display_message(ERROR_MESSAGE,
						"create_settings_item_wx.  Could not get settings string");
				 return_code=0;
			}
			DEALLOCATE(settings_string);
	 }
	 else
	 {
			display_message(ERROR_MESSAGE,
				 "create_settings_item_wx.  Invalid argument(s)");
			return_code=0;
	 }
	 LEAVE;

	 return (return_code);
}

static int spectrum_editor_wx_make_settings_list(
	struct Spectrum_editor *spectrum_editor)
/*******************************************************************************
LAST MODIFIED : 30 August 2007

DESCRIPTION :
Clears then fills the settings list RowColumn with descriptions of the settings
of the type spectrum_editor->settings_type.
==============================================================================*/
{
	 int return_code, selection, num_of_items;

	 ENTER(spectrum_editor_wx_make_settings_list);
	 if (spectrum_editor)
	 {
			if (spectrum_editor->spectrum_settings_checklist)
			{
				 selection = spectrum_editor->spectrum_settings_checklist->GetSelection();
				 spectrum_editor->spectrum_settings_checklist->Clear();
				 FOR_EACH_OBJECT_IN_LIST(cmzn_spectrumcomponent)(
						create_settings_item_wx,(void *)spectrum_editor,
						get_cmzn_spectrumcomponent_list( spectrum_editor->edit_spectrum));
				 num_of_items = spectrum_editor->spectrum_settings_checklist->GetCount();
				 if (num_of_items>=selection+1)
				 {
						spectrum_editor->spectrum_settings_checklist->SetSelection(selection);
				 }
				 else
				 {
						spectrum_editor->spectrum_settings_checklist->SetSelection(selection-1);
				 }
				 return_code=1;
			}
			else
			{
				 display_message(ERROR_MESSAGE,
						"spectrum_editor_wx_make_settings_list. Invalid checklist");
				 return_code = 0;
			}
	 }
	else
	{
		display_message(ERROR_MESSAGE,
			"spectrum_editor_wx_make_settings_list.  Invalid argument(s)");
		return_code=0;
	}
	 LEAVE;

	return (return_code);
} /* spectrum_editor_wx_make_settings_list */

static int spectrum_editor_wx_set_type(
	struct Spectrum_editor *spectrum_editor)
/*******************************************************************************
LAST MODIFIED : 13 March 1998

DESCRIPTION :
Sets the current settings type on the choose menu according to the
current settings in spectrum_editor_settings.
==============================================================================*/
{
	enum cmzn_spectrumcomponent_scale_type current_type;
	int return_code = 1;

	ENTER(spectrum_editor_settings_set_type);
	if (spectrum_editor &&
		 spectrum_editor->current_settings &&
		 spectrum_editor->spectrum_type_choice)
	{
		current_type = cmzn_spectrumcomponent_get_scale_type(
			spectrum_editor->current_settings);
		switch (current_type)
		{
			case CMZN_SPECTRUMCOMPONENT_SCALE_TYPE_LINEAR:
			{
				spectrum_editor->spectrum_type_choice->SetStringSelection(wxT("Linear"));
			} break;
			case CMZN_SPECTRUMCOMPONENT_SCALE_TYPE_LOG:
			{
				spectrum_editor->spectrum_type_choice->SetStringSelection(wxT("Log"));
			} break;
			default:
			{
			 /* Do nothing */
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"spectrum_editor_wx_set_type.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* spectrum_editor_settings_set_type */

int spectrum_editor_wx_set_settings(struct Spectrum_editor *spectrum_editor)
/*******************************************************************************
LAST MODIFIED : 30 August 2007

DESCRIPTION :
Changes the currently chosen settings.
==============================================================================*/
{
	 char temp_string[50];
	 enum cmzn_spectrumcomponent_scale_type type;
	 enum cmzn_spectrumcomponent_colour_mapping_type colour_mapping_type = CMZN_SPECTRUMCOMPONENT_COLOUR_MAPPING_TYPE_ALPHA;
	 float exaggeration, step_value, band_ratio;
	 int component, number_of_bands, black_band_proportion; //,i, num_children, number_of_bands, black_band_proportion,
	 bool extend, fix;
	 int return_code;
	 struct cmzn_spectrumcomponent *new_settings;

	 ENTER(spectrum_editor_wx_set_settings);
	 if (spectrum_editor)
	 {
			new_settings = spectrum_editor->current_settings;
			if (new_settings != 0)
			{
				 spectrum_editor_wx_set_type(spectrum_editor);
				 type = cmzn_spectrumcomponent_get_scale_type(new_settings);
				 if (spectrum_editor->spectrum_reverse_checkbox)
				 {
						spectrum_editor->spectrum_reverse_checkbox->SetValue(
							cmzn_spectrumcomponent_is_colour_reverse(new_settings));
				 }
				 if (spectrum_editor->spectrum_colour_mapping_type_choice)
				 {
						colour_mapping_type = cmzn_spectrumcomponent_get_colour_mapping_type(
							 new_settings);
						switch (colour_mapping_type)
						{
							 case CMZN_SPECTRUMCOMPONENT_COLOUR_MAPPING_TYPE_ALPHA:
							 {
									spectrum_editor->spectrum_colour_mapping_type_choice->SetStringSelection(wxT("Alpha"));
							 } break;
							 case CMZN_SPECTRUMCOMPONENT_COLOUR_MAPPING_TYPE_BLUE:
							 {
									spectrum_editor->spectrum_colour_mapping_type_choice->SetStringSelection(wxT("Blue"));
							 } break;
							 case CMZN_SPECTRUMCOMPONENT_COLOUR_MAPPING_TYPE_GREEN:
							 {
									spectrum_editor->spectrum_colour_mapping_type_choice->SetStringSelection(wxT("Green"));
							 } break;
							 case CMZN_SPECTRUMCOMPONENT_COLOUR_MAPPING_TYPE_MONOCHROME:
							 {
									spectrum_editor->spectrum_colour_mapping_type_choice->SetStringSelection(wxT("Monochrome"));
							 } break;
							 case CMZN_SPECTRUMCOMPONENT_COLOUR_MAPPING_TYPE_RAINBOW:
							 {
									spectrum_editor->spectrum_colour_mapping_type_choice->SetStringSelection(wxT("Rainbow"));
							 } break;
							 case CMZN_SPECTRUMCOMPONENT_COLOUR_MAPPING_TYPE_RED:
							 {
									spectrum_editor->spectrum_colour_mapping_type_choice->SetStringSelection(wxT("Red"));
							 } break;
							 case CMZN_SPECTRUMCOMPONENT_COLOUR_MAPPING_TYPE_WHITE_TO_BLUE:
							 {
									spectrum_editor->spectrum_colour_mapping_type_choice->SetStringSelection(wxT("White to blue"));
							 } break;
							 case CMZN_SPECTRUMCOMPONENT_COLOUR_MAPPING_TYPE_WHITE_TO_GREEN:
							 {
									spectrum_editor->spectrum_colour_mapping_type_choice->SetStringSelection(wxT("White to green"));
							 } break;
							 case CMZN_SPECTRUMCOMPONENT_COLOUR_MAPPING_TYPE_STEP:
							 {
									spectrum_editor->spectrum_colour_mapping_type_choice->SetStringSelection(wxT("Step"));
							 } break;
							 case CMZN_SPECTRUMCOMPONENT_COLOUR_MAPPING_TYPE_BANDED:
							 {
									spectrum_editor->spectrum_colour_mapping_type_choice->SetStringSelection(wxT("Contour bands"));
							 } break;
							 case CMZN_SPECTRUMCOMPONENT_COLOUR_MAPPING_TYPE_WHITE_TO_RED:
							 {
									spectrum_editor->spectrum_colour_mapping_type_choice->SetStringSelection(wxT("White to red"));
							 } break;
						}
				 }

				 if (spectrum_editor->spectrum_exaggeration_text)
				 {
						exaggeration = cmzn_spectrumcomponent_get_exaggeration(new_settings);
						sprintf(temp_string,"%10g",fabs(exaggeration));
						spectrum_editor->spectrum_exaggeration_text->SetValue(wxString::FromAscii(temp_string));
						if (exaggeration >= 0.0)
						{
							 spectrum_editor->spectrum_left_right_radio_box->SetSelection(0);
						}
						else
						{
							 spectrum_editor->spectrum_left_right_radio_box->SetSelection(1);
						}
				 }
				 if (CMZN_SPECTRUMCOMPONENT_SCALE_TYPE_LOG == type)
				 {
						spectrum_editor->spectrum_exaggeration_text->Enable(true);
						spectrum_editor->spectrum_left_right_radio_box->Enable(true);
				 }
				 else
				 {
						spectrum_editor->spectrum_exaggeration_text->Enable(false);
						spectrum_editor->spectrum_left_right_radio_box->Enable(false);
				 }
				if (spectrum_editor->spectrum_data_component_text)
				{
					component = cmzn_spectrumcomponent_get_field_component(new_settings);
					sprintf(temp_string,"%d",component);
					spectrum_editor->spectrum_data_component_text->SetValue(wxString::FromAscii(temp_string));
				}
				if (spectrum_editor->spectrum_number_of_bands_text)
				{
					 number_of_bands = cmzn_spectrumcomponent_get_number_of_bands(new_settings);
					 sprintf(temp_string,"%10d",number_of_bands);
					 spectrum_editor->spectrum_number_of_bands_text->SetValue(wxString::FromAscii(temp_string));
				}
				if (spectrum_editor->spectrum_ratio_of_black_bands_text)
				{
					black_band_proportion = cmzn_spectrumcomponent_get_black_band_proportion(new_settings);
					band_ratio = (float)(black_band_proportion) / 1000.0;
					sprintf(temp_string,"%10g",band_ratio);
					spectrum_editor->spectrum_ratio_of_black_bands_text->SetValue(wxString::FromAscii(temp_string));
				}
				if (CMZN_SPECTRUMCOMPONENT_COLOUR_MAPPING_TYPE_BANDED == colour_mapping_type)
				{
					 spectrum_editor->spectrum_number_of_bands_text->Enable(true);
					 spectrum_editor->spectrum_ratio_of_black_bands_text->Enable(true);
				}
				else
				{
					 spectrum_editor->spectrum_number_of_bands_text->Enable(false);
					 spectrum_editor->spectrum_ratio_of_black_bands_text->Enable(false);
				}
				if (spectrum_editor->spectrum_editor_step_value_text)
				{
					step_value = cmzn_spectrumcomponent_get_step_value(new_settings);
					sprintf(temp_string,"%10g",step_value);
					spectrum_editor->spectrum_editor_step_value_text->SetValue(wxString::FromAscii(temp_string));
				}
				if (CMZN_SPECTRUMCOMPONENT_COLOUR_MAPPING_TYPE_STEP == colour_mapping_type)
				{
					 spectrum_editor->spectrum_editor_step_value_text->Enable(true);
				}
				else
				{
					 spectrum_editor->spectrum_editor_step_value_text->Enable(false);
				}
				if (spectrum_editor->spectrum_range_min_text)
				{
					sprintf(temp_string,"%10g",
						cmzn_spectrumcomponent_get_range_minimum(new_settings));
					spectrum_editor->spectrum_range_min_text->SetValue(wxString::FromAscii(temp_string));
				}
				if (spectrum_editor->spectrum_range_max_text)
				{
					sprintf(temp_string,"%10g",
						cmzn_spectrumcomponent_get_range_maximum(new_settings));
					spectrum_editor->spectrum_range_max_text->SetValue(wxString::FromAscii(temp_string));
				}
				if (spectrum_editor->spectrum_extend_below_check)
				{
					 extend = cmzn_spectrumcomponent_is_extend_below(new_settings);
					 spectrum_editor->spectrum_extend_below_check->SetValue(extend);
				}
				if (spectrum_editor->spectrum_extend_above_check)
				{
					 extend = cmzn_spectrumcomponent_is_extend_above(new_settings);
					 spectrum_editor->spectrum_extend_above_check->SetValue(extend);
				}
				if (spectrum_editor->spectrum_fix_maximum_check)
				{
					 fix = cmzn_spectrumcomponent_is_fix_maximum(new_settings);
					 spectrum_editor->spectrum_fix_maximum_check->SetValue(fix);
					 spectrum_editor->spectrum_range_max_text->Enable(!fix);
				}
				if (spectrum_editor->spectrum_fix_minimum_check)
				{
					 fix = cmzn_spectrumcomponent_is_fix_minimum(new_settings);
					 spectrum_editor->spectrum_fix_minimum_check->SetValue(fix);
					 spectrum_editor->spectrum_range_min_text->Enable(!fix);
				}
				if (spectrum_editor->spectrum_normalised_colour_range_min_text)
				{
					sprintf(temp_string,"%10g", cmzn_spectrumcomponent_get_colour_minimum(new_settings));
					spectrum_editor->spectrum_normalised_colour_range_min_text->SetValue(wxString::FromAscii(temp_string));
					if (CMZN_SPECTRUMCOMPONENT_COLOUR_MAPPING_TYPE_BANDED == colour_mapping_type
						|| CMZN_SPECTRUMCOMPONENT_COLOUR_MAPPING_TYPE_STEP == colour_mapping_type)
					{
						spectrum_editor->spectrum_normalised_colour_range_min_text->Enable(false);
					}
					else
					{
						spectrum_editor->spectrum_normalised_colour_range_min_text->Enable(true);
					}
				}
				if (spectrum_editor->spectrum_normalised_colour_range_max_text)
				{
					sprintf(temp_string,"%10g", cmzn_spectrumcomponent_get_colour_maximum(new_settings));
					spectrum_editor->spectrum_normalised_colour_range_max_text->SetValue(wxString::FromAscii(temp_string));
					if (CMZN_SPECTRUMCOMPONENT_COLOUR_MAPPING_TYPE_BANDED == colour_mapping_type || CMZN_SPECTRUMCOMPONENT_COLOUR_MAPPING_TYPE_STEP == colour_mapping_type)
					{
						spectrum_editor->spectrum_normalised_colour_range_max_text->Enable(false);
					}
					else
					{
						spectrum_editor->spectrum_normalised_colour_range_max_text->Enable(true);
					}
				}
				spectrum_editor->spectrum_lower_panel->Layout();
				spectrum_editor->spectrum_higher_panel->Layout();
				return_code = 1;
			}
			else
			{
				 display_message(ERROR_MESSAGE,
						"spectrum_editor_wx_set_settings.  current settings not available");
				 return_code = 0;
			}
	 }
	 else
	 {
			display_message(ERROR_MESSAGE,
				 "spectrum_editor_wx_set_settings.  Invalid argument(s)");
			return_code = 0;
	 }
	 LEAVE;

	 return (return_code);
}

static int spectrum_editor_wx_select_settings_item(
	struct Spectrum_editor *spectrum_editor)
/*******************************************************************************
LAST MODIFIED : 30 August

DESCRIPTION :
Checks if current_settings is in the settings_rowcol; if not (or it was NULL)
the first item in this list becomes the current_settings, or NULL if empty.
The line/settings editors for the previous settings are then Unmanaged, and that
for the new settings Managed and filled with the new values.
If current_settings is NULL, no editing fields are displayed.
==============================================================================*/
{
	int return_code,i,num_children;
	struct cmzn_spectrumcomponent *temp_settings;
	ENTER(spectrum_editor_select_settings_item);
	/* check arguments */
	if (spectrum_editor)
	{
		 /* get list of settings items */
		 num_children = spectrum_editor->spectrum_settings_checklist->GetCount();
		 if (0<num_children)
		 {
				for (i=1;i<(num_children+1);i++)
				{
					 temp_settings = cmzn_spectrum_get_component_at_position(spectrum_editor->edit_spectrum,i);
					 if (!spectrum_editor->current_settings)
					 {
							spectrum_editor->current_settings=temp_settings;
					 }
					 if (temp_settings==spectrum_editor->current_settings)
					 {
							spectrum_editor->spectrum_settings_checklist->SetSelection(i-1, true);
					 }
				}
		 }
		 else
		 {
				spectrum_editor->current_settings=(struct cmzn_spectrumcomponent *)NULL;
		 }
		/* Grey Delete and Priority buttons if no current_settings */
		 spectrum_editor->spectrum_settings_del_button->Enable(
				(struct cmzn_spectrumcomponent *)NULL != spectrum_editor->current_settings);
		 spectrum_editor->spectrum_settings_up_button->Enable(
				(struct cmzn_spectrumcomponent *)NULL != spectrum_editor->current_settings);
		 spectrum_editor->spectrum_settings_dn_button->Enable(
				(struct cmzn_spectrumcomponent *)NULL != spectrum_editor->current_settings);
		 /* send selected object to settings editor */
		 return_code=spectrum_editor_wx_set_settings(
				spectrum_editor);
	}
	else
	{
		 display_message(ERROR_MESSAGE,
				"spectrum_editor_select_settings_item.  Invalid argument(s)");
		 return_code=0;
	}
	LEAVE;

	return (return_code);
} /* spectrum_editor_select_settings_item */

int spectrum_editor_settings_wx_key_presssed(struct Spectrum_editor *spectrum_editor, const char *flag)
/*******************************************************************************
LAST MODIFIED : 31 August 2007

DESCRIPTION :
Called when a modify button - add, delete, up, down - is activated.
==============================================================================*/
{
	 int list_changed = 0, position, return_code = 0;
	 struct cmzn_spectrumcomponent *settings;

	 ENTER(spectrum_editor_settings_wx_key_presssed);
	 if (spectrum_editor->edit_spectrum && flag)
	 {
			if (strcmp(flag, "Add") == 0)
			{
				 settings=CREATE(cmzn_spectrumcomponent)();
				 if (settings != 0)
				 {
						return_code=1;
						if (spectrum_editor->current_settings)
						{
							 /* copy current settings into new settings */
							 return_code=COPY(cmzn_spectrumcomponent)(settings,
									spectrum_editor->current_settings);
							 cmzn_spectrumcomponent_set_active(settings, true);
						}
						if (return_code&&Spectrum_add_component(
									 spectrum_editor->edit_spectrum,settings,0))
						{
							 list_changed=1;
							 spectrum_editor->current_settings=settings;
						}
						cmzn_spectrumcomponent_destroy(&settings);
				 }
			}
			else if (strcmp(flag, "Del") == 0)
			{
				 list_changed=cmzn_spectrum_remove_spectrumcomponent(
						spectrum_editor->edit_spectrum,spectrum_editor->current_settings);
				 spectrum_editor->current_settings=(struct cmzn_spectrumcomponent *)NULL;
			}
			else if (strcmp(flag, "Up") == 0)
			{
				 /* increase position of current_settings by 1 */
				 if (1<(position=cmzn_spectrum_get_component_position(
									 spectrum_editor->edit_spectrum,spectrum_editor->current_settings)))
				 {
						list_changed=1;
						ACCESS(cmzn_spectrumcomponent)(spectrum_editor->current_settings);
						cmzn_spectrum_remove_spectrumcomponent(spectrum_editor->edit_spectrum,
							 spectrum_editor->current_settings);
						Spectrum_add_component(spectrum_editor->edit_spectrum,
							 spectrum_editor->current_settings,position-1);
						DEACCESS(cmzn_spectrumcomponent)(&(spectrum_editor->current_settings));
				 }
			}
			else if (strcmp(flag, "Dn") == 0)
			{
				 /* decrease position of current_settings by 1 */
				 position=cmzn_spectrum_get_component_position(
								spectrum_editor->edit_spectrum,spectrum_editor->current_settings);
				 if (position > 0)
				 {
						list_changed=1;
						ACCESS(cmzn_spectrumcomponent)(spectrum_editor->current_settings);
						cmzn_spectrum_remove_spectrumcomponent(spectrum_editor->edit_spectrum,
							 spectrum_editor->current_settings);
						Spectrum_add_component(spectrum_editor->edit_spectrum,
							 spectrum_editor->current_settings,position+1);
						DEACCESS(cmzn_spectrumcomponent)(&(spectrum_editor->current_settings));
				 }
			}
			if (list_changed)
			{
				 spectrum_editor_wx_make_settings_list(spectrum_editor);
				 /* inform the client of the changes */
				 spectrum_editor_wx_select_settings_item(spectrum_editor);
				 make_colour_bar(spectrum_editor);
			}
	 }
	 else
	 {
			display_message(ERROR_MESSAGE,
				 "spectrum_editor_settings_wx_key_presssed.  Invalid argument(s)");
	 }
	 LEAVE;

	 return (return_code);
}

int spectrum_editor_wx_refresh(struct Spectrum_editor *spectrum_editor)
/*******************************************************************************
LAST MODIFIED : 12 August 2002

DESCRIPTION :
Clears all the settings_changed flags globally (later) and in the list of
settings.
==============================================================================*/
{
	int return_code;

	ENTER(spectrum_editor_wx_refresh);
	if (spectrum_editor)
	{
		if (spectrum_editor->edit_spectrum)
		{
			FOR_EACH_OBJECT_IN_LIST(cmzn_spectrumcomponent)(
				cmzn_spectrumcomponent_clear_changed,(void *)NULL,
				get_cmzn_spectrumcomponent_list( spectrum_editor->edit_spectrum ));
			spectrum_editor_wx_make_settings_list(spectrum_editor);
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"spectrum_editor_wx_refresh.  Missing edit_spectrum");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"spectrum_editor_wx_refresh.  Missing spectrum_editor");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* spectrum_editor_wx_refresh */

void spectrum_editor_wx_key_presssed(struct Spectrum_editor *spectrum_editor, const char *flag)
/*******************************************************************************
LAST MODIFIED : 31 August 2007

DESCRIPTION :
Called when a button - OK, Apply, Revert or Cancel - is activated.
==============================================================================*/
{
	 ENTER(spectrum_editor_wx_key_presssed);
	 if ((strcmp(flag, "Cancel") == 0) ||
			(strcmp(flag, "Revert") == 0))
	 {
			spectrum_editor_wx_set_spectrum(spectrum_editor,
				 spectrum_editor->current_spectrum);
	 }
	 if ((strcmp(flag, "Apply") == 0) ||
			(strcmp(flag, "OK") == 0))
	 {
			if (spectrum_editor->current_spectrum&&
				 spectrum_editor->edit_spectrum)
			{
				 if (MANAGER_MODIFY_NOT_IDENTIFIER(cmzn_spectrum,name)(
							spectrum_editor->current_spectrum, spectrum_editor->edit_spectrum,
								spectrum_editor->spectrum_manager))
				 {
						spectrum_editor_wx_refresh(spectrum_editor);
				 }
			}
	 }

	 if ((strcmp(flag, "OK") == 0) ||
			(strcmp(flag, "Cancel") == 0))
	 {
			DESTROY(Spectrum_editor_dialog)(spectrum_editor->spectrum_editor_dialog_address);
	 }
	 LEAVE;
}

void spectrum_editor_wx_update_settings(struct Spectrum_editor *spectrum_editor,
	 struct cmzn_spectrumcomponent *new_settings)
/*******************************************************************************
LAST MODIFIED : 31 August 2007

DESCRIPTION :
Callback for when changes made in the settings editor.
==============================================================================*/
{
	ENTER(spectrum_editor_wx_update_settings);
	if (spectrum_editor && new_settings)
	{
		/* keep visibility of current_settings */
		 bool active = cmzn_spectrumcomponent_is_active(spectrum_editor->current_settings);
		 cmzn_spectrumcomponent_modify(spectrum_editor->current_settings,new_settings,
				get_cmzn_spectrumcomponent_list( spectrum_editor->edit_spectrum ));
		 cmzn_spectrumcomponent_set_active(spectrum_editor->current_settings, active);
		 spectrum_editor_wx_make_settings_list(spectrum_editor);
		 Spectrum_calculate_range(spectrum_editor->edit_spectrum);
		 make_colour_bar(spectrum_editor);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"spectrum_editor_update_settings.  Invalid argument(s)");
	}
	LEAVE;
}

class wxSpectrumEditor : public wxFrame
{
	 Spectrum_editor *spectrum_editor;
	 wxPanel *spectrum_region_chooser_panel;
	 wxRegionChooser *spectrum_region_chooser;
	 DEFINE_MANAGER_CLASS(cmzn_spectrum);
	 Managed_object_listbox<cmzn_spectrum, MANAGER_CLASS(cmzn_spectrum)>
	 *spectrum_object_listbox;
	 DEFINE_MANAGER_CLASS(cmzn_scenefilter);
	 Managed_object_chooser<cmzn_scenefilter,MANAGER_CLASS(cmzn_scenefilter)>
	 *filter_chooser;
public:

	 wxSpectrumEditor(Spectrum_editor *spectrum_editor):
			spectrum_editor(spectrum_editor)
	 {
			wxXmlInit_spectrum_editor_wx();
			wxXmlResource::Get()->LoadFrame(this,
				 (wxWindow *)NULL, _T("CmguiSpectrumEditor"));
			this->SetIcon(cmiss_icon_xpm);
			spectrum_editor->spectrum_listbox_panel =
				 XRCCTRL(*this, "SpectrumListPanel", wxPanel);
			spectrum_editor->spectrum_listbox_panel->SetSize(wxDefaultCoord,wxDefaultCoord,
				400, 100);
			spectrum_editor->spectrum_listbox_panel->SetMinSize(wxSize(-1,100));
			spectrum_object_listbox =
				 new Managed_object_listbox<cmzn_spectrum, MANAGER_CLASS(cmzn_spectrum)>
				 (spectrum_editor->spectrum_listbox_panel, (struct cmzn_spectrum*)NULL, spectrum_editor->spectrum_manager,
						(MANAGER_CONDITIONAL_FUNCTION(cmzn_spectrum) *)NULL, (void *)NULL, spectrum_editor->user_interface);
			Callback_base<cmzn_spectrum* > *spectrum_editor_spectrum_list_callback =
				 new Callback_member_callback< cmzn_spectrum*,
				 wxSpectrumEditor, int (wxSpectrumEditor::*)(cmzn_spectrum *) >
				 (this, &wxSpectrumEditor::spectrum_editor_spectrum_list_callback);
			spectrum_object_listbox->set_callback(spectrum_editor_spectrum_list_callback);
			wxPanel *spectrum_region_chooser_panel =
				 XRCCTRL(*this, "wxSpectrumRegionChooserPanel", wxPanel);
			char *initial_path;
		   initial_path = cmzn_region_get_root_region_path();
		   spectrum_region_chooser = new wxRegionChooser(spectrum_region_chooser_panel,
				spectrum_editor->root_region, initial_path);
			DEALLOCATE(initial_path);
			Callback_base< cmzn_region* > *spectrum_region_chooser_callback =
				new Callback_member_callback< cmzn_region*,
				wxSpectrumEditor, int (wxSpectrumEditor::*)(cmzn_region *) >
				(this, &wxSpectrumEditor::spectrum_region_chooser_callback);
			spectrum_region_chooser->set_callback(spectrum_region_chooser_callback);
			wxPanel *spectrum_filter_chooser_panel =
				 XRCCTRL(*this, "wxSpectrumFilterChooserPanel", wxPanel);
			cmzn_scenefilter_id filter =
				cmzn_scenefiltermodule_get_default_scenefilter(spectrum_editor->filter_module);
			filter_chooser =
				 new Managed_object_chooser<cmzn_scenefilter, MANAGER_CLASS(cmzn_scenefilter)>
				 (spectrum_filter_chooser_panel, filter,
					 cmzn_scenefiltermodule_get_manager(spectrum_editor->filter_module),
						(MANAGER_CONDITIONAL_FUNCTION(cmzn_scenefilter) *)NULL, (void *)NULL,
						spectrum_editor->user_interface);
			cmzn_scenefilter_destroy(&filter);

			XRCCTRL(*this, "wxSpectrumRangeMinText", wxTextCtrl)->Connect(wxEVT_KILL_FOCUS,
				 wxCommandEventHandler(wxSpectrumEditor::OnSpectrumSettingRangeValueEntered),
				 NULL, this);
			XRCCTRL(*this,"wxSpectrumRangeMaxText", wxTextCtrl)->Connect(wxEVT_KILL_FOCUS,
				 wxCommandEventHandler(wxSpectrumEditor::OnSpectrumSettingRangeValueEntered),
				 NULL, this);
			XRCCTRL(*this,"wxSpectrumExaggerationTextCtrl", wxTextCtrl)->Connect(wxEVT_KILL_FOCUS,
				 wxCommandEventHandler(wxSpectrumEditor::OnSpectrumExaggerationSettingsChanged),
				 NULL, this);
			XRCCTRL(*this,"wxSpectrumDataComponentText", wxTextCtrl)->Connect(wxEVT_KILL_FOCUS,
				 wxCommandEventHandler(wxSpectrumEditor::OnSpectrumDataValueEntered),
				 NULL, this);
			XRCCTRL(*this,"wxSpectrumNumberOfBandsTextCtrl", wxTextCtrl)->Connect(wxEVT_KILL_FOCUS,
				 wxCommandEventHandler(wxSpectrumEditor::OnSpectrumBandsValuesEntered),
				 NULL, this);
			XRCCTRL(*this,"wxSpectrumRatioOfBlackBandsTextCtrl", wxTextCtrl)->Connect(wxEVT_KILL_FOCUS,
				 wxCommandEventHandler(wxSpectrumEditor::OnSpectrumBandsValuesEntered),
				 NULL, this);
			XRCCTRL(*this,"wxSpectrumStepValueTextCtrl", wxTextCtrl)->Connect(wxEVT_KILL_FOCUS,
				 wxCommandEventHandler(wxSpectrumEditor::OnSpectrumStepValueEntered),
				 NULL, this);
			XRCCTRL(*this,"wxSpectrumNormalisedColourRangeMin", wxTextCtrl)->Connect(wxEVT_KILL_FOCUS,
				 wxCommandEventHandler(wxSpectrumEditor::OnSpectrumNormalisedColourRangeValueEntered),
				 NULL, this);
			XRCCTRL(*this,"wxSpectrumNormalisedColourRangeMax", wxTextCtrl)->Connect(wxEVT_KILL_FOCUS,
				 wxCommandEventHandler(wxSpectrumEditor::OnSpectrumNormalisedColourRangeValueEntered),
				 NULL, this);
	 };

	 wxSpectrumEditor()
	 {
	 };

	 ~wxSpectrumEditor()
	 {
			delete filter_chooser;
			delete spectrum_object_listbox;
			delete spectrum_region_chooser;
	 };

 int spectrum_editor_spectrum_list_callback(cmzn_spectrum *spectrum)
/*******************************************************************************
LAST MODIFIED : 7 January 2008

DESCRIPTION :
Callback from wxObjectListBox<Spectrum> when choice is made.
==============================================================================*/
{
	 ENTER(material_editor_graphical_material_callback);
	 if (spectrum != NULL)
	 {
		 make_current_spectrum(spectrum_editor, spectrum);
	 }
	 return 1;
}

void spectrum_editor_spectrum_list_set_selected(cmzn_spectrum *spectrum)
{
	 ENTER(material_editor_graphical_material_list_set_selected);
	 if (spectrum_object_listbox)
	 {
			spectrum_object_listbox->set_object(spectrum);
	 }
	 LEAVE;
}

struct cmzn_spectrum *spectrum_editor_spectrum_list_get_selected()
{
	 if (spectrum_object_listbox)
	 {
			return (spectrum_object_listbox->get_object());
	 }
	 else
	 {
			return ((cmzn_spectrum *)NULL);
	 }
}

int set_autorange_scene()
{
	cmzn_region_id current_region = spectrum_region_chooser->get_region();
	if (spectrum_editor->autorange_scene)
		cmzn_scene_destroy(&spectrum_editor->autorange_scene);
	spectrum_editor->autorange_scene = cmzn_region_get_scene(current_region);
	return 1;
}

int spectrum_region_chooser_callback(cmzn_region *region)
{
	set_autorange_scene();
	return 1;
}

void OnSpectrumOverwriteColourChecked(wxCommandEvent &event)
/*******************************************************************************
LAST MODIFIED : 10 Jan 2008

DESCRIPTION :
Callback for Spectrum clear colour checkbox changed.
==============================================================================*/
{
	USE_PARAMETER(event);
	bool isChecked = (0 != spectrum_editor->spectrum_overwrite_colour_radiobox->GetSelection());
	if (cmzn_spectrum_is_material_overwrite(spectrum_editor->edit_spectrum) != isChecked)
	{
		cmzn_spectrum_set_material_overwrite(spectrum_editor->edit_spectrum, isChecked);
	}
}

void OnSpectrumSettingsSelected(wxCommandEvent &event)
/*******************************************************************************
LAST MODIFIED : 31 August 2007

DESCRIPTION :
Callback for Spectrum settings.
==============================================================================*/
{
	 struct cmzn_spectrumcomponent *temp_settings;
	 int selection;
	USE_PARAMETER(event);
	 selection = spectrum_editor->spectrum_settings_checklist->GetSelection();
	 temp_settings = cmzn_spectrum_get_component_at_position(spectrum_editor->edit_spectrum,selection+1);
	 if (temp_settings)
	 {
			if (temp_settings != spectrum_editor->current_settings)
			{
				 spectrum_editor->current_settings = temp_settings;
			}
			if (spectrum_editor->spectrum_settings_checklist->IsChecked(selection) !=
				 static_cast<bool>(cmzn_spectrumcomponent_is_active(temp_settings)))
			{
				cmzn_spectrumcomponent_set_active(temp_settings,
					  spectrum_editor->spectrum_settings_checklist->IsChecked(selection));
				 spectrum_editor_wx_update_settings(spectrum_editor, temp_settings);
			}
			spectrum_editor_wx_select_settings_item(spectrum_editor);
	 }
}

void OnSpectrumSettingListAddPressed(wxCommandEvent &event)
/*******************************************************************************
LAST MODIFIED : 31 August 2007

DESCRIPTION :
Callback for Spectrum settings.
==============================================================================*/
{
	USE_PARAMETER(event);
	 spectrum_editor_settings_wx_key_presssed(spectrum_editor, "Add");
}

void OnSpectrumSettingListDelPressed(wxCommandEvent &event)
/*******************************************************************************
LAST MODIFIED : 31 August 2007

DESCRIPTION :
Callback for Spectrum settings.
==============================================================================*/
{
	USE_PARAMETER(event);
	 spectrum_editor_settings_wx_key_presssed(spectrum_editor, "Del");
}

void OnSpectrumSettingListUpPressed(wxCommandEvent &event)
/*******************************************************************************
LAST MODIFIED : 31 August 2007

DESCRIPTION :
Callback for Spectrum settings.
==============================================================================*/
{
	USE_PARAMETER(event);
	 spectrum_editor_settings_wx_key_presssed(spectrum_editor, "Up");
}

void OnSpectrumSettingListDnPressed(wxCommandEvent &event)
/*******************************************************************************
LAST MODIFIED : 31 August 2007

DESCRIPTION :
Callback for Spectrum settings.
==============================================================================*/
{
	USE_PARAMETER(event);
	 spectrum_editor_settings_wx_key_presssed(spectrum_editor, "Dn");
}

void OnSpectrumSettingRangeValueEntered(wxCommandEvent &event)
/*******************************************************************************
LAST MODIFIED : 31 August 2007

DESCRIPTION :
Callback for the range text widgets.
==============================================================================*/
{
	const char *text;
	float new_parameter;
	int selection;
	struct cmzn_spectrumcomponent *settings;

	ENTER(OnSpectrumSettingRangeValueEntered);
	USE_PARAMETER(event);
	/* check arguments */
	selection = spectrum_editor->spectrum_settings_checklist->GetSelection();
	settings = cmzn_spectrum_get_component_at_position(spectrum_editor->edit_spectrum,selection+1);
	if (settings != 0)
	{
		if (spectrum_editor->spectrum_range_min_text)
		{
			wxString min_string = spectrum_editor->spectrum_range_min_text->GetValue();
			text = min_string.mb_str(wxConvUTF8);
			if (text)
			{
				sscanf(text,"%f",&new_parameter);
				cmzn_spectrumcomponent_set_range_minimum(settings, new_parameter);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"OnSpectrumSettingRangeValueEntered.  Missing range minimum text");
			}
		}
		if (spectrum_editor->spectrum_range_max_text)
		{
			wxString max_string = spectrum_editor->spectrum_range_max_text->GetValue();
			text = max_string.mb_str(wxConvUTF8);
			if (text)
			{
				sscanf(text,"%f",&new_parameter);
				cmzn_spectrumcomponent_set_range_maximum(settings, new_parameter);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"OnSpectrumSettingRangeValueEntered.  Missing range maximum text");
			}
		}
		spectrum_editor_wx_update_settings(spectrum_editor, settings);
		spectrum_editor_wx_set_settings(spectrum_editor);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			 "OnSpectrumSettingRangeValueEntered.  Invalid argument(s)");
	}
	LEAVE;
} /* OnSpectrumSettingRangeEntered */

void OnSpectrumSettingRangeChecked(wxCommandEvent &event)
/*******************************************************************************
LAST MODIFIED : 31 August 2007

DESCRIPTION :
Callback for the range text widgets.
==============================================================================*/
{
	 int selection;
	 struct cmzn_spectrumcomponent *settings;

	 ENTER(OnSpectrumSettingRangeChecked);
	USE_PARAMETER(event);
	 selection = spectrum_editor->spectrum_settings_checklist->GetSelection();
	 if (spectrum_editor &&
			(settings = cmzn_spectrum_get_component_at_position(spectrum_editor->edit_spectrum,selection+1)))
	 {
			if (spectrum_editor->spectrum_extend_above_check)
			{

				cmzn_spectrumcomponent_set_extend_above(settings,
					 spectrum_editor->spectrum_extend_above_check->IsChecked());
			}
			if (spectrum_editor->spectrum_extend_below_check)
			{
				cmzn_spectrumcomponent_set_extend_below(settings,
					 spectrum_editor->spectrum_extend_below_check->IsChecked());
			}
			if (spectrum_editor->spectrum_fix_maximum_check)
			{
				 cmzn_spectrumcomponent_set_fix_maximum(settings,
						spectrum_editor->spectrum_fix_maximum_check->IsChecked());
			}
			if (spectrum_editor->spectrum_fix_minimum_check)
			{
				 cmzn_spectrumcomponent_set_fix_minimum(settings,
						spectrum_editor->spectrum_fix_minimum_check->IsChecked());
			}
			spectrum_editor_wx_update_settings(spectrum_editor, settings);
			spectrum_editor_wx_set_settings(spectrum_editor);
	 }
	 else
	 {
			display_message(ERROR_MESSAGE,
				 "OnSpectrumSettingRangeValueChecked.  Invalid argument(s)");
	 }
	 LEAVE;
}

void OnSpectrumSettingListOKPressed(wxCommandEvent &event)
/*******************************************************************************
LAST MODIFIED : 31 August 2007

DESCRIPTION :
Callback for Spectrum settings.
==============================================================================*/
{
	USE_PARAMETER(event);
	 spectrum_editor_wx_key_presssed(spectrum_editor, "OK");
}

void OnSpectrumSettingListApplyPressed(wxCommandEvent &event)
/*******************************************************************************
LAST MODIFIED : 31 August 2007

DESCRIPTION :
Callback for Spectrum settings.
==============================================================================*/
{
	USE_PARAMETER(event);
	 spectrum_editor_wx_key_presssed(spectrum_editor, "Apply");
}

void OnSpectrumSettingListRevertPressed(wxCommandEvent &event)
/*******************************************************************************
LAST MODIFIED : 31 August 2007

DESCRIPTION :
Callback for Spectrum settings.
==============================================================================*/
{
	USE_PARAMETER(event);
	 spectrum_editor_wx_key_presssed(spectrum_editor, "Revert");
}

void OnSpectrumSettingListCancelPressed(wxCommandEvent &event)
/*******************************************************************************
LAST MODIFIED : 31 August 2007

DESCRIPTION :
Callback for Spectrum settings.
==============================================================================*/
{
	USE_PARAMETER(event);
	 spectrum_editor_wx_key_presssed(spectrum_editor, "Cancel");
}

void OnSpectrumColourMappingTypeChoice(wxCommandEvent &event)
/*******************************************************************************
LAST MODIFIED : 4 September 2007

DESCRIPTION :
Callback for colour settings
==============================================================================*/
{
	enum cmzn_spectrumcomponent_colour_mapping_type new_colour_mapping_type = CMZN_SPECTRUMCOMPONENT_COLOUR_MAPPING_TYPE_ALPHA;
	struct cmzn_spectrumcomponent *settings;
	const char *string_selection;
	int selection;

	ENTER(OnSpectrumColourMappingTypeChoice);

	USE_PARAMETER(event);
	selection = spectrum_editor->spectrum_settings_checklist->GetSelection();
	if (spectrum_editor &&
		 (settings = cmzn_spectrum_get_component_at_position(spectrum_editor->edit_spectrum,selection+1)))
	{
		/* get the widget from the call data */
		wxString tmpstr = spectrum_editor->spectrum_colour_mapping_type_choice->GetStringSelection();
		string_selection = tmpstr.mb_str(wxConvUTF8);
		if (string_selection != 0)
		{
			if (strcmp(string_selection, "Alpha") == 0)
			{
				new_colour_mapping_type = CMZN_SPECTRUMCOMPONENT_COLOUR_MAPPING_TYPE_ALPHA;
			}
			else if (strcmp(string_selection, "Blue") == 0)
			{
				new_colour_mapping_type = CMZN_SPECTRUMCOMPONENT_COLOUR_MAPPING_TYPE_BLUE;
			}
			else if (strcmp(string_selection, "Green") == 0)
			{
				new_colour_mapping_type = CMZN_SPECTRUMCOMPONENT_COLOUR_MAPPING_TYPE_GREEN;
			}
			else if (strcmp(string_selection, "Monochrome") == 0)
			{
				new_colour_mapping_type = CMZN_SPECTRUMCOMPONENT_COLOUR_MAPPING_TYPE_MONOCHROME;
			}
			else if (strcmp(string_selection, "Rainbow") == 0)
			{
				new_colour_mapping_type = CMZN_SPECTRUMCOMPONENT_COLOUR_MAPPING_TYPE_RAINBOW;
			}
			else if (strcmp(string_selection, "Red") == 0)
			{
				new_colour_mapping_type = CMZN_SPECTRUMCOMPONENT_COLOUR_MAPPING_TYPE_RED;
			}
			else if (strcmp(string_selection, "White to blue") == 0)
			{
				new_colour_mapping_type = CMZN_SPECTRUMCOMPONENT_COLOUR_MAPPING_TYPE_WHITE_TO_BLUE;
			}
			else if (strcmp(string_selection, "White to green") == 0)
			{
				new_colour_mapping_type = CMZN_SPECTRUMCOMPONENT_COLOUR_MAPPING_TYPE_WHITE_TO_GREEN;
			}
			else if (strcmp(string_selection, "Step") == 0)
			{
				new_colour_mapping_type = CMZN_SPECTRUMCOMPONENT_COLOUR_MAPPING_TYPE_STEP;
			}
			else if (strcmp(string_selection, "Contour Bands") == 0)
			{
				new_colour_mapping_type = CMZN_SPECTRUMCOMPONENT_COLOUR_MAPPING_TYPE_BANDED;
			}
			else if (strcmp(string_selection, "White to red") == 0)
			{
				new_colour_mapping_type = CMZN_SPECTRUMCOMPONENT_COLOUR_MAPPING_TYPE_WHITE_TO_RED;
			}
			if (cmzn_spectrumcomponent_get_colour_mapping_type(settings) != new_colour_mapping_type)
			{
				if (cmzn_spectrumcomponent_set_colour_mapping_type(settings, new_colour_mapping_type))
				{
					spectrum_editor_wx_update_settings(spectrum_editor, settings);
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"spectrum_editor_settings_colour_menu_CB.  "
				"Could not find the activated menu item");
		}
		/* make sure the correct render render is shown in case of error */
		 spectrum_editor_wx_set_settings(spectrum_editor);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"OnSpectrumColourMappingTypeChoice.  Invalid argument(s)");
	}
	LEAVE;
}

void OnSpectrumNormalisedColourRangeValueEntered(wxCommandEvent &event)
/*******************************************************************************
LAST MODIFIED : 4 September

DESCRIPTION :
Callback for the colour_value text widgets.
==============================================================================*/
{
	const char *text;
	float new_parameter;
	struct cmzn_spectrumcomponent *settings;
	int selection;

	ENTER(OnSpectrumColourMappingTypeChoice);
	USE_PARAMETER(event);
	selection = spectrum_editor->spectrum_settings_checklist->GetSelection();
	if (spectrum_editor &&
		(settings = cmzn_spectrum_get_component_at_position(spectrum_editor->edit_spectrum,selection+1)))
	{
		wxString min_string = spectrum_editor->spectrum_normalised_colour_range_min_text->GetValue();
		text = min_string.mb_str(wxConvUTF8);
		if (text)
		{
			sscanf(text,"%f",&new_parameter);
			cmzn_spectrumcomponent_set_colour_minimum(settings, new_parameter);
			text = NULL;
		}
		wxString max_string = spectrum_editor->spectrum_normalised_colour_range_max_text->GetValue();
		text = max_string.mb_str(wxConvUTF8);
		if (text)
		{
			sscanf(text,"%f",&new_parameter);
			cmzn_spectrumcomponent_set_colour_maximum(settings, new_parameter);
		}
		spectrum_editor_wx_update_settings(spectrum_editor, settings);
		spectrum_editor_wx_set_settings(spectrum_editor);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"OnSpectrumNormalisedColourRangeValueEntered.  Invalid argument(s)");
	}
	LEAVE;
}

void OnSpectrumSettingsReverseChecked(wxCommandEvent &event)
/*******************************************************************************
LAST MODIFIED : 4 Sepetember 2007

DESCRIPTION :
Callback for the settings reverse toggle button.
==============================================================================*/
{
	 int selection;
	 struct cmzn_spectrumcomponent *settings;

	 ENTER(spectrum_editor_settings_reverse_toggle_CB);
	USE_PARAMETER(event);
	 selection = spectrum_editor->spectrum_settings_checklist->GetSelection();
	 if (spectrum_editor &&
			(settings = cmzn_spectrum_get_component_at_position(spectrum_editor->edit_spectrum,selection+1)))
	 {
			 cmzn_spectrumcomponent_set_colour_reverse(settings,
				 spectrum_editor->spectrum_reverse_checkbox->GetValue());
			spectrum_editor_wx_update_settings(spectrum_editor, settings);
			/* make sure the correct reverse flag is shown in case of error */
			spectrum_editor_wx_set_settings(spectrum_editor);
	 }
	 else
	 {
			display_message(ERROR_MESSAGE,
				 "OnSpectrumSettingsReverseChecked.  Invalid argument(s)");
	 }
	 LEAVE;
}

void OnSpectrumTypeChoice(wxCommandEvent &event)
/*******************************************************************************
LAST MODIFIED : 4 Sepetember 2007

DESCRIPTION :
Callback for the settings type.
==============================================================================*/
{
	 enum cmzn_spectrumcomponent_scale_type new_spectrum_type = CMZN_SPECTRUMCOMPONENT_SCALE_TYPE_INVALID;
	 struct cmzn_spectrumcomponent *settings;
	 int selection;
	 const char *string_selection;

	 ENTER(OnSpectrumTypeChoice);
	USE_PARAMETER(event);
	 selection = spectrum_editor->spectrum_settings_checklist->GetSelection();
	 if (spectrum_editor &&
			(settings = cmzn_spectrum_get_component_at_position(spectrum_editor->edit_spectrum,selection+1)))
	 {
			/* get the widget from the call data */
			wxString tmpstr = spectrum_editor->spectrum_type_choice->GetStringSelection();
			string_selection = tmpstr.mb_str(wxConvUTF8);
			if (string_selection != 0)
			{
				 if (strcmp(string_selection, "Linear") == 0)
				 {
						new_spectrum_type = CMZN_SPECTRUMCOMPONENT_SCALE_TYPE_LINEAR;
				 }
				 else if (strcmp(string_selection, "Log") == 0)
				 {
						new_spectrum_type = CMZN_SPECTRUMCOMPONENT_SCALE_TYPE_LOG;
				 }
				 if (cmzn_spectrumcomponent_get_scale_type(settings) != new_spectrum_type)
				 {
						if (cmzn_spectrumcomponent_set_scale_type(settings, new_spectrum_type))
						{
							 spectrum_editor_wx_update_settings(spectrum_editor, settings);
						}
				 }
			}
			else
			{
				 display_message(ERROR_MESSAGE,"OnSpectrumTypeChoice.  "
						"Could not find the activated menu item");
			}
			spectrum_editor_wx_set_settings(spectrum_editor);
	 }
	 else
	 {
			display_message(ERROR_MESSAGE,
				 "OnSpectrumTypeChoice.  Invalid argument(s)");
	 }
	 LEAVE;
}

void OnSpectrumExaggerationSettingsChanged(wxCommandEvent &event)
/*******************************************************************************
LAST MODIFIED : 30 July 1998

DESCRIPTION :
Callback for the exaggeration widgets.
==============================================================================*/
{
	const char *text;
	float new_parameter;
	struct cmzn_spectrumcomponent *settings;
	int selection;

	ENTER(OnSpectrumExaggerationSettingsChanged);
	USE_PARAMETER(event);
	selection = spectrum_editor->spectrum_settings_checklist->GetSelection();
	if (spectrum_editor &&
		(settings = cmzn_spectrum_get_component_at_position(spectrum_editor->edit_spectrum,selection+1)))
	{
		wxString exaggeration_string = spectrum_editor->spectrum_exaggeration_text->GetValue();
		text = exaggeration_string.mb_str(wxConvUTF8);
		if (text)
		{
			sscanf(text,"%f",&new_parameter);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"OnSpectrumExaggerationSettingsChanged.  Missing widget text");
		}
		if (spectrum_editor->spectrum_left_right_radio_box->GetSelection() == 1)
		{
			new_parameter *= -1.0;
		}
		if(new_parameter !=
			cmzn_spectrumcomponent_get_exaggeration(settings))
		{
			cmzn_spectrumcomponent_set_exaggeration(settings, new_parameter);
			spectrum_editor_wx_update_settings(spectrum_editor, settings);
		}
		spectrum_editor_wx_set_settings(spectrum_editor);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"OnSpectrumExaggerationSettingsChanged.  Invalid argument(s)");
	}
	LEAVE;
}

void OnSpectrumDataValueEntered(wxCommandEvent &event)
/*******************************************************************************
LAST MODIFIED : 4 September 2007

DESCRIPTION :
Callback for the component widgets.
==============================================================================*/
{
	const char *text;
	int new_component, selection;
	struct cmzn_spectrumcomponent *settings;

	ENTER(OnSpectrumDataValueEntered);
	USE_PARAMETER(event);
	selection = spectrum_editor->spectrum_settings_checklist->GetSelection();
	if (spectrum_editor &&
		(settings = cmzn_spectrum_get_component_at_position(spectrum_editor->edit_spectrum,selection+1)))
	{
		new_component = 1;
		wxString component_string = spectrum_editor->spectrum_data_component_text->GetValue();
		text = component_string.mb_str(wxConvUTF8);
		if (text)
		{
			sscanf(text,"%d",&new_component);
		}
		else
		{
			display_message(ERROR_MESSAGE,
			"OnSpectrumDataValueEntered.  Missing widget text");
		}
		if(new_component !=
			cmzn_spectrumcomponent_get_field_component(settings))
		{
			cmzn_spectrumcomponent_set_field_component(settings,new_component);
			spectrum_editor_wx_update_settings(spectrum_editor, settings);
		}
		spectrum_editor_wx_set_settings(spectrum_editor);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"spectrum_editor_settings_component_CB.  Invalid argument(s)");
	}
	LEAVE;
}

void OnSpectrumBandsValuesEntered(wxCommandEvent &event)
/*******************************************************************************
LAST MODIFIED : 4 September 2007

DESCRIPTION :
Callback for the band  widgets.
==============================================================================*/
{
	const char *text;
	int new_parameter, selection;
	float new_ratio;
	struct cmzn_spectrumcomponent *settings;

	ENTER(OnSpectrumBandsValuesEntered);
	USE_PARAMETER(event);
	selection = spectrum_editor->spectrum_settings_checklist->GetSelection();
	if (spectrum_editor &&
		(settings = cmzn_spectrum_get_component_at_position(spectrum_editor->edit_spectrum,selection+1)))
	{
		wxString num_bands_string = spectrum_editor->spectrum_number_of_bands_text->GetValue();
		text = num_bands_string.mb_str(wxConvUTF8);
		if (text)
		{
			sscanf(text,"%d",&new_parameter);
			text = NULL;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"OnSpectrumBandsValuesEntered.  Missing widget text");
		}
		if(new_parameter !=
			cmzn_spectrumcomponent_get_number_of_bands(settings))
		{
			cmzn_spectrumcomponent_set_number_of_bands(settings,new_parameter);
			spectrum_editor_wx_update_settings(spectrum_editor, settings);
		}
		wxString sprectrum_ratio_string = spectrum_editor->spectrum_ratio_of_black_bands_text->GetValue();
		text = sprectrum_ratio_string.mb_str(wxConvUTF8);
		if (text)
		{
			sscanf(text,"%f",&new_ratio);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"OnSpectrumBandsValuesEntered.  Missing widget text");
		}
		new_parameter = (int)(new_ratio * 1000.0 + 0.5);
		if(new_parameter !=
			cmzn_spectrumcomponent_get_black_band_proportion(settings))
		{
			cmzn_spectrumcomponent_set_black_band_proportion(settings,new_parameter);
			spectrum_editor_wx_update_settings(spectrum_editor, settings);
		}
		spectrum_editor_wx_set_settings(spectrum_editor);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"spectrum_editor_settings_bands_CB.  Invalid argument(s)");
	}
	LEAVE;
}

void OnSpectrumStepValueEntered(wxCommandEvent &event)
{
	const char *text;
	float new_parameter;
	int selection;
	struct cmzn_spectrumcomponent *settings;

	ENTER(OnSpectrumStepValueEntered);
USE_PARAMETER(event);
	selection = spectrum_editor->spectrum_settings_checklist->GetSelection();
	if (spectrum_editor &&
		(settings = cmzn_spectrum_get_component_at_position(spectrum_editor->edit_spectrum,selection+1)))
	{
		wxString step_value = spectrum_editor->spectrum_editor_step_value_text->GetValue();
		text = step_value.mb_str(wxConvUTF8);
		if (text)
		{
			sscanf(text,"%f",&new_parameter);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"OnSpectrumStepValueEntered.  Missing widget text");
		}
		if(new_parameter !=
			cmzn_spectrumcomponent_get_step_value(settings))
		{
			cmzn_spectrumcomponent_set_step_value(settings, new_parameter);
			spectrum_editor_wx_update_settings(spectrum_editor, settings);
		}
		spectrum_editor_wx_set_settings(spectrum_editor);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"OnSpectrumStepValueEntered.  Invalid argument(s)");
	}
	LEAVE;
}

void OnSpectrumAutorangePressed(wxCommandEvent &event)
{
	USE_PARAMETER(event);
	if (spectrum_editor)
	{
		double maximum, minimum;
		int maxRanges = cmzn_scene_get_spectrum_data_range(spectrum_editor->autorange_scene,
			filter_chooser->get_object(), spectrum_editor->current_spectrum
			/* Not spectrum_to_be_modified_copy as this ptr
				identifies the valid graphics objects */,
			/*valuesCount*/1, &minimum, &maximum);
		if ( maxRanges >= 1 )
		{
			Spectrum_set_minimum_and_maximum(
				spectrum_editor->edit_spectrum,
				minimum, maximum );
			spectrum_editor_wx_set_settings(spectrum_editor);
			spectrum_editor_wx_make_settings_list(spectrum_editor);
			make_colour_bar(spectrum_editor);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"OnSpectrumAutorangePressed.  Invalid argument(s)");
	}
	LEAVE;
}

void OnSpectrumEditorCreateNewSpectrum(wxCommandEvent& event)
{
	ENTER(OnMaterialEditorCreateNewMaterial);
	cmzn_spectrum *spectrum;
	USE_PARAMETER(event);
	wxTextEntryDialog *NewSpectrumDialog = new wxTextEntryDialog(this, wxT("Enter name"),
		wxT("Please Enter Name"), wxT("TEMP"), wxOK|wxCANCEL|wxCENTRE, wxDefaultPosition);
	if (NewSpectrumDialog->ShowModal() == wxID_OK)
	{
		wxString text = NewSpectrumDialog->GetValue();
		spectrum = cmzn_spectrum_create_private();
		cmzn_spectrum_set_name(spectrum, text.mb_str(wxConvUTF8));
		if (spectrum)
		{
			if(MANAGER_COPY_WITHOUT_IDENTIFIER(cmzn_spectrum,name)
				(spectrum,spectrum_editor->edit_spectrum))
			{
				ADD_OBJECT_TO_MANAGER(cmzn_spectrum)(
					spectrum, spectrum_editor->spectrum_manager);
				make_current_spectrum(spectrum_editor, spectrum);
			}
		}
	}
	delete NewSpectrumDialog;
	LEAVE;
}

void OnSpectrumEditorDeleteSpectrum(wxCommandEvent& event)
{
	ENTER(OnSpectrumEditorDeleteSpectrum);

	USE_PARAMETER(event);
	REMOVE_OBJECT_FROM_MANAGER(cmzn_spectrum)(
		spectrum_editor->current_spectrum,spectrum_editor->spectrum_manager);
	make_current_spectrum(spectrum_editor, NULL);

	LEAVE;
}

void OnSpectrumEditorRenameSpectrum(wxCommandEvent& event)
{
	ENTER(OnSpectrumEditorRenameSpectrum);
	USE_PARAMETER(event);
	wxTextEntryDialog *NewSpectrumDialog = new wxTextEntryDialog(this, wxT("Enter name"),
		wxT("Please Enter Name"), spectrum_object_listbox->get_string_selection(),
		wxOK|wxCANCEL|wxCENTRE, wxDefaultPosition);
	if (NewSpectrumDialog->ShowModal() == wxID_OK)
	{
		wxString text = NewSpectrumDialog->GetValue();
		cmzn_spectrum_set_name(spectrum_editor->current_spectrum, text.mb_str(wxConvUTF8));
	}
	delete NewSpectrumDialog;

	LEAVE;
}

void CloseSpectrumEditor(wxCloseEvent &event)
{
	 ENTER(CloseSpectrumEditor);
	USE_PARAMETER(event);
	 spectrum_editor_wx_set_spectrum(spectrum_editor,
			spectrum_editor->current_spectrum);
	 DESTROY(Spectrum_editor_dialog)(spectrum_editor->spectrum_editor_dialog_address);
	 LEAVE;
}

DECLARE_EVENT_TABLE();
};

BEGIN_EVENT_TABLE(wxSpectrumEditor, wxFrame)
	 EVT_BUTTON(XRCID("wxSpectrumCreateButton"),wxSpectrumEditor::OnSpectrumEditorCreateNewSpectrum)
	 EVT_BUTTON(XRCID("wxSpectrumDeleteButton"),wxSpectrumEditor::OnSpectrumEditorDeleteSpectrum)
	 EVT_BUTTON(XRCID("wxSpectrumRenameButton"),wxSpectrumEditor::OnSpectrumEditorRenameSpectrum)
	 EVT_RADIOBOX(XRCID("wxSpectrumOverwriteColourRadioBox"),wxSpectrumEditor::OnSpectrumOverwriteColourChecked)
	 EVT_CHECKLISTBOX(XRCID("wxSpectrumSettingsCheckList"), wxSpectrumEditor::OnSpectrumSettingsSelected)
	 EVT_LISTBOX(XRCID("wxSpectrumSettingsCheckList"), wxSpectrumEditor::OnSpectrumSettingsSelected)
	 EVT_BUTTON(XRCID("wxSpectrumSettingsAdd"),wxSpectrumEditor::OnSpectrumSettingListAddPressed)
	 EVT_BUTTON(XRCID("wxSpectrumSettingsDel"),wxSpectrumEditor::OnSpectrumSettingListDelPressed)
	 EVT_BUTTON(XRCID("wxSpectrumSettingsUp"),wxSpectrumEditor::OnSpectrumSettingListUpPressed)
	 EVT_BUTTON(XRCID("wxSpectrumSettingsDn"),wxSpectrumEditor::OnSpectrumSettingListDnPressed)
	 EVT_TEXT_ENTER(XRCID("wxSpectrumRangeMinText"), wxSpectrumEditor::OnSpectrumSettingRangeValueEntered)
	 EVT_TEXT_ENTER(XRCID("wxSpectrumRangeMaxText"), wxSpectrumEditor::OnSpectrumSettingRangeValueEntered)
	 EVT_CHECKBOX(XRCID("wxSpectrumExtendAboveCheck"), wxSpectrumEditor::OnSpectrumSettingRangeChecked)
	 EVT_CHECKBOX(XRCID("wxSpectrumExtendBelowCheck"), wxSpectrumEditor::OnSpectrumSettingRangeChecked)
	 EVT_CHECKBOX(XRCID("wxSpectrumFixMinimumCheck"), wxSpectrumEditor::OnSpectrumSettingRangeChecked)
	 EVT_CHECKBOX(XRCID("wxSpectrumFixMaximumCheck"), wxSpectrumEditor::OnSpectrumSettingRangeChecked)
	 EVT_BUTTON(XRCID("wxSpectrumOKButton"),wxSpectrumEditor::OnSpectrumSettingListOKPressed)
	 EVT_BUTTON(XRCID("wxSpectrumApplyButton"),wxSpectrumEditor::OnSpectrumSettingListApplyPressed)
	 EVT_BUTTON(XRCID("wxSpectrumRevertButton"),wxSpectrumEditor::OnSpectrumSettingListRevertPressed)
	 EVT_BUTTON(XRCID("wxSpectrumCancelButton"),wxSpectrumEditor::OnSpectrumSettingListCancelPressed)
	 EVT_CHOICE(XRCID("wxSpectrumColourChoice"),wxSpectrumEditor::OnSpectrumColourMappingTypeChoice)
	 EVT_TEXT_ENTER(XRCID("wxSpectrumNormalisedColourRangeMin"), wxSpectrumEditor::OnSpectrumNormalisedColourRangeValueEntered)
	 EVT_TEXT_ENTER(XRCID("wxSpectrumNormalisedColourRangeMax"), wxSpectrumEditor::OnSpectrumNormalisedColourRangeValueEntered)
	 EVT_CHECKBOX(XRCID("wxSpectrumReverseCheck"), wxSpectrumEditor::OnSpectrumSettingsReverseChecked)
	 EVT_CHOICE(XRCID("wxSpectrumTypeChoice"), wxSpectrumEditor::OnSpectrumTypeChoice)
	 EVT_TEXT_ENTER(XRCID("wxSpectrumExaggerationTextCtrl"), wxSpectrumEditor::OnSpectrumExaggerationSettingsChanged)
	 EVT_RADIOBOX(XRCID("wxSpectrumLeftRightRadioBox"), wxSpectrumEditor::OnSpectrumExaggerationSettingsChanged)
	 EVT_TEXT_ENTER(XRCID("wxSpectrumDataComponentText"), wxSpectrumEditor::OnSpectrumDataValueEntered)
	 EVT_TEXT_ENTER(XRCID("wxSpectrumNumberOfBandsTextCtrl"), wxSpectrumEditor::OnSpectrumBandsValuesEntered)
	 EVT_TEXT_ENTER(XRCID("wxSpectrumRatioOfBlackBandsTextCtrl"), wxSpectrumEditor::OnSpectrumBandsValuesEntered)
	 EVT_TEXT_ENTER(XRCID("wxSpectrumStepValueTextCtrl"), wxSpectrumEditor::OnSpectrumStepValueEntered)
	 EVT_BUTTON(XRCID("wxSpectrumAutorangeButton"),wxSpectrumEditor::OnSpectrumAutorangePressed)
	 EVT_CLOSE(wxSpectrumEditor::CloseSpectrumEditor)
END_EVENT_TABLE()

static int make_current_spectrum(
	struct Spectrum_editor *spectrum_editor,
	struct cmzn_spectrum *spectrum)
/*******************************************************************************
LAST MODIFIED : 7 January 2008

DESCRIPTION :
Destroys the edit_spectrum member of <spectrum_editor> and rebuilds it as
a complete copy of <Spectrum>.
==============================================================================*/
{
	int return_code = 0;

	ENTER(make_current_spectrum);
	if (spectrum_editor)
	{
		return_code=1;
		if (spectrum)
		{
			if (!IS_MANAGED(cmzn_spectrum)(spectrum,
				spectrum_editor->spectrum_manager))
			{
#if defined (TEST_CODE)
				display_message(ERROR_MESSAGE,
					"make_current_spectrum.  Spectrum not managed");
#endif /* defined (TEST_CODE) */
				spectrum=(struct cmzn_spectrum *)NULL;
				return_code=0;
			}
		}
		if (!spectrum)
		{
			spectrum=FIRST_OBJECT_IN_MANAGER_THAT(cmzn_spectrum)(
				(MANAGER_CONDITIONAL_FUNCTION(cmzn_spectrum) *)NULL,
				(void *)NULL,
				spectrum_editor->spectrum_manager);
		}
		spectrum_editor->wx_spectrum_editor->spectrum_editor_spectrum_list_set_selected(
			 spectrum);
		if (spectrum_editor->current_spectrum != spectrum)
		{
			REACCESS(cmzn_spectrum)(&spectrum_editor->current_spectrum, spectrum);
		}
		spectrum_editor_wx_set_spectrum(
			 spectrum_editor, spectrum_editor->current_spectrum);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"make_current_spectrum.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* make_current_spectrum */

int spectrum_editor_viewer_input_callback(
	struct Scene_viewer_app *scene_viewer_app, struct Graphics_buffer_input *input,
	void *spectrum_editor_void)
/*******************************************************************************
LAST MODIFIED : 2 July 2002

DESCRIPTION :
Callback for when input is received by the scene_viewer.
==============================================================================*/
{
	int return_code;
	struct Spectrum_editor *spectrum_editor;

	spectrum_editor=(struct Spectrum_editor *)spectrum_editor_void;
	if (spectrum_editor != 0)
	{
		if (CMZN_SCENEVIEWERINPUT_EVENT_TYPE_BUTTON_PRESS==input->type)
		{
			/* Increment the type - cycles through label divisions and colours */
			if (spectrum_editor->viewer_type == 5)
				spectrum_editor->viewer_type = 0;
			else
				spectrum_editor->viewer_type++;
			make_colour_bar(spectrum_editor);
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"spectrum_editor_viewer_input_CB.  Invalid argument(s)");
		return_code = 0;
	}
	return(return_code);
} /* spectrum_editor_viewer_input_CB */

void Spectrum_editor_spectrum_change(
	 struct MANAGER_MESSAGE(cmzn_spectrum) *message, void *spectrum_editor_void)
/*******************************************************************************
LAST MODIFIED : 7 January 2008

DESCRIPTION :
Something has changed globally in the spectrum manager. Update the
current spectrum.
==============================================================================*/
{
	struct Spectrum_editor *spectrum_editor;

	ENTER(Spectrum_editor_spectrum_change);
	if (message && (spectrum_editor = (struct Spectrum_editor *)spectrum_editor_void))
	{
		if ((NULL == spectrum_editor->current_spectrum) ||
			(MANAGER_MESSAGE_GET_OBJECT_CHANGE(cmzn_spectrum)(message,
				spectrum_editor->current_spectrum) &
			MANAGER_CHANGE_REMOVE(cmzn_spectrum)))
		{
			spectrum_editor->current_spectrum =
				FIRST_OBJECT_IN_MANAGER_THAT(cmzn_spectrum)(
					NULL,(void *)NULL,	spectrum_editor->spectrum_manager);
			spectrum_editor_wx_set_spectrum(spectrum_editor,spectrum_editor->current_spectrum);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Materia_editor_materia_change.  Invalid argument(s)");
	}
	LEAVE;
} /* Materia_editor_materia_change */

struct Spectrum_editor *CREATE(Spectrum_editor)(
	 struct Spectrum_editor_dialog **spectrum_editor_dialog_address,
	 struct cmzn_spectrum *spectrum,
	 struct cmzn_font *font,
	 struct Graphics_buffer_app_package *graphics_buffer_package,
	 struct User_interface *user_interface,
	 struct cmzn_graphics_module *graphics_module,
	 cmzn_region_id root_region,
	 struct cmzn_region *spectrum_region)
/*******************************************************************************
LAST MODIFIED : 6 May 2004

DESCRIPTION :
Creates a spectrum_editor widget.
==============================================================================*/
{
	int return_code;
	struct Spectrum_editor *spectrum_editor;
	struct Colour background_colour = {0.1, 0.1, 0.1}, black ={0, 0, 0},
		white = { 1.0, 1.0, 1.0 };
	double ambient_colour[3] = {0.1, 0.1, 0.1};
	struct Graphics_buffer_app *graphics_buffer;
	struct cmzn_light *viewer_light = 0, *ambient_light = 0;
	double light_direction[3] = { 0.0, -0.2, -1.0 };

	ENTER(CREATE(Spectrum_editor));
	spectrum_editor = (struct Spectrum_editor *)NULL;
	if (user_interface && graphics_module)
	{
			/* allocate memory */
			if (ALLOCATE(spectrum_editor,struct Spectrum_editor,1))
			{
				/* initialise the structure */
				spectrum_editor->spectrum_editor_dialog_address = spectrum_editor_dialog_address;
				spectrum_editor->current_settings = (struct cmzn_spectrumcomponent *)NULL;
				spectrum_editor->current_spectrum=(struct cmzn_spectrum *)NULL;
				spectrum_editor->edit_spectrum=(struct cmzn_spectrum *)NULL;
				spectrum_editor->private_region=(struct cmzn_region *)NULL;
				spectrum_editor->user_interface=user_interface;
				spectrum_editor->material_manager_callback_id=(void *)NULL;
				spectrum_editor->spectrum_manager_callback_id=(void *)NULL;
				spectrum_editor->spectrum_manager = cmzn_graphics_module_get_spectrum_manager(
					graphics_module);
				spectrum_editor->filter_module = cmzn_graphics_module_get_scenefiltermodule(graphics_module);
				spectrum_editor->root_region = cmzn_region_access(root_region);
				spectrum_editor->editorMaterial = (cmzn_material *)NULL;
				spectrum_editor->labelMaterial = (cmzn_material *)NULL;
				spectrum_editor->colourBar = 0;
				spectrum_editor->autorange_scene = (cmzn_scene_id)NULL;
				spectrum_editor->spectrum_editor_scene = (cmzn_scene_id)NULL;
				spectrum_editor->spectrum_editor_scene_viewer = (struct Scene_viewer_app *)NULL;
				spectrum_editor->viewer_type = 0;
				spectrum_editor->spectrum_range_min_text = NULL;
				spectrum_editor->spectrum_range_max_text = NULL;
				spectrum_editor->spectrum_settings_checklist = NULL;
				spectrum_editor->spectrum_settings_add_button = NULL;
				spectrum_editor->spectrum_settings_del_button = NULL;
				spectrum_editor->spectrum_settings_up_button = NULL;
				spectrum_editor->spectrum_settings_dn_button = NULL;
				spectrum_editor->spectrum_reverse_checkbox = NULL;
				spectrum_editor->spectrum_colour_mapping_type_choice = NULL;
				spectrum_editor->spectrum_exaggeration_text = NULL;
				spectrum_editor->spectrum_left_right_radio_box = NULL;
				spectrum_editor->spectrum_data_component_text = NULL;
				spectrum_editor->spectrum_number_of_bands_text = NULL;
				spectrum_editor->spectrum_ratio_of_black_bands_text = NULL;
				spectrum_editor->spectrum_editor_step_value_text = NULL;
				spectrum_editor->spectrum_extend_below_check = NULL;
				spectrum_editor->spectrum_extend_above_check = NULL;
				spectrum_editor->spectrum_fix_minimum_check = NULL;
				spectrum_editor->spectrum_fix_maximum_check = NULL;
				spectrum_editor->spectrum_normalised_colour_range_min_text = NULL;
				spectrum_editor->spectrum_normalised_colour_range_max_text = NULL;
				spectrum_editor->spectrum_type_choice = NULL;
				spectrum_editor->spectrum_higher_panel = NULL;
				spectrum_editor->spectrum_lower_panel = NULL;
				// User_Interface
				spectrum_editor->wx_spectrum_editor = (wxSpectrumEditor *)NULL;
				wxLogNull logNo;
				spectrum_editor->wx_spectrum_editor = new wxSpectrumEditor(spectrum_editor);
				spectrum_editor->spectrum_panel = XRCCTRL(*spectrum_editor->wx_spectrum_editor
					 , "SpectrumPanel", wxPanel);
				spectrum_editor->spectrum_panel->SetSize(wxDefaultCoord,wxDefaultCoord,
					 400, 150);
				spectrum_editor->spectrum_panel->SetMinSize(wxSize(-1,150));
				spectrum_editor->spectrum_overwrite_colour_radiobox = XRCCTRL(*spectrum_editor->wx_spectrum_editor,
					 "wxSpectrumOverwriteColourRadioBox", wxRadioBox);
				spectrum_editor->spectrum_editor_frame = XRCCTRL(*spectrum_editor->wx_spectrum_editor
					 , "CmguiSpectrumEditor", wxFrame);
				spectrum_editor->spectrum_editor_frame->SetSize(wxDefaultCoord,wxDefaultCoord,
					 400, 700);
				spectrum_editor->spectrum_editor_frame->SetMinSize(wxSize(1,1));
				spectrum_editor->spectrum_range_min_text = XRCCTRL(*spectrum_editor->wx_spectrum_editor,
					 "wxSpectrumRangeMinText", wxTextCtrl);
				spectrum_editor->spectrum_range_max_text = XRCCTRL(*spectrum_editor->wx_spectrum_editor,
					 "wxSpectrumRangeMaxText", wxTextCtrl);
				spectrum_editor->spectrum_settings_checklist = XRCCTRL(*spectrum_editor->wx_spectrum_editor,
					 "wxSpectrumSettingsCheckList", wxCheckListBox);
				spectrum_editor->spectrum_settings_add_button = XRCCTRL(*spectrum_editor->wx_spectrum_editor,
					 "wxSpectrumSettingsAdd", wxButton);
				spectrum_editor->spectrum_settings_del_button = XRCCTRL(*spectrum_editor->wx_spectrum_editor,
					 "wxSpectrumSettingsDel", wxButton);
				spectrum_editor->spectrum_settings_up_button = XRCCTRL(*spectrum_editor->wx_spectrum_editor,
					 "wxSpectrumSettingsUp", wxButton);
				spectrum_editor->spectrum_settings_dn_button = XRCCTRL(*spectrum_editor->wx_spectrum_editor,
					 "wxSpectrumSettingsDn", wxButton);
				spectrum_editor->spectrum_reverse_checkbox = XRCCTRL(*spectrum_editor->wx_spectrum_editor,
					 "wxSpectrumReverseCheck", wxCheckBox);
				spectrum_editor->spectrum_colour_mapping_type_choice = XRCCTRL(*spectrum_editor->wx_spectrum_editor,
					 "wxSpectrumColourChoice", wxChoice);
				spectrum_editor->spectrum_exaggeration_text = XRCCTRL(*spectrum_editor->wx_spectrum_editor,
					 "wxSpectrumExaggerationTextCtrl", wxTextCtrl);
				spectrum_editor->spectrum_left_right_radio_box = XRCCTRL(*spectrum_editor->wx_spectrum_editor,
					 "wxSpectrumLeftRightRadioBox", wxRadioBox);
				spectrum_editor->spectrum_data_component_text = XRCCTRL(*spectrum_editor->wx_spectrum_editor,
					 "wxSpectrumDataComponentText", wxTextCtrl);
				spectrum_editor->spectrum_number_of_bands_text = XRCCTRL(*spectrum_editor->wx_spectrum_editor,
					 "wxSpectrumNumberOfBandsTextCtrl", wxTextCtrl);
				spectrum_editor->spectrum_ratio_of_black_bands_text = XRCCTRL(*spectrum_editor->wx_spectrum_editor,
					 "wxSpectrumRatioOfBlackBandsTextCtrl", wxTextCtrl);
				spectrum_editor->spectrum_editor_step_value_text = XRCCTRL(*spectrum_editor->wx_spectrum_editor,
					 "wxSpectrumStepValueTextCtrl", wxTextCtrl);
				spectrum_editor->spectrum_extend_below_check = XRCCTRL(*spectrum_editor->wx_spectrum_editor,
					 "wxSpectrumExtendBelowCheck", wxCheckBox);
				spectrum_editor->spectrum_extend_above_check = XRCCTRL(*spectrum_editor->wx_spectrum_editor,
					 "wxSpectrumExtendAboveCheck", wxCheckBox);
				spectrum_editor->spectrum_fix_minimum_check = XRCCTRL(*spectrum_editor->wx_spectrum_editor,
					 "wxSpectrumFixMinimumCheck", wxCheckBox);
				spectrum_editor->spectrum_fix_maximum_check = XRCCTRL(*spectrum_editor->wx_spectrum_editor,
					 "wxSpectrumFixMaximumCheck", wxCheckBox);
				spectrum_editor->spectrum_normalised_colour_range_min_text = XRCCTRL(*spectrum_editor->wx_spectrum_editor,
					 "wxSpectrumNormalisedColourRangeMin", wxTextCtrl);
				spectrum_editor->spectrum_normalised_colour_range_max_text = XRCCTRL(*spectrum_editor->wx_spectrum_editor,
					 "wxSpectrumNormalisedColourRangeMax", wxTextCtrl);
				spectrum_editor->spectrum_type_choice = XRCCTRL(*spectrum_editor->wx_spectrum_editor,
					 "wxSpectrumTypeChoice", wxChoice);
				spectrum_editor->spectrum_higher_panel = XRCCTRL(*spectrum_editor->wx_spectrum_editor,
					 "wxSpectrumHigherPanel", wxScrolledWindow);
				spectrum_editor->spectrum_lower_panel = XRCCTRL(*spectrum_editor->wx_spectrum_editor,
					 "wxSpectrumLowerPanel", wxScrolledWindow);
				struct cmzn_spectrum *temp_spectrum = spectrum;
				if (!temp_spectrum)
				{
					temp_spectrum = FIRST_OBJECT_IN_MANAGER_THAT(cmzn_spectrum)(
						(MANAGER_CONDITIONAL_FUNCTION(cmzn_spectrum) *)NULL,
						(void *)NULL, spectrum_editor->spectrum_manager);
				}
				make_current_spectrum(spectrum_editor, temp_spectrum);
				spectrum_editor->wx_spectrum_editor->set_autorange_scene();
				spectrum_editor->wx_spectrum_editor->Show();
				spectrum_editor->spectrum_manager_callback_id =
					MANAGER_REGISTER(cmzn_spectrum)(
							Spectrum_editor_spectrum_change, (void *)spectrum_editor,
							spectrum_editor->spectrum_manager);
				if (spectrum_editor->wx_spectrum_editor != NULL)
				{
					 spectrum_editor->spectrum_lower_panel->SetScrollbars(10,10,10,40);
					 spectrum_editor->spectrum_higher_panel->SetScrollbars(10,10,40,40);
					 return_code = 1;
					 spectrum_editor->editorMaterial = cmzn_material_create_private();
					 cmzn_material_set_name(spectrum_editor->editorMaterial, "editorMaterial");
					 Graphical_material_set_ambient(spectrum_editor->editorMaterial, &black );
					 Graphical_material_set_diffuse(spectrum_editor->editorMaterial, &black );
					 Graphical_material_set_shininess(spectrum_editor->editorMaterial, 0.3 );
					 spectrum_editor->labelMaterial = cmzn_material_create_private();
					 cmzn_material_set_name(spectrum_editor->labelMaterial, "labelMaterial");
					 Graphical_material_set_ambient(spectrum_editor->labelMaterial, &white );
					 Graphical_material_set_diffuse(spectrum_editor->labelMaterial, &white );
					 Graphical_material_set_shininess(spectrum_editor->labelMaterial, 0.3 );
					 if ( return_code )
					 {
							/* Create new rendition */
							spectrum_editor->private_region = cmzn_region_access(spectrum_region);
							spectrum_editor->spectrum_editor_scene = cmzn_scene::create(spectrum_editor->private_region,
								graphics_module);
							cmzn_lightmodule *light_module= cmzn_graphics_module_get_lightmodule(graphics_module);
							cmzn_lightmodule_begin_change(light_module);
							viewer_light = cmzn_lightmodule_create_light(light_module);
							cmzn_light_set_name(viewer_light, "spectrum_editor_light");
							cmzn_light_set_direction(viewer_light, &light_direction[0]);
							ambient_light =cmzn_lightmodule_create_light(light_module);
							cmzn_light_set_name(ambient_light, "spectrum_editor_ambient_light");
							cmzn_light_set_type(ambient_light,CMZN_LIGHT_TYPE_AMBIENT);
							cmzn_light_set_colour_rgb(ambient_light,&ambient_colour[0]);
							cmzn_lightmodule_end_change(light_module);
							cmzn_lightmodule_destroy(&light_module);
							graphics_buffer = create_Graphics_buffer_wx(
								graphics_buffer_package, spectrum_editor->spectrum_panel,
								GRAPHICS_BUFFER_ANY_BUFFERING_MODE, GRAPHICS_BUFFER_ANY_STEREO_MODE,
								/*minimum_colour_buffer_depth*/8,
								/*minimum_depth_buffer_depth*/8,
								/*minimum_accumulation_buffer_depth*/0, (struct Graphics_buffer_app *)NULL);
							if (graphics_buffer)
							{
								spectrum_editor->spectrum_editor_scene_viewer =
									Scene_viewer_app_for_spectrum_create(graphics_buffer,
										&background_colour,
										viewer_light,
										ambient_light,
										NULL,
										spectrum_editor->spectrum_editor_scene, spectrum_editor->user_interface);
								cmzn_sceneviewer_begin_change(spectrum_editor->spectrum_editor_scene_viewer->core_scene_viewer);
								Scene_viewer_set_input_mode(
									spectrum_editor->spectrum_editor_scene_viewer->core_scene_viewer,
									SCENE_VIEWER_NO_INPUT );
								cmzn_sceneviewer_set_projection_mode(
									spectrum_editor->spectrum_editor_scene_viewer->core_scene_viewer,
									CMZN_SCENEVIEWER_PROJECTION_MODE_PARALLEL);
								Scene_viewer_app_add_input_callback(
									spectrum_editor->spectrum_editor_scene_viewer,
									spectrum_editor_viewer_input_callback,(void *)spectrum_editor, 0);
								spectrum_editor->spectrum_panel->Fit();
								wxSize viewerSize = spectrum_editor->spectrum_panel->GetSize();
								cmzn_sceneviewer_set_viewport_size(
									spectrum_editor->spectrum_editor_scene_viewer->core_scene_viewer,
									viewerSize.GetWidth(), viewerSize.GetHeight());
								Scene_viewer_set_lookat_parameters(
									spectrum_editor->spectrum_editor_scene_viewer->core_scene_viewer,0,0,1,0,0,0,
									0,1,0);
								Scene_viewer_set_view_simple(
									spectrum_editor->spectrum_editor_scene_viewer->core_scene_viewer,0,0,0,0.22,
									46,10);
								if (spectrum_editor->edit_spectrum)
								{
									make_colour_bar(spectrum_editor);
								}
								spectrum_editor->spectrum_panel->SetSize(viewerSize.GetWidth()-1, viewerSize.GetHeight()-1);
								spectrum_editor->spectrum_panel->SetSize(viewerSize.GetWidth(), viewerSize.GetHeight());
								spectrum_editor->wx_spectrum_editor->Show();
								spectrum_editor->spectrum_panel->Show();
								cmzn_sceneviewer_end_change(spectrum_editor->spectrum_editor_scene_viewer->core_scene_viewer);
								DEACCESS(Graphics_buffer_app)(&graphics_buffer);
							}
							cmzn_light_destroy(&viewer_light);
							cmzn_light_destroy(&ambient_light);
					 }
				}
			}
			else
			{
				 display_message(ERROR_MESSAGE,
						"CREATE(Spectrum_editor).  "
						"Could not allocate spectrum_editor widget structure");
			}
	}
	else
	{
		 display_message(ERROR_MESSAGE,
			"CREATE(Spectrum_editor).  Invalid argument(s)");
	}
	LEAVE;

	return (spectrum_editor);
} /* CREATE(Spectrum_editor) */

int spectrum_editor_wx_set_spectrum(
	struct Spectrum_editor *spectrum_editor, struct cmzn_spectrum *spectrum)
/*******************************************************************************
LAST MODIFIED : 12 August 2002

DESCRIPTION :
Set the <spectrum> to be edited by the <spectrum_editor>.
==============================================================================*/
{
	int return_code;

	ENTER(spectrum_editor_wx_set_spectrum);
	if (spectrum_editor)
	{
		if (spectrum)
		{
			if (make_edit_spectrum(spectrum_editor,spectrum))
			{
				/* continue with the current_settings_type */
				 spectrum_editor_wx_make_settings_list(spectrum_editor);
				/* select the first settings item in the list (if any) */
				spectrum_editor->current_settings=(struct cmzn_spectrumcomponent *)NULL;
				spectrum_editor_wx_select_settings_item(spectrum_editor);
				/* turn on callbacks from settings editor */
			}
			else
			{
				spectrum=(struct cmzn_spectrum *)NULL;
			}
		}
		if (!spectrum)
		{
			/* turn off settings editor by passing NULL settings */
			spectrum_editor->current_settings=(struct cmzn_spectrumcomponent *)NULL;
			spectrum_editor_wx_set_settings(spectrum_editor);
			if (spectrum_editor->edit_spectrum)
			{
				DEACCESS(cmzn_spectrum)(&(spectrum_editor->edit_spectrum));
			}
		}
		make_colour_bar(spectrum_editor);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"spectrum_editor_wx_set_spectrum.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* spectrum_editor_set_spectrum */

void spectrum_editor_wx_bring_up_editor(struct Spectrum_editor *spectrum_editor)
/*******************************************************************************
LAST MODIFIED : 10 Jan 2008

DESCRIPTION :
bring the spectrum editor to the front.
==============================================================================*/
{
	 if (spectrum_editor->wx_spectrum_editor)
	 {
			spectrum_editor->wx_spectrum_editor->Raise();
	 }
	 else
	 {
			display_message(ERROR_MESSAGE,
				 "spectrum_editor_bring_up_editor.  Invalid argument(s)");
	 }
}

int DESTROY(Spectrum_editor)(struct Spectrum_editor **spectrum_editor_address)
/*******************************************************************************
LAST MODIFIED : 10 Jan 2008

DESCRIPTION :
Destroys the <*spectrum_editor_address> and sets
<*spectrum_editor_address> to NULL.
==============================================================================*/
{
	int return_code;
	struct Spectrum_editor *spectrum_editor;

	ENTER(DESTROY(Spectrum_editor));
	if (spectrum_editor_address &&
		(spectrum_editor = *spectrum_editor_address))
	{
		return_code = 1;
		delete spectrum_editor->wx_spectrum_editor;
		cmzn_material_destroy(&spectrum_editor->editorMaterial);
		cmzn_material_destroy(&spectrum_editor->labelMaterial);
		cmzn_glyph_colour_bar_destroy(&spectrum_editor->colourBar);
		if (spectrum_editor->autorange_scene)
		{
			cmzn_scene_destroy(&(spectrum_editor->autorange_scene));
		}
		if (spectrum_editor->root_region)
		{
			cmzn_region_destroy(&(spectrum_editor->root_region));
		}
		if (spectrum_editor->private_region)
		{
			cmzn_region_destroy(&(spectrum_editor->private_region));
		}
		if (spectrum_editor->spectrum_editor_scene)
		{
			cmzn_scene_destroy(&spectrum_editor->spectrum_editor_scene);
		}
		Scene_viewer_app_remove_input_callback(spectrum_editor->spectrum_editor_scene_viewer,
			spectrum_editor_viewer_input_callback,(void *)spectrum_editor);
		DESTROY(Scene_viewer_app)(&spectrum_editor->spectrum_editor_scene_viewer);
		/* destroy edit_spectrum */
		if (spectrum_editor->current_spectrum)
		{
			DEACCESS(cmzn_spectrum)(&spectrum_editor->current_spectrum);
		}
		if (spectrum_editor->edit_spectrum)
		{
			DEACCESS(cmzn_spectrum)(
				&(spectrum_editor->edit_spectrum));
		}
		cmzn_scenefiltermodule_destroy(&spectrum_editor->filter_module);
		if (spectrum_editor->spectrum_manager_callback_id)
		{
			 MANAGER_DEREGISTER(cmzn_spectrum)(
					spectrum_editor->spectrum_manager_callback_id,
					spectrum_editor->spectrum_manager);
			 spectrum_editor->spectrum_manager_callback_id = (void *)NULL;
		}
		DEALLOCATE(*spectrum_editor_address);
		*spectrum_editor_address = (struct Spectrum_editor *)NULL;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(Spectrum_editor).  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Spectrum_editor) */


