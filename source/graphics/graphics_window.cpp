/*******************************************************************************
FILE : graphics_window.cpp

LAST MODIFIED : 9 May 2007

DESCRIPTION:
Code for opening, closing and working a cmgui 3D display window.

Have get/set routines for parameters specific to window and/or which have
widgets that are automatically updated if you set them. Use these functions
if supplied, otherwise use Graphics_window_get_Scene_viewer() for the pane_no of
interest and set scene_viewer values directly.
==============================================================================*/
/* OpenCMISS-Cmgui Application
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#include <string>
#if 1
#include "configure/cmgui_configure.h"
#endif /* defined (1) */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "opencmiss/zinc/fieldmodule.h"
#include "opencmiss/zinc/scenefilter.h"
#include "opencmiss/zinc/sceneviewer.h"
#include "command/parser.h"
#include "computed_field/computed_field_image.h"
#include "general/debug.h"
#include "general/geometry.h"
#include "general/indexed_list_private.h"
#include "general/manager_private.h"
#include "general/matrix_vector.h"
#include "general/mystring.h"
#include "general/object.h"
#include "general/photogrammetry.h"
#include "graphics/colour.h"
#include "graphics/graphics.h"
#include "graphics/graphics_window.h"
#include "graphics/graphics_window_private.hpp"
#if defined (WX_USER_INTERFACE)
#include "wx/wx.h"
#include <wx/tglbtn.h>
#include <wx/splitter.h>
#include "wx/xrc/xmlres.h"
#include "choose/choose_manager_class.hpp"
#include "graphics/graphics_window.xrch"
#endif /* defined (WX_USER_INTERFACE)*/
#include "graphics/light.hpp"
#include "graphics/scene.h"
#include "graphics/scene.hpp"
#include "graphics/scenefilter.hpp"
#include "graphics/scenefilter_app.hpp"
#include "graphics/scene_viewer.h"
#include "graphics/texture.h"
#include "graphics/transform_tool.h"
#if defined (WX_USER_INTERFACE)
#include "computed_field/computed_field_finite_element.h"
#include "computed_field/computed_field_set.h"
#include "node/node_tool.h"
#include "interaction/interactive_tool_private.h"
#include "icon/Data_tool_clicked.xpm"
#include "icon/Data_tool_unclicked.xpm"
#include "icon/Element_point_tool_clicked.xpm"
#include "icon/Element_point_tool_unclicked.xpm"
#include "icon/Element_tool_clicked.xpm"
#include "icon/Element_tool_unclicked.xpm"
#if defined (USE_OPENCASCADE)
#include "icon/Cad_tool_clicked.xpm"
#include "icon/Cad_tool_unclicked.xpm"
#endif /* defined (USE_OPENCASCADE) */
#include "icon/Node_tool_clicked.xpm"
#include "icon/Node_tool_unclicked.xpm"
#include "icon/Transform_tool_clicked.xpm"
#include "icon/Transform_tool_unclicked.xpm"
#include "icon/cmiss_icon.xpm"
#include "icon/backward.xpm"
#include "icon/backward_by_frame.xpm"
#include "icon/fastbackward.xpm"
#include "icon/fastforward.xpm"
#include "icon/forward.xpm"
#include "icon/forward_by_frame.xpm"
#include "icon/stop_button.xpm"
#include "icon/cross.xpm"
#include "icon/cross_selected.xpm"
#endif /* defined (WX_USER_INTERFACE) */
#include "general/message.h"
#include "user_interface/user_interface.h"
/* for writing bitmap to file: */
#include "general/image_utilities.h"
#include "three_d_drawing/graphics_buffer.h"
#include "time/time_keeper_app.hpp"
#include "user_interface/confirmation.h"
#if defined (WIN32_USER_INTERFACE)
//#define WINDOWS_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#endif /* defined (WIN32_USER_INTERFACE) */
#include "user_interface/process_list_or_write_command.hpp"
#include "graphics/colour_app.h"
#include "graphics/scene_app.h"
#include "graphics/light_app.h"
#include "three_d_drawing/graphics_buffer_app.h"
#include "graphics/scene_viewer_app.h"
#include "region/cmiss_region_chooser_wx.hpp"
/*
Module constants
----------------
*/



#define TIME_STEP 0.1
static const char *axis_name[7]={"??","x","y","z","-x","-y","-z"};

/*
Module types
------------
*/
#if defined (WX_USER_INTERFACE)
class wxGraphicsWindow;
#endif /* defined (WX_USER_INTERFACE) */

struct Graphics_window
/*******************************************************************************
LAST MODIFIED : 8 September 2000

DESCRIPTION :
Contains information for a graphics window.
==============================================================================*/
{
	/* identifier for uniquely specifying window: */
	const char *name;

	/* after clearing in create, following to be modified only by manager */
	/* need to keep graphics window manager so window can be destroyed by self */
	struct MANAGER(Graphics_window) *graphics_window_manager;
	int manager_change_status;

	struct Graphics_buffer_app_package *graphics_buffer_package;
	cmzn_sceneviewermodule_id sceneviewermodule;
#if defined (GTK_USER_INTERFACE)
	GtkWidget *shell_window;
#if GTK_MAJOR_VERSION >= 2
	gulong close_handler_id;
#endif /* GTK_MAJOR_VERSION >= 2 */
#elif defined (WIN32_USER_INTERFACE)
	HWND hWnd;
#elif defined (WX_USER_INTERFACE)
	double maximum_time, minimum_time, current_time;
	wxBitmapButton *fast_backward, *backward_by_frame, *backward, *stop_button,
		*forward, *forward_by_frame, *fast_forward, *hide_time_bitmapbutton;
	wxBitmapButton *transform_tool_button, *node_tool_button, *data_tool_button,
		*element_tool_button, *cad_tool_button, *element_point_tool_button;
	wxButton *front_view_options;
	wxCheckBox *wx_perspective_button, *time_play_every_frame_checkbox;
	wxChoice *up_view_options;
	wxFrame *GraphicsWindowTitle;
	wxGraphicsWindow *wx_graphics_window;
	wxGridSizer *grid_field;
	wxPanel *panel, *panel2, *panel3, *panel4,*interactive_toolbar_panel, *time_editor_panel, *right_panel;
	wxScrolledWindow  *left_panel, *ToolPanel;
	wxSlider *time_slider;
	wxTextCtrl *time_text_ctrl, *time_framerate_text_ctrl, *time_step_size_text_ctrl;
	wxToggleButton *time_editor_togglebutton;
#endif /* defined (GTK_USER_INTERFACE) */
	/* scene_viewers and their parameters: */
	enum Graphics_window_layout_mode layout_mode;
	struct Scene_viewer_app **scene_viewer_array;
	/* The viewing_width and viewing_height are the size of the viewing area when
		 the graphics window has only one pane. When multiple panes are used, they
		 are separated by 2 pixel borders within the viewing area.
		 These defaults are read in by Graphics_window_read_defaults. */
	int default_viewing_height,default_viewing_width;
	/* number_of_panes is a function of layout_mode, but stored for convenience */
	int number_of_panes;
	/* number_of_scene_viewers that exist in this graphics_window */
	int number_of_scene_viewers;
	/* angle of view in degrees set by set_std_view_angle function */
	double std_view_angle;
	/* distance between eyes for 3-D viewing */
	double eye_spacing;
	/* for speeding/slowing translate, tumble and zoom */
	double default_translate_rate,default_tumble_rate,default_zoom_rate;
	int ortho_up_axis,ortho_front_axis;
	/* current pane for commands affecting view. First pane is 0 internally, but
		 user should be presented with this value+1, so the first pane is 1 */
	int current_pane;
	int antialias_mode;
	int perturb_lines;
	enum Scene_viewer_input_mode input_mode;
	enum cmzn_sceneviewer_blending_mode blending_mode;
	double depth_of_field;
	double focal_depth;
	/*???DB.  Do these belong here ? */
	/* time parameters for animation */
	/* current time for frame */
	double time_value;
	/* time at node */
	double animation_time;
	/* maximum animation time */
	double time_max;
	/* time step between frames */
	double time_step;
	/* not sure if graphics window should keep pointer to scene - could just get
		 it from scene_viewers; could even be different in each scene_viewer */
	cmzn_scene *scene;
	/* graphics window does not need to keep managers now that changes handled
		 by scene_viewer */
	cmzn_scenefiltermodule_id filter_module;
	/* interaction */
	struct MANAGER(Interactive_tool) *interactive_tool_manager;
	/* Note: interactive_tool is NOT accessed by graphics_window; the chooser
		 will update it if the current one is destroyed */
	struct Interactive_tool *interactive_tool;
	/* the time_slider is attached to the default_time_keeper,
		the reference is kept so that the callbacks can be undone */
	struct Time_keeper_app *time_keeper_app;
	struct User_interface *user_interface;
	cmzn_region_id root_region;
	/* the number of objects accessing this window. The window cannot be removed
		from manager unless it is 1 (ie. only the manager is accessing it) */
	int access_count;
}; /* struct Graphics_window */

FULL_DECLARE_INDEXED_LIST_TYPE(Graphics_window);

FULL_DECLARE_MANAGER_TYPE(Graphics_window);

struct Graphics_window_ortho_axes
{
	int up,front;
}; /* struct Graphics_window_ortho_axes */

/*
Module functions
----------------
*/
DECLARE_INDEXED_LIST_MODULE_FUNCTIONS(Graphics_window, \
		name,const char *,strcmp)
DECLARE_LOCAL_MANAGER_FUNCTIONS(Graphics_window);

struct MANAGER(Interactive_tool) *Graphics_window_get_interactive_tool_manager(struct Graphics_window *window);
/*******************************************************************************
LAST MODIFIED : 27 March 2007

DESCRIPTION :
Prototype.
==============================================================================*/



static int axis_name_to_axis_number(const char *axis_name)
/*******************************************************************************
LAST MODIFIED : 16 December 1997

DESCRIPTION :
Axes are numbered from 1 to 6 in the order X,Y,Z,-X,-Y,-Z. This function
returns the axis_number for the given axis_name. Eg. "-Y" -> 5.
A return value of 0 indicates an invalid name or other error.
==============================================================================*/
{
	int axis_number;

	ENTER(axis_name_to_axis_number);
	if (axis_name&&(0<strlen(axis_name)))
	{
		if (strchr(axis_name,'x')||strchr(axis_name,'X'))
		{
			axis_number=1;
		}
		else
		{
			if (strchr(axis_name,'y')||strchr(axis_name,'Y'))
			{
				axis_number=2;
			}
			else
			{
				if (strchr(axis_name,'z')||strchr(axis_name,'Z'))
				{
					axis_number=3;
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Invalid orthographic axis: %s",axis_name);
					axis_number=0;
				}
			}
		}
		if (axis_number&&('-'==axis_name[0]))
		{
			axis_number += 3;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"axis_name_to_axis_number.  Missing axis_name");
		axis_number=0;
	}
	LEAVE;

	return (axis_number);
} /* axis_name_to_axis_number */

static int axis_number_to_axis_vector(int axis_number,double axis[3])
/*******************************************************************************
LAST MODIFIED : 16 December 1997

DESCRIPTION :
Axes are numbered from 1 to 6 in the order X,Y,Z,-X,-Y,-Z. This function
converts the axis number to a three component [float] vector. eg 1 -> (1,0,0).
==============================================================================*/
{
	int return_code;

	ENTER(axis_number_to_axis_vector);
	return_code=1;
	switch (axis_number)
	{
		case 1: /* +X */
		{
			axis[0]= 1.0;
			axis[1]= 0.0;
			axis[2]= 0.0;
		} break;
		case 2: /* +Y */
		{
			axis[0]= 0.0;
			axis[1]= 1.0;
			axis[2]= 0.0;
		} break;
		case 3: /* +Z */
		{
			axis[0]= 0.0;
			axis[1]= 0.0;
			axis[2]= 1.0;
		} break;
		case 4: /* -X */
		{
			axis[0]=-1.0;
			axis[1]= 0.0;
			axis[2]= 0.0;
		} break;
		case 5: /* -Y */
		{
			axis[0]= 0.0;
			axis[1]=-1.0;
			axis[2]= 0.0;
		} break;
		case 6: /* -Z */
		{
			axis[0]= 0.0;
			axis[1]= 0.0;
			axis[2]=-1.0;
		} break;
		default:
		{
			display_message(ERROR_MESSAGE,
				"axis_number_to_axis_vector.  Invalid axis_number");
			return_code=0;
		} break;
	}
	LEAVE;

	return (return_code);
} /* axis_number_to_axis_vector */

#if defined (WIN32_USER_INTERFACE)
static int Graphics_window_adjust_sizes_for_window_frame(int *width, int *height)
{
	DWORD dwStyle;
	RECT window_size;
	window_size.left = 0;
	window_size.right = *width;
	window_size.top = 0;
	window_size.bottom = *height;

	/* AdjustWindowRect doesn't work for WS_SIZEBOX so attempt to account for this manually */
	/* This style should match that used by the Window Create function (except for WS_SIZEBOX). */
	dwStyle = WS_CAPTION | WS_POPUPWINDOW | WS_VISIBLE;
	AdjustWindowRect(&window_size, dwStyle, /*hasMenu*/false);
	window_size.right += 2 * (GetSystemMetrics(SM_CXSIZEFRAME) - GetSystemMetrics(SM_CXFIXEDFRAME));
	window_size.bottom += 2 * (GetSystemMetrics(SM_CYSIZEFRAME) - GetSystemMetrics(SM_CYFIXEDFRAME));

	*width = window_size.right - window_size.left;
	*height = window_size.bottom - window_size.top;

	return (1);
}
#endif /* defined (WIN32_USER_INTERFACE) */

/*
Widget Callback Module functions
--------------------------------
*/

