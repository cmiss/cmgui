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

#include "time/time_keeper_app.hpp"
#include "opencmiss/zinc/fieldgroup.h"
#include "command/cmiss.h"
#include "context/context.hpp"
#include "context/context_app.h"
#include "general/debug.h"
#include "general/mystring.h"
#include "general/object.h"
#include "graphics/graphics_module.hpp"
#include "graphics/scene.hpp"
#include "region/cmiss_region.hpp"
#include <set>

struct cmzn_context_app *cmzn_context_app_access(cmzn_context_app *context);
int cmzn_context_app_destroy(struct cmzn_context_app **context_address);

struct cmzn_context_app
{
	int access_count;
	struct Context *context;
	struct cmzn_command_data  *default_command_data;
	struct User_interface_module *UI_module;
	struct Event_dispatcher *event_dispatcher;
};

struct Context *cmzn_context_app_get_core_context(struct cmzn_context_app *context)
{
	return context->context;
}

class Context_holder
{
private:
	std::set<cmzn_context_app *> contextsList;

	Context_holder()
	{
	}

	~Context_holder()
	{
		destroy();
	}

public:

	static Context_holder *getInstance()
	{
		static Context_holder contextHolder;
		return &contextHolder;
	}

	void destroy()
	{
		std::set<cmzn_context_app *>::iterator pos = contextsList.begin();
		while (pos != contextsList.end())
		{
			cmzn_context_app * temp_pointer = *pos;
			cmzn_context_app_destroy(&temp_pointer);
			++pos;
		}
		contextsList.clear();
	}

	void insert(cmzn_context_app * context_in)
	{
		if (cmzn_context_app_access(context_in))
		{
			contextsList.insert(context_in);
		}
	}

	int hasEntry(cmzn_context_app * context_in)
	{
		if (contextsList.find(context_in) != contextsList.end())
		{
			return true;
		}
		return false;
	}

};

struct cmzn_context_app *cmzn_context_app_create(const char *id)
{
	struct cmzn_context_app *context = NULL;
	if (ALLOCATE(context, struct cmzn_context_app, 1))
	{
		context->access_count = 1;
		context->context = cmzn_context_create(id);
		context->default_command_data = NULL;
		context->UI_module = NULL;
		context->event_dispatcher = NULL;
		Context_holder::getInstance()->insert(context);
	}

	return context;
}

int cmzn_context_app_destroy(struct cmzn_context_app **context_address)
{
	int return_code = 0;
	struct cmzn_context_app *context = NULL;

	if (context_address && NULL != (context = *context_address))
	{
		context->access_count--;

		if (0 == context->access_count)
		{
			if (context->default_command_data)
				cmzn_command_data_destroy(&context->default_command_data);
			if (context->UI_module)
				User_interface_module_destroy(&context->UI_module);
			if (context->event_dispatcher)
			{
				DESTROY(Event_dispatcher)(&context->event_dispatcher);
			}
			if (context->context)
			{
				cmzn_context_destroy(&context->context);
			}
			DEALLOCATE(*context_address);
		}
		*context_address = NULL;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmzn_context_destroy.  Missing context address");
		return_code = 0;
	}

	return return_code;
}

struct cmzn_context_app *cmzn_context_app_access(struct cmzn_context_app *context)
{
	if (context)
	{
		context->access_count++;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmzn_context_access.  Missing context");
	}
	return context;
}

struct cmzn_command_data *cmzn_context_get_default_command_interpreter(struct cmzn_context_app *context)
{
	struct cmzn_command_data *command_data = NULL;

	if (context && context->UI_module)
	{
		if (!context->default_command_data)
		{
			context->default_command_data = CREATE(cmzn_command_data)(context, context->UI_module);
		}
		if (context->default_command_data)
		{
			command_data = cmzn_command_data_access(context->default_command_data);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmzn_context_get_default_command_interpreter.  Missing context "
			"or user interface has not been enable yet.");
	}

	return command_data;
}
#if defined (WX_USER_INTERFACE) || (!defined (WIN32_USER_INTERFACE) && !defined (_MSC_VER))
struct User_interface_module *cmzn_context_create_user_interface(
	struct cmzn_context_app *context, int in_argc, char *in_argv[],
	void *user_interface_instance)
#else
struct User_interface_module *cmzn_context_create_user_interface(
	struct Context *context, int in_argc, char *in_argv[],
	HINSTANCE current_instance, HINSTANCE previous_instance,
	LPSTR command_line,int initial_main_window_state,
	void *user_interface_instance)
