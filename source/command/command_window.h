/*******************************************************************************
FILE : command_window.h

LAST MODIFIED : 27 April 1999

DESCRIPTION :
Definitions for command window structure, and associated functions
==============================================================================*/
#if !defined (COMMAND_WINDOW_H)
#define COMMAND_WINDOW_H

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
	struct Execute_command *execute_command,struct User_interface *user_interface,
	char *version_id_string);
/*******************************************************************************
LAST MODIFIED : 24 July 1998

DESCRIPTION :
==============================================================================*/

#if !defined (WINDOWS_DEV_FLAG)
int add_to_command_list(char *command,struct Command_window *command_window);
/*******************************************************************************
LAST MODIFIED : 16 June 1996

DESCRIPTION :
Adds the <command> to the bottom of the list for the <command_window>.
==============================================================================*/

int set_command_prompt(char *prompt,struct Command_window *command_window);
/*******************************************************************************
LAST MODIFIED : 16 June 1996

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
	char *command_string);
/*******************************************************************************
LAST MODIFIED : 27 April 1999

DESCRIPTION :
Fills the command entry area of the command window with <command_string>, ready
to be edited and entered. Used eg. by the comfile window to place a command in
responce to a single mouse click on it.
Does not override the command prompt.
==============================================================================*/

int write_command_window(char *message,struct Command_window *command_window);
/*******************************************************************************
LAST MODIFIED : 16 June 1996

DESCRIPTION :
Writes the <message> to the <command_window>.
==============================================================================*/
#endif /* !defined (WINDOWS_DEV_FLAG) */

int modify_Command_window(struct Parse_state *parse_state,void *dummy,
	void *command_window_void);
/*******************************************************************************
LAST MODIFIED : 9 November 1998

DESCRIPTION :
Modifys the <command_window_void> according to the command in the <parse_state>.
==============================================================================*/
#endif /* !defined (COMMAND_WINDOW_H) */
