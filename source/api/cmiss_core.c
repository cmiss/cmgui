/*******************************************************************************
FILE : cmiss_core.c

LAST MODIFIED : 12 August 2003

DESCRIPTION :
The public interface to the some of the internal functions of cmiss.
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
#include <stdlib.h>
#include "general/debug.h"
#include "user_interface/message.h"

/*
Global functions
----------------
*/

void *Cmiss_allocate(int bytes)
/*******************************************************************************
LAST MODIFIED : 12 August 2003

DESCRIPTION :
==============================================================================*/
{
	void *return_ptr;

	ENTER(Cmiss_allocate);
	if (bytes)
	{
		ALLOCATE(return_ptr, char, bytes);
	}
	else
	{
		return_ptr = NULL;
	}
	LEAVE;

	return (return_ptr);
} /* Cmiss_allocate */

int Cmiss_deallocate(void *ptr)
/*******************************************************************************
LAST MODIFIED : 12 September 2002

DESCRIPTION :
Frees the memory associated with the pointer.  Used to clean up when functions
return buffers allocated internally to cmiss.
==============================================================================*/
{
	int return_code;

	ENTER(Cmiss_deallocate);
	if (ptr)
	{
		DEALLOCATE(ptr);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"Cmiss_deallocate.  "
			"Invalid pointer.");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_deallocate */
