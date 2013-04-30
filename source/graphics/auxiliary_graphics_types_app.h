




struct User_interface;
struct Parse_state;


typedef GLfloat Triple[3];

typedef float Triple[3];

/**
 * A modifier function for setting graphic face type XI1_0, XI1_1 etc.
 */
int set_graphic_face_type(struct Parse_state *state, void *face_type_address_void,
	void *dummy_user_data);

int set_Circle_discretization(struct Parse_state *state,
	void *circle_discretization_void,void *user_interface_void);
/*******************************************************************************
LAST MODIFIED : 2 June 1998

DESCRIPTION :
A modifier function for setting number of segments used to draw circles.
==============================================================================*/

int set_Element_discretization(struct Parse_state *state,
	void *element_discretization_void,void *user_interface_void);
/*******************************************************************************
LAST MODIFIED : 30 October 1996

DESCRIPTION :
A modifier function for setting discretization in each element direction.
==============================================================================*/