#if defined (GTK_USER_INTERFACE)
static void Graphics_window_gtk_close_CB(GtkObject *object, gpointer graphics_window_void)
/*******************************************************************************
LAST MODIFIED : 24 January 2007

DESCRIPTION :
Called when "close" is selected from the window menu, or it is double clicked.
==============================================================================*/
{
	struct Graphics_window *graphics_window;

	ENTER(Graphics_window_gtk_close_CB);
	USE_PARAMETER(object);
	graphics_window=(struct Graphics_window *)graphics_window_void;
	if (graphics_window)
	{
		if (graphics_window->graphics_window_manager)
		{
			REMOVE_OBJECT_FROM_MANAGER(Graphics_window)(graphics_window,
				graphics_window->graphics_window_manager);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_window_gtk_close_CB.  Missing Graphics_window");
	}
	LEAVE;
} /* Graphics_window_gtk_close_CB */
#endif /* defined (GTK_USER_INTERFACE) */

static int Graphics_window_set_interactive_tool(
	struct Graphics_window *graphics_window,
	struct Interactive_tool *interactive_tool)
/*******************************************************************************
LAST MODIFIED : 13 June 2000

DESCRIPTION :
Sets the <interactive_tool> in use in the <graphics_window>. Updates the
toolbar to match the selection.
==============================================================================*/
{
	int pane_no,return_code;

	ENTER(Graphics_window_set_interactive_tool);
	if (graphics_window && graphics_window->scene_viewer_array)
	{
		graphics_window->interactive_tool=interactive_tool;
		if (Interactive_tool_is_Transform_tool(interactive_tool))
		{
			Graphics_window_set_input_mode(graphics_window,SCENE_VIEWER_TRANSFORM);
		}
		else
		{
			Graphics_window_set_input_mode(graphics_window,SCENE_VIEWER_SELECT);
		}
		for (pane_no=0;(pane_no<graphics_window->number_of_scene_viewers);pane_no++)
		{
			Scene_viewer_set_interactive_tool(
				graphics_window->scene_viewer_array[pane_no],interactive_tool);
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_window_set_interactive_tool.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Graphics_window_set_interactive_tool */

#if defined (WX_USER_INTERFACE)
static int Graphics_window_time_keeper_app_callback(struct Time_keeper_app *time_keeper_app,
	enum Time_keeper_app_event event, void *graphics_window_void)
/*******************************************************************************
LAST MODIFIED : 18 February 1999

DESCRIPTION :
Updates the time display of the time_slider
==============================================================================*/
{
	int return_code = 0;
	struct Graphics_window *window;
#if defined (WX_USER_INTERFACE)
	wxString time_string;
	int time_slider_index;
#endif

	ENTER(Graphics_window_time_keeper_callback);
	USE_PARAMETER(event);

	if (time_keeper_app && (window = (struct Graphics_window *)graphics_window_void))
	{
		if (window->time_slider && window->time_text_ctrl)
		{
			 switch(event)
			 {
					case TIME_KEEPER_APP_NEW_TIME:
					{
						 window->current_time = window->time_keeper_app->getTimeKeeper()->getTime();
						 time_string.Printf(_T("%f"),window->current_time);
						 window->time_text_ctrl->ChangeValue(time_string);
						 return_code = 1;
					} break;
					case TIME_KEEPER_APP_NEW_MINIMUM:
					{
						 window->minimum_time = window->time_keeper_app->getTimeKeeper()->getMinimum();
						 return_code = 1;
					} break;
					case TIME_KEEPER_APP_NEW_MAXIMUM:
					{
						 window->maximum_time = window->time_keeper_app->getTimeKeeper()->getMaximum();
						 return_code = 1;
					} break;
					default:
					{
						 display_message(ERROR_MESSAGE,
								"Graphics_window_time_keeper_callback.  Unknown time_keeper event");
						 return_code = 0;
					} break;
			 }
			 if (return_code)
			 {
				 if (window->maximum_time - window->minimum_time > 0)
				 {
					 time_slider_index = (int)(
						 (window->current_time - window->minimum_time)/(window->maximum_time - window->minimum_time) * 1000 + 0.5);
					 window->time_slider->SetValue(time_slider_index);
				 }
			 }
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_window_time_keeper_callback.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Graphics_window_time_keeper_callback */
#endif /* defined (WX_USER_INTERFACE) */

/*
Manager Callback Module functions
---------------------------------
*/

void Graphics_window_Scene_viewer_view_changed(struct Scene_viewer_app *scene_viewer,
	void *dummy_void, void *window_void)
/*******************************************************************************
LAST MODIFIED : 5 July 2000

DESCRIPTION :
Called whenever the view changes in parallel and perspective mode by mouse
movements in the scene_viewer. This function works out which pane has changed
and calls Graphics_window_view_changed to synchronise views in certain
layout_modes, eg. GRAPHICS_WINDOW_LAYOUT_ORTHOGRAPHIC.
==============================================================================*/
{
	int changed_pane,pane_no;
	struct Graphics_window *window;

	ENTER(Graphics_window_Scene_viewer_view_changed);
	USE_PARAMETER(dummy_void);
	if ((window=(struct Graphics_window *)window_void)&&scene_viewer)
	{
		changed_pane=-1;
		for (pane_no=0;pane_no<window->number_of_panes;pane_no++)
		{
			if (scene_viewer == window->scene_viewer_array[pane_no])
			{
				changed_pane=pane_no;
			}
		}
		if (-1 != changed_pane)
		{
			Graphics_window_view_changed(window,changed_pane);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Graphics_window_Scene_viewer_view_changed.  Unknown Scene_viewer");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_window_Scene_viewer_view_changed.  Invalid argument(s)");
	}
	LEAVE;
} /* Graphics_window_Scene_viewer_view_changed */

/*
Command Parsing Module functions
--------------------------------
*/

static int set_Graphics_window_ortho_axes(struct Parse_state *state,
	void *ortho_axes_void,void *dummy_user_data)
/*******************************************************************************
LAST MODIFIED : 8 October 1998

DESCRIPTION :
Modifier function to set the up and front directions for defining the
orthographic axes.
==============================================================================*/
{
	const char *current_token;
	int return_code;
	struct Graphics_window_ortho_axes *ortho_axes;

	ENTER(set_Graphics_window_ortho_axes);
	USE_PARAMETER(dummy_user_data);
	if (state)
	{
		ortho_axes=(struct Graphics_window_ortho_axes *)ortho_axes_void;
		if (ortho_axes)
		{
			current_token=state->current_token;
			if (current_token)
			{
				if (strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
				{
					ortho_axes->up=axis_name_to_axis_number(current_token);
					if (ortho_axes->up)
					{
						return_code=shift_Parse_state(state,1);
						if (return_code)
						{
							current_token=state->current_token;
							if (current_token)
							{
								ortho_axes->front=axis_name_to_axis_number(current_token);
								if (ortho_axes->front)
								{
									return_code=shift_Parse_state(state,1);
								}
								else
								{
									return_code=0;
								}
							}
						}
					}
					else
					{
						return_code=0;
					}
				}
				else
				{
					display_message(INFORMATION_MESSAGE,
						" ORTHO_UP{X/Y/Z/-X/-Y/-Z} ORTHO_FRONT{X/Y/Z/-X/-Y/-Z}");
					return_code=1;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,"Missing ORTHO_UP and ORTHO_FRONT");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"set_Graphics_window_ortho_axes.  Missing ortho_axes structure");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_Graphics_window_ortho_axes.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_Graphics_window_ortho_axes */

int set_image_field(struct Parse_state *state,void *field_address_void,
		void *root_region_void)
{
	const char *current_token;
	int return_code = 0;
	struct cmzn_region *root_region = NULL;
	struct Computed_field *temp_field = NULL, **field_address = NULL;

	ENTER(set_image_field);
	if (state)
	{
		current_token=state->current_token;
		if (current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				field_address=(struct Computed_field **)field_address_void;
				root_region = (struct cmzn_region *)root_region_void;
				if (field_address	&& root_region)
				{
					if (fuzzy_string_compare(current_token,"NONE"))
					{
						if (*field_address)
						{
							DEACCESS(Computed_field)(field_address);
							*field_address=(struct Computed_field *)NULL;
						}
						return_code=1;
					}
					else
					{
						struct cmzn_region *region = NULL;
						char *region_path = NULL, *field_name = NULL;
						if (cmzn_region_get_partial_region_path(root_region,
							current_token, &region, &region_path, &field_name))
						{
							cmzn_fieldmodule *field_module = cmzn_region_get_fieldmodule(region);
							return_code=1;
							if (field_name && (strlen(field_name) > 0) &&
								(strchr(field_name, CMZN_REGION_PATH_SEPARATOR_CHAR)	== NULL))
							{
								temp_field = cmzn_fieldmodule_find_field_by_name(field_module,
									field_name);
								if (temp_field &&
										!Computed_field_is_image_type(temp_field,NULL))
								{
									DEACCESS(Computed_field)(&temp_field);
									display_message(ERROR_MESSAGE,
										"set_image_field.  Field specify does not contain image "
										"information.");
									return_code=0;
								}
							}
							cmzn_fieldmodule_destroy(&field_module);
						}
						if (region_path)
							DEALLOCATE(region_path);
						if (field_name)
							DEALLOCATE(field_name);
						if (temp_field)
						{
							if (*field_address!=temp_field)
							{
								REACCESS(Computed_field)(field_address, temp_field);
							}
							DEACCESS(Computed_field)(&temp_field);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"set_image_field.  Image field does not exist");
							return_code=0;
						}
					}
					shift_Parse_state(state,1);
				}
				else
				{
					display_message(ERROR_MESSAGE,"set_image_field.  Invalid argument(s)");
					return_code=0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE," IMAGE_NAME|none");
				field_address=(struct Computed_field **)field_address_void;
				if (field_address)
				{
					temp_field= *field_address;
					if (temp_field)
					{
						char *temp_name = cmzn_field_get_name(temp_field);
						display_message(INFORMATION_MESSAGE,"[%s]",temp_name);
						DEALLOCATE(temp_name);
					}
					else
					{
						display_message(INFORMATION_MESSAGE,"[none]");
					}
				}
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Missing field name");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_image_field.  Missing state");
		return_code=0;
	}

	LEAVE;

	return (return_code);
} /* set_image_field */

static int modify_Graphics_window_background(struct Parse_state *state,
	void *window_void,void *modify_graphics_window_data_void)
/*******************************************************************************
LAST MODIFIED : 11 February 2002

DESCRIPTION :
Parser commands for modifying the background colours and textures of the
current pane of the <window>. Note that the <all_panes> options signifies that
the changes are to be applied to all panes.
==============================================================================*/
{
	char all_panes_flag,no_undistort_flag,undistort_flag;
	double max_pixels_per_polygon,texture_placement[4];
	int first_pane, last_pane, pane_no, pixelsx, pixelsy, pixelsz, return_code,
		undistort_on;
	struct Graphics_window *window;
	struct Modify_graphics_window_data *modify_graphics_window_data;
	struct Option_table *option_table, *undistort_option_table;
	struct Scene_viewer_app *scene_viewer;
	/* do not make the following static as 'set' flag must start at 0 */
	struct Set_vector_with_help_data texture_placement_data=
		{4," TEXTURE_LEFT TEXTURE_TOP TEXTURE_WIDTH TEXTURE_HEIGHT",0};
	struct Computed_field *image_field = NULL;

	ENTER(modify_Graphics_window_background);
	if (state)
	{
		modify_graphics_window_data=(struct Modify_graphics_window_data *)
			modify_graphics_window_data_void;
		if (modify_graphics_window_data)
		{
			if (state->current_token)
			{
				struct Colour background_colour = { 0.0, 0.0, 0.0, 0.0 };
				/* get defaults from scene_viewer for first pane of window */
				if ((window=(struct Graphics_window *)window_void)&&
					 (window->scene_viewer_array) &&
					(scene_viewer=window->scene_viewer_array[window->current_pane]))
				{
					double rgba[4];
					cmzn_sceneviewer_get_background_colour_rgba(scene_viewer->core_scene_viewer, rgba);
					background_colour.red   = rgba[0];
					background_colour.green = rgba[1];
					background_colour.blue  = rgba[2];
					background_colour.alpha = rgba[3];
					image_field = cmzn_field_image_base_cast(
						Scene_viewer_get_background_image_field(scene_viewer->core_scene_viewer));
					Scene_viewer_get_background_texture_info(scene_viewer->core_scene_viewer,
						&(texture_placement[0]),&(texture_placement[1]),
						&(texture_placement[2]),&(texture_placement[3]),
						&undistort_on,&max_pixels_per_polygon);
				}
				else
				{
					scene_viewer = 0;
					texture_placement[0]=texture_placement[1]=
						texture_placement[2]=texture_placement[3]=0.0;
					undistort_on=0;
					max_pixels_per_polygon=0.0;
				}
				undistort_flag=0;
				no_undistort_flag=0;
				all_panes_flag=0;
				option_table = CREATE(Option_table)();
				/* all_panes */
				Option_table_add_char_flag_entry(option_table, "all_panes",
					&all_panes_flag);
				/* colour */
				Option_table_add_entry(option_table, "colour",
					&background_colour, NULL, set_Colour);
				/* max_pixels_per_polygon */
				Option_table_add_double_entry(option_table, "max_pixels_per_polygon",
					&max_pixels_per_polygon);
				/* texture */
				Option_table_add_entry(option_table, "texture",
					&image_field, modify_graphics_window_data->root_region, set_image_field);
				/* tex_placement */
				Option_table_add_double_vector_with_help_entry(option_table, "tex_placement",
					texture_placement, &texture_placement_data);
				/* undistort_texture/no_undistort_texture */
				undistort_option_table = CREATE(Option_table)();
				Option_table_add_char_flag_entry(undistort_option_table, "undistort_texture",
					&undistort_flag);
				Option_table_add_char_flag_entry(undistort_option_table, "no_undistort_texture",
					&no_undistort_flag);
				Option_table_add_suboption_table(option_table, undistort_option_table);
				return_code=Option_table_multi_parse(option_table, state);
				if (return_code)
				{
					if (scene_viewer)
					{
						return_code=1;
						if (all_panes_flag)
						{
							first_pane=0;
							last_pane=window->number_of_scene_viewers-1;
						}
						else
						{
							first_pane=last_pane=window->current_pane;
						}
						if (undistort_flag&&no_undistort_flag)
						{
							display_message(ERROR_MESSAGE,
								"modify_Graphics_window_background. "
								"Only one of undistort_texture|no_undistort_texture");
						}
						else
						{
							if (undistort_flag)
							{
								undistort_on=1;
							}
							else if (no_undistort_flag)
							{
								undistort_on=0;
							}
						}
						for (pane_no=first_pane;pane_no <= last_pane;pane_no++)
						{
							cmzn_sceneviewer *sceneviewer = window->scene_viewer_array[pane_no]->core_scene_viewer;
							cmzn_sceneviewer_begin_change(sceneviewer);
							const double rgba[4] =
								{ background_colour.red, background_colour.green, background_colour.blue, background_colour.alpha };
							cmzn_sceneviewer_set_background_colour_rgba(sceneviewer, rgba);
							cmzn_field_image_id image = cmzn_field_cast_image(image_field);
							Scene_viewer_set_background_image_field(sceneviewer, image);
							if ((texture_placement[2] == 0.0) && (texture_placement[3] == 0.0))
							{
								if (image_field)
								{
									/* Get the default size from the texture itself */
									Texture_get_original_size(cmzn_field_image_get_texture(image),
										&pixelsx, &pixelsy, &pixelsz);
									texture_placement[2] = pixelsx;
									texture_placement[3] = pixelsy;
									Scene_viewer_set_background_texture_info(sceneviewer,
										texture_placement[0],texture_placement[1],
										texture_placement[2],texture_placement[3],
										undistort_on,max_pixels_per_polygon);
								}
							}
							else
							{
								Scene_viewer_set_background_texture_info(sceneviewer,
									texture_placement[0],texture_placement[1],
									texture_placement[2],texture_placement[3],
									undistort_on,max_pixels_per_polygon);
							}
							cmzn_field_image_destroy(&image);
							cmzn_sceneviewer_end_change(sceneviewer);
						}
						Graphics_window_update_now(window);
					}
					else
					{
						display_message(ERROR_MESSAGE,"modify_Graphics_window_background. "
							"Missing or invalid scene_viewer");
						display_parse_state_location(state);
						return_code=0;
					}
				}
				DESTROY(Option_table)(&option_table);
				if (image_field)
				{
					DEACCESS(Computed_field)(&image_field);
				}
			}
			else
			{
				display_message(WARNING_MESSAGE,
					"Missing window background modifications");
				display_parse_state_location(state);
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"modify_Graphics_window_background.  "
				"Missing modify_graphics_window_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"modify_Graphics_window_background.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* modify_Graphics_window_background */


#if defined (WX_USER_INTERFACE)

int Graphics_window_bring_up_time_editor_wx(Graphics_window *window, void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 26 October 2007

DESCRIPTION :
Bring up the time editor, WX_USER_INTERFACE only.
==============================================================================*/
{
	 int return_code;

	 ENTER(Graphics_window_set_time_settings_wx);
	 USE_PARAMETER(dummy_void);
	 if (window)
	 {
			window->time_editor_togglebutton->SetValue(1);
			window->right_panel->Freeze();
			window->time_editor_panel->Show(true);
			window->right_panel->Layout();
			window->right_panel->Update();
			window->right_panel->Thaw();
			return_code = 1;
	 }
	 else
	 {
			return_code = 0;
	 }

	 LEAVE;
	 return (return_code);
}

int Graphics_window_update_time_settings_wx(Graphics_window *window, void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 26 October 2007

DESCRIPTION :
Change the values on the wx interface if different.
==============================================================================*/
{
	 int return_code = 0;
	 double time;
	 wxString text_entry;

	 ENTER(Graphics_window_set_time_settings_wx);
	 USE_PARAMETER(dummy_void);

	 if (window->wx_graphics_window)
	 {
			return_code = 1;
			time=window->time_keeper_app->getSpeed();
			text_entry.Printf(_T("%f"),time);
			window->time_framerate_text_ctrl->ChangeValue(text_entry);

			time = window->time_step;
			text_entry.Printf(_T("%f"),time);

			window->time_step_size_text_ctrl->ChangeValue(text_entry);
			window->time_play_every_frame_checkbox->SetValue(
				window->time_keeper_app->getPlayEveryFrame());
			return_code = 1;
	 }
	 LEAVE;

	 return (return_code);
}

class wxGraphicsWindow : public wxFrame
{
	 Graphics_window *graphics_window;
	 wxBitmapButton *last_button;
	 wxChoice *view_options;
	 wxFrame *Redrawwindow;
	 wxChoice *up_view_options;
	 wxButton *front_view_options;
	 wxString front_choices;
	 wxScrolledWindow *leftpanel, *toolscrolledwindow;
	 int wx_ortho_up_axis;
	 int wx_ortho_front_axis;
	 int location;
	 int up_choices;
	 int choices;
	 wxPanel *time_editor_panel;
	 DEFINE_MANAGER_CLASS(cmzn_scenefilter);
	 Managed_object_chooser<cmzn_scenefilter,MANAGER_CLASS(cmzn_scenefilter)>
	 *graphics_window_filter_chooser;
	 wxRegionChooser *region_chooser;
public:

	 wxGraphicsWindow(Graphics_window *graphics_window):
			graphics_window(graphics_window)
	 {
			wxXmlInit_graphics_window();
			graphics_window->wx_graphics_window =  (wxGraphicsWindow *)NULL;
			wxXmlResource::Get()->LoadFrame(this,
			   (wxWindow *)NULL, _T("CmguiGraphicsWindow"));
			this->SetIcon(cmiss_icon_xpm);
			last_button = (wxBitmapButton*)NULL;
			time_editor_panel = NULL;
			wxPanel *graphics_window_filter_chooser_panel =
				 XRCCTRL(*this, "GraphicsWindowFilterChooserPanel", wxPanel);
			cmzn_scenefilter_id filter = cmzn_scenefiltermodule_get_default_scenefilter(graphics_window->filter_module);
			graphics_window_filter_chooser =
				 new Managed_object_chooser<cmzn_scenefilter, MANAGER_CLASS(cmzn_scenefilter)>
				 (graphics_window_filter_chooser_panel, filter,
					 cmzn_scenefiltermodule_get_manager(graphics_window->filter_module),
						(MANAGER_CONDITIONAL_FUNCTION(cmzn_scenefilter) *)NULL, (void *)NULL,
						graphics_window->user_interface);
			cmzn_scenefilter_destroy(&filter);
			Callback_base< cmzn_scenefilter* > *graphics_window_filter_callback =
				 new Callback_member_callback< cmzn_scenefilter*,
				 wxGraphicsWindow, int (wxGraphicsWindow::*)(cmzn_scenefilter *) >
				 (this, &wxGraphicsWindow::graphics_window_filter_callback);
			graphics_window_filter_chooser->set_callback(graphics_window_filter_callback);
			wxPanel *graphics_window_region_chooser_panel =
				 XRCCTRL(*this, "GraphicsWindowRegionChooserPanel", wxPanel);
			char *initial_path;
		   initial_path = cmzn_region_get_root_region_path();
		   region_chooser = new wxRegionChooser(graphics_window_region_chooser_panel,
			graphics_window->root_region, initial_path);
			DEALLOCATE(initial_path);
			Callback_base< cmzn_region* > *graphics_window_region_callback =
				 new Callback_member_callback< cmzn_region*,
				 wxGraphicsWindow, int (wxGraphicsWindow::*)(cmzn_region *) >
				 (this, &wxGraphicsWindow::graphics_window_region_callback);
			region_chooser->set_callback(graphics_window_region_callback);

			XRCCTRL(*this, "GraphicsWindowTimeTextCtrl", wxTextCtrl)->Connect(wxEVT_KILL_FOCUS,
				wxCommandEventHandler(wxGraphicsWindow::OnTimeTextEntered), NULL, this);
			XRCCTRL(*this, "GraphicsWindowTimeFramerateTextCtrl", wxTextCtrl)->Connect(wxEVT_KILL_FOCUS,
				wxCommandEventHandler(wxGraphicsWindow::OnTimeFramerateTextEntered), NULL, this);
			XRCCTRL(*this, "GraphicsWindowTimeStepSizeTextCtrl", wxTextCtrl)->Connect(wxEVT_KILL_FOCUS,
				wxCommandEventHandler(wxGraphicsWindow::OnTimeStepSizeTextEntered), NULL, this);
	 };

	 wxGraphicsWindow()
	 {
	 };

	 ~wxGraphicsWindow()
	 {
			int pane_no;
			if (graphics_window->time_keeper_app)
				graphics_window->time_keeper_app->removeCallback(Graphics_window_time_keeper_app_callback,
					(void *)graphics_window);
			if (graphics_window->scene_viewer_array)
			{
				 /* close the Scene_viewer(s) */
				 for (pane_no=0;pane_no<graphics_window->number_of_scene_viewers;pane_no++)
				 {
						DESTROY(Scene_viewer_app)(&(graphics_window->scene_viewer_array[pane_no]));
				 }
				 DEALLOCATE(graphics_window->scene_viewer_array);
				 graphics_window->scene_viewer_array = NULL;
			}
			graphics_window->wx_graphics_window = NULL;
			if (graphics_window->graphics_window_manager)
			{
				REMOVE_OBJECT_FROM_MANAGER(Graphics_window)(graphics_window,
					graphics_window->graphics_window_manager);
			}
			delete graphics_window_filter_chooser;
			delete region_chooser;
	 };

int graphics_window_region_callback(cmzn_region *region)
/*******************************************************************************
LAST MODIFIED : 9 February 2007

DESCRIPTION :
Callback from wxChooser<Scene> when choice is made.
=================================================*/
{
	int pane_no;
	struct Scene_viewer_app *scene_viewer;

	if ((graphics_window->scene_viewer_array) &&
		(scene_viewer = graphics_window->scene_viewer_array[0]))
	{
		cmzn_scene_id scene = cmzn_region_get_scene(region);
		for (pane_no=0;pane_no<graphics_window->number_of_scene_viewers;
			pane_no++)
		{
			scene_viewer=graphics_window->scene_viewer_array[pane_no];
			if (scene)
			{
				cmzn_scene_destroy(&(graphics_window->scene));
				graphics_window->scene = cmzn_scene_access(scene);
				cmzn_sceneviewer_set_scene(scene_viewer->core_scene_viewer,scene);
				Graphics_window_update_now(graphics_window);
			}
		}
		cmzn_scene_destroy(&scene);
		return 1;
	}
	else
	{
		return 0;
	 }
}

int graphics_window_filter_callback(cmzn_scenefilter_id filter)
/*******************************************************************************
LAST MODIFIED : 9 February 2007

DESCRIPTION :
Callback from wxChooser<Scene> when choice is made.
=================================================*/
{
	int pane_no;
	struct Scene_viewer_app *scene_viewer;

	if ((graphics_window->scene_viewer_array) &&
		(scene_viewer = graphics_window->scene_viewer_array[0]))
	{
		for (pane_no=0;pane_no<graphics_window->number_of_scene_viewers;
			pane_no++)
		{
			scene_viewer=graphics_window->scene_viewer_array[pane_no];
			if (filter)
			{
				cmzn_sceneviewer_set_scenefilter(scene_viewer->core_scene_viewer,filter);
				Graphics_window_update_now(graphics_window);
			}
		}
		return 1;
	}
	else
	{
		return 0;
	 }
}

void graphics_window_set_region_chooser_selected_item(cmzn_region_id region)
{
	 region_chooser->set_region(region);
}

void graphics_window_set_filter_chooser_selected_item(cmzn_scenefilter_id filter)
{
	graphics_window_filter_chooser->set_object(filter);
}

   void OnViewallpressed(wxCommandEvent& event)
	 {
	USE_PARAMETER(event);
			if (Graphics_window_view_all(graphics_window))
			{
				 Graphics_window_update(graphics_window);
			}
	 }

	 void OnSaveaspressed(wxCommandEvent& event)
	 {
			enum Texture_storage_type storage;
			int force_onscreen, height, width;
			struct Cmgui_image *cmgui_image;
			struct Cmgui_image_information *cmgui_image_information;
			wxString file_name;
			wxString filepath;
			char*  filename;
			wxFileDialog *saveImage = new wxFileDialog (this,wxT("Save file"),wxT(""),wxT(""),
				 wxT("PNG files (*.png)|*.png|JPEG files (*.jpg)|*.jpg|SGI files (*.sgi)|*.sgi|TIF files (*.tiff)|*.tiff|BMP files (*.bmp)|*.bmp|GIF files (*.gif)|*.gif"),wxFD_SAVE,wxDefaultPosition);

	USE_PARAMETER(event);
			if (saveImage->ShowModal() == wxID_OK)
			{
				 file_name=saveImage->GetFilename();
				 filepath=saveImage->GetPath();
				 filename=duplicate_string(filepath.mb_str(wxConvUTF8));
#if !defined (__WXMSW__)
				 int filter_index;
				 if ((strstr(filename, ".png") == NULL) && (strstr(filename, ".jpg") == NULL) && (strstr(filename, ".sgi") == NULL) &&
						(strstr(filename, ".tiff")== NULL) && (strstr(filename, ".bmp")== NULL) && (strstr(filename, ".gif") == NULL) &&
						(strstr(filename, ".PNG") == NULL) && (strstr(filename, ".JPG") == NULL) && (strstr(filename, ".SGI") == NULL) &&
						(strstr(filename, ".TIFF")== NULL) && (strstr(filename, ".BMP")== NULL) && (strstr(filename, ".GIF") == NULL))
				 {
						filter_index=saveImage->GetFilterIndex();
						if (filter_index == 0)
						{
							 strcat (filename,".png");
						}
						else if (filter_index == 1)
						{
							 strcat (filename,".jpg");
						}
						else if (filter_index == 2)
						{
							 strcat (filename,".sgi");
						}
						else if (filter_index == 3)
						{
							 strcat (filename,".tiff");
						}
						else if (filter_index == 4)
						{
							 strcat (filename,".bmp");
						}
						else if (filter_index == 5)
						{
							 strcat (filename,".gif");
						}
				 }
#endif  /*!defined (__WXMSW__)*/

				 storage = TEXTURE_RGBA;
				 force_onscreen = 0;
				 width = 0;
				 height = 0;
				 cmgui_image = Graphics_window_get_image(graphics_window,
					force_onscreen, width, height, /*preferred_antialias*/8,
					/*preferred_transparency_layers*/0, storage);
				 if(cmgui_image)
				 {
						cmgui_image_information = CREATE(Cmgui_image_information)();
						Cmgui_image_information_add_file_name(cmgui_image_information,filename);
						Cmgui_image_write(cmgui_image, cmgui_image_information);
						DESTROY(Cmgui_image_information)(&cmgui_image_information);
						DESTROY(Cmgui_image)(&cmgui_image);
				 }
				 DEALLOCATE(filename);
			}
	 }

	 void OnViewOptionspressed(wxCommandEvent& event)
	 {
		USE_PARAMETER(event);
	  view_options = XRCCTRL(*this,"View", wxChoice);
	  up_view_options = XRCCTRL(*this,"UpViewOptions", wxChoice);
	  front_view_options = XRCCTRL(*this,"FrontViewOptions", wxButton);

	  choices = view_options->GetCurrentSelection();
	  if (choices == 0)
			{
				 front_view_options->Enable();
				 up_view_options->Enable();
				 Graphics_window_set_layout_mode(graphics_window,GRAPHICS_WINDOW_LAYOUT_2D);
			}
	  else if (choices == 1 )
			{
				 front_view_options->Disable();
				 up_view_options->Disable();
				 Graphics_window_set_layout_mode(graphics_window,GRAPHICS_WINDOW_LAYOUT_FREE_ORTHO);
			}
	  else if (choices == 2)
			{
				 front_view_options->Enable();
				 up_view_options->Enable();
				 Graphics_window_set_layout_mode(graphics_window,GRAPHICS_WINDOW_LAYOUT_FRONT_BACK);
			}
	  else if (choices == 3)
			{
				 front_view_options->Enable();
				 up_view_options->Enable();
				 Graphics_window_set_layout_mode(graphics_window,GRAPHICS_WINDOW_LAYOUT_FRONT_SIDE);
			}
	  else if (choices == 4)
			{
				 front_view_options->Enable();
				 up_view_options->Enable();
				 Graphics_window_set_layout_mode(graphics_window,GRAPHICS_WINDOW_LAYOUT_ORTHOGRAPHIC);
			}
	  else if (choices == 5)
			{
				 front_view_options->Disable();
				 up_view_options->Disable();
				 Graphics_window_set_layout_mode(graphics_window,GRAPHICS_WINDOW_LAYOUT_PSEUDO_3D);
			}
	  else if (choices == 6)
			{
				 front_view_options->Disable();
				 up_view_options->Disable();
				 Graphics_window_set_layout_mode(graphics_window,GRAPHICS_WINDOW_LAYOUT_TWO_FREE);
			}
	  else if (choices == 7)
			{
				 front_view_options->Disable();
				 up_view_options->Disable();
				 Graphics_window_set_layout_mode(graphics_window,GRAPHICS_WINDOW_LAYOUT_SIMPLE);
			}
	  else
			{
				 printf("OnViewOptionspressed error. Invalid argument.");
			}
			wxPanel *grid_panel = XRCCTRL(*this,"GridPanel", wxPanel);
			grid_panel->Freeze();
			grid_panel->Layout();
			grid_panel->Update();
			grid_panel->Thaw();
			graphics_window->GraphicsWindowTitle->SetSize(graphics_window->GraphicsWindowTitle->GetSize()+wxSize(0,1));
			graphics_window->GraphicsWindowTitle->SetSize(graphics_window->GraphicsWindowTitle->GetSize()-wxSize(0,1));
			graphics_window->GraphicsWindowTitle->Layout();
	 }

void OnPerspectivePressed(wxCommandEvent& event)
{
	USE_PARAMETER(event);

	enum Scene_viewer_projection_mode projection_mode = SCENE_VIEWER_PARALLEL;
	if (graphics_window->wx_perspective_button->GetValue())
	{
		projection_mode = SCENE_VIEWER_PERSPECTIVE;
	}
	if (Graphics_window_get_projection_mode(graphics_window, graphics_window->current_pane)
			!= projection_mode)
	{
		switch (graphics_window->layout_mode)
		{
			case GRAPHICS_WINDOW_LAYOUT_SIMPLE:
			case GRAPHICS_WINDOW_LAYOUT_ORTHOGRAPHIC:
			case GRAPHICS_WINDOW_LAYOUT_FREE_ORTHO:
			{
				Graphics_window_set_projection_mode(graphics_window,0,projection_mode);
			} break;
			case GRAPHICS_WINDOW_LAYOUT_FRONT_BACK:
			case GRAPHICS_WINDOW_LAYOUT_FRONT_SIDE:
			case GRAPHICS_WINDOW_LAYOUT_PSEUDO_3D:
			case GRAPHICS_WINDOW_LAYOUT_TWO_FREE:
			{
				Graphics_window_set_projection_mode(graphics_window,0,projection_mode);
				Graphics_window_set_projection_mode(graphics_window,1,projection_mode);
			} break;
			default:
			{
			} break;
		}
	}
}

void OnTimeEditorButtonPressed(wxCommandEvent& event)
{
	 ENTER(OnTimeEditorButtonPressed);

	USE_PARAMETER(event);
	 graphics_window->time_editor_panel->Show(graphics_window->time_editor_togglebutton->GetValue());
	 if (graphics_window->GraphicsWindowTitle)
	 {
			graphics_window->right_panel->Freeze();
			graphics_window->right_panel->Layout();
			graphics_window->right_panel->Update();
			graphics_window->right_panel->Thaw();
			graphics_window->GraphicsWindowTitle->SetSize(graphics_window->GraphicsWindowTitle->GetSize()+wxSize(0,1));
			graphics_window->GraphicsWindowTitle->SetSize(graphics_window->GraphicsWindowTitle->GetSize()-wxSize(0,1));
			graphics_window->GraphicsWindowTitle->Layout();
	 }

	 LEAVE;
}

	 void OnUpViewOptionspressed(wxCommandEvent& event)
	 {
		USE_PARAMETER(event);
	  wxString option[6] = { wxT("x"), wxT("y"), wxT("z"),wxT("-x"), wxT("-y"), wxT("-z")};
	  up_view_options = XRCCTRL(*this,"UpViewOptions", wxChoice);
	  front_view_options = XRCCTRL(*this,"FrontViewOptions", wxButton);
	  up_choices = up_view_options->GetCurrentSelection();
	  front_choices = front_view_options->GetLabel();
	  for (int n=0; n<6; n++)
			{
				 if (front_choices == option[n])
						location = n;
			}
	  if ((up_choices == location) || (up_choices == ((location+3) % 6)))
			{
				 front_view_options->SetLabel(option[(location+1) % 6]);
			}

	  front_choices = front_view_options->GetLabel();
	  wx_ortho_up_axis = axis_name_to_axis_number(option[up_choices].mb_str(wxConvUTF8));
	  wx_ortho_front_axis = axis_name_to_axis_number(front_choices.mb_str(wxConvUTF8));
	  Graphics_window_set_orthographic_axes(graphics_window,wx_ortho_up_axis, wx_ortho_front_axis);
	  Graphics_window_set_layout_mode(graphics_window,graphics_window->layout_mode);
	  Graphics_window_update(graphics_window);
	 }

	 void OnFrontViewOptionspressed(wxCommandEvent& event)
	 {
		USE_PARAMETER(event);
		wxString option[6] = { wxT("x"), wxT("y"), wxT("z"),wxT("-x"), wxT("-y"), wxT("-z")};
	  up_view_options = XRCCTRL(*this,"UpViewOptions", wxChoice);
	  front_view_options = XRCCTRL(*this,"FrontViewOptions", wxButton);
	  up_choices = up_view_options->GetCurrentSelection();
	  front_choices = front_view_options->GetLabel();
	  for (int n=0; n<6; n++) {
				 if (front_choices == option[n])
						location = n;
			}
	  {
				 if ((((location+1) % 6) == up_choices) || (((location+4) % 6) == up_choices))
						front_view_options->SetLabel(option[(location+2) % 6]);
				 else
						front_view_options->SetLabel(option[(location+1) % 6]);
	  }

	  front_choices = front_view_options->GetLabel();
	  wx_ortho_up_axis = axis_name_to_axis_number(option[up_choices].mb_str(wxConvUTF8));
	  wx_ortho_front_axis = axis_name_to_axis_number(front_choices.mb_str(wxConvUTF8));
	  Graphics_window_set_orthographic_axes(graphics_window,wx_ortho_up_axis,wx_ortho_front_axis);
	  Graphics_window_set_layout_mode(graphics_window,graphics_window->layout_mode);
	  Graphics_window_update(graphics_window);
	 }

	void InteractiveButtonSetClickedBMP(wxBitmapButton *button)
	{
		if (last_button != button)
		{
			wxString wx_test_string = button->GetName();
			const char *test_string = wx_test_string.mb_str(wxConvUTF8);
			if (strcmp("Transform tool", test_string) == 0)
			{
				wxBitmap interactive_clicked_bmp(Transform_tool_clicked_xpm);
				button->SetBitmapLabel(interactive_clicked_bmp);
			}
			else if (strcmp("Node tool", test_string) == 0)
			{
				wxBitmap interactive_clicked_bmp(Node_tool_clicked_xpm);
				button->SetBitmapLabel(interactive_clicked_bmp);
			}
			else if (strcmp("Data tool", test_string) == 0)
			{
				wxBitmap interactive_clicked_bmp(Data_tool_clicked_xpm);
				button->SetBitmapLabel(interactive_clicked_bmp);
			}
			else if (strcmp("Element tool", test_string) == 0)
			{
				wxBitmap interactive_clicked_bmp(Element_tool_clicked_xpm);
				button->SetBitmapLabel(interactive_clicked_bmp);
			}
#if defined (USE_OPENCASCADE)
			else if (strcmp("Cad tool", test_string) == 0)
			{
				wxBitmap interactive_clicked_bmp(Cad_tool_clicked_xpm);
				button->SetBitmapLabel(interactive_clicked_bmp);
			}
#endif /* defined (USE_OPENCASCADE) */
			else if (strcmp("Element point tool", test_string) == 0)
			{
				wxBitmap interactive_clicked_bmp(Element_point_tool_clicked_xpm);
				button->SetBitmapLabel(interactive_clicked_bmp);
			}
		}
	}

void GraphicsWindowSetInteractiveButtonSelected(wxBitmapButton *button, Interactive_tool *tool, Graphics_window *graphics_window)
{
	if (last_button != button)
	{
		InteractiveButtonSetClickedBMP(button);
		wxWindowList child_list = graphics_window->ToolPanel->GetChildren();
		wxWindowListNode *child = child_list.GetFirst();
		while (child)
		{
			child->GetData()->Hide();
			child = child->GetNext();
		}
		if (last_button)
		{
			wxString wx_test_string = last_button->GetName();
			const char *test_string = wx_test_string.mb_str(wxConvUTF8);
			if (strcmp("Transform tool", test_string) == 0)
			{
				wxBitmap interactive_unclicked_bmp(Transform_tool_unclicked_xpm);
				last_button->SetBitmapLabel(interactive_unclicked_bmp);
			}
			if (strcmp("Node tool", test_string) == 0)
			{
				wxBitmap interactive_unclicked_bmp(Node_tool_unclicked_xpm);
				last_button->SetBitmapLabel(interactive_unclicked_bmp);
			}
			else if (strcmp("Data tool", test_string) == 0)
			{
				wxBitmap interactive_unclicked_bmp(Data_tool_unclicked_xpm);
				last_button->SetBitmapLabel(interactive_unclicked_bmp);
			}
			else if (strcmp("Element tool", test_string) == 0)
			{
				wxBitmap interactive_unclicked_bmp(Element_tool_unclicked_xpm);
				last_button->SetBitmapLabel(interactive_unclicked_bmp);
			}
#if defined (USE_OPENCASCADE)
			else if (strcmp("Cad tool", test_string) == 0)
			{
				wxBitmap interactive_clicked_bmp(Cad_tool_unclicked_xpm);
				last_button->SetBitmapLabel(interactive_clicked_bmp);
			}
#endif /* defined (USE_OPENCASCADE) */
			else if (strcmp("Element point tool", test_string) == 0)
			{
				wxBitmap interactive_unclicked_bmp(Element_point_tool_unclicked_xpm);
				last_button->SetBitmapLabel(interactive_unclicked_bmp);
			}
		}
		last_button = button;
		Interactive_tool_bring_up_dialog(tool,graphics_window);
		Graphics_window_set_interactive_tool(graphics_window, tool);
	}
}

void OnSplitterPositionChanged(wxSplitterEvent &event)
{
	USE_PARAMETER(event);
	 toolscrolledwindow =XRCCTRL(*this, "ToolPanel", wxScrolledWindow);
	 toolscrolledwindow->SetSize(toolscrolledwindow->GetSize()+wxSize(0,1));
	 toolscrolledwindow->SetSize(toolscrolledwindow->GetSize()-wxSize(0,1));
}

void OnTimeSliderChanged( wxScrollEvent& event)
{
	 int value;
	 double time;
	 wxString time_string;
	 ENTER(OnTimeSliderChanged);

	USE_PARAMETER(event);
	 value = graphics_window->time_slider->GetValue();
	 time = (double)(((graphics_window->maximum_time - graphics_window->minimum_time) * (double)value)/ (double)1000) + graphics_window->minimum_time;
	 time_string.Printf(_T("%f"), time);
	 graphics_window->time_text_ctrl->ChangeValue(time_string);
	 if(graphics_window->time_keeper_app)
	 {
		 graphics_window->time_keeper_app->requestNewTime(time);
	 }

	 LEAVE;
}

void OnTimeTextEntered(wxCommandEvent& event)
{
	USE_PARAMETER(event);
	wxString wx_text_entry = graphics_window->time_text_ctrl->GetValue();
	const char *text_entry = wx_text_entry.mb_str(wxConvUTF8);
	if (text_entry)
	{
		double time;
		sscanf(text_entry, "%lg", &time);
		graphics_window->time_keeper_app->requestNewTime(time);
	}
	Graphics_window_time_keeper_app_callback(graphics_window->time_keeper_app,
		TIME_KEEPER_APP_NEW_TIME,(void *)graphics_window);
}

void OnTimeForwardPressed(wxCommandEvent& event)
{
	USE_PARAMETER(event);
	graphics_window->time_keeper_app->play(CMZN_TIMEKEEPER_PLAY_DIRECTION_FORWARD);
}

void OnTimeStopPressed(wxCommandEvent& event)
{
	USE_PARAMETER(event);
	graphics_window->time_keeper_app->stop();
}

void OnTimeBackwardPressed(wxCommandEvent& event)
{
	USE_PARAMETER(event);
	graphics_window->time_keeper_app->play(CMZN_TIMEKEEPER_PLAY_DIRECTION_REVERSE);
}

void OnTimeForwardByStepPressed(wxCommandEvent& event)
{
	USE_PARAMETER(event);
	double time = graphics_window->time_keeper_app->getTimeKeeper()->getTime();
	if ((time + graphics_window->time_step) < graphics_window->maximum_time)
	{
		graphics_window->time_keeper_app->requestNewTime(time + graphics_window->time_step);
	}
	else
	{
		graphics_window->time_keeper_app->requestNewTime(graphics_window->maximum_time);
	}
}

void OnTimeBackwardByStepPressed(wxCommandEvent& event)
{
	USE_PARAMETER(event);
	double time = graphics_window->time_keeper_app->getTimeKeeper()->getTime();
	if ((time - graphics_window->time_step) > graphics_window->minimum_time)
	{
		graphics_window->time_keeper_app->requestNewTime(time - graphics_window->time_step);
	}
	else
	{
		graphics_window->time_keeper_app->requestNewTime(graphics_window->minimum_time);
	}
}

void OnTimeFastForwardPressed(wxCommandEvent& event)
{
	USE_PARAMETER(event);
	double time = graphics_window->time_keeper_app->getTimeKeeper()->getTime();
	if ((time + 2.0*graphics_window->time_step) < graphics_window->maximum_time)
	{
		graphics_window->time_keeper_app->requestNewTime(time + 2.0*graphics_window->time_step);
	}
	else
	{
		graphics_window->time_keeper_app->requestNewTime(graphics_window->maximum_time);
	}
}

void OnTimeFastBackwardPressed(wxCommandEvent& event)
{
	USE_PARAMETER(event);
	double time = graphics_window->time_keeper_app->getTimeKeeper()->getTime();
	if ((time - 2.0*graphics_window->time_step) > graphics_window->minimum_time)
	{
		graphics_window->time_keeper_app->requestNewTime(time - 2.0*graphics_window->time_step);
	}
	else
	{
		graphics_window->time_keeper_app->requestNewTime(graphics_window->minimum_time);
	}
}

void OnTimeHidePressed(wxCommandEvent& event)
{
	USE_PARAMETER(event);
	graphics_window->time_editor_togglebutton->SetValue(0);
	graphics_window->time_editor_panel->Show(false);
	if (graphics_window->GraphicsWindowTitle)
	{
		graphics_window->GraphicsWindowTitle->SetSize(graphics_window->GraphicsWindowTitle->GetSize()+wxSize(0,1));
		graphics_window->GraphicsWindowTitle->SetSize(graphics_window->GraphicsWindowTitle->GetSize()-wxSize(0,1));
		graphics_window->GraphicsWindowTitle->Layout();
	}
}

void OnTimeFramerateTextEntered(wxCommandEvent& event)
{
	USE_PARAMETER(event);
	wxString wx_text_entry = graphics_window->time_framerate_text_ctrl->GetValue();
	const char *text_entry = wx_text_entry.mb_str(wxConvUTF8);
	if (text_entry)
	{
		double framerate;
		sscanf(text_entry, "%lg", &framerate);
		graphics_window->time_keeper_app->setSpeed(framerate);
	}
	Graphics_window_update_time_settings_wx(graphics_window, (void *)NULL);
}

void OnTimeStepSizeTextEntered(wxCommandEvent& event)
{
	USE_PARAMETER(event);
	wxString wx_text_entry = graphics_window->time_step_size_text_ctrl->GetValue();
	const char *text_entry = wx_text_entry.mb_str(wxConvUTF8);
	if (text_entry)
	{
		double timestep;
		sscanf(text_entry, "%lg", &timestep);
		graphics_window->time_step = timestep;
		graphics_window->time_keeper_app->setTimeStep(graphics_window->time_step);
	}
	Graphics_window_update_time_settings_wx(graphics_window, (void *)NULL);
}

void OnEveryFrameChecked(wxCommandEvent& event)
{
	USE_PARAMETER(event);
	if (graphics_window->time_play_every_frame_checkbox->GetValue())
	{
		graphics_window->time_keeper_app->setPlayEveryFrame();
	}
	else
	{
		graphics_window->time_keeper_app->setPlaySkipFrames();
	}
	Graphics_window_update_time_settings_wx(graphics_window, (void *)NULL);
}

 DECLARE_EVENT_TABLE();
};

BEGIN_EVENT_TABLE(wxGraphicsWindow, wxFrame)
	 EVT_BUTTON(XRCID("Button1"),wxGraphicsWindow::OnViewallpressed)
	 EVT_BUTTON(XRCID("Button2"),wxGraphicsWindow::OnSaveaspressed)
	 EVT_CHECKBOX(XRCID("PerspectiveButton"), wxGraphicsWindow::OnPerspectivePressed)
	 EVT_TOGGLEBUTTON(XRCID("TimeEditorToggleButton"), wxGraphicsWindow::OnTimeEditorButtonPressed)
	 EVT_CHOICE(XRCID("View"),wxGraphicsWindow::OnViewOptionspressed)
	 EVT_CHOICE(XRCID("UpViewOptions"),wxGraphicsWindow::OnUpViewOptionspressed)
	 EVT_BUTTON(XRCID("FrontViewOptions"),wxGraphicsWindow::OnFrontViewOptionspressed)
	 EVT_SPLITTER_SASH_POS_CHANGED(XRCID("GraphicsWindowSplitter"),wxGraphicsWindow::OnSplitterPositionChanged)
	 EVT_COMMAND_SCROLL(XRCID("GraphicsWindowTimeSlider"),wxGraphicsWindow::OnTimeSliderChanged)
	 EVT_TEXT_ENTER(XRCID("GraphicsWindowTimeTextCtrl"),wxGraphicsWindow::OnTimeTextEntered)
	 EVT_BUTTON(XRCID("GraphicsWindowForward"), wxGraphicsWindow::OnTimeForwardPressed)
	 EVT_BUTTON(XRCID("GraphicsWindowStop"), wxGraphicsWindow::OnTimeStopPressed)
	 EVT_BUTTON(XRCID("GraphicsWindowBackward"), wxGraphicsWindow::OnTimeBackwardPressed)
	 EVT_BUTTON(XRCID("GraphicsWindowForwardByStep"), wxGraphicsWindow::OnTimeForwardByStepPressed)
	 EVT_BUTTON(XRCID("GraphicsWindowBackwardByStep"), wxGraphicsWindow::OnTimeBackwardByStepPressed)
	 EVT_BUTTON(XRCID("GraphicsWindowFastForward"), wxGraphicsWindow::OnTimeFastForwardPressed)
	 EVT_BUTTON(XRCID("GraphicsWindowFastBackward"), wxGraphicsWindow::OnTimeFastBackwardPressed)
	 EVT_BUTTON(XRCID("GraphicsWindowHideTimeBitmapButton"), wxGraphicsWindow::OnTimeHidePressed)
	 EVT_TEXT_ENTER(XRCID("GraphicsWindowTimeFramerateTextCtrl"),wxGraphicsWindow::OnTimeFramerateTextEntered)
	 EVT_TEXT_ENTER(XRCID("GraphicsWindowTimeStepSizeTextCtrl"),wxGraphicsWindow::OnTimeStepSizeTextEntered)
	 EVT_CHECKBOX(XRCID("GraphcisWindowTimePlayEveryFrameCheckBox"), wxGraphicsWindow::OnEveryFrameChecked)
END_EVENT_TABLE()

class wxInteractiveToolButton : public wxBitmapButton
{
  Interactive_tool *tool;
  Graphics_window *graphics_window;

public:

  wxInteractiveToolButton(Interactive_tool *tool, Graphics_window *graphics_window) :
	tool(tool), graphics_window(graphics_window)
	 {
	 };

	 wxInteractiveToolButton()
	 {
	 };

  ~wxInteractiveToolButton()
  {
  };

  void OnInteractiveButtonPressed(wxCommandEvent& event)
  {
		USE_PARAMETER(event);
	graphics_window->wx_graphics_window->GraphicsWindowSetInteractiveButtonSelected
	  (this, tool,graphics_window);
   }

};

static int add_interactive_tool_to_wx_toolbar(struct Interactive_tool *interactive_tool,
	void *graphics_window_void)
{
	Graphics_window *window = static_cast<Graphics_window*>(graphics_window_void);
	wxPanel *panel = window->interactive_toolbar_panel;
	//	 wxSizer *sizer = panel->GetSizer();
	wxInteractiveToolButton *button = new wxInteractiveToolButton(interactive_tool, window);
	const char *interactive_tool_name = Interactive_tool_get_display_name(interactive_tool);
	int return_int = 0;

	if (window->grid_field == NULL)
	{
			window->grid_field = new wxGridSizer(0,3,1,1);
	}
	if (strcmp("Transform tool", interactive_tool_name) == 0)
	{
		wxBitmap interactive_unclicked_bmp(Transform_tool_unclicked_xpm);
		button->Create(panel, /*id*/-1, interactive_unclicked_bmp);
		button->SetName(wxT("Transform tool"));
		window->transform_tool_button = button;
		window->grid_field->Add(button);
		return_int = 1;
	}
	else if (strcmp("Node tool", interactive_tool_name) == 0)
	{
		wxBitmap interactive_unclicked_bmp(Node_tool_unclicked_xpm);
		button->Create(panel, /*id*/-1, interactive_unclicked_bmp);
		button->SetName(wxT("Node tool"));
		window->node_tool_button = button;
		window->grid_field->Add(button);
		return_int = 1;
	}
	else if (strcmp("Data tool", interactive_tool_name) == 0)
	{
		wxBitmap interactive_unclicked_bmp(Data_tool_unclicked_xpm);
		button->Create(panel, /*id*/-1, interactive_unclicked_bmp);
		button->SetName(wxT("Data tool"));
		window->data_tool_button = button;
		window->grid_field->Add(button);
		return_int = 1;
	}
	else if (strcmp("Element tool", interactive_tool_name) == 0)
	{
		wxBitmap interactive_unclicked_bmp(Element_tool_unclicked_xpm);
		button->Create(panel, /*id*/-1, interactive_unclicked_bmp);
		button->SetName(wxT("Element tool"));
		window->element_tool_button = button;
		window->grid_field->Add(button);
		return_int = 1;
	}
#if defined (USE_OPENCASCADE)
	else if (strcmp("Cad tool", interactive_tool_name) == 0)
	{
		wxBitmap interactive_unclicked_bmp(Cad_tool_unclicked_xpm);
		button->Create(panel, /*id*/-1, interactive_unclicked_bmp);
		button->SetName(wxT("Cad tool"));
		window->cad_tool_button = button;
		window->grid_field->Add(button);
		return_int = 1;
	}
#endif /* defined (USE_OPENCASCADE) */
	else if (strcmp("Element point tool", interactive_tool_name) == 0)
	{
		wxBitmap interactive_unclicked_bmp(Element_point_tool_unclicked_xpm);
		button->Create(panel, /*id*/-1, interactive_unclicked_bmp);
		button->SetName(wxT("Element point tool"));
		window->element_point_tool_button = button;
		window->grid_field->Add(button);
		return_int = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			 "add_interactive_tool_to_wx_toolbar.  Could not find bitmap for the interactive tool");
		return_int = 0;
	}
	if (window->grid_field != NULL)
	{
		panel->SetSizer(window->grid_field);
		window->grid_field->SetSizeHints(panel);
		window->grid_field->Layout();
		panel->Layout();
	}
	if (return_int ==1)
	{
		DEALLOCATE(interactive_tool_name);

		if (Interactive_tool_is_Transform_tool(interactive_tool))
		{
			wxBitmap interactive_clicked_bmp(Transform_tool_clicked_xpm);
			button->SetBitmapLabel(interactive_clicked_bmp);
			window->wx_graphics_window->GraphicsWindowSetInteractiveButtonSelected
				(button, interactive_tool, window);
		}
		button->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
			wxCommandEventHandler(wxInteractiveToolButton::OnInteractiveButtonPressed));
		return (1);
	}
	else
	{
		return (0);
	}
}

#endif /* defined (WX_USER_INTERFACE) */


static int modify_Graphics_window_image(struct Parse_state *state,
	void *window_void,void *modify_graphics_window_data_void)
/*******************************************************************************
LAST MODIFIED : 18 November 1998

DESCRIPTION :
Parser commands for setting the scene and how it is displayed (time, light model
etc.) in all panes of the <window>.
==============================================================================*/
{
	int return_code;
	struct Modify_graphics_window_data *modify_graphics_window_data;
	struct Option_table *option_table;

	ENTER(modify_Graphics_window_image);
	if (state)
	{
		modify_graphics_window_data=(struct Modify_graphics_window_data *)
			modify_graphics_window_data_void;
		if (modify_graphics_window_data)
		{
			if (state->current_token)
			{
				/* get defaults from scene viewer for first pane of window */
				Graphics_window *window = static_cast<Graphics_window *>(window_void);
				cmzn_scene *scene = 0;
				cmzn_sceneviewer *first_sceneviewer = 0;
				int lightingLocalViewer = 0;
				int lightingTwoSided = 1;
				if (window)
				{
					if ((window->scene_viewer_array) && (window->scene_viewer_array[0]))
					{
						first_sceneviewer = window->scene_viewer_array[0]->core_scene_viewer;
						if (first_sceneviewer)
						{
							lightingLocalViewer = first_sceneviewer->isLightingLocalViewer() ? 1 : 0;
							lightingTwoSided = first_sceneviewer->isLightingTwoSided() ? 1 : 0;
						}
					}
					scene=cmzn_scene_access(window->scene);
				}
				char *dummy_light_model_name = 0;
				cmzn_scenefilter_id filter = 0;
				if (first_sceneviewer)
					filter = cmzn_sceneviewer_get_scenefilter(first_sceneviewer);
				cmzn_light *light_to_add = 0;
				cmzn_light *light_to_remove = 0;
				struct Set_vector_with_help_data
					rotate_command_data = {4, " AXIS_X AXIS_Y AXIS_Z ANGLE", 0};
				double rotate_angle = 0.0;
				double rotate_axis[3], rotate_data[4];
				char update_flag = 0;
				char view_all_flag = 0;
				option_table = CREATE(Option_table)();
				/* add_light */
				Option_table_add_entry(option_table, "add_light", &light_to_add,
					modify_graphics_window_data->light_manager, set_cmzn_light);
				/* filter */
				Option_table_add_entry(option_table, "filter", &filter,
					modify_graphics_window_data->filter_module, set_cmzn_scenefilter);
				/* light_model - obsolete; name is consumed */
				Option_table_add_string_entry(option_table, "light_model",
					&dummy_light_model_name, " NAME (OBSOLETE, IGNORED)");
				/* local_viewer_lighting|infinite_viewer_lighting */
				Option_table_add_switch(option_table, "local_viewer_lighting",
					"infinite_viewer_lighting", &lightingLocalViewer);
				/* remove_light */
				Option_table_add_entry(option_table, "remove_light", &light_to_remove,
					modify_graphics_window_data->light_manager, set_cmzn_light);
				/* rotate */
				Option_table_add_double_vector_with_help_entry(option_table, "rotate",
					rotate_data, &rotate_command_data);
				/* scene */
				Option_table_add_entry(option_table, "scene", &scene,
					modify_graphics_window_data->root_region, set_Scene);
				/* two_sided_lighting|one_sided_lighting */
				Option_table_add_switch(option_table, "two_sided_lighting",
					"one_sided_lighting", &lightingTwoSided);
				/* update */
				Option_table_add_char_flag_entry(option_table, "update",
					&update_flag);
				/* view_all */
				Option_table_add_char_flag_entry(option_table, "view_all",
					&view_all_flag);
				return_code=Option_table_multi_parse(option_table, state);
				if (return_code)
				{
					if (first_sceneviewer)
					{
						return_code=1;
						if (light_to_add && cmzn_sceneviewer_has_light(first_sceneviewer, light_to_add))
						{
							/*display_message(WARNING_MESSAGE,"Light is already in window");*/
							cmzn_light_destroy(&light_to_add);
							light_to_add=(struct cmzn_light *)NULL;
						}
						if (light_to_remove &&
							!cmzn_sceneviewer_has_light(first_sceneviewer, light_to_remove))
						{
							display_message(WARNING_MESSAGE,
								"gfx modify window.  Cannot remove light that is not in window");
							cmzn_light_destroy(&light_to_remove);
							light_to_remove=(struct cmzn_light *)NULL;
						}
						if (rotate_command_data.set)
						{
							rotate_axis[0]=rotate_data[0];
							rotate_axis[1]=rotate_data[1];
							rotate_axis[2]=rotate_data[2];
							rotate_angle=rotate_data[3]*(PI/180.0);
						}
						/* set values for all panes */
						for (int pane_no = 0; pane_no < window->number_of_scene_viewers; ++pane_no)
						{
							cmzn_sceneviewer *sceneviewer = window->scene_viewer_array[pane_no]->core_scene_viewer;
							cmzn_sceneviewer_begin_change(sceneviewer);
							cmzn_sceneviewer_set_lighting_local_viewer(sceneviewer, (lightingLocalViewer) ? true : false);
							cmzn_sceneviewer_set_lighting_two_sided(sceneviewer, (lightingTwoSided) ? true : false);
							if (light_to_add)
								cmzn_sceneviewer_add_light(sceneviewer, light_to_add);
							if (light_to_remove)
								cmzn_sceneviewer_remove_light(sceneviewer, light_to_remove);
							if (rotate_command_data.set)
							{
								Scene_viewer_rotate_about_lookat_point(sceneviewer, rotate_axis, rotate_angle);
							}
							if (scene)
							{
								cmzn_sceneviewer_set_scene(sceneviewer, scene);
								cmzn_region_id region = cmzn_scene_get_region_internal(scene);
#if defined (WX_USER_INTERFACE)
								window->wx_graphics_window->
									 graphics_window_set_region_chooser_selected_item(region);
#endif
							}
							if (filter)
							{
								cmzn_sceneviewer_set_scenefilter(sceneviewer, filter);
								window->wx_graphics_window->
									graphics_window_set_filter_chooser_selected_item(filter);
							}
							cmzn_sceneviewer_end_change(sceneviewer);
						}
						if (scene)
						{
							/* maintain pointer to scene in graphics_window */
							cmzn_scene_destroy(&(window->scene));
							window->scene = cmzn_scene_access(scene);
						}
						if (view_all_flag)
						{
							Graphics_window_view_all(window);
						}
						/* redraw all active scene_viewers */
						Graphics_window_update_now(window);
					}
					else
					{
						display_message(ERROR_MESSAGE,"modify_Graphics_window_image. "
							"Missing or invalid window and/or scene viewer");
						display_parse_state_location(state);
						return_code=0;
					}
				}
				DESTROY(Option_table)(&option_table);
				if (filter)
				{
					cmzn_scenefilter_destroy(&filter);
				}
				if (scene)
				{
					cmzn_scene_destroy(&scene);
				}
				if (dummy_light_model_name)
					DEALLOCATE(dummy_light_model_name);
				if (light_to_add)
				{
					cmzn_light_destroy(&light_to_add);
				}
				if (light_to_remove)
				{
					cmzn_light_destroy(&light_to_remove);
				}
			}
			else
			{
				display_message(WARNING_MESSAGE,"Missing window image modifications");
				display_parse_state_location(state);
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"modify_Graphics_window_image.  "
				"Missing modify_graphics_window_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"modify_Graphics_window_image.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* modify_Graphics_window_image */

static int modify_Graphics_window_layout(struct Parse_state *state,
	void *graphics_window_void,void *user_data_void)
/*******************************************************************************
LAST MODIFIED : 28 June 2000

DESCRIPTION :
Parser commands for setting the scene and how it is displayed (time, light model
etc.) in all panes of the <window>.
==============================================================================*/
{
	const char **valid_strings, *layout_mode_string;
	double eye_spacing;
	enum Graphics_window_layout_mode layout_mode,old_layout_mode;
	int height,number_of_valid_strings,old_height,old_width,return_code,width;
	struct Graphics_window *graphics_window;
	struct Graphics_window_ortho_axes ortho_axes;
	struct Option_table *option_table;

	ENTER(modify_Graphics_window_layout);
	USE_PARAMETER(user_data_void);
	if (state)
	{
		if (state->current_token)
		{
			/* get defaults from scene_viewer for first pane of window */
			graphics_window=(struct Graphics_window *)graphics_window_void;
			if (graphics_window)
			{
				layout_mode=Graphics_window_get_layout_mode(graphics_window);
				Graphics_window_get_viewing_area_size(graphics_window,&width,&height);
				ortho_axes.up=graphics_window->ortho_up_axis;
				ortho_axes.front=graphics_window->ortho_front_axis;
				eye_spacing=graphics_window->eye_spacing;
			}
			else
			{
				layout_mode=GRAPHICS_WINDOW_LAYOUT_SIMPLE;
				width=0;
				height=0;
				ortho_axes.up=0;
				ortho_axes.front=0;
				eye_spacing=0.0;
			}
			old_width=width;
			old_height=height;
			old_layout_mode=layout_mode;

			option_table=CREATE(Option_table)();
			/* layout_mode */
			layout_mode_string=(char *)Graphics_window_layout_mode_string(layout_mode);
			valid_strings=Graphics_window_layout_mode_get_valid_strings(
				&number_of_valid_strings);
			Option_table_add_enumerator(option_table,number_of_valid_strings,
				valid_strings,&layout_mode_string);
			DEALLOCATE(valid_strings);
			/* eye_spacing */
			Option_table_add_entry(option_table,"eye_spacing",&eye_spacing,
				NULL,set_double);
			/* height */
			Option_table_add_entry(option_table,"height",&height,
				NULL,set_int_non_negative);
			/* ortho_axes */
			Option_table_add_entry(option_table,"ortho_axes",&ortho_axes,
				NULL,set_Graphics_window_ortho_axes);
			/* width */
			Option_table_add_entry(option_table,"width",&width,
				NULL,set_int_non_negative);
			return_code=Option_table_multi_parse(option_table,state);
			if (return_code)
			{
				if (graphics_window)
				{
					if ((ortho_axes.up != graphics_window->ortho_up_axis)||
						(ortho_axes.front != graphics_window->ortho_front_axis))
					{
						Graphics_window_set_orthographic_axes(graphics_window,
							ortho_axes.up,ortho_axes.front);
						/* always force layout to be reset */
						old_layout_mode=GRAPHICS_WINDOW_LAYOUT_MODE_INVALID;
					}
					if (eye_spacing != graphics_window->eye_spacing)
					{
						Graphics_window_set_eye_spacing(graphics_window,eye_spacing);
						/* always force layout to be reset */
						old_layout_mode=GRAPHICS_WINDOW_LAYOUT_MODE_INVALID;
					}
					layout_mode=
						Graphics_window_layout_mode_from_string(layout_mode_string);
					if (layout_mode != old_layout_mode)
					{
						Graphics_window_set_layout_mode(graphics_window,layout_mode);
					}
					if ((width != old_width) || (height != old_height))
					{
						Graphics_window_set_viewing_area_size(graphics_window,
							width,height);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"modify_Graphics_window_layout.  Missing graphics_window");
					return_code=0;
				}
			}
			DESTROY(Option_table)(&option_table);
		}
		else
		{
			display_message(WARNING_MESSAGE,"Missing window layout modifications");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"modify_Graphics_window_layout.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* modify_Graphics_window_layout */

/***************************************************************************//**
 * Deprecated command: reports overlay scene no longer supported.
 */
static int modify_Graphics_window_overlay(struct Parse_state *state,
	void *window_void,void *modify_graphics_window_data_void)
{
	int return_code = 0;

	ENTER(modify_Graphics_window_overlay);
	USE_PARAMETER(window_void);
	USE_PARAMETER(modify_graphics_window_data_void);
	if (state)
	{
		display_message(INFORMATION_MESSAGE, "Graphics window overlay scene is no longer supported. "
			"Individual graphics are now drawn in overlay by choosing a window-relative coordinate system.\n");
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"modify_Graphics_window_overlay.  Missing state");
	}
	LEAVE;

	return (return_code);
} /* modify_Graphics_window_overlay */

int Graphics_window_set_antialias_mode(struct Graphics_window *graphics_window,
	int antialias_mode)
/*******************************************************************************
LAST MODIFIED : 13 June 2000

DESCRIPTION :
Sets the number of times the images is oversampled to antialias the image. Only
certain values are supported 0/1 = off, 2, 4 & 8 are on.
==============================================================================*/
{
	int pane_no,return_code;

	ENTER(Graphics_window_set_antialias_mode);
	if (graphics_window && graphics_window->scene_viewer_array)
	{
		if (1==antialias_mode)
		{
			antialias_mode=0;
		}
		return_code=1;
		for (pane_no=0;(pane_no<graphics_window->number_of_scene_viewers)&&return_code;
			pane_no++)
		{
			return_code = cmzn_sceneviewer_set_antialias_sampling(
				graphics_window->scene_viewer_array[pane_no]->core_scene_viewer,antialias_mode);
		}
		if (return_code)
		{
			graphics_window->antialias_mode=antialias_mode;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_window_set_antialias_mode.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;
	return (return_code);
} /* Graphics_window_set_antialias_mode */

int Graphics_window_set_depth_of_field(struct Graphics_window *graphics_window,
	double depth_of_field, double focal_depth)
/*******************************************************************************
LAST MODIFIED : 5 December 2006

DESCRIPTION :
==============================================================================*/
{
	int pane_no,return_code;

	ENTER(Graphics_window_set_antialias_mode);
	if (graphics_window && graphics_window->scene_viewer_array)
	{
		return_code=1;
		for (pane_no=0;(pane_no<graphics_window->number_of_scene_viewers)&&return_code;
			pane_no++)
		{
			return_code=Scene_viewer_set_depth_of_field(
				graphics_window->scene_viewer_array[pane_no]->core_scene_viewer,
				depth_of_field, focal_depth);
		}
		if (return_code)
		{
			graphics_window->depth_of_field=depth_of_field;
			graphics_window->focal_depth=focal_depth;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_window_set_depth_of_field.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;
	return (return_code);
} /* Graphics_window_set_depth_of_field */

int Graphics_window_set_perturb_lines(struct Graphics_window *graphics_window,
	int perturb_lines)
/*******************************************************************************
LAST MODIFIED : 13 June 2000

DESCRIPTION :
Sets if the <graphics_window> perturbs lines or not, using <perturb_lines>
(1==TRUE,0==FALSE)
==============================================================================*/
{
	int pane_no,return_code;

	ENTER(Graphics_window_set_perturb_lines);
	if (graphics_window && graphics_window->scene_viewer_array)
	{
		return_code=1;
		for (pane_no=0;(pane_no<graphics_window->number_of_scene_viewers)&&return_code;
			pane_no++)
		{
			return_code = (CMZN_OK == cmzn_sceneviewer_set_perturb_lines_flag(
				graphics_window->scene_viewer_array[pane_no]->core_scene_viewer, (0 != perturb_lines)));
		}
		if (return_code)
		{
			graphics_window->perturb_lines=perturb_lines;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_window_set_perturb_lines.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;
	return (return_code);
} /* Graphics_window_set_perturb_lines */

int Graphics_window_set_blending_mode(struct Graphics_window *graphics_window,
	enum cmzn_sceneviewer_blending_mode blending_mode)
/*******************************************************************************
LAST MODIFIED : 16 April 2003

DESCRIPTION :
Sets the <blending_mode> used by the <graphics_window>.
==============================================================================*/
{
	int pane_no,return_code;

	ENTER(Graphics_window_set_blending_mode);
	if (graphics_window && graphics_window->scene_viewer_array)
	{
		return_code=1;
		for (pane_no=0;(pane_no<graphics_window->number_of_scene_viewers)&&return_code;
			pane_no++)
		{
			return_code = cmzn_sceneviewer_set_blending_mode(
				graphics_window->scene_viewer_array[pane_no]->core_scene_viewer,blending_mode);
		}
		if (return_code)
		{
			graphics_window->blending_mode=blending_mode;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_window_set_blending_mode.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;
	return (return_code);
} /* Graphics_window_set_blending_mode */

static int modify_Graphics_window_set(struct Parse_state *state,
	void *window_void,void *modify_graphics_window_data_void)
/*******************************************************************************
LAST MODIFIED : 13 June 2000

DESCRIPTION :
Parser commands for setting simple parameters applicable to the whole <window>.
==============================================================================*/
{
	char fast_transparency_flag,slow_transparency_flag;
	const char *blending_mode_string,**valid_strings;
	double depth_of_field, focal_depth, std_view_angle;
	enum cmzn_sceneviewer_blending_mode blending_mode;
	enum cmzn_sceneviewer_transparency_mode transparency_mode;
	int antialias_mode,current_pane,i,number_of_tools,
		number_of_valid_strings,order_independent_transparency,pane_no,
		perturb_lines,redraw,return_code,transparency_layers = 0;
	struct Graphics_window *graphics_window;
	struct Interactive_tool *interactive_tool;
	struct Modify_graphics_window_data *modify_graphics_window_data;
	struct Option_table *antialias_option_table,*option_table,
		 *transparency_option_table;
#if defined (WX_USER_INTERFACE)
	char hide_time_editor_flag, show_time_editor_flag;
	struct Option_table *time_editor_option_table;
#endif /* defined (WX_USER_INTERFACE) */

	ENTER(modify_Graphics_window_set);
	if (state)
	{
		modify_graphics_window_data=(struct Modify_graphics_window_data *)
			modify_graphics_window_data_void;
		if (modify_graphics_window_data)
		{
			/* Keep the handle in case we need it sometime */
			USE_PARAMETER(modify_graphics_window_data);
			if (state->current_token)
			{
				/* get defaults from scene_viewer for first pane of window */
				graphics_window=(struct Graphics_window *)window_void;
				if (graphics_window)
				{
					current_pane=graphics_window->current_pane+1;
					depth_of_field=graphics_window->depth_of_field;
					focal_depth=graphics_window->focal_depth;
					std_view_angle=graphics_window->std_view_angle;
					interactive_tool=graphics_window->interactive_tool;
					antialias_mode=graphics_window->antialias_mode;
					perturb_lines=graphics_window->perturb_lines;
					blending_mode=graphics_window->blending_mode;
				}
				else
				{
					current_pane=1;
					depth_of_field = 0.0;
					focal_depth = 0.0;
					std_view_angle=40.0;
					interactive_tool=(struct Interactive_tool *)NULL;
					antialias_mode=0;
					perturb_lines=0;
					blending_mode = CMZN_SCENEVIEWER_BLENDING_MODE_NORMAL;
				}
				fast_transparency_flag = 0;
				slow_transparency_flag = 0;
				order_independent_transparency = 0;
#if defined (WX_USER_INTERFACE)
				hide_time_editor_flag=0;
				show_time_editor_flag=0;
#endif /* defined (WX_USER_INTERFACE) */
				option_table=CREATE(Option_table)();
				/* antialias/no_antialias */
				antialias_option_table=CREATE(Option_table)();
				Option_table_add_entry(antialias_option_table,"antialias",
					&antialias_mode,(void *)NULL,set_int_positive);
				Option_table_add_entry(antialias_option_table,"no_antialias",
					&antialias_mode,(void *)NULL,unset_int_switch);
				Option_table_add_suboption_table(option_table,antialias_option_table);
				/* blending mode */
				blending_mode_string =
					ENUMERATOR_STRING(cmzn_sceneviewer_blending_mode)(blending_mode);
				valid_strings = ENUMERATOR_GET_VALID_STRINGS(cmzn_sceneviewer_blending_mode)(
					&number_of_valid_strings,
					(ENUMERATOR_CONDITIONAL_FUNCTION(cmzn_sceneviewer_blending_mode) *)NULL,
					(void *)NULL);
				Option_table_add_enumerator(option_table, number_of_valid_strings,
					valid_strings, &blending_mode_string);
				DEALLOCATE(valid_strings);
				/* current_pane */
				Option_table_add_entry(option_table,"current_pane",
					&current_pane,(void *)NULL,set_int);
				/* depth_of_field */
				Option_table_add_entry(option_table,"depth_of_field",
					&depth_of_field,(void *)NULL,set_double);
				/* focal_depth */
				Option_table_add_entry(option_table,"focal_depth",
					&focal_depth,(void *)NULL,set_double);
				/* transform|other tools. tool_names not deallocated until later */
				const char *tool_name = 0;
				char **tool_names = interactive_tool_manager_get_tool_names(
					modify_graphics_window_data->interactive_tool_manager,
					&number_of_tools,interactive_tool,&tool_name);
				if (tool_names)
					Option_table_add_enumerator(option_table,number_of_tools,
						const_cast<const char **>(tool_names), &tool_name);
				/* perturb_lines|normal_lines */
				Option_table_add_switch(option_table,"perturb_lines","normal_lines",
					&perturb_lines);
				/* std_view_angle */
				Option_table_add_entry(option_table,"std_view_angle",
					&std_view_angle,(void *)NULL,set_double);
#if defined (WX_USER_INTERFACE)
				time_editor_option_table = CREATE(Option_table)();
				Option_table_add_entry(time_editor_option_table,"show_time_editor",
					 &show_time_editor_flag,(void *)NULL,set_char_flag);
				Option_table_add_entry(time_editor_option_table,"hide_time_editor",
					 &hide_time_editor_flag,(void *)NULL,set_char_flag);
				Option_table_add_suboption_table(option_table, time_editor_option_table);
#endif /* defined (WX_USER_INTERFACE) */
				/* fast_transparency|slow_transparency */
				transparency_option_table=CREATE(Option_table)();
				Option_table_add_entry(transparency_option_table,"fast_transparency",
					&fast_transparency_flag,(void *)NULL,set_char_flag);
				Option_table_add_entry(transparency_option_table,
					"order_independent_transparency",&order_independent_transparency,(void *)NULL,
					set_int_positive);
				Option_table_add_entry(transparency_option_table,"slow_transparency",
					&slow_transparency_flag,(void *)NULL,set_char_flag);
				Option_table_add_suboption_table(option_table,
					transparency_option_table);
				return_code=Option_table_multi_parse(option_table,state);
				if (return_code)
				{
					if ((1>current_pane)||(current_pane>graphics_window->number_of_panes))
					{
						display_message(WARNING_MESSAGE,"current_pane may be from 1 to %d",
							graphics_window->number_of_panes);
						return_code = 0;
					}
#if defined (WX_USER_INTERFACE)
					if (show_time_editor_flag + hide_time_editor_flag > 1)
					{
						display_message(ERROR_MESSAGE,"Only one of "
							"show_time_editor/hide_time_editor");
						return_code = 0;
					}
#endif /* defined (WX_USER_INTERFACE) */
					if ((fast_transparency_flag+slow_transparency_flag+
						 (0 < order_independent_transparency)) > 1)
					{
						display_message(ERROR_MESSAGE,"Only one of "
							"fast_transparency/slow_transparency/order_independent_transparency");
						return_code = 0;
					}
					if (blending_mode_string)
					{
						STRING_TO_ENUMERATOR(cmzn_sceneviewer_blending_mode)(
							blending_mode_string, &blending_mode);
					}
					if (return_code && graphics_window)
					{
						redraw=0;
						/* user deals with pane numbers one higher than internally */
						current_pane -= 1;
						if (current_pane != graphics_window->current_pane)
						{
							Graphics_window_set_current_pane(graphics_window,current_pane);
						}
						interactive_tool=FIND_BY_IDENTIFIER_IN_MANAGER(Interactive_tool,name)(
							tool_name,graphics_window->interactive_tool_manager);
						if (interactive_tool)
						{
#if defined (WX_USER_INTERFACE)
							wxBitmapButton *current_tool_button;
							if (strcmp("transform_tool", tool_name) == 0)
							{
									current_tool_button = graphics_window->transform_tool_button;
							}
							if (strcmp("node_tool", tool_name) == 0)
							{
									current_tool_button = graphics_window->node_tool_button;
							}
							else if (strcmp("data_tool", tool_name) == 0)
							{
									current_tool_button = graphics_window->data_tool_button;
							}
							else if (strcmp("element_tool", tool_name) == 0)
							{
									current_tool_button = graphics_window->element_tool_button;
							}
							else if (strcmp("cad_tool", tool_name) == 0)
							{
									current_tool_button = graphics_window->cad_tool_button;
							}
							else if (strcmp("element_point_tool", tool_name) == 0)
							{
									current_tool_button = graphics_window->element_point_tool_button;
							}
							else
							{
									current_tool_button = NULL;
							}
							if (current_tool_button != NULL)
							{
									graphics_window->wx_graphics_window->GraphicsWindowSetInteractiveButtonSelected(
										 current_tool_button, interactive_tool, graphics_window);
							}
#endif /*defined (WX_USER_INTERFACE) */
							Graphics_window_set_interactive_tool(graphics_window,
								interactive_tool);
						}
						if (std_view_angle != graphics_window->std_view_angle)
						{
							Graphics_window_set_std_view_angle(graphics_window,
								std_view_angle);
						}
						if (antialias_mode != graphics_window->antialias_mode)
						{
							Graphics_window_set_antialias_mode(graphics_window,
								antialias_mode);
							redraw=1;
						}
						if ((depth_of_field != graphics_window->depth_of_field)
							|| (focal_depth != graphics_window->focal_depth))
						{
							Graphics_window_set_depth_of_field(graphics_window,
								depth_of_field, focal_depth);
							redraw=1;
						}
						if (perturb_lines != graphics_window->perturb_lines)
						{
							Graphics_window_set_perturb_lines(graphics_window,perturb_lines);
							redraw=1;
						}
#if defined (WX_USER_INTERFACE)
						if (show_time_editor_flag || hide_time_editor_flag)
						{
							 if (graphics_window->time_editor_panel)
							 {
									graphics_window->time_editor_panel->Show(show_time_editor_flag);
									graphics_window->time_editor_togglebutton->SetValue(show_time_editor_flag);
							 }
							 if (show_time_editor_flag && graphics_window->GraphicsWindowTitle)
							 {
									graphics_window->GraphicsWindowTitle->SetSize(graphics_window->GraphicsWindowTitle->GetSize()+wxSize(0,1));
									graphics_window->GraphicsWindowTitle->SetSize(graphics_window->GraphicsWindowTitle->GetSize()-wxSize(0,1));
									graphics_window->GraphicsWindowTitle->Layout();
							 }
						}
#endif /* defined (WX_USER_INTERFACE) */
						if (fast_transparency_flag || slow_transparency_flag || order_independent_transparency)
						{
							if (fast_transparency_flag)
							{
								transparency_mode = CMZN_SCENEVIEWER_TRANSPARENCY_MODE_FAST;
							}
							else if (order_independent_transparency)
							{
								transparency_mode = CMZN_SCENEVIEWER_TRANSPARENCY_MODE_ORDER_INDEPENDENT;
								transparency_layers = order_independent_transparency;
							}
							else
							{
								transparency_mode = CMZN_SCENEVIEWER_TRANSPARENCY_MODE_SLOW;
							}
							for (pane_no=0;pane_no<graphics_window->number_of_scene_viewers;
								pane_no++)
							{
								cmzn_sceneviewer_set_transparency_mode(
									graphics_window->scene_viewer_array[pane_no]->core_scene_viewer,
									transparency_mode);
								if (order_independent_transparency)
								{
									cmzn_sceneviewer_set_transparency_layers(
										graphics_window->scene_viewer_array[pane_no]->core_scene_viewer,
										transparency_layers);
								}
							}
							redraw=1;
						}
						if (blending_mode != graphics_window->blending_mode)
						{
							Graphics_window_set_blending_mode(graphics_window,
								blending_mode);
							redraw=1;
						}
						if (redraw)
						{
							Graphics_window_update_now(graphics_window);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"modify_Graphics_window_set.  Missing window");
						return_code=0;
					}
				}
				if (tool_names)
				{
					for (i=0;i<number_of_tools;i++)
					{
						DEALLOCATE(tool_names[i]);
					}
					DEALLOCATE(tool_names);
				}
				DESTROY(Option_table)(&option_table);
			}
			else
			{
				display_message(WARNING_MESSAGE,"Missing window settings");
				display_parse_state_location(state);
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"modify_Graphics_window_set.  "
				"Missing modify_graphics_window_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"modify_Graphics_window_set.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* modify_Graphics_window_set */

static int modify_Graphics_window_view(struct Parse_state *state,
	void *window_void,void *modify_graphics_window_data_void)
/*******************************************************************************
LAST MODIFIED : 14 February 2005

DESCRIPTION :
Parser commands for modifying the view in the current pane of <window>,
view angle, interest point etc.
==============================================================================*/
{
	char allow_skew_flag,absolute_viewport_flag,distorting_relative_viewport_flag,
		custom_projection_flag,parallel_projection_flag,perspective_projection_flag,
		relative_viewport_flag;
	double bottom,eye[3],clip_plane_add[4],clip_plane_remove[4],far_plane,left,lookat[3],
		modelview_matrix[16],
		ndc_placement[4],near_plane,photogrammetry_matrix[12],projection_matrix[16],right,
		top,up[3],view_angle,viewport_coordinates[4];
	enum Scene_viewer_projection_mode projection_mode;
	int number_of_components,return_code;
	struct Graphics_window *window;
	struct Option_table *option_table, *projection_mode_option_table,
		*viewport_mode_option_table;
	struct Scene_viewer_app *scene_viewer;
	static struct Set_vector_with_help_data
		clip_plane_add_data=
			{4," A B C D",0},
		clip_plane_remove_data=
			{4," A B C D",0},
		modelview_matrix_data=
			{16," M11 M12 M13 M14 M21 M22 M23 M24 M31 M32 M33 M34 M41 M42 M43 M44",0},
		ndc_placement_data=
			{4," NDC_LEFT NDC_TOP NDC_WIDTH NDC_HEIGHT",0},
		photogrammetry_matrix_data=
			{12," T11 T12 T13 T21 T22 T23 T31 T32 T33 T41 T42 T43",0},
		projection_matrix_data=
			{16," M11 M12 M13 M14 M21 M22 M23 M24 M31 M32 M33 M34 M41 M42 M43 M44",0},
		viewport_coordinates_data=
			{4," VIEWPORT_LEFT VIEWPORT_TOP PIXELS_PER_UNIT_X PIXELS_PER_UNIT_Y",0};


	ENTER(modify_Graphics_window_view);
	USE_PARAMETER(modify_graphics_window_data_void);
	if (state)
	{
		if (state->current_token)
		{
			/* get defaults from scene_viewer of current pane of window */
			if ((window=(struct Graphics_window *)window_void)&&
				 (window->scene_viewer_array != NULL) &&
				 (scene_viewer=window->scene_viewer_array[window->current_pane])&&
				 cmzn_sceneviewer_get_lookat_parameters(scene_viewer->core_scene_viewer, eye, lookat, up) &&
				Scene_viewer_get_viewing_volume(scene_viewer->core_scene_viewer,&left,&right,
					&bottom,&top,&near_plane,&far_plane))
			{
				view_angle = cmzn_sceneviewer_get_view_angle(scene_viewer->core_scene_viewer);
				view_angle *= (180.0/PI);
			}
			else
			{
				eye[0]=eye[1]=eye[2]=0.0;
				lookat[0]=lookat[1]=lookat[2]=0.0;
				up[0]=up[1]=up[2]=0.0;
				left=right=bottom=top=0.0;
				near_plane=far_plane=0.0;
				view_angle=0.0;
				scene_viewer = 0;
			}
			allow_skew_flag=0;
			absolute_viewport_flag=0;
			distorting_relative_viewport_flag=0;
			relative_viewport_flag=0;
			custom_projection_flag=0;
			parallel_projection_flag=0;
			perspective_projection_flag=0;
			number_of_components=3;
			clip_plane_add_data.set=0;
			clip_plane_remove_data.set=0;
			modelview_matrix_data.set=0;
			ndc_placement_data.set=0;
			photogrammetry_matrix_data.set=0;
			projection_matrix_data.set=0;
			viewport_coordinates_data.set=0;
			option_table = CREATE(Option_table)();
			/* allow_skew */
			Option_table_add_char_flag_entry(option_table, "allow_skew", &allow_skew_flag);
			/* clip_plane_add */
			Option_table_add_double_vector_with_help_entry(option_table, "clip_plane_add",
				clip_plane_add, &clip_plane_add_data);
			/* clip_plane_remove */
			Option_table_add_double_vector_with_help_entry(option_table, "clip_plane_remove",
				clip_plane_remove, &clip_plane_remove_data);
			/* eye_point */
			Option_table_add_double_vector_entry(option_table, "eye_point", eye,
				&number_of_components);
			/* far_clipping_plane */
			Option_table_add_double_entry(option_table, "far_clipping_plane", &far_plane);
			/* interest_point */
			Option_table_add_double_vector_entry(option_table, "interest_point", lookat,
				&number_of_components);
			/* modelview_matrix */
			Option_table_add_double_vector_with_help_entry(option_table, "modelview_matrix",
				modelview_matrix, &modelview_matrix_data);
			/* ndc_placement (normalised device coordinate placement) */
			Option_table_add_double_vector_with_help_entry(option_table, "ndc_placement",
				ndc_placement, &ndc_placement_data);
			/* near_clipping_plane */
			Option_table_add_double_entry(option_table, "near_clipping_plane", &near_plane);
			/* photogrammetry_matrix */
			Option_table_add_double_vector_with_help_entry(option_table, "photogrammetry_matrix",
				photogrammetry_matrix, &photogrammetry_matrix_data);
			/* projection_matrix */
			Option_table_add_double_vector_with_help_entry(option_table, "projection_matrix",
				projection_matrix, &projection_matrix_data);
			/* projection mode: custom/parallel/perspective */
			projection_mode_option_table = CREATE(Option_table)();
			Option_table_add_char_flag_entry(projection_mode_option_table, "custom",
				&custom_projection_flag);
			Option_table_add_char_flag_entry(projection_mode_option_table, "parallel",
				&parallel_projection_flag);
			Option_table_add_char_flag_entry(projection_mode_option_table, "perspective",
				&perspective_projection_flag);
			Option_table_add_suboption_table(option_table,
				projection_mode_option_table);
			/* up_vector */
			Option_table_add_double_vector_entry(option_table, "up_vector", up,
				&number_of_components);
			/* viewport_coordinates */
			Option_table_add_double_vector_with_help_entry(option_table, "viewport_coordinates",
				viewport_coordinates, &viewport_coordinates_data);
			/* view_angle */
			Option_table_add_double_entry(option_table, "view_angle", &view_angle);
			/* viewport mode: absolute_viewport/relative_viewport */
			viewport_mode_option_table = CREATE(Option_table)();
			Option_table_add_char_flag_entry(viewport_mode_option_table, "absolute_viewport",
				&absolute_viewport_flag);
			Option_table_add_char_flag_entry(viewport_mode_option_table, "relative_viewport",
				&relative_viewport_flag);
			Option_table_add_char_flag_entry(viewport_mode_option_table, "distorting_relative_viewport",
				&distorting_relative_viewport_flag);
			Option_table_add_suboption_table(option_table,
				viewport_mode_option_table);
			return_code=Option_table_multi_parse(option_table, state);
			if (return_code)
			{
				if (1<(absolute_viewport_flag+relative_viewport_flag+
					distorting_relative_viewport_flag))
				{
					display_message(WARNING_MESSAGE,
						"Only one of absolute_viewport/distorting_relative_viewport/relative_viewport");
					absolute_viewport_flag=0;
					distorting_relative_viewport_flag=0;
					relative_viewport_flag=0;
				}
				if (1<(custom_projection_flag+parallel_projection_flag+
					perspective_projection_flag))
				{
					display_message(WARNING_MESSAGE,
						"Only one of parallel/perspective/custom");
					custom_projection_flag=0;
					parallel_projection_flag=0;
					perspective_projection_flag=0;
				}
				if (photogrammetry_matrix_data.set&&
					(modelview_matrix_data.set || projection_matrix_data.set))
				{
					display_message(WARNING_MESSAGE,"photogrammetry_matrix "
						"may not be used with modelview_matrix or projection_matrix");
					photogrammetry_matrix_data.set=0;
				}
				if (scene_viewer)
				{
					if (parallel_projection_flag)
					{
						Graphics_window_set_projection_mode(window,window->current_pane,
							SCENE_VIEWER_PARALLEL);
					}
					if (perspective_projection_flag)
					{
						Graphics_window_set_projection_mode(window,window->current_pane,
							SCENE_VIEWER_PERSPECTIVE);
					}
					if (custom_projection_flag)
					{
						Graphics_window_set_projection_mode(window,window->current_pane,
							SCENE_VIEWER_CUSTOM);
					}
					projection_mode=
						Graphics_window_get_projection_mode(window,window->current_pane);
					if (SCENE_VIEWER_CUSTOM != projection_mode)
					{
						/* must set viewing volume before view_angle otherwise view_angle
							is overwritten */
						Scene_viewer_set_viewing_volume(scene_viewer->core_scene_viewer,left,right,bottom,top,
							near_plane,far_plane);
						if (allow_skew_flag)
						{
							Scene_viewer_set_lookat_parameters(scene_viewer->core_scene_viewer,
								eye[0],eye[1],eye[2],lookat[0],lookat[1],lookat[2],
								up[0],up[1],up[2]);
						}
						else
						{
							cmzn_sceneviewer_set_lookat_parameters_non_skew(scene_viewer->core_scene_viewer,
								eye, lookat, up);
						}
						/* must set view angle after lookat parameters */
						if ((0.0<view_angle)&&(view_angle<180.0))
						{
							cmzn_sceneviewer_set_view_angle(scene_viewer->core_scene_viewer, view_angle*(PI/180.0));
						}
						else
						{
							display_message(WARNING_MESSAGE,
								"View angle should be between 0 and 180 degrees.");
						}
						if (absolute_viewport_flag)
						{
							cmzn_sceneviewer_set_viewport_mode(scene_viewer->core_scene_viewer,
								CMZN_SCENEVIEWER_VIEWPORT_MODE_ABSOLUTE);
						}
						if (relative_viewport_flag)
						{
							cmzn_sceneviewer_set_viewport_mode(scene_viewer->core_scene_viewer,
								CMZN_SCENEVIEWER_VIEWPORT_MODE_RELATIVE);
						}
						if (distorting_relative_viewport_flag)
						{
							cmzn_sceneviewer_set_viewport_mode(scene_viewer->core_scene_viewer,
								CMZN_SCENEVIEWER_VIEWPORT_MODE_DISTORTING_RELATIVE);
						}
					}
					/*???RC should have checks on whether you can set these for the
						current layout_mode */
					if (modelview_matrix_data.set)
					{
						if (SCENE_VIEWER_CUSTOM==projection_mode)
						{
							Scene_viewer_set_modelview_matrix(scene_viewer->core_scene_viewer,
								modelview_matrix);
						}
						else
						{
							display_message(WARNING_MESSAGE,
								"modelview_matrix may only be used with CUSTOM projection");
						}
					}
					/* must set ndc_placement before photogrammetry_matrix */
					if (ndc_placement_data.set)
					{
						Scene_viewer_set_NDC_info(scene_viewer->core_scene_viewer,ndc_placement[0],
							ndc_placement[1],ndc_placement[2],ndc_placement[3]);
					}
					if (projection_matrix_data.set)
					{
						if (SCENE_VIEWER_CUSTOM==projection_mode)
						{
							Scene_viewer_set_projection_matrix(scene_viewer->core_scene_viewer,
								projection_matrix);
						}
						else
						{
							display_message(WARNING_MESSAGE,
								"projection_matrix may only be used with CUSTOM projection");
						}
					}
					if (photogrammetry_matrix_data.set)
					{
						if (SCENE_VIEWER_CUSTOM==projection_mode)
						{
#if defined (MIRAGE)
							double NDC_left,NDC_bottom,NDC_top,NDC_width,NDC_height;

							/* photogrammetry_matrix must be after ndc_placement viewing
								 volume since it uses the latest NDC, near and far values */
							if (Scene_viewer_get_viewing_volume(scene_viewer,&left,&right,
								&bottom,&top,&near_plane,&far_plane)&&Scene_viewer_get_NDC_info(
									scene_viewer,&NDC_left,&NDC_top,&NDC_width,&NDC_height))
							{
								NDC_bottom=NDC_top-NDC_height;
								photogrammetry_to_graphics_projection(photogrammetry_matrix,
									near_plane,far_plane,NDC_left,NDC_bottom,NDC_width,NDC_height,
									modelview_matrix,projection_matrix,eye,lookat,up);
								Scene_viewer_set_modelview_matrix(scene_viewer,
									modelview_matrix);
								Scene_viewer_set_lookat_parameters(scene_viewer,
									eye[0],eye[1],eye[2],lookat[0],lookat[1],lookat[2],
									up[0],up[1],up[2]);
								Scene_viewer_set_NDC_info(scene_viewer,
									NDC_left,NDC_top,NDC_width,NDC_height);
								Scene_viewer_set_projection_matrix(scene_viewer,
									projection_matrix);
							}
							else
							{
								display_message(ERROR_MESSAGE,"modify_Graphics_window_view.  "
									"Could not apply photogrammetry_matrix");
								return_code=0;
							}
#else /* defined (MIRAGE) */
							display_message(INFORMATION_MESSAGE,
								"photogrammetry_matrix is not available\n");
#endif /* defined (MIRAGE) */
						}
						else
						{
							display_message(WARNING_MESSAGE,
								"photogrammetry_matrix may only be used with CUSTOM projection");
						}
					}
					if (viewport_coordinates_data.set)
					{
						Scene_viewer_set_viewport_info(scene_viewer->core_scene_viewer,
							viewport_coordinates[0],viewport_coordinates[1],
							viewport_coordinates[2],viewport_coordinates[3]);
					}
					if (clip_plane_remove_data.set)
					{
						Scene_viewer_remove_clip_plane(scene_viewer->core_scene_viewer,
							clip_plane_remove[0], clip_plane_remove[1], clip_plane_remove[2],
							clip_plane_remove[3]);
					}
					if (clip_plane_add_data.set)
					{
						Scene_viewer_add_clip_plane(scene_viewer->core_scene_viewer,
							clip_plane_add[0], clip_plane_add[1], clip_plane_add[2],
							clip_plane_add[3]);
					}
					/* redraw the Scene_viewer */
					Scene_viewer_app_redraw_now(scene_viewer);
					/* allow changes to flow to tied panes */
					Graphics_window_view_changed(window,window->current_pane);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"modify_Graphics_window_view.  Missing or invalid scene_viewer");
					display_parse_state_location(state);
					return_code=0;
				}
			}
			DESTROY(Option_table)(&option_table);
		}
		else
		{
			display_message(WARNING_MESSAGE,"Missing window view modifications");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"modify_Graphics_window_view.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* modify_Graphics_window_view */

/*
Global functions
----------------
*/
struct Graphics_window *CREATE(Graphics_window)(const char *name,
	enum Graphics_window_buffering_mode buffering_mode,
	enum Graphics_window_stereo_mode stereo_mode,
	int minimum_colour_buffer_depth, int minimum_depth_buffer_depth,
	int minimum_accumulation_buffer_depth,
	struct Graphics_buffer_app_package *graphics_buffer_package,
	cmzn_scenefiltermodule_id filter_module,cmzn_scene *scene,
	struct MANAGER(Interactive_tool) *interactive_tool_manager,
	struct Time_keeper_app *default_time_keeper_app,
	struct User_interface *user_interface,
	cmzn_region_id root_region,
	cmzn_sceneviewermodule_id sceneviewermodule)
/*******************************************************************************
LAST MODIFIED : 6 May 2004

DESCRIPTION:
Creates a Graphics_window object, window shell and widgets. Returns a pointer
to the newly created object. The Graphics_window maintains a pointer to the
manager it is to live in, since users will want to close windows with the
window manager widgets.
Each window has a unique <name> that can be used to identify it, and which
will be printed on the windows title bar.
A stereo buffering mode will automatically be chosen when the visual supports
it.
==============================================================================*/
{
	char *window_title;
	enum Graphics_buffer_buffering_mode graphics_buffer_buffering_mode = GRAPHICS_BUFFER_ANY_BUFFERING_MODE;
	enum Graphics_buffer_stereo_mode graphics_buffer_stereo_mode = GRAPHICS_BUFFER_ANY_STEREO_MODE;;
#if defined (WX_USER_INTERFACE)
	double eye[3],eye_distance,front[3],lookat[3],up[3],view[3];
	int ortho_front_axis,ortho_up_axis;
#endif /*(WX_USER_INTERFACE) */
	int pane_no;
	struct Graphics_buffer_app *graphics_buffer;
	struct Graphics_window *window=NULL;

	ENTER(create_graphics_window);
	if (name&&((GRAPHICS_WINDOW_ANY_BUFFERING_MODE==buffering_mode)||
		(GRAPHICS_WINDOW_SINGLE_BUFFERING==buffering_mode)||
		(GRAPHICS_WINDOW_DOUBLE_BUFFERING==buffering_mode))&&
		((GRAPHICS_WINDOW_ANY_STEREO_MODE==stereo_mode)||
		(GRAPHICS_WINDOW_MONO==stereo_mode)||
		(GRAPHICS_WINDOW_STEREO==stereo_mode))&&
		filter_module&&scene&&interactive_tool_manager&&
		graphics_buffer_package&&user_interface && root_region && sceneviewermodule)
	{
		/* Try to allocate space for the window structure */
		if (ALLOCATE(window,struct Graphics_window,1)&&
			ALLOCATE(window->name,char,strlen(name)+1))
		{
			strcpy((char *)window->name,name);
			/* initialize the fields of the window structure */
			window->access_count=1;
			window->sceneviewermodule = cmzn_sceneviewermodule_access(sceneviewermodule);
			window->eye_spacing=0.25;
			window->std_view_angle=40.0;
			window->root_region = cmzn_region_access(root_region);
			window->graphics_window_manager=
				(struct MANAGER(Graphics_window) *)NULL;
			window->manager_change_status = MANAGER_CHANGE_NONE(Graphics_window);
			window->graphics_buffer_package = graphics_buffer_package;
			window->filter_module=cmzn_scenefiltermodule_access(filter_module);
			window->scene=cmzn_scene_access(scene);
			window->time_keeper_app = ACCESS(Time_keeper_app)(default_time_keeper_app);
			window->interactive_tool_manager=interactive_tool_manager;
			window->interactive_tool=
				FIND_BY_IDENTIFIER_IN_MANAGER(Interactive_tool,name)(
					"transform_tool",window->interactive_tool_manager);
			window->user_interface=user_interface;
			window->default_viewing_height=512;
			window->default_viewing_width=512;
			window->default_translate_rate=1.0;
			window->default_tumble_rate=1.5;
			window->default_zoom_rate=1.0;
			window->layout_mode=GRAPHICS_WINDOW_LAYOUT_ORTHOGRAPHIC;
			window->number_of_scene_viewers = 0;
			window->number_of_panes=0;
			window->scene_viewer_array = 0;
			window->current_pane=0;
			window->antialias_mode=0;
			window->perturb_lines=0;
			window->blending_mode = CMZN_SCENEVIEWER_BLENDING_MODE_NORMAL;
			window->depth_of_field=0.0;
			window->focal_depth=0.0;
			/* the input_mode set here is changed below */
			window->input_mode=SCENE_VIEWER_NO_INPUT;
			if (ALLOCATE(window_title,char,50+strlen(name)))
			{
				sprintf(window_title,"CMGUI Graphics window %s",name);
			}
			window->ortho_up_axis=0;
			window->ortho_front_axis=0;
			switch (buffering_mode)
			{
				case GRAPHICS_WINDOW_ANY_BUFFERING_MODE:
				{
					graphics_buffer_buffering_mode=
						GRAPHICS_BUFFER_ANY_BUFFERING_MODE;
				} break;
				case GRAPHICS_WINDOW_SINGLE_BUFFERING:
				{
					graphics_buffer_buffering_mode=
						GRAPHICS_BUFFER_SINGLE_BUFFERING;
				} break;
				case GRAPHICS_WINDOW_DOUBLE_BUFFERING:
				{
					graphics_buffer_buffering_mode=
						GRAPHICS_BUFFER_DOUBLE_BUFFERING;
				} break;
				default:
				{
					display_message(ERROR_MESSAGE,
						"CREATE(Graphics_window).  Invalid buffering mode");
				}
			}
			switch (stereo_mode)
			{
				case GRAPHICS_WINDOW_ANY_STEREO_MODE:
				{
					graphics_buffer_stereo_mode=
						GRAPHICS_BUFFER_ANY_STEREO_MODE;
				} break;
				case GRAPHICS_WINDOW_MONO:
				{
					graphics_buffer_stereo_mode=
						GRAPHICS_BUFFER_MONO;
				} break;
				case GRAPHICS_WINDOW_STEREO:
				{
					graphics_buffer_stereo_mode=
						GRAPHICS_BUFFER_STEREO;
				} break;
				default:
				{
					display_message(ERROR_MESSAGE,
						"CREATE(Graphics_window).  Invalid buffering mode");
				}
			}
#if defined (GTK_USER_INTERFACE) /* switch (USER_INTERFACE) */
			window->close_handler_id = 0;
			window->shell_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
			if (window->shell_window)
			{
				gtk_window_set_title(GTK_WINDOW(window->shell_window),
					window_title);
				graphics_buffer = create_Graphics_buffer_gtkgl(
					graphics_buffer_package, GTK_CONTAINER(window->shell_window),
					graphics_buffer_buffering_mode, graphics_buffer_stereo_mode,
					minimum_colour_buffer_depth, minimum_depth_buffer_depth,
					minimum_accumulation_buffer_depth);
				if (graphics_buffer)
				{
					/* create one Scene_viewers */
					window->number_of_scene_viewers = 1;
					if (ALLOCATE(window->scene_viewer_array,
						struct Scene_viewer *,
						window->number_of_scene_viewers))
					{
						pane_no = 0;
						window->scene_viewer_array[pane_no] =
						 CREATE(Scene_viewer)(graphics_buffer, background_colour, lightmodule,
							default_light, light_model_module,default_light_model, filter_module,
							window->scene, user_interface);
						if (window->scene_viewer_array[pane_no])
						{
							Scene_viewer_set_interactive_tool(
								window->scene_viewer_array[pane_no],
								window->interactive_tool);
							/* get scene_viewer transform callbacks to allow
								synchronising of views in multiple panes */
							Scene_viewer_app_add_sync_callback(
								window->scene_viewer_array[pane_no],
								Graphics_window_Scene_viewer_view_changed,
								window);
							cmzn_sceneviewer_set_translation_rate(
								window->scene_viewer_array[pane_no], 2.0);
							cmzn_sceneviewer_set_tumble_rate(
								window->scene_viewer_array[pane_no], 1.5);
							cmzn_sceneviewer_set_zoom_rate(
								window->scene_viewer_array[pane_no], 2.0);

							/* set the initial layout */
							Graphics_window_set_layout_mode(window,
								GRAPHICS_WINDOW_LAYOUT_SIMPLE);
							/* give the window its default size */
							Graphics_window_set_viewing_area_size(window,
								window->default_viewing_width,
								window->default_viewing_height);
							/* initial view is of all of the current scene */
							Graphics_window_view_all(window);

#if GTK_MAJOR_VERSION >= 2
							window->close_handler_id =
								g_signal_connect (G_OBJECT(window->shell_window), "destroy",
									G_CALLBACK(Graphics_window_gtk_close_CB), (gpointer)window);
#else /* GTK_MAJOR_VERSION >= 2 */
							gtk_signal_connect(GTK_OBJECT(window->shell_window), "destroy",
								GTK_SIGNAL_FUNC(Graphics_window_gtk_close_CB), (gpointer)window);
#endif /* GTK_MAJOR_VERSION >= 2 */

							gtk_widget_show_all(window->shell_window);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"CREATE(Graphics_window).  "
								"Could not create graphics buffer.");
							DEACCESS(Graphics_window)(&window);
							window = (struct Graphics_window *)NULL;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"CREATE(Graphics_window).  "
							"Could not allocate memory for scene viewer array.");
						DEACCESS(Graphics_window)(&window);
						window = (struct Graphics_window *)NULL;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"CREATE(Graphics_window).  "
						"Could not create graphics buffer.");
					DEACCESS(Graphics_window)(&window);
					window = (struct Graphics_window *)NULL;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"CREATE(Graphics_window).  Unable to get main window.");
				window = (struct Graphics_window *)NULL;
			}
#elif defined (WIN32_USER_INTERFACE) /* switch (USER_INTERFACE) */
			BOOL win32_return_code;
			static const char *class_name="Graphics_window";
			WNDCLASS class_information;

			/* check if the class is registered */
			win32_return_code=GetClassInfo(User_interface_get_instance(user_interface),
				class_name,&class_information);

			if (win32_return_code==FALSE)
			{
				/* register class */
				class_information.cbClsExtra=0;
				class_information.cbWndExtra=sizeof(struct Graphics_window *);
				class_information.hbrBackground=(HBRUSH)(COLOR_WINDOW+1);
				class_information.hCursor=LoadCursor(NULL,IDC_ARROW);
				class_information.hIcon=LoadIcon(
					User_interface_get_instance(user_interface),
					"Command_window_icon");
				class_information.hInstance=User_interface_get_instance(
					user_interface);
				class_information.lpfnWndProc=DefWindowProc;
				class_information.lpszClassName=class_name;
				class_information.style=CS_OWNDC;
				class_information.lpszMenuName=NULL;
				if (RegisterClass(&class_information))
				{
					win32_return_code=TRUE;
				}
			}

			/* create the window */
			if (win32_return_code!=FALSE)
			{
				int width = window->default_viewing_width;
				int height = window->default_viewing_height;
				Graphics_window_adjust_sizes_for_window_frame(&width, &height);

				if (window->hWnd=CreateWindow(class_name, window_title,
					WS_CAPTION | WS_POPUPWINDOW | WS_VISIBLE | WS_SIZEBOX,
					0, 0, width, height,
					NULL, NULL, User_interface_get_instance(user_interface), NULL))
				{
					if (graphics_buffer = create_Graphics_buffer_win32(
						graphics_buffer_package,
						window->hWnd, (HDC)NULL,
						graphics_buffer_buffering_mode, graphics_buffer_stereo_mode,
						minimum_colour_buffer_depth, minimum_depth_buffer_depth,
						minimum_accumulation_buffer_depth))
					{
						   SetWindowLongPtr(window->hWnd, 0, (LONG_PTR)graphics_buffer);

						/* create one Scene_viewers */
						window->number_of_scene_viewers = 1;
						if (ALLOCATE(window->scene_viewer_array,
							struct Scene_viewer *,
							window->number_of_scene_viewers))
						{
							pane_no = 0;
							if (window->scene_viewer_array[pane_no] =
								 CREATE(Scene_viewer)(graphics_buffer,
								 background_colour, light_manager,default_light,
								 light_model_manager,default_light_model,
								 filter_module, window->scene,
								 user_interface))
							{
								Scene_viewer_set_interactive_tool(
									window->scene_viewer_array[pane_no],
									window->interactive_tool);
								/* get scene_viewer transform callbacks to allow
									synchronising of views in multiple panes */
								Scene_viewer_app_add_sync_callback(
									window->scene_viewer_array[pane_no],
									Graphics_window_Scene_viewer_view_changed,
									window);
								cmzn_sceneviewer_set_translation_rate(
									window->scene_viewer_array[pane_no], 2.0);
								cmzn_sceneviewer_set_tumble_rate(
									window->scene_viewer_array[pane_no], 1.5);
								cmzn_sceneviewer_set_zoom_rate(
									window->scene_viewer_array[pane_no], 2.0);

								/* set the initial layout */
								Graphics_window_set_layout_mode(window,
									GRAPHICS_WINDOW_LAYOUT_SIMPLE);
								/* give the window its default size */
								Graphics_window_set_viewing_area_size(window,
									window->default_viewing_width,
									window->default_viewing_height);
								/* initial view is of all of the current scene */
								Graphics_window_view_all(window);

							}
							else
							{
								display_message(ERROR_MESSAGE,
									"CREATE(Graphics_window).  "
									"Could not create scene_viewer.");
								DEACCESS(Graphics_window)(&window);
								window = (struct Graphics_window *)NULL;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"CREATE(Graphics_window).  "
								"Could not allocate memory for scene viewer array.");
							DEACCESS(Graphics_window)(&window);
							window = (struct Graphics_window *)NULL;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"CREATE(Graphics_window).  "
							"Could not create graphics buffer.");
						DEACCESS(Graphics_window)(&window);
						window = (struct Graphics_window *)NULL;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"CREATE(Graphics_window).  Could not create window");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"CREATE(Graphics_window).  Unable to register class information");
			}
#elif defined (WX_USER_INTERFACE) /* switch (USER_INTERFACE) */
			USE_PARAMETER(pane_no);
			USE_PARAMETER(minimum_colour_buffer_depth);
			USE_PARAMETER(minimum_depth_buffer_depth);
			USE_PARAMETER(minimum_accumulation_buffer_depth);
			USE_PARAMETER(Graphics_window_Scene_viewer_view_changed);

			wxLogNull logNo;
			window->wx_graphics_window = new wxGraphicsWindow(window);
			window->grid_field =NULL;
			window->GraphicsWindowTitle = XRCCTRL(*window->wx_graphics_window,
				 "CmguiGraphicsWindow", wxFrame);
			window->GraphicsWindowTitle->SetTitle(wxString::FromAscii(window_title));
			window->wx_perspective_button = XRCCTRL(*window->wx_graphics_window,
				 "PerspectiveButton", wxCheckBox);
			if (window_title)
				 DEALLOCATE(window_title);
			/* The time_slider receives messages from the
				 default_time_keeper of the scene */
			window->GraphicsWindowTitle->SetMinSize(wxSize(0,0));
			window->panel = XRCCTRL(*window->wx_graphics_window, "Panel", wxPanel);
			window->panel2 = XRCCTRL(*window->wx_graphics_window, "Panel2", wxPanel);
			window->panel3 = XRCCTRL(*window->wx_graphics_window, "Panel3", wxPanel);
			window->panel4 = XRCCTRL(*window->wx_graphics_window, "Panel4", wxPanel);
			window->up_view_options = XRCCTRL(*window->wx_graphics_window,
				 "UpViewOptions", wxChoice);
			window->up_view_options->Disable();
			window->front_view_options = XRCCTRL(*window->wx_graphics_window,
				 "FrontViewOptions", wxButton);
			window->front_view_options->Disable();
			window->left_panel =XRCCTRL(*window->wx_graphics_window, "GraphicsLeftPanel", wxScrolledWindow);
			window->left_panel->FitInside();
			window->left_panel->SetMinSize(wxSize(0,10));
			window->ToolPanel = XRCCTRL(*window->wx_graphics_window,
				 "ToolPanel", wxScrolledWindow);
			window->ToolPanel ->Layout();
			window->interactive_toolbar_panel = NULL;

			/* setting up the time editor */
			wxBitmap cross_bmp(cross_xpm);
			window->hide_time_bitmapbutton = XRCCTRL(*window->wx_graphics_window,
				 "GraphicsWindowHideTimeBitmapButton", wxBitmapButton);
			window->hide_time_bitmapbutton->SetBitmapLabel(cross_bmp);
			wxBitmap cross_selected_bmp(cross_selected_xpm);
			window->hide_time_bitmapbutton->SetBitmapFocus(cross_selected_xpm);

			window->time_slider = XRCCTRL(*window->wx_graphics_window,
				 "GraphicsWindowTimeSlider", wxSlider);
			window->time_slider->SetRange(0,1000);

			wxBitmap time_editor_bmp1(fastbackward_xpm);
			window->fast_backward = XRCCTRL(*window->wx_graphics_window,
				 "GraphicsWindowFastBackward", wxBitmapButton);
			window->fast_backward->SetBitmapLabel(time_editor_bmp1);

			wxBitmap time_editor_bmp2(backward_by_frame_xpm);
			window->backward_by_frame = XRCCTRL(*window->wx_graphics_window,
				 "GraphicsWindowBackwardByStep", wxBitmapButton);
			window->backward_by_frame->SetBitmapLabel(time_editor_bmp2);

			wxBitmap time_editor_bmp3(backward_xpm);
			window->backward = XRCCTRL(*window->wx_graphics_window,
				 "GraphicsWindowBackward", wxBitmapButton);
			window->backward->SetBitmapLabel(time_editor_bmp3);

			wxBitmap time_editor_bmp4(stop_button_xpm);
			window->stop_button = XRCCTRL(*window->wx_graphics_window,
				 "GraphicsWindowStop", wxBitmapButton);
			window->stop_button->SetBitmapLabel(time_editor_bmp4);

			wxBitmap time_editor_bmp5(forward_xpm);
			window->forward = XRCCTRL(*window->wx_graphics_window,
				 "GraphicsWindowForward", wxBitmapButton);
			window->forward->SetBitmapLabel(time_editor_bmp5);

			wxBitmap time_editor_bmp6(forward_by_frame_xpm);
			window->forward_by_frame = XRCCTRL(*window->wx_graphics_window,
				 "GraphicsWindowForwardByStep", wxBitmapButton);
			window->forward_by_frame->SetBitmapLabel(time_editor_bmp6);

			wxBitmap time_editor_bmp7(fastforward_xpm);
			window->fast_forward = XRCCTRL(*window->wx_graphics_window,
				 "GraphicsWindowFastForward", wxBitmapButton);
			window->fast_forward->SetBitmapLabel(time_editor_bmp7);

			window->time_editor_togglebutton = XRCCTRL(*window->wx_graphics_window,
				 "TimeEditorToggleButton", wxToggleButton);
			window->time_editor_togglebutton->SetValue(0);
			window->time_text_ctrl = XRCCTRL(*window->wx_graphics_window,
				 "GraphicsWindowTimeTextCtrl", wxTextCtrl);
			window->time_slider = XRCCTRL(*window->wx_graphics_window,
				 "GraphicsWindowTimeSlider", wxSlider);
			window->right_panel = XRCCTRL(*window->wx_graphics_window,
				 "GraphicsRightPanel", wxPanel);
			window->time_editor_panel = XRCCTRL(*window->wx_graphics_window,
				 "TimeEditorPanel", wxPanel);
			window->time_framerate_text_ctrl = XRCCTRL(*window->wx_graphics_window,
				 "GraphicsWindowTimeFramerateTextCtrl", wxTextCtrl);
			window->time_step_size_text_ctrl = XRCCTRL(*window->wx_graphics_window,
				 "GraphicsWindowTimeStepSizeTextCtrl", wxTextCtrl);
			window->time_play_every_frame_checkbox = XRCCTRL(*window->wx_graphics_window,
				 "GraphcisWindowTimePlayEveryFrameCheckBox", wxCheckBox);

			window->minimum_time = window->time_keeper_app->getTimeKeeper()->getMinimum();
			window->maximum_time = window->time_keeper_app->getTimeKeeper()->getMaximum();
			window->time_step = 0.1;
			if (window->time_keeper_app->getTimeKeeper()->hasTimeObject())
			{
				 window->time_editor_panel->Show(1);
				 window->time_editor_togglebutton->SetValue(1);
			}
			window->time_keeper_app->addCallback(
				 Graphics_window_time_keeper_app_callback,
				 (void *)window,
				 (enum Time_keeper_app_event) (TIME_KEEPER_APP_NEW_TIME |
						TIME_KEEPER_APP_NEW_MINIMUM | TIME_KEEPER_APP_NEW_MAXIMUM ));
			Graphics_window_time_keeper_app_callback(window->time_keeper_app, TIME_KEEPER_APP_NEW_TIME,
				 (void *)window);
			Graphics_window_update_time_settings_wx(window, (void *)NULL);

			USE_PARAMETER(add_interactive_tool_to_wx_toolbar);

			/* make sure the first scene_viewer shows */
			if (window->panel)
			{
				graphics_buffer = create_Graphics_buffer_wx(
					graphics_buffer_package,
					window->panel, graphics_buffer_buffering_mode, graphics_buffer_stereo_mode,
					minimum_colour_buffer_depth, minimum_depth_buffer_depth,
					minimum_accumulation_buffer_depth, (Graphics_buffer_app *)NULL);
				 if (graphics_buffer)
				 {
						/* create one Scene_viewers */
						window->number_of_scene_viewers = 1;
						if (ALLOCATE(window->scene_viewer_array,
									struct Scene_viewer_app *,
									window->number_of_scene_viewers))
						{
							 pane_no = 0;
							 cmzn_scenefilter_id filter = cmzn_scenefiltermodule_get_default_scenefilter(
								 window->filter_module);
							 window->scene_viewer_array[pane_no] = CREATE(Scene_viewer_app)(graphics_buffer,
								window->sceneviewermodule, filter, window->scene, user_interface);
							 cmzn_scenefilter_destroy(&filter);
							 if (window->scene_viewer_array[pane_no])
							 {
									ortho_up_axis=window->ortho_up_axis;
									ortho_front_axis=window->ortho_front_axis;
									window->ortho_up_axis=0;
									window->ortho_front_axis=0;
									Graphics_window_set_orthographic_axes(window,
										 ortho_up_axis,ortho_front_axis);
									window->interactive_toolbar_panel =
										 XRCCTRL(*window->wx_graphics_window, "ToolbarPanel", wxPanel);
									wxBoxSizer *toolbar_sizer = new wxBoxSizer( wxVERTICAL );
									window->interactive_toolbar_panel->SetSizer(toolbar_sizer);
									FOR_EACH_OBJECT_IN_MANAGER(Interactive_tool)(
										 add_interactive_tool_to_wx_toolbar,
										 (void *)window,
										 interactive_tool_manager);
									Scene_viewer_set_interactive_tool(
										 window->scene_viewer_array[pane_no],
										 window->interactive_tool);
									/* get scene_viewer transform callbacks to allow
										 synchronising of views in multiple panes */
									Scene_viewer_app_add_sync_callback(
										 window->scene_viewer_array[pane_no],
										 Graphics_window_Scene_viewer_view_changed,
										 window);
									cmzn_sceneviewer_set_translation_rate(
										 window->scene_viewer_array[pane_no]->core_scene_viewer, 2.0);
									cmzn_sceneviewer_set_tumble_rate(
										 window->scene_viewer_array[pane_no]->core_scene_viewer, 1.5);
									cmzn_sceneviewer_set_zoom_rate(
										 window->scene_viewer_array[pane_no]->core_scene_viewer, 2.0);
									if (cmzn_sceneviewer_get_lookat_parameters(
										window->scene_viewer_array[0]->core_scene_viewer, eye, lookat, up) &&
										axis_number_to_axis_vector(window->ortho_up_axis, up) &&
										axis_number_to_axis_vector(window->ortho_front_axis, front))
									{
										 view[0]=eye[0]-lookat[0];
										 view[1]=eye[1]-lookat[1];
										 view[2]=eye[2]-lookat[2];
										 eye_distance=normalize3(view);
										 Scene_viewer_set_lookat_parameters(
												window->scene_viewer_array[0]->core_scene_viewer,
												lookat[0]+eye_distance*front[0],
												lookat[1]+eye_distance*front[1],
												lookat[2]+eye_distance*front[2],
												lookat[0],lookat[1],lookat[2],up[0],up[1],up[2]);
									}
									/* set the initial layout */
									Graphics_window_set_layout_mode(window,
										 GRAPHICS_WINDOW_LAYOUT_SIMPLE);
									/* give the window its default size */
									int temp_width, temp_height;
									window->wx_graphics_window->GetSize(&temp_width, &temp_height);
									wxSplitterWindow *splitterWindow = XRCCTRL(*window->wx_graphics_window,"GraphicsWindowSplitter", wxSplitterWindow);
									splitterWindow->SetSize(temp_width, temp_height);
									splitterWindow->SetSashPosition(200);
									Graphics_window_set_viewing_area_size(window,
										 window->default_viewing_width,
										 window->default_viewing_height);
									/* initial view is of all of the current scene */
									Graphics_window_view_all(window);
									window->wx_graphics_window->Show();
									window->wx_perspective_button->SetValue((
										SCENE_VIEWER_PERSPECTIVE == Graphics_window_get_projection_mode(window, 0)));
							 }
							 else
							 {
									display_message(ERROR_MESSAGE,
										 "CREATE(Graphics_window).  "
										 "Could not create scene_viewer.");
									DEACCESS(Graphics_window)(&window);
									window = (struct Graphics_window *)NULL;
							 }
						}
						else
						{
							 display_message(ERROR_MESSAGE,
									"CREATE(Graphics_window).  "
									"Could not allocate memory for scene viewer array.");
							 DEACCESS(Graphics_window)(&window);
							 window = (struct Graphics_window *)NULL;
						}
						DEACCESS(Graphics_buffer_app)(&graphics_buffer);
				 }
				 else
				 {
						display_message(ERROR_MESSAGE,
							 "CREATE(Graphics_window).  "
							 "Could not create graphics buffer.");
						DEACCESS(Graphics_window)(&window);
						window = (struct Graphics_window *)NULL;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"CREATE(Graphics_window).  "
					"Unable to fetch panel widget.");
				DEACCESS(Graphics_window)(&window);
				window = (struct Graphics_window *)NULL;
			}
#elif defined (CARBON_USER_INTERFACE) /* switch (USER_INTERFACE) */
			USE_PARAMETER(graphics_buffer);
			USE_PARAMETER(pane_no);
			USE_PARAMETER(minimum_colour_buffer_depth);
			USE_PARAMETER(minimum_depth_buffer_depth);
			USE_PARAMETER(minimum_accumulation_buffer_depth);
			USE_PARAMETER(Graphics_window_Scene_viewer_view_changed);

			WindowRef carbon_window;
			WindowAttributes  window_attributes;
			Rect content_rectangle;
			CFStringRef title_key;
			CFStringRef carbon_window_title;
			OSStatus result;

			window_attributes = kWindowResizableAttribute
				| kWindowCloseBoxAttribute
				| kWindowLiveResizeAttribute
				| kWindowStandardHandlerAttribute
				| kWindowStandardDocumentAttributes
				| kWindowFullZoomAttribute;

			content_rectangle.left = 10;
			content_rectangle.top = 10;
			content_rectangle.right = 500;
			content_rectangle.bottom = 500;

			CreateNewWindow (kDocumentWindowClass, window_attributes,
				&content_rectangle, &carbon_window);

			title_key = CFStringCreateWithCString (
				kCFAllocatorDefault, window_title,
				kCFStringEncodingMacRoman);

			carbon_window_title = CFCopyLocalizedString(title_key, NULL);

			result = SetWindowTitleWithCFString (carbon_window, carbon_window_title);

			CFRelease (title_key);

			CFRelease (carbon_window_title);


			if (graphics_buffer = create_Graphics_buffer_Carbon(
					 graphics_buffer_package,
					 carbon_window,
					 graphics_buffer_buffering_mode, graphics_buffer_stereo_mode,
					 minimum_colour_buffer_depth, minimum_depth_buffer_depth,
					 minimum_accumulation_buffer_depth))
			{
						/* create one Scene_viewers */
						window->number_of_scene_viewers = 1;
						if (ALLOCATE(window->scene_viewer_array,
							struct Scene_viewer *,
							window->number_of_scene_viewers))
						{
							pane_no = 0;
							if (window->scene_viewer_array[pane_no] =
								 CREATE(Scene_viewer)(graphics_buffer,
								 background_colour, lightmodule,default_light,
								 light_model_module,default_light_model,
								 filter_module, window->scene,
								 user_interface))
							{
								Scene_viewer_set_interactive_tool(
									window->scene_viewer_array[pane_no],
									window->interactive_tool);
								/* get scene_viewer transform callbacks to allow
									synchronising of views in multiple panes */
								Scene_viewer_app_add_sync_callback(
									window->scene_viewer_array[pane_no],
									Graphics_window_Scene_viewer_view_changed,
									window);
								cmzn_sceneviewer_set_translation_rate(
									window->scene_viewer_array[pane_no], 2.0);
								cmzn_sceneviewer_set_tumble_rate(
									window->scene_viewer_array[pane_no], 1.5);
								cmzn_sceneviewer_set_zoom_rate(
									window->scene_viewer_array[pane_no], 2.0);

								/* set the initial layout */
								Graphics_window_set_layout_mode(window,
									GRAPHICS_WINDOW_LAYOUT_SIMPLE);
								/* give the window its default size */
								Graphics_window_set_viewing_area_size(window,
									window->default_viewing_width,
									window->default_viewing_height);
								/* initial view is of all of the current scene */
								Graphics_window_view_all(window);

							}
							else
							{
								display_message(ERROR_MESSAGE,
									"CREATE(Graphics_window).  "
									"Could not create scene_viewer.");
								DEACCESS(Graphics_window)(&window);
								window = (struct Graphics_window *)NULL;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"CREATE(Graphics_window).  "
								"Could not allocate memory for scene viewer array.");
							DEACCESS(Graphics_window)(&window);
							window = (struct Graphics_window *)NULL;
						}
			}



			RepositionWindow (carbon_window, NULL, kWindowCascadeOnMainScreen);

			ShowWindow (carbon_window);

#endif /* switch (USER_INTERFACE) */
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"CREATE(graphics_window).  Insufficient memory for graphics window");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"CREATE(graphics_window).  Invalid argument(s)");
		window=(struct Graphics_window *)NULL;
	}
	LEAVE;

	return (window);
} /* CREATE(graphics_window) */

int DESTROY(Graphics_window)(struct Graphics_window **graphics_window_address)
/*******************************************************************************
LAST MODIFIED : 9 May 2000

DESCRIPTION:
Frees the contents of the Graphics_window structure and then the object itself,
then closes down the window shell and widgets it uses. Note that responsibility
for removing the graphics_window from a global list of windows is left with the
calling routine. See also Graphics_window_close_CB and
Graphics_window_destroy_CB.
==============================================================================*/
{
	int return_code;
	struct Graphics_window *window;

	ENTER(DESTROY(graphics_window));
	if (graphics_window_address&&(window= *graphics_window_address))
	{
#if !defined (WX_USER_INTERFACE)
		 /* the class wxGraphicsWindow destructor will handle the
				destruction of the scene viewers. */
		 int pane_no;
		if (window->scene_viewer_array)
		{
			/* close the Scene_viewer(s) */
			for (pane_no=0;pane_no<window->number_of_scene_viewers;pane_no++)
			{
				DESTROY(Scene_viewer)(&(window->scene_viewer_array[pane_no]));
			}
			DEALLOCATE(window->scene_viewer_array);
		}
#endif /* !defined (WX_USER_INTERFACE) */
		if (window->filter_module)
		{
			cmzn_scenefiltermodule_destroy(&window->filter_module);
		}
#if defined (WX_USER_INTERFACE)
		if (window->interactive_tool_manager)
		{
			 DESTROY(MANAGER(Interactive_tool))(&window->interactive_tool_manager);
		}
#endif /* (WX_USER_INTERFACE) */
#if defined (WIN32_USER_INTERFACE) /* switch (USER_INTERFACE) */
		DestroyWindow(window->hWnd);
#elif defined (GTK_USER_INTERFACE) /* switch (USER_INTERFACE) */
#if GTK_MAJOR_VERSION >= 2
		g_signal_handler_disconnect (G_OBJECT(window->shell_window),
			window->close_handler_id);
#endif /* GTK_MAJOR_VERSION >= 2 */
		gtk_widget_destroy (window->shell_window);
#endif /* switch (USER_INTERFACE) */
		/* no longer accessing scene */
		cmzn_scene_destroy(&(window->scene));
		/* Time keeper callback will be removed throught the destructor of
			 the wx_graphics_window along with the scene viewer so that
			 whenever user closes the window, the scene viewer and the
			 timecallback will be removed immediately and will not cause any
			 crashes occur as it may before this bug fix. The old behaviour is to wait
			 till the user terminate the whole program and then destroy all
			 graphics window component which led to a problem when the
			 callback try to draw something to the already destroyed
			 wxGLCanvas in the scene viewer before terminating the program
			 and have a already closed graphics window. */
		if(window->time_keeper_app)
		{
			DEACCESS(Time_keeper_app)(&(window->time_keeper_app));
		}
		if (window->root_region)
		{
		 cmzn_region_destroy(&window->root_region);
		}
		cmzn_sceneviewermodule_destroy(&window->sceneviewermodule);
#if defined (WX_USER_INTERFACE)
		if (window->wx_graphics_window)
		{
			 delete (window->wx_graphics_window);
			 window->wx_graphics_window = NULL;
		}
#endif /* !defined (WX_USER_INTERFACE) */
		DEALLOCATE(window->name);
		DEALLOCATE(*graphics_window_address);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(graphics_window).  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(graphics_window) */

DECLARE_OBJECT_FUNCTIONS(Graphics_window)
DECLARE_DEFAULT_GET_OBJECT_NAME_FUNCTION(Graphics_window)

DECLARE_INDEXED_LIST_FUNCTIONS(Graphics_window)
DECLARE_FIND_BY_IDENTIFIER_IN_INDEXED_LIST_FUNCTION(Graphics_window, \
	name,const char *,strcmp)
DECLARE_INDEXED_LIST_IDENTIFIER_CHANGE_FUNCTIONS(Graphics_window,name)

PROTOTYPE_MANAGER_COPY_WITH_IDENTIFIER_FUNCTION(Graphics_window,name)
{
	const char *name;
	int return_code;

	ENTER(MANAGER_COPY_WITH_IDENTIFIER(Graphics_window,name));
	/* check arguments */
	if (source&&destination)
	{
		if (source->name)
		{
			if (ALLOCATE(name,char,strlen(source->name)+1))
			{
				strcpy((char *)name,source->name);
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"MANAGER_COPY_WITH_IDENTIFIER(Graphics_window,name).  "
					"Insufficient memory");
				return_code=0;
			}
		}
		else
		{
			name=(const char *)NULL;
			return_code=1;
		}
		if (return_code)
		{
			return_code = MANAGER_COPY_WITHOUT_IDENTIFIER(Graphics_window,name)(destination,source);
			if (return_code)
			{
				/* copy values */
				DEALLOCATE(destination->name);
				destination->name=name;
			}
			else
			{
				DEALLOCATE(name);
				display_message(ERROR_MESSAGE,
					"MANAGER_COPY_WITH_IDENTIFIER(Graphics_window,name).  "
					"Could not copy without identifier");
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"MANAGER_COPY_WITH_IDENTIFIER(Graphics_window,name).  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* MANAGER_COPY_WITH_IDENTIFIER(Graphics_window,name) */

PROTOTYPE_MANAGER_COPY_WITHOUT_IDENTIFIER_FUNCTION(Graphics_window,name)
{
	int return_code;

	ENTER(MANAGER_COPY_WITHOUT_IDENTIFIER(Graphics_window,name));
	if (source&&destination)
	{
		/*???RC have problems with copying filter_module? messages? */
		printf("MANAGER_COPY_WITHOUT_IDENTIFIER(Graphics_window,name).  "
			"Not used\n");
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"MANAGER_COPY_WITHOUT_IDENTIFIER(Graphics_window,name).  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* MANAGER_COPY_WITHOUT_IDENTIFIER(Graphics_window,name) */

PROTOTYPE_MANAGER_COPY_IDENTIFIER_FUNCTION(Graphics_window,name,const char *)
{
	char *destination_name;
	int return_code;

	ENTER(MANAGER_COPY_IDENTIFIER(Graphics_window,name));
	if (name&&destination)
	{
		if (ALLOCATE(destination_name,char,strlen(name)+1))
		{
			strcpy(destination_name,name);
			if (destination->name)
			{
				DEALLOCATE(destination->name);
			}
			destination->name=destination_name;
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"MANAGER_COPY_IDENTIFIER(Graphics_window,name).  Insufficient memory");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"MANAGER_COPY_IDENTIFIER(Graphics_window,name).  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* MANAGER_COPY_IDENTIFIER(Graphics_window,name) */

DECLARE_MANAGER_FUNCTIONS(Graphics_window,graphics_window_manager)

DECLARE_DEFAULT_MANAGED_OBJECT_NOT_IN_USE_FUNCTION(Graphics_window,graphics_window_manager)

DECLARE_MANAGER_IDENTIFIER_FUNCTIONS(Graphics_window,name, \
	const char *,graphics_window_manager)

char *Graphics_window_manager_get_new_name(
	struct MANAGER(Graphics_window) *graphics_window_manager)
/*******************************************************************************
LAST MODIFIED : 13 October 1998

DESCRIPTION :
Makes up a default name string for a graphics window, based on numbers and
starting at "1". Up to the calling routine to deallocate the returned string.
==============================================================================*/
{
	char *return_name,temp_name[10];
	int number;

	ENTER(Graphics_window_manager_get_new_name);
	if (graphics_window_manager)
	{
		number=1;
		sprintf(temp_name,"%d",number);
		while (FIND_BY_IDENTIFIER_IN_MANAGER(Graphics_window,name)(temp_name,
			graphics_window_manager))
		{
			number++;
			sprintf(temp_name,"%d",number);
		}
		if (ALLOCATE(return_name,char,strlen(temp_name)+1))
		{
			strcpy(return_name,temp_name);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_window_manager_get_new_name.  Missing graphics_window_manager");
		return_name=(char *)NULL;
	}
	LEAVE;

	return (return_name);
} /* Graphics_window_manager_get_new_name */

int Graphics_window_get_current_pane(struct Graphics_window *window)
/*******************************************************************************
LAST MODIFIED : 6 October 1998

DESCRIPTION :
Returns the current_pane of the <window>, from 0 to number_of_panes-1.
==============================================================================*/
{
	int current_pane;

	ENTER(Graphics_window_get_current_pane);
	if (window)
	{
		current_pane=window->current_pane;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_window_get_current_pane.  Invalid argument(s)");
		current_pane=0;
	}
	LEAVE;

	return (current_pane);
} /* Graphics_window_get_current_pane */

int Graphics_window_set_current_pane(struct Graphics_window *window,
	int pane_no)
/*******************************************************************************
LAST MODIFIED : 15 October 1998

DESCRIPTION :
Sets the current_pane of the <window> to <pane_no>, from 0 to number_of_panes-1.
==============================================================================*/
{
	enum Scene_viewer_projection_mode projection_mode;
	int return_code;

	ENTER(Graphics_window_set_current_pane);
	if (window&&window->scene_viewer_array&&(0<=pane_no)&&(pane_no<window->number_of_panes))
	{
		window->current_pane=pane_no;
		/* make sure the parallel/perspective button is set up for pane */
		Scene_viewer_get_projection_mode(window->scene_viewer_array[pane_no]->core_scene_viewer,
			&projection_mode);
		Graphics_window_set_projection_mode(window,pane_no,projection_mode);

		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_window_set_current_pane.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Graphics_window_set_current_pane */

double Graphics_window_get_eye_spacing(struct Graphics_window *graphics_window)
/*******************************************************************************
LAST MODIFIED : 15 October 1998

DESCRIPTION :
Returns the eye_spacing from the <graphics_window> used for 3-D viewing.
==============================================================================*/
{
	double eye_spacing;

	ENTER(Graphics_window_get_eye_spacing);
	if (graphics_window)
	{
		eye_spacing=graphics_window->eye_spacing;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_window_get_eye_spacing.  Missing graphics window");
		eye_spacing=0.0;
	}
	LEAVE;

	return (eye_spacing);
} /* Graphics_window_get_eye_spacing */

int Graphics_window_set_eye_spacing(struct Graphics_window *graphics_window,
	double eye_spacing)
/*******************************************************************************
LAST MODIFIED : 13 November 1998

DESCRIPTION :
Sets the <eye_spacing> for the <graphics_window> used for 3-D viewing.
==============================================================================*/
{
	int pane_no,return_code;

	ENTER(Graphics_window_set_eye_spacing);
	if (graphics_window && graphics_window->scene_viewer_array)
	{
		graphics_window->eye_spacing=eye_spacing;
		/* Set this on all the scene viewers too */
		for (pane_no=0;pane_no<graphics_window->number_of_scene_viewers;pane_no++)
		{
			Scene_viewer_set_stereo_eye_spacing(graphics_window->scene_viewer_array[pane_no]->core_scene_viewer,
				eye_spacing);
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_window_set_eye_spacing.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Graphics_window_set_eye_spacing */

enum Scene_viewer_input_mode Graphics_window_get_input_mode(
	struct Graphics_window *window)
/*******************************************************************************
LAST MODIFIED : 6 October 1998

DESCRIPTION :
Returns the current input mode of the graphics window. Valid return values are
SCENE_VIEWER_NO_INPUT, SCENE_VIEWER_SELECT and SCENE_VIEWER_TRANSFORM.
==============================================================================*/
{
	enum Scene_viewer_input_mode input_mode;

	ENTER(Graphics_window_get_input_mode);
	if (window)
	{
		input_mode=window->input_mode;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_window_get_input_mode.  Invalid argument(s)");
		/* return any old value */
		input_mode=SCENE_VIEWER_NO_INPUT;
	}
	LEAVE;

	return (input_mode);
} /* Graphics_window_get_input_mode */

int Graphics_window_set_input_mode(struct Graphics_window *window,
	enum Scene_viewer_input_mode input_mode)
/*******************************************************************************
LAST MODIFIED : 6 October 1998

DESCRIPTION :
Sets the current input mode of the <window> to <input_mode>. Valid input_modes
are SCENE_VIEWER_NO_INPUT, SCENE_VIEWER_SELECT and SCENE_VIEWER_TRANSFORM.
==============================================================================*/
{
	int pane_no,return_code;

	ENTER(Graphics_window_set_input_mode);
	if (window&&window->scene_viewer_array&&((SCENE_VIEWER_NO_INPUT==input_mode)||
		(SCENE_VIEWER_SELECT==input_mode)||(SCENE_VIEWER_TRANSFORM==input_mode)))
	{
		return_code=1;
		for (pane_no=0;pane_no<window->number_of_scene_viewers;pane_no++)
		{
			Scene_viewer_set_input_mode(window->scene_viewer_array[pane_no]->core_scene_viewer,input_mode);
		}
		if (input_mode != window->input_mode)
		{
			window->input_mode=input_mode;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_window_set_input_mode.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Graphics_window_set_input_mode */

enum Graphics_window_layout_mode Graphics_window_get_layout_mode(
	struct Graphics_window *window)
/*******************************************************************************
LAST MODIFIED : 28 June 2000

DESCRIPTION :
Returns the layout mode in effect on the <window>.
==============================================================================*/
{
	enum Graphics_window_layout_mode layout_mode;

	ENTER(Graphics_window_get_layout_mode);
	if (window)
	{
		layout_mode=window->layout_mode;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_window_get_layout_mode.  Invalid argument(s)");
		layout_mode=GRAPHICS_WINDOW_LAYOUT_MODE_INVALID;
	}
	LEAVE;

	return (layout_mode);
} /* Graphics_window_get_layout_mode */

int Graphics_window_set_layout_mode(struct Graphics_window *window,
	enum Graphics_window_layout_mode layout_mode)
/*******************************************************************************
LAST MODIFIED : 29 September 2000

DESCRIPTION :
Sets the layout mode in effect on the <window>.
==============================================================================*/
{
#if defined (WX_USER_INTERFACE)
	double bottom,clip_factor,far_plane,left,
		near_plane,radius,right,top;
#endif /* defined (WX_USER_INTERFACE) */
	double eye[3],eye_distance,front[3],lookat[3],up[3],view[3];
	enum Scene_viewer_projection_mode projection_mode;
	int new_layout,new_number_of_panes,pane_no,return_code;
#if defined (WX_USER_INTERFACE)
	enum cmzn_sceneviewer_transparency_mode transparency_mode;
	int perturb_lines;
	struct Graphics_buffer_app *graphics_buffer;
	int transparency_layers;
#endif /* defined (WX_USER_INTERFACE) */
#if defined (WX_USER_INTERFACE)
	wxPanel* current_panel = NULL;
#endif /* defined (WX_USER_INTERFACE) */

	ENTER(Graphics_window_set_layout_mode);
	if (window&&window->scene_viewer_array)
	{
		return_code=1;
		new_number_of_panes =
			Graphics_window_layout_mode_get_number_of_panes(layout_mode);
		if (new_number_of_panes > window->number_of_scene_viewers)
		{
#if defined (WX_USER_INTERFACE)
			struct Scene_viewer_app *first_scene_viewer_app = window->scene_viewer_array[0];
			cmzn_sceneviewer *first_sceneviewer = first_scene_viewer_app->core_scene_viewer;
			cmzn_sceneviewer_get_lookat_parameters(first_sceneviewer, eye, lookat, up) &&
			Scene_viewer_get_viewing_volume(first_sceneviewer,
				&left, &right, &bottom, &top, &near_plane, &far_plane);
			radius = 0.5*(right - left);

			if (REALLOCATE(window->scene_viewer_array,
					window->scene_viewer_array, struct Scene_viewer_app *,
					new_number_of_panes))
			{
				for (pane_no = window->number_of_scene_viewers ;
					  return_code && (pane_no < new_number_of_panes) ; pane_no++)
				{
					switch (pane_no)
					{
						/* First viewing area is pane_no 0 */
						case 1:
						{
							current_panel = window->panel2;
						} break;
						case 2:
						{
							current_panel = window->panel3;
						} break;
						case 3:
						{
							current_panel = window->panel4;
						} break;
						default:
						{
							display_message(ERROR_MESSAGE, "Graphics_window_set_layout_mode.  "
								"Invalid pane to create");
							return_code = 0;
						} break;
					}
					// create_Graphics_buffer_wx will eventually create a
					// graphics buffer from another buffer if a buffer_to_match
					// is passed in.
					graphics_buffer = create_Graphics_buffer_wx(window->graphics_buffer_package,
						current_panel, GRAPHICS_BUFFER_DOUBLE_BUFFERING, GRAPHICS_BUFFER_ANY_STEREO_MODE,
						/*minimum_colour_buffer_depth*/24, /*minimum_depth_buffer_depth*/16,
						/*minimum_accumulation_buffer_depth*/0, /*buffer_to_match*/0);
					if (graphics_buffer)
					{
						cmzn_scenefilter_id filter = cmzn_sceneviewer_get_scenefilter(first_sceneviewer);
						window->scene_viewer_array[pane_no]=
							CREATE(Scene_viewer_app)(graphics_buffer,	window->sceneviewermodule,
								filter,window->scene, window->user_interface);
						cmzn_scenefilter_destroy(&filter);
						if (window->scene_viewer_array[pane_no])
						{
							cmzn_sceneviewer *pane_sceneviewer = window->scene_viewer_array[pane_no]->core_scene_viewer;
							Scene_viewer_set_interactive_tool(
								window->scene_viewer_array[pane_no],
								window->interactive_tool);
							if (Interactive_tool_is_Transform_tool(window->interactive_tool))
							{
								Scene_viewer_set_input_mode(pane_sceneviewer,SCENE_VIEWER_TRANSFORM);
							}
							else
							{
								Scene_viewer_set_input_mode(pane_sceneviewer,SCENE_VIEWER_SELECT);
							}
							/* get scene_viewer transform callbacks to allow
								synchronising of views in multiple panes */
							Scene_viewer_app_add_sync_callback(
								window->scene_viewer_array[pane_no],
								Graphics_window_Scene_viewer_view_changed,
								window);
							cmzn_sceneviewer_set_translation_rate(
								pane_sceneviewer,
								window->default_translate_rate);
							cmzn_sceneviewer_set_tumble_rate(
								pane_sceneviewer,
								window->default_tumble_rate);
							cmzn_sceneviewer_set_zoom_rate(
								pane_sceneviewer,
								window->default_zoom_rate);
							clip_factor = 4.0;
							Scene_viewer_set_view_simple(
								pane_sceneviewer,
								lookat[0], lookat[1], lookat[2],
								radius, window->std_view_angle, clip_factor*radius);
							perturb_lines = cmzn_sceneviewer_get_perturb_lines_flag(first_sceneviewer);
							cmzn_sceneviewer_set_perturb_lines_flag(pane_sceneviewer,
								0 != perturb_lines);
							cmzn_sceneviewer_set_lighting_local_viewer(pane_sceneviewer,
								cmzn_sceneviewer_is_lighting_local_viewer(first_sceneviewer));
							cmzn_sceneviewer_set_lighting_two_sided(pane_sceneviewer,
								cmzn_sceneviewer_is_lighting_two_sided(first_sceneviewer));
							transparency_layers = cmzn_sceneviewer_get_transparency_layers(first_sceneviewer);
							cmzn_sceneviewer_set_transparency_layers(pane_sceneviewer,
								transparency_layers);
							transparency_mode = cmzn_sceneviewer_get_transparency_mode(
								first_sceneviewer);
							cmzn_sceneviewer_set_transparency_mode(pane_sceneviewer,
								transparency_mode);
							cmzn_sceneviewer_set_antialias_sampling(
								pane_sceneviewer,window->antialias_mode);
						}
						else
						{
							return_code=0;
						}
					}
					else
					{
						return_code = 0;
					}
				}
				window->number_of_scene_viewers = new_number_of_panes;
			}
			else
			{
				return_code = 0;
			}
#else
			display_message(ERROR_MESSAGE,
				"Graphics_window_set_layout_mode.  "
				"More than one scene viewer in the graphics window not implemented for this version.");
			return_code=0;
#endif /* defined (WX_USER_INTERFACE) */
		}
		if (return_code)
		{
			new_layout=(layout_mode != window->layout_mode);
			if (new_layout)
			{
				window->layout_mode=layout_mode;
				/* get the number of panes for the new layout */
				window->number_of_panes=
					Graphics_window_layout_mode_get_number_of_panes(layout_mode);
#if defined (WX_USER_INTERFACE)
				wxChoice *layout_choice = XRCCTRL(
					 *window->wx_graphics_window, "View", wxChoice);
				layout_choice->SetStringSelection(
					 wxString::FromAscii(Graphics_window_layout_mode_string(layout_mode)));
#endif /* defined (SWITCH_USER_INTERFACE) */
				/* awaken scene_viewers in panes to be used; put others to sleep */
				for (pane_no=0;pane_no<window->number_of_scene_viewers;pane_no++)
				{
					if (pane_no < window->number_of_panes)
					{
						Scene_viewer_awaken(window->scene_viewer_array[pane_no]->core_scene_viewer);
					}
					else
					{
						Scene_viewer_sleep(window->scene_viewer_array[pane_no]->core_scene_viewer);
					}
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE, "Graphics_window_set_layout_mode.  "
				"Insufficient scene_viewers for layout mode");
			/* Keep the old layout */
			new_layout = 0;
			layout_mode = window->layout_mode;
		}
		/* get projection mode of pane 0, using parallel for custom */
		if (SCENE_VIEWER_PERSPECTIVE==
			Graphics_window_get_projection_mode(window,0))
		{
			projection_mode=SCENE_VIEWER_PERSPECTIVE;
		}
		else
		{
			projection_mode=SCENE_VIEWER_PARALLEL;
		}
		switch (layout_mode)
		{
			case GRAPHICS_WINDOW_LAYOUT_SIMPLE:
			{
				if (new_layout)
				{
#if defined (WX_USER_INTERFACE)

					  window->panel2->Hide();
					  window->panel3->Hide();
					  window->panel4->Hide();

#endif /* defined (WX_USER_INTERFACE) */
					/* re-enable tumbling in main scene viewer */
					cmzn_sceneviewer_set_translation_rate(window->scene_viewer_array[0]->core_scene_viewer,
						window->default_translate_rate);
					cmzn_sceneviewer_set_tumble_rate(window->scene_viewer_array[0]->core_scene_viewer,
						window->default_tumble_rate);
					cmzn_sceneviewer_set_zoom_rate(window->scene_viewer_array[0]->core_scene_viewer,
						window->default_zoom_rate);
				}
			} break;
			case GRAPHICS_WINDOW_LAYOUT_2D:
			{
				if (new_layout)
				{


#if defined (WX_USER_INTERFACE)
					window->panel2->Hide();
					window->panel3->Hide();
					window->panel4->Hide();
					/* un-grey orthographic view controls */
					//XtSetSensitive(window->orthographic_form,True);
#endif /* defined (WX_USER_INTERFACE) */

					/* disable tumbling in main scene viewer */
					cmzn_sceneviewer_set_translation_rate(window->scene_viewer_array[0]->core_scene_viewer,
						window->default_translate_rate);
					cmzn_sceneviewer_set_tumble_rate(window->scene_viewer_array[0]->core_scene_viewer,
						/*tumble_rate*/0.0);
					cmzn_sceneviewer_set_zoom_rate(window->scene_viewer_array[0]->core_scene_viewer,
						window->default_zoom_rate);
				}
				/* set the plan view in pane 0 */
				if (cmzn_sceneviewer_get_lookat_parameters(window->scene_viewer_array[0]->core_scene_viewer, eye, lookat, up) &&
					axis_number_to_axis_vector(window->ortho_up_axis,up)&&
					axis_number_to_axis_vector(window->ortho_front_axis,front))
				{
					view[0]=eye[0]-lookat[0];
					view[1]=eye[1]-lookat[1];
					view[2]=eye[2]-lookat[2];
					eye_distance=normalize3(view);
					Scene_viewer_set_lookat_parameters(window->scene_viewer_array[0]->core_scene_viewer,
						lookat[0]+eye_distance*up[0],lookat[1]+eye_distance*up[1],
						lookat[2]+eye_distance*up[2],lookat[0],lookat[1],lookat[2],
						-front[0],-front[1],-front[2]);
					Scene_viewer_app_redraw_now(window->scene_viewer_array[0]);
				}
			} break;
			case GRAPHICS_WINDOW_LAYOUT_ORTHOGRAPHIC:
			case GRAPHICS_WINDOW_LAYOUT_FREE_ORTHO:
			{
				if (new_layout)
				{
#if defined (WX_USER_INTERFACE)
					window->panel2->Show();
					window->panel3->Show();
					window->panel4->Show();

					/* un-grey orthographic view controls */
					//XtSetSensitive(window->orthographic_form,True);
#endif /* defined (WX_USER_INTERFACE) */

					/* make sure pane 0 is not using a custom projection */
					Graphics_window_set_projection_mode(window,0,projection_mode);
					/* re-enable tumbling in main scene viewer */
					cmzn_sceneviewer_set_translation_rate(window->scene_viewer_array[0]->core_scene_viewer,
						window->default_translate_rate);
					cmzn_sceneviewer_set_tumble_rate(window->scene_viewer_array[0]->core_scene_viewer,
						window->default_tumble_rate);
					cmzn_sceneviewer_set_zoom_rate(window->scene_viewer_array[0]->core_scene_viewer,
						window->default_zoom_rate);
					/* make sure panes 1-3 use parallel projection and disable tumble */
					for (pane_no=1;pane_no<4;pane_no++)
					{
						Graphics_window_set_projection_mode(window,pane_no,
							SCENE_VIEWER_PARALLEL);
						cmzn_sceneviewer_set_translation_rate(window->scene_viewer_array[pane_no]->core_scene_viewer,
							window->default_translate_rate);
						cmzn_sceneviewer_set_tumble_rate(window->scene_viewer_array[pane_no]->core_scene_viewer,
							/*tumble_rate*/0.0);
						cmzn_sceneviewer_set_zoom_rate(window->scene_viewer_array[pane_no]->core_scene_viewer,
							window->default_zoom_rate);
					}
				}
				/* four views, 3 tied together as front, side and plan views */
				/* set the plan view in pane 1 */
				if (cmzn_sceneviewer_get_lookat_parameters(window->scene_viewer_array[1]->core_scene_viewer, eye, lookat, up) &&
					axis_number_to_axis_vector(window->ortho_up_axis,up)&&
					axis_number_to_axis_vector(window->ortho_front_axis,front))
				{
					view[0]=eye[0]-lookat[0];
					view[1]=eye[1]-lookat[1];
					view[2]=eye[2]-lookat[2];
					eye_distance=normalize3(view);
					Scene_viewer_set_lookat_parameters(window->scene_viewer_array[1]->core_scene_viewer,
						lookat[0]+eye_distance*up[0],lookat[1]+eye_distance*up[1],
						lookat[2]+eye_distance*up[2],lookat[0],lookat[1],lookat[2],
						-front[0],-front[1],-front[2]);
				}
				/* put tied views in proper relationship to plan view in pane 1 */
				Graphics_window_view_changed(window,1);
				/* Graphics_window_view_changed redraws the scene viewers that are
					 tied to pane 1, but not pane 1 itself, hence: */
				Scene_viewer_app_redraw_now(window->scene_viewer_array[1]);
			} break;
			case GRAPHICS_WINDOW_LAYOUT_FRONT_BACK:
			case GRAPHICS_WINDOW_LAYOUT_FRONT_SIDE:
			case GRAPHICS_WINDOW_LAYOUT_PSEUDO_3D:
			case GRAPHICS_WINDOW_LAYOUT_TWO_FREE:
			{
				if (new_layout)
				{

#if defined (WX_USER_INTERFACE)
					window->panel2->Show();
					window->panel3->Hide();
					window->panel4->Hide();
					/* un-grey orthographic view controls */
					//XtSetSensitive(window->orthographic_form,True);
#endif /* defined (WX_USER_INTERFACE) */
					/* make sure panes 0 and 1 use same projection, and enable tumble */
					for (pane_no=0;pane_no<2;pane_no++)
					{
						Graphics_window_set_projection_mode(window,pane_no,projection_mode);
						cmzn_sceneviewer_set_translation_rate(window->scene_viewer_array[pane_no]->core_scene_viewer,
							window->default_translate_rate);
						cmzn_sceneviewer_set_tumble_rate(window->scene_viewer_array[pane_no]->core_scene_viewer,
							window->default_tumble_rate);
						cmzn_sceneviewer_set_zoom_rate(window->scene_viewer_array[pane_no]->core_scene_viewer,
							window->default_zoom_rate);
					}
#if defined (WX_USER_INTERFACE)
					//XtVaSetValues(window->viewing_area1,
					//	XmNrightPosition,1,XmNbottomPosition,2,NULL);
					//XtVaSetValues(window->viewing_area2,
					//	XmNbottomPosition,2,NULL);
					window->panel2->Show();
					window->panel3->Hide();
					window->panel4->Hide();
					//     window->lowerpanels->SetFlag(0);

					/* un-grey orthographic view controls */
					//XtSetSensitive(window->orthographic_form,True);
#endif /* defined (WX_USER_INTERFACE) */
				}
				if ((GRAPHICS_WINDOW_LAYOUT_FRONT_BACK==layout_mode)||
					(GRAPHICS_WINDOW_LAYOUT_FRONT_SIDE==layout_mode))
				{
					/* set the front view in pane 0 */
					if (cmzn_sceneviewer_get_lookat_parameters(window->scene_viewer_array[0]->core_scene_viewer, eye, lookat, up) &&
						axis_number_to_axis_vector(window->ortho_up_axis,up)&&
						axis_number_to_axis_vector(window->ortho_front_axis,front))
					{
						view[0]=eye[0]-lookat[0];
						view[1]=eye[1]-lookat[1];
						view[2]=eye[2]-lookat[2];
						eye_distance=normalize3(view);
						Scene_viewer_set_lookat_parameters(window->scene_viewer_array[0]->core_scene_viewer,
							lookat[0]+eye_distance*front[0],lookat[1]+eye_distance*front[1],
							lookat[2]+eye_distance*front[2],lookat[0],lookat[1],lookat[2],
							up[0],up[1],up[2]);
					}
					Scene_viewer_app_redraw_now(window->scene_viewer_array[0]);
				}
				/* put tied views in proper relationship to front view in pane 0 */
				Graphics_window_view_changed(window,0);
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"Graphics_window_set_layout_mode.  Unknown layout_mode");
				return_code=0;
			} break;
		}
		/* Always set current_pane to the pane 0 when layout re-established */
		Graphics_window_set_current_pane(window,0);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_window_set_layout_mode.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Graphics_window_set_layout_mode */

int Graphics_window_get_orthographic_axes(struct Graphics_window *window,
	int *ortho_up_axis,int *ortho_front_axis)
/*******************************************************************************
LAST MODIFIED : 13 October 1998

DESCRIPTION :
Returns the "up" and "front" axes of the graphics window.
Axis numbers are from 1 to 6, where 1=x, 2=y, 3=z, 4=-x, 5=-y and 6=-z.
==============================================================================*/
{
	int return_code;

	ENTER(Graphics_window_get_orthographic_axes);
	if (window&&ortho_up_axis&&ortho_front_axis)
	{
		*ortho_up_axis = window->ortho_up_axis;
		*ortho_front_axis = window->ortho_front_axis;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_window_get_orthographic_axes.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Graphics_window_get_orthographic_axes */

int Graphics_window_set_orthographic_axes(struct Graphics_window *window,
	int ortho_up_axis,int ortho_front_axis)
/*******************************************************************************
LAST MODIFIED : 8 October 1998

DESCRIPTION :
Sets the "up" and "front" axes of the graphics window. Used for layout_modes
such as GRAPHICS_WINDOW_LAYOUT_ORTHOGRAPHIC.
Axis numbers are from 1 to 6, where 1=x, 2=y, 3=z, 4=-x, 5=-y and 6=-z.
==============================================================================*/
{
	int return_code = 0;

	ENTER(Graphics_window_set_orthographic_axes);
	if (window)
	{
		if (window->ortho_up_axis != ortho_up_axis)
		{
			window->ortho_up_axis=ortho_up_axis;
		}
		if (window->ortho_front_axis != ortho_front_axis)
		{
			window->ortho_front_axis=ortho_front_axis;
		}
		/* Ensure the up and front axes are legal... */
		if (!window->ortho_up_axis)
		{
			window->ortho_up_axis=3;
		}
		if (!window->ortho_front_axis)
		{
			window->ortho_front_axis=5;
		}
		/* ...and orthogonal */
		if ((window->ortho_up_axis % 3)==(window->ortho_front_axis % 3))
		{
			window->ortho_front_axis=(window->ortho_front_axis % 6)+1;
		}
		/* update the widgets if values have changed in this function */
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_window_set_orthographic_axes.  Missing graphics_window");
		return_code=0;
	}
	LEAVE;

	return return_code;
} /* Graphics_window_set_orthographic_axes */

enum Scene_viewer_projection_mode Graphics_window_get_projection_mode(
	struct Graphics_window *window,int pane_no)
/*******************************************************************************
LAST MODIFIED : 5 October 1998

DESCRIPTION :
Returns the projection mode used by pane <pane_no> of <window>.
==============================================================================*/
{
	enum Scene_viewer_projection_mode projection_mode;

	ENTER(Graphics_window_get_projection_mode);
	if (window&&window->scene_viewer_array&&(0<=pane_no)&&
		 (pane_no<window->number_of_panes)&&
		(window->scene_viewer_array[pane_no]))
	{
		Scene_viewer_get_projection_mode(window->scene_viewer_array[pane_no]->core_scene_viewer,
			&projection_mode);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_window_get_projection_mode.  Invalid argument(s)");
		projection_mode=SCENE_VIEWER_PARALLEL;
	}
	LEAVE;

	return (projection_mode);
} /* Graphics_window_get_projection_mode */

struct MANAGER(Interactive_tool) *Graphics_window_get_interactive_tool_manager(struct Graphics_window *window)
/*******************************************************************************
LAST MODIFIED : 27 March 2007

DESCRIPTION :
Returns the interactive_tool_manager.
==============================================================================*/
{
	ENTER(Graphics_window_get_interactive_tool_manager);
	return (window->interactive_tool_manager);
	LEAVE;
}


int Graphics_window_set_projection_mode(struct Graphics_window *window,
	int pane_no,enum Scene_viewer_projection_mode projection_mode)
/*******************************************************************************
LAST MODIFIED : 28 June 2000

DESCRIPTION :
Sets the <projection_mode> used by pane <pane_no> of <window>. Allowable values
are SCENE_VIEWER_PARALLEL,	SCENE_VIEWER_PERSPECTIVE and SCENE_VIEWER_CUSTOM.
Whether you can set this for a pane depends on current layout_mode of window.
Must call Graphics_window_view_changed after changing tied pane.
==============================================================================*/
{
	int return_code;

	ENTER(Graphics_window_set_projection_mode);
	if (window&&window->scene_viewer_array&&
		 (0<=pane_no)&&(pane_no<window->number_of_panes)&&
		 (window->scene_viewer_array[pane_no]))
	{
		if (Graphics_window_layout_mode_is_projection_mode_valid_for_pane(
			window->layout_mode,pane_no,projection_mode))
		{
			return_code=Scene_viewer_set_projection_mode(
				window->scene_viewer_array[pane_no]->core_scene_viewer,projection_mode);
			if (return_code)
			{
				/* update perspective button widget if current_pane changed */
				if (pane_no == window->current_pane)
				{
#if defined (WX_USER_INTERFACE)
					/* set perspective widgets */
					if (window->wx_perspective_button)
					{
						 if (SCENE_VIEWER_PERSPECTIVE == projection_mode)
						 {
								window->wx_perspective_button->SetValue(true);
						 }
						 else
						 {
								window->wx_perspective_button->SetValue(false);
						 }
						 if ((!Graphics_window_layout_mode_is_projection_mode_valid_for_pane(
										 window->layout_mode,pane_no,SCENE_VIEWER_PERSPECTIVE))||
								(SCENE_VIEWER_CUSTOM==projection_mode))
						 {
								window->wx_perspective_button->Disable();
						 }
						 else
						 {
								window->wx_perspective_button->Enable();
						 }
					}
#endif /* defined (WX_USER_INTERFACE) */
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Graphics_window_set_projection_mode.  "
				"Projection mode not valid for pane");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_window_set_projection_mode.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Graphics_window_set_projection_mode */

cmzn_scene *Graphics_window_get_Scene(struct Graphics_window *window)
/*******************************************************************************
LAST MODIFIED : 6 October 1998

DESCRIPTION :
Returns the Scene for the <graphics_window>.
???DB.  Used for setting the scene time.  Could alternatively have
	Graphics_window_set_time
==============================================================================*/
{
	cmzn_scene *scene;

	ENTER(Graphics_window_get_Scene);
	if (window)
	{
		scene=window->scene;
	}
	else
	{
		display_message(ERROR_MESSAGE,"Graphics_window_get_Scene.  Missing window");
		scene=(cmzn_scene *)NULL;
	}
	LEAVE;

	return (scene);
} /* Graphics_window_get_Scene */

struct Scene_viewer_app *Graphics_window_get_Scene_viewer(
	struct Graphics_window *window,int pane_no)
/*******************************************************************************
LAST MODIFIED : 8 October 1998

DESCRIPTION :
Returns the Scene_viewer in pane <pane_no> of <window>. Calling function can
then set view and other parameters for the scene_viewer directly.
==============================================================================*/
{
	struct Scene_viewer_app *scene_viewer;

	ENTER(Graphics_window_get_Scene_viewer);
	if (window&&window->scene_viewer_array&&
		 (0<=pane_no)&&(pane_no<window->number_of_panes) &&
		 (window->scene_viewer_array!=NULL))
	{
		scene_viewer=window->scene_viewer_array[pane_no];
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_window_get_Scene_viewer.  Invalid argument(s)");
		scene_viewer = 0;
	}
	LEAVE;

	return (scene_viewer);
} /* Graphics_window_get_Scene_viewer */

double Graphics_window_get_std_view_angle(
	struct Graphics_window *graphics_window)
/*******************************************************************************
LAST MODIFIED : 16 December 1997

DESCRIPTION :
Returns the std_view_angle from the <graphics_window>.
==============================================================================*/
{
	double return_angle;

	ENTER(Graphics_window_get_std_view_angle);
	if (graphics_window)
	{
		return_angle=graphics_window->std_view_angle;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_window_get_std_view_angle.  Missing graphics window");
		return_angle=0.0;
	}
	LEAVE;

	return (return_angle);
} /* Graphics_window_get_std_view_angle */

int Graphics_window_set_std_view_angle(struct Graphics_window *graphics_window,
	double std_view_angle)
/*******************************************************************************
LAST MODIFIED : 6 October 1998

DESCRIPTION :
Sets the <std_view_angle> for the <graphics_window>. The std_view_angle (in
degrees) is used by the Graphics_window_view_all function which positions the
viewer the correct distance away to see the currently visible scene at that
angle.
==============================================================================*/
{
	int return_code;

	ENTER(Graphics_window_set_std_view_angle);
	if (graphics_window)
	{
		if ((1.0 <= std_view_angle)&&(179.0 >= std_view_angle))
		{
			graphics_window->std_view_angle=std_view_angle;
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Graphics_window_set_std_view_angle.  std_view_angle out of range");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_window_set_std_view_angle.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Graphics_window_set_std_view_angle */

int Graphics_window_get_viewing_area_size(struct Graphics_window *window,
	int *viewing_width,int *viewing_height)
/*******************************************************************************
LAST MODIFIED : 14 October 1998

DESCRIPTION :
The viewing_width and viewing_height are the size of the viewing area when the
graphics window has only one pane. When multiple panes are used, they are
separated by 2 pixel borders within the viewing area.
==============================================================================*/
{
#if defined (WIN32_USER_INTERFACE)
	RECT rect;
#endif /* defined (WIN32_USER_INTERFACE) */
	int return_code;

	ENTER(Graphics_window_get_viewing_area_size);
	if (window&&viewing_width&&viewing_height)
	{
#if defined (GTK_USER_INTERFACE)
#if GTK_MAJOR_VERSION >= 2
		gtk_window_get_size(GTK_WINDOW(window->shell_window),
			viewing_width, viewing_height);
		return_code=1;
#else /* GTK_MAJOR_VERSION >= 2 */
		*viewing_width = window->shell_window->allocation.width;
		*viewing_height = window->shell_window->allocation.height;
		return_code=1;
#endif /* GTK_MAJOR_VERSION >= 2 */
#elif defined (WX_USER_INTERFACE)
		XRCCTRL(*window->wx_graphics_window,"GridPanel", wxPanel)
			 ->GetClientSize(viewing_width, viewing_height);
		return_code=1;
#elif defined (WIN32_USER_INTERFACE)
		if (0 != GetClientRect(window->hWnd, &rect))
		{
			*viewing_width = rect.right;
			*viewing_height = rect.bottom;
			return_code = 1;
		}
		else
		{
			return_code = 0;
		}
#else
		display_message(ERROR_MESSAGE,
			"Graphics_window_get_viewing_area_size.  Not implemented for this user interface.");
		return_code=0;
#endif /* defined (USER_INTERFACE) */
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_window_get_viewing_area_size.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Graphics_window_get_viewing_area_size */

int Graphics_window_set_viewing_area_size(struct Graphics_window *window,
	int viewing_width,int viewing_height)
/*******************************************************************************
LAST MODIFIED : 14 October 1998

DESCRIPTION :
The viewing_width and viewing_height are the size of the viewing area when the
graphics window has only one pane. When multiple panes are used, they are
separated by 2 pixel borders within the viewing area.
==============================================================================*/
{
	int return_code;

	ENTER(Graphics_window_set_viewing_area_size);
	if (window&&(0<=viewing_width)&&(0<=viewing_height))
	{
#if defined (GTK_USER_INTERFACE)
#if GTK_MAJOR_VERSION >= 2
		gtk_window_resize(GTK_WINDOW(window->shell_window),
			viewing_width, viewing_height);
#else /* GTK_MAJOR_VERSION >= 2 */
		gtk_widget_set_usize(window->shell_window, viewing_width, viewing_height);
#endif /* GTK_MAJOR_VERSION >= 2 */
#elif defined (WX_USER_INTERFACE)
		int temp_height, temp_width;
		wxPanel *graphics_grid_panel = XRCCTRL(
			 *window->wx_graphics_window,"GridPanel", wxPanel);
		graphics_grid_panel->GetSize(&temp_width, &temp_height);
		if ((temp_width != viewing_width) || (temp_height != viewing_height))
		{
			 window->left_panel->GetSize(&temp_width, &temp_height);
			 /* the following part is to fix problem found before the
					graphics window is initialised. It is all hardcode, should
					modify it*/
			temp_height = 0;
			if (window->time_editor_panel->IsShown())
			{
				 int temp;
				 window->time_editor_panel->GetSize(&temp, &temp_height);
			}
			 /*end*/
			graphics_grid_panel->GetContainingSizer()->SetMinSize(viewing_width+temp_width-10, viewing_height+temp_height);
			graphics_grid_panel->GetContainingSizer()->SetDimension(-1,
				 -1, viewing_width+temp_width-10, viewing_height+temp_height);
			 window->wx_graphics_window->GetSizer()->SetSizeHints(window->wx_graphics_window);
			 window->GraphicsWindowTitle->Fit();
			 graphics_grid_panel->SetMinSize(wxSize(10,10));
			 window->GraphicsWindowTitle->SetSize(window->GraphicsWindowTitle->GetSize()+wxSize(0,1));
			 window->GraphicsWindowTitle->SetSize(window->GraphicsWindowTitle->GetSize()-wxSize(0,1));
			 window->GraphicsWindowTitle->SetMinSize(wxSize(20,20));
			 window->GraphicsWindowTitle->Layout();
		}
#elif defined (WIN32_USER_INTERFACE)
		Graphics_window_adjust_sizes_for_window_frame(&viewing_width, &viewing_height);

		RECT current_position;
		GetWindowRect(window->hWnd, &current_position);

		MoveWindow(window->hWnd,
			current_position.left, current_position.top,
			viewing_width, viewing_height, true);

#endif /* switch (USER_INTERFACE) */

		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_window_set_viewing_area_size.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Graphics_window_set_viewing_area_size */

int Graphics_window_update(struct Graphics_window *graphics_window)
/*******************************************************************************
LAST MODIFIED : 6 October 1998

DESCRIPTION :
Forces a redraw on <graphics_window> at the next idle moment.
In future want a flag in the graphics window which if on delays the redraw
until an explicit "gfx update" is entered.
==============================================================================*/
{
	int return_code,pane_no;

	ENTER(Graphics_window_update);
	if (graphics_window && graphics_window->scene_viewer_array)
	{
		for (pane_no=0;pane_no<graphics_window->number_of_panes;pane_no++)
		{
			Scene_viewer_app_redraw(graphics_window->scene_viewer_array[pane_no]);
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_window_update.  Missing graphics window");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Graphics_window_update */

int Graphics_window_update_now(struct Graphics_window *graphics_window)
/*******************************************************************************
LAST MODIFIED : 6 October 1998

DESCRIPTION :
Forces a redraw on <graphics_window>.
In future want a flag in the graphics window which if on delays the redraw
until an explicit "gfx update" is entered.
==============================================================================*/
{
	int return_code,pane_no;

	ENTER(Graphics_window_update_now);
	if (graphics_window && graphics_window->scene_viewer_array)
	{
#if defined (NEW_CODE)
		/* Handle all the X events so that the window resizing gets done */
		while (XtAppPending(graphics_window->user_interface->application_context))
		{
			application_main_step(graphics_window->user_interface);
		}
#endif /* defined (NEW_CODE) */
		for (pane_no=0;pane_no<graphics_window->number_of_panes;pane_no++)
		{
			Scene_viewer_app_redraw_now(graphics_window->scene_viewer_array[pane_no]);
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_window_update_now.  Missing graphics window");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Graphics_window_update_now */

int Graphics_window_update_now_iterator(struct Graphics_window *graphics_window,
	void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 6 October 1998

DESCRIPTION :
Iterator function for forcing a redraw on <graphics_window>.
In future want a flag in the graphics window which if on delays the redraw
until an explicit "gfx update" is entered.
==============================================================================*/
{
	int return_code,pane_no;

	ENTER(Graphics_window_update_now_iterator);
	USE_PARAMETER(dummy_void);
	if (graphics_window && graphics_window->scene_viewer_array)
	{
		for (pane_no=0;pane_no<graphics_window->number_of_panes;pane_no++)
		{
			Scene_viewer_app_redraw_now(graphics_window->scene_viewer_array[pane_no]);
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_window_update_now_iterator.  Missing graphics window");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Graphics_window_update_now_iterator */

int Graphics_window_update_now_without_swapbuffers(
	struct Graphics_window *graphics_window)
/*******************************************************************************
LAST MODIFIED : 6 October 1998

DESCRIPTION :
Forces a redraw on <graphics_window>.  Allows the window to be updated but kept
in the backbuffer so that utility functions such as the movie extensions can get
the pixels out of the backbuffer before the frames are swapped.
==============================================================================*/
{
	int return_code,pane_no;

	ENTER(Graphics_window_update_now_without_swapbuffers);
	if (graphics_window && graphics_window->scene)
	{
		for (pane_no=0;pane_no<graphics_window->number_of_panes;pane_no++)
		{
			Scene_viewer_app_redraw_now_without_swapbuffers
				(graphics_window->scene_viewer_array[pane_no]);
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_window_update_now_without_swapbuffers.  "
			"Missing graphics window");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Graphics_window_update_now_without_swapbuffers */

int Graphics_window_get_frame_pixels(struct Graphics_window *window,
	enum Texture_storage_type storage, int *width, int *height,
	int preferred_antialias, int preferred_transparency_layers,
	unsigned char **frame_data, int force_onscreen)
/*******************************************************************************
LAST MODIFIED : 18 September 2002

DESCRIPTION :
Returns the contents of the graphics window as pixels.  <width> and <height>
will be respected if the window is drawn offscreen and they are non zero,
otherwise they are set in accordance with current size of the graphics window.
If <preferred_antialias> or <preferred_transparency_layers> are non zero then they
attempt to override the default values for just this call.
If <force_onscreen> is non zero then the pixels will always be grabbed from the
graphics window on screen.
==============================================================================*/
{
	int frame_width, frame_height, number_of_components, return_code, antialias;
	double bottom = 0.0, fraction_across, fraction_down, left,
		NDC_left = 0.0, NDC_top = 0.0, NDC_width = 0.0, NDC_height = 0.0,
		original_NDC_left, original_NDC_top, original_NDC_width, original_NDC_height,
		original_left, original_right, original_bottom, original_top,
		original_near_plane, original_far_plane, right, top = 0.0,
		viewport_left, viewport_top = 0.0, viewport_pixels_per_x = 0.0, viewport_pixels_per_y = 0.0,
		original_viewport_left, original_viewport_top,
		original_viewport_pixels_per_x, original_viewport_pixels_per_y,
		real_left, real_right, real_bottom, real_top,
		scaled_NDC_width,scaled_NDC_height ;
	int i, j, number_of_panes = 0, pane = 0,
		pane_i, pane_j, pane_width = 0, pane_height = 0, panes_across = 0, panes_down = 0,
		patch_width, patch_height,
		tile_height, tile_width, tiles_across, tiles_down, panel_width, panel_height;
#if defined (OPENGL_API) && defined (USE_MSAA) && defined (WX_USER_INTERFACE)
	int multisample_framebuffer_flag = 0;
#endif
	struct Graphics_buffer_app *offscreen_buffer;
	struct Scene_viewer_app *scene_viewer;
#if defined (SGI)
/* The Octane can only handle 1024 */
#define PBUFFER_MAX (1024)
#else
#define PBUFFER_MAX (2048)
#endif /* defined (SGI) */

	ENTER(Graphics_window_get_frame_pixels);
	if (window && width && height)
	{
		// force complete build of all graphics in scene for image output, otherwise may get only incremental output
		cmzn_scenefilter_id filter = cmzn_sceneviewer_get_scenefilter((window->scene_viewer_array[0]->core_scene_viewer));
		build_Scene(window->scene, filter);
		cmzn_scenefilter_destroy(&filter);

		double frame_split_ration = 1.0;
		Graphics_window_get_viewing_area_size(window, &panel_width,
			&panel_height);
		antialias = preferred_antialias;
		if (antialias == -1)
		{
			antialias = window->antialias_mode;
		}
		if ((*width) && (*height))
		{
			frame_width = *width;
			frame_height = *height;
		}
		else
		{
			/* Only use the window size if either dimension is zero */
			frame_width = panel_width;
			frame_height = panel_height;
			*width = frame_width;
			*height = frame_height;
		}
		/* If working offscreen try and allocate as large an area as possible */
		if (!force_onscreen)
		{
			offscreen_buffer = (struct Graphics_buffer_app *)NULL;
#define PANE_BORDER (2)
			switch (window->layout_mode)
			{
				case GRAPHICS_WINDOW_LAYOUT_SIMPLE:
				case GRAPHICS_WINDOW_LAYOUT_2D:
				{
					number_of_panes = 1;
					panes_across = 1;
					panes_down = 1;
					pane_width = frame_width;
					pane_height = frame_height;
				} break;
				case GRAPHICS_WINDOW_LAYOUT_ORTHOGRAPHIC:
				case GRAPHICS_WINDOW_LAYOUT_FREE_ORTHO:
				{
					number_of_panes = 4;
					panes_across = 2;
					panes_down = 2;
					/* Reduce the pane_width by one pixel to leave a border */
					pane_width = (frame_width - PANE_BORDER) / 2;
					pane_height = (frame_height - PANE_BORDER) / 2;
				} break;
				case GRAPHICS_WINDOW_LAYOUT_FRONT_BACK:
				case GRAPHICS_WINDOW_LAYOUT_FRONT_SIDE:
				case GRAPHICS_WINDOW_LAYOUT_PSEUDO_3D:
				case GRAPHICS_WINDOW_LAYOUT_TWO_FREE:
				{
					number_of_panes = 2;
					panes_across = 2;
					panes_down = 1;
					/* Reduce the pane_width by one pixel to leave a border */
					pane_width = (frame_width - PANE_BORDER) / 2;
					pane_height = frame_height;
					frame_split_ration = 2.0;
				} break;
				default:
				{
					display_message(ERROR_MESSAGE,
						"Graphics_window_get_frame_pixels.  Unknown layout_mode");
					return_code=0;
				} break;
			}

			if (pane_width <= panel_width)
			{
				tile_width = pane_width/panes_across;
				fraction_across = 1.0;
				tiles_across = 1;
			}
			else
			{
				tile_width = panel_width/panes_across;
				fraction_across = (double)pane_width / (double)tile_width;
				tiles_across = (int)ceil(fraction_across);
			}
			if (pane_height <= panel_height)
			{
				tile_height = pane_height/panes_down;
				fraction_down = 1.0;
				tiles_down = 1;
			}
			else
			{
				tile_height = panel_height/panes_down;
				fraction_down = (double)pane_height / (double)tile_height;
				tiles_down = (int)ceil(fraction_down);
			}

			if (!(offscreen_buffer = create_Graphics_buffer_offscreen_from_buffer(
				  tile_width, tile_height, /*buffer_to_match*/Scene_viewer_app_get_graphics_buffer(
				  Graphics_window_get_Scene_viewer(window, 0)))))
			{
				force_onscreen = 1;
			}
		}
		if (!force_onscreen)
		{
			if (GRAPHICS_BUFFER_GL_EXT_FRAMEBUFFER_TYPE ==
				Graphics_buffer_get_type(Graphics_buffer_app_get_core_buffer(offscreen_buffer)))
			{
				for (pane = 0 ; pane < number_of_panes ; pane++)
				{
					Scene_viewer_app_redraw_now(
						Graphics_window_get_Scene_viewer(window,pane));
				}
			}
			number_of_components =
				Texture_storage_type_get_number_of_components(storage);
			if (ALLOCATE(*frame_data, unsigned char,
				number_of_components * (frame_width) * (frame_height)))
			{
				return_code = 1;
				for (pane = 0 ; pane < number_of_panes ; pane++)
				{
					struct Graphics_buffer_app *current_buffer;
					if (pane == 0)
					{
						current_buffer = offscreen_buffer;
					}
					else
					{
						current_buffer = create_Graphics_buffer_offscreen_from_buffer(
							tile_width, tile_height, /*buffer_to_match*/Scene_viewer_app_get_graphics_buffer(
							 Graphics_window_get_Scene_viewer(window, pane)));
					}
					if (current_buffer)
					{
						Graphics_buffer_app_make_current(current_buffer);
						if (Graphics_buffer_get_type(Graphics_buffer_app_get_core_buffer(current_buffer)) ==
							GRAPHICS_BUFFER_GL_EXT_FRAMEBUFFER_TYPE)
						{
							if (antialias > 1)
							{
		#if !defined (USE_MSAA)
								display_message(WARNING_MESSAGE,
									"Graphics_window_get_frame_pixels. Cmgui-wx does not write"
									"image with anti-aliasing under offscreen mode at the moment.");
		#else
		#if defined (OPENGL_API) && defined (USE_MSAA)
								multisample_framebuffer_flag =
									Graphics_buffer_set_multisample_framebuffer(Graphics_buffer_app_get_core_buffer(current_buffer), antialias);
		#endif
		#endif
							}
						}

		#if defined (OPENGL_API)
						if (number_of_panes > 1)
						{
							/* Clear the buffer as we are going to leave a border between panes */
							glClearColor(0.0,0.0,0.0,0.);
							glClear(GL_COLOR_BUFFER_BIT);
						}
		#endif /* defined (OPENGL_API) */
						if ((tiles_across > 1) || (panes_across > 1))
						{
							glPixelStorei(GL_PACK_ROW_LENGTH, frame_width);
						}
						pane_i = pane % panes_across;
						pane_j = pane / panes_across;
						scene_viewer = Graphics_window_get_Scene_viewer(window,pane);
						if ((tiles_across > 1) || (tiles_down > 1))
						{
							Scene_viewer_get_viewing_volume(scene_viewer->core_scene_viewer,
								&original_left, &original_right, &original_bottom, &original_top,
								&original_near_plane, &original_far_plane);
							Scene_viewer_get_NDC_info(scene_viewer->core_scene_viewer,
								&original_NDC_left, &original_NDC_top, &original_NDC_width, &original_NDC_height);
							Scene_viewer_get_viewport_info(scene_viewer->core_scene_viewer,
								&original_viewport_left, &original_viewport_top,
								&original_viewport_pixels_per_x, &original_viewport_pixels_per_y);
							Scene_viewer_get_viewing_volume_and_NDC_info_for_specified_size(scene_viewer->core_scene_viewer,
									frame_width/frame_split_ration, frame_height, panel_width, panel_height, &real_left,
								&real_right, &real_bottom, &real_top, &scaled_NDC_width, &scaled_NDC_height);
							NDC_width = scaled_NDC_width / fraction_across;
							NDC_height = scaled_NDC_height / fraction_down ;
							viewport_pixels_per_x = original_viewport_pixels_per_x;
							viewport_pixels_per_y = original_viewport_pixels_per_y;
						}
						for (j = 0 ; return_code && (j < tiles_down) ; j++)
						{
							if ((tiles_across > 1) || (tiles_down > 1))
							{
								bottom = real_bottom + (double)j * (real_top - real_bottom) / fraction_down;
								top = real_bottom
									+ (double)(j + 1) * (real_top - real_bottom) / fraction_down;
								NDC_top = original_NDC_top + (double)j * original_NDC_height / fraction_down;
								viewport_top = ((j + 1) * tile_height - pane_height) / viewport_pixels_per_y;
							}
							for (i = 0 ; return_code && (i < tiles_across) ; i++)
							{
								if ((tiles_across > 1) || (tiles_down > 1))
								{
									left = real_left + (double)i * (real_right - real_left) / fraction_across;
									right = real_left +
										(double)(i + 1) * (real_right - real_left) / fraction_across;
									NDC_left = original_NDC_left + (double)i *
										 original_NDC_width / fraction_across;
									viewport_left = i * tile_width / viewport_pixels_per_x;
									Scene_viewer_set_viewing_volume(scene_viewer->core_scene_viewer,
										left, right, bottom, top,
										original_near_plane, original_far_plane);
									Scene_viewer_set_NDC_info(scene_viewer->core_scene_viewer,
											NDC_left, NDC_top, NDC_width, NDC_height);
									Scene_viewer_set_viewport_info(scene_viewer->core_scene_viewer,
										viewport_left, viewport_top,
										viewport_pixels_per_x, viewport_pixels_per_y);
								}
								if (Graphics_buffer_get_type(Graphics_buffer_app_get_core_buffer(current_buffer)) ==
									GRAPHICS_BUFFER_GL_EXT_FRAMEBUFFER_TYPE )
								{
	#if !defined (USE_MSAA)
									Scene_viewer_render_scene_in_viewport_with_overrides(scene_viewer->core_scene_viewer,
										/*left*/0, /*bottom*/0, /*right*/tile_width, /*top*/tile_height,
										/*preferred_antialias*/0, preferred_transparency_layers,
										/*drawing_offscreen*/1);
	#else
									Scene_viewer_render_scene_in_viewport_with_overrides(scene_viewer->core_scene_viewer,
										/*left*/0, /*bottom*/0, /*right*/tile_width, /*top*/tile_height,
										antialias, preferred_transparency_layers,
										/*drawing_offscreen*/1);
	#endif
								}
								else
								{
									Scene_viewer_render_scene_in_viewport_with_overrides(scene_viewer->core_scene_viewer,
										/*left*/0, /*bottom*/0, /*right*/tile_width, /*top*/tile_height,
										antialias, preferred_transparency_layers,
										/*drawing_offscreen*/1);
								}
								if (return_code)
								{
									if (i < tiles_across - 1)
									{
										patch_width = tile_width;
									}
									else
									{
										patch_width = pane_width - tile_width * (tiles_across - 1);
									}
									if (j < tiles_down - 1)
									{
										patch_height = tile_height;
									}
									else
									{
										patch_height = pane_height - tile_height * (tiles_down - 1);
									}

									if (Graphics_buffer_get_type(Graphics_buffer_app_get_core_buffer(current_buffer)) ==
										GRAPHICS_BUFFER_GL_EXT_FRAMEBUFFER_TYPE)
									{
	#if defined (OPENGL_API) && defined (USE_MSAA) && defined (WX_USER_INTERFACE)
										if (multisample_framebuffer_flag)
										{
											Graphics_buffer_blit_framebuffer(Graphics_buffer_app_get_core_buffer(current_buffer));
										}
	#endif
									}
									return_code=Graphics_library_read_pixels(*frame_data +
										(i * tile_width + pane_i * (pane_width + PANE_BORDER) +
											(j * tile_height + (panes_down - 1 - pane_j) * (pane_height + PANE_BORDER))
											* frame_width) * number_of_components,
										patch_width, patch_height, storage, /*front_buffer*/0);
									if (Graphics_buffer_get_type(Graphics_buffer_app_get_core_buffer(current_buffer)) ==
										GRAPHICS_BUFFER_GL_EXT_FRAMEBUFFER_TYPE)
									{
	#if defined (OPENGL_API) && defined (USE_MSAA) && defined (WX_USER_INTERFACE)
										if (multisample_framebuffer_flag)
										{
											Graphics_buffer_reset_multisample_framebuffer(Graphics_buffer_app_get_core_buffer(current_buffer));
										}
	#endif
									}
								}
							}
						}
						if (tiles_across > 1)
						{
							glPixelStorei(GL_PACK_ROW_LENGTH, 0);
						}
						if ((tiles_across > 1) || (tiles_down > 1))
						{
							Scene_viewer_set_viewing_volume(scene_viewer->core_scene_viewer,
								original_left, original_right, original_bottom, original_top,
								original_near_plane, original_far_plane);
							Scene_viewer_set_NDC_info(scene_viewer->core_scene_viewer,
								original_NDC_left, original_NDC_top, original_NDC_width, original_NDC_height);
							Scene_viewer_set_viewport_info(scene_viewer->core_scene_viewer,
								original_viewport_left, original_viewport_top,
								original_viewport_pixels_per_x, original_viewport_pixels_per_y);
						}
						DESTROY(Graphics_buffer_app)(&current_buffer);
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Graphics_window_get_frame_pixels.  Unable to allocate pixels");
				return_code=0;
			}
		}
		else
		{
			/* Always use the window size if grabbing from screen */
			Graphics_window_get_viewing_area_size(window, &frame_width,
				&frame_height);
			if ((frame_width != *width) || (frame_height != *height))
			{
				display_message(WARNING_MESSAGE,
					"Graphics_window_get_frame_pixels.  "
					"When grabbing from the screen the width and height are forced to match the window size %d,%d", frame_width, frame_height);
				*width = frame_width;
				*height = frame_height;
			}
			Scene_viewer_app_redraw_now_with_overrides(
				Graphics_window_get_Scene_viewer(window,/*pane_no*/0),
				antialias, preferred_transparency_layers);
			number_of_components =
				Texture_storage_type_get_number_of_components(storage);
			if (ALLOCATE(*frame_data, unsigned char,
				number_of_components * (frame_width) * (frame_height)))
			{
				switch (window->layout_mode)
				{
					case GRAPHICS_WINDOW_LAYOUT_SIMPLE:
					case GRAPHICS_WINDOW_LAYOUT_2D:
					{
						/* Only one pane */
						if (!(return_code=Graphics_library_read_pixels(*frame_data, frame_width,
									frame_height, storage, /*front_buffer*/0)))
						{
							DEALLOCATE(*frame_data);
						}
					} break;
					case GRAPHICS_WINDOW_LAYOUT_ORTHOGRAPHIC:
					case GRAPHICS_WINDOW_LAYOUT_FREE_ORTHO:
					{
						/* Four panes */
						display_message(ERROR_MESSAGE,"Graphics_window_get_frame_pixels.  "
							"Can only grab single pane windows onscreen currently");
						return_code=1;
					} break;
					case GRAPHICS_WINDOW_LAYOUT_FRONT_BACK:
					case GRAPHICS_WINDOW_LAYOUT_FRONT_SIDE:
					case GRAPHICS_WINDOW_LAYOUT_PSEUDO_3D:
					case GRAPHICS_WINDOW_LAYOUT_TWO_FREE:
					{
						/* Two panes, side by side */
						display_message(ERROR_MESSAGE,"Graphics_window_get_frame_pixels.  "
							"Can only grab single pane windows onscreen currently");
						return_code=1;
					} break;
					default:
					{
						display_message(ERROR_MESSAGE,
							"Graphics_window_get_frame_pixels.  Unknown layout_mode");
						DEALLOCATE(*frame_data);
						return_code=0;
					} break;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Graphics_window_get_frame_pixels.  Unable to allocate pixels");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_window_get_frame_pixels.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return return_code;
} /* Graphics_window_get_frame_pixels */

struct Cmgui_image *Graphics_window_get_image(struct Graphics_window *window,
	int force_onscreen, int preferred_width, int preferred_height,
	int preferred_antialias, int preferred_transparency_layers,
	enum Texture_storage_type storage)
/*******************************************************************************
LAST MODIFIED : 18 September 2002

DESCRIPTION :
Creates and returns a Cmgui_image from the image in <window>, usually for
writing. The image has a single depth plane and is in RGBA format.
Up to the calling function to DESTROY the returned Cmgui_image.
If <force_onscreen> is set then the pixels are grabbed directly from the window
display and the <preferred_width> and <preferred_height> are ignored.
Currently limited to 1 byte per component -- may want to improve for HPC.
==============================================================================*/
{
	unsigned char *frame_data;
	int bytes_per_pixel, height, number_of_bytes_per_component,
		number_of_components, width;
	struct Cmgui_image *cmgui_image;

	ENTER(Graphics_window_get_image);
	cmgui_image = (struct Cmgui_image *)NULL;
	if (window)
	{
		number_of_components =
			Texture_storage_type_get_number_of_components(storage);
		number_of_bytes_per_component = 1;
		bytes_per_pixel = number_of_components*number_of_bytes_per_component;
		width = preferred_width;
		height = preferred_height;
		if (Graphics_window_get_frame_pixels(window, storage,
			&width, &height, preferred_antialias, preferred_transparency_layers,
			&frame_data, force_onscreen))
		{
			cmgui_image = Cmgui_image_constitute(width, height,
				number_of_components, number_of_bytes_per_component,
				width*bytes_per_pixel, frame_data);
			if (!cmgui_image)
			{
				display_message(ERROR_MESSAGE,
					"Graphics_window_get_image.  Could not constitute image");
			}
			DEALLOCATE(frame_data);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Graphics_window_get_image.  Could not get frame pixels");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_window_get_image.  Missing window");
	}
	LEAVE;

	return (cmgui_image);
} /* Graphics_window_get_image */

int Graphics_window_view_all(struct Graphics_window *window)
/*******************************************************************************
LAST MODIFIED : 16 October 2001

DESCRIPTION :
Finds the x, y and z ranges from the scene and sets the view parameters so
that everything can be seen, and with window's std_view_angle. Also adjusts
near and far clipping planes; if specific values are required, should follow
with commands for setting these.
==============================================================================*/
{
	int return_code;

	if (window && window->scene_viewer_array)
	{
		return_code = 1;
		cmzn_scenefilter_id filter = cmzn_sceneviewer_get_scenefilter((window->scene_viewer_array[0]->core_scene_viewer));
		double min[3] = { 0.0, 0.0, 0.0 };
		double max[3] = { 0.0, 0.0, 0.0 };
		cmzn_scene_get_coordinates_range(window->scene, filter, min, max);
		const double centre_x = 0.5*(min[0] + max[0]);
		const double centre_y = 0.5*(min[1] + max[1]);
		const double centre_z = 0.5*(min[2] + max[2]);
		const double size_x = max[0] - min[0];
		const double size_y = max[1] - min[1];
		const double size_z = max[2] - min[2];
		cmzn_scenefilter_destroy(&filter);
		double radius = 0.5*sqrt(size_x*size_x + size_y*size_y + size_z*size_z);
		if (0 == radius)
		{
			/* get current "radius" from first scene viewer */
			double left, right, bottom, top, near_plane, far_plane;
			Scene_viewer_get_viewing_volume(window->scene_viewer_array[0]->core_scene_viewer,
				&left, &right, &bottom, &top, &near_plane, &far_plane);
			radius = 0.5*(right - left);
		}
		else
		{
			/*???RC width_factor should be read in from defaults file */
			const int width_factor = 1.05;
			/* enlarge radius to keep image within edge of window */
			radius *= width_factor;
		}

		/*???RC clip_factor should be read in from defaults file: */
		const int clip_factor = 4.0;
		for (int pane_no = 0; (pane_no < window->number_of_scene_viewers) &&
			return_code; pane_no++)
		{
			return_code = Scene_viewer_set_view_simple(
				window->scene_viewer_array[pane_no]->core_scene_viewer, centre_x, centre_y, centre_z,
				radius, window->std_view_angle, clip_factor*radius);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_window_view_all.  Invalid argument(s)");
		return_code=0;
	}
	return (return_code);
}

int Graphics_window_view_changed(struct Graphics_window *window,
	int changed_pane)
/*******************************************************************************
LAST MODIFIED : 29 September 2000

DESCRIPTION :
Call this function whenever the view in a pane has changed. Depending on the
current layout_mode, the function adjusts the view in all the panes tied to
<changed_pane> to maintain the relationship expected for it.
==============================================================================*/
{
	/* This is only used by this routine now, everything else uses the
		number_of_scene_viewers stored in the graphics window */
#define GRAPHICS_WINDOW_MAX_NUMBER_OF_PANES (4)
	double angle,bottom,eye[3],eye_distance,extra_axis[3],
		far_plane[GRAPHICS_WINDOW_MAX_NUMBER_OF_PANES],left,lookat[3],
		near_plane[GRAPHICS_WINDOW_MAX_NUMBER_OF_PANES],right,top,up[3],view[3],
		view_left[3];
	enum Scene_viewer_projection_mode projection_mode;
	int pane_no,return_code;

	ENTER(Graphics_window_view_changed);
	if (window&&window->scene_viewer_array&&(0<=changed_pane)&&(changed_pane<window->number_of_panes)&&
		(window->number_of_scene_viewers <= GRAPHICS_WINDOW_MAX_NUMBER_OF_PANES))
	{
		/* 1. get lookat parameters and viewing volume of changed_pane. Also need
			 separate near and far clipping planes for each pane_no */
		for (pane_no=0;pane_no<window->number_of_panes;pane_no++)
		{
			if (pane_no != changed_pane)
			{
				Scene_viewer_stop_animations(window->scene_viewer_array[pane_no]->core_scene_viewer);
			}
			Scene_viewer_get_viewing_volume(window->scene_viewer_array[pane_no]->core_scene_viewer,
				&left,&right,&bottom,&top,&(near_plane[pane_no]),&(far_plane[pane_no]));
		}
		return_code=(cmzn_sceneviewer_get_lookat_parameters(
			window->scene_viewer_array[changed_pane]->core_scene_viewer, eye, lookat, up) &&
			Scene_viewer_get_viewing_volume(window->scene_viewer_array[changed_pane]->core_scene_viewer,
				&left,&right,&bottom,&top,&(near_plane[changed_pane]),&(far_plane[changed_pane])));
		if (return_code)
		{
			projection_mode=Graphics_window_get_projection_mode(window,changed_pane);
			/* get orthogonal view, up and left directions in changed_pane */
			view[0]=eye[0]-lookat[0];
			view[1]=eye[1]-lookat[1];
			view[2]=eye[2]-lookat[2];
			eye_distance=normalize3(view);
			cross_product3(view,up,view_left);
			normalize3(view_left);
			cross_product3(view_left,view,up);
			switch (window->layout_mode)
			{
				case GRAPHICS_WINDOW_LAYOUT_SIMPLE:
				case GRAPHICS_WINDOW_LAYOUT_2D:
				case GRAPHICS_WINDOW_LAYOUT_TWO_FREE:
				{
					/* nothing to do: only one pane or not connected */
				} break;
				case GRAPHICS_WINDOW_LAYOUT_ORTHOGRAPHIC:
				{
					if (window->number_of_scene_viewers >= 4)
					{
						/* panes 2, 3 and 4 tied in third angle orthographic projection. Pane
							0 shows the iso_metric view in the octant of the other views */
						/* first give each tied scene viewer the same lookat parameters and
							viewing volume (except for near and far) */
						for (pane_no=0;pane_no<4;pane_no++)
						{
							Scene_viewer_set_lookat_parameters(window->scene_viewer_array[pane_no]->core_scene_viewer,
								eye[0],eye[1],eye[2],lookat[0],lookat[1],lookat[2],
								up[0],up[1],up[2]);
							Scene_viewer_set_viewing_volume(window->scene_viewer_array[pane_no]->core_scene_viewer,
								left,right,bottom,top,near_plane[pane_no],far_plane[pane_no]);
						}
						/* now rotate all tied panes but the changed_pane to get the proper
							relationship between them */
						switch (changed_pane)
						{
							case 0:
							{
								/* extra_axis = front,up */
								extra_axis[0]=view[0]+up[0];
								extra_axis[1]=view[1]+up[1];
								extra_axis[2]=view[2]+up[2];
								normalize3(extra_axis);
								Scene_viewer_rotate_about_lookat_point(window->scene_viewer_array[1]->core_scene_viewer,
									view_left,0.25*PI);
								Scene_viewer_rotate_about_lookat_point(window->scene_viewer_array[1]->core_scene_viewer,
									extra_axis,0.25*PI);
								Scene_viewer_rotate_about_lookat_point(window->scene_viewer_array[2]->core_scene_viewer,
									view_left,-0.25*PI);
								Scene_viewer_rotate_about_lookat_point(window->scene_viewer_array[2]->core_scene_viewer,
									extra_axis,-0.25*PI);
								Scene_viewer_rotate_about_lookat_point(window->scene_viewer_array[3]->core_scene_viewer,
									view_left,-0.25*PI);
								Scene_viewer_rotate_about_lookat_point(window->scene_viewer_array[3]->core_scene_viewer,
									extra_axis,0.25*PI);
								Scene_viewer_app_redraw_now(window->scene_viewer_array[1]);
								Scene_viewer_app_redraw_now(window->scene_viewer_array[2]);
								Scene_viewer_app_redraw_now(window->scene_viewer_array[3]);
							} break;
							case 1:
							{
								Scene_viewer_rotate_about_lookat_point(window->scene_viewer_array[0]->core_scene_viewer,
									view_left,-0.25*PI);
								Scene_viewer_rotate_about_lookat_point(window->scene_viewer_array[0]->core_scene_viewer,
									view,-0.25*PI);
								Scene_viewer_rotate_about_lookat_point(window->scene_viewer_array[3]->core_scene_viewer,
									view_left,-0.5*PI);
								Scene_viewer_rotate_about_lookat_point(window->scene_viewer_array[2]->core_scene_viewer,
									view_left,-0.5*PI);
								Scene_viewer_rotate_about_lookat_point(window->scene_viewer_array[2]->core_scene_viewer,
									view,-0.5*PI);
								Scene_viewer_app_redraw_now(window->scene_viewer_array[0]);
								Scene_viewer_app_redraw_now(window->scene_viewer_array[2]);
								Scene_viewer_app_redraw_now(window->scene_viewer_array[3]);
							} break;
							case 2:
							{
								/* extra_axis = left, front */
								extra_axis[0]=view_left[0]+view[0];
								extra_axis[1]=view_left[1]+view[1];
								extra_axis[2]=view_left[2]+view[2];
								normalize3(extra_axis);
								Scene_viewer_rotate_about_lookat_point(window->scene_viewer_array[0]->core_scene_viewer,
									up,0.25*PI);
								Scene_viewer_rotate_about_lookat_point(window->scene_viewer_array[0]->core_scene_viewer,
									extra_axis,0.25*PI);
								Scene_viewer_rotate_about_lookat_point(window->scene_viewer_array[3]->core_scene_viewer,
									up,0.5*PI);
								Scene_viewer_rotate_about_lookat_point(window->scene_viewer_array[1]->core_scene_viewer,
									up,0.5*PI);
								Scene_viewer_rotate_about_lookat_point(window->scene_viewer_array[1]->core_scene_viewer,
									view,0.5*PI);
								Scene_viewer_app_redraw_now(window->scene_viewer_array[0]);
								Scene_viewer_app_redraw_now(window->scene_viewer_array[1]);
								Scene_viewer_app_redraw_now(window->scene_viewer_array[3]);
							} break;
							case 3:
							{
								/* extra_axis = left, rear */
								extra_axis[0]=view_left[0]-view[0];
								extra_axis[1]=view_left[1]-view[1];
								extra_axis[2]=view_left[2]-view[2];
								normalize3(extra_axis);
								Scene_viewer_rotate_about_lookat_point(window->scene_viewer_array[0]->core_scene_viewer,
									up,-0.25*PI);
								Scene_viewer_rotate_about_lookat_point(window->scene_viewer_array[0]->core_scene_viewer,
									extra_axis,0.25*PI);
								Scene_viewer_rotate_about_lookat_point(window->scene_viewer_array[1]->core_scene_viewer,
									view_left,0.5*PI);
								Scene_viewer_rotate_about_lookat_point(window->scene_viewer_array[2]->core_scene_viewer,
									up,-0.5*PI);
								Scene_viewer_app_redraw_now(window->scene_viewer_array[0]);
								Scene_viewer_app_redraw_now(window->scene_viewer_array[1]);
								Scene_viewer_app_redraw_now(window->scene_viewer_array[2]);
							} break;
						}
					}
				} break;
				case GRAPHICS_WINDOW_LAYOUT_FREE_ORTHO:
				{
					if (window->number_of_scene_viewers >= 4)
					{
						/* pane 0 is not tied at present */
						if (0 != changed_pane)
						{
							/* panes 2, 3 and 4 tied in third angle orthographic projection */
							/* first give each tied scene viewer the same lookat parameters and
								viewing volume (except for near and far) */
							for (pane_no=1;pane_no<4;pane_no++)
							{
								Scene_viewer_set_lookat_parameters(window->scene_viewer_array[pane_no]->core_scene_viewer,
									eye[0],eye[1],eye[2],lookat[0],lookat[1],lookat[2],
									up[0],up[1],up[2]);
								Scene_viewer_set_viewing_volume(window->scene_viewer_array[pane_no]->core_scene_viewer,
									left,right,bottom,top,near_plane[pane_no],far_plane[pane_no]);
							}
							/* now rotate all tied panes but the changed_pane to get the proper
								relationship between them */
							switch (changed_pane)
							{
								case 1:
								{
									Scene_viewer_rotate_about_lookat_point(window->scene_viewer_array[3]->core_scene_viewer,
										view_left,-0.5*PI);
									Scene_viewer_rotate_about_lookat_point(window->scene_viewer_array[2]->core_scene_viewer,
										view_left,-0.5*PI);
									Scene_viewer_rotate_about_lookat_point(window->scene_viewer_array[2]->core_scene_viewer,
										view,-0.5*PI);
									Scene_viewer_app_redraw_now(window->scene_viewer_array[2]);
									Scene_viewer_app_redraw_now(window->scene_viewer_array[3]);
								} break;
								case 2:
								{
									Scene_viewer_rotate_about_lookat_point(window->scene_viewer_array[3]->core_scene_viewer,
										up,0.5*PI);
									Scene_viewer_rotate_about_lookat_point(window->scene_viewer_array[1]->core_scene_viewer,
										up,0.5*PI);
									Scene_viewer_rotate_about_lookat_point(window->scene_viewer_array[1]->core_scene_viewer,
										view,0.5*PI);
									Scene_viewer_app_redraw_now(window->scene_viewer_array[1]);
									Scene_viewer_app_redraw_now(window->scene_viewer_array[3]);
								} break;
								case 3:
								{
									Scene_viewer_rotate_about_lookat_point(window->scene_viewer_array[1]->core_scene_viewer,
										view_left,0.5*PI);
									Scene_viewer_rotate_about_lookat_point(window->scene_viewer_array[2]->core_scene_viewer,
										up,-0.5*PI);
									Scene_viewer_app_redraw_now(window->scene_viewer_array[1]);
									Scene_viewer_app_redraw_now(window->scene_viewer_array[2]);
								} break;
							}
						}
					}
				} break;
				case GRAPHICS_WINDOW_LAYOUT_FRONT_BACK:
				case GRAPHICS_WINDOW_LAYOUT_FRONT_SIDE:
				{
					if (window->number_of_scene_viewers >= 2)
					{
						if (GRAPHICS_WINDOW_LAYOUT_FRONT_BACK==window->layout_mode)
						{
							/* panes 0 and 1 tied as front and back */
							angle=PI;
						}
						else
						{
							/* panes 0 and 1 tied as front and side (third angle) */
							angle=PI/2.0;
						}
						/* first give each tied scene viewer the same lookat parameters,
							viewing volume (except for near and far) and projection_mode */
						for (pane_no=0;pane_no<window->number_of_panes;pane_no++)
						{
							Graphics_window_set_projection_mode(window,pane_no,projection_mode);
							Scene_viewer_set_lookat_parameters(window->scene_viewer_array[pane_no]->core_scene_viewer,
								eye[0],eye[1],eye[2],lookat[0],lookat[1],lookat[2],
								up[0],up[1],up[2]);
							Scene_viewer_set_viewing_volume(window->scene_viewer_array[pane_no]->core_scene_viewer,
								left,right,bottom,top,near_plane[pane_no],far_plane[pane_no]);
						}
						/* now rotate all tied panes but the changed_pane to get the proper
							relationship between them */
						switch (changed_pane)
						{
							case 0:
							{
								Scene_viewer_rotate_about_lookat_point(window->scene_viewer_array[1]->core_scene_viewer,
									up,angle);
								Scene_viewer_app_redraw_now(window->scene_viewer_array[1]);
							} break;
							case 1:
							{
								Scene_viewer_rotate_about_lookat_point(window->scene_viewer_array[0]->core_scene_viewer,
									up,-angle);
								Scene_viewer_app_redraw_now(window->scene_viewer_array[0]);
							} break;
						}
					}
				} break;
				case GRAPHICS_WINDOW_LAYOUT_PSEUDO_3D:
				{
					if (window->number_of_scene_viewers >= 2)
					{
						if (0.0<eye_distance)
						{
							/* panes 0 and 1 tied as right and left eye views, respectively */
							/* first give each tied scene viewer the same lookat parameters,
								viewing volume (except for near and far) and projection_mode */
							for (pane_no=0;pane_no<window->number_of_panes;pane_no++)
							{
								Graphics_window_set_projection_mode(window,pane_no,
									projection_mode);
								Scene_viewer_set_lookat_parameters(window->scene_viewer_array[pane_no]->core_scene_viewer,
									eye[0],eye[1],eye[2],lookat[0],lookat[1],lookat[2],
									up[0],up[1],up[2]);
								Scene_viewer_set_viewing_volume(window->scene_viewer_array[pane_no]->core_scene_viewer,
									left,right,bottom,top,near_plane[pane_no],far_plane[pane_no]);
							}
							/* now rotate all tied panes but the changed_pane to get the proper
								relationship between them */
							/* 2 panes show views from eyes separated by eye_spacing, and
								looking at the same lookat point. Hence, can calculate the angle
								difference between the views using the eye_distance. */
							angle=2.0*atan(0.5*window->eye_spacing/eye_distance);
							switch (changed_pane)
							{
								case 0:
								{
									Scene_viewer_rotate_about_lookat_point(window->scene_viewer_array[1]->core_scene_viewer,
										up,-angle);
									Scene_viewer_app_redraw_now(window->scene_viewer_array[1]);
								} break;
								case 1:
								{
									Scene_viewer_rotate_about_lookat_point(window->scene_viewer_array[0]->core_scene_viewer,
										up,angle);
									Scene_viewer_app_redraw_now(window->scene_viewer_array[0]);
								} break;
							}
						}
					}
				} break;
				default:
				{
					display_message(ERROR_MESSAGE,
					"Graphics_window_view_changed.  Unknown layout_mode");
					return_code=0;
				} break;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Graphics_window_view_changed.  Invalid scene_viewer");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_window_view_changed.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return return_code;
} /* Graphics_window_view_changed */

int view_and_up_vectors_to_xyz_rotations(double *view, double *up,
	double *rotations)
/*******************************************************************************
LAST MODIFIED : 6 April 2001

DESCRIPTION :
Deduces three <rotations> about the global x, global y and global z axes that
rotate the initial view (looking in the -z direction with y up) to look in the
<view> direction upright in the <up>-direction. The final up-vector will be
orthogonal with the view direction even if not initially; <up> may not be
co-linear with <view>.
Rotations are returned in radians. Note the results are non-unique.
These rotations are used in the MAYA viewing model.
==============================================================================*/
{
  int return_code;
	double c, m[9], norm_right[3], norm_view[3], ry[9], rz[9], s, x_view[3];

	ENTER(view_and_up_vectors_to_xyz_rotations);
	if (view && up && rotations)
	{
		norm_view[0] = view[0];
		norm_view[1] = view[1];
		norm_view[2] = view[2];
		/* establish right vector at view x up and normalise it and view */
		if ((0.0 != normalize3(norm_view)) &&
			cross_product3(norm_view, up, norm_right) &&
			(0.0 != normalize3(norm_right)))
		{
			/* Calculate the z and y rotations */
			rotations[2] = atan2(norm_right[1], norm_right[0]);
			rotations[1] = atan2(-norm_right[2],
				sqrt(norm_right[0]*norm_right[0] + norm_right[1]*norm_right[1]));

			/* create matrix ry that undoes the y rotation */
			identity_matrix(3, ry);
			s = sin(-rotations[1]);
			c = cos(-rotations[1]);
			ry[0] = c;
			ry[2] = s;
			ry[6] = -s;
			ry[8] = c;
			/* create matrix rz that undoes the z rotation */
			identity_matrix(3, rz);
			s = sin(-rotations[2]);
			c = cos(-rotations[2]);
			rz[0] = c;
			rz[1] = -s;
			rz[3] = s;
			rz[4] = c;
			/* multiply to get matrix m that undoes the z and y rotations */
			multiply_matrix(3, 3, 3, ry, rz, m);
			/* multiply norm_view by m to get initial view rotated just by x angle */
			multiply_matrix(3, 3, 1, m, norm_view, x_view);

			/* calculate the x-rotation */
			rotations[0] = atan2(x_view[1], -x_view[2]);

			return_code = 1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"view_and_up_vectors_to_xyz_rotations.  "
				"View and/or up vectors zero or not orthogonal");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"view_and_up_vectors_to_xyz_rotations.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* view_and_up_vectors_to_xyz_rotations */

int list_Graphics_window(struct Graphics_window *window,void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 6 April 2001

DESCRIPTION :
Writes the properties of the <window> to the command window.
==============================================================================*/
{
	char line[80],*name, *opengl_version, *opengl_vendor, *opengl_extensions;
	double bottom, depth_of_field, eye[3], far_plane, focal_depth,
		horizontal_view_angle, left, lookat[3],
		max_pixels_per_polygon, modelview_matrix[16],
		NDC_height, NDC_left, NDC_top, NDC_width,
		near_plane, projection_matrix[16], right, rotations[3],
		texture_height, texture_width, top,
		up[3], view[3], view_angle, viewport_left, viewport_top,
		viewport_pixels_per_unit_x, viewport_pixels_per_unit_y;
	enum Scene_viewer_buffering_mode buffering_mode;
	enum Scene_viewer_projection_mode projection_mode;
	enum cmzn_sceneviewer_transparency_mode transparency_mode;
	enum cmzn_sceneviewer_viewport_mode viewport_mode;
	int accumulation_buffer_depth,colour_buffer_depth,depth_buffer_depth,
		height, return_code,width,
		undistort_on,visual_id;
	unsigned transparency_layers;
	struct Texture *texture;

	ENTER(list_Graphics_window);
	USE_PARAMETER(dummy_void);
	if (window && window->scene_viewer_array)
	{
		cmzn_sceneviewer *first_sceneviewer = window->scene_viewer_array[0]->core_scene_viewer;
		display_message(INFORMATION_MESSAGE,"Graphics window : %s\n",window->name);
		buffering_mode=Scene_viewer_get_buffering_mode(first_sceneviewer);
		if (buffering_mode)
		{
			display_message(INFORMATION_MESSAGE,"  %s\n",
				Scene_viewer_buffering_mode_string(buffering_mode));
		}
		/* image */
		cmzn_region_id region = cmzn_scene_get_region_internal(window->scene);
		name = cmzn_region_get_relative_path(region, window->root_region);
		if (name)
		{
			display_message(INFORMATION_MESSAGE,"  scene: %s\n",name);
			DEALLOCATE(name);
		}
		cmzn_scenefilter_id filter = cmzn_sceneviewer_get_scenefilter(first_sceneviewer);
		name = cmzn_scenefilter_get_name(filter);
		if (name)
		{
			display_message(INFORMATION_MESSAGE,"  filter: %s\n",name);
			DEALLOCATE(name);
		}
		cmzn_scenefilter_destroy(&filter);

		if (first_sceneviewer->isLightingLocalViewer())
			display_message(INFORMATION_MESSAGE, "  local viewer lighting");
		else
			display_message(INFORMATION_MESSAGE, "  infinite viewer lighting");
		if (first_sceneviewer->isLightingTwoSided())
			display_message(INFORMATION_MESSAGE, "  two-sided lighting");
		else
			display_message(INFORMATION_MESSAGE, "  one-sided lighting");

		display_message(INFORMATION_MESSAGE,"  lights:\n");
		for_each_cmzn_light_in_Scene_viewer(first_sceneviewer,list_cmzn_light_name,
			(void *)"    ");
		/* layout */
		display_message(INFORMATION_MESSAGE,"  layout: %s\n",
			Graphics_window_layout_mode_string(window->layout_mode));
		display_message(INFORMATION_MESSAGE,
			"  orthographic up and front axes: %s %s\n",
			axis_name[window->ortho_up_axis],axis_name[window->ortho_front_axis]);
		display_message(INFORMATION_MESSAGE,
			"  eye_spacing: %g\n",window->eye_spacing);
		Graphics_window_get_viewing_area_size(window,&width,&height);
		display_message(INFORMATION_MESSAGE,"  viewing width x height: %d x %d\n",
			width,height);
		/* background and view in each active pane */
		for (int pane_no = 0; pane_no < window->number_of_panes; ++pane_no)
		{
			cmzn_sceneviewer *pane_sceneviewer = window->scene_viewer_array[pane_no]->core_scene_viewer;
			display_message(INFORMATION_MESSAGE,"  pane: %d\n",pane_no+1);
			/* background */
			double rgb[3];
			cmzn_sceneviewer_get_background_colour_rgb(pane_sceneviewer, rgb);
			display_message(INFORMATION_MESSAGE,
				"    background colour R G B: %g %g %g\n",
				rgb[0], rgb[1], rgb[2]);
			cmzn_field_image_id image_field=
				Scene_viewer_get_background_image_field(pane_sceneviewer);
			texture = cmzn_field_image_get_texture(image_field);
			cmzn_field_image_destroy(&image_field);
			if (texture && Scene_viewer_get_background_texture_info(pane_sceneviewer,
					&left,&top,&texture_width,&texture_height,&undistort_on,
					&max_pixels_per_polygon)&&
				GET_NAME(Texture)(texture,&name))
			{
				display_message(INFORMATION_MESSAGE,
					"    background texture %s\n",name);
				display_message(INFORMATION_MESSAGE,
					"      (placement: left=%g top=%g width=%g height=%g\n",
					left,top,texture_width,texture_height);
				display_message(INFORMATION_MESSAGE,"      (undistort: ");
				if (undistort_on)
				{
					display_message(INFORMATION_MESSAGE,"on");
				}
				else
				{
					display_message(INFORMATION_MESSAGE,"off");
				}
				display_message(INFORMATION_MESSAGE,
					", max pixels/polygon=%g)\n",max_pixels_per_polygon);
				DEALLOCATE(name);
			}
			/* view */
			Scene_viewer_get_projection_mode(pane_sceneviewer, &projection_mode);
			display_message(INFORMATION_MESSAGE,"    projection: %s\n",
				Scene_viewer_projection_mode_string(projection_mode));
			if (SCENE_VIEWER_CUSTOM==projection_mode)
			{
				Scene_viewer_get_modelview_matrix(pane_sceneviewer,modelview_matrix);
				Scene_viewer_get_projection_matrix(pane_sceneviewer,projection_matrix);
				display_message(INFORMATION_MESSAGE,
					"    modelview matrix  = | %13.6e %13.6e %13.6e %13.6e |\n",
					modelview_matrix[0],modelview_matrix[1],
					modelview_matrix[2],modelview_matrix[3]);
				display_message(INFORMATION_MESSAGE,
					"                      = | %13.6e %13.6e %13.6e %13.6e |\n",
					modelview_matrix[4],modelview_matrix[5],
					modelview_matrix[6],modelview_matrix[7]);
				display_message(INFORMATION_MESSAGE,
					"                      = | %13.6e %13.6e %13.6e %13.6e |\n",
					modelview_matrix[8],modelview_matrix[9],
					modelview_matrix[10],modelview_matrix[11]);
				display_message(INFORMATION_MESSAGE,
					"                      = | %13.6e %13.6e %13.6e %13.6e |\n",
					modelview_matrix[12],modelview_matrix[13],
					modelview_matrix[14],modelview_matrix[15]);
				display_message(INFORMATION_MESSAGE,
					"    projection matrix = | %13.6e %13.6e %13.6e %13.6e |\n",
					projection_matrix[0],projection_matrix[1],
					projection_matrix[2],projection_matrix[3]);
				display_message(INFORMATION_MESSAGE,
					"                      = | %13.6e %13.6e %13.6e %13.6e |\n",
					projection_matrix[4],projection_matrix[5],
					projection_matrix[6],projection_matrix[7]);
				display_message(INFORMATION_MESSAGE,
					"                      = | %13.6e %13.6e %13.6e %13.6e |\n",
					projection_matrix[8],projection_matrix[9],
					projection_matrix[10],projection_matrix[11]);
				display_message(INFORMATION_MESSAGE,
					"                      = | %13.6e %13.6e %13.6e %13.6e |\n",
					projection_matrix[12],projection_matrix[13],
					projection_matrix[14],projection_matrix[15]);
			}
			else
			{
				/* parallel/perspective: write eye and interest points and up-vector */
				cmzn_sceneviewer_get_lookat_parameters(pane_sceneviewer, eye, lookat, up);
				view[0] = lookat[0] - eye[0];
				view[1] = lookat[1] - eye[1];
				view[2] = lookat[2] - eye[2];
				Scene_viewer_get_viewing_volume(pane_sceneviewer,&left,&right,
					&bottom,&top,&near_plane,&far_plane);
				view_angle = cmzn_sceneviewer_get_view_angle(pane_sceneviewer);
				Scene_viewer_get_horizontal_view_angle(pane_sceneviewer,
					&horizontal_view_angle);
				display_message(INFORMATION_MESSAGE,
					"    eye point: %g %g %g\n",eye[0],eye[1],eye[2]);
				display_message(INFORMATION_MESSAGE,
					"    interest point: %g %g %g\n",lookat[0],lookat[1],lookat[2]);
				display_message(INFORMATION_MESSAGE,
					"    up vector: %g %g %g\n",up[0],up[1],up[2]);
				display_message(INFORMATION_MESSAGE,
					"    view angle (across NDC, degrees) diagonal, horizontal : %g %g\n",
					view_angle*(180/PI), horizontal_view_angle*(180/PI));

				display_message(INFORMATION_MESSAGE,
					"    near and far clipping planes: %g %g\n",near_plane,far_plane);
				/* work out if view is skew = up not orthogonal to view direction */
				eye[0] -= lookat[0];
				eye[1] -= lookat[1];
				eye[2] -= lookat[2];
				normalize3(eye);
				normalize3(up);
				if (0.00001<fabs(dot_product3(up,eye)))
				{
					display_message(INFORMATION_MESSAGE,
						"    view is skew (up-vector not orthogonal to view direction)\n");
				}
				else
				{
					if (view_and_up_vectors_to_xyz_rotations(view, up, rotations))
					{
						display_message(INFORMATION_MESSAGE,
							"    view rotations about x, y and z axes (degrees): %g %g %g\n",
							rotations[0]*(180.0/PI), rotations[1]*(180.0/PI),
							rotations[2]*(180.0/PI));
					}
				}
			}
			viewport_mode = cmzn_sceneviewer_get_viewport_mode(pane_sceneviewer);
			display_message(INFORMATION_MESSAGE,"    %s\n",
				cmzn_sceneviewer_viewport_mode_string(viewport_mode));
			Scene_viewer_get_NDC_info(pane_sceneviewer,
				&NDC_left,&NDC_top,&NDC_width,&NDC_height);
			display_message(INFORMATION_MESSAGE,
				"    NDC placement: left=%g top=%g width=%g height=%g\n",
				NDC_left,NDC_top,NDC_width,NDC_height);
			Scene_viewer_get_viewport_info(pane_sceneviewer,
				&viewport_left,&viewport_top,&viewport_pixels_per_unit_x,
				&viewport_pixels_per_unit_y);
			display_message(INFORMATION_MESSAGE,
				"    Viewport coordinates: left=%g top=%g pixels/unit x=%g y=%g\n",
				viewport_left,viewport_top,viewport_pixels_per_unit_x,
				viewport_pixels_per_unit_y);
			/* frame count */
			display_message(INFORMATION_MESSAGE,
				"    Rendered frame count: %d\n",
				Scene_viewer_get_frame_count(pane_sceneviewer));
		}

		/* settings */
		if (GET_NAME(Interactive_tool)(window->interactive_tool,&name))
		{
			display_message(INFORMATION_MESSAGE,"  Interactive tool: %s\n",name);
			DEALLOCATE(name);
		}
		transparency_mode = cmzn_sceneviewer_get_transparency_mode(first_sceneviewer);
		display_message(INFORMATION_MESSAGE,
			"  Transparency mode: %s\n",cmzn_sceneviewer_transparency_mode_string(
				transparency_mode));
		if (transparency_mode == CMZN_SCENEVIEWER_TRANSPARENCY_MODE_ORDER_INDEPENDENT)
		{
			transparency_layers = cmzn_sceneviewer_get_transparency_layers(first_sceneviewer);
			display_message(INFORMATION_MESSAGE,"    transparency_layers: %d\n",
				transparency_layers);
		}
		display_message(INFORMATION_MESSAGE,
			"  Current pane: %d\n",window->current_pane+1);
		display_message(INFORMATION_MESSAGE,
			"  Standard view angle: %g degrees\n",window->std_view_angle);
		bool perturb_lines = cmzn_sceneviewer_get_perturb_lines_flag(first_sceneviewer);
		if (perturb_lines)
		{
			display_message(INFORMATION_MESSAGE,"  perturbed lines: on\n");
		}
		else
		{
			display_message(INFORMATION_MESSAGE,"  perturbed lines: off\n");
		}
		int antialias = cmzn_sceneviewer_get_antialias_sampling(first_sceneviewer);
		if (antialias)
		{
			display_message(INFORMATION_MESSAGE,"  anti-aliasing at %d\n",antialias);
		}
		else
		{
			display_message(INFORMATION_MESSAGE,"  no anti-aliasing\n");
		}
		Scene_viewer_get_depth_of_field(first_sceneviewer,
			&depth_of_field, &focal_depth);
		if (depth_of_field > 0.0)
		{
			display_message(INFORMATION_MESSAGE,"  depth of field %lf\n", depth_of_field);
			display_message(INFORMATION_MESSAGE,"  focal depth %lf\n", focal_depth);
		}
		else
		{
			display_message(INFORMATION_MESSAGE,"  infinite depth of field\n");
		}
		enum cmzn_sceneviewer_blending_mode blending_mode =
			cmzn_sceneviewer_get_blending_mode(first_sceneviewer);
		display_message(INFORMATION_MESSAGE,"  blending_mode: %s\n",
			ENUMERATOR_STRING(cmzn_sceneviewer_blending_mode)(blending_mode));
		/* OpenGL information */
		if (Scene_viewer_get_opengl_information(window->scene_viewer_array[0],
			&opengl_version, &opengl_vendor, &opengl_extensions, &visual_id,
			&colour_buffer_depth, &depth_buffer_depth, &accumulation_buffer_depth))
		{
			display_message(INFORMATION_MESSAGE,"  OpenGL Information\n");
			display_message(INFORMATION_MESSAGE,"    Version %s\n", opengl_version);
			display_message(INFORMATION_MESSAGE,"    Vendor %s\n", opengl_vendor);
			display_message(INFORMATION_MESSAGE,"    Visual ID %d\n",visual_id);
			display_message(INFORMATION_MESSAGE,"    Colour buffer depth %d\n",colour_buffer_depth);
			display_message(INFORMATION_MESSAGE,"    Depth buffer depth %d\n",depth_buffer_depth);
			display_message(INFORMATION_MESSAGE,"    Accumulation buffer depth %d\n",accumulation_buffer_depth);
		}
		sprintf(line,"  access count = %i\n",window->access_count);
		display_message(INFORMATION_MESSAGE,line);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Graphics_window.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* list_Graphics_window */

int process_list_or_write_Graphics_window_commands(struct Graphics_window *window,
	class Process_list_or_write_command_class *process_message)
/*******************************************************************************
LAST MODIFIED : 15 August 2007

DESCRIPTION :
Writes the commands for creating the <window> and establishing the views in it
to the command window. Or writes the commands for creating the window
and establishing the views in it to the command window to a com file.
==============================================================================*/
{
	char *name,*prefix;
	double bottom, depth_of_field, eye[3],far_plane,
		focal_depth, left,lookat[3],max_pixels_per_polygon,modelview_matrix[16],
		NDC_height,NDC_left,NDC_top,NDC_width,near_plane,projection_matrix[16],right,
		texture_height,texture_width,top,up[3],view_angle,viewport_left,
		viewport_top,viewport_pixels_per_unit_x,viewport_pixels_per_unit_y;
	enum Scene_viewer_buffering_mode buffering_mode;
	enum Scene_viewer_projection_mode projection_mode;
	enum cmzn_sceneviewer_transparency_mode transparency_mode;
	enum cmzn_sceneviewer_viewport_mode viewport_mode;
	int height,i,pane_no,return_code,width,undistort_on;
	unsigned transparency_layers;
	struct Texture *texture;

	ENTER(process_list_or_write_Graphics_window_commands);
	if (window&&window->scene_viewer_array&& ALLOCATE(prefix,char,50+strlen(window->name)))
	{
		cmzn_sceneviewer *first_sceneviewer = window->scene_viewer_array[0]->core_scene_viewer;
		name=duplicate_string(window->name);
		if (name)
		{
			/* put quotes around name if it contains special characters */
			make_valid_token(&name);
			process_message->process_command(INFORMATION_MESSAGE,"gfx create window %s",name);
			DEALLOCATE(name);
		}
		buffering_mode=Scene_viewer_get_buffering_mode(first_sceneviewer);
		if (buffering_mode)
		{
			process_message->process_command(INFORMATION_MESSAGE," %s",
				Scene_viewer_buffering_mode_string(buffering_mode));
		}
		process_message->process_command(INFORMATION_MESSAGE,";\n");
		/* image */
		process_message->process_command(INFORMATION_MESSAGE,"gfx modify window %s image",
			window->name);
		cmzn_region_id region = cmzn_scene_get_region_internal(window->scene);
		name = cmzn_region_get_relative_path(region, window->root_region);
		if (name)
		{
			/* put quotes around name if it contains special characters */
			make_valid_token(&name);
			process_message->process_command(INFORMATION_MESSAGE," scene %s",name);
			DEALLOCATE(name);
		}
		cmzn_scenefilter_id filter = cmzn_sceneviewer_get_scenefilter(window->scene_viewer_array[0]->core_scene_viewer);
		name = cmzn_scenefilter_get_name(filter);
		if (name)
		{
			make_valid_token(&name);
			process_message->process_command(INFORMATION_MESSAGE," filter %s",name);
			DEALLOCATE(name);
		}
		cmzn_scenefilter_destroy(&filter);

		if (first_sceneviewer->isLightingLocalViewer())
			process_message->process_command(INFORMATION_MESSAGE, " local_viewer_lighting");
		else
			process_message->process_command(INFORMATION_MESSAGE, " infinite_viewer_lighting");
		if (first_sceneviewer->isLightingTwoSided())
			process_message->process_command(INFORMATION_MESSAGE, " two_sided_lighting");
		else
			process_message->process_command(INFORMATION_MESSAGE, " one_sided_lighting");

		process_message->process_command(INFORMATION_MESSAGE,";\n");
		sprintf(prefix,"gfx modify window %s image add_light ",window->name);
		for_each_cmzn_light_in_Scene_viewer(window->scene_viewer_array[0]->core_scene_viewer,
			list_cmzn_light_name_command, (void *)prefix);

		/* layout */
		Graphics_window_get_viewing_area_size(window,&width,&height);
		process_message->process_command(INFORMATION_MESSAGE,
			"gfx modify window %s layout %s ortho_axes %s %s eye_spacing %g"
			" width %d height %d;\n",window->name,
			Graphics_window_layout_mode_string(window->layout_mode),
			axis_name[window->ortho_up_axis],axis_name[window->ortho_front_axis],
			window->eye_spacing,width,height);
		/* background and view in each active pane */
		for (pane_no=0;pane_no<window->number_of_panes;pane_no++)
		{
			cmzn_sceneviewer *sceneviewer = window->scene_viewer_array[pane_no]->core_scene_viewer;
			process_message->process_command(INFORMATION_MESSAGE,
				"gfx modify window %s set current_pane %d;\n",
				window->name,pane_no+1);
			/* background */
			process_message->process_command(INFORMATION_MESSAGE,
				"gfx modify window %s background",window->name);
			double rgb[3];
			cmzn_sceneviewer_get_background_colour_rgb(sceneviewer, rgb);
			process_message->process_command(INFORMATION_MESSAGE,
				" colour %g %g %g", rgb[0], rgb[1], rgb[2]);
			cmzn_field_image_id image_field=
				Scene_viewer_get_background_image_field(sceneviewer);
			texture = cmzn_field_image_get_texture(image_field);
			cmzn_field_image_destroy(&image_field);
			if (texture &&	Scene_viewer_get_background_texture_info(sceneviewer,
					&left,&top,&texture_width,&texture_height,&undistort_on,
					&max_pixels_per_polygon)&&
				GET_NAME(Texture)(texture,&name))
			{
				/* put quotes around name if it contains special characters */
				make_valid_token(&name);
				process_message->process_command(INFORMATION_MESSAGE," texture %s",name);
				process_message->process_command(INFORMATION_MESSAGE," tex_placement %g %g %g %g",
					left,top,texture_width,texture_height);
				if (undistort_on)
				{
					process_message->process_command(INFORMATION_MESSAGE," undistort_texture");
				}
				else
				{
					process_message->process_command(INFORMATION_MESSAGE," no_undistort_texture");
				}
				process_message->process_command(INFORMATION_MESSAGE," max_pixels_per_polygon %g",
					max_pixels_per_polygon);
				DEALLOCATE(name);
			}
			else
			{
				process_message->process_command(INFORMATION_MESSAGE," texture none");
			}
			process_message->process_command(INFORMATION_MESSAGE,";\n");
			/* view */
			process_message->process_command(INFORMATION_MESSAGE,
				"gfx modify window %s view",window->name);
			Scene_viewer_get_projection_mode(sceneviewer, &projection_mode);
			process_message->process_command(INFORMATION_MESSAGE," %s",
				Scene_viewer_projection_mode_string(projection_mode));
			if (SCENE_VIEWER_CUSTOM==projection_mode)
			{
				Scene_viewer_get_modelview_matrix(sceneviewer,modelview_matrix);
				Scene_viewer_get_projection_matrix(sceneviewer,projection_matrix);
				process_message->process_command(INFORMATION_MESSAGE," modelview_matrix");
				for (i=0;i<16;i++)
				{
					process_message->process_command(INFORMATION_MESSAGE," %13.6e",modelview_matrix[i]);
				}
				process_message->process_command(INFORMATION_MESSAGE," projection_matrix");
				for (i=0;i<16;i++)
				{
					process_message->process_command(INFORMATION_MESSAGE," %13.6e",projection_matrix[i]);
				}
			}
			else
			{
				/* parallel/perspective: write eye and interest points and up-vector */
				cmzn_sceneviewer_get_lookat_parameters(sceneviewer, eye, lookat, up);
				Scene_viewer_get_viewing_volume(sceneviewer,&left,&right,
					&bottom,&top,&near_plane,&far_plane);
				view_angle = cmzn_sceneviewer_get_view_angle(sceneviewer);
				process_message->process_command(INFORMATION_MESSAGE,
					" eye_point %g %g %g",eye[0],eye[1],eye[2]);
				process_message->process_command(INFORMATION_MESSAGE,
					" interest_point %g %g %g",lookat[0],lookat[1],lookat[2]);
				process_message->process_command(INFORMATION_MESSAGE,
					" up_vector %g %g %g",up[0],up[1],up[2]);
				process_message->process_command(INFORMATION_MESSAGE,
					" view_angle %g",view_angle*180/PI);
				process_message->process_command(INFORMATION_MESSAGE,
					" near_clipping_plane %g far_clipping_plane %g",near_plane,far_plane);
				/* work out if view is skew = up not orthogonal to view direction */
				eye[0] -= lookat[0];
				eye[1] -= lookat[1];
				eye[2] -= lookat[2];
				normalize3(eye);
				normalize3(up);
				if (0.00001<fabs(dot_product3(up,eye)))
				{
					process_message->process_command(INFORMATION_MESSAGE," allow_skew");
				}
			}
			viewport_mode = cmzn_sceneviewer_get_viewport_mode(sceneviewer);
			process_message->process_command(INFORMATION_MESSAGE," %s",
				cmzn_sceneviewer_viewport_mode_string(viewport_mode));


			Scene_viewer_get_NDC_info(sceneviewer,
				&NDC_left,&NDC_top,&NDC_width,&NDC_height);
			process_message->process_command(INFORMATION_MESSAGE," ndc_placement %g %g %g %g",
				NDC_left,NDC_top,NDC_width,NDC_height);
			Scene_viewer_get_viewport_info(sceneviewer,
				&viewport_left,&viewport_top,&viewport_pixels_per_unit_x,
				&viewport_pixels_per_unit_y);
			process_message->process_command(INFORMATION_MESSAGE,
				" viewport_coordinates %g %g %g %g",viewport_left,viewport_top,
				viewport_pixels_per_unit_x,viewport_pixels_per_unit_y);
			process_message->process_command(INFORMATION_MESSAGE,";\n");
		}
		/* settings */
		process_message->process_command(INFORMATION_MESSAGE,
			"gfx modify window %s set",window->name);
		if (GET_NAME(Interactive_tool)(window->interactive_tool,&name))
		{
			process_message->process_command(INFORMATION_MESSAGE," %s",name);
			DEALLOCATE(name);
		}
		process_message->process_command(INFORMATION_MESSAGE,
			" current_pane %d",window->current_pane+1);
		process_message->process_command(INFORMATION_MESSAGE,
			" std_view_angle %g",window->std_view_angle);
		bool perturb_lines = cmzn_sceneviewer_get_perturb_lines_flag(window->scene_viewer_array[0]->core_scene_viewer);
		if (perturb_lines)
		{
			process_message->process_command(INFORMATION_MESSAGE," perturb_lines");
		}
		else
		{
			process_message->process_command(INFORMATION_MESSAGE," normal_lines");
		}
		int antialias = cmzn_sceneviewer_get_antialias_sampling(window->scene_viewer_array[0]->core_scene_viewer);
		if (antialias)
		{
			process_message->process_command(INFORMATION_MESSAGE," antialias %d",antialias);
		}
		else
		{
			process_message->process_command(INFORMATION_MESSAGE," no_antialias");
		}
		Scene_viewer_get_depth_of_field(window->scene_viewer_array[0]->core_scene_viewer,
			&depth_of_field, &focal_depth);
		if (depth_of_field > 0.0)
		{
			process_message->process_command(INFORMATION_MESSAGE," depth_of_field %lf", depth_of_field);
			process_message->process_command(INFORMATION_MESSAGE," focal_depth %lf", focal_depth);
		}
		else
		{
			process_message->process_command(INFORMATION_MESSAGE," depth_of_field 0.0");
		}
		cmzn_sceneviewer *sceneviewer = window->scene_viewer_array[0]->core_scene_viewer;
		transparency_mode = cmzn_sceneviewer_get_transparency_mode(sceneviewer);
		process_message->process_command(INFORMATION_MESSAGE," %s",
			cmzn_sceneviewer_transparency_mode_string(transparency_mode));
		if (transparency_mode == CMZN_SCENEVIEWER_TRANSPARENCY_MODE_ORDER_INDEPENDENT)
		{
			transparency_layers = cmzn_sceneviewer_get_transparency_layers(sceneviewer);
			process_message->process_command(INFORMATION_MESSAGE," %d",transparency_layers);
		}
		enum cmzn_sceneviewer_blending_mode blending_mode =
			cmzn_sceneviewer_get_blending_mode(window->scene_viewer_array[0]->core_scene_viewer);
		process_message->process_command(INFORMATION_MESSAGE," %s",
			ENUMERATOR_STRING(cmzn_sceneviewer_blending_mode)(blending_mode));
		process_message->process_command(INFORMATION_MESSAGE,";\n");
		DEALLOCATE(prefix);
		return_code=1;
	}
	else
	{
		process_message->process_command(ERROR_MESSAGE,
			"process_list_or_write_Graphics_window_commands.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* process_list_or_write_Graphics_window_commands */

int list_Graphics_window_commands(struct Graphics_window *window,
	 void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 15 August 2007

DESCRIPTION :
Writes the commands for creating the <window> and establishing the views in it
to the command window. Or writes the commadns for creating the window
and establishing the views in it to the command window to a com file.
==============================================================================*/
{
	int return_code = 0;

	ENTER(list_Graphics_window_commands);
	USE_PARAMETER(dummy_void);
	Process_list_command_class *list_message =
		new Process_list_command_class();
	if (list_message)
	{
		return_code = process_list_or_write_Graphics_window_commands(window, list_message);
		delete list_message;
	}
	LEAVE;

	return (return_code);
}

int write_Graphics_window_commands_to_comfile(struct Graphics_window *window,
	 void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 15 August 2007

DESCRIPTION :
Writes the commadns for creating the window
and establishing the views in it to the command window to a com file.
==============================================================================*/
{
	int return_code = 0;

	ENTER(write_Graphics_window_commands_to_comfile);
	USE_PARAMETER(dummy_void);
	Process_write_command_class *write_message =
		new Process_write_command_class();
	if (write_message)
	{
		return_code = process_list_or_write_Graphics_window_commands(window, write_message);
		delete write_message;
	}
	LEAVE;

	return (return_code);
}

#if defined (WX_USER_INTERFACE)
static int modify_Graphics_window_node_tool(struct Parse_state *state,
	 void *window_void, void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 18 February 2008

DESCRIPTION :
Executes a GFX NODE_TOOL or GFX_DATA_TOOL command. If <data_tool_flag> is set,
then the <data_tool> is being modified, otherwise the <node_tool>.
Which tool that is being modified is passed in <node_tool_void>.
==============================================================================*/
{
	int return_code = 0;
	Graphics_window *window = NULL;
	Node_tool *node_tool = NULL;
	USE_PARAMETER(dummy_void);
	ENTER(execute_command_gfx_node_tool);
	if (state)
	{
		window = (Graphics_window *)window_void;
		if (window)
		{
			Interactive_tool *interactive_tool = FIND_BY_IDENTIFIER_IN_MANAGER(Interactive_tool,name)("node_tool", window->interactive_tool_manager);
			if (interactive_tool)
			{
				node_tool = static_cast<Node_tool *>(Interactive_tool_get_tool_data(interactive_tool));
			}
		}
		return_code = Node_tool_execute_command_with_parse_state(node_tool, state);
		if (node_tool && return_code)
			Node_tool_set_wx_interface(node_tool);
	}
	else
	{
		 display_message(ERROR_MESSAGE,
				"execute_command_gfx_node_tool.  Invalid argument(s)");
		 return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_gfx_node_tool */
#endif /* defined (GTK_USER_INTERFACE) || defined
			  (WIN32_USER_INTERFACE) || defined (CARBON_USER_INTERFACE) */

int modify_Graphics_window(struct Parse_state *state,void *help_mode,
	void *modify_graphics_window_data_void)
/*******************************************************************************
LAST MODIFIED : 18 November 1998

DESCRIPTION :
Parser commands for modifying graphics windows - views, lighting, etc.
See comments with struct Modify_graphics_window_data;
Parameter <help_mode> should be NULL when calling this function.
==============================================================================*/
{
	int return_code;
	struct Graphics_window *window;
	struct Modify_graphics_window_data *modify_graphics_window_data;
	struct Option_table *help_option_table, *option_table,
		*valid_window_option_table;

	ENTER(modify_Graphics_window);
	if (state)
	{
		modify_graphics_window_data=(struct Modify_graphics_window_data *)modify_graphics_window_data_void;
		if (modify_graphics_window_data)
		{
			return_code=1;
			/* initialize defaults */
			window=(struct Graphics_window *)NULL;
			if (!help_mode)
			{
				if ((!state->current_token)||
					(strcmp(PARSER_HELP_STRING,state->current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
				{
					option_table = CREATE(Option_table)();
					/* default: graphics window */
					Option_table_add_entry(option_table, NULL, (void *)&window,
						(void *)modify_graphics_window_data->graphics_window_manager,
						set_Graphics_window);
					return_code=Option_table_parse(option_table, state);
					DESTROY(Option_table)(&option_table);
				}
				else
				{
					help_option_table = CREATE(Option_table)();
					/* write help - use help_mode flag to get correct behaviour */
					Option_table_add_entry(help_option_table, "GRAPHICS_WINDOW_NAME",
						(void *)1, modify_graphics_window_data_void,
						modify_Graphics_window);
					return_code=Option_table_parse(help_option_table, state);
					DESTROY(Option_table)(&help_option_table);
				}
			}
			if (return_code)
			{
				valid_window_option_table = CREATE(Option_table)();
				Option_table_add_entry(valid_window_option_table, "background",
						(void *)window, modify_graphics_window_data_void,
						modify_Graphics_window_background);
				Option_table_add_entry(valid_window_option_table, "image",
						(void *)window, modify_graphics_window_data_void,
						modify_Graphics_window_image);
				Option_table_add_entry(valid_window_option_table, "layout",
						(void *)window, modify_graphics_window_data_void,
						modify_Graphics_window_layout);
#if defined (WX_USER_INTERFACE)
				Option_table_add_entry(valid_window_option_table, "node_tool",
						(void *)window, NULL, modify_Graphics_window_node_tool);
#endif // defined (WX_USER_INTERFACE)
				Option_table_add_entry(valid_window_option_table, "overlay",
						(void *)window, modify_graphics_window_data_void,
						modify_Graphics_window_overlay);
				Option_table_add_entry(valid_window_option_table, "set",
						(void *)window, modify_graphics_window_data_void,
						modify_Graphics_window_set);
				Option_table_add_entry(valid_window_option_table, "view",
						(void *)window, modify_graphics_window_data_void,
						modify_Graphics_window_view);
				return_code = Option_table_parse(valid_window_option_table, state);
				DESTROY(Option_table)(&valid_window_option_table);
			}
			if (window)
			{
				DEACCESS(Graphics_window)(&window);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"modify_Graphics_window.  Missing modify_graphics_window_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"modify_Graphics_window.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* modify_Graphics_window */

int set_Graphics_window(struct Parse_state *state,void *window_address_void,
	void *graphics_window_manager_void)
/*******************************************************************************
LAST MODIFIED : 10 December 1997

DESCRIPTION :
Used in command parsing to translate a window name into a Graphics_window.
NOTE: Calling function must remember to ACCESS any window passed to this
function, and DEACCESS any returned window.
???RC set_Object routines could become a macro.
==============================================================================*/
{
	const char *current_token;
	int return_code;
	struct Graphics_window *window,**window_address;
	struct MANAGER(Graphics_window) *graphics_window_manager;

	ENTER(set_Graphics_window);
	if (state)
	{
		current_token=state->current_token;
		if (current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				if ((window_address=(struct Graphics_window **)window_address_void)&&
					(graphics_window_manager=
					(struct MANAGER(Graphics_window) *)graphics_window_manager_void))
				{
					if ((window=FIND_BY_IDENTIFIER_IN_MANAGER(Graphics_window,name)(
						current_token,graphics_window_manager)))
					{
						ACCESS(Graphics_window)(window);
						if (*window_address)
						{
							DEACCESS(Graphics_window)(window_address);
						}
						*window_address=window;
						return_code=shift_Parse_state(state,1);
					}
					else
					{
						display_message(WARNING_MESSAGE,"Unknown window: %s",current_token);
						display_parse_state_location(state);
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"set_Graphics_window.  Invalid argument(s)");
					return_code=0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE," WINDOW_NUMBER");
				if ((window_address=(struct Graphics_window **)window_address_void)&&
					(window= *window_address))
				{
					display_message(INFORMATION_MESSAGE,"[%s]",window->name);
				}
				return_code=1;
			}
		}
		else
		{
			display_message(WARNING_MESSAGE,"Missing window number");
			display_parse_state_location(state);
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_Graphics_window.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_Graphics_window */

const char *Graphics_window_layout_mode_string(
	enum Graphics_window_layout_mode layout_mode)
/*******************************************************************************
LAST MODIFIED : 28 June 2000

DESCRIPTION :
Returns a string label for the <layout_mode>, used in widgets and parsing.
NOTE: Calling function must not deallocate returned string.
==============================================================================*/
{
	const char *return_string;

	ENTER(Graphics_window_layout_mode_string);
	switch (layout_mode)
	{
		case GRAPHICS_WINDOW_LAYOUT_2D:
		{
			return_string="2d";
		} break;
		case GRAPHICS_WINDOW_LAYOUT_FREE_ORTHO:
		{
			return_string="free_ortho";
		} break;
		case GRAPHICS_WINDOW_LAYOUT_FRONT_BACK:
		{
			return_string="front_back";
		} break;
		case GRAPHICS_WINDOW_LAYOUT_FRONT_SIDE:
		{
			return_string="front_side";
		} break;
		case GRAPHICS_WINDOW_LAYOUT_ORTHOGRAPHIC:
		{
			return_string="orthographic";
		} break;
		case GRAPHICS_WINDOW_LAYOUT_PSEUDO_3D:
		{
			return_string="pseudo_3d";
		} break;
		case GRAPHICS_WINDOW_LAYOUT_SIMPLE:
		{
			return_string="simple";
		} break;
		case GRAPHICS_WINDOW_LAYOUT_TWO_FREE:
		{
			return_string="two_free";
		} break;
		default:
		{
			display_message(ERROR_MESSAGE,
				"Graphics_window_layout_mode_string.  Unknown layout mode");
			return_string=(char *)NULL;
		}
	}
	LEAVE;

	return (return_string);
} /* Graphics_window_layout_mode_string */

const char **Graphics_window_layout_mode_get_valid_strings(
	int *number_of_valid_strings)
/*******************************************************************************
LAST MODIFIED : 28 June 2000

DESCRIPTION :
Returns an allocated array of pointers to all static strings for valid
Graphics_window_layout_modes - obtained from function
Graphics_window_layout_mode_string.
Up to calling function to deallocate returned array - but not the strings in it!
==============================================================================*/
{
	const char **valid_strings;
	int i, layout_mode;

	ENTER(Graphics_window_layout_mode_get_valid_strings);
	if (number_of_valid_strings)
	{
		*number_of_valid_strings=0;
		layout_mode=static_cast<int>(GRAPHICS_WINDOW_LAYOUT_MODE_BEFORE_FIRST);
		layout_mode++;
		while (layout_mode < static_cast<int>(GRAPHICS_WINDOW_LAYOUT_MODE_AFTER_LAST))
		{
			(*number_of_valid_strings)++;
			layout_mode++;
		}
		if (ALLOCATE(valid_strings,const char *,*number_of_valid_strings))
		{
			layout_mode=static_cast<int>(GRAPHICS_WINDOW_LAYOUT_MODE_BEFORE_FIRST);
			layout_mode++;
			i=0;
			while (layout_mode < static_cast<int>(GRAPHICS_WINDOW_LAYOUT_MODE_AFTER_LAST))
			{
				valid_strings[i]=(const char *)Graphics_window_layout_mode_string(
					static_cast<enum Graphics_window_layout_mode>(layout_mode));
				i++;
				layout_mode++;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Graphics_window_layout_mode_get_valid_strings.  Not enough memory");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_window_layout_mode_get_valid_strings.  Invalid argument");
		valid_strings=(const char **)NULL;
	}
	LEAVE;

	return (valid_strings);
} /* Graphics_window_layout_mode_get_valid_strings */

enum Graphics_window_layout_mode Graphics_window_layout_mode_from_string(
	const char *layout_mode_string)
/*******************************************************************************
LAST MODIFIED : 28 June 2000

DESCRIPTION :
Returns the <Graphics_window_layout_mode> described by <layout_mode_string>,
or GRAPHICS_WINDOW_LAYOUT_MODE_INVALID if not recognized.
==============================================================================*/
{
	int layout_mode;

	ENTER(Graphics_window_layout_mode_from_string);
	if (layout_mode_string)
	{
		layout_mode=static_cast<int>(GRAPHICS_WINDOW_LAYOUT_MODE_BEFORE_FIRST);
		layout_mode++;
		while ((layout_mode < static_cast<int>(GRAPHICS_WINDOW_LAYOUT_MODE_AFTER_LAST))&&
			(!fuzzy_string_compare_same_length(layout_mode_string,
				Graphics_window_layout_mode_string(
					static_cast<enum Graphics_window_layout_mode>(layout_mode)))))
		{
			layout_mode++;
		}
		if (static_cast<int>(GRAPHICS_WINDOW_LAYOUT_MODE_AFTER_LAST)==layout_mode)
		{
			layout_mode=static_cast<int>(GRAPHICS_WINDOW_LAYOUT_MODE_INVALID);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_window_layout_mode_from_string.  Invalid argument");
		layout_mode=GRAPHICS_WINDOW_LAYOUT_MODE_INVALID;
	}
	LEAVE;

	return (static_cast<enum Graphics_window_layout_mode>(layout_mode));
} /* Graphics_window_layout_mode_from_string */

int Graphics_window_layout_mode_get_number_of_panes(
	enum Graphics_window_layout_mode layout_mode)
/*******************************************************************************
LAST MODIFIED : 28 June 2000

DESCRIPTION :
Returns the number of panes in a graphics window with the given <layout_mode>.
==============================================================================*/
{
	int number_of_panes;

	ENTER(Graphics_window_layout_mode_get_number_of_panes);
	switch (layout_mode)
	{
		case GRAPHICS_WINDOW_LAYOUT_SIMPLE:
		case GRAPHICS_WINDOW_LAYOUT_2D:
		{
			number_of_panes=1;
		} break;
		case GRAPHICS_WINDOW_LAYOUT_ORTHOGRAPHIC:
		case GRAPHICS_WINDOW_LAYOUT_FREE_ORTHO:
		{
			number_of_panes=4;
		} break;
		case GRAPHICS_WINDOW_LAYOUT_FRONT_BACK:
		case GRAPHICS_WINDOW_LAYOUT_FRONT_SIDE:
		case GRAPHICS_WINDOW_LAYOUT_PSEUDO_3D:
		case GRAPHICS_WINDOW_LAYOUT_TWO_FREE:
		{
			number_of_panes=2;
		} break;
		default:
		{
			display_message(ERROR_MESSAGE,
				"Graphics_window_layout_mode_get_number_of_panes.  Unknown layout mode");
			number_of_panes=0;
		}
	}
	LEAVE;

	return (number_of_panes);
} /* Graphics_window_layout_mode_get_number_of_panes */

int Graphics_window_layout_mode_is_projection_mode_valid_for_pane(
	enum Graphics_window_layout_mode layout_mode,int pane_no,
	enum Scene_viewer_projection_mode projection_mode)
/*******************************************************************************
LAST MODIFIED : 28 June 2000

DESCRIPTION :
Returns true if the <projection_mode> can be used with pane <pane_no> of a
graphics window with the given <layout_mode>.
==============================================================================*/
{
	int return_code;

	ENTER(Graphics_window_layout_mode_is_projection_mode_valid_for_pane);
	switch (layout_mode)
	{
		case GRAPHICS_WINDOW_LAYOUT_SIMPLE:
		{
			return_code=((0==pane_no)&&(
				(SCENE_VIEWER_PARALLEL==projection_mode)||
				(SCENE_VIEWER_PERSPECTIVE==projection_mode)||
				(SCENE_VIEWER_CUSTOM==projection_mode)));
		} break;
		case GRAPHICS_WINDOW_LAYOUT_2D:
		{
			return_code=((0==pane_no)&&(
				(SCENE_VIEWER_PARALLEL==projection_mode)||
				(SCENE_VIEWER_PERSPECTIVE==projection_mode)));
		} break;
		case GRAPHICS_WINDOW_LAYOUT_ORTHOGRAPHIC:
		case GRAPHICS_WINDOW_LAYOUT_FREE_ORTHO:
		{
			return_code=((0==pane_no)&&(
				(SCENE_VIEWER_PARALLEL==projection_mode)||
				(SCENE_VIEWER_PERSPECTIVE==projection_mode)))||
				((1<=pane_no)&&(4>pane_no)&&(SCENE_VIEWER_PARALLEL==projection_mode));
		} break;
		case GRAPHICS_WINDOW_LAYOUT_FRONT_BACK:
		case GRAPHICS_WINDOW_LAYOUT_FRONT_SIDE:
		case GRAPHICS_WINDOW_LAYOUT_PSEUDO_3D:
		case GRAPHICS_WINDOW_LAYOUT_TWO_FREE:
		{
			return_code=(0<=pane_no)&&(2>pane_no)&&(
				(SCENE_VIEWER_PARALLEL==projection_mode)||
				(SCENE_VIEWER_PERSPECTIVE==projection_mode));
		} break;
		default:
		{
			display_message(ERROR_MESSAGE,
				"Graphics_window_layout_mode_is_projection_mode_valid_for_pane.  "
				"Unknown layout mode");
			return_code=0;
		}
	}
	LEAVE;

	return (return_code);
} /* Graphics_window_layout_mode_is_projection_mode_valid_for_pane */

#if defined (WX_USER_INTERFACE)
wxScrolledWindow *Graphics_window_get_interactive_tool_panel(struct Graphics_window *graphics_window)
/*******************************************************************************
LAST MODIFIED : 9 February 2007

DESCRIPTION :
Returns the panel to embed the interactive tool into.
==============================================================================*/
{
	 wxScrolledWindow *panel;
	ENTER(Graphics_window_get_interactive_tool_panel);
	if (graphics_window)
	{
		 panel = graphics_window->ToolPanel;
		 panel->SetScrollbars(10, 20, 25, 50);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_window_layout_mode_from_string.  Invalid argument");
		panel = (wxScrolledWindow *)NULL;
	}
	LEAVE;

	return (panel);
} /* Graphics_window_get_interactive_tool_panel */
#endif /* defined (WX_USER_INTERFACE) */

