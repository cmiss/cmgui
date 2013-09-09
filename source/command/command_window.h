/*******************************************************************************
FILE : command_window.h

LAST MODIFIED : 27 June 2002

DESCRIPTION :
Definitions for command window structure, and associated functions
==============================================================================*/
/* OpenCMISS-Cmgui Application
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#if !defined (COMMAND_WINDOW_H)
#define COMMAND_WINDOW_H

#if 1
#include "configure/cmgui_configure.h"
#endif /* defined (1) */

#include "command/command.h"
#include "general/object.h"
#include "user_interface/user_interface.h"

/*
Global constants
----------------
*/
/* for allowing other applications to communicate with cmgui (issue commands) */
#define CMGUI_VERSION_PROPERTY "_CMGUI_VERSION"
#define CMGUI_LOCK_PROPERTY "_CMGUI_LOCK"
#define CMGUI_COMMAND_PROPERTY "_CMGUI_COMMAND"
#define CMGUI_RESPONSE_PROPERTY "_CMGUI_RESPONSE"

#if defined (WIN32_USER_INTERFACE) || defined (GTK_USER_INTERFACE) || defined (WX_USER_INTERFACE)
#define USE_CMGUI_COMMAND_WINDOW
#endif /* defined (WIN32_USER_INTERFACE) || defined (GTK_USER_INTERFACE) || defined (WX_USER_INTERFACE) */

/*
Global types
------------
*/
struct Command_window;

/*
Global functions
----------------
*/
struct Command_window *CREATE(Command_window)(
	struct Execute_command *execute_command,struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 24 July 1998

DESCRIPTION :
==============================================================================*/

int DESTROY(Command_window)(struct Command_window **command_window_pointer);
/*******************************************************************************
LAST MODIFIED : 28 February 2000

DESCRIPTION:
==============================================================================*/

int add_to_command_list(const char *command,struct Command_window *command_window);
/*******************************************************************************
LAST MODIFIED : 16 June 1996

DESCRIPTION :
Adds the <command> to the bottom of the list for the <command_window>.
==============================================================================*/

int Command_window_set_command_prompt(struct Command_window *command_window,
	const char *prompt);
/*******************************************************************************
LAST MODIFIED : 27 June 2002

DESCRIPTION :
Sets the value of the <prompt> for the <command_window>.
==============================================================================*/

int reset_command_box(struct Command_window *command_window);
/*******************************************************************************
LAST MODIFIED : 16 June 1996

DESCRIPTION :
Resets all functions of the command box widget.
==============================================================================*/

int Command_window_set_command_string(struct Command_window *command_window,
	const char *command_string);
/*******************************************************************************
LAST MODIFIED : 27 April 1999

DESCRIPTION :
Fills the command entry area of the command window with <command_string>, ready
to be edited and entered. Used eg. by the comfile window to place a command in
responce to a single mouse click on it.
Does not override the command prompt.
==============================================================================*/

int write_command_window(const char *message,struct Command_window *command_window);
/*******************************************************************************
LAST MODIFIED : 16 June 1996

DESCRIPTION :
Writes the <message> to the <command_window>.
==============================================================================*/

int modify_Command_window(struct Parse_state *parse_state,void *dummy,
	void *command_window_void);
/*******************************************************************************
LAST MODIFIED : 9 November 1998

DESCRIPTION :
Modifys the <command_window_void> according to the command in the <parse_state>.
==============================================================================*/

int Command_window_set_cmgui_string(struct Command_window *command_window,
	const char *name_string, const char *version_string,const char *date_string,
	const char *copyright_string, const char *build_string, const char *revision_string);
#endif /* !defined (COMMAND_WINDOW_H) */
