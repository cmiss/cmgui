/*******************************************************************************
FILE : cmgui.c

LAST MODIFIED : 18 April 2002

DESCRIPTION :
???DB.  Prototype main program for an application that uses the "cmgui tools".
	It is imagined that this will include - cmgui, emap and dfn.  Started as
	general/cmgui.c .
???DB.  Can all the #if defined (MOTIF) and #if defined (WIN32_USER_INTERFACE) be hidden in
	macros defined in user_interface.h ?
???DB.  global_database.h has been removed.  It was specific to cmgui, and
	couldn't be extended to a mix and match use of tools.  I see two possibilities
	1 Managers/globals are passed.  I am going to try this, but I'm not sure what
		happens with automatic creation of managers.
	2 Managers/globals have externs in the modules that need them.  I don't like
		this because it fixes the names and is less clear.
???DB.  Add an argument which causes the command tree to be written to stdout
==============================================================================*/
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#if defined (WIN32_USER_INTERFACE)
#include <windows.h>
#endif /* defined (WIN32_USER_INTERFACE) */
/*SAB I have concatenated the correct version file for each version
  externally in the shell with cat #include "version.h"*/
#include "comfile/comfile_window.h"
#include "command/cmiss.h"
#include "command/command.h"
#if defined (MOTIF) || defined (WIN32_USER_INTERFACE)
#include "command/command_window.h"
#endif /* defined (MOTIF) || defined (WIN32_USER_INTERFACE) */
#include "command/console.h"
#if defined (MOTIF)
#include "element/element_creator.h"
#include "element/element_point_tool.h"
#include "element/element_point_viewer.h"
#include "element/element_tool.h"
#endif /* defined (MOTIF) */
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_component_operations.h"
#include "computed_field/computed_field_compose.h"
#include "computed_field/computed_field_composite.h"
#include "computed_field/computed_field_coordinate.h"
#include "computed_field/computed_field_control_curve.h"
#include "computed_field/computed_field_deformation.h"
#include "computed_field/computed_field_derivatives.h"
#include "computed_field/computed_field_external.h"
#include "computed_field/computed_field_finite_element.h"
#include "computed_field/computed_field_fibres.h"
#include "computed_field/computed_field_integration.h"
#include "computed_field/computed_field_matrix_operations.h"
#include "computed_field/computed_field_sample_texture.h"
#include "computed_field/computed_field_vector_operations.h"
#if defined (MOTIF)
#include "computed_field/computed_field_window_projection.h"
#endif /* defined (MOTIF) */
#include "finite_element/finite_element.h"
#if defined (MOTIF)
#include "finite_element/grid_field_calculator.h"
#endif /* defined (MOTIF) */
#include "general/debug.h"
#include "general/error_handler.h"
#include "general/image_utilities.h"
#include "graphics/environment_map.h"
#include "graphics/glyph.h"
#if defined (MOTIF)
#include "graphics/graphics_window.h"
#endif /* defined (MOTIF) */
#include "graphics/light.h"
#include "graphics/light_model.h"
#include "graphics/material.h"
#if defined (MOTIF)
#include "graphics/movie_graphics.h"
#endif /* defined (MOTIF) */
#include "graphics/scene.h"
#if defined (MOTIF)
#include "graphics/scene_editor.h"
#endif /* defined (MOTIF) */
#include "graphics/spectrum.h"
#if defined (MOTIF)
#include "graphics/transform_tool.h"
#endif /* defined (MOTIF) */
#include "graphics/volume_texture.h"
#if defined (MOTIF)
#include "interaction/interactive_tool.h"
#include "interaction/select_tool.h"
#endif /* defined (MOTIF) */
#include "io_devices/conversion.h"
#if defined (SELECT_DESCRIPTORS)
#include "io_devices/io_device.h"
#endif /* defined (SELECT_DESCRIPTORS) */
#if defined (MOTIF)
#include "node/node_tool.h"
#include "node/node_viewer.h"
#endif /* defined (MOTIF) */
#include "selection/any_object_selection.h"
#include "selection/element_point_ranges_selection.h"
#include "selection/element_selection.h"
#include "selection/node_selection.h"
#if defined (SGI_MOVIE_FILE)
#include "three_d_drawing/movie_extensions.h"
#endif /* defined (SGI_MOVIE_FILE) */
#include "time/time_keeper.h"
#include "user_interface/event_dispatcher.h"
#if defined (MOTIF)
#include "user_interface/filedir.h"
#endif /* defined (MOTIF) */
#include "user_interface/message.h"
#include "user_interface/user_interface.h"
#include "curve/control_curve.h"
#if defined (MOTIF)
#include "view/coord.h"
#endif /* defined (MOTIF) */
#include "unemap/unemap_package.h"
#if defined (CELL)
#include "cell/cell_interface.h"
#endif /* defined (CELL) */
#if defined (F90_INTERPRETER)
#include "command/f90_interpreter.h"
#else /* defined (F90_INTERPRETER) */
#if defined (PERL_INTERPRETER)
#include "perl_interpreter.h"
#endif /* defined (PERL_INTERPRETER) */
#endif /* defined (F90_INTERPRETER) */

/*
Global variables
----------------
*/

/*
Module types
------------
*/
typedef struct
/*******************************************************************************
LAST MODIFIED : 12 December 1996

DESCRIPTION :
==============================================================================*/
{
#if defined (MOTIF)
	Pixel background_colour,foreground_colour;
#endif /* defined (MOTIF) */
	char *examples_directory,*help_directory,*help_url,*startup_comfile;
} User_settings;

/*
Module variables
----------------
*/

/*
Module functions
----------------
*/
#if defined (MOTIF) || defined (WIN32_USER_INTERFACE)
static int display_error_message(char *message,void *command_window_void)
/*******************************************************************************
LAST MODIFIED : 25 July 1999

DESCRIPTION :
Display a cmgui error message.
==============================================================================*/
{
	int return_code;

	ENTER(display_error_message);
	if (command_window_void)
	{
		write_command_window("ERROR: ",
			(struct Command_window *)command_window_void);
		return_code=write_command_window(message,
			(struct Command_window *)command_window_void);
		write_command_window("\n",(struct Command_window *)command_window_void);
	}
	else
	{
		printf("ERROR: %s\n",message);
	}
	LEAVE;

	return (return_code);
} /* display_error_message */
#endif /* defined (MOTIF) || defined (WIN32_USER_INTERFACE) */

#if defined (MOTIF) || defined (WIN32_USER_INTERFACE)
static int display_information_message(char *message,void *command_window_void)
/*******************************************************************************
LAST MODIFIED : 25 July 1999

DESCRIPTION :
Display a cmgui information message.
==============================================================================*/
{
	int return_code;

	ENTER(display_error_message);
	if (command_window_void)
	{
		return_code=write_command_window(message,
			(struct Command_window *)command_window_void);
	}
	else
	{
		printf(message);
	}
	LEAVE;

	return (return_code);
} /* display_information_message */
#endif /* defined (MOTIF) || defined (WIN32_USER_INTERFACE) */

#if defined (MOTIF) || defined (WIN32_USER_INTERFACE)
static int display_warning_message(char *message,void *command_window_void)
/*******************************************************************************
LAST MODIFIED : 25 July 1999

DESCRIPTION :
Display a cmgui warning message.
???DB.  write_output is for the command_window - needs a better name.
==============================================================================*/
{
	int return_code;

	ENTER(display_warning_message);
	if (command_window_void)
	{
		write_command_window("WARNING: ",
			(struct Command_window *)command_window_void);
		return_code=write_command_window(message,
			(struct Command_window *)command_window_void);
		write_command_window("\n",(struct Command_window *)command_window_void);
	}
	else
	{
		printf("WARNING: %s\n",message);
	}
	LEAVE;

	return (return_code);
} /* display_warning_message */
#endif /* defined (MOTIF) || defined (WIN32_USER_INTERFACE) */

static int cmgui_execute_comfile(char *comfile_name,char *example_id,
	char *examples_directory,char *example_symbol,char **example_comfile_name,
	struct Execute_command *execute_command)
