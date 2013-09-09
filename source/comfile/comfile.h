/*******************************************************************************
FILE : comfile.h

LAST MODIFIED : 29 June 2002

DESCRIPTION :
Commands and functions for comfiles.
==============================================================================*/
/* OpenCMISS-Cmgui Application
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#if !defined (COMFILE_H)
#define COMFILE_H

#if 1
#include "configure/cmgui_configure.h"
#endif /* defined (1) */

#if defined (WX_USER_INTERFACE)
#include "comfile/comfile_window_wx.h"
#endif /* defined (WX_USER_INTERFACE) */
#include "command/parser.h"
#include "general/io_stream.h"
#include "user_interface/user_interface.h"

/*
Global types
------------
*/

struct Open_comfile_data
/*******************************************************************************
LAST MODIFIED : 18 April 2002

DESCRIPTION :
==============================================================================*/
{
	char example_flag,*examples_directory;
	const char *example_symbol,*file_extension,*file_name;
	int execute_count;
	struct Execute_command *execute_command,*set_command;
	struct IO_stream_package *io_stream_package;
#if defined (WX_USER_INTERFACE)
	struct MANAGER(Comfile_window) *comfile_window_manager;
#endif /* defined (WX_USER_INTERFACE) */
	struct User_interface *user_interface;
}; /* struct Open_comfile_data */

/*
Global functions
----------------
*/

int open_comfile(struct Parse_state *state,void *dummy_to_be_modified,
	void *open_comfile_data_void);
/*******************************************************************************
LAST MODIFIED : 29 June 2002

DESCRIPTION 
Opens a comfile, and a window if it is to be executed.  If a comfile is not
specified on the command line, a file selection box is presented to the user.
==============================================================================*/
#endif /* !defined (COMFILE_H) */
