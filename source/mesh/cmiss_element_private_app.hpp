/* OpenCMISS-Cmgui Application
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

extern "C" {

}

struct Option_table;

/***************************************************************************//**
 * Adds token to the option table for setting a mesh from a region.
 *
 * @param option_table  Table to add token to
 * @param token  Token to be matched. Can be NULL for final, default entry.
 * @param region  Pointer to region owning mesh.
 * @param mesh_address  Address of mesh to set. Must be initialised to 0 or
 * existing accessed pointer to mesh. Caller is responsible for deaccessing.
 */
int Option_table_add_mesh_entry(struct Option_table *option_table,
	const char *token, cmzn_region_id region, cmzn_mesh_id *mesh_address);

