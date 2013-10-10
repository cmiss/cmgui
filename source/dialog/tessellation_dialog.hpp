/***************************************************************************//**
 * tessellation_dialog.hpp
 *
 * Dialog for describing editing tessellation
 */
/* OpenCMISS-Cmgui Application
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <graphics/tessellation.hpp>
#include <wx/wx.h>
#include <map>

#ifndef TESSELATION_DIALOG_HPP
#define TESSELATION_DIALOG_HPP

class TessellationItem : public wxPanel {

public:

 	TessellationItem(wxWindow* parent, MANAGER(cmzn_tessellation) *tessellation_manager_in,
 		cmzn_tessellation *tessellation_in);
	void update_global();

private:

	cmzn_tessellation *tessellation;
	int labelChanged, refinementChanged, divisionsChanged, circleDivisionsChanged;
  MANAGER(cmzn_tessellation) *tessellation_manager;
 	void do_layout();
 	void set_callback();
	void update_divisions_string_for_dialog();
	void update_refinement_string_for_dialog();
	void update_circle_divisions_string_for_dialog();
	void OnTessellationTextEntered(wxCommandEvent& event);
	void OnTessellationApplyPressed(wxCommandEvent& event);

protected:
	wxTextCtrl *tessellationLabel, *refinementTextCtrl, *divisionsTextCtrl, *circleDivisionsTextCtrl;
	wxButton *applyButton;
};

class TessellationDialog: public wxDialog {
public:

    TessellationDialog(struct cmzn_tessellationmodule *tessellationmodule_in, wxWindow* parent, int id,
    	const wxPoint& pos=wxDefaultPosition, const wxSize& size=wxDefaultSize);
    int add_managed_object(cmzn_tessellation *tessellation);
    void manager_callback(struct MANAGER_MESSAGE(cmzn_tessellation) *message);
    virtual ~TessellationDialog() {
    	if (tessellation_manager_callback_id)
    	{
    		MANAGER_DEREGISTER(cmzn_tessellation)(
    			tessellation_manager_callback_id,	tessellation_manager);
    	}
    }

private:
    struct cmzn_tessellationmodule *tessellationmodule;
    MANAGER(cmzn_tessellation) *tessellation_manager;
    void *tessellation_manager_callback_id;
    void set_properties();
    void do_layout();
    void create_managed_objects_table();
		void OnTessellationDialogAddNewPressed(wxCommandEvent & event);
    std::map<cmzn_tessellation *, TessellationItem *> itemMap;

protected:
    wxStaticBox* sizer_1_staticbox;
    wxStaticText* label_1;
    wxStaticText* label_2;
    wxStaticText* label_3;
    wxStaticText* label_4;
    wxStaticText* label_5;
    wxButton *addNewButton;
    wxScrolledWindow* TessellationItemsPanel;
};
#endif // TESSELATION_DIALOG_HPP
