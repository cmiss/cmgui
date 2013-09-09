/*******************************************************************************
FILE : idle.h

LAST MODIFIED : 21 March 2005

DESCRIPTION :
The private interface to idle callback functions of cmgui.
==============================================================================*/
/* OpenCMISS-Cmgui Application
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef __IDLE_H__
#define __IDLE_H__

#include "event_dispatcher.h"
#include "api/cmiss_idle.h"

#define Idle_package cmzn_idle_package
#define Idle_package_id cmzn_idle_package_id
#define Idle_callback cmzn_idle_callback
#define Idle_callback_id cmzn_idle_callback_id
#define Idle_package_add_callback cmzn_idle_package_add_callback
#define Idle_callback_function cmzn_idle_callback_function

Idle_package_id CREATE(Idle_package)(struct Event_dispatcher *event_dispatcher);

int DESTROY(Idle_package)(Idle_package_id *pkg);

int DESTROY(Idle_callback)(Idle_package_id pkg, Idle_callback_id *callback);

#endif /* __IDLE_H__ */