#endif
{
	USE_PARAMETER(user_interface_instance);
	struct User_interface_module *UI_module = NULL;

	if (context)
	{
		if (!context->UI_module)
		{
#if defined (WX_USER_INTERFACE)
			if (user_interface_instance)
				Event_dispatcher_set_wx_instance(cmzn_context_app_get_default_event_dispatcher(context), user_interface_instance);
#endif
#if defined (WX_USER_INTERFACE) || (!defined (WIN32_USER_INTERFACE) && !defined (_MSC_VER))
			context->UI_module = User_interface_module_create(
				context, in_argc, in_argv);
#else
			context->UI_module = User_interface_module_create(
				context, in_argc, in_argv, current_instance,
				previous_instance, command_line, initial_main_window_state);
#endif
		}
		if (context->UI_module)
		{
			UI_module = User_interface_module_access(context->UI_module);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmzn_context_create_user_interface.  Missing context.");
	}

	return UI_module;
}

struct Event_dispatcher *cmzn_context_app_get_default_event_dispatcher(struct cmzn_context_app *context)
{
	struct Event_dispatcher *event_dispatcher = NULL;
	if (context)
	{
		if (!context->event_dispatcher)
		{
			context->event_dispatcher = CREATE(Event_dispatcher)();
		}
		event_dispatcher = context->event_dispatcher;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmzn_context_app_get_default_event_dispatcher.  Missing context.");
	}
	return event_dispatcher;
}

int cmzn_context_app_execute_command(cmzn_context_app *context,
	const char *command)
{
	int return_code = 0;
	if (context && context->UI_module && command)
	{
		struct cmzn_command_data *command_data =
			cmzn_context_get_default_command_interpreter(context);
		return_code = cmiss_execute_command(command, (void *)command_data);
		cmzn_command_data_destroy(&command_data);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmzn_context_execute_command.  Missing context or user interface or"
			"command is empty.");
	}

	return return_code;
}

int cmzn_context_run_main_loop(cmzn_context_app *context)
{
	int return_code = 0;
	if (context && context->UI_module)
	{
		struct cmzn_command_data *command_data =
			cmzn_context_get_default_command_interpreter(context);
		return_code = cmzn_command_data_main_loop(command_data);
		cmzn_command_data_destroy(&command_data);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmzn_context_start_main_loop.  Missing context or user interface");
	}

	return return_code;
}

//-- cmzn_sceneviewermodule_id cmzn_context_app_get_default_sceneviewermodule(
//-- 	cmzn_context_app *context)
//-- {
//-- 	cmzn_sceneviewermodule *sceneviewermodule = NULL;
//-- 	if (context && context->UI_module && context->UI_module->sceneviewermodule)
//-- 	{
//-- 		sceneviewermodule = context->UI_module->sceneviewermodule;
//-- 	}
//-- 	else
//-- 	{
//-- 		display_message(ERROR_MESSAGE,
//-- 			"cmzn_context_get_default_sceneviewermodule.  "
//-- 			"Missing context or user interface");
//-- 	}
//-- 	return sceneviewermodule;
//-- }

#if defined (WX_USER_INTERFACE) || (!defined (WIN32_USER_INTERFACE) && !defined (_MSC_VER))
int cmzn_context_app_enable_user_interface(cmzn_context_app *context, void *user_interface_instance)
#else
int cmzn_context_enable_user_interface(
	cmzn_context_id context, HINSTANCE current_instance, HINSTANCE previous_instance,
	LPSTR command_line,int initial_main_window_state, void *user_interface_instance)
#endif
{
	int return_code = 0;
#if defined (WX_USER_INTERFACE) || (!defined (WIN32_USER_INTERFACE) && !defined (_MSC_VER))
	struct User_interface_module *UI_module = cmzn_context_create_user_interface(
		context, 0, 0, user_interface_instance);
#else
	struct User_interface_module *UI_module=  cmzn_context_create_user_interface(
		context, 0, 0, current_instance, previous_instance,
		command_line, initial_main_window_state, user_interface_instance);
#endif
	if (UI_module)
	{
		UI_module->external = 1;
		return_code = 1;
		User_interface_module_destroy(&UI_module);
	}

	return return_code;
}

int cmzn_context_app_process_idle_event(cmzn_context_app *context)
{
	if (context && context->event_dispatcher)
	{
		return Event_dispatcher_process_idle_event(context->event_dispatcher);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmzn_context_do_idle_event.  Missing context or event dispatcher.");
	}

	return 0;
}

void Context_internal_cleanup()
{
	Context_holder::getInstance()->destroy();

	/* Write out any memory blocks still ALLOCATED when MEMORY_CHECKING is
		on.  When MEMORY_CHECKING is off this function does nothing but only list
		when context holder does not have an entry of this context */
	list_memory(/*count_number*/0, /*show_pointers*/0, /*increment_counter*/0,
		/*show_structures*/1);
}
