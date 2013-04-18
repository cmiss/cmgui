/*******************************************************************************
FILE : time_keeper_app.cpp

LAST MODIFIED : 21 January 2003

DESCRIPTION :
This object defines a relationship between a bunch of time objects, keeps them
in sync and allows control such as play, rewind and fast forward.
This is intended to be multithreaded......
==============================================================================*/
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is cmgui.
 *
 * The Initial Developer of the Original Code is
 * Auckland Uniservices Ltd, Auckland, New Zealand.
 * Portions created by the Initial Developer are Copyright (C) 2005
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */
#include <math.h>
#include <stdio.h>

#include "zinc/timekeeper.h"
#include "general/debug.h"
#include "general/list_private.h"
#include "general/mystring.h"
#include "general/object.h"
#include "general/time.h"
#include "general/message.h"
#include "time/time.h"
#include "time/time_keeper.hpp"
#include "time/time_keeper_app.hpp"
#include "time/time_private.h"
#include "user_interface/event_dispatcher.h"
#include "general/enumerator_conversion.hpp"

struct Time_keeper_app_callback_data
{
	Time_keeper_app_callback callback;
	void *callback_user_data;
	enum Time_keeper_app_event event_mask;
	struct Time_keeper_app_callback_data *next;
};

int Time_keeper_app_timer_event_handler(void *time_keeper_app_void)
{
	Time_keeper_app *time_keeper_app = 0;
	if (NULL != (time_keeper_app = (struct Time_keeper_app *)time_keeper_app_void))
	{
		return time_keeper_app->timerEvent();
	}

	return 0;
}

Time_keeper_app::Time_keeper_app(Cmiss_time_keeper *time_keeper_in,
	struct Event_dispatcher *event_dispatcher):
	play_mode(TIME_KEEPER_APP_PLAY_LOOP),
	play_remaining(0.0),
	speed(1.0),
	step(0.0),
	play_direction(TIME_KEEPER_PLAY_FORWARD),
	play_every_frame(0),
	play_start_seconds(0),
	play_start_microseconds(0),
	real_time(0),
	playing(0),
	timeout_callback_id(0),
	event_dispatcher(event_dispatcher),
	callback_list(0),
	time_keeper(Cmiss_time_keeper_access(time_keeper_in)),
	access_count(1)
{
}

Time_keeper_app::~Time_keeper_app()
{
	struct Time_keeper_app_callback_data *next_callback = NULL,
		*callback_data = callback_list;
	while(callback_data)
	{
		next_callback = callback_data->next;
		DEALLOCATE(callback_data);
		callback_data = next_callback;
	}
	Cmiss_time_keeper_destroy(&time_keeper);
}

int Time_keeper_app::notifyClients(enum Time_keeper_app_event event_mask)
{
	int return_code = 1;
	struct Time_keeper_app_callback_data *callback_data = callback_list;
	while(callback_data)
	{
		if(callback_data->event_mask & event_mask)
		{
			return_code = (callback_data->callback)(this,
				event_mask, callback_data->callback_user_data);
		}
		callback_data = callback_data->next;
	}
	return return_code;
}

int Time_keeper_app::play(enum Time_keeper_play_direction play_direction_in)
{
	int return_code;

	return_code = 1;
	play_remaining = 0;
	if(!isPlaying())
	{
		play_direction = play_direction_in;
		/*notify clients before playing. If play fails, notify of stop*/
		notifyClients(TIME_KEEPER_APP_STARTED);
		if(playPrivate())
		{
			playing = 1;
		}
		else
		{
			playing = 0;
			notifyClients(TIME_KEEPER_APP_STOPPED);
		}
	}
	else
	{
		if(play_direction_in != play_direction)
		{
			stopPrivate();
			play_direction = play_direction_in;
			playPrivate();
			notifyClients(TIME_KEEPER_APP_CHANGED_DIRECTION);
		}
	}

	return (return_code);
} /* Time_keeper_play */

int Time_keeper_app::isPlaying()
{
	return playing;
}

void Time_keeper_app::stopPrivate()
{
	if(timeout_callback_id)
	{
		Event_dispatcher_remove_timeout_callback(event_dispatcher,
			timeout_callback_id);
		timeout_callback_id = (struct Event_dispatcher_timeout_callback *)NULL;
	}
} /* Time_keeper_stop_private */

