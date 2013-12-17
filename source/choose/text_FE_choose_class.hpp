/**
 * FILE : text_FE_choose_class.hpp
 *
 * Template and classes for implementing an text dialog control for choosing
 * object from a region.
 */
/* OpenCMISS-Cmgui Application
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#if !defined ( TEXT_FE_CHOOSE_CLASS_HPP)
#define TEXT_FE_CHOOSE_CLASS_HPP

#include <stdio.h>

#include "wx/wx.h"
#include "general/callback_class.hpp"

#include "general/debug.h"
#include "general/message.h"
#include "user_interface/user_interface.h"

template < class FE_object > class FE_object_text_chooser : public wxTextCtrl
{
public:
	typedef int conditional_function_type(FE_object *object, void *user_data);
private:
	cmzn_region_id region;
	cmzn_field_domain_type domain_type;
	cmzn_fieldmodulenotifier_id fieldmodulenotifier;
	FE_object *current_object, *last_updated_object;
	conditional_function_type *conditional_function;
	void *conditional_function_user_data;
	Callback_base<FE_object*> *update_callback;

public:
	FE_object_text_chooser(wxWindow *parent, cmzn_region_id region,
		cmzn_field_domain_type domain_type, FE_object *initial_object,
		conditional_function_type *conditional_function,
		void *conditional_function_user_data) :
			wxTextCtrl(parent, /*id*/-1, wxT("") ,wxPoint(0,0), wxSize(-1,-1),wxTE_PROCESS_ENTER),
			region(0), domain_type(domain_type), fieldmodulenotifier(),
			current_object(0), last_updated_object(0),
			conditional_function(conditional_function),
			conditional_function_user_data(conditional_function_user_data)
	{
		update_callback = (Callback_base<FE_object*> *)NULL;
		Connect(wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler(FE_object_text_chooser::OnTextEnter));
		Connect(wxEVT_KILL_FOCUS, wxCommandEventHandler(FE_object_text_chooser::OnTextEnter));
		this->set_region(region);
		select_object(initial_object);
		wxBoxSizer *sizer = new wxBoxSizer( wxHORIZONTAL );
		sizer->Add(this, wxSizerFlags(1).Align(wxALIGN_CENTER).Expand());
		parent->SetSizer(sizer);
		typedef int (FE_object_text_chooser::*Member_function)(FE_object *);
		update_callback = new Callback_member_callback<FE_object*,
			FE_object_text_chooser, Member_function>(
				this, &FE_object_text_chooser::text_chooser_callback);
		Show();
	}

	~FE_object_text_chooser()
	{
		cmzn_fieldmodulenotifier_destroy(&this->fieldmodulenotifier);
		if (update_callback)
			delete update_callback;
	}

	void set_region(cmzn_region *regionIn)
	{
		cmzn_fieldmodulenotifier_destroy(&this->fieldmodulenotifier);
		this->region = regionIn;
		cmzn_fieldmodule_id fieldmodule = cmzn_region_get_fieldmodule(region);
		this->fieldmodulenotifier = cmzn_fieldmodule_create_fieldmodulenotifier(fieldmodule);
		cmzn_fieldmodulenotifier_set_callback(this->fieldmodulenotifier,
			FE_object_text_chooser::fieldmoduleevent, static_cast<void *>(this));
		cmzn_fieldmodule_destroy(&fieldmodule);
	}

	int text_chooser_callback(FE_object *object)
	{
		if (update_callback)
			update_callback->callback_function(object);
		return 1;
	}

	Callback_base<FE_object*> *get_callback()
	{
		return update_callback;
	}

	void set_callback(Callback_base<FE_object*> *new_callback)
	{
		if (update_callback)
			delete update_callback;
		update_callback = new_callback;
	}

	// note: returned object is not accessed
	FE_object *get_object()
	{
		wxString tmpString = GetValue();
		// hardcoded for nodes
		cmzn_fieldmodule_id fieldmodule = cmzn_region_get_fieldmodule(region);
		cmzn_nodeset_id nodeset = cmzn_fieldmodule_find_nodeset_by_field_domain_type(
			fieldmodule, this->domain_type);
		int identifier = atoi(tmpString.mb_str(wxConvUTF8));
		FE_object *return_object = cmzn_nodeset_find_node_by_identifier(nodeset, identifier);
		cmzn_nodeset_destroy(&nodeset);
		cmzn_fieldmodule_destroy(&fieldmodule);
		select_object(return_object);
		if (return_object)
		{
			// dodgy: avoid accessing
			FE_object *tmp = return_object;
			cmzn_node_destroy(&tmp);
			return return_object;
		}
		return 0;
	}

	void set_object(FE_object *new_object)
	{
		last_updated_object = new_object;
		select_object(new_object);
	}

	/**
	 * Tells CMGUI about the current values. Sends a pointer to the current object.
	 * Avoids sending repeated updates if the object address has not changed.
	 */
	void update()
	{
		if (current_object != last_updated_object)
		{
#if defined (NEW_CODE)
			if (update_callback.procedure)
				(update_callback.procedure)(widget, update_callback.data, current_object);
#endif // defined (NEW_CODE)
			last_updated_object = current_object;
		}
	}

	/**
	 * Makes sure the <new_object> is valid for this text chooser, then calls an
	 * update in case it has changed, and writes the new object string in the widget.
	 */
	void select_object(FE_object *new_object)
	{
		static const char *null_object_name = "<NONE>";

		wxString tmpstr = GetValue();
		const char *current_string = tmpstr.mb_str(wxConvUTF8);

		// hard-coded for node
		cmzn_fieldmodule_id fieldmodule = cmzn_region_get_fieldmodule(region);
		cmzn_nodeset_id nodeset = cmzn_fieldmodule_find_nodeset_by_field_domain_type(
			fieldmodule, this->domain_type);

		if (current_string != 0)
		{
			if (new_object && ((!cmzn_nodeset_contains_node(nodeset, new_object)) ||
				(conditional_function &&
					!(conditional_function(new_object, conditional_function_user_data)))))
			{
				display_message(ERROR_MESSAGE,
					"TEXT_CHOOSE_FROM_FE_REGION_SELECT_OBJECT(object_type).  Invalid object");
				new_object = (FE_object *)0;
			}
			if (new_object)
				current_object = new_object;
			else if (!current_object)
			{
				cmzn_nodeiterator_id iter = cmzn_nodeset_create_nodeiterator(nodeset);
				while (0 != (current_object = cmzn_nodeiterator_next(iter)))
				{
					// remove access on current_object
					cmzn_node_id tmp = current_object;
					cmzn_node_destroy(&tmp);
					if ((!this->conditional_function) ||
						this->conditional_function(current_object, this->conditional_function_user_data))
						break;
				}
			}

			/* write out the current_object */
			if (current_object)
			{
				char object_name[50];
				sprintf(object_name, "%d", cmzn_node_get_identifier(current_object));
				set_item(object_name);
			}
			else if (strcmp(null_object_name, current_string))
				set_item(null_object_name);
			/* inform the client of any change */
			update();
		}
		cmzn_nodeset_destroy(&nodeset);
		cmzn_fieldmodule_destroy(&fieldmodule);
	}

	/**
	 * Updates the chosen object and text field in response to messages.
	 */
	static void fieldmoduleevent(cmzn_fieldmoduleevent_id event,
		void *text_choose_object_void)
	{
		FE_object_text_chooser *chooser = static_cast<FE_object_text_chooser *>(text_choose_object_void);
		if (event && chooser)
		{
			cmzn_fieldmodule_id fieldmodule = cmzn_region_get_fieldmodule(chooser->region);
			cmzn_nodeset_id nodeset = cmzn_fieldmodule_find_nodeset_by_field_domain_type(fieldmodule, chooser->domain_type);
			cmzn_nodesetchanges_id nodesetchanges = cmzn_fieldmoduleevent_get_nodesetchanges(event, nodeset);
			cmzn_node_change_flags nodeChange = cmzn_nodesetchanges_get_node_change_flags(nodesetchanges, chooser->current_object);
			if (nodeChange & CMZN_NODE_CHANGE_FLAG_REMOVE)
				chooser->select_object((FE_object *)0);
			cmzn_nodesetchanges_destroy(&nodesetchanges);
			cmzn_nodeset_destroy(&nodeset);
			cmzn_fieldmodule_destroy(&fieldmodule);
		}
	}

	void set_item(const char *new_item)
	{
		if (new_item)
			SetValue(wxString::FromAscii(new_item));
	}

	void OnTextEnter(wxCommandEvent& event)
	{
		USE_PARAMETER(event);
		if (update_callback)
			update_callback->callback_function(get_object());
	}
};

#endif /* TEXT_FE_CHOOSE_CLASS_HPP */
