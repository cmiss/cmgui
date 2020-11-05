/*******************************************************************************
FILE : cmiss.h

LAST MODIFIED : 16 September 2004

DESCRIPTION :
Functions and types for executing cmiss commands.

This should only be included in cmgui.c and command/cmiss.c
==============================================================================*/
/* OpenCMISS-Cmgui Application
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#if !defined (COMMAND_CMZN_H)
#define COMMAND_CMZN_H

#include "command/command.h"
#include "context/context.hpp"
#include "context/user_interface_module.h"
#if defined (BUILD_WITH_CMAKE)
#include "configure/cmgui_configure.h"
#endif /* defined (BUILD_WITH_CMAKE) */
#include "general/io_stream.h"
#include "general/manager.h"
#include "region/cmiss_region.hpp"
#if defined (WIN32_USER_INTERFACE)
//#define WINDOWS_LEAN_AND_MEAN
#if !defined (NOMINMAX)
#define NOMINMAX
#endif
#include <windows.h>
#endif /* defined (WIN32_USER_INTERFACE) */

/*
Global types
------------
*/

struct cmzn_command_data;
/*******************************************************************************
LAST MODIFIED : 12 August 2002

DESCRIPTION :
Shifted the cmzn_command_data to be internal to cmiss.c
==============================================================================*/

struct Cmgui_command_line_options
/*******************************************************************************
LAST MODIFIED : 19 May 2006

DESCRIPTION :
Command line options to be parsed by read_cmgui_command_line_options.
==============================================================================*/
{
	char batch_mode_flag;
	char cm_start_flag;
	char *cm_epath_directory_name;
	char *cm_parameters_file_name;
	char command_list_flag;
	char console_mode_flag;
	char *epath_directory_name;
	char *example_file_name;
	char *execute_string;
	char write_help_flag;
	char *id_name;
	char mycm_start_flag;
	char no_display_flag;
	char server_mode_flag;
	int random_number_seed;
	int visual_id_number;
	/* default option; no token */
	char *command_file_name;
};

/*
Global functions
----------------
*/

int cmiss_execute_command(const char *command_string,void *command_data);
/*******************************************************************************
LAST MODIFIED : 9 November 1998

DESCRIPTION :
Execute a <command_string>.
==============================================================================*/

#if defined (F90_INTERPRETER) || defined (USE_PERL_INTERPRETER)
void execute_command(const char *command_string,void *command_data_void, int *quit,
  int *error);
/*******************************************************************************
LAST MODIFIED : 28 March 2000

DESCRIPTION:
==============================================================================*/
#endif /* defined (F90_INTERPRETER) || defined (USE_PERL_INTERPRETER) */

int cmiss_set_command(const char *command_string,void *command_data_void);
/*******************************************************************************
LAST MODIFIED : 27 April 1999

DESCRIPTION:
Sets the <command_string> in the command box of the cmgui command_window, ready
for editing or entering. If there is no command_window, does nothing.
==============================================================================*/

struct cmzn_command_data *CREATE(cmzn_command_data)(struct cmzn_context_app *context,
	struct User_interface_module *UI_module);

/*******************************************************************************
LAST MODIFIED : 19 December 2002

DESCRIPTION :
Initialise all the subcomponents of cmgui and create the cmzn_command_data
==============================================================================*/

/*******************************************************************************
 * Returns a new reference to the command data with reference count incremented.
 * Caller is responsible for destroying the new reference.
 *
 * @param command_data  The command data to obtain a new reference to.
 * @return  New command data reference with incremented reference count.
 */
struct cmzn_command_data * cmzn_command_data_access(
	struct cmzn_command_data * command_data);

/***************************************************************************//**
 * Destroys reference to the command data and sets pointer/handle to NULL.
 * Internally this just decrements the reference count.
 *
 * @param command_data_address  Address of command data reference.
 * @return  1 on success, 0 if invalid arguments.
 */
int cmzn_command_data_destroy(
	struct cmzn_command_data **command_data_address);

int cmzn_command_data_main_loop(struct cmzn_command_data *command_data);
/*******************************************************************************
LAST MODIFIED : 19 December 2002

DESCRIPTION :
Process events until some events request the program to finish.
==============================================================================*/

struct cmzn_region *cmzn_command_data_get_root_region(
	struct cmzn_command_data *command_data);
/*******************************************************************************
LAST MODIFIED : 18 April 2003

DESCRIPTION :
Returns the root region from the <command_data>.
==============================================================================*/

struct Execute_command *cmzn_command_data_get_execute_command(
	struct cmzn_command_data *command_data);
/*******************************************************************************
LAST MODIFIED : 28 May 2003

DESCRIPTION :
Returns the execute command structure from the <command_data>, useful for
executing cmiss commands from C.
==============================================================================*/

struct IO_stream_package *cmzn_command_data_get_IO_stream_package(
	struct cmzn_command_data *command_data);
/*******************************************************************************
LAST MODIFIED : 16 September 2004

DESCRIPTION :
Returns the io_stream_package structure from the <command_data>
==============================================================================*/

struct cmzn_fdio_package* cmzn_command_data_get_fdio_package(
	struct cmzn_command_data *command_data);
/*******************************************************************************
LAST MODIFIED : 10 March 2005

DESCRIPTION :
Gets an Fdio_package for this <command_data>
==============================================================================*/

struct cmzn_idle_package* cmzn_command_data_get_idle_package(
	struct cmzn_command_data *command_data);
/*******************************************************************************
LAST MODIFIED : 21 March 2005

DESCRIPTION :
Gets an Idle_package for this <command_data>
==============================================================================*/

struct User_interface *cmzn_command_data_get_user_interface(
	struct cmzn_command_data *command_data);
/*******************************************************************************
LAST MODIFIED : 25 January 2006

DESCRIPTION :
Gets the user_interface for this <command_data>
==============================================================================*/

struct MANAGER(Graphics_window) *cmzn_command_data_get_graphics_window_manager(
	struct cmzn_command_data *command_data);
/*******************************************************************************
LAST MODIFIED : 26 January 2007

DESCRIPTION :
Returns the graphics_window manager from the <command_data>.
If this version of cmgui does not have graphics windows the returned pointer
will be NULL.
==============================================================================*/

/***************************************************************************//**
 * Set various strings which are important to the wxwidgets about box.
 * It will not do anything if wxwidget is not in use.
 *
 * @param command_data  Pointer to a cmzn_command_data object.
 * @param name_string  Given name of the current build.
 * @param version_string  Version of the current build.
 * @param date_string  Date of build of the current build.
 * @param copyright_string  Copyright statement of the current build.
 * @param build_string  Machine information of the current build.
 * @param revision_string  Subversion revision number of the current build.
 * @return  user_interface_module if successfully create the user interface,
 *    otherwise NULL.
 */
int cmzn_command_data_set_cmgui_string(struct cmzn_command_data *command_data,
	const char *name_string, const char *version_string,const char *date_string,
	const char *copyright_string, const char *build_string, const char *revision_string);

/***************************************************************************//**
 * Process command line options.
 *
 * @param argc  number of arguments.
 * @param argv  array of the value of arguments.
 * @param command_line_options  pointer to command line options object.
 * @return  1 if successfully called otherwise 0.
 */
int cmzn_command_data_process_command_line(int argc, char *argv[],
	struct Cmgui_command_line_options *command_line_options);

#endif /* !defined (COMMAND_CMZN_H) */
