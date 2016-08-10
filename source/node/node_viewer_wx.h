/***************************************************************************//**
 * FILE : node_viewer_wx.h
 *
 * Dialog for selecting nodes and viewing and/or editing field values. Works
 * with selection to display the last selected node, or set it if entered in
 * this dialog.
 */
/* OpenCMISS-Cmgui Application
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#if !defined (NODE_VIEWER_WX_H)
#define NODE_VIEWER_WX_H
 
#include "opencmiss/zinc/types/regionid.h"
#include "opencmiss/zinc/types/timekeeperid.h"

/*
Global Types
------------
*/

/***************************************************************************//**
 * Node viewer/editor dialog object.
 */
struct Node_viewer;

/*
Global Functions
----------------
*/

/***************************************************************************//**
 * Creates a dialog for choosing nodes and displaying and editing their fields.
 */
struct Node_viewer *Node_viewer_create(
	struct Node_viewer **node_viewer_address,
	const char *dialog_title,
	cmzn_region_id root_region, cmzn_field_domain_type domain_type,
	cmzn_timekeeper_id timekeeper);

/***************************************************************************//**
 * Closes and destroys the Node_viewer.
 */
int Node_viewer_destroy(struct Node_viewer **node_viewer_address);

/***************************************************************************//**
 * Pops the window for <node_viewer> to the front of those visible.
 */
int Node_viewer_bring_window_to_front(struct Node_viewer *node_viewer);

#endif /* !defined (NODE_VIEWER_WX_H) */
