/*******************************************************************************
FILE : event_dispatcher.h

LAST MODIFIED : 5 March 2002

DESCRIPTION :
Routines for managing the main event loop in cmiss and dispatching events on
registered file descriptors to the correct callbacks.
==============================================================================*/
#if !defined (EVENT_DISPATCHER_H)
#define EVENT_DISPATCHER_H

#include "general/object.h"
#if defined (USE_XTAPP_CONTEXT) /* switch (USER_INTERFACE) */
#include <Xm/Xm.h>
#elif defined (WIN32_USER_INTERFACE) /* switch (USER_INTERFACE) */
#elif defined (USE_GTK_MAIN_STEP) /* switch (USER_INTERFACE) */
#elif 1 /* switch (USER_INTERFACE) */
/* This is the default code, it is an event dispatcher designed to run 
	without any particular user interface, I define a preprocess value here to
	make it easy to switch through the code */
#include <general/time.h>
#define USE_GENERIC_EVENT_DISPATCHER
#endif /* switch (USER_INTERFACE) */

/*
Global types
------------
*/

struct Event_dispatcher;

struct Event_dispatcher_descriptor_callback;

struct Event_dispatcher_timeout_callback;

struct Event_dispatcher_idle_callback;

#if defined (USE_GENERIC_EVENT_DISPATCHER)
struct Event_dispatcher_descriptor_set
/*******************************************************************************
LAST MODIFIED : 12 November 2002

DESCRIPTION :

==============================================================================*/
{
	fd_set read_set;
	fd_set write_set;
	fd_set error_set;
	int max_timeout_ns;
};
#endif /* defined (USE_GENERIC_EVENT_DISPATCHER) */

typedef int Event_dispatcher_simple_descriptor_callback_function(
	int file_descriptor, void *user_data);
/*******************************************************************************
LAST MODIFIED : 13 November 2002

DESCRIPTION :

==============================================================================*/

#if defined (USE_GENERIC_EVENT_DISPATCHER)
typedef int Event_dispatcher_descriptor_query_function(
	struct Event_dispatcher_descriptor_set *descriptor_set, void *user_data);
/*******************************************************************************
LAST MODIFIED : 12 November 2002

DESCRIPTION :
If <timeout> is set to be non zero then it is included as a maximum sleep time.
==============================================================================*/
#endif /* defined (USE_GENERIC_EVENT_DISPATCHER) */

#if defined (USE_GENERIC_EVENT_DISPATCHER)
typedef int Event_dispatcher_descriptor_check_function(
	struct Event_dispatcher_descriptor_set *descriptor_set,
	void *user_data);
/*******************************************************************************
LAST MODIFIED : 12 November 2002

DESCRIPTION :
==============================================================================*/
#endif /* defined (USE_GENERIC_EVENT_DISPATCHER) */

#if defined (USE_GENERIC_EVENT_DISPATCHER)
typedef int Event_dispatcher_descriptor_dispatch_function(void *user_data);
/*******************************************************************************
LAST MODIFIED : 12 November 2002

DESCRIPTION :
==============================================================================*/
#endif /* defined (USE_GENERIC_EVENT_DISPATCHER) */

typedef int Event_dispatcher_idle_function(void *user_data);
/*******************************************************************************
LAST MODIFIED : 5 March 2002

DESCRIPTION :
==============================================================================*/

enum Event_dispatcher_idle_priority
/*******************************************************************************
LAST MODIFIED : 5 March 2002

DESCRIPTION :
==============================================================================*/
{
	EVENT_DISPATCHER_X_PRIORITY,
	EVENT_DISPATCHER_TRACKING_EDITOR_PRIORITY,
	EVENT_DISPATCHER_IDLE_UPDATE_SCENE_VIEWER_PRIORITY,
	EVENT_DISPATCHER_SYNC_SCENE_VIEWERS_PRIORITY,
	EVENT_DISPATCHER_TUMBLE_SCENE_VIEWER_PRIORITY
};

typedef int Event_dispatcher_timeout_function(void *user_data);
/*******************************************************************************
LAST MODIFIED : 5 March 2002

DESCRIPTION :
==============================================================================*/

/*
Global functions
----------------
*/

struct Event_dispatcher *CREATE(Event_dispatcher)(void);
/*******************************************************************************
LAST MODIFIED : 5 March 2002

DESCRIPTION :
==============================================================================*/

int DESTROY(Event_dispatcher)(struct Event_dispatcher **event_dispatcher);
/*******************************************************************************
LAST MODIFIED : 5 March 2002

DESCRIPTION :
Destroys an Event_dispatcher object
==============================================================================*/

struct Event_dispatcher_descriptor_callback *Event_dispatcher_add_simple_descriptor_callback(
	struct Event_dispatcher *event_dispatcher, int file_descriptor,
	Event_dispatcher_simple_descriptor_callback_function *callback_function,
	void *user_data); 
