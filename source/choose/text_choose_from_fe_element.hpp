/*******************************************************************************
FILE : text_choose_from_fe_element.hpp

LAST MODIFIED : 28 March 2007

DESCRIPTION :
Macros for implementing an option menu dialog control for choosing an object
from its manager. Handles manager messages to keep the menu up-to-date.
Calls the client-specified callback routine if a different object is chosen.
==============================================================================*/
/* OpenCMISS-Cmgui Application
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#if !defined (TEXT_CHOOSE_FROM_FE_ELEMENT_H)
#define TEXT_CHOOSE_FROM_FE_ELEMENT_H

#include <stdio.h>
#include "wx/wx.h"
#include "zinc/element.h"
#include "zinc/fieldmodule.h"
#include "computed_field/field_module.hpp"
#include "general/callback_class.hpp"
#include "general/debug.h"
#include "finite_element/finite_element_mesh.hpp"
#include "finite_element/finite_element_region.h"
#include "general/message.h"
#include "user_interface/user_interface.h"

struct FE_element;

class wxFeElementTextChooser : public wxTextCtrl
{
private:
	struct FE_element *current_object,*last_updated_object;
	cmzn_region_id region;
	cmzn_fieldmodulenotifier_id fieldmodulenotifier;
	LIST_CONDITIONAL_FUNCTION(FE_element) *conditional_function;
	void *conditional_function_user_data;
	Callback_base<FE_element*> *callback;

public:
	wxFeElementTextChooser(wxWindow *parent,
			FE_element *initial_object, cmzn_region_id regionIn,
			LIST_CONDITIONAL_FUNCTION(FE_element) *conditional_function,
			void *conditional_function_user_data) :
		wxTextCtrl(parent, /*id*/-1, wxT("") ,wxPoint(0,0), wxSize(-1,-1),wxTE_PROCESS_ENTER),
		current_object(0),
		last_updated_object(0),
		region(0),
		fieldmodulenotifier(0),
		conditional_function(conditional_function),
		conditional_function_user_data(conditional_function_user_data),
		callback(0)
	{
		this->setRegion(regionIn);
		Connect(wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler(wxFeElementTextChooser::OnTextEnter));
		Connect(wxEVT_KILL_FOCUS, wxCommandEventHandler(wxFeElementTextChooser::OnTextEnter));
		select_object(initial_object);
		wxBoxSizer *sizer = new wxBoxSizer( wxHORIZONTAL );
		sizer->Add(this, wxSizerFlags(1).Align(wxALIGN_CENTER).Expand());
		parent->SetSizer(sizer);
		Show();
	}

	~wxFeElementTextChooser()
	{
		cmzn_fieldmodulenotifier_destroy(&this->fieldmodulenotifier);
	}

	int set_callback(Callback_base<FE_element*> *callback_object)
	{
		callback = callback_object;
		return (1);
	}

/*
Module variables
----------------
*/

	int update()
/*****************************************************************************
LAST MODIFIED : 28 January 2003

DESCRIPTION :
Tells CMGUI about the current values. Sends a pointer to the current object.
Avoids sending repeated updates if the object address has not changed.
============================================================================*/
{
	int return_code;

	ENTER(TEXT_CHOOSE_FROM_FE_REGION_UPDATE(object_type));
		if (current_object != last_updated_object)
		{
#if defined (NEW_CODE)
			if (update_callback.procedure)
			{
				/* now call the procedure with the user data and the position data */
				(update_callback.procedure)(
					widget, update_callback.data,
					current_object);
			}
#endif // defined (NEW_CODE)
			last_updated_object = current_object;
		}
		return_code=1;
	LEAVE;

	return (return_code);
} /* TEXT_CHOOSE_FROM_FE_REGION_UPDATE(object_type) */

	int select_object(struct FE_element *new_object)
