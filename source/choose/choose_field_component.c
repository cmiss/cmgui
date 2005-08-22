/*******************************************************************************
FILE : choose_field_component.c

LAST MODIFIED : 21 November 2001

DESCRIPTION :
Specialized chooser widget that allows a component of an FE_field to be
selected from an option menu.
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
#include <stdio.h>
#include "general/debug.h"
#include "choose/choose_field_component.h"
#include "choose/chooser.h"
#include "finite_element/finite_element.h"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"

/*
Module variables
----------------
*/
struct Choose_field_component
/*******************************************************************************
LAST MODIFIED : 9 February 2000

DESCRIPTION :
Contains information required by the choose_field_component_widget.
==============================================================================*/
{
	struct Callback_data update_callback;
	struct Chooser *chooser;
	struct FE_field *field;
	Widget parent,widget;
}; /* struct Choose_field_component */

/*
Module functions
----------------
*/

static int choose_field_component_update(
	struct Choose_field_component *choose_field_component)
/*******************************************************************************
LAST MODIFIED : 9 February 2000

DESCRIPTION :
Tells client that the component has changed.
==============================================================================*/
{
	int return_code;

	ENTER(choose_field_component_update);
	if (choose_field_component)
	{
		if (choose_field_component->update_callback.procedure)
		{
			(choose_field_component->update_callback.procedure)(
				choose_field_component->widget,
				choose_field_component->update_callback.data,
				Chooser_get_item(choose_field_component->chooser));
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"choose_field_component_update.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* choose_field_component_update */

static void choose_field_component_destroy_CB(Widget widget,
	XtPointer client_data,XtPointer reason)
/*******************************************************************************
LAST MODIFIED : 9 February 2000

DESCRIPTION :
Callback when chooser destroyed - so also destroy choose_field_component.
==============================================================================*/
{
	struct Choose_field_component *choose_field_component;

	ENTER(choose_field_component_destroy_CB);
	USE_PARAMETER(widget);
	USE_PARAMETER(reason);
	if (choose_field_component=(struct Choose_field_component *)client_data)
	{
		DESTROY(Chooser)(&(choose_field_component->chooser));
		DEALLOCATE(choose_field_component);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"choose_field_component_destroy_CB.  Invalid argument(s)");
	}
	LEAVE;
} /* choose_field_component_destroy_CB */

static void choose_field_component_update_callback(Widget widget,
	void *choose_field_component_void,void *current_item_void)
/*******************************************************************************
LAST MODIFIED : 20 January 2000

DESCRIPTION :
Callback for change of coordinate field.
==============================================================================*/
{
	struct Choose_field_component *choose_field_component;

	ENTER(choose_field_component_update_callback);
	USE_PARAMETER(widget);
	USE_PARAMETER(current_item_void);
	if (choose_field_component=
		(struct Choose_field_component *)choose_field_component_void)
	{
		choose_field_component_update(choose_field_component);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"choose_field_component_update_callback.  Invalid argument(s)");
	}
	LEAVE;
} /* choose_field_component_update_callback */

static int choose_field_component_get_items(struct FE_field *field,
	int *number_of_items,void ***items_address,char ***item_names_address)
/*******************************************************************************
LAST MODIFIED : 10 October 2003

DESCRIPTION :
Returns the components of <field> in a form suitable for Choosers.
<*number_of_items> will contain the number of components in the field;
<*items_address> will point to an array of component numbers cast to void *;
<*item_names> will contain an array of allocated component names;
All returned strings and arrays must be deallocated by the calling function.
==============================================================================*/
{
	char **item_names;
	int i,j,number_of_components,return_code;
	void **items;

	ENTER(choose_field_component_get_items);
	if (field&&number_of_items&&items_address&&item_names_address&&
		(0<(number_of_components = get_FE_field_number_of_components(field))))
	{
		items = (void **)NULL;
		item_names = (char **)NULL;
		if (ALLOCATE(items,void *,number_of_components)&&
			ALLOCATE(item_names,char *,number_of_components))
		{
			return_code=1;
			for (i=0;(i<number_of_components)&&return_code;i++)
			{
				items[i] = INT2VOIDPTR(i);
				if (!(item_names[i]=get_FE_field_component_name(field,i)))
				{
					display_message(ERROR_MESSAGE,"choose_field_component_get_items.  "
						"Could not get field component name");
					return_code=0;
					for (j=0;j<i;j++)
					{
						DEALLOCATE(item_names[j]);
					}
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"choose_field_component_get_items.  Not enough memory");
			return_code=0;
		}
		if (return_code)
		{
			*number_of_items = number_of_components;
			*items_address = items;
			*item_names_address = item_names;
		}
		else
		{
			if (items)
			{
				DEALLOCATE(items);
			}
			if (item_names)
			{
				DEALLOCATE(item_names);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"choose_field_component_get_items.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* choose_field_component_get_items */

/*
Global functions
----------------
*/

Widget create_choose_field_component_widget(Widget parent,
	struct FE_field *field, int component_no,
	struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 10 October 2003

DESCRIPTION :
Creates an option menu from which a component of the <field> may be chosen,
initially with the given <component_no>.
Note: Choose_field_component will be automatically DESTROYed with its widgets.
<user_interface> supplies fonts.
==============================================================================*/
{
	char **item_names;
	int i,number_of_items;
	struct Callback_data callback;
	struct Choose_field_component *choose_field_component;
	void **items;
	Widget return_widget;

	ENTER(create_choose_field_component_widget);
	return_widget=(Widget)NULL;
	if (parent && user_interface)
	{
		if (ALLOCATE(choose_field_component,struct Choose_field_component,1))
		{
			choose_field_component->field=field;
			choose_field_component->chooser=(struct Chooser *)NULL;
			choose_field_component->widget=(Widget)NULL;
			choose_field_component->parent=parent;
			choose_field_component->update_callback.procedure=
				(Callback_procedure *)NULL;
			choose_field_component->update_callback.data=(void *)NULL;
			number_of_items=0;
			items=(void **)NULL;
			item_names=(char **)NULL;
			if (((struct FE_field *)NULL == field)||
				choose_field_component_get_items(field,&number_of_items,&items,
					&item_names))
			{
				if (choose_field_component->chooser = CREATE(Chooser)(parent,
					number_of_items, items, item_names, INT2VOIDPTR(component_no),
					&(choose_field_component->widget), user_interface))
				{
					/* add choose_field_component as user data to chooser widget */
					XtVaSetValues(choose_field_component->widget,
						XmNuserData,choose_field_component,NULL);
					/* add destroy callback for chooser widget */
					XtAddCallback(choose_field_component->widget,XmNdestroyCallback,
						choose_field_component_destroy_CB,
						(XtPointer)choose_field_component);
					/* get updates when chooser changes */
					callback.data=(void *)choose_field_component;
					callback.procedure=choose_field_component_update_callback;
					Chooser_set_update_callback(choose_field_component->chooser,
						&callback);
					return_widget=choose_field_component->widget;
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"create_choose_field_component_widget.  Could not create chooser");
					DEALLOCATE(choose_field_component);
				}
				if (items)
				{
					DEALLOCATE(items);
				}
				if (item_names)
				{ 
					for (i=0;i<number_of_items;i++)
					{
						DEALLOCATE(item_names[i]);
					}
					DEALLOCATE(item_names);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"create_choose_field_component_widget.  Could not get items");
				DEALLOCATE(choose_field_component);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"create_choose_field_component_widget.  "
				"Could not allocate choose_field_component");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_choose_field_component_widget.  Invalid argument(s)");
	}
	LEAVE;

	return (return_widget);
} /* CREATE_choose_field_component_widget */

struct Callback_data *choose_field_component_get_callback(
	Widget choose_field_component_widget)
/*******************************************************************************
LAST MODIFIED : 9 February 2000

DESCRIPTION :
Returns a pointer to the callback item of the choose_field_component_widget.
==============================================================================*/
{
	struct Callback_data *callback;
	struct Choose_field_component *choose_field_component;

	ENTER(choose_field_component_get_callback);
	if (choose_field_component_widget)
	{
		/* Get the pointer to the data for the choose_field_component dialog */
		XtVaGetValues(choose_field_component_widget,XmNuserData,
			&choose_field_component,NULL);
		if (choose_field_component)
		{
			callback=&(choose_field_component->update_callback);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"choose_field_component_get_callback.  Missing widget data");
			callback=(struct Callback_data *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"choose_field_component_get_callback.  Missing widget");
		callback=(struct Callback_data *)NULL;
	}
	LEAVE;

	return (callback);
} /* choose_field_component_get_callback */

int choose_field_component_set_callback(Widget choose_field_component_widget,
	struct Callback_data *new_callback)
/*******************************************************************************
LAST MODIFIED : 9 February 2000

DESCRIPTION :
Changes the callback item of the choose_field_component_widget.
==============================================================================*/
{
	int return_code;
	struct Choose_field_component *choose_field_component;

	ENTER(choose_field_component_set_callback);
	if (choose_field_component_widget&&new_callback)
	{
		/* Get the pointer to the data for the choose_field_component dialog */
		XtVaGetValues(choose_field_component_widget,XmNuserData,
			&choose_field_component,NULL);
		if (choose_field_component)
		{
			choose_field_component->update_callback.procedure=new_callback->procedure;
			choose_field_component->update_callback.data=new_callback->data;
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"choose_field_component_set_callback.  Missing widget data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"choose_field_component_set_callback.  Missing widget");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* choose_field_component_set_callback */

int choose_field_component_get_field_component(
	Widget choose_field_component_widget,struct FE_field **field,
	int *component_no)
/*******************************************************************************
LAST MODIFIED : 10 October 2003

DESCRIPTION :
Returns the current field and component number in the
choose_field_component_widget.
==============================================================================*/
{
	int return_code;
	struct Choose_field_component *choose_field_component;

	ENTER(choose_field_component_get_field_component);
	if (choose_field_component_widget)
	{
		/* Get the pointer to the data for the choose_field_component dialog */
		XtVaGetValues(choose_field_component_widget,XmNuserData,
			&choose_field_component,NULL);
		if (choose_field_component)
		{
			*field=choose_field_component->field;
			*component_no=
				VOIDPTR2INT(Chooser_get_item(choose_field_component->chooser));
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"choose_field_component_get_field_component.  Missing widget data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"choose_field_component_get_field_component.  Missing widget");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* choose_field_component_get_field_component */

int choose_field_component_set_field_component(
	Widget choose_field_component_widget,
	struct FE_field *field,int component_no)
/*******************************************************************************
LAST MODIFIED : 10 October 2003

DESCRIPTION :
Changes the field component in the choose_field_component_widget.
==============================================================================*/
{
	char **item_names;
	int i,number_of_items,return_code;
	struct Choose_field_component *choose_field_component;
	void **items;

	ENTER(choose_field_component_set_field_component);
	if (choose_field_component_widget)
	{
		/* Get the pointer to the data for the choose_field_component dialog */
		XtVaGetValues(choose_field_component_widget,XmNuserData,
			&choose_field_component,NULL);
		if (choose_field_component)
		{
			/* set the field immediately in case choose calls back with change */
			choose_field_component->field=field;
			number_of_items=0;
			items=(void **)NULL;
			item_names=(char **)NULL;
			if (((struct FE_field *)NULL == field)||
				choose_field_component_get_items(field,&number_of_items,&items,
					&item_names))
			{
				if (Chooser_build_main_menu(
					choose_field_component->chooser,number_of_items,
					items,item_names,INT2VOIDPTR(component_no)))
				{
					return_code=1;
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"choose_field_component_set_field_component.  "
						"Could not build chooser menu");
					return_code=0;
				}
				if (items)
				{
					DEALLOCATE(items);
				}
				if (item_names)
				{
					for (i=0;i<number_of_items;i++)
					{
						DEALLOCATE(item_names[i]);
					}
					DEALLOCATE(item_names);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"choose_field_component_set_field_component.  Could not get items");
				return_code=0;
			}
			if (!return_code)
			{
				choose_field_component->field=(struct FE_field *)NULL;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"choose_field_component_set_field_component.  Missing widget data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"choose_field_component_set_field_component.  Missing widget");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* choose_field_component_set_field_component */

