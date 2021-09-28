/***************************************************************************//**
 * tessellation_dialog.cpp
 *
 * Dialog for describing editing tessellation
 */
/* OpenCMISS-Cmgui Application
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <iterator>
#include "dialog/tessellation_dialog.hpp"
#include "icon/cmiss_icon.xpm"
#include "general/debug.h"

TessellationItem::TessellationItem(wxWindow* parent, MANAGER(cmzn_tessellation) *tessellation_manager_in,
	cmzn_tessellation *tessellation_in) :
	wxPanel(parent, -1)
{
	tessellation_manager = tessellation_manager_in;
	tessellation = tessellation_in;
	labelChanged = 0;
	refinementChanged = 0;
	divisionsChanged = 0;
	circleDivisionsChanged = 0;
	char *name = cmzn_tessellation_get_name(tessellation);
	SetName(wxString::FromAscii(name));
	wxSizer *Sizer = parent->GetSizer();

	Sizer->Add(this, 0, wxEXPAND | wxALL, 5);
	tessellationLabel = new wxTextCtrl(this, wxID_ANY,
			wxEmptyString, wxDefaultPosition, wxSize(-1, 25), wxBORDER_NONE
					|wxTE_CENTRE | wxTE_PROCESS_ENTER | wxTE_PROCESS_TAB);
	tessellationLabel->ChangeValue(wxString::FromAscii(name));
	divisionsTextCtrl = new wxTextCtrl(this, wxID_ANY,
		wxEmptyString, wxDefaultPosition, wxSize(-1, 25), wxTE_CENTRE
		| wxTE_PROCESS_ENTER | wxTE_PROCESS_TAB);
	update_divisions_string_for_dialog();
	refinementTextCtrl = new wxTextCtrl(this, wxID_ANY,
		wxEmptyString, wxDefaultPosition, wxSize(-1, 25), wxTE_CENTRE
		| wxTE_PROCESS_ENTER | wxTE_PROCESS_TAB);
	update_refinement_string_for_dialog();
	circleDivisionsTextCtrl = new wxTextCtrl(this, wxID_ANY,
		wxEmptyString, wxDefaultPosition, wxSize(-1, 25), wxTE_CENTRE
		| wxTE_PROCESS_ENTER | wxTE_PROCESS_TAB);
	update_circle_divisions_string_for_dialog();
	applyButton = new wxButton(this, wxID_ANY, wxT("apply"),
			wxDefaultPosition, wxSize(-1, 30), wxBU_EXACTFIT);
	applyButton->Disable();
	do_layout();
	set_callback();
	DEALLOCATE(name);
}

void TessellationItem::do_layout()
{
	wxBoxSizer* objectPanelSizer = new wxBoxSizer(wxHORIZONTAL);
	objectPanelSizer->Add(tessellationLabel, 1, wxEXPAND
			| wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 0);
	objectPanelSizer->Add(divisionsTextCtrl, 1, wxEXPAND
			| wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 0);
	objectPanelSizer->Add(refinementTextCtrl, 1, wxEXPAND
			| wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 0);
	objectPanelSizer->Add(circleDivisionsTextCtrl, 1, wxEXPAND
			| wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 0);
	objectPanelSizer->Add(applyButton, 1, wxEXPAND | wxALIGN_CENTER_HORIZONTAL
			| wxALIGN_CENTER_VERTICAL, 0);
	SetSizer(objectPanelSizer);
	objectPanelSizer->Fit(this);
}

void TessellationItem::set_callback()
{
	divisionsTextCtrl->Connect(wxEVT_COMMAND_TEXT_ENTER,
		wxCommandEventHandler(TessellationItem::OnTessellationTextEntered),
			NULL, this);
	refinementTextCtrl->Connect(wxEVT_COMMAND_TEXT_ENTER,
		wxCommandEventHandler(TessellationItem::OnTessellationTextEntered),
			NULL, this);
	circleDivisionsTextCtrl->Connect(wxEVT_COMMAND_TEXT_ENTER,
		wxCommandEventHandler(TessellationItem::OnTessellationTextEntered),
			NULL, this);
	tessellationLabel->Connect(wxEVT_COMMAND_TEXT_ENTER,
		wxCommandEventHandler(TessellationItem::OnTessellationTextEntered),
			NULL, this);
	applyButton->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
		wxCommandEventHandler(TessellationItem::OnTessellationApplyPressed),
			NULL, this);
	divisionsTextCtrl->Connect(wxEVT_KILL_FOCUS,
		wxCommandEventHandler(TessellationItem::OnTessellationTextEntered),
			NULL, this);
	refinementTextCtrl->Connect(wxEVT_KILL_FOCUS,
		wxCommandEventHandler(TessellationItem::OnTessellationTextEntered),
			NULL, this);
	circleDivisionsTextCtrl->Connect(wxEVT_KILL_FOCUS,
		wxCommandEventHandler(TessellationItem::OnTessellationTextEntered),
			NULL, this);
	tessellationLabel->Connect(wxEVT_KILL_FOCUS,
		wxCommandEventHandler(TessellationItem::OnTessellationTextEntered),
			NULL, this);
}

void TessellationItem::update_divisions_string_for_dialog()
{
	int i = 0;
	int number_of_divisions = cmzn_tessellation_get_minimum_divisions(tessellation, 0, 0);
	wxString temp;
	if (number_of_divisions > 0)
	{
		int *divisions = new int[number_of_divisions];
		cmzn_tessellation_get_minimum_divisions(tessellation,
				number_of_divisions, divisions);
		temp.Printf(wxString::FromAscii("%d"), divisions[0]);
		for (i = 1; i < number_of_divisions; i++)
		{
			char temp_char[20];
			sprintf(temp_char, "*%d", divisions[i]);
			temp.append(wxString::FromAscii(temp_char));
		}
		delete [] divisions;
	}
	else
	{
		temp = "0";
	}
	divisionsTextCtrl->ChangeValue(temp);
}

void TessellationItem::update_refinement_string_for_dialog()
{
	int i = 0;
	int number_of_refinements = cmzn_tessellation_get_refinement_factors(tessellation, 0, 0);
	wxString temp;
	if (number_of_refinements > 0)
	{
		int *refinements = new int[number_of_refinements];
		cmzn_tessellation_get_refinement_factors(tessellation,
				number_of_refinements, refinements);
		temp.Printf(wxString::FromAscii("%d"), refinements[0]);
		for (i = 1; i < number_of_refinements; i++)
		{
			char temp_char[20];
			sprintf(temp_char, "*%d", refinements[i]);
			temp.append(wxString::FromAscii(temp_char));
		}
		delete [] refinements;
	}
	else
	{
		temp = "0";
	}
	refinementTextCtrl->ChangeValue(temp);
}

void TessellationItem::update_circle_divisions_string_for_dialog()
{
	int circleDivisions = cmzn_tessellation_get_circle_divisions(tessellation);
	char temp_char[20];
	sprintf(temp_char, "%d", circleDivisions);
	wxString temp(temp_char);
	circleDivisionsTextCtrl->ChangeValue(temp);
}

void TessellationItem::OnTessellationTextEntered(wxCommandEvent& event)
{
	wxTextCtrl *textctrl = (wxTextCtrl *)event.GetEventObject();
	if (textctrl->IsModified())
	{
		if (tessellationLabel == textctrl)
		{
			labelChanged = 1;
		}
		else if (refinementTextCtrl == textctrl)
		{
			wxString temp = textctrl->GetValue();
			int size = 0, *values = NULL;
			if (string_to_divisions(temp.mb_str(wxConvUTF8), &values, &size))
			{
				refinementChanged = 1;
			}
			else
			{
				update_refinement_string_for_dialog();
				refinementChanged = 0;
			}
			if (values)
			{
				DEALLOCATE(values);
			}
		}
		else if (divisionsTextCtrl == textctrl)
		{
			wxString temp = textctrl->GetValue();
			int size = 0, *values = NULL;
			if (string_to_divisions(temp.mb_str(wxConvUTF8), &values, &size))
			{
				divisionsChanged = 1;
			}
			else
			{
				update_divisions_string_for_dialog();
				divisionsChanged = 0;
			}
			if (values)
			{
				DEALLOCATE(values);
			}
		}
		else if (circleDivisionsTextCtrl == textctrl)
		{
			wxString temp = textctrl->GetValue();
			int circleDivisions = atoi(temp.mb_str(wxConvUTF8));
			if (circleDivisions > 0)
			{
				circleDivisionsChanged = 1;
			}
			else
			{
				update_circle_divisions_string_for_dialog();
			}
		}
		if (labelChanged || divisionsChanged || refinementChanged || circleDivisionsChanged)
		{
			applyButton->Enable();
		}
		else
		{
			applyButton->Disable();
		}
	}
}

void TessellationItem::OnTessellationApplyPressed(wxCommandEvent& event)
{
	USE_PARAMETER(event);

	MANAGER_BEGIN_CACHE(cmzn_tessellation)(tessellation_manager);
	if (refinementChanged)
	{
		wxString temp = refinementTextCtrl->GetValue();
		int size = 0, *values = NULL;
		if (string_to_divisions(temp.mb_str(wxConvUTF8), &values, &size))
		{
			cmzn_tessellation_set_refinement_factors(tessellation, size, values);
		}
		if (values)
		{
			DEALLOCATE(values);
		}
		refinementChanged = 0;
	}
	if (labelChanged)
	{
		wxString temp = tessellationLabel->GetValue();
		if (!cmzn_tessellation_set_name(tessellation, temp.mb_str(wxConvUTF8)))
		{
			char *name = cmzn_tessellation_get_name(tessellation);
			tessellationLabel->ChangeValue(wxString::FromAscii(name));
			DEALLOCATE(name);
		}
		else
		{
			SetName(temp);
		}
		labelChanged = 0;
	}
	if (divisionsChanged)
	{
		wxString temp = divisionsTextCtrl->GetValue();
		int size = 0, *values = NULL;
		if (string_to_divisions(temp.mb_str(wxConvUTF8), &values, &size))
		{
			cmzn_tessellation_set_minimum_divisions(tessellation, size, values);
		}
		if (values)
		{
			DEALLOCATE(values);
		}
		divisionsChanged = 0;
	}
	if (circleDivisionsChanged)
	{
		wxString temp = circleDivisionsTextCtrl->GetValue();
		int circleDivisions = atoi(temp.mb_str(wxConvUTF8));
		if (circleDivisions > 0)
		{
			cmzn_tessellation_set_circle_divisions(tessellation, circleDivisions);
		}
		circleDivisionsChanged = 0;
	}
	applyButton->Disable();
	MANAGER_END_CACHE(cmzn_tessellation)(tessellation_manager);
}

void TessellationItem::update_global()
{
	update_refinement_string_for_dialog();
	update_divisions_string_for_dialog();
	char *name = cmzn_tessellation_get_name(tessellation);
	tessellationLabel->ChangeValue(wxString::FromAscii(name));
	SetName(wxString::FromAscii(name));
	DEALLOCATE(name);
	labelChanged = 0;
	refinementChanged = 0;
	divisionsChanged = 0;
	applyButton->Disable();
}

int TessellationDialog_add_managed_object(cmzn_tessellation *tessellation, void *Tdlg_void)
{
	TessellationDialog *Tdlg = (TessellationDialog *)Tdlg_void;
	int return_code = Tdlg->add_managed_object(tessellation);
	return return_code;
}

void TessellationManagerCallback(struct MANAGER_MESSAGE(cmzn_tessellation) *message,
	void *TessellationDialog_void)
{
	TessellationDialog *dialog = (TessellationDialog *)TessellationDialog_void;
	dialog->manager_callback(message);
}

TessellationDialog::TessellationDialog(cmzn_context *contextIn, cmzn_tessellationmodule *tessellationmoduleIn, wxWindow* parent,
		int id, const wxPoint& pos, const wxSize& size) :
	context(cmzn_context_access(contextIn)),
	tessellationmodule(tessellationmoduleIn),
	tessellation_manager(cmzn_tessellationmodule_get_manager(this->tessellationmodule)),
	tessellation_manager_callback_id(nullptr),
	wxDialog(parent, id, wxString::FromAscii("Tessellation Editor"), pos, size, wxDEFAULT_FRAME_STYLE|wxDIALOG_NO_PARENT)
{
	sizer_1_staticbox = new wxStaticBox(this, -1, wxEmptyString);
	label_1 = new wxStaticText(this, wxID_ANY, wxT("Tessellation\n"), wxDefaultPosition,
		wxDefaultSize, wxALIGN_CENTRE);
	label_2 = new wxStaticText(this, wxID_ANY, wxT("Minimum\n Divisions\n#*#*...\n"),
		wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE);
	label_3 = new wxStaticText(this, wxID_ANY, wxT("Refinement\n Factors\n#*#*...\n"),
		wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE);
	label_4 = new wxStaticText(this, wxID_ANY, wxT("Circle\n Divisions\n# >= 3\n"),
		wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE);
	label_5 = new wxStaticText(this, wxID_ANY, wxT(""),
		wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE);
	TessellationItemsPanel = new wxScrolledWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxVSCROLL);
	addNewButton = new wxButton(TessellationItemsPanel, wxID_ANY, wxT("add tessellation"),
		wxPoint(-1,-1), wxSize(-1,25), wxBU_EXACTFIT);
		this->SetIcon(cmiss_icon_xpm);
	set_properties();
	do_layout();
	itemMap.clear();
	create_managed_objects_table();
	this->tessellation_manager_callback_id = MANAGER_REGISTER(cmzn_tessellation)(TessellationManagerCallback,
		(void *)this, tessellation_manager);
	addNewButton->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
	wxCommandEventHandler(TessellationDialog::OnTessellationDialogAddNewPressed),
	NULL, this);
}

int TessellationDialog::add_managed_object(cmzn_tessellation *tessellation)
{
	TessellationItem *objectPanel = new TessellationItem(TessellationItemsPanel,
			tessellation_manager, tessellation);
	if (objectPanel)
	{
		itemMap.insert(std::make_pair(tessellation, objectPanel));
		return 1;
	}
	else
	{
		return 0;
	}
}

void TessellationDialog::create_managed_objects_table()
{
	FOR_EACH_OBJECT_IN_MANAGER(cmzn_tessellation)(
		TessellationDialog_add_managed_object, (void *)this,	tessellation_manager);
	wxSizer *sizer = TessellationItemsPanel->GetSizer();
	sizer->Detach(addNewButton);
	sizer->Add(addNewButton, 0 ,wxALIGN_LEFT|wxALIGN_BOTTOM, 0);
	sizer->FitInside(TessellationItemsPanel);
	GetSizer()->Fit(this);
  Layout();
}

void TessellationDialog::set_properties()
{
	SetTitle(wxT("Tessellation Editor"));
}

void TessellationDialog::do_layout()
{
	wxStaticBoxSizer* sizer_1 = new wxStaticBoxSizer(sizer_1_staticbox, wxVERTICAL);
	wxBoxSizer* sizer_2 = new wxBoxSizer(wxHORIZONTAL);
		wxBoxSizer *panel_sizer = new wxBoxSizer( wxVERTICAL );
	sizer_2->Add(label_1, 1, wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_BOTTOM, 0);
	sizer_2->Add(label_2, 1, wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_BOTTOM, 0);
	sizer_2->Add(label_3, 1, wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_BOTTOM, 0);
	sizer_2->Add(label_4, 1, wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_BOTTOM, 0);
	sizer_2->Add(label_5, 1, wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_BOTTOM, 0);
	panel_sizer->Add(addNewButton, 0 ,wxALIGN_LEFT|wxALIGN_BOTTOM, 0);
	TessellationItemsPanel->SetSizer(panel_sizer);
	sizer_1->Add(sizer_2, 0, wxEXPAND, 0);
	sizer_1->Add(TessellationItemsPanel, 1, wxEXPAND, 0);
	SetSizer(sizer_1);
	sizer_1->Fit(this);
	Layout();
}

void TessellationDialog::OnTessellationDialogAddNewPressed(wxCommandEvent &event)
{
	USE_PARAMETER(event);
	MANAGER_BEGIN_CACHE(cmzn_tessellation)(tessellation_manager);
	cmzn_tessellation *new_tessellation =
		cmzn_tessellationmodule_create_tessellation(tessellationmodule);
	cmzn_tessellation_set_managed(new_tessellation, true);
	cmzn_tessellation_destroy(&new_tessellation);
	MANAGER_END_CACHE(cmzn_tessellation)(tessellation_manager);
}

void TessellationDialog::manager_callback(struct MANAGER_MESSAGE(cmzn_tessellation) *message)
{
	wxSizer *sizer = TessellationItemsPanel->GetSizer();
	std::map<cmzn_tessellation *, TessellationItem *>::iterator pos;
	wxSize oldSize = GetSizer()->GetMinSize();
	int change_flags = 0;
	for (pos = itemMap.begin(); pos != itemMap.end();)
	{
		change_flags = MANAGER_MESSAGE_GET_OBJECT_CHANGE(cmzn_tessellation)(
			message, pos->first);
		if (change_flags & MANAGER_CHANGE_RESULT(cmzn_tessellation))
		{
			TessellationItem *itemPanel = pos->second;
			if (itemPanel)
			{
				itemPanel->update_global();
			}
			++pos;
		}
		else if (change_flags & MANAGER_CHANGE_REMOVE(cmzn_tessellation))
		{
			sizer->Detach(pos->second);
			sizer->Layout();
			delete pos->second;
			itemMap.erase(pos++);
		}
		else
		{
			++pos;
		}
	}
	struct LIST(cmzn_tessellation) *changed_tessellation_list =
		MANAGER_MESSAGE_GET_CHANGE_LIST(cmzn_tessellation)(message,
			MANAGER_CHANGE_ADD(cmzn_tessellation));
	if (changed_tessellation_list)
	{
		FOR_EACH_OBJECT_IN_LIST(cmzn_tessellation)(
			TessellationDialog_add_managed_object, (void *)this, changed_tessellation_list);
		DESTROY_LIST(cmzn_tessellation)(&changed_tessellation_list);
		sizer->Detach(addNewButton);
		sizer->Add(addNewButton, 0 ,wxALIGN_LEFT|wxALIGN_BOTTOM, 0);
	}
	sizer->Fit(TessellationItemsPanel);
	wxSize newSize = GetSizer()->GetMinSize();
	wxSize originalSize = GetSize();
	int height = newSize.GetHeight();
	if (originalSize.GetHeight() > height)
	{
		height = originalSize.GetHeight();
	}
	else
	{
		height = originalSize.GetHeight() + height - oldSize.GetHeight();
	}
	SetSize(wxSize(originalSize.GetWidth(), height));
}
