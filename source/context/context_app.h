/***************************************************************************//**
 * context.cpp
 *
 * The main root structure of cmgui.
 */
/* OpenCMISS-Cmgui Application
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#if !defined (CONTEXT_APP_H)
#define CONTEXT_APP_H

#define Context cmzn_context
struct Context;

#include "zinc/context.h"
#include "command/cmiss.h"
#include "context/user_interface_module.h"

/**
 * Create the application context.
 * @param id
 * @return
 */
struct cmzn_context_app *cmzn_context_app_create(const char *id);

/**
 * Destroy the given handle to the application context.
 * @param context_address
 * @return
 */
int cmzn_context_app_destroy(struct cmzn_context_app **context_address);

/***************************************************************************//**
 * Return the default command data object in context.
 *
 * @param context  Pointer to a cmiss_context object.
 * @return  the default command data if successfully, otherwise NULL.
 */
struct cmzn_command_data *cmzn_context_get_default_command_interpreter(
	struct cmzn_context_app *context);

/***************************************************************************//**
 * Return the event dispatcher in context.
 *
 * @param context  Pointer to a cmiss_context object.
 * @return  the default event_dispatcher if successfully, otherwise NULL.
 */
struct Event_dispatcher *cmzn_context_app_get_default_event_dispatcher(
	struct cmzn_context_app *context);

/***************************************************************************//**
 * Create and returns the internal user interface module in cmgui.
 *
 * @param context  Pointer to a cmiss_context object.
 * @return  user_interface_module if successfully create the user interface,
 *    otherwise NULL.
 */
#if defined (WX_USER_INTERFACE) || (!defined (WIN32_USER_INTERFACE) && !defined (_MSC_VER))
struct User_interface_module *cmzn_context_create_user_interface(
	struct cmzn_context_app *context, int in_argc, char *in_argv[], void *user_interface_instance);
#else
struct User_interface_module *cmzn_context_create_user_interface(
	struct cmzn_context_app *context, int in_argc, char *in_argv[],
	HINSTANCE current_instance, HINSTANCE previous_instance,
	LPSTR command_line,int initial_main_window_state, void *user_interface_instance);
#endif

/***************************************************************************//**
 * Clean up any internally accessed context pointers
 */
void Context_internal_cleanup();

/**
 * Returns a the handle to the core context from the given application context.
 * @param context
 * @return
 */
struct Context *cmzn_context_app_get_core_context(struct cmzn_context_app *context);

#endif /* !defined (CONTEXT_H) */