/*****************************************************************************
LAST MODIFIED : 30 April 2003

DESCRIPTION :
Makes sure the <new_object> is valid for this text chooser, then calls an
update in case it has changed, and writes the new object string in the widget.
============================================================================*/ \
	{
	const char *current_string;
	static const char *null_object_name="<NONE>";
	int return_code;

	ENTER(TEXT_CHOOSE_FROM_FE_REGION_SELECT_OBJECT(object_type));
	wxString tmp = GetValue();
	current_string = tmp.mb_str(wxConvUTF8);
	if (current_string)
	{
		FE_region *fe_region = cmzn_region_get_FE_region(this->region);
		FE_mesh *fe_mesh;
		if (new_object &&
			(fe_mesh = FE_region_find_FE_mesh_by_dimension(fe_region, cmzn_element_get_dimension(new_object))) &&
			((!(fe_mesh->containsElement(new_object)) ||
				(conditional_function &&
				!(conditional_function(new_object, conditional_function_user_data))))))
		{
			display_message(ERROR_MESSAGE,
				"TEXT_CHOOSE_FROM_FE_REGION_SELECT_OBJECT(object_type).  Invalid object");
				new_object=(struct FE_element *)NULL;
		}
		if (new_object)
		{
			current_object=new_object;
		}
		else
		{
			if (!current_object)
			{
				current_object=
					FE_region_get_first_FE_element_that(
						fe_region,
						conditional_function,
						conditional_function_user_data);
			}
		}
		/* write out the current_object */
		if (current_object)
		{
			char object_name[30];
			sprintf(object_name, "%d", cmzn_element_get_identifier(current_object));
			if (strcmp(object_name,current_string))
			{
				SetValue(wxString::FromAscii(object_name));
			}
		}
		else
		{
			if (strcmp(null_object_name,current_string))
			{
				SetValue(wxString::FromAscii(null_object_name));
			}
		}
		/* inform the client of any change */
		update();
		return_code=1;
	}
	else
	{
		return_code = 0;
	}

	LEAVE;

	return (return_code);
} /* TEXT_CHOOSE_FROM_FE_REGION_SELECT_OBJECT(object_type) */

/**
 * Callback from fieldmodule.
 * Updates the chosen object and text field in response to events.
 */
static void cmzn_fieldmoduleevent_to_chooser(cmzn_fieldmoduleevent_id event, void *chooser_void)
{
	wxFeElementTextChooser *chooser = static_cast<wxFeElementTextChooser *>(chooser_void);
	if (chooser && chooser->current_object)
	{
		cmzn_fieldmodule_id fieldmodule = cmzn_region_get_fieldmodule(chooser->region);
		int dimension = cmzn_element_get_dimension(chooser->current_object);
		cmzn_mesh_id mesh = cmzn_fieldmodule_find_mesh_by_dimension(fieldmodule, dimension);
		cmzn_meshchanges_id meshchanges = cmzn_fieldmoduleevent_get_meshchanges(event, mesh);
		cmzn_element_change_flags change = cmzn_meshchanges_get_element_change_flags(meshchanges, chooser->current_object);
		if (change & CMZN_ELEMENT_CHANGE_FLAG_REMOVE)
			chooser->select_object(static_cast<cmzn_element_id>(0));
		cmzn_meshchanges_destroy(&meshchanges);
		cmzn_mesh_destroy(&mesh);
		cmzn_fieldmodule_destroy(&fieldmodule);
	}
}

int GetCallback(struct FE_element *new_object)
/*****************************************************************************
LAST MODIFIED : 28 January 2003

DESCRIPTION :
Returns a pointer to the callback item of the text_choose_object_widget.
============================================================================*/
{
	//	struct Callback_data *return_address;

	ENTER(TEXT_CHOOSE_FROM_FE_REGION_GET_CALLBACK(FE_element));

	USE_PARAMETER(new_object);
		/* Get the pointer to the data for the text_choose_object dialog */
	//		text_choose_object = wxFeElementTextChooser->GetValue();
#if defined (NEW_CODE)
		if (text_choose_object)
		{
			//			return_address=&(update_callback);
		}
		else
		{
// 			display_message(ERROR_MESSAGE,
// 				"TEXT_CHOOSE_FROM_FE_REGION_GET_CALLBACK(" #object_type
// 				").  Missing widget data");
			//		return_address=(struct FE_element *)NULL;
		}
#endif
	LEAVE;

	return 1;
} /* TEXT_CHOOSE_FROM_FE_REGION_GET_CALLBACK(object_type) */

