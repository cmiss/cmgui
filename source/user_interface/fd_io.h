/*******************************************************************************
FILE : fd_io.c

LAST MODIFIED : 26 May 2005

DESCRIPTION :
The private interface to file-descriptor I/O functions of cmiss.

==============================================================================*/
/* OpenCMISS-Cmgui Application
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef __FD_IO_H__
#define __FD_IO_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "api/cmiss_fdio.h"

/* Map private names to public function names. */
#define Fdio_package cmzn_fdio_package
#define Fdio_package_id cmzn_fdio_package_id
#define Fdio cmzn_fdio
#define Fdio_id cmzn_fdio_id
#define destroy_Fdio_package DESTROY(cmzn_fdio_package)
#define Fdio_package_create_Fdio cmzn_fdio_package_create_cmzn_fdio
#define destroy_Fdio DESTROY(cmzn_fdio)
#define Fdio_callback cmzn_fdio_callback
#define Fdio_set_read_callback cmzn_fdio_set_read_callback
#define Fdio_set_write_callback cmzn_fdio_set_write_callback

/* Forward declaration to avoid cycle... */
struct Event_dispatcher;

Fdio_package_id CREATE(Fdio_package)(struct Event_dispatcher *event_dispatcher);
/*******************************************************************************
LAST MODIFIED : 15 February 2005

DESCRIPTION :
Creates a new cmzn_IO_package, given an event dispatcher.
==============================================================================*/

int DESTROY(Fdio)(cmzn_fdio_id* handle);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __FD_IO_H__ */
