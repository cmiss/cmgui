/*******************************************************************************
FILE : transformation_editor_wx.hpp

LAST MODIFIED : 27 February 2008

DESCRIPTION :
Create a cpp class that act as a transformation editor for the wx widgets.
==============================================================================*/
/* OpenCMISS-Cmgui Application
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#if !defined (TRANSFORMATION_EDITOR_WX_HPP)
#define TRANSFORMATION_EDITOR_WX_HPP

#include "wx/wx.h"
#include "wx/image.h"
#include <wx/tglbtn.h>
#include <wx/statline.h>
#include <wx/spinbutt.h>

#include "general/debug.h"
#include "graphics/auxiliary_graphics_types.h"
#include "graphics/scene.h"
#include "io_devices/matrix.h"
#include "graphics/graphics_library.h"
#include "graphics/quaternion.hpp"

class Transformation_editor : public wxPanel
{

public:

	 Transformation_editor(wxPanel *parent, const char *panel_name, struct cmzn_scene *scene,
			int *auto_apply);
	 int set_transformation(gtMatrix *transformation_matrix);
	 void set_scene(struct cmzn_scene *scene);
	 void ApplyTransformation(int force_apply);

	 ~Transformation_editor()
	 {
			if (global_quat)
			{
				 DEALLOCATE(global_quat);
			}
			delete transformation_editor_quaternion;
	 }

private:

	 void connect_callback();
	 void set_properties();
	 void do_layout();
	 int position_direction_to_transformation_matrix(
			struct Dof3_data *position, struct Dof3_data *direction,
			gtMatrix *transformation_matrix);
	 void OnTransformationEditorTextEntered(wxCommandEvent& event);
	 void transformation_editor_wx_update_position_and_direction();
	 void transformation_editor_wx_get_rate_of_change_from_interface_slider();
	 void OnTransformationEditorRateofChangeSliderChanged(wxCommandEvent& event);
	 void OnTransformationEditor_spin_button_up(wxCommandEvent& event);
	 void OnTransformationEditor_spin_button_down(wxCommandEvent& event);
	 void transformation_editor_wx_spin_button_change_value(wxSpinButton *temp_object, int flag);
	 void transformation_editor_wx_direction_system_choice_changed();
	 void OnTransformationEditorDirectionSystemChoice(wxCommandEvent& event);
	 void transformation_editor_quaternion_to_gtmatrix();
	 int scale_factor_to_transformation_matrix(Triple scale_factor,
			gtMatrix *transformation_editor_transformation_matrix);

protected:
	 int *auto_apply_flag, direction_system_index;
	 struct cmzn_scene *current_scene;
	 gtMatrix transformation_editor_transformation_matrix;
	 int rate_of_change;
	 struct Dof3_data global_direction, global_position;
	 Quaternion *transformation_editor_quaternion;
	 double *global_quat;
	 wxPanel *transformation_editor_panel;
	 wxStaticLine *Transformation_editor_wx_staticline;
	 wxStaticBox* direction_sizer_8_staticbox;
	 wxStaticBox* direction_sizer_7_staticbox;
	 wxStaticBox* direction_sizer_6_staticbox;
	 wxStaticBox* direction_sizer_13_staticbox;
	 wxStaticBox* position_sizer_8_staticbox;
	 wxStaticBox* position_sizer_7_staticbox;
	 wxStaticBox* position_sizer_6_staticbox;
	 wxStaticBox* position_sizer_13_staticbox;
	 wxStaticBox* Transformation_editor_wx_position_staticbox;
	 wxStaticBox* Transformation_editor_wx_direction_staticbox;
	 wxChoice* Transformation_editor_wx_global_choice;
	 wxButton* Transformation_editor_wx_position_save_button;
	 wxButton* Transformation_editor_wx_position_reset_button;
	 wxToggleButton* Transformation_editor_wx_position_lock_data_toggle_button;
	 wxToggleButton* Transformation_editor_wx_position_link_resolution_toggle_button;
	 wxStaticText* Transformation_editor_wx_position_label_1;
	 wxChoice* Transformation_editor_wx_position_coord_system_choice;
	 wxStaticText* Transformation_editor_wx_position_label_2;
	 wxSlider* Transformation_editor_wx_position_slider_1;
	 wxStaticText* Transformation_editor_wx_position_label_3;
	 wxTextCtrl* Transformation_editor_wx_position_text_ctrl_x;
	 wxSpinButton *Transformation_editor_wx_position_spin_button_1;
	 wxStaticText* Transformation_editor_wx_position_label_4;
	 wxTextCtrl* Transformation_editor_wx_position_text_ctrl_y;
	 wxSpinButton *Transformation_editor_wx_position_spin_button_2;
	 wxStaticText* Transformation_editor_wx_position_label_5;
	 wxTextCtrl* Transformation_editor_wx_position_text_ctrl_z;
	 wxSpinButton *Transformation_editor_wx_position_spin_button_3;
	 wxStaticText* Transformation_editor_wx_position_label_6;
	 wxTextCtrl* Transformation_editor_wx_position_text_ctrl_scale_factor;
	 wxSpinButton *Transformation_editor_wx_position_spin_button_4;
	 wxButton* Transformation_editor_wx_direction_save_button;
	 wxButton* Transformation_editor_wx_direction_reset_button;
	 wxToggleButton* Transformation_editor_wx_direction_lock_data_toggle_button;
	 wxToggleButton* Transformation_editor_wx_direction_link_resolution_toggle_button;
	 wxStaticText* Transformation_editor_wx_direction_label_1;
	 wxChoice* Transformation_editor_wx_direction_coord_system_choice;
	 wxStaticText* Transformation_editor_wx_direction_label_3;
	 wxTextCtrl* Transformation_editor_wx_direction_text_ctrl_1;
	 wxSpinButton *Transformation_editor_wx_direction_spin_button_1;
	 wxStaticText* Transformation_editor_wx_direction_label_4;
	 wxTextCtrl* Transformation_editor_wx_direction_text_ctrl_2;
	 wxSpinButton *Transformation_editor_wx_direction_spin_button_2;
	 wxStaticText* Transformation_editor_wx_direction_label_5;
	 wxTextCtrl* Transformation_editor_wx_direction_text_ctrl_3;
	 wxSpinButton *Transformation_editor_wx_direction_spin_button_3;
	 wxStaticText* Transformation_editor_wx_direction_label_6;
	 wxTextCtrl* Transformation_editor_wx_direction_text_ctrl_4;
	 wxSpinButton *Transformation_editor_wx_direction_spin_button_4;
	 wxString transformation_editor_direction_text[2][3];
	 Triple global_scale_factor;
};
#endif /* !defined (TRANSFORMATION_EDITOR_WX_HPP) */
