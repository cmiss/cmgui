/*******************************************************************************
FILE : cmiss_region_chooser_wx.hpp

LAST MODIFIED : 21 February 2007

DESCRIPTION :
==============================================================================*/
/* OpenCMISS-Cmgui Application
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#if !defined (CMZN_REGION_CHOOSER_WX_HPP)
#define CMZN_REGION_CHOOSER_WX_HPP

#include "wx/wx.h"
#include "general/callback_class.hpp"

struct cmzn_region;
class cmzn_region_changes;

class wxRegionChooser : public wxChoice
{
private:
	cmzn_region *root_region;
	Callback_base<cmzn_region*> *callback;

public:
	wxRegionChooser(wxWindow *parent, 
		cmzn_region *root_region, const char *initial_path);
/*******************************************************************************
LAST MODIFIED : 14 February 2007

DESCRIPTION :
==============================================================================*/

	~wxRegionChooser();
/*******************************************************************************
LAST MODIFIED : 9 January 2003

DESCRIPTION :
==============================================================================*/

	int set_callback(Callback_base<cmzn_region*> *callback_object)
	{
		if (callback)
			delete callback;
		callback = callback_object;
		return (1);
	}

	char *get_path();
/*******************************************************************************
LAST MODIFIED : 14 February 2007

DESCRIPTION :
Gets <path> of chosen region in the <chooser>.
Up to the calling function to DEALLOCATE the returned path.
==============================================================================*/

	cmzn_region *get_region();
/*******************************************************************************
LAST MODIFIED : 22 February 2007

DESCRIPTION :
Returns to <region_address> the region chosen in the <chooser>.
==============================================================================*/

	int set_region(struct cmzn_region *region);
/*******************************************************************************
LAST MODIFIED : 22 February 2007

DESCRIPTION :
Returns to <region_address> the region chosen in the <chooser>.
==============================================================================*/

	int set_path(const char *path);
/*******************************************************************************
LAST MODIFIED : 14 February 2007

DESCRIPTION :
Sets <path> of chosen region in the <chooser>.
==============================================================================*/

private:
	void OnChoiceSelected(wxCommandEvent& Event);
	
	int notify_callback();

	int build_main_menu(cmzn_region *root_region,
		const char *initial_path);
	int append_children(cmzn_region *current_region,
		const char *current_path, const char *initial_path);

	static void RegionChange(struct cmzn_region *root_region,
		cmzn_region_changes *region_changes, void *region_chooser_void);
};
#endif /* !defined (CMZN_REGION_CHOOSER_H) */