int SetCallback(struct FE_element *new_object)
/***************************************************************************** \
LAST MODIFIED : 28 January 2003 \
\
DESCRIPTION : \
Changes the callback item of the text_choose_object_widget. \
============================================================================*/
{
	int return_code;
	ENTER(TEXT_CHOOSE_FROM_FE_REGION_SET_CALLBACK(object_type));

	USE_PARAMETER(new_object);
	/* Get the pointer to the data for the text_choose_object dialog */
	//	text_choose_object = wxFeElementTextChooser->GetValue();
#if defined (NEW_CODE)
	if (text_choose_object)
	{
		update_callback.procedure=
			new_callback->procedure;
		update_callback.data=new_callback->data;
		return_code=1;
	}
	else
		{
			display_message(ERROR_MESSAGE,
			 "TEXT_CHOOSE_FROM_FE_REGION_SET_CALLBACK(" #object_type
				").  Missing widget data");
			return_code=0;
		}
	}
#endif

	LEAVE;

	return (return_code);
} /* TEXT_CHOOSE_FROM_FE_REGION_SET_CALLBACK(object_type) */

int setRegion(cmzn_region_id regionIn)
{
	if (regionIn != this->region)
	{
		if (this->region)
			cmzn_fieldmodulenotifier_destroy(&this->fieldmodulenotifier);
		this->region = regionIn;
		this->select_object((struct FE_element *)NULL);
		if (this->region)
		{
			cmzn_fieldmodule_id fieldmodule = cmzn_region_get_fieldmodule(this->region);
			this->fieldmodulenotifier = cmzn_fieldmodule_create_fieldmodulenotifier(fieldmodule);
			cmzn_fieldmodulenotifier_set_callback(this->fieldmodulenotifier,
				wxFeElementTextChooser::cmzn_fieldmoduleevent_to_chooser, (void *)this);
			cmzn_fieldmodule_destroy(&fieldmodule);
		}
	}
	return 1;
}

	struct FE_element *get_object()
/***************************************************************************** \
LAST MODIFIED : 28 January 2003 \
\
DESCRIPTION : \
Returns the currently chosen object in the text_choose_object_widget. \
============================================================================*/
{
	wxString tmp = GetValue();
	FE_region *fe_region = cmzn_region_get_FE_region(this->region);
	const int highest_dimension = FE_region_get_highest_dimension(fe_region);
	FE_mesh *fe_mesh = FE_region_find_FE_mesh_by_dimension(fe_region, highest_dimension);
	FE_element *new_object = 0;
	if (fe_mesh)
		new_object = fe_mesh->findElementByIdentifier(atoi(tmp.mb_str(wxConvUTF8)));
	select_object(new_object);
	return current_object;
}

int set_object(struct FE_element *new_object)
/*****************************************************************************
LAST MODIFIED : 28 January 2003 \
\
DESCRIPTION : \
Changes the chosen object in the text_choose_object_widget. \
============================================================================*/
{
	int return_code;

	ENTER(TEXT_CHOOSE_FROM_FE_REGION_SET_OBJECT(object_type));
	last_updated_object=new_object;
	return_code = select_object(new_object);

	LEAVE;
	return (return_code);

} /* TEXT_CHOOSE_FROM_FE_REGION_SET_OBJECT(object_type) */

void OnTextEnter(wxCommandEvent& event)
{
	USE_PARAMETER(event);
	notify_callback();
}

int notify_callback()
{
	if (callback)
	{
		callback->callback_function(get_object());
	}
	return (1);
}

};
#endif /*defined(TEXT_CHOOSE_FROM_FE_ELEMENT_H)*/
