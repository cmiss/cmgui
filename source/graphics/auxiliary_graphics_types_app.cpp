

#include "zinc/types/elementid.h"

#include "command/parser.h"

#include "general/enumerator_private.h"

#include "general/message.h"
#include "general/mystring.h"
#include "general/enumerator_private.hpp"
#include "general/message.h"
#include "general/debug.h"
#include "user_interface/user_interface.h"
#include "graphics/auxiliary_graphics_types.h"

int set_graphic_face_type(struct Parse_state *state, void *face_type_address_void,
	void *dummy_user_data)
{
	int return_code = 1;
	USE_PARAMETER(dummy_user_data);
	enum Cmiss_element_face_type *face_type_address =
		static_cast<Cmiss_element_face_type*>(face_type_address_void);
	if (state && face_type_address)
	{
		const char *current_token = state->current_token;
		if (current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				/*???DB.  Only valid for cubes (not polygons) */
				if (fuzzy_string_compare(current_token,"XI1_0"))
				{
					*face_type_address = CMISS_ELEMENT_FACE_XI1_0;
					shift_Parse_state(state,1);
				}
				else if (fuzzy_string_compare(current_token,"XI1_1"))
				{
					*face_type_address = CMISS_ELEMENT_FACE_XI1_1;
					shift_Parse_state(state,1);
				}
				else if (fuzzy_string_compare(current_token,"XI2_0"))
				{
					*face_type_address = CMISS_ELEMENT_FACE_XI2_0;
					shift_Parse_state(state,1);
				}
				else if (fuzzy_string_compare(current_token,"XI2_1"))
				{
					*face_type_address = CMISS_ELEMENT_FACE_XI2_1;
					shift_Parse_state(state,1);
				}
				else if (fuzzy_string_compare(current_token,"XI3_0"))
				{
					*face_type_address = CMISS_ELEMENT_FACE_XI3_0;
					shift_Parse_state(state,1);
				}
				else if (fuzzy_string_compare(current_token,"XI3_1"))
				{
					*face_type_address = CMISS_ELEMENT_FACE_XI3_1;
					shift_Parse_state(state,1);
				}
				else
				{
					display_message(ERROR_MESSAGE, "set_graphic_face_type.  Invalid face type %s", current_token);
					return_code = 0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE,
					" <xi1_0|xi1_1|xi2_0|xi2_1|xi3_0|xi3_1>");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE, "set_graphic_face_type.  Missing face type");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_graphic_face_type.  Invalid argument(s)");
		return_code = 0;
	}
	return (return_code);
}

int set_circle_divisions(struct Parse_state *state,
	void *circle_divisions_void, void *dummy_void)
{
	int return_code;
	int *circle_divisions;
	if (state)
	{
		const char *current_token=state->current_token;
		if (current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				circle_divisions = (int *)circle_divisions_void;
				if (circle_divisions)
				{
					if (1==sscanf(current_token,"%d",circle_divisions))
					{
						shift_Parse_state(state,1);
						return_code = ((*circle_divisions) >= 3);
						if (!return_code)
						{
							display_message(ERROR_MESSAGE, "Circle divisions must be >= 3");
							display_parse_state_location(state);
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"Invalid circle divisions: %s",
							current_token);
						display_parse_state_location(state);
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"set_circle_divisions.  Missing circle divisions");
					return_code=0;
				}
			}
			else
			{
				circle_divisions = (int *)circle_divisions_void;
				if (circle_divisions && (3 <= *circle_divisions))
				{
					display_message(INFORMATION_MESSAGE,
						" #[%d]{integer>=3}", *circle_divisions);
				}
				else
				{
					display_message(INFORMATION_MESSAGE," #{integer>=3}");
				}
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Missing circle discretization");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_circle_divisions.  Missing state");
		return_code=0;
	}
	return (return_code);
}

int set_Element_discretization(struct Parse_state *state,
	void *element_discretization_void,void *user_interface_void)
/*******************************************************************************
LAST MODIFIED : 27 April 1999

DESCRIPTION :
A modifier function for setting discretization in each element direction.
==============================================================================*/
{
	const char *current_token;
	int return_code,multiple_default;
	struct Element_discretization *element_discretization;

	USE_PARAMETER(user_interface_void);
	ENTER(set_Element_discretization);
	if (state)
	{
		current_token=state->current_token;
		if (current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				element_discretization=
					(struct Element_discretization *)element_discretization_void;
				if (element_discretization)
				{
					if (1==sscanf(current_token,"%d",
						&(element_discretization->number_in_xi1)))
					{
						if (2==sscanf(current_token,"%d*%d",
							&(element_discretization->number_in_xi1),
							&(element_discretization->number_in_xi2)))
						{
							if (3!=sscanf(current_token,"%d*%d*%d",
								&(element_discretization->number_in_xi1),
								&(element_discretization->number_in_xi2),
								&(element_discretization->number_in_xi3)))
							{
								element_discretization->number_in_xi3=
									element_discretization->number_in_xi2;
							}
						}
						else
						{
							element_discretization->number_in_xi2=
								element_discretization->number_in_xi1;
							element_discretization->number_in_xi3=
								element_discretization->number_in_xi2;
						}
						shift_Parse_state(state,1);
						return_code=check_Element_discretization(element_discretization);
					}
					else
					{
						display_message(ERROR_MESSAGE,"Invalid discretization: %s",
							current_token);
						display_parse_state_location(state);
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"set_Element_discretization.  Missing element_discretization");
					return_code=0;
				}
			}
			else
			{
				/* write help */
				element_discretization=
					(struct Element_discretization *)element_discretization_void;
				if (element_discretization)
				{
					multiple_default=
						(0>element_discretization->number_in_xi1)
						||(0>element_discretization->number_in_xi2)
						||(0>element_discretization->number_in_xi3);
					if (element_discretization->number_in_xi3)
					{
						if (multiple_default)
						{
							display_message(INFORMATION_MESSAGE,
								" #*#*#[CURRENT]{integer*integer*integer}");
						}
						else
						{
							display_message(INFORMATION_MESSAGE,
								" #*#*#[%d*%d*%d]{integer*integer*integer}",
								element_discretization->number_in_xi1,
								element_discretization->number_in_xi2,
								element_discretization->number_in_xi3);
						}
					}
					else
					{
						if (element_discretization->number_in_xi2)
						{
							if (multiple_default)
							{
								display_message(INFORMATION_MESSAGE,
									" #*#[CURRENT]{integer*integer}");
							}
							else
							{
								display_message(INFORMATION_MESSAGE,
									" #*#[%d*%d]{integer*integer}",
									element_discretization->number_in_xi1,
									element_discretization->number_in_xi2);
							}
						}
						else
						{
							if (multiple_default)
							{
								display_message(INFORMATION_MESSAGE," #[CURRENT]{integer}");
							}
							else
							{
								display_message(INFORMATION_MESSAGE," #[%d]{integer}",
									element_discretization->number_in_xi1);
							}
						}
					}
				}
				else
				{
					display_message(INFORMATION_MESSAGE,
						" #*#*#{integer*integer*integer}");
				}
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Missing element discretization");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_Element_discretization.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_Element_discretization */