void Time_keeper_app::stop()
{
	if(isPlaying())
	{
		stopPrivate();
		playing = 0;
		play_remaining = 0;
		notifyClients(TIME_KEEPER_APP_STOPPED);
	}
}

double Time_keeper_app::getSpeed()
{
	return speed;
} /* Time_keeper_get_speed */

void Time_keeper_app::setSpeed(double speed_in)
{
	speed = speed_in;
} /* Time_keeper_set_speed */


int Time_keeper_app::playPrivate()
{
	int return_code = 0, looping = 0;
	struct Time_object_info *object_info;
	struct timeval timeofday;
	double current_time = time_keeper->getTime();
	double minimum = time_keeper->getMinimum(),
		maximum = time_keeper->getMaximum();

	if(!timeout_callback_id)
	{
		switch(play_direction)
		{
		case TIME_KEEPER_PLAY_FORWARD:
		{
			if(current_time < minimum)
			{
				current_time = minimum;
			}
			if (current_time  >= maximum)
			{
				current_time = minimum;
				looping =1;
			}
		} break;
		case TIME_KEEPER_PLAY_BACKWARD:
		{
			if(current_time > maximum)
			{
				current_time = maximum;
			}
			if(current_time <= minimum)
			{
				current_time = maximum;
				looping =1;
			}
		} break;
		}
		gettimeofday(&timeofday, (struct timezone *)NULL);
		play_start_seconds = timeofday.tv_sec;
		play_start_microseconds = timeofday.tv_usec;
		real_time = current_time;
		time_keeper->setTimeQuiet(current_time);
		struct Time_object_info *object_info = time_keeper->getObjectInfo();
		while(object_info)
		{
			Time_object_set_current_time_privileged(object_info->time_object ,current_time);
			/* if loop then check the current time is a valid callback time of time object,
				 if it is a valid callback time then notify clients for the change */
			if (looping && Time_object_check_valid_callback_time(
				object_info->time_object, current_time,
				play_direction))
			{
				Time_object_notify_clients_privileged(object_info->time_object);
			}
			object_info->next_callback_due = Time_object_get_next_callback_time(
				object_info->time_object, current_time,
				play_direction);
			object_info = object_info->next;
		}
		notifyClients(TIME_KEEPER_APP_NEW_TIME);

		return_code = setPlayTimeout();
	}

	return (return_code);
}


int Time_keeper_app::addCallback(Time_keeper_app_callback callback, void *user_data,
	enum Time_keeper_app_event event_mask)
{
	int return_code = 0;
	struct Time_keeper_app_callback_data *callback_data, *previous;

	if (callback)
	{
		if(ALLOCATE(callback_data, struct Time_keeper_app_callback_data, 1))
		{
			callback_data->callback = callback;
			callback_data->callback_user_data = user_data;
			callback_data->event_mask = event_mask;
			callback_data->next = (struct Time_keeper_app_callback_data *)NULL;
			if(callback_list)
			{
				previous = callback_list;
				while(previous->next)
				{
					previous = previous->next;
				}
				previous->next = callback_data;
			}
			else
			{
				callback_list = callback_data;
			}
			return_code = 1;
		}
	}

	return (return_code);
}

int Time_keeper_app::removeCallback(Time_keeper_app_callback callback,
	void *user_data)
{
	int return_code = 1;
	struct Time_keeper_app_callback_data *callback_data, *previous;

	if (callback && callback_list)
	{
		callback_data = callback_list;
		if((callback_data->callback == callback)
			&& (callback_data->callback_user_data == user_data))
		{
			callback_list = callback_data->next;
			DEALLOCATE(callback_data);
		}
		else
		{
			return_code = 0;
			while(!return_code && callback_data->next)
			{
				previous = callback_data;
				callback_data = callback_data->next;
				if((callback_data->callback == callback)
					&& (callback_data->callback_user_data == user_data))
				{
					previous->next = callback_data->next;
					DEALLOCATE(callback_data);
					return_code = 1;
				}
			}
		}
	}
	else
	{
		return_code=0;
	}

	return (return_code);
}

