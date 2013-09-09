/*******************************************************************************
FILE : choose_class.hpp

LAST MODIFIED : 9 February 2007

DESCRIPTION :
Class for implementing an wxList dialog control for choosing an object
from its manager (subject to an optional conditional function). Handles manager
messages to keep the menu up-to-date.
Calls the client-specified callback routine if a different object is chosen.
==============================================================================*/
/* OpenCMISS-Cmgui Application
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#if !defined (CHOOSE_CLASS_H)
#define CHOOSE_CLASS_H

#include "general/callback_class.hpp"

template < class Object > class wxChooser : public wxChoice
/*****************************************************************************
LAST MODIFIED : 8 February 2007

DESCRIPTION :
============================================================================*/
{
private:
	Callback_base< Object > *callback;

public:
	wxChooser<Object>(wxPanel *parent,
		int number_of_items, Object *items,
		char **item_names, Object current_object,
		User_interface *user_interface) :
		wxChoice(parent, /*id*/-1, wxPoint(0,0), wxSize(-1,-1))
	{
		USE_PARAMETER(user_interface);
		callback = NULL;
		build_main_menu(number_of_items, items, item_names, current_object);

		Connect(wxEVT_COMMAND_CHOICE_SELECTED,
			wxCommandEventHandler(wxChooser::OnChoiceSelected));

		wxBoxSizer *sizer = new wxBoxSizer( wxHORIZONTAL );
		sizer->Add(this,
			wxSizerFlags(1).Align(wxALIGN_CENTER).Expand());
		parent->SetSizer(sizer);

		Show();
	}

	~wxChooser()
	 {
		if (callback)
		{
			delete callback;
		}
	 }

	void OnChoiceSelected(wxCommandEvent& event)
	{
		USE_PARAMETER(event);
		if (callback)
		{
			callback->callback_function(get_item());
		}
   }

	Object get_item()
	{
		return (static_cast<Object>(GetClientData(GetSelection())));
	}

	int set_callback(Callback_base< Object > *callback_object)
	{
		if (callback)
		{
			delete callback;
			callback = NULL;
		}
		callback = callback_object;
		return (1);
	}

	int set_item(Object new_item)
	{
		unsigned int i, return_code;
		return_code = 0;
		unsigned int number_of_items = GetCount();
		for (i = 0 ; i < number_of_items ; i++)
		{
			if (new_item == GetClientData(i))
			{
				SetSelection(i);
				return_code = 1;
				break;
			}
		}
		return (return_code);
	}

	 int get_number_of_item()
	{
		 unsigned int return_code;
		 return_code = 0;
		 return_code = GetCount();
		return (return_code);
	}

	int build_main_menu(int number_of_items,
		Object *items, char **item_names,
		Object current_item)
	{
		int current_item_index, i;
		wxString wxname;
		Clear();
		current_item_index = 0;
		for (i = 0 ; i < number_of_items ; i++)
		{
			wxname = wxString::FromAscii(item_names[i]);
			Append(wxname, items[i]);
			if (current_item == items[i])
			{
				current_item_index = i;
			}
		}
		SetSelection(current_item_index);
		return 1;
	}
};

#endif /* !defined (CHOOSE_CLASS_H) */
