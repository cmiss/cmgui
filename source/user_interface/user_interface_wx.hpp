/*******************************************************************************
FILE : user_interface.hpp

DESCRIPTION :
wxWidgets c++ definitions for the user interface.
==============================================================================*/
/* OpenCMISS-Cmgui Application
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#if !defined (USER_INTERFACE_WX_H)
#define USER_INTERFACE_WX_H

#if 1
#include "configure/cmgui_configure.h"
#endif /* defined (1) */

#if defined (WX_USER_INTERFACE)
# include <wx/wx.h>

struct Event_dispatcher;

class wxCmguiApp : public wxApp
{
	Event_dispatcher *event_dispatcher;

public:
	wxCmguiApp() 
		: wxApp()
		, event_dispatcher(0)
	{
	}

	virtual ~wxCmguiApp()
	{
	}

	virtual bool OnInit();

	virtual wxAppTraits * CreateTraits();

	void OnIdle(wxIdleEvent& event);

	void SetEventDispatcher(Event_dispatcher *event_dispatcher_in);

#if defined (__WXDEBUG__)
	void OnAssertFailure(const wxChar *file, int line, const wxChar* func, const wxChar* cond, const wxChar *msg)
	{
		USE_PARAMETER(file);
		USE_PARAMETER(line);
		USE_PARAMETER(func);
		USE_PARAMETER(cond);
		USE_PARAMETER(msg);
	}
#endif /* defined (__WXDEBUG__) */

	DECLARE_EVENT_TABLE();
};	

#endif /*defined (WX_USER_INTERFACE)*/

#endif /* !defined (USER_INTERFACE_WX_H) */
