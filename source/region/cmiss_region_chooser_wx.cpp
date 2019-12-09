/*******************************************************************************
FILE : cmiss_region_chooser_wx.cpp

LAST MODIFIED : 22 February 2007

DESCRIPTION :
==============================================================================*/
/* OpenCMISS-Cmgui Application
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#include "opencmiss/zinc/region.h"
#include "region/cmiss_region_chooser_wx.hpp"
#include "general/debug.h"
#include "general/mystring.h"
#include "region/cmiss_region.h"
#include "general/message.h"

wxRegionChooser::wxRegionChooser(wxWindow *parent,
	cmzn_region *root_region, const char *initial_path) :
	wxChoice(parent, /*id*/-1, wxPoint(0,0), wxSize(-1,-1)),
	root_region(cmzn_region_access(root_region))
/*******************************************************************************
LAST MODIFIED : 22 February 2007

DESCRIPTION :
==============================================================================*/
{
	callback = NULL;
	build_main_menu(root_region, initial_path);

	Connect(wxEVT_COMMAND_CHOICE_SELECTED,
		wxCommandEventHandler(wxRegionChooser::OnChoiceSelected));

	cmzn_region_add_callback(root_region,
		wxRegionChooser::RegionChange, this);

	wxBoxSizer *sizer = new wxBoxSizer( wxHORIZONTAL );
	sizer->Add(this,
		wxSizerFlags(1).Align(wxALIGN_CENTER).Expand());
	parent->SetSizer(sizer);

	Show();

}

wxRegionChooser::~wxRegionChooser()
/*******************************************************************************
LAST MODIFIED : 22 February 2007

DESCRIPTION :
==============================================================================*/
{
	cmzn_region_remove_callback(root_region,
		wxRegionChooser::RegionChange, this);
	cmzn_region_destroy(&root_region);
	if (callback)
		delete callback;
}

char *wxRegionChooser::get_path()
/*******************************************************************************
LAST MODIFIED : 14 February 2007

DESCRIPTION :
Gets <path> of chosen region in the <chooser>.
Up to the calling function to DEALLOCATE the returned path.
==============================================================================*/
{
	char *return_name;
	wxString tmpstr = GetString(GetSelection());
	const char *item_name = tmpstr.mb_str(wxConvUTF8);
	// Trim the first character which is the root region symbol
	return_name = duplicate_string(item_name);
	return (return_name);
}

cmzn_region *wxRegionChooser::get_region()
/*******************************************************************************
LAST MODIFIED : 22 February 2007

DESCRIPTION :
Returns to <region_address> the region chosen in the <chooser>.
==============================================================================*/
{
	wxString tmpstr = GetString(GetSelection());
	cmzn_region_id child_region = cmzn_region_find_subregion_at_path(
		root_region, tmpstr.mb_str(wxConvUTF8));
	// don't want to access object here
	cmzn_region_id tmp = child_region;
	cmzn_region_destroy(&tmp);
	return (child_region);
}

int wxRegionChooser::set_region(struct cmzn_region *region)
{
	char *path = NULL;
	if (region)
	{
		char *temp_path = cmzn_region_get_path(region);
		int string_length = strlen(temp_path);
		if (strcmp(temp_path, "/") && (string_length > 1))
		{
			ALLOCATE(path, char, string_length);
			strncpy(path, temp_path, string_length);
			path[string_length - 1] = '\0';
			DEALLOCATE(temp_path);
		}
		else
		{
			path = temp_path;
		}
	}
	else
		path = duplicate_string("/");
	int return_code = set_path(path);
	DEALLOCATE(path);
	return return_code;
}

int wxRegionChooser::set_path(const char *path)
/*******************************************************************************
LAST MODIFIED : 14 February 2007

DESCRIPTION :
Sets <path> of chosen region in the <chooser>.
==============================================================================*/
{
	unsigned int found = 0, i;

	for (i = 0 ; !found && (i < GetCount()) ; i++)
	{
		wxString tmpstr = GetString(i);
		if (!strcmp(path, tmpstr.mb_str(wxConvUTF8)))
		{
			found = 1;
			SetSelection(i);
		}
	}
	if (!found)
	{
		display_message(ERROR_MESSAGE, "wxRegionChooser::set_path.  "
			"Child region not found for path: %s", path);
	}
	return (found);
}

int wxRegionChooser::append_children(cmzn_region *current_region,
	const char *current_path, const char *initial_path)
/*******************************************************************************
LAST MODIFIED : 22 February 2007

DESCRIPTION :
Sets <path> of chosen region in the <chooser>.
==============================================================================*/
{
	char *child_name, *child_path;

	cmzn_region *child_region = cmzn_region_get_first_child(current_region);
	while (NULL != child_region)
	{
		child_name = cmzn_region_get_name(child_region);

		ALLOCATE(child_path, char, strlen(current_path) + strlen(child_name) + 2);
		sprintf(child_path, "%s%c%s", current_path, CMZN_REGION_PATH_SEPARATOR_CHAR,
			child_name);
		Append(wxString::FromAscii(child_path));
		if (!strcmp(child_path, initial_path))
		{
			SetSelection(GetCount() - 1);
		}

		//Recurse
		append_children(child_region, child_path, initial_path);

		DEALLOCATE(child_name);
		DEALLOCATE(child_path);
		cmzn_region_reaccess_next_sibling(&child_region);
	}
	return 1;
}

int wxRegionChooser::build_main_menu(cmzn_region *root_region,
	const char *initial_path)
/*******************************************************************************
LAST MODIFIED : 22 February 2007

DESCRIPTION :
Updates the menu in the wxRegionChooser to reflect the heirarchy of the <root_region>.
If <initial_path> matches the pathname of one of the children then this object
is selected.
==============================================================================*/
{
	char *root_path;
	Clear();

	root_path = cmzn_region_get_root_region_path();
	Append(wxString::FromAscii(root_path));
	if (!strcmp(root_path, initial_path))
	{
		SetSelection(GetCount() - 1);
	}
	append_children(root_region, "", initial_path);
	DEALLOCATE(root_path);

	if (wxNOT_FOUND == GetSelection())
	{
		SetSelection(0);
		notify_callback();
	}

	return 1;
}

void wxRegionChooser::RegionChange(struct cmzn_region *root_region,
	cmzn_region_changes *region_changes, void *region_chooser_void)
/*******************************************************************************
LAST MODIFIED : 22 February 2007

DESCRIPTION :
Sets <path> of chosen region in the <chooser>.
==============================================================================*/
{
	wxRegionChooser *region_chooser;

	USE_PARAMETER(region_changes);
	region_chooser = static_cast<wxRegionChooser *>(region_chooser_void);
	if (region_chooser != NULL)
	{
		 char *temp;
		 temp = region_chooser->get_path();
		region_chooser->build_main_menu(root_region, temp);
		DEALLOCATE(temp);
	}
}

void wxRegionChooser::OnChoiceSelected(wxCommandEvent& event)
{
	USE_PARAMETER(event);
	notify_callback();
}

int wxRegionChooser::notify_callback()
{
	if (callback)
	{
		callback->callback_function(get_region());
	}
	return (1);
}
