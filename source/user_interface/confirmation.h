/*******************************************************************************
FILE : confirmation.h

LAST MODIFIED : 7 July 1999

DESCRIPTION :
Routines for waiting for user input.
==============================================================================*/
/* OpenCMISS-Cmgui Application
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#if !defined (CONFIRMATION_H)
#define CONFIRMATION_H

#if 1
#include "configure/cmgui_configure.h"
#endif /* defined (1) */

#include "user_interface/user_interface.h"

/*
Global types
------------
*/

/*
Global functions
----------------
*/
int confirmation_warning_ok_cancel(const char *title,const char *prompt,
	struct User_interface *user_interface
#if  defined (WX_USER_INTERFACE) 
	 , struct Execute_command *execute_command
#endif /* defined (WX_USER_INTERFACE) */
																	 );
/*******************************************************************************
LAST MODIFIED : 11 June 1999

DESCRIPTION :
This routine supplies a dialog window which requires a response
before anything else will continue and returns 1 if the OK button
is clicked and 0 if the cancel button is clicked.
==============================================================================*/

int confirmation_error_ok(char *title,char *prompt,
	struct User_interface *user_interface
#if  defined (WX_USER_INTERFACE) 
	 , struct Execute_command *execute_command
#endif /* defined (WX_USER_INTERFACE) */
													);
/*******************************************************************************
LAST MODIFIED : 11 June 1999

DESCRIPTION :
This routine supplies a error dialog window which requires a response before
anything else will continue and returns as the OK button is clicked.  No other
options are supplied.
==============================================================================*/

int confirmation_information_ok(char *title,char *prompt,
	struct User_interface *user_interface
#if  defined (WX_USER_INTERFACE) 
	 , struct Execute_command *execute_command
#endif /* defined (WX_USER_INTERFACE) */
																);
/*******************************************************************************
LAST MODIFIED : 11 June 1999

DESCRIPTION :
This routine supplies a information dialog window which requires a response
before anything else will continue and returns as the OK button is clicked.  No
other options are supplied.
==============================================================================*/

int confirmation_warning_ok(const char *title,const char *prompt,
	struct User_interface *user_interface
#if  defined (WX_USER_INTERFACE) 
	 , struct Execute_command *execute_command
#endif /* defined (WX_USER_INTERFACE) */
														);
/*******************************************************************************
LAST MODIFIED : 11 June 1999

DESCRIPTION :
This routine supplies a warning dialog window which requires a response before
anything else will continue and returns as the OK button is clicked.  No other
options are supplied.
==============================================================================*/

int confirmation_question_yes_no(char *title,char *prompt,
	struct User_interface *user_interface
#if  defined (WX_USER_INTERFACE) 
	 , struct Execute_command *execute_command
#endif /* defined (WX_USER_INTERFACE) */
																 );
/*******************************************************************************
LAST MODIFIED : 11 June 1999

DESCRIPTION :
This routine supplies a dialog window which requires a response
before anything else will continue.  It returns one if the Yes button
is clicked and No if it isn't.
==============================================================================*/

char *confirmation_get_read_filename(const char *extension,
	struct User_interface *user_interface
#if  defined (WX_USER_INTERFACE) 
	 , struct Execute_command *execute_command
#endif /* defined (WX_USER_INTERFACE) */
																		 );
/*******************************************************************************
LAST MODIFIED : 11 June 1999

DESCRIPTION :
This routine supplies a file selection dialog window
==============================================================================*/

char *confirmation_get_write_filename(const char *extension,
	struct User_interface *user_interface
#if  defined (WX_USER_INTERFACE) 
	 , struct Execute_command *execute_command
#endif /* defined (WX_USER_INTERFACE) */
																			);
/*******************************************************************************
LAST MODIFIED : 11 June 1999

DESCRIPTION :
This routine supplies a file selection dialog window
==============================================================================*/

char *confirmation_change_current_working_directory(
	struct User_interface *user_interface
#if  defined (WX_USER_INTERFACE) 
	 , struct Execute_command *execute_command
#endif /* defined (WX_USER_INTERFACE) */
	);
/*******************************************************************************
LAST MODIFIED : 7 July 1999

DESCRIPTION :
This routine supplies a file selection dialog window for changing the current
working directory.  The new directory will be created if necessary.
==============================================================================*/

#endif /* !defined (CONFIRMATION_H) */