/*******************************************************************************
LAST MODIFIED : 16 October 1998

DESCRIPTION :
Executes the comfile specified on the command line.
==============================================================================*/
{
	int return_code;

	ENTER(cmgui_execute_comfile);
	return_code=0;
	if ((comfile_name||example_id)&&execute_command)
	{
		if (example_id)
		{
			if (examples_directory&&example_symbol)
			{
				/* set the examples directory */
				sprintf(global_temp_string,"set dir ");
				strcat(global_temp_string,example_symbol);
				strcat(global_temp_string,"=");
				strcat(global_temp_string,example_id);
				Execute_command_execute_string(execute_command,global_temp_string);
				sprintf(global_temp_string,"open comfile ");
				if (comfile_name)
				{
					strcat(global_temp_string,comfile_name);
				}
				else
				{
					if (*example_comfile_name)
					{
						strcat(global_temp_string,*example_comfile_name);
					}
					else
					{
						strcat(global_temp_string,"example_");
						strcat(global_temp_string,example_id);
					}
				}
				strcat(global_temp_string,";");
				strcat(global_temp_string,example_symbol);
				strcat(global_temp_string," execute");
				return_code=Execute_command_execute_string(execute_command,global_temp_string);
			}
			else
			{
				display_message(ERROR_MESSAGE,
				"cmgui_execute_comfile.  Missing examples_directory or example_symbol");
			}
		}
		else
		{
			/* open the command line comfile */
			sprintf(global_temp_string,"open comfile ");
			strcat(global_temp_string,comfile_name);
			strcat(global_temp_string," execute");
			return_code=Execute_command_execute_string(execute_command, global_temp_string);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmgui_execute_comfile.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* cmgui_execute_comfile */

/*
Main program
------------
*/
#if !defined (WIN32_USER_INTERFACE)
int main(int argc,char *argv[])
#else /* !defined (WIN32_USER_INTERFACE) */
int WINAPI WinMain(HINSTANCE current_instance,HINSTANCE previous_instance,
	LPSTR command_line,int initial_main_window_state)
	/* using WinMain as the entry point tells Windows that it is a gui and to use
		the graphics device interface functions for I/O */
	/*???DB.  WINDOWS a zero return code if WinMain does get into the message
		loop.  Other application interfaces may expect something else.  Should this
		failure code be #define'd ? */
	/*???DB. Win32 SDK says that don't have to call it WinMain */
#endif /* !defined (WIN32_USER_INTERFACE) */
/*******************************************************************************
LAST MODIFIED : 26 June 2002

DESCRIPTION :
Main program for the CMISS Graphical User Interface
==============================================================================*/
{
	char *cm_examples_directory,*cm_parameters_file_name,*comfile_name,
		*example_id,*examples_directory,*execute_string,*version_command_id,
		version_id_string[100],*version_ptr,version_temp[20];
	float default_light_direction[3]={0.0,-0.5,-1.0};
	int
#if defined (WIN32_USER_INTERFACE)
		WINAPI
#endif /* defined (WIN32_USER_INTERFACE) */
		return_code;
	int batch_mode, console_mode, command_list,no_display,non_random,start_cm,start_mycm;
#if defined (F90_INTERPRETER) || defined (PERL_INTERPRETER)
	int status;
#endif /* defined (F90_INTERPRETER) || defined (PERL_INTERPRETER) */
#if defined (MOTIF)
	char *arg;
	Display *display;
	int i;
#define XmNbackgroundColour "backgroundColour"
#define XmCBackgroundColour "BackgroundColour"
#define XmNforegroundColour "foregroundColour"
#define XmCForegroundColour "ForegroundColour"
#define XmNexamplesDirectory "examplesDirectory"
#define XmCExamplesDirectory "ExamplesDirectory"
#define XmNstartupComfile "startupComfile"
#define XmCStartupComfile "StartupComfile"
#define XmNhelpDirectory "helpDirectory"
#define XmCHelpDirectory "HelpDirectory"
#define XmNhelpUrl "helpUrl"
#define XmCHelpUrl "HelpUrl"
	static XtResource resources[]=
	{
		{
			XmNbackgroundColour,
			XmCBackgroundColour,
			XmRPixel,
			sizeof(Pixel),
			XtOffsetOf(User_settings,background_colour),
			XmRString,
			"black"
		},
		{
			XmNforegroundColour,
			XmCForegroundColour,
			XmRPixel,
			sizeof(Pixel),
			XtOffsetOf(User_settings,foreground_colour),
			XmRString,
			"white"
		},
		{
			XmNexamplesDirectory,
			XmCExamplesDirectory,
			XmRString,
			sizeof(char *),
			XtOffsetOf(User_settings,examples_directory),
			XmRString,
			""
		},
		{
			XmNstartupComfile,
			XmCStartupComfile,
			XmRString,
			sizeof(char *),
			XtOffsetOf(User_settings,startup_comfile),
			XmRImmediate,
			(XtPointer)0
		},
		{
			XmNhelpDirectory,
			XmCHelpDirectory,
			XmRString,
			sizeof(char *),
			XtOffsetOf(User_settings,help_directory),
			XmRString,
			""
		},
		{
			XmNhelpUrl,
			XmCHelpUrl,
			XmRString,
			sizeof(char *),
			XtOffsetOf(User_settings,help_url),
			XmRString,
			"http://www.esc.auckland.ac.nz/Groups/Bioengineering/CMISS/help/index_user.html"
		},
	};
#endif /* defined (MOTIF) */
#if defined (MOTIF)
/*???DB.  Need a setup routine for file I/O ? */
/*???DB.  Put in open_user_interface and always have ? */
	static MrmRegisterArg callbacks[]=
	{
		{"open_file_and_read",(XtPointer)open_file_and_read},
		{"open_file_and_write",(XtPointer)open_file_and_write}
	};
#endif /* defined (MOTIF) */
	struct Cmiss_command_data command_data;
	struct Colour ambient_colour,default_colour;
	struct Computed_field *computed_field;
	struct Computed_field_finite_element_package
		*computed_field_finite_element_package;
	struct Command_window *command_window;
	struct Coordinate_system rect_coord_system,temp_coordinate_system;
	struct Execute_command *execute_command, *set_command;
	struct FE_field *fe_field;
	struct Graphical_material *default_selected_material;
	struct GT_object *glyph, *mirror_glyph;
	struct MANAGER(Computed_field) *computed_field_manager;
	struct Modifier_entry set_file_name_option_table[]=
	{
		{CMGUI_EXAMPLE_DIRECTORY_SYMBOL,NULL,NULL,set_file_name},
		{NULL,NULL,NULL,set_file_name}
	};
	User_settings user_settings;

#if defined (MOTIF)
	XColor rgb;
#endif /* defined (MOTIF) */

#if defined (MOTIF)
	ENTER(main);
#endif /* defined (MOTIF) */
#if defined (WIN32_USER_INTERFACE)
	ENTER(WinMain);
#endif /* defined (WIN32_USER_INTERFACE) */
	return_code=1;

	/* initialize application specific global variables */
	execute_command = CREATE(Execute_command)(cmiss_execute_command,
		(void *)(&command_data));
	command_data.execute_command= execute_command;
	set_command = CREATE(Execute_command)(cmiss_set_command,
		(void *)(&command_data));
	command_data.set_command= set_command;
	command_data.user_interface= (struct User_interface *)NULL;
#if defined (MOTIF)
	command_data.control_curve_editor_dialog=(Widget)NULL;
	command_data.data_grabber_dialog=(Widget)NULL;
	command_data.sync_2d_3d_dialog=(Widget)NULL;
	command_data.emoter_slider_dialog=(Widget)NULL;
	command_data.grid_field_calculator_dialog=(Widget)NULL;
	command_data.input_module_dialog=(Widget)NULL;
	command_data.interactive_data_editor_dialog=(Widget)NULL;
	command_data.interactive_node_editor_dialog=(Widget)NULL;
	command_data.material_editor_dialog=(Widget)NULL;
	command_data.node_group_slider_dialog=(Widget)NULL;
	command_data.data_viewer=(struct Node_viewer *)NULL;
	command_data.node_viewer=(struct Node_viewer *)NULL;
	command_data.element_point_viewer=(struct Element_point_viewer *)NULL;
	command_data.prompt_window=(struct Prompt_window *)NULL;
	command_data.projection_window=(struct Projection_window *)NULL;
	command_data.scene_editor = (struct Scene_editor *)NULL;
	/*???RC.  Temporary - should allow more than one */
	command_data.spectrum_editor_dialog=(Widget)NULL;
	command_data.time_editor_dialog=(struct Time_editor_dialog *)NULL;
	/*???RC.  Temporary - should allow more than one */
#endif /* defined (MOTIF) */
	command_data.command_console = (struct Console *)NULL;
#if defined (UNEMAP)
	command_data.unemap_system_window=(struct System_window *)NULL;
#endif /* defined (UNEMAP) */
#if defined (CELL)
	command_data.cell_interface = (struct Cell_interface *)NULL;
#endif /* defined (CELL) */
#if defined (MIRAGE)
	command_data.tracking_editor_dialog=(struct Tracking_editor_dialog *)NULL;
#endif /* defined (MIRAGE) */
	command_data.example_directory=(char *)NULL;
	(set_file_name_option_table[0]).user_data=
		&(command_data.example_directory);
	command_data.set_file_name_option_table=set_file_name_option_table;

#if defined (MOTIF)
	command_data.comfile_window_manager=(struct MANAGER(Comfile_window) *)NULL;
#endif /* defined (MOTIF) */
	command_data.default_light=(struct Light *)NULL;
	command_data.light_manager=(struct MANAGER(Light) *)NULL;
	command_data.default_light_model=(struct Light_model *)NULL;
	command_data.light_model_manager=(struct MANAGER(Light_model) *)NULL;
	command_data.environment_map_manager=(struct MANAGER(Environment_map) *)NULL;
	command_data.texture_manager=(struct MANAGER(Texture) *)NULL;
	command_data.volume_texture_manager=(struct MANAGER(VT_volume_texture) *)NULL;
	command_data.default_graphical_material=(struct Graphical_material *)NULL;
	command_data.graphical_material_manager=(struct MANAGER(Graphical_material) *)NULL;
	command_data.default_spectrum=(struct Spectrum *)NULL;
	command_data.spectrum_manager=(struct MANAGER(Spectrum) *)NULL;
#if defined (MOTIF)
	command_data.graphics_window_manager=(struct MANAGER(Graphics_window) *)NULL;
#endif /* defined (MOTIF) */
	command_data.control_curve_manager=(struct MANAGER(Control_curve) *)NULL;
	command_data.basis_manager=(struct MANAGER(FE_basis) *)NULL;
	command_data.element_manager=(struct MANAGER(FE_element) *)NULL;
	command_data.element_group_manager=(struct MANAGER(GROUP(FE_element) *))NULL;
	command_data.node_manager=(struct MANAGER(FE_node) *)NULL;
	command_data.node_group_manager=(struct MANAGER(GROUP(FE_node) *))NULL;
	command_data.data_manager=(struct MANAGER(FE_node) *)NULL;
	command_data.data_group_manager=(struct MANAGER(GROUP(FE_node) *))NULL;
	command_data.streampoint_list=(struct Streampoint *)NULL;
#if defined (SELECT_DESCRIPTORS)
	command_data.device_list=(struct LIST(Io_device) *)NULL;
#endif /* defined (SELECT_DESCRIPTORS) */
	command_data.graphics_object_list=(struct LIST(GT_object) *)NULL;
	command_data.glyph_list=(struct LIST(GT_object) *)NULL;	
	command_data.any_object_selection=(struct Any_object_selection *)NULL;
	command_data.element_point_ranges_selection=(struct Element_point_ranges_selection *)NULL;
	command_data.element_selection=(struct FE_element_selection *)NULL;
	command_data.data_selection=(struct FE_node_selection *)NULL;
	command_data.node_selection=(struct FE_node_selection *)NULL;
	command_data.interactive_tool_manager=(struct MANAGER(Interactive_tool) *)NULL;
	command_data.computed_field_package=(struct Computed_field_package *)NULL;
	command_data.default_scene=(struct Scene *)NULL;
	command_data.scene_manager=(struct MANAGER(Scene) *)NULL;
	command_data.unemap_package=(struct Unemap_package *)NULL;
	command_data.command_window=(struct Command_window *)NULL;
	command_data.fe_time = (struct FE_time *)NULL;
#if defined (SGI_MOVIE_FILE)
	command_data.movie_graphics_manager=(struct MANAGER(Movie_graphics) *)NULL;
#endif /* defined (SGI_MOVIE_FILE) */
#if defined (MOTIF)
	command_data.transform_tool=(struct Interactive_tool *)NULL;
	command_data.node_tool=(struct Node_tool *)NULL;
	command_data.element_tool=(struct Element_tool *)NULL;
	command_data.data_tool=(struct Node_tool *)NULL;
	command_data.element_point_tool=(struct Element_point_tool *)NULL;
	command_data.select_tool=(struct Select_tool *)NULL;

	command_data.element_creator=(struct Element_creator *)NULL;
#endif /* defined (MOTIF) */
	command_data.examples_directory=(char *)NULL;
	command_data.example_comfile=(char *)NULL;
	command_data.cm_examples_directory=(char *)NULL;
	command_data.cm_parameters_file_name=(char *)NULL;
	command_data.default_time_keeper = (struct Time_keeper *)NULL;
#if defined (MOTIF)
	command_data.background_colour.red=(float)0;
	command_data.background_colour.green=(float)0;
	command_data.background_colour.blue=(float)0;
	command_data.foreground_colour.red=(float)1;
	command_data.foreground_colour.green=(float)1;
	command_data.foreground_colour.blue=(float)1;
#endif /* defined (MOTIF) */
	command_data.examples_directory=(char *)NULL;
	command_data.help_directory=(char *)NULL;
	command_data.help_url=(char *)NULL;

	/* display the version */
	display_message(INFORMATION_MESSAGE, VERSION "\n");

	/* check for command_list (needed because don't CREATE(User_interface) for
		command_list) */
	command_list=0;
	batch_mode = 0;
	console_mode = 0;
	no_display = 0;
#if defined (MOTIF)
	i=1;
	while ((i<argc)&&strcmp("-command_list",argv[i]))
	{
		i++;
	}
	if (i<argc)
	{
		command_list=1;
	}
	i=1;
	while ((i<argc)&&strncmp("-no_display",argv[i],4))
	{
		i++;
	}
	if (i<argc)
	{
		no_display = 1;
	}
#endif /* defined (MOTIF) */

	/* create the managers */

#if defined (MOTIF)
	/* comfile window manager */
	command_data.comfile_window_manager = CREATE(MANAGER(Comfile_window))();
#endif /* defined (MOTIF) */

	/* light manager */
	if (command_data.light_manager=CREATE(MANAGER(Light))())
	{
		if (command_data.default_light=CREATE(Light)("default"))
		{
			set_Light_type(command_data.default_light,INFINITE_LIGHT);
			default_colour.red=1.0;
			default_colour.green=1.0;
			default_colour.blue=1.0;		
			set_Light_colour(command_data.default_light,&default_colour); 
				
			set_Light_direction(command_data.default_light,default_light_direction);
			/*???DB.  Include default as part of manager ? */
			ACCESS(Light)(command_data.default_light);
			if (!ADD_OBJECT_TO_MANAGER(Light)(command_data.default_light,
				command_data.light_manager))
			{
				DEACCESS(Light)(&(command_data.default_light));
			}
		}
	}
	if (command_data.light_model_manager=CREATE(MANAGER(Light_model))())
	{
		if (command_data.default_light_model=CREATE(Light_model)("default"))
		{
			ambient_colour.red=0.2;
			ambient_colour.green=0.2;
			ambient_colour.blue=0.2;
			Light_model_set_ambient(command_data.default_light_model,&ambient_colour);
			Light_model_set_side_mode(command_data.default_light_model,
				LIGHT_MODEL_TWO_SIDED);
			/*???DB.  Include default as part of manager ? */			
			ACCESS(Light_model)(command_data.default_light_model);
			if (!ADD_OBJECT_TO_MANAGER(Light_model)(
				command_data.default_light_model,command_data.light_model_manager))
			{
				DEACCESS(Light_model)(&(command_data.default_light_model));
			}			
		}
	}
	/* environment map manager */
	command_data.environment_map_manager=CREATE(MANAGER(Environment_map))();
	/* texture manager */
	command_data.texture_manager=CREATE(MANAGER(Texture))();
	/* volume texture manager */
	command_data.volume_texture_manager=CREATE(MANAGER(VT_volume_texture))();
	/* graphical material manager */
	if (command_data.graphical_material_manager=
		CREATE(MANAGER(Graphical_material))())
	{
		struct Colour colour;

		if (command_data.default_graphical_material=
			CREATE(Graphical_material)("default"))
		{
			/* ACCESS so can never be destroyed */
			ACCESS(Graphical_material)(command_data.default_graphical_material);
			if (!ADD_OBJECT_TO_MANAGER(Graphical_material)(
				command_data.default_graphical_material,
				command_data.graphical_material_manager))
			{
				DEACCESS(Graphical_material)(
					&(command_data.default_graphical_material));
			}
		}
		/* create material "default_selected" to be bright red for highlighting
			 selected graphics */
		if (default_selected_material=CREATE(Graphical_material)(
			"default_selected"))
		{
			colour.red=1.0;
			colour.green=0.0;
			colour.blue=0.0;
			Graphical_material_set_ambient(default_selected_material,&colour);
			Graphical_material_set_diffuse(default_selected_material,&colour);
			/* ACCESS so can never be destroyed */
			ACCESS(Graphical_material)(default_selected_material);
			if (!ADD_OBJECT_TO_MANAGER(Graphical_material)(default_selected_material,
				command_data.graphical_material_manager))
			{
				DEACCESS(Graphical_material)(&default_selected_material);
			}
		}	
	}
	/* spectrum manager */
	if (command_data.spectrum_manager=CREATE(MANAGER(Spectrum))())
	{
		if (command_data.default_spectrum=CREATE(Spectrum)("default"))
		{
			Spectrum_set_simple_type(command_data.default_spectrum,
				BLUE_TO_RED_SPECTRUM);
			Spectrum_set_minimum_and_maximum(command_data.default_spectrum,0,1);
			/* ACCESS so can never be destroyed */
			ACCESS(Spectrum)(command_data.default_spectrum);
			if (!ADD_OBJECT_TO_MANAGER(Spectrum)(command_data.default_spectrum,
				command_data.spectrum_manager))
			{
				DEACCESS(Spectrum)(&(command_data.default_spectrum));
			}
		}
	}
#if defined (MOTIF)
	/* graphics window manager.  Note there is no default window. */
	command_data.graphics_window_manager=CREATE(MANAGER(Graphics_window))();
#endif /* defined (MOTIF) */
	/* FE_element_field_info manager */
		/*???DB.  To be done */
	all_FE_element_field_info=CREATE(LIST(FE_element_field_info))();
	/* FE_element_shape manager */
		/*???DB.  To be done */
	all_FE_element_shape=CREATE(LIST(FE_element_shape))();
	/* FE_field manager */
	command_data.fe_field_manager=CREATE(MANAGER(FE_field))();
	/* FE_element manager */
	rect_coord_system.type = RECTANGULAR_CARTESIAN;

	command_data.fe_time = ACCESS(FE_time)(CREATE(FE_time)());

	command_data.control_curve_manager=CREATE(MANAGER(Control_curve))();

	command_data.basis_manager=CREATE(MANAGER(FE_basis))();
	command_data.element_manager=CREATE(MANAGER(FE_element))();
		/*???DB.  Also manages faces and lines */
	command_data.element_group_manager=CREATE(MANAGER(GROUP(FE_element)))();
	command_data.node_manager=CREATE(MANAGER(FE_node))();
	command_data.node_group_manager=CREATE(MANAGER(GROUP(FE_node)))();
	command_data.data_manager=CREATE(MANAGER(FE_node))();
	command_data.data_group_manager=CREATE(MANAGER(GROUP(FE_node)))();

	/* create graphics object list */
		/*???RC.  Eventually want graphics object manager */
	command_data.graphics_object_list=CREATE(LIST(GT_object))();

#if defined (SELECT_DESCRIPTORS)
	/* create device list */
		/*SAB.  Eventually want device manager */
	command_data.device_list=CREATE(LIST(Io_device))();
#endif /* defined (SELECT_DESCRIPTORS) */

	/* create glyph list */
		/*???RC.  Eventually want glyph manager */
	if (command_data.glyph_list=CREATE(LIST(GT_object))())
	{
		/* add standard glyphs */
		if (glyph=make_glyph_arrow_line("arrow_line",0.25,0.125))
		{
			ADD_OBJECT_TO_LIST(GT_object)(glyph,command_data.glyph_list);
		}
		mirror_glyph = glyph;
		if (glyph=make_glyph_mirror("mirror_arrow_line",mirror_glyph))
		{
			ADD_OBJECT_TO_LIST(GT_object)(glyph,command_data.glyph_list);
		}
		if (glyph=make_glyph_arrow_solid("arrow_solid",12,2./3.,1./6.))
		{
			ADD_OBJECT_TO_LIST(GT_object)(glyph,command_data.glyph_list);
		}
		mirror_glyph = glyph;
		if (glyph=make_glyph_mirror("mirror_arrow_solid",mirror_glyph))
		{
			ADD_OBJECT_TO_LIST(GT_object)(glyph,command_data.glyph_list);
		}
		if (glyph=make_glyph_axes("axes",0.1,0.025,0.1))
		{
			ADD_OBJECT_TO_LIST(GT_object)(glyph,command_data.glyph_list);
		}
		if (glyph=make_glyph_cone("cone",12))
		{
			ADD_OBJECT_TO_LIST(GT_object)(glyph,command_data.glyph_list);
		}
		mirror_glyph = glyph;
		if (glyph=make_glyph_mirror("mirror_cone",mirror_glyph))
		{
			ADD_OBJECT_TO_LIST(GT_object)(glyph,command_data.glyph_list);
		}
		if (glyph=make_glyph_cross("cross"))
		{
			ADD_OBJECT_TO_LIST(GT_object)(glyph,command_data.glyph_list);
		}
		if (glyph=make_glyph_cube_solid("cube_solid"))
		{
			ADD_OBJECT_TO_LIST(GT_object)(glyph,command_data.glyph_list);
		}
		if (glyph=make_glyph_cube_wireframe("cube_wireframe"))
		{
			ADD_OBJECT_TO_LIST(GT_object)(glyph,command_data.glyph_list);
		}
		if (glyph=make_glyph_cylinder("cylinder6",6))
		{
			ADD_OBJECT_TO_LIST(GT_object)(glyph,command_data.glyph_list);
		}
		if (glyph=make_glyph_cylinder("cylinder",12))
		{
			ADD_OBJECT_TO_LIST(GT_object)(glyph,command_data.glyph_list);
		}
		if (glyph=make_glyph_cylinder("cylinder_hires",48))
		{
			ADD_OBJECT_TO_LIST(GT_object)(glyph,command_data.glyph_list);
		}
		if (glyph=make_glyph_sphere("diamond",4,2))
		{
			ADD_OBJECT_TO_LIST(GT_object)(glyph,command_data.glyph_list);
		}
		if (glyph=make_glyph_line("line"))
		{
			ADD_OBJECT_TO_LIST(GT_object)(glyph,command_data.glyph_list);
		}
		mirror_glyph = glyph;
		if (glyph=make_glyph_mirror("mirror_line",mirror_glyph))
		{
			ADD_OBJECT_TO_LIST(GT_object)(glyph,command_data.glyph_list);
		}
		if (glyph=make_glyph_point("point",g_POINT_MARKER,0))
		{
			ADD_OBJECT_TO_LIST(GT_object)(glyph,command_data.glyph_list);
		}
		if (glyph=make_glyph_sheet("sheet"))
		{
			ADD_OBJECT_TO_LIST(GT_object)(glyph,command_data.glyph_list);
		}
		if (glyph=make_glyph_sphere("sphere",12,6))
		{
			ADD_OBJECT_TO_LIST(GT_object)(glyph,command_data.glyph_list);
		}
		if (glyph=make_glyph_sphere("sphere_hires",48,24))
		{
			ADD_OBJECT_TO_LIST(GT_object)(glyph,command_data.glyph_list);
		}
	}

	/* global list of selected objects */
	command_data.any_object_selection=CREATE(Any_object_selection)();
	command_data.element_point_ranges_selection=
		CREATE(Element_point_ranges_selection)();
	command_data.element_selection=CREATE(FE_element_selection)();
	command_data.data_selection=CREATE(FE_node_selection)();
	command_data.node_selection=CREATE(FE_node_selection)();

#if defined (MOTIF)
	/* interactive_tool manager */
	command_data.interactive_tool_manager=CREATE(MANAGER(Interactive_tool))();
#endif /* defined (MOTIF) */

	/* computed field manager and default computed fields zero, xi,
		 default_coordinate, etc. */
	/*???RC should the default computed fields be established in
		CREATE(Computed_field_package)? */
	command_data.computed_field_package=CREATE(Computed_field_package)(
		command_data.fe_field_manager, command_data.element_manager);
	computed_field_manager=Computed_field_package_get_computed_field_manager(
		command_data.computed_field_package);

	/* Add Computed_fields to the Computed_field_package */
	computed_field_finite_element_package = 
		(struct Computed_field_finite_element_package *)NULL;
	if (command_data.computed_field_package)
	{
		Computed_field_register_types_coordinate(
			command_data.computed_field_package);
		Computed_field_register_types_component_operations(
			command_data.computed_field_package);
		if (command_data.element_group_manager)
		{
			Computed_field_register_types_compose(
				command_data.computed_field_package, 
				command_data.element_group_manager);
		}
		Computed_field_register_types_composite(
			command_data.computed_field_package);		
		if (command_data.control_curve_manager)
		{
			Computed_field_register_types_control_curve(
				command_data.computed_field_package, 
				command_data.control_curve_manager);
		}
		Computed_field_register_types_derivatives(
			command_data.computed_field_package);
		Computed_field_register_types_external(
			command_data.computed_field_package);
		Computed_field_register_types_fibres(
			command_data.computed_field_package);
		Computed_field_register_types_matrix_operations(
			command_data.computed_field_package);
		Computed_field_register_types_vector_operations(
			command_data.computed_field_package);
#if defined (MOTIF)
		if (command_data.graphics_window_manager)
		{
			Computed_field_register_type_window_projection(
				command_data.computed_field_package, 
				command_data.graphics_window_manager);
		}
#endif /* defined (MOTIF) */
		if (command_data.texture_manager)
		{
			Computed_field_register_type_sample_texture(
				command_data.computed_field_package, 
				command_data.texture_manager);
		}
		if (command_data.element_manager)
		{
			Computed_field_register_type_integration(
				command_data.computed_field_package, 
				command_data.element_manager);
		}
		if (command_data.fe_field_manager && command_data.fe_time)
		{
			computed_field_finite_element_package =
				Computed_field_register_types_finite_element(
					command_data.computed_field_package,
					command_data.fe_field_manager, command_data.fe_time);
			if (!((computed_field=CREATE(Computed_field)("cmiss_number"))&&
				Computed_field_set_coordinate_system(computed_field,
					&rect_coord_system)&&
				Computed_field_set_type_cmiss_number(computed_field)&&
				Computed_field_set_read_only(computed_field)&&
				ADD_OBJECT_TO_MANAGER(Computed_field)(computed_field,
					computed_field_manager)))
			{
				DESTROY(Computed_field)(&computed_field);
			}
			/* create the grid_point_number field and add to FE_field_manager - 
				 wrapper Computed_field will automatically be made for it. Has special
				 use for setting grid_points in Element_point_viewer */
			temp_coordinate_system.type = NOT_APPLICABLE;
			if (!((fe_field=CREATE(FE_field)(command_data.fe_time))&&
				set_FE_field_name(fe_field,"grid_point_number")&&
				set_FE_field_value_type(fe_field,INT_VALUE)&&
				set_FE_field_number_of_components(fe_field,1)&&
				set_FE_field_type_general(fe_field)&&
				set_FE_field_CM_field_type(fe_field,CM_GENERAL_FIELD)&&
				set_FE_field_coordinate_system(fe_field,&temp_coordinate_system)&&
				set_FE_field_component_name(fe_field,0,"grid_point_number")&&
				ADD_OBJECT_TO_MANAGER(FE_field)(fe_field,
					command_data.fe_field_manager)))
			{
				DESTROY(FE_field)(&fe_field);
			}

			if (!((computed_field=CREATE(Computed_field)("xi"))&&
				Computed_field_set_coordinate_system(computed_field,
					&rect_coord_system)&&
				Computed_field_set_type_xi_coordinates(computed_field)&&
				Computed_field_set_read_only(computed_field)&&
				ADD_OBJECT_TO_MANAGER(Computed_field)(computed_field,
					computed_field_manager)))
			{
				DESTROY(Computed_field)(&computed_field);
			}
		}
		Computed_field_register_types_deformation(
			command_data.computed_field_package);
	}

	/* scene manager */
		/*???RC.   LOTS of managers need to be created before this */
	if (command_data.scene_manager=CREATE(MANAGER(Scene))())
	{
		if (command_data.default_scene=CREATE(Scene)("default"))
		{
			Scene_enable_graphics(command_data.default_scene,command_data.glyph_list,
				command_data.graphical_material_manager,
				command_data.default_graphical_material,command_data.light_manager,
				command_data.spectrum_manager,command_data.default_spectrum,
				command_data.texture_manager);
			Scene_set_graphical_element_mode(command_data.default_scene,
				GRAPHICAL_ELEMENT_LINES,
				Computed_field_package_get_computed_field_manager(
				command_data.computed_field_package),command_data.element_manager,
				command_data.element_group_manager,command_data.fe_field_manager,
				command_data.node_manager,command_data.node_group_manager,
				command_data.data_manager,command_data.data_group_manager,
				command_data.element_point_ranges_selection,
				command_data.element_selection,command_data.node_selection,
				command_data.data_selection,command_data.user_interface);
			/*???RC.  May want to use functions to modify default_scene here */
			/* eg. to add model lights, etc. */
			/* ACCESS default so can never be destroyed */
			/*???RC.  Should be able to change: eg. gfx set default scene NAME */
			/*???DB.  Include default as part of manager ? */
			ACCESS(Scene)(command_data.default_scene);
			if (!ADD_OBJECT_TO_MANAGER(Scene)(command_data.default_scene,
				command_data.scene_manager))
			{
				DEACCESS(Scene)(&(command_data.default_scene));
			}
		}
	}
#if defined (MIRAGE)
/*	command_data.digitiser_window_manager=CREATE(MANAGER(Digitiser_window))();*/
#endif /* defined (MIRAGE) */
	/* now set up the conversion routines */
		/*???DB.  Can this be put elsewhere ? */
	conversion_init();
#if defined (MOTIF)
	/* initialize the coordinate widget manager */
		/*???DB.  Still needs to be turned into a manager */
	coord_widget_init();
#endif /* defined (MOTIF) */
#if defined (SGI_MOVIE_FILE)
	command_data.movie_graphics_manager=CREATE(MANAGER(Movie_graphics))();
#endif /* defined (SGI_MOVIE_FILE) */
	/* parse the command line */
		/*???DB.  Is this the right place for this ? */
		/*???DB.  Can this be done with command_parser ? */
		/*???GMH.  We need to change to REAL argument parsing */
	/* flag to say randomise */
	non_random= -1;
	/* flag for starting cm */
	start_cm=0;
	/* flag for starting mycm */
	start_mycm=0;
	/* to over-ride all other example directory settings */
	examples_directory=(char *)NULL;
	/* back-end examples directory */
	cm_examples_directory=(char *)NULL;
	/* back-end parameters file */
	cm_parameters_file_name=(char *)NULL;
	/* the comfile is in the examples directory */
	example_id=(char *)NULL;
	/* a string executed by the interpreter before loading any comfiles */
	execute_string=(char *)NULL;
	/* set no command id supplied */
	version_command_id=(char *)NULL;

	if (!command_list)
	{
		if (command_data.event_dispatcher = CREATE(Event_dispatcher)())
		{
			if (no_display)
			{
				return_code = 1;
			}
			else
			{
#if defined (UNIX) /* switch (Operating_System) */
				if (command_data.user_interface = CREATE(User_interface)
					(&argc, argv, command_data.event_dispatcher, "Cmgui",
					"cmgui"))
				{
					return_code = 1;
				}
				else
				{
					display_message(ERROR_MESSAGE,"Could not create User interface");
					return_code=0;
				}
#elif defined (WIN32_USER_INTERFACE) /* switch (Operating_System) */
				if (command_data.user_interface = CREATE(User_interface)
					(current_instance, previous_instance, command_line,
					 initial_main_window_state, command_data.event_dispatcher))
				{
					return_code = 1;
				}
				else
				{
					display_message(ERROR_MESSAGE,"Could not create User interface");
					return_code=0;
				}
#endif  /* switch (Operating_System) */
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Could not create Event dispatcher.");
			return_code=0;
		}
	}
	if (return_code)
	{
		/* the name of the comfile to be run on startup */
		comfile_name=(char *)NULL;
		if (command_list)
		{
#if defined (MOTIF)
			command_data.examples_directory=(char *)NULL;
			command_data.cm_examples_directory=(char *)NULL;
			command_data.cm_parameters_file_name=(char *)NULL;
#endif /* defined (MOTIF) */
#if defined (MOTIF)
#if defined (F90_INTERPRETER) || defined (PERL_INTERPRETER)
			create_interpreter(argc, argv, comfile_name, &status);
#endif /* defined (F90_INTERPRETER) || defined (PERL_INTERPRETER) */
			cmiss_execute_command("??",(void *)(&command_data));
#if defined (F90_INTERPRETER) || defined (PERL_INTERPRETER)
			destroy_interpreter(&status);
#endif /* defined (F90_INTERPRETER) || defined (PERL_INTERPRETER) */
#endif /* defined (MOTIF) */
		}
		else
		{
#if defined (MOTIF)
			i=1;
			while (return_code&&(i<argc))
			{
				arg=argv[i];
				if ('-'==arg[0])
				{
					switch (arg[1])
					{
						case 'c':
						{
							switch (arg[2])
							{
								case 'o':
								{
									switch (arg[3])
									{
										case 'm':
										{
											/* command_list has already been set */
										} break;
										case 'n':
										{
											console_mode = 1;
										} break;
										default:
										{
											return_code=0;
										} break;
									}
								} break;
								case 'm':
								{
									switch (arg[3])
									{
										case '\0':
										{
											start_cm=1;
										} break;
										case '_':
										{
											switch (arg[4])
											{
												case 'e':
												{
													/* back-end examples directory path */
													i++;
													if (i<argc)
													{
														if (cm_examples_directory)
														{
															DEALLOCATE(cm_examples_directory);
														}
														if (ALLOCATE(cm_examples_directory,char,
															strlen(argv[i])+1))
														{
															strcpy(cm_examples_directory,argv[i]);
														}
														else
														{
															display_message(ERROR_MESSAGE,
																"main.  Insufficient memory for back-end examples directory path");
															return_code=0;
														}
													}
													else
													{
														return_code=0;
													}
												} break;
												case 'p':
												{
													/* cm parameters file name */
													i++;
													if (i<argc)
													{
														if (cm_parameters_file_name)
														{
															DEALLOCATE(cm_parameters_file_name);
														}
														if (ALLOCATE(cm_parameters_file_name,char,
															strlen(argv[i])+1))
														{
															strcpy(cm_parameters_file_name,argv[i]);
														}
														else
														{
															display_message(ERROR_MESSAGE,
																"main.  Insufficient memory for parameters file name");
															return_code=0;
														}
													}
													else
													{
														return_code=0;
													}
												} break;
												default:
												{
													return_code=0;
												} break;
											}
										} break;
										default:
										{
											return_code=0;
										} break;
									}
								} break;
								default:
								{
									return_code=0;
								} break;
							}
						} break;
						case 'd':
						{
							/* skip the display name */
							i++;
						} break;
						case 'v':
						{
							/* read in the specified visual */
							i++;
							User_interface_set_specified_visual_id(command_data.user_interface,
								atoi(argv[i]));
						} break;
						case 'e':
						{
							switch (arg[2])
							{
								case 'p':
								{
									/* examples directory path */
									i++;
									if (i<argc)
									{
										if (examples_directory)
										{
											DEALLOCATE(examples_directory);
										}
										if (ALLOCATE(examples_directory,char,strlen(argv[i])+1))
										{
											strcpy(examples_directory,argv[i]);
										}
										else
										{
											display_message(ERROR_MESSAGE,
												"main.  Insufficient memory for examples directory path");
											return_code=0;
										}
									}
									else
									{
										return_code=0;
									}
								} break;
								case 'x':
								{
									switch (arg[3])
									{
										case 'a':
										case '\0':
										{
											/* example id */
											i++;
											if (i<argc)
											{
												if (example_id)
												{
													DEALLOCATE(example_id);
												}
												if (ALLOCATE(example_id,char,strlen(argv[i])+1))
												{
													strcpy(example_id,argv[i]);
												}
												else
												{
													display_message(ERROR_MESSAGE,
														"main.  Insufficient memory for example_id");
													return_code=0;
												}
											}
											else
											{
												return_code=0;
											}
										} break;
										case 'e':
										{
											/* execute */
											i++;
											if (i<argc)
											{
												if (execute_string)
												{
													DEALLOCATE(execute_string);
												}
												if (ALLOCATE(execute_string,char,strlen(argv[i])+1))
												{
													strcpy(execute_string,argv[i]);
												}
												else
												{
													display_message(ERROR_MESSAGE,
														"main.  Insufficient memory for example_id");
													return_code=0;
												}
											}
											else
											{
												return_code=0;
											}
										} break;
									}
								} break;
							}
						} break;
						case 'm':
						{
							start_mycm=1;
						} break;
						case 'r':
						{
							/* set a flag to say don't randomise */
							i++;
							if (i<argc)
							{
								non_random=atoi(argv[i]);
							}
							else
							{
								return_code=0;
							}
						} break;
						case 'i':
						{
							i++;
							if (i<argc)
							{
								version_command_id=argv[i];
							}
							else
							{
								return_code=0;
							}
						} break;
						case 'b':
						{
							batch_mode=1;
						} break;
						case 'n':
						{
							/* no_display has already been set */
						} break;
						default:
						{
							return_code=0;
						} break;
					}
				}
				else
				{
					if (comfile_name)
					{
						DEALLOCATE(comfile_name);
					}
					if (ALLOCATE(comfile_name,char,strlen(argv[i])+1))
					{
						strcpy(comfile_name,argv[i]);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"main.  Insufficient memory for comfile name");
						return_code=0;
					}
				}
				i++;
			}
#endif /* defined (MOTIF) */

#if defined (F90_INTERPRETER) || defined (PERL_INTERPRETER)
			create_interpreter(argc, argv, comfile_name, &status);
#endif /* defined (F90_INTERPRETER) || defined (PERL_INTERPRETER) */

			/* set up image library */
#if defined (UNIX) /* switch (Operating_System) */
			Open_image_environment(*argv);
#elif defined (WIN32_USER_INTERFACE) /* switch (Operating_System) */
			/* SAB Passing a string to this function so that it
				starts up, should get the correct thing from
				the windows system */
			Open_image_environment("cmgui");
#endif /* switch (Operating_System) */

			command_data.default_time_keeper=ACCESS(Time_keeper)(
				CREATE(Time_keeper)("default", command_data.event_dispatcher,
				command_data.user_interface));
			if(command_data.default_scene)
			{
				Scene_enable_time_behaviour(command_data.default_scene,
					command_data.default_time_keeper);
			}
#if defined (MOTIF)
			if (command_data.user_interface)
			{
				command_data.transform_tool=create_Interactive_tool_transform(
					command_data.user_interface);
				ADD_OBJECT_TO_MANAGER(Interactive_tool)(command_data.transform_tool,
					command_data.interactive_tool_manager);
				command_data.node_tool=CREATE(Node_tool)(
					command_data.interactive_tool_manager,
					command_data.node_manager,/*use_data*/0,
					command_data.node_group_manager,
					command_data.element_manager,
					command_data.node_selection,
					command_data.computed_field_package,
					command_data.default_graphical_material,
					command_data.user_interface,
					command_data.default_time_keeper);
				command_data.element_tool=CREATE(Element_tool)(
					command_data.interactive_tool_manager,
					command_data.element_manager,
					command_data.element_group_manager,
					command_data.element_selection,
					command_data.element_point_ranges_selection,
					command_data.default_graphical_material,
					command_data.user_interface);
				command_data.data_tool=CREATE(Node_tool)(
					command_data.interactive_tool_manager,
					command_data.data_manager,/*use_data*/1,
					command_data.data_group_manager,
					(struct MANAGER(FE_element) *)NULL,
					command_data.data_selection,
					command_data.computed_field_package,
					command_data.default_graphical_material,
					command_data.user_interface,
					command_data.default_time_keeper);
				command_data.element_point_tool=CREATE(Element_point_tool)(
					command_data.interactive_tool_manager,
					command_data.element_point_ranges_selection,
					command_data.default_graphical_material);
				command_data.select_tool=CREATE(Select_tool)(
					command_data.interactive_tool_manager,
					command_data.any_object_selection,
					command_data.default_graphical_material);
			}
#endif /* defined (MOTIF) */

			if (return_code)
			{
				/* initialize random number generator */
				if (-1==non_random)
				{
					/* randomise */
					srand(time(NULL));
					/*???DB.  time is not ANSI */
				}
				else
				{
					/* randomise using given seed */
					srand(non_random);
				}
				user_settings.startup_comfile = (char *)NULL;
				if (no_display)
				{
#if defined (MOTIF)
					command_data.background_colour.red=0.0;
					command_data.background_colour.green=0.0;
					command_data.background_colour.blue=0.0;
					command_data.foreground_colour.red=1.0;
					command_data.foreground_colour.green=1.0;
					command_data.foreground_colour.blue=1.0;
#endif /* defined (MOTIF) */
				}
				else
				{
					/* retrieve application specific constants */
#if defined (MOTIF)
					display = User_interface_get_display(command_data.user_interface);
					XtVaGetApplicationResources(User_interface_get_application_shell(
						command_data.user_interface),&user_settings,resources,XtNumber(resources),NULL);
					/*???DB.  User settings should be divided among tools */
					/* retrieve the background rgb settings */
					rgb.pixel=user_settings.background_colour;
					XQueryColor(display,DefaultColormap(display,DefaultScreen(display)),&rgb);
					/*???DB.  Get rid of 65535 ? */
					command_data.background_colour.red=(float)(rgb.red)/(float)65535;
					command_data.background_colour.green=(float)(rgb.green)/(float)65535;
					command_data.background_colour.blue=(float)(rgb.blue)/(float)65535;
					/* retrieve the foreground rgb settings */
					rgb.pixel=user_settings.foreground_colour;
					XQueryColor(display,DefaultColormap(display,DefaultScreen(display)),&rgb);
					/*???DB.  Get rid of 65535 ? */
					command_data.foreground_colour.red=(float)(rgb.red)/(float)65535;
					command_data.foreground_colour.green=(float)(rgb.green)/(float)65535;
					command_data.foreground_colour.blue=(float)(rgb.blue)/(float)65535;
					if ((command_data.foreground_colour.red==
						command_data.background_colour.red)&&
						(command_data.foreground_colour.green==
							command_data.background_colour.green)&&
						(command_data.foreground_colour.blue==
							command_data.background_colour.blue))
					{
						command_data.foreground_colour.red=
							1-command_data.background_colour.red;
						command_data.foreground_colour.green=
							1-command_data.background_colour.green;
						command_data.foreground_colour.blue=
							1-command_data.background_colour.blue;
					}
				}
				/*???DB.  Temporary */
				sprintf(global_temp_string,
					"gfx modify material default alpha 1 ambient %g %g %g diffuse %g %g %g",
					command_data.foreground_colour.red,
					command_data.foreground_colour.green,
					command_data.foreground_colour.blue,
					command_data.foreground_colour.red,
					command_data.foreground_colour.green,
					command_data.foreground_colour.blue);
				cmiss_execute_command(global_temp_string,(void *)(&command_data));
				if (no_display || examples_directory)
				{
					command_data.examples_directory=examples_directory;
				}
				else
				{
					command_data.examples_directory=user_settings.examples_directory;
				}
				command_data.cm_examples_directory=cm_examples_directory;
				command_data.cm_parameters_file_name=cm_parameters_file_name;
				if(no_display)
				{
					command_data.help_directory=(char *)NULL;
					command_data.help_url=(char *)NULL;
				}
				else
				{
					command_data.help_directory=user_settings.help_directory;
					command_data.help_url=user_settings.help_url;
				}
#endif /* defined (MOTIF) */
#if defined (MOTIF)
				if (no_display)
				{
					return_code = 1;
				}
				else
				{
					/* register the callbacks in the global name table */
					if (MrmSUCCESS==MrmRegisterNames(callbacks,XtNumber(callbacks)))
					{
#endif /* defined (MOTIF) */
						/* create the main window */
						/* construct the version ID string which is exported in the command
							windows version atom */
						strcpy(version_id_string,"cmiss*");
						/* version number */
						if (version_ptr=strstr(VERSION,"version "))
						{
							strncpy(version_temp,version_ptr+8,11);
							version_temp[11] = 0;
							strcat(version_id_string,version_temp);
						}
						strcat(version_id_string,"*");
						/* id from runtime */
						if (version_command_id)
						{
							strcat(version_id_string,version_command_id);
						}
						strcat(version_id_string,"*");
						/* link and runtime options */
						if ( start_mycm )
						{
							strcat(version_id_string, "mycm ");
						}
						else 
						{
							if (start_cm)
							{
								strcat(version_id_string, "cm ");
							}
						}
#if defined (UNEMAP)
						strcat(version_id_string,"unemap ");
#endif /* defined (UNEMAP) */
#if defined (MIRAGE)
						strcat(version_id_string,"mirage ");
#endif /* defined (MIRAGE) */
#if defined (INCLUDE_XVG)
						strcat(version_id_string,"xvg ");
#endif /* defined (INCLUDE_XVG) */

#if defined (MOTIF) || defined (WIN32_USER_INTERFACE)
						if (console_mode)
						{
#endif /* defined (MOTIF) || defined (WIN32_USER_INTERFACE) */
							if (!(command_data.command_console = CREATE(Console)(command_data.execute_command,
								command_data.event_dispatcher, fileno(stdin))))
							{
								display_message(ERROR_MESSAGE,"main.  "
									"Unable to create console.");
							}
#if defined (MOTIF) || defined (WIN32_USER_INTERFACE)
						}
						else
						{
							if (command_window=CREATE(Command_window)(execute_command,
									 command_data.user_interface,version_id_string))
							{
								command_data.command_window=command_window;
								if (!batch_mode)
								{
									/* set up messages */
									set_display_message_function(ERROR_MESSAGE,
										display_error_message,command_window);
									set_display_message_function(INFORMATION_MESSAGE,
										display_information_message,command_window);
									set_display_message_function(WARNING_MESSAGE,
										display_warning_message,command_window);
#if defined (PERL_INTERPRETER)
									redirect_interpreter_output(&return_code);
#endif /* defined (PERL_INTERPRETER) */
								}

#if defined (MOTIF)
								XSetErrorHandler(x_error_handler);
#endif /* defined (MOTIF) */
								return_code = 1;
							}
							else
							{
								display_message(ERROR_MESSAGE,"Unable to create command window");
								return_code=0;
							}
						}
#endif /* defined (MOTIF) || defined (WIN32_USER_INTERFACE) */
#if defined (MOTIF)
					}
					else
					{
						display_message(ERROR_MESSAGE,"Unable to register callbacks");
						return_code=0;
					}
#endif /* defined (MOTIF) */
				}
				if(return_code)
				{
#if defined (MOTIF)
					if (start_cm||start_mycm)
					{
						sprintf(global_temp_string,"create cm");
						if (start_mycm)
						{
							strcat(global_temp_string," mycm");
						}
						if (cm_parameters_file_name)
						{
							strcat(global_temp_string," parameters ");
							strcat(global_temp_string,cm_parameters_file_name);
						}
						if (cm_examples_directory)
						{
							strcat(global_temp_string," examples_directory ");
							strcat(global_temp_string,cm_examples_directory);
						}
						/* start the back-end */
						cmiss_execute_command(global_temp_string,
							(void *)(&command_data));
					}
					if (execute_string)
					{
						cmiss_execute_command(execute_string,(void *)(&command_data));
					}
					if (user_settings.startup_comfile)
					{
						/* Can't get the startupComfile name without X at the moment */
						cmgui_execute_comfile(user_settings.startup_comfile, NULL,
							NULL, NULL, (char **)NULL, execute_command);
					}
					if (example_id||comfile_name)
					{
						/* open the command line comfile */
						cmgui_execute_comfile(comfile_name,example_id,
							command_data.examples_directory,
							CMGUI_EXAMPLE_DIRECTORY_SYMBOL, &command_data.example_comfile,
							execute_command);
						DEALLOCATE(comfile_name);
						DEALLOCATE(example_id);
					}
#endif /* defined (MOTIF) */
					/*							START_ERROR_HANDLING;*/
					switch (signal_code)
					{
#if defined (MOTIF)
						/*???DB.  SIGBUS is not POSIX */
						case SIGBUS:
						{
							printf("Bus error occurred\n");
							display_message(ERROR_MESSAGE,"Bus error occurred");
						} break;
#endif /* defined (MOTIF) */
						case SIGFPE:
						{
							printf("Floating point exception occurred\n");
							display_message(ERROR_MESSAGE,
								"Floating point exception occurred");
						} break;
						case SIGILL:
						{
							printf("Illegal instruction occurred\n");
							display_message(ERROR_MESSAGE,
								"Illegal instruction occurred");
						} break;
						case SIGSEGV:
						{
							printf("Invalid memory reference occurred\n");
							display_message(ERROR_MESSAGE,
								"Invalid memory reference occurred");
						} break;
					}
					if(!batch_mode)
					{
						/* user interface loop */						
						return_code=Event_dispatcher_main_loop(command_data.event_dispatcher);
						/*???RC.  Need clean up routines, eg.
						  X3dThreeDDrawingCleanUp(display); */
					}
				}

#if defined (SGI_MOVIE_FILE)
				DESTROY(MANAGER(Movie_graphics))(&command_data.movie_graphics_manager);
#endif /* defined (SGI_MOVIE_FILE) */

#if defined (CELL)
				/*created in execute_command_cell_open in command/cmiss.c */
				if (command_data.cell_interface)
				{
					DESTROY(Cell_interface)(&command_data.cell_interface);
				}
#endif /* defined (CELL) */

#if defined (UNEMAP)			
				/*created in execute_command_unemap_open in command/cmiss.c */
				DESTROY(Unemap_package)(&command_data.unemap_package); 
#endif /* defined (UNEMAP) */

#if defined (MOTIF)
				/* viewers */
				if (command_data.data_viewer)
				{
					DESTROY(Node_viewer)(&(command_data.data_viewer));
				}
				if (command_data.node_viewer)
				{
					DESTROY(Node_viewer)(&(command_data.node_viewer));
				}
				if (command_data.element_point_viewer)
				{
					DESTROY(Element_point_viewer)(&(command_data.element_point_viewer));
				}
				if (command_data.scene_editor)
				{
					DESTROY(Scene_editor)(&(command_data.scene_editor));
				}

				DESTROY(MANAGER(Graphics_window))(
					&command_data.graphics_window_manager);
#endif /* defined (MOTIF) */

				if (computed_field_finite_element_package)
				{
					Computed_field_deregister_types_finite_element(
						computed_field_finite_element_package);
				}

#if defined (MOTIF)
				if (command_data.element_creator)
				{
					DESTROY(Element_creator)(&command_data.element_creator);
				}
				/* destroy Interactive_tools and manager */
				if (command_data.select_tool)
				{
					DESTROY(Select_tool)(&command_data.select_tool);
				}
				if (command_data.element_point_tool)
				{
					DESTROY(Element_point_tool)(&command_data.element_point_tool);
				}
				if (command_data.data_tool)
				{
					DESTROY(Node_tool)(&command_data.data_tool);
				}
				if (command_data.element_tool)
				{
					DESTROY(Element_tool)(&command_data.element_tool);
				}
				if (command_data.node_tool)
				{
					DESTROY(Node_tool)(&command_data.node_tool);
				}
				DESTROY(MANAGER(Interactive_tool))(
					&(command_data.interactive_tool_manager));
#endif /* defined (MOTIF) */

				DEACCESS(Scene)(&(command_data.default_scene));
				DESTROY(MANAGER(Scene))(&command_data.scene_manager);

				if(!no_display)
				{
#if defined (MOTIF) || defined (WIN32_USER_INTERFACE)
					if (console_mode)
					{
#endif /* defined (MOTIF) || defined (WIN32_USER_INTERFACE) */
						if (command_data.command_console)
						{
							DESTROY(Console)(&command_data.command_console);
						}
#if defined (MOTIF) || defined (WIN32_USER_INTERFACE)
					}
					else
					{
						DESTROY(Command_window)(&command_data.command_window);
					}
#endif /* defined (MOTIF) || defined (WIN32_USER_INTERFACE) */
					/* reset up messages */
					set_display_message_function(ERROR_MESSAGE,
					  (Display_message_function *)NULL, NULL);
					set_display_message_function(INFORMATION_MESSAGE,
					  (Display_message_function *)NULL, NULL);
					set_display_message_function(WARNING_MESSAGE,
					  (Display_message_function *)NULL, NULL);
					/* close the user interface */
					DESTROY(User_interface)(&(command_data.user_interface));
				}

				DEACCESS(Time_keeper)(&command_data.default_time_keeper);

				DESTROY(Computed_field_package)(&command_data.computed_field_package);

				DESTROY(Any_object_selection)(&(command_data.any_object_selection));
				DESTROY(FE_node_selection)(&(command_data.data_selection));
				DESTROY(FE_node_selection)(&(command_data.node_selection));
				DESTROY(FE_element_selection)(&(command_data.element_selection));
				DESTROY(Element_point_ranges_selection)(
					&(command_data.element_point_ranges_selection));

#if defined (SELECT_DESCRIPTORS)
				DESTROY(LIST(Io_device))(&command_data.device_list);
#endif /* defined (SELECT_DESCRIPTORS) */

				DESTROY(LIST(GT_object))(&command_data.graphics_object_list);
				DESTROY(LIST(GT_object))(&command_data.glyph_list);

				DESTROY(MANAGER(FE_field))(&command_data.fe_field_manager);

				DESTROY(MANAGER(Control_curve))(&command_data.control_curve_manager);

				DESTROY(MANAGER(GROUP(FE_element)))(&command_data.element_group_manager);
				DESTROY(MANAGER(FE_element))(&command_data.element_manager);
				DESTROY(MANAGER(GROUP(FE_node)))(&command_data.node_group_manager);
				DESTROY(MANAGER(FE_node))(&command_data.node_manager);
				DESTROY(MANAGER(GROUP(FE_node)))(&command_data.data_group_manager);
				DESTROY(MANAGER(FE_node))(&command_data.data_manager);
				DESTROY(MANAGER(FE_basis))(&command_data.basis_manager);

				/* check if there are any objects in all lists; if so, do not destroy
					 them since this will cause memory addresses to be deallocated twice
					 as DEACCESS will also request the objects be removed from the list.
					 Do, however, send out an error message */
				if (0 == NUMBER_IN_LIST(FE_element_field_info)(
					all_FE_element_field_info))
				{
					DESTROY(LIST(FE_element_field_info))(&all_FE_element_field_info);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"%d element field information object(s) still in use",
						NUMBER_IN_LIST(FE_element_field_info)(all_FE_element_field_info));
				}
				if (0 == NUMBER_IN_LIST(FE_element_shape)(all_FE_element_shape))
				{
					DESTROY(LIST(FE_element_shape))(&all_FE_element_shape);
				}
				else
				{
					display_message(ERROR_MESSAGE, "%d element shape(s) still in use",
						NUMBER_IN_LIST(FE_element_shape)(all_FE_element_shape));
				}

				DEACCESS(Spectrum)(&(command_data.default_spectrum));
				DESTROY(MANAGER(Spectrum))(&command_data.spectrum_manager);
				DEACCESS(Graphical_material)(&(command_data.default_graphical_material));			
				DEACCESS(Graphical_material)(&(default_selected_material));			
				DESTROY(MANAGER(Graphical_material))(&command_data.graphical_material_manager);

				DESTROY(MANAGER(VT_volume_texture))(&command_data.volume_texture_manager);
				DESTROY(MANAGER(Texture))(&command_data.texture_manager);
				DESTROY(MANAGER(Environment_map))(&command_data.environment_map_manager);				
				DEACCESS(Light_model)(&(command_data.default_light_model));
				DESTROY(MANAGER(Light_model))(&command_data.light_model_manager);
				DEACCESS(Light)(&(command_data.default_light));
				DESTROY(MANAGER(Light))(&command_data.light_manager);
#if defined (MOTIF)
				DESTROY(MANAGER(Comfile_window))(&command_data.comfile_window_manager);
#endif /* defined (MOTIF) */

				DEACCESS(FE_time)(&(command_data.fe_time));

				if (examples_directory)
				{
					DEALLOCATE(examples_directory);
				}
				if (command_data.example_directory)
				{
					DEALLOCATE(command_data.example_directory);
				}
				if (command_data.example_comfile)
				{
					DEALLOCATE(command_data.example_comfile);
				}
				if (execute_string)
				{
					DEALLOCATE(execute_string);
				}

				DESTROY(Execute_command)(&execute_command);
				DESTROY(Execute_command)(&set_command);

#if defined (F90_INTERPRETER) || defined (PERL_INTERPRETER)
				destroy_interpreter(&status);
#endif /* defined (F90_INTERPRETER) || defined (PERL_INTERPRETER) */

				if (command_data.event_dispatcher)
				{
					DESTROY(Event_dispatcher)(&command_data.event_dispatcher);
				}		

				/* Write out any memory blocks still ALLOCATED when MEMORY_CHECKING is
					on.  When MEMORY_CHECKING is off this function does nothing */
				list_memory(/*count_number*/1, /*show_pointers*/0, /*increment_counter*/0, /*show_structures*/1);
			}
		}
	}
	if (!return_code)
	{
		display_message(INFORMATION_MESSAGE,
			"Usage :\n");
		display_message(INFORMATION_MESSAGE,
			"  cmgui\n");
		display_message(INFORMATION_MESSAGE,
			"    <-batch>                                batch mode\n");
		display_message(INFORMATION_MESSAGE,
			"    <-cm>                                   start cm on entry\n");
		display_message(INFORMATION_MESSAGE,
			"    <-cm_epath PATH_TO_EXAMPLES_DIRECTORY>  override the cm examples directory\n");
		display_message(INFORMATION_MESSAGE,
			"    <-cm_parameters PARAMETER_FILE_NAME>    parameters for cm\n");
		display_message(INFORMATION_MESSAGE,
			"    <-console>                              run using the console instead of command window\n");
		display_message(INFORMATION_MESSAGE,
			"    <-command_list>                         write a list of the commands\n");
		display_message(INFORMATION_MESSAGE,
			"    <-display HOST:0>                       override host to display on\n");
		display_message(INFORMATION_MESSAGE,
			"    <-epath PATH_TO_EXAMPLES_DIRECTORY>     override the examples directory\n");
		display_message(INFORMATION_MESSAGE,
			"    <-example EXAMPLE_ID>                   command file is an example\n");
		display_message(INFORMATION_MESSAGE,
			"    <-execute EXECUTE_STRING>               string executed by cmiss before comfiles\n");
		display_message(INFORMATION_MESSAGE,
			"    <-h>                                    display this usage\n");
		display_message(INFORMATION_MESSAGE,
			"    <-id ID>                                an id for this instance\n");
		display_message(INFORMATION_MESSAGE,
			"    <-mycm>                                 start mycm on entry\n");
		display_message(INFORMATION_MESSAGE,
			"    <-random NUMBER>                        random number seed (else random)\n");
		display_message(INFORMATION_MESSAGE,
			"    <-visual NUMBER>                        specify a particular OpenGl visual id\n");
		display_message(INFORMATION_MESSAGE,
			"    <COMMAND_FILE_NAME>                     execute the command file\n");
		return_code=0;
	}

	LEAVE;

	return (return_code);
} /* main */
