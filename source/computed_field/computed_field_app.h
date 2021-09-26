/* OpenCMISS-Cmgui Application
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */


#if !defined (COMPUTED_FIELD_APP_H_)
#define COMPUTED_FIELD_APP_H_

#include "opencmiss/zinc/region.h"
#include "command/parser.h"
#include "computed_field/computed_field.h"
#include "computed_field/field_module.hpp"
#include "general/message.h"
#include "region/cmiss_region.hpp"

/**
 * Argument to field modifier functions supplying field module, name etc.
 * Only to be used for defining one field.
 */
class Computed_field_modify_data
{
private:
	cmzn_fieldmodule *field_module;  // fieldmodule field is being defined in, accessed
	char *field_name;  // name of field being defined
	cmzn_field *field;  // existing field to modify, replaced with new field if none

public:

	/** Set modify data for field in field module with the given name.
	 * If there is an existing field of that name, store it to redefine in
	 * define_field, otherwise store the name to set in define_field.
	 */
	Computed_field_modify_data(cmzn_fieldmodule *field_module_in, const char *field_name_in);

	~Computed_field_modify_data();

	cmzn_fieldmodule *get_field_module() const
	{
		return this->field_module;
	}

	const char *get_field_name() const
	{
		return this->field_name;
	}

	/** Redefine the existing field, or define the new field.
	 * If existing field, copy definition from new field and deaccess new field.
	 * If new field, give it the new field name and take over reference.
	 * Sets MANAGED attribute so it is not destroyed.
	 *
	 * @param new_field  Field to take ownership of.
	 * @return  1 if field supplied, 0 if not.
	 */
	int define_field(cmzn_field *new_field);

	/**
	 * Get the existing field being modified, or nullptr then the field just created
	 * @return  Non-accessed field.
	 */
	cmzn_field *get_field() const
	{
		return this->field;
	}

	cmzn_region *get_region() const
	{
		return cmzn_fieldmodule_get_region_internal(this->field_module);
	}

	MANAGER(cmzn_field) *get_field_manager() const
	{
		return this->get_region()->getFieldManager();
	}
};

/**
 * The base class for each computed field classes own package.
 * Provides reference counting.
 */
class Computed_field_type_package
{
private:
	unsigned int access_count;

public:

	Computed_field_type_package() :
		access_count(0)
	{
	}

	void addref()
	{
		access_count++;
	}

	void removeref()
	{
		if (access_count > 1)
		{
			access_count--;
		}
		else
		{
			delete this;
		}
	}

protected:

	virtual ~Computed_field_type_package()
	{
	}
};

/**
 * Minimum set of type-specific data for gfx define field commands.
 * Contains nothing now that field manager is extracted from region, which is
 * passed around as part of Computed_field_modify_data in to_be_modified argument.
 */
class Computed_field_simple_package : public Computed_field_type_package
{
};

struct Computed_field_package;

int define_Computed_field(struct Parse_state *state,void *field_copy_void,
	void *define_field_package_void);
/*******************************************************************************
LAST MODIFIED : 3 February 1999

DESCRIPTION :
Modifier entry function for creating and modifying Computed_fields. Format for
parameters from the parse state are:
  FIELD_NAME|NEW_FIELD_NAME
	components #
	  rectangular_cartesian/cylindrical_polar/spherical_polar/prolate_sph...
		component
		  FIELD_NAME.COMPONENT_NAME
		composite
				  scalars FIELD_NAME FIELD_NAME... FIELD_NAME{number_of_components}
		gradient
				  scalar FIELD_NAME
					coordinate FIELD_NAME
		rc_coordinate
					coordinate FIELD_NAME
		scale
				  field FIELD_NAME
				  values # # ... #{number_of_components}
		... (more as more types added)
Note that the above layout is used because:
1. The number_of_components is often prerequisite information for setting up
the modifier functions for certain types of computed field, eg. "composite"
requires as many scalar fields as there are components, while scale has as many
floats.
2. The number_of_components and coordinate system are options for all types of
computed field so it makes sense that they are set before splitting up into the
options for the various types.
The <field_copy_void> parameter, if set, points to the field we are to modify
and should not itself be managed.
==============================================================================*/


int list_Computed_field_commands(struct Computed_field *field,
	void *command_prefix_void);
/*******************************************************************************
LAST MODIFIED : 25 January 1999

DESCRIPTION :
Writes the properties of the <field> to the command window.
==============================================================================*/

int list_Computed_field_commands_if_managed_source_fields_in_list(
	struct Computed_field *field, void *list_commands_data_void);
/*******************************************************************************
LAST MODIFIED : 14 December 2001

DESCRIPTION :
Calls list_Computed_field_commands if the field is not already in the list,
has no source fields, or all its source fields are either not managed or
already in the list. If the field is listed, it is added to the list.
Ensures field command list comes out in the order they need to be created.
Note, must be cycled through as many times as it takes till listed_fields -> 0.
Second argument is a struct List_Computed_field_commands_data.
==============================================================================*/

struct Computed_field_package *CREATE(Computed_field_package)(
	struct MANAGER(Computed_field) *computed_field_manager);
/*******************************************************************************
LAST MODIFIED : 20 May 2008

DESCRIPTION :
Creates a Computed_field_package which is used by the rest of the program to
access everything to do with computed fields.
The root_region's computed_field_manager is passed in to support old code that
expects it to be in the package. This is temporary until all code gets the true
manager from the respective cmzn_region.
==============================================================================*/

int DESTROY(Computed_field_package)(
	struct Computed_field_package **package_address);
/*******************************************************************************
LAST MODIFIED : 3 February 1999

DESCRIPTION :
Frees memory/deaccess objects in computed_field_package at <*package_address>.
Cancels any further messages from the root_region.
==============================================================================*/

int Computed_field_package_remove_types(
	struct Computed_field_package *computed_field_package);
/*******************************************************************************
LAST MODIFIED : 24 January 2007

DESCRIPTION :
Unregisters each of the computed field types added.
==============================================================================*/

struct MANAGER(Computed_field) * Computed_field_package_get_computed_field_manager(struct Computed_field_package *computed_field_package);
/*******************************************************************************
LAST MODIFIED : 3 February 1999

DESCRIPTION :
Extracts the computed_field_manager from the computed_field_package. Note that
the rest of the program should use this sparingly - it is really only here to
allow interfacing to the choose_object widgets.
==============================================================================*/

/**
 * Returns a pointer to a sharable simple type package which just contains a
 * function to access the Computed_field_package.
 */
Computed_field_simple_package *Computed_field_package_get_simple_package(
	struct Computed_field_package *computed_field_package);

/** Set name of field to the stem name supplied or append with _# to make unique. */
void cmzn_field_set_name_unique(cmzn_field *field, const char *stemName);

#endif