int Time_keeper_app::requestNewTime(double new_time)
{
	int playing = 0;

	if(timeout_callback_id)
	{
		playing = 1;
		stopPrivate();
	}
	time_keeper->setTime(new_time);
	notifyClients(TIME_KEEPER_APP_NEW_TIME);
	if(playing)
	{
		playPrivate();
	}

	return 1;
}

Cmiss_time_keeper *Time_keeper_app::getTimeKeeper()
{
	return time_keeper;
}

int Time_keeper_app::timerEvent()
{
	double event_time = 0.0, real_time_elapsed, closest_object_time, event_interval;
	int first_event_time, return_code;
	Time_keeper_app *time_keeper_app;
	struct Time_object_info *object_info;
	struct timeval timeofday;
	double current_time = 0.0;

	if(timeout_callback_id)
	{
		timeout_callback_id = (struct Event_dispatcher_timeout_callback *)NULL;
		first_event_time = 1;

		gettimeofday(&timeofday, (struct timezone *)NULL);
		real_time_elapsed = (double)(timeofday.tv_sec -
			play_start_seconds) + ((double)(timeofday.tv_usec - play_start_microseconds) / 1000000.0);
		real_time_elapsed *= speed;
		play_start_seconds = timeofday.tv_sec;
		play_start_microseconds = timeofday.tv_usec;
		/* Set an interval from within which we will do every event pending event.
				When we are playing every frame we want this to be much smaller */
		if (play_every_frame)
		{
			event_interval = 0.00001 * speed;
		}
		else
		{
			event_interval = 0.01 * speed;
		}
		switch(play_direction)
		{
		case TIME_KEEPER_PLAY_FORWARD:
		{
			if(play_every_frame)
			{
				real_time += step;
			}
			else
			{
				real_time += real_time_elapsed;
			}
			/* Record the time_keeper->time so that if a callback changes it
						then we do a full restart */
			time_keeper->setTimeQuiet(real_time);
			object_info = time_keeper->getObjectInfo();
			while(object_info)
			{
				/* Do all the events in the next event interval */
				if(object_info->next_callback_due < real_time + event_interval)
				{
					if(!play_every_frame)
					{
						/* Then look for the event that should have occurred most
									recently */
						closest_object_time =
							Time_object_get_next_callback_time(
								object_info->time_object, real_time + event_interval,
								TIME_KEEPER_PLAY_BACKWARD);
						if(closest_object_time >= object_info->next_callback_due)
						{
							object_info->next_callback_due = closest_object_time;
						}
					}
					if(first_event_time || object_info->next_callback_due > event_time)
					{
						first_event_time = 0;
						event_time = object_info->next_callback_due;
					}
					if (object_info->next_callback_due < time_keeper->getMaximum())
					{
						Time_object_set_current_time_privileged(object_info->time_object,
							object_info->next_callback_due);
						Time_object_notify_clients_privileged(object_info->time_object);
					}
					object_info->next_callback_due = Time_object_get_next_callback_time(
						object_info->time_object, real_time + event_interval,
						TIME_KEEPER_PLAY_FORWARD);
				}
				else if (play_remaining && (real_time + event_interval)>time_keeper->getMaximum())
				{
					/* when playing the remaining time, the difference between
								 next_callback_due and current time is normally large then the event interval,
								 so the actual event time needs to be set to maximum here */
					event_time = time_keeper->getMaximum();
					play_remaining = 0;
				}
				object_info = object_info->next;
			}
			if (real_time >= time_keeper->getMaximum())
			{
				play_remaining = 0;
			}
		} break;
		case TIME_KEEPER_PLAY_BACKWARD:
		{
			if(play_every_frame)
			{
				real_time -= step;
			}
			else
			{
				real_time -= real_time_elapsed;
			}
			/* Record the time_keeper->time so that if a callback changes it
						then we do a full restart */
			time_keeper->setTimeQuiet(real_time);

			object_info = time_keeper->getObjectInfo();
			while(object_info)
			{
				/* Do all the events in the next 10 milliseconds */
				if(object_info->next_callback_due > real_time - event_interval)
				{
					if(!play_every_frame)
					{
						/* Then look for the event that should have occurred most
									recently */
						closest_object_time =
							Time_object_get_next_callback_time(
								object_info->time_object, real_time - event_interval,
								TIME_KEEPER_PLAY_FORWARD);
						if(closest_object_time <= object_info->next_callback_due)
						{
							object_info->next_callback_due = closest_object_time;
						}
					}
					if(first_event_time || object_info->next_callback_due < event_time)
					{
						first_event_time = 0;
						event_time = object_info->next_callback_due;
					}
					if (object_info->next_callback_due >= time_keeper->getMinimum())
					{
						Time_object_set_current_time_privileged(object_info->time_object,
							object_info->next_callback_due);
						Time_object_notify_clients_privileged(object_info->time_object);
					}
					object_info->next_callback_due = Time_object_get_next_callback_time(
						object_info->time_object, real_time - event_interval,
						TIME_KEEPER_PLAY_BACKWARD);
				}
				else if (play_remaining && (real_time - event_interval)<time_keeper->getMinimum())
				{
					/* when playing the remaining time, the difference between
								 next_callback_due and current time is normally large then the event interval,
								 so the actual event time needs to be set to minimum here */
					event_time = time_keeper->getMinimum();
					play_remaining = 0;
				}
				object_info = object_info->next;
			}
			if (real_time <= time_keeper->getMinimum())
			{
				play_remaining = 0;
			}
		} break;
		}

		if(time_keeper->getTime() == real_time)
		{
			/* We want it to appear to the clients that only actual event times
					have occured */
			time_keeper->setTimeQuiet(event_time);
			if(!first_event_time)
			{
				notifyClients(TIME_KEEPER_APP_NEW_TIME);
			}
			setPlayTimeout();
		}
		else
		{
			playPrivate();
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Time_keeper_xttimer_event_handler.  Unknown source for callback");
		return_code = 0;
	}

	return (return_code);
}

void Time_keeper_app::setPlayLoop()
{
	play_mode = TIME_KEEPER_APP_PLAY_LOOP;
}

void Time_keeper_app::setPlayOnce()
{
	play_mode = TIME_KEEPER_APP_PLAY_ONCE;
}

void Time_keeper_app::setPlaySwing()
{
	play_mode = TIME_KEEPER_APP_PLAY_SWING;
}

int Time_keeper_app::getPlayEveryFrame()
{
	return play_every_frame;
}

void Time_keeper_app::setPlayEveryFrame()
{
	play_every_frame = 1;
}

void Time_keeper_app::setPlaySkipFrames()
{
	play_every_frame = 0;
}

int Time_keeper_app::setPlayTimeout()
{
	double next_time, real_time_elapsed, sleep;
	int return_code;
	struct Time_object_info *object_info;
	struct timeval timeofday;
	unsigned long sleep_s, sleep_ns;
	double maximum = time_keeper->getMaximum(),
		minimum = time_keeper->getMinimum(),
		current_time = time_keeper->getTime();

	if (event_dispatcher)
	{
		object_info = time_keeper->getObjectInfo();
		if(object_info)
		{
			switch(play_direction)
			{
			case TIME_KEEPER_PLAY_FORWARD:
			{
				next_time = object_info->next_callback_due;
				object_info = object_info->next;
				while(object_info)
				{
					if(object_info->next_callback_due < next_time )
					{
						next_time = object_info->next_callback_due;
					}
					object_info = object_info->next;
				}

				if ((next_time > maximum) &&
					(current_time < maximum))
				{
					next_time = maximum;
					play_remaining = 1;
				}
				if((next_time > maximum) && !(play_remaining))
				{
					if(current_time > minimum)
					{
						switch(play_mode)
						{
						case TIME_KEEPER_APP_PLAY_LOOP:
						{
							return_code = playPrivate();
						} break ;
						case TIME_KEEPER_APP_PLAY_SWING:
						{
							time_keeper->setTimeQuiet(maximum);
							play_direction = TIME_KEEPER_PLAY_BACKWARD;
							notifyClients(TIME_KEEPER_APP_CHANGED_DIRECTION);
							return_code = playPrivate();
						} break ;
						case TIME_KEEPER_APP_PLAY_ONCE:
						{
							stop();
							return_code = 0;
						} break;
						default:
						{
							stop();
							return_code = 0;
						} break;
						}
					}
					else
					{
						stop();
						return_code = 0;
					}
				}
				else
				{
					if(next_time >= real_time)
					{
						step = next_time - real_time;

						gettimeofday(&timeofday, (struct timezone *)NULL);
						real_time_elapsed = (double)(timeofday.tv_sec -
							play_start_seconds) + ((double)(timeofday.tv_usec
								- play_start_microseconds) / 1000000.0);
						sleep = step / speed - real_time_elapsed;
						if (sleep > 0)
						{
							sleep_s = (unsigned long)floor(sleep);
							sleep_ns = (unsigned long)((sleep - floor(sleep))*1e9);
						}
						else
						{
							sleep_s = 0;
							sleep_ns = 0;
						}
						if((sleep_s < 1) && (sleep_ns < 3000000))
						{
							/* Ensure all the events from the previous timestamp are
										processed before the events from this next callback occur */
							sleep_ns = 3000000;
						}
						timeout_callback_id = Event_dispatcher_add_timeout_callback(event_dispatcher, (unsigned long)sleep_s, sleep_ns,
							Time_keeper_app_timer_event_handler, (void *)this);
						return_code=1;
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Time_keeper_set_play_timeout.  Next time is in the past");
						return_code=0;
					}
				}
			} break;
			case TIME_KEEPER_PLAY_BACKWARD:
			{
				next_time = object_info->next_callback_due;
				object_info = object_info->next;
				while(object_info)
				{
					if(object_info->next_callback_due > next_time )
					{
						next_time = object_info->next_callback_due;
					}
					object_info = object_info->next;
				}
				if ((next_time < minimum) && (current_time > minimum))
				{
					next_time = minimum;
					play_remaining = 1;
				}
				if((next_time < minimum) && !(play_remaining))
				{
					if(current_time < maximum)
					{
						switch(play_mode)
						{
						case TIME_KEEPER_APP_PLAY_LOOP:
						{
							return_code = playPrivate();
						} break ;
						case TIME_KEEPER_APP_PLAY_SWING:
						{
							time_keeper->setTimeQuiet(minimum);
							play_direction = TIME_KEEPER_PLAY_FORWARD;
							notifyClients(TIME_KEEPER_APP_CHANGED_DIRECTION);
							return_code = playPrivate();
						} break ;
						case TIME_KEEPER_APP_PLAY_ONCE:
						{
							stop();
							return_code=0;
						} break;
						default:
						{
							stop();
							return_code=0;
						} break;
						}
					}
					else
					{
						stop();
						return_code=0;
					}
				}
				else
				{
					/*???DB.  Changed from < to <= */
					if(next_time <= real_time)
					{
						step = real_time - next_time;

						gettimeofday(&timeofday, (struct timezone *)NULL);
						real_time_elapsed = (double)(timeofday.tv_sec -
							play_start_seconds) + ((double)(timeofday.tv_usec
								- play_start_microseconds) / 1000000.0);
						sleep = step / speed - real_time_elapsed;
						if (sleep > 0)
						{
							sleep_s = (unsigned long)floor(sleep);
							sleep_ns = (unsigned long)((sleep - floor(sleep))*1e9);
						}
						else
						{
							sleep_s = 0;
							sleep_ns = 0;
						}
						if((sleep_s < 1) && (sleep_ns < 3000000))
						{
							/* Ensure all the events from the previous timestamp are
										processed before the events from this next callback occur */
							sleep_ns = 3000000;
						}
						timeout_callback_id = Event_dispatcher_add_timeout_callback(
							event_dispatcher, sleep_s, sleep_ns,
							Time_keeper_app_timer_event_handler, (void *)this);
						return_code=1;
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Time_keeper_set_play_timeout.  Playing back %lf and next time %lf is in the future (real_time %lf)",
							current_time, next_time, real_time);
						return_code=0;
					}
				}
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"Time_keeper_set_play_timeout.  Unknown play direction");
				return_code=0;
			} break;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Time_keeper_set_play_timeout.  No time objects in timekeeper");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Time_keeper_set_play_timeout.  Missing time_keeper or event_dispatcher");
		return_code=0;
	}

	return (return_code);
}

int DESTROY(Time_keeper_app)(struct Time_keeper_app **time_keeper_app_address)
{
	int return_code = 0;

	if (time_keeper_app_address && (*time_keeper_app_address))
	{
		delete *time_keeper_app_address;
		*time_keeper_app_address = NULL;
		return_code = 1;
	}

	return (return_code);
}

DECLARE_OBJECT_FUNCTIONS(Time_keeper_app)
