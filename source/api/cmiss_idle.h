/*******************************************************************************
FILE : api/cmiss_idle.h

LAST MODIFIED : 21 March, 2005

DESCRIPTION :
The public interface to idle callbacks
==============================================================================*/
/* OpenCMISS-Cmgui Application
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef __API_CMZN_IDLE_H__
#define __API_CMZN_IDLE_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct cmzn_idle_package * cmzn_idle_package_id;
/*******************************************************************************
LAST MODIFIED : 21 March 2005

DESCRIPTION :
An identifier for an idle package object.
==============================================================================*/

typedef struct cmzn_idle_callback * cmzn_idle_callback_id;
/*******************************************************************************
LAST MODIFIED : 21 March 2005

DESCRIPTION :
An identifier for an idle callback object.
==============================================================================*/

typedef int cmzn_idle_callback_function(void *user_data);
/*******************************************************************************
LAST MODIFIED : 21 March 2005

DESCRIPTION :
The type used for idle callback data.
==============================================================================*/

cmzn_idle_callback_id cmzn_idle_package_add_callback(cmzn_idle_package_id pkg,
 cmzn_idle_callback_function *callback, void *user_data);
/*******************************************************************************
LAST MODIFIED : 21 March 2005

DESCRIPTION :
Sets an idle callback.
==============================================================================*/

int cmzn_idle_callback_destroy(cmzn_idle_package_id pkg,
	cmzn_idle_callback_id *callback);
/*******************************************************************************
LAST MODIFIED : 21 March 2005

DESCRIPTION :
Destroys an idle callback, causing it to not be called any longer.
==============================================================================*/

int cmzn_idle_package_destroy(cmzn_idle_package_id *pkg);
/*******************************************************************************
LAST MODIFIED : 21 March 2005

DESCRIPTION :
Destroys the idle package object.
==============================================================================*/

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */
#endif /* __API_CMZN_IDLE_H__ */
