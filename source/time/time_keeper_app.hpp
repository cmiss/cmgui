/***************************************************************************//**
 * time_keeper_app.hpp
 *
 * Declaration of time keeper classes and functions.
 */
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
 * Portions created by the Initial Developer are Copyright (C) 2010
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

#ifndef TIME_KEEPER_APP_HPP
#define TIME_KEEPER_APP_HPP

#include "time/time_keeper.hpp"
#include <ctime>

PROTOTYPE_OBJECT_FUNCTIONS(Time_keeper_app);

struct Time_keeper_app_callback_data;

enum Time_keeper_app_event
{
	/* These constants are bit masked and so should be powers of two */
	TIME_KEEPER_APP_NEW_TIME = 1,
	TIME_KEEPER_APP_STARTED = 2,
	TIME_KEEPER_APP_STOPPED = 4,
	TIME_KEEPER_APP_CHANGED_DIRECTION = 8,
	TIME_KEEPER_APP_NEW_MINIMUM = 16,
	TIME_KEEPER_APP_NEW_MAXIMUM = 32
};


enum Time_keeper_app_play_mode
{
	TIME_KEEPER_APP_PLAY_ONCE,
	TIME_KEEPER_APP_PLAY_LOOP,
	TIME_KEEPER_APP_PLAY_SWING
};

typedef int (*Time_keeper_app_callback)(struct Time_keeper_app *time_keeper_app,
	enum Time_keeper_app_event event, void *user_data);

class Time_keeper_app
{
private:

	enum Time_keeper_app_play_mode play_mode;
	int play_remaining;
	double speed;
	double step;
	enum Time_keeper_play_direction play_direction;
	int play_every_frame;
	time_t play_start_seconds;
	long play_start_microseconds;
	double real_time;
	int playing;
	struct Event_dispatcher_timeout_callback *timeout_callback_id;
	struct Event_dispatcher *event_dispatcher;
	struct Time_keeper_app_callback_data *callback_list;
	Cmiss_time_keeper *time_keeper;

	int notifyClients(enum Time_keeper_app_event event_mask);

public:

	int access_count;

	Time_keeper_app(Cmiss_time_keeper *time_keeper_in,	struct Event_dispatcher *event_dispatcher);

	~Time_keeper_app();

	inline Time_keeper_app *access()
	{
		++access_count;
		return this;
	}

	static inline int deaccess(Time_keeper_app **time_keeper_app_address)
	{
		return DEACCESS(Time_keeper_app)(time_keeper_app_address);
	}

	int play(enum Time_keeper_play_direction play_direction_in);

	int isPlaying();

	void stopPrivate();

	void stop();

	double getSpeed();

	void setSpeed(double speed_in);

	int playPrivate();

	int addCallback(Time_keeper_app_callback callback, void *user_data,
		enum Time_keeper_app_event event_mask);

	int removeCallback(Time_keeper_app_callback callback,
		void *user_data);

	int requestNewTime(double new_time);

	Cmiss_time_keeper *getTimeKeeper();

	int timerEvent();

	void setPlayLoop();

	void setPlayOnce();

	void setPlaySwing();

	int getPlayEveryFrame();

	void setPlayEveryFrame();

	void setPlaySkipFrames();

	int setPlayTimeout();
};

#endif
