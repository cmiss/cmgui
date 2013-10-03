/***************************************************************************//**
 * time_keeper_app.hpp
 *
 * Declaration of time keeper classes and functions.
 */
/* OpenCMISS-Cmgui Application
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

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

struct Time_keeper_app
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
	cmzn_timekeeper *time_keeper;

	int notifyClients(enum Time_keeper_app_event event_mask);

public:

	int access_count;

	Time_keeper_app(cmzn_timekeeper *time_keeper_in,	struct Event_dispatcher *event_dispatcher);

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

	cmzn_timekeeper *getTimeKeeper();

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