/*******************************************************************************
LAST MODIFIED : 13 November 2002

DESCRIPTION :
A simple descriptor callback used when only a single read <file_descriptor> is 
required to be tested for the callback.
==============================================================================*/

#if defined (USE_GENERIC_EVENT_DISPATCHER)
struct Event_dispatcher_descriptor_callback *Event_dispatcher_add_descriptor_callback(
	struct Event_dispatcher *event_dispatcher,
	Event_dispatcher_descriptor_query_function *query_function,
	Event_dispatcher_descriptor_check_function *check_function,
	Event_dispatcher_descriptor_dispatch_function *dispatch_function,
	void *user_data); 
/*******************************************************************************
LAST MODIFIED : 13 November 2002

DESCRIPTION :
This function is only implemented when using the GENERIC_EVENT_DISPATCHER.
When using Xt or Gtk as the main loop only 
Event_dispatcher_add_simple_descriptor_callback is fully implemented.
In these cases callbacks added using this function will just be ignored.
==============================================================================*/
#endif /* defined (USE_GENERIC_EVENT_DISPATCHER) */

int Event_dispatcher_remove_descriptor_callback(
	struct Event_dispatcher *event_dispatcher, 
	struct Event_dispatcher_descriptor_callback *callback_id);
/*******************************************************************************
LAST MODIFIED : 13 November 2002

DESCRIPTION :
Remove the <callback_id> created by either Event_dispatcher_add_descriptor_callback
or Event_dispatcher_add_simple_descriptor_callback from the <event_dispatcher>.
==============================================================================*/

struct Event_dispatcher_timeout_callback *Event_dispatcher_add_timeout_callback_at_time(
	struct Event_dispatcher *event_dispatcher, unsigned long timeout_s, unsigned long timeout_ns,
	Event_dispatcher_timeout_function *timeout_function, void *user_data);
/*******************************************************************************
LAST MODIFIED : 5 March 2002

DESCRIPTION :
==============================================================================*/

struct Event_dispatcher_timeout_callback *Event_dispatcher_add_timeout_callback(
	struct Event_dispatcher *event_dispatcher, unsigned long timeout_s, unsigned long timeout_ns,
	Event_dispatcher_timeout_function *timeout_function, void *user_data);
/*******************************************************************************
LAST MODIFIED : 5 March 2002

DESCRIPTION :
==============================================================================*/

int Event_dispatcher_remove_timeout_callback(
	struct Event_dispatcher *event_dispatcher, 
	struct Event_dispatcher_timeout_callback *callback_id);
/*******************************************************************************
LAST MODIFIED : 5 March 2002

DESCRIPTION :
==============================================================================*/

struct Event_dispatcher_idle_callback *Event_dispatcher_add_idle_callback(
	struct Event_dispatcher *event_dispatcher, 
	Event_dispatcher_idle_function *idle_function, void *user_data,
	enum Event_dispatcher_idle_priority priority);
/*******************************************************************************
LAST MODIFIED : 5 March 2002

DESCRIPTION :
==============================================================================*/

int Event_dispatcher_remove_idle_callback(
	struct Event_dispatcher *event_dispatcher, 
	struct Event_dispatcher_idle_callback *callback_id);
/*******************************************************************************
LAST MODIFIED : 5 March 2002

DESCRIPTION :
==============================================================================*/

struct Event_dispatcher_idle_callback *Event_dispatcher_set_special_idle_callback(
	struct Event_dispatcher *event_dispatcher, 
	Event_dispatcher_idle_function *idle_function, void *user_data,
	enum Event_dispatcher_idle_priority priority);
/*******************************************************************************
LAST MODIFIED : 12 March 2002

DESCRIPTION :
==============================================================================*/

int Event_dispatcher_do_one_event(struct Event_dispatcher *event_dispatcher); 
/*******************************************************************************
LAST MODIFIED : 5 March 2002

DESCRIPTION :
==============================================================================*/

int Event_dispatcher_main_loop(struct Event_dispatcher *event_dispatcher); 
/*******************************************************************************
LAST MODIFIED : 5 March 2002

DESCRIPTION :
==============================================================================*/

int Event_dispatcher_end_main_loop(struct Event_dispatcher *event_dispatcher);
/*******************************************************************************
LAST MODIFIED : 4 March 2002

DESCRIPTION :
==============================================================================*/

#if defined (USE_XTAPP_CONTEXT)
int Event_dispatcher_set_application_context(struct Event_dispatcher *event_dispatcher,
	XtAppContext application_context);
/*******************************************************************************
LAST MODIFIED : 4 June 2002

DESCRIPTION :
==============================================================================*/
#endif /* defined (USE_XTAPP_CONTEXT) */
#endif /* !defined (EVENT_DISPATCHER_H) */
