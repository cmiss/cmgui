/* OpenCMISS-Cmgui Application
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */





#if 1
#include "configure/cmgui_configure.h"
#endif /* defined (BUILD_WITH_CMAKE) */

#if defined (GTK_USER_INTERFACE)
#include <gtk/gtk.h>
#if ( GTK_MAJOR_VERSION < 2 ) || defined (WIN32_SYSTEM)
#define GTK_USE_GTKGLAREA
#endif /* ( GTK_MAJOR_VERSION < 2 ) || defined (WIN32_SYSTEM)*/
#if defined (GTK_USE_GTKGLAREA)
#include <gtkgl/gtkglarea.h>
#else /* defined (GTK_USE_GTKGLAREA) */
#include <gtk/gtkgl.h>
#include "graphics/graphics_library.h"
#endif /* defined (GTK_USE_GTKGLAREA) */
#endif /* defined (GTK_USER_INTERFACE) */
#if defined (WIN32_USER_INTERFACE)
#if defined (__CYGWIN__) || defined (__MINGW32__)
/* cygwin/mingw win32 header definitions, so we can tell what version
   we have and to get WINVER defines */
#include <w32api.h>
/* Minimum supported version to allow use of AlphaBlend function */
#define WINVER Windows98
#endif /* defined (__CYGWIN__) || defined (__MINGW32__) */




#include <GL/gl.h>
#endif /* defined (WIN32_USER_INTERFACE) */
#if defined (CARBON_USER_INTERFACE)
#include <OpenGL/glu.h>
#include <AGL/agl.h>
#endif /* defined (CARBON_USER_INTERFACE) */
#if defined (WX_USER_INTERFACE)
#if defined (WIN32)
//#	define WINDOWS_LEAN_AND_MEAN
#	define NOMINMAX
#	include <windows.h>
#endif
#define GL_GLEXT_PROTOTYPES
#include "graphics/graphics_library.h"
#include <wx/wx.h>
#include <wx/glcanvas.h>
#include <wx/debug.h>
#endif /* defined (WX_USER_INTERFACE) */
#include "general/debug.h"
#include "general/message.h"
#include "general/callback_private.h"
#include "general/debug.h"
#include "general/indexed_list_private.h"
#include "general/object.h"
#include "graphics/scene_viewer.h"
#include "three_d_drawing/graphics_buffer.h"
#include "three_d_drawing/graphics_buffer_app.h"
#if defined (UNIX) && !defined (DARWIN)
#include "user_interface/event_dispatcher.h"
#endif
#include "user_interface/user_interface.h"
#include "three_d_drawing/window_system_extensions.h"

enum Graphics_buffer_class
/*******************************************************************************
LAST MODIFIED : 10 March 2005

DESCRIPTION :
==============================================================================*/
{
	GRAPHICS_BUFFER_ONSCREEN_CLASS, /* A normal graphics buffer */
	GRAPHICS_BUFFER_OFFSCREEN_SHARED_CLASS, /* Try to create an offscreen buffer with
															 shared display lists */
	GRAPHICS_BUFFER_OFFSCREEN_CLASS  /* Try to create an offscreen buffer,
													 don't worry whether it shares context or not */
};

#if defined (WX_USER_INTERFACE)
class wxGraphicsBuffer;
#endif /* defined (WX_USER_INTERFACE) */

#if defined (WIN32_USER_INTERFACE)
enum Graphics_buffer_pbuffer_support
{
	GRAPHICS_BUFFER_PBUFFER_SUPPORT_UNKNOWN,
	GRAPHICS_BUFFER_PBUFFER_SUPPORT_AVAILABLE,
	GRAPHICS_BUFFER_PBUFFER_SUPPORT_UNAVAILABLE
};
#endif /* defined (WIN32_USER_INTERFACE) */



struct Graphics_buffer_app_package
{

	struct Graphics_buffer_package *core_package;
#if defined (GTK_USER_INTERFACE)
#  if defined (GTK_USE_GTKGLAREA)
	  GtkWidget *share_glarea;
#  else /* defined (GTK_USE_GTKGLAREA) */
	  GdkGLContext *share_glcontext;
#  endif /* defined (GTK_USE_GTKGLAREA) */
#endif /* defined (GTK_USER_INTERFACE) */
#if defined (WIN32_USER_INTERFACE)
	HGLRC wgl_shared_context;
	User_interface *user_interface;

	/* Hidden window used to get connection to independent driver OpenGL */
	HWND hidden_accelerated_window;
	Graphics_buffer *hidden_graphics_buffer;
	enum Graphics_buffer_pbuffer_support pbuffer_support_available;
	/* Use a separate package for this hidden graphics buffer so we don't try to
	 * share lists with it and in the Intel single context version it has a different context. */
	Graphics_buffer_package *hidden_graphics_package;
	/* Flag to enable work around on Intel cards when using a single context the viewport
	 * isn't updated correctly by forcing a glscissor command to update the viewport size. */
	int intel_single_context_force_clipping;

#endif /* defined (WIN32_USER_INTERFACE) */
#if defined (WX_USER_INTERFACE)
	wxGLCanvas* wxSharedCanvas;
	wxFrame* sharedFrame;
#endif /* defined (WX_USER_INTERFACE) */
};

FULL_DECLARE_CMZN_CALLBACK_TYPES(Graphics_buffer_callback, \
	struct Graphics_buffer_app *, void *);

FULL_DECLARE_CMZN_CALLBACK_TYPES(Graphics_buffer_input_callback, \
	struct Graphics_buffer_app *, struct Graphics_buffer_input *);

/*******************************************************************************
LAST MODIFIED : 5 May 2004

DESCRIPTION :
==============================================================================*/

struct Graphics_buffer_app
{


	struct Graphics_buffer_app_package *package;
	int access_count;

	struct Graphics_buffer *core_buffer;

	struct LIST(CMZN_CALLBACK_ITEM(Graphics_buffer_callback))
		  *initialise_callback_list;
	struct LIST(CMZN_CALLBACK_ITEM(Graphics_buffer_callback))
		  *resize_callback_list;
	struct LIST(CMZN_CALLBACK_ITEM(Graphics_buffer_callback))
		  *expose_callback_list;
	struct LIST(CMZN_CALLBACK_ITEM(Graphics_buffer_input_callback))
		  *input_callback_list;

#if defined (GTK_USER_INTERFACE)

	/* For GRAPHICS_BUFFER_GTKGLAREA_TYPE and GRAPHICS_BUFFER_GTKGLEXT_TYPE */
	GtkWidget *glarea;

#  if defined (GTK_USE_GTKGLAREA)
	   /* For GRAPHICS_BUFFER_GTKGLAREA_TYPE */
	   /* No inquiry functions so we save the state */



#   else /* defined (GTK_USE_GTKGLAREA) */
	   /* For GRAPHICS_BUFFER_GTKGLEXT_TYPE */
	   GdkGLConfig *glconfig;
	   GdkGLContext *glcontext;
	   GdkGLDrawable *gldrawable;

	gulong initialise_handler_id;
	gulong resize_handler_id;
	gulong expose_handler_id;
	gulong button_press_handler_id;
	gulong button_release_handler_id;
	gulong key_press_handler_id;
	gulong key_release_handler_id;
	gulong motion_handler_id;

#   endif /* ! defined (GTK_USER_GLAREA) */
#endif /* defined (GTK_USER_INTERFACE) */
#if defined (WIN32_USER_INTERFACE)
	HWND hWnd;
	HDC hDC;
	HGLRC hRC;
	int pixel_format;
	/* x, y, width and height are used with the windowless mode (no hWnd).
		x and y locate the current hDC with respect to the top left corner of the plugin.
		width and height are the total size of the plugin port.  */
	int width;
	int height;
	int x;
	int y;
	/* Work around bug in firefox 3.0 and 3.5 where mouse coordinates are always given with respect
	 * to the containing window, rather than the last SetWindow call.
	 * Try to discern the offset of the containing window by assuming we will get a full
	 * window redraw before any mouse events.
	 * See https://tracker.physiomeproject.org/show_bug.cgi?id=921
	 */
	int mouse_x;
	int mouse_y;
	/* For offscreen rendering */
	int offscreen_width;
	int offscreen_height;
#ifdef WGL_ARB_pbuffer
	HPBUFFERARB pbuffer;
#endif /* defined (WGL_ARB_pbuffer) */
	/* We need the creation parameters to support delayed creation and recreation
		as the pbuffer changes size. */
	int minimum_colour_buffer_depth;
	int minimum_depth_buffer_depth;
	int minimum_accumulation_buffer_depth;
	int minimum_alpha_buffer_depth;
	enum Graphics_buffer_stereo_mode stereo_mode;

	/* Windows bitmap, either used with non accelerated windows OpenGL or
		for copying from pbuffer for rendering to screen */
	HBITMAP device_independent_bitmap;
	HDC device_independent_bitmap_hdc;
	void *device_independent_bitmap_pixels;
	/* So we know how to composite we need to keep the buffering mode */
	enum Graphics_buffer_buffering_mode buffering_mode;
	/* Some calls to the scene viewer mean that we will need to rerender
	   the offscreen window (such as resizing) so this flag tells us this. */
	int offscreen_render_required;
#endif /* defined (WIN32_USER_INTERFACE) */
#if defined (CARBON_USER_INTERFACE)
	WindowRef theWindow;
	/* These parameters are provided by the hosting application and should be
		respected by this graphics_buffer */
	int width;
	int height;
	int clip_width;
	int clip_height;
	AGLPixelFormat aglPixelFormat;
	AGLContext aglContext;
	EventHandlerRef expose_handler_ref;
	EventHandlerRef mouse_handler_ref;
	EventHandlerRef resize_handler_ref;
#endif /* defined (CARBON_USER_INTERFACE) */
#if defined (WX_USER_INTERFACE)
	wxPanel *parent;
	wxGraphicsBuffer *canvas;
#endif /* defined (WX_USER_INTERFACE) */
};


struct Graphics_buffer_package *Graphics_buffer_package_get_core_package(struct Graphics_buffer_app_package* package)
{
	if (package)
		return package->core_package;
	return 0;
}


#if defined (WIN32_USER_INTERFACE)
static LRESULT CALLBACK Graphics_buffer_callback_proc(HWND window, UINT message_identifier,
	WPARAM first_message, LPARAM second_message);
#endif /* defined (WIN32_USER_INTERFACE) */

DEFINE_CMZN_CALLBACK_MODULE_FUNCTIONS(Graphics_buffer_callback, void)

DEFINE_CMZN_CALLBACK_FUNCTIONS(Graphics_buffer_callback, \
	struct Graphics_buffer_app *,void *)

DEFINE_CMZN_CALLBACK_MODULE_FUNCTIONS(Graphics_buffer_input_callback, void)

DEFINE_CMZN_CALLBACK_FUNCTIONS(Graphics_buffer_input_callback, \
	struct Graphics_buffer_app *, struct Graphics_buffer_input *)

DECLARE_OBJECT_FUNCTIONS(Graphics_buffer_app)

static struct Graphics_buffer_app *CREATE(Graphics_buffer_app)(
	struct Graphics_buffer_app_package *package,
		enum Graphics_buffer_type type,
		enum Graphics_buffer_buffering_mode buffering_mode,
		enum Graphics_buffer_stereo_mode stereo_mode)
/*******************************************************************************
LAST MODIFIED : 4 May 2004

DESCRIPTION :
This is static as it is designed to be called by the different constructors
contained in the this module only.
==============================================================================*/
{


	struct Graphics_buffer_app *buffer = 0;

	if (ALLOCATE(buffer, struct Graphics_buffer_app, 1))
	{
		buffer->access_count = 1;
		buffer->package = package;
		buffer->core_buffer = CREATE(Graphics_buffer)(package->core_package, type, buffering_mode, stereo_mode);
		buffer->initialise_callback_list=
			CREATE(LIST(CMZN_CALLBACK_ITEM(Graphics_buffer_callback)))();
		buffer->resize_callback_list=
			CREATE(LIST(CMZN_CALLBACK_ITEM(Graphics_buffer_callback)))();
		buffer->expose_callback_list=
			CREATE(LIST(CMZN_CALLBACK_ITEM(Graphics_buffer_callback)))();
		buffer->input_callback_list=
			CREATE(LIST(CMZN_CALLBACK_ITEM(Graphics_buffer_input_callback)))();

#if defined (GTK_USER_INTERFACE)

/* For GRAPHICS_BUFFER_GTKGLAREA_TYPE and GRAPHICS_BUFFER_GTKGLEXT_TYPE */
	buffer->glarea = (GtkWidget *)NULL;

#   if defined (GTK_USE_GTKGLAREA)
/* For GRAPHICS_BUFFER_GTKGLAREA_TYPE */
		/* No inquiry functions so we save the state */
		buffer->buffering_mode = GRAPHICS_BUFFER_ANY_BUFFERING_MODE;
		 buffer->stereo_mode = GRAPHICS_BUFFER_ANY_STEREO_MODE;
#   else /* defined (GTK_USE_GTKGLAREA) */
/* For GRAPHICS_BUFFER_GTKGLEXT_TYPE */
		buffer->glconfig = (GdkGLConfig *)NULL;
		buffer->glcontext = (GdkGLContext *)NULL;
		buffer->gldrawable = (GdkGLDrawable *)NULL;
#   endif /* ! defined (GTK_USER_GLAREA) */
#endif /* defined (GTK_USER_INTERFACE) */

#if defined (WIN32_USER_INTERFACE)
		 buffer->hWnd = (HWND)NULL;
		 buffer->hDC = (HDC)NULL;
		 buffer->hRC = (HGLRC)NULL;
		 buffer->pixel_format = 0;



		 buffer->height = 0;
		 buffer->x = 0;
		 buffer->y = 0;
		 buffer->mouse_x = 0;
		 buffer->mouse_y = 0;
		 buffer->offscreen_width = 0;
		 buffer->offscreen_height = 0;
#ifdef WGL_ARB_pbuffer
		 buffer->pbuffer = (HPBUFFERARB)NULL;
#endif /* defined (WGL_ARB_pbuffer) */
		buffer->buffering_mode = GRAPHICS_BUFFER_ANY_BUFFERING_MODE;
		 buffer->minimum_colour_buffer_depth = 0;
		 buffer->minimum_depth_buffer_depth = 0;
		 buffer->minimum_accumulation_buffer_depth = 0;
		 buffer->minimum_alpha_buffer_depth = 0;
		 buffer->stereo_mode = GRAPHICS_BUFFER_ANY_STEREO_MODE;
		 buffer->device_independent_bitmap = (HBITMAP)NULL;
		 buffer->device_independent_bitmap_hdc = (HDC)NULL;
		 buffer->device_independent_bitmap_pixels = NULL;
		 buffer->offscreen_render_required = 0;
#endif // defined (WIN32_USER_INTERFACE)



#if defined (CARBON_USER_INTERFACE)
		 buffer->theWindow = 0;
		 buffer->width = 0;
		 buffer->height = 0;
		 buffer->clip_width = 0;
		 buffer->clip_height = 0;
		 buffer->aglPixelFormat = (AGLPixelFormat)NULL;
		 buffer->aglContext = (AGLContext)NULL;
		 buffer->expose_handler_ref = (EventHandlerRef)NULL;
		 buffer->mouse_handler_ref = (EventHandlerRef)NULL;
		 buffer->resize_handler_ref = (EventHandlerRef)NULL;
#endif /* defined (CARBON_USER_INTERFACE) */
#if defined (WX_USER_INTERFACE)
		 buffer->parent = (wxPanel *)NULL;
		 buffer->canvas = (wxGraphicsBuffer *)NULL;
#endif /* defined (CARBON_USER_INTERFACE) */
	}
	else
	{
		display_message(ERROR_MESSAGE,"CREATE(Graphics_buffer_app). Unable to allocate buffer structure");
	}

	return (buffer);
} /* CREATE(Graphics_buffer_app) */





#if defined (WX_USER_INTERFACE)
class wxGraphicsBuffer : public wxGLCanvas
{
	Graphics_buffer_app *graphics_buffer;
	wxPanel *parent;
	int key_code, cursor_x, cursor_y;
public:

	wxGraphicsBuffer(wxPanel *parent, wxGLCanvas* sharedCanvas,
		Graphics_buffer_app *graphics_buffer
		 , int *attrib_list):
	wxGLCanvas(parent, sharedCanvas, wxID_ANY, wxDefaultPosition, parent->GetSize(),
		 wxFULL_REPAINT_ON_RESIZE, wxT("GLCanvas")
		 , attrib_list),
		graphics_buffer(graphics_buffer), parent(parent), key_code(0), cursor_x(-1),
		cursor_y(-1)
	{
	}

	~wxGraphicsBuffer()
	{
		if (graphics_buffer)
		{
			graphics_buffer->canvas = (wxGraphicsBuffer *)NULL;
			if ((this == graphics_buffer->package->wxSharedCanvas))
			{
				graphics_buffer->package->wxSharedCanvas = NULL;
			}
		}
	}

	void ClearGraphicsBufferReference()
	{
		graphics_buffer = 0;
	}


	void OnPaint( wxPaintEvent& WXUNUSED(event) )
	{

		/* Unfortunately can't find a better place to copy the shareable context */
		if (!graphics_buffer->package->wxSharedCanvas)
		{
			graphics_buffer->package->wxSharedCanvas = this;
		}
		/* must always be here */
		wxPaintDC dc(this);

		CMZN_CALLBACK_LIST_CALL(Graphics_buffer_callback)(
			graphics_buffer->expose_callback_list, graphics_buffer, NULL);
	}

	void OnSize(wxSizeEvent& event)
	{
		// this is also necessary to update the context on some platforms
		wxGLCanvas::OnSize(event);

		Graphics_buffer_set_height(graphics_buffer->core_buffer, event.GetSize().GetHeight());
		Graphics_buffer_set_width(graphics_buffer->core_buffer, event.GetSize().GetWidth());

		CMZN_CALLBACK_LIST_CALL(Graphics_buffer_callback)(
			graphics_buffer->resize_callback_list, graphics_buffer, NULL);
	}

	void OnEraseBackground(wxEraseEvent& WXUNUSED(event))
	{
		/* Do nothing, to avoid flashing on MSW */
	}

	void OnKeyEvent(cmzn_sceneviewerinput *input)
	{
		input->button_number = 0;
		input->position_x = cursor_x;
		input->position_y = cursor_y;
		CMZN_CALLBACK_LIST_CALL(Graphics_buffer_input_callback)(
			graphics_buffer->input_callback_list, graphics_buffer, input);
	}

	void OnKeyUp( wxKeyEvent& event )
	{
		struct Graphics_buffer_input input;
		input.type = CMZN_SCENEVIEWERINPUT_EVENT_TYPE_KEY_RELEASE;
		key_code = event.GetKeyCode();
		input.key_code = key_code;
		int input_modifier = 0;
		if (event.ShiftDown())
		{
			input_modifier |= GRAPHICS_BUFFER_INPUT_MODIFIER_SHIFT;
		}
#if defined (DARWIN)
		if (event.CmdDown())
		{
			input_modifier |= GRAPHICS_BUFFER_INPUT_MODIFIER_SHIFT;
		}
#endif
		if (event.ControlDown())
		{
			input_modifier |= GRAPHICS_BUFFER_INPUT_MODIFIER_CONTROL;
		}
		if (event.AltDown())
		{
			input_modifier |= GRAPHICS_BUFFER_INPUT_MODIFIER_ALT;
		}
		input.modifiers = static_cast<enum Graphics_buffer_input_modifier>(input_modifier);
		OnKeyEvent(&input);
		event.Skip();
	}

	void OnKeyDown( wxKeyEvent& event )
	{
		struct Graphics_buffer_input input;
		input.type = CMZN_SCENEVIEWERINPUT_EVENT_TYPE_KEY_PRESS;
		key_code = event.GetKeyCode();
		input.key_code = key_code;
		int input_modifier = 0;
		if (event.ShiftDown())
		{
			input_modifier |= GRAPHICS_BUFFER_INPUT_MODIFIER_SHIFT;
		}
#if defined (DARWIN)
		if (event.CmdDown())
		{
			input_modifier |= GRAPHICS_BUFFER_INPUT_MODIFIER_SHIFT;
		}
#endif
		if (event.ControlDown())
		{
			input_modifier |= GRAPHICS_BUFFER_INPUT_MODIFIER_CONTROL;
		}
		if (event.AltDown())
		{
			input_modifier |= GRAPHICS_BUFFER_INPUT_MODIFIER_ALT;
		}
		input.modifiers = static_cast<enum Graphics_buffer_input_modifier>(input_modifier);
		OnKeyEvent(&input);
		event.Skip();
	}

	void OnMouse( wxMouseEvent& event )
	{
		int input_modifier, return_code = 1;
		struct Graphics_buffer_input input;

		input.type = CMZN_SCENEVIEWERINPUT_EVENT_TYPE_INVALID;
		input.button_number = 0;
		input.key_code = key_code;
		cursor_x = input.position_x = event.GetX();
		cursor_y = input.position_y = event.GetY();
		input_modifier = 0;
		if (event.Leaving())
		{
			cursor_x = input.position_x = -1;
			cursor_y = input.position_y = -1;
		}
		if (event.ShiftDown())
		{
			input_modifier |= GRAPHICS_BUFFER_INPUT_MODIFIER_SHIFT;
		}
#if defined (DARWIN)
		if (event.CmdDown())
		{
			input_modifier |= GRAPHICS_BUFFER_INPUT_MODIFIER_SHIFT;
		}
#endif
		if (event.ControlDown())
		{
			input_modifier |= GRAPHICS_BUFFER_INPUT_MODIFIER_CONTROL;
		}
		if (event.AltDown())
		{
			input_modifier |= GRAPHICS_BUFFER_INPUT_MODIFIER_ALT;
		}

		if (event.Dragging())
		{
			input.type = CMZN_SCENEVIEWERINPUT_EVENT_TYPE_MOTION_NOTIFY;
			if (event.LeftIsDown())
			{
				input_modifier |= GRAPHICS_BUFFER_INPUT_MODIFIER_BUTTON1;
			}
		}
		else if (event.ButtonDown())
		{
			if (this != this->FindFocus())
			{
				input.key_code = 0;
				this->SetFocus();
			}
			input.type = CMZN_SCENEVIEWERINPUT_EVENT_TYPE_BUTTON_PRESS;
			switch (event.GetButton())
			{
				case wxMOUSE_BTN_LEFT:
				{
					input.button_number = 1;
				} break;
				case wxMOUSE_BTN_MIDDLE:
				{
					input.button_number = 2;
				} break;
				case wxMOUSE_BTN_RIGHT:
				{
					input.button_number = 3;
				} break;
				case wxMOUSE_BTN_NONE:
				default:
				{
					display_message(ERROR_MESSAGE,
						"wxGraphicsBuffer_input_callback::OnMouse.  Invalid button");
					return_code=0;
				} break;
			}
		}
		else if (event.ButtonUp())
		{
			input.type = CMZN_SCENEVIEWERINPUT_EVENT_TYPE_BUTTON_RELEASE;
			switch (event.GetButton())
			{
				case wxMOUSE_BTN_LEFT:
				{
					input.button_number = 1;
				} break;
				case wxMOUSE_BTN_MIDDLE:
				{
					input.button_number = 2;
				} break;
				case wxMOUSE_BTN_RIGHT:
				{
					input.button_number = 3;
				} break;
				case wxMOUSE_BTN_NONE:
				default:
				{
					display_message(ERROR_MESSAGE,
						"wxGraphicsBuffer_input_callback::OnMouse.  Invalid button");
					return_code=0;
				} break;
			}
		}
		else
		{
			/* Ignore other events */
			return_code = 0;
		}

		input.modifiers = static_cast<enum Graphics_buffer_input_modifier>(input_modifier);

		if (return_code)
		{
			CMZN_CALLBACK_LIST_CALL(Graphics_buffer_input_callback)(
				graphics_buffer->input_callback_list, graphics_buffer, &input);
		}
	}

	DECLARE_EVENT_TABLE();
};

BEGIN_EVENT_TABLE(wxGraphicsBuffer, wxGLCanvas)
	EVT_SIZE(wxGraphicsBuffer::OnSize)
	EVT_PAINT(wxGraphicsBuffer::OnPaint)
	EVT_ERASE_BACKGROUND(wxGraphicsBuffer::OnEraseBackground)
	EVT_KEY_UP(wxGraphicsBuffer::OnKeyUp)
	EVT_KEY_DOWN(wxGraphicsBuffer::OnKeyDown)
	EVT_MOUSE_EVENTS(wxGraphicsBuffer::OnMouse)
END_EVENT_TABLE()

class wxTestingBuffer : public wxGLCanvas
{
	 wxPanel *parent;
	 Graphics_buffer_app *graphics_buffer;

public:
	wxTestingBuffer(wxPanel *parent, Graphics_buffer_app *graphics_buffer, int *attrib_array)
		: wxGLCanvas(parent, wxID_ANY, wxDefaultPosition, parent->GetSize(),
			wxFULL_REPAINT_ON_RESIZE, wxT("GLCanvas"), attrib_array)
		, parent(parent), graphics_buffer(graphics_buffer)
	{
	}

	~wxTestingBuffer()
	{
	}


	void setSharedCanvas()
	{
		graphics_buffer->package->wxSharedCanvas = this;
#if WXWIN_COMPATIBILITY_2_8
#if defined (UNIX)
#if !defined (DARWIN)
		GTKInitImplicitContext();
#endif
#endif
#endif
	}
};


void Graphics_buffer_create_buffer_wx(
	struct Graphics_buffer_app *buffer,
	struct Graphics_buffer_app_package *graphics_buffer_package,
	wxPanel *parent,
	enum Graphics_buffer_buffering_mode buffering_mode,
	enum Graphics_buffer_stereo_mode stereo_mode,
	int minimum_colour_buffer_depth, int minimum_depth_buffer_depth,
	int minimum_accumulation_buffer_depth,
	int width, int height, struct Graphics_buffer_app  *buffer_to_match)
{

	int *visual_attributes;
	int return_code = 0;
	wxLogNull logNo;
	if (buffer)
	{
		buffer->parent = parent;

		if (buffer->core_buffer->type == GRAPHICS_BUFFER_GL_EXT_FRAMEBUFFER_TYPE)
		{
#if defined (OPENGL_API) && defined (GL_EXT_framebuffer_object)
			Graphics_buffer_initialise_framebuffer(buffer->core_buffer, width, height);
#endif
		}
		else if (buffer->core_buffer->type == GRAPHICS_BUFFER_WX_OFFSCREEN_TYPE)
		{
		}
		else
		{
#if defined (UNIX)
#if !defined (DARWIN)
			int *attribute_ptr, number_of_visual_attributes, selection_level;
			visual_attributes = NULL;
			number_of_visual_attributes = 0;
			Event_dispatcher_use_wxCmguiApp_OnAssertFailure(1);
			number_of_visual_attributes = 20;
			return_code = 0;
			/* test either there are visual attributes stored in the current
					 buffer or not*/
			if (buffer_to_match)
			{
				if (buffer_to_match->core_buffer->attrib_list)
				{
					return_code = 1;
				}
			}
			if (!return_code)
			{
				/* if not, test, create a new visual attribute list and create a
							new canvas, else use the current visual attribute list*/
				if (ALLOCATE(visual_attributes, int, number_of_visual_attributes))
				{
					selection_level = 5;
					while ((selection_level > 0) && (buffer->core_buffer->attrib_list == NULL) || (selection_level == 5))
					{
						attribute_ptr = visual_attributes;
						*attribute_ptr = WX_GL_RGBA;
						attribute_ptr++;
						*attribute_ptr = WX_GL_MIN_RED;
						attribute_ptr++;
						*attribute_ptr = (minimum_colour_buffer_depth + 2) / 3;
						attribute_ptr++;
						*attribute_ptr = WX_GL_MIN_GREEN;
						attribute_ptr++;
						*attribute_ptr = (minimum_colour_buffer_depth + 2) / 3;
						attribute_ptr++;
						*attribute_ptr = WX_GL_MIN_BLUE;
						attribute_ptr++;
						*attribute_ptr = (minimum_colour_buffer_depth + 2) / 3;
						attribute_ptr++;
						if (selection_level > 3)
						{
							*attribute_ptr = WX_GL_MIN_ALPHA;
							attribute_ptr++;
							*attribute_ptr = 1;
							attribute_ptr++;
						}
						if (minimum_depth_buffer_depth > 0)
						{
							*attribute_ptr = WX_GL_DEPTH_SIZE;
							attribute_ptr++;
							*attribute_ptr = minimum_depth_buffer_depth;
							attribute_ptr++;
						}
						else
						{
							if (selection_level > 2)
							{
								/* Try to get a depth buffer anyway */
								*attribute_ptr = WX_GL_DEPTH_SIZE;
								attribute_ptr++;
								*attribute_ptr = 16;
								attribute_ptr++;
							}
						}
						if (minimum_accumulation_buffer_depth > 0)
						{
							*attribute_ptr = WX_GL_MIN_ACCUM_RED;
							attribute_ptr++;
							*attribute_ptr = (minimum_accumulation_buffer_depth + 2) / 3;
							attribute_ptr++;
							*attribute_ptr = WX_GL_MIN_ACCUM_GREEN;
							attribute_ptr++;
							*attribute_ptr = (minimum_accumulation_buffer_depth + 2) / 3;
							attribute_ptr++;
							*attribute_ptr = WX_GL_MIN_ACCUM_BLUE;
							attribute_ptr++;
							*attribute_ptr = (minimum_accumulation_buffer_depth + 2) / 3;
							attribute_ptr++;
						}
						else
						{
							if (selection_level > 4)
							{
								/* Try to get an accumulation buffer anyway */
								*attribute_ptr = WX_GL_MIN_ACCUM_RED;
								attribute_ptr++;
								*attribute_ptr = (minimum_colour_buffer_depth + 2) / 3;
								attribute_ptr++;
								*attribute_ptr = WX_GL_MIN_ACCUM_GREEN;
								attribute_ptr++;
								*attribute_ptr = (minimum_colour_buffer_depth + 2) / 3;
								attribute_ptr++;
								*attribute_ptr = WX_GL_MIN_ACCUM_BLUE;
								attribute_ptr++;
								*attribute_ptr = (minimum_colour_buffer_depth + 2) / 3;
								attribute_ptr++;
							}
						}
						switch (buffering_mode)
						{
						case GRAPHICS_BUFFER_SINGLE_BUFFERING:
						case GRAPHICS_BUFFER_DOUBLE_BUFFERING:
						{
							*attribute_ptr = WX_GL_DOUBLEBUFFER;
							attribute_ptr++;
						}break;
						case GRAPHICS_BUFFER_ANY_BUFFERING_MODE:
						case GRAPHICS_BUFFER_RENDER_OFFSCREEN_AND_COPY:
						case GRAPHICS_BUFFER_RENDER_OFFSCREEN_AND_BLEND:
						{
							/* Do nothing */
						} break;
						}
						switch (stereo_mode)
						{
						case GRAPHICS_BUFFER_MONO:
						case GRAPHICS_BUFFER_STEREO:
						{
							*attribute_ptr = GL_STEREO;
							attribute_ptr++;
						} break;
						case GRAPHICS_BUFFER_ANY_STEREO_MODE:
						{
							/* default GRAPHICS_BUFFER_ANY_STEREO_MODE*/
						} break;
						}
						*attribute_ptr = 0;
						attribute_ptr++;
						bool supported = wxGLCanvas::IsDisplaySupported(visual_attributes);
						selection_level--;
						if ((selection_level == 0) && (!supported))
						{
							DEALLOCATE(visual_attributes);
							visual_attributes = NULL;
							buffer->core_buffer->attrib_list = visual_attributes;
						}
						else if (supported)
						{
							buffer->core_buffer->attrib_list = visual_attributes;
						}

					}
				}
			}
			else
			{
				if (buffer_to_match->core_buffer->attrib_list)
				{
					/* if attrib_list is found on the buffer to match, copy it
								 into the new buffer, if not found, that means the
								 current buffer does not have any special attributes
								 setting, thus the new attributes will be default as NULL */
					int count;
					int *buffer_to_match_attribute_ptr;
					if (ALLOCATE(buffer->core_buffer->attrib_list,int, number_of_visual_attributes))
					{
						buffer_to_match_attribute_ptr = buffer_to_match->core_buffer->attrib_list;
						attribute_ptr = buffer->core_buffer->attrib_list;
						for (count = 0; count < number_of_visual_attributes; count++)
						{
							*attribute_ptr = *buffer_to_match_attribute_ptr;
							attribute_ptr++;
							buffer_to_match_attribute_ptr++;
						}
					}
				}
			}
#else /*defined (DARWIN) */
			/* Mac will receive an argument from wxGLCanvas to get
				   the best settings but requires the program to state
					 all the desired settings with a minimum settings. */
			visual_attributes = NULL;
			if (ALLOCATE(buffer->core_buffer->attrib_list, int, 25))
			{
				buffer->core_buffer->attrib_list[0] = WX_GL_RGBA;
				buffer->core_buffer->attrib_list[1] = WX_GL_DOUBLEBUFFER;
				buffer->core_buffer->attrib_list[2] = WX_GL_DEPTH_SIZE;
				buffer->core_buffer->attrib_list[3] = 1;
				buffer->core_buffer->attrib_list[4] = WX_GL_MIN_RED;
				buffer->core_buffer->attrib_list[5] = 1;
				buffer->core_buffer->attrib_list[6] = WX_GL_MIN_GREEN;
				buffer->core_buffer->attrib_list[7] = 1;
				buffer->core_buffer->attrib_list[8] = WX_GL_MIN_BLUE;
				buffer->core_buffer->attrib_list[9] = 1;
				buffer->core_buffer->attrib_list[10] = WX_GL_MIN_ALPHA;
				buffer->core_buffer->attrib_list[11] = 1;
				buffer->core_buffer->attrib_list[12] = WX_GL_MIN_ACCUM_RED;
				buffer->core_buffer->attrib_list[13] = 1;
				buffer->core_buffer->attrib_list[14] = WX_GL_MIN_ACCUM_GREEN;
				buffer->core_buffer->attrib_list[15] = 1;
				buffer->core_buffer->attrib_list[16] = WX_GL_MIN_ACCUM_BLUE;
				buffer->core_buffer->attrib_list[17] = 1;
				buffer->core_buffer->attrib_list[18] = WX_GL_MIN_ACCUM_ALPHA;
				buffer->core_buffer->attrib_list[19] = 1;
				buffer->core_buffer->attrib_list[20] = WX_GL_DEPTH_SIZE;
				buffer->core_buffer->attrib_list[21] = 1;
				buffer->core_buffer->attrib_list[22] = WX_GL_STENCIL_SIZE;
				buffer->core_buffer->attrib_list[23] = 1;
				buffer->core_buffer->attrib_list[24] = 0;
			};
#endif /*defined (DARWIN) */
#else /* defined (UNIX) */
			USE_PARAMETER(buffer_to_match);
			USE_PARAMETER(buffering_mode);
			USE_PARAMETER(minimum_accumulation_buffer_depth);
			USE_PARAMETER(minimum_colour_buffer_depth);
			USE_PARAMETER(minimum_depth_buffer_depth);
			USE_PARAMETER(stereo_mode);

			/* The above routine does not work for win32 as it does not have the
					 member m_vi in wxGLCanvas.
					 should find a way to get the best buffer, but this default setting should work fine. */
			visual_attributes = NULL;
			if ALLOCATE(buffer->core_buffer->attrib_list, int, 5)
			{
				buffer->core_buffer->attrib_list[0] = WX_GL_DOUBLEBUFFER;
				buffer->core_buffer->attrib_list[1] = WX_GL_RGBA;
				buffer->core_buffer->attrib_list[2] = WX_GL_MIN_ALPHA;
				buffer->core_buffer->attrib_list[3] = 8;
				buffer->core_buffer->attrib_list[4] = 0;
			}
#endif /* defined (UNIX) */
			if (!buffer->package->wxSharedCanvas)
			{
				wxFrame *frame = new wxFrame(0, -1, wxT("temporary"));
				wxPanel *temp = new wxPanel(frame);
				struct Graphics_buffer_app *temp_buffer;
				temp_buffer = CREATE(Graphics_buffer_app)(graphics_buffer_package,
					GRAPHICS_BUFFER_ONSCREEN_TYPE, GRAPHICS_BUFFER_ANY_BUFFERING_MODE, GRAPHICS_BUFFER_ANY_STEREO_MODE);
				temp_buffer->parent = temp;
				temp_buffer->core_buffer->attrib_list = NULL;
				//frame->Show(true);
				wxTestingBuffer *testingbuffer = new wxTestingBuffer(temp, temp_buffer,
													buffer->core_buffer->attrib_list);
				testingbuffer->setSharedCanvas();
				frame->Show(false);
				buffer->package->sharedFrame = frame;
				DESTROY(Graphics_buffer_app)(&temp_buffer);
			}
			buffer->canvas = new wxGraphicsBuffer(parent,
												  buffer->package->wxSharedCanvas,
												  buffer, buffer->core_buffer->attrib_list);

			wxBoxSizer *topsizer = new wxBoxSizer( wxVERTICAL );
			topsizer->Add(buffer->canvas, wxSizerFlags(1).Align(wxALIGN_CENTER).Expand());
			parent->SetSizer(topsizer);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Graphics_buffer_create_buffer_wx.  "
						"Unable to create generic Graphics_buffer.");
		buffer = 0;
	}
	LEAVE;

} /* Graphics_buffer_create_buffer_wx */
#endif /* defined (WX_USER_INTERFACE) */

#if defined (GTK_USER_INTERFACE)
static void Graphics_buffer_gtkglarea_initialise_callback(GtkWidget *widget,
	gpointer graphics_buffer_void)
/*******************************************************************************
LAST MODIFIED : 10 July 2002

DESCRIPTION :
Called when part of the graphics_buffer window is initialised. Does not attempt to
redraw just the initialised area. Instead, it redraws the whole picture, but only
if there are no more initialise events pending.
==============================================================================*/
{
	struct Graphics_buffer *graphics_buffer;

	ENTER(graphics_buffer_gtkglarea_initialise_callback);
	if (widget && (graphics_buffer = (struct Graphics_buffer *)graphics_buffer_void))
	{
#if ! defined (GTK_USE_GTKGLAREA)
		graphics_buffer->glcontext = gtk_widget_get_gl_context(graphics_buffer->glarea);
		if (!graphics_buffer->package->share_glcontext)
		{
			/* This context is owned by the widget, so we can't keep a
			 * reference to it past the life of the widget, and we
			 * certainly can't destroy it. So instead, make a copy of
			 * it. The choice of glarea as the GLDrawable is arbitrary.
			 */
			graphics_buffer->package->share_glcontext =
				gtk_widget_create_gl_context(graphics_buffer->glarea,
				graphics_buffer->glcontext, TRUE, GDK_GL_RGBA_TYPE);

#if defined (UNIX)
			Graphics_library_initialise_gtkglext_glx_extensions(
				gdk_gl_context_get_gl_config(graphics_buffer->package->share_glcontext));
#endif /* defined (UNIX) */
		}
		graphics_buffer->gldrawable = gtk_widget_get_gl_drawable(graphics_buffer->glarea);
#endif /* defined (GTK_USER_INTERFACE) */
		CMZN_CALLBACK_LIST_CALL(Graphics_buffer_callback)(
			graphics_buffer->initialise_callback_list, graphics_buffer, NULL);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"graphics_buffer_gtkglarea_initialise_callback.  Invalid argument(s)");
	}
	LEAVE;
} /* graphics_buffer_gtkglarea_initialise_callback */
#endif /* defined (GTK_USER_INTERFACE) */

#if defined (GTK_USER_INTERFACE)
static void Graphics_buffer_gtkglarea_resize_callback(GtkWidget *widget,
	GtkAllocation *allocation, gpointer graphics_buffer_void)
/*******************************************************************************
LAST MODIFIED : 10 July 2002

DESCRIPTION :
Called when part of the graphics_buffer window is resized. Does not attempt to
redraw just the resized area. Instead, it redraws the whole picture, but only
if there are no more resize events pending.
==============================================================================*/
{
	struct Graphics_buffer *graphics_buffer;

	ENTER(graphics_buffer_gtkglarea_resize_callback);
	USE_PARAMETER(allocation);
	if (widget && (graphics_buffer = (struct Graphics_buffer *)graphics_buffer_void))
	{
		CMZN_CALLBACK_LIST_CALL(Graphics_buffer_callback)(
			graphics_buffer->resize_callback_list, graphics_buffer, NULL);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"graphics_buffer_gtkglarea_resize_callback.  Invalid argument(s)");
	}
	LEAVE;
} /* graphics_buffer_gtkglarea_resize_callback */
#endif /* defined (GTK_USER_INTERFACE) */

#if defined (GTK_USER_INTERFACE)
static gboolean Graphics_buffer_gtkglarea_expose_callback(GtkWidget *widget,
	GdkEventExpose *expose_event, gpointer graphics_buffer_void)
/*******************************************************************************
LAST MODIFIED : 10 July 2002

DESCRIPTION :
Called when part of the graphics_buffer window is exposed. Does not attempt to
redraw just the exposed area. Instead, it redraws the whole picture, but only
if there are no more expose events pending.
==============================================================================*/
{
	struct Graphics_buffer *graphics_buffer;

	ENTER(Graphics_buffer_gtkglarea_expose_callback);
	USE_PARAMETER(expose_event);
	if (widget && (graphics_buffer = (struct Graphics_buffer *)graphics_buffer_void))
	{
		if (0 == expose_event->count)
		{
			CMZN_CALLBACK_LIST_CALL(Graphics_buffer_callback)(
				graphics_buffer->expose_callback_list, graphics_buffer, NULL);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"graphics_buffer_gtkglarea_expose_callback.  Invalid argument(s)");
	}
	LEAVE;

	return(TRUE);
} /* graphics_buffer_gtkglarea_expose_callback */
#endif /* defined (GTK_USER_INTERFACE) */

#if defined (GTK_USER_INTERFACE)
static gboolean Graphics_buffer_gtkglarea_button_callback(GtkWidget *widget,
	GdkEventButton *button_event, gpointer graphics_buffer_structure)
/*******************************************************************************
LAST MODIFIED : 11 July 2002

DESCRIPTION :
The callback for mouse button input in the graphics_buffer window. The
resulting behaviour depends on the <graphics_buffer> input_mode. In Transform mode
mouse clicks and drags are converted to transformation; in Select mode OpenGL
picking is performed with picked objects and mouse click and drag information
returned to the scene.
==============================================================================*/
{
	int input_modifier, return_code;
	struct Graphics_buffer *graphics_buffer;
	struct Graphics_buffer_input input;

	ENTER(Graphics_buffer_gtkglarea_button_callback);
	USE_PARAMETER(widget);
	if ((graphics_buffer=(struct Graphics_buffer *)graphics_buffer_structure)
		&& button_event)
	{
		return_code = 1;
		input.type = GRAPHICS_BUFFER_INVALID_INPUT;
		switch(button_event->type)
		{
			case GDK_BUTTON_PRESS:
			{
				input.type = GRAPHICS_BUFFER_BUTTON_PRESS;
			} break;
			case GDK_BUTTON_RELEASE:
			{
				input.type = GRAPHICS_BUFFER_BUTTON_RELEASE;
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"Graphics_buffer_gtkglarea_button_callback.  Unknown button event");
				return_code=0;
				/* This event type is not being passed on */
			} break;
		}
		input.key_code = 0;
		input.button_number = button_event->button;
		/* Maybe I should change Graphics_buffer_input to have a higher
			resolution for position */
		input.position_x = static_cast<int>(button_event->x);
		input.position_y = static_cast<int>(button_event->y);
		input_modifier = 0;
		if (GDK_SHIFT_MASK&(button_event->state))
		{
			input_modifier |= GRAPHICS_BUFFER_INPUT_MODIFIER_SHIFT;sharedContext
		}
		if (GDK_CONTROL_MASK&(button_event->state))
		{
			input_modifier |= GRAPHICS_BUFFER_INPUT_MODIFIER_CONTROL;
		}
		if (GDK_MOD1_MASK&(button_event->state))
		{
			input_modifier |= GRAPHICS_BUFFER_INPUT_MODIFIER_ALT;
		}
		if (GDK_BUTTON1_MASK&(button_event->state))
		{
			input_modifier |= GRAPHICS_BUFFER_INPUT_MODIFIER_BUTTON1;
		}
		input.input_modifier = static_cast<enum Graphics_buffer_input_modifier>
			(input_modifier);
		if (return_code)
		{
			CMZN_CALLBACK_LIST_CALL(Graphics_buffer_input_callback)(
				graphics_buffer->input_callback_list, graphics_buffer, &input);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_buffer_gtkglarea_button_callback.  Invalid argument(s)");
	}
	LEAVE;

	return(TRUE);
} /* Graphics_buffer_gtkglarea_button_callback */
#endif /* defined (GTK_USER_INTERFACE) */

#if defined (WIN32_USER_INTERFACE)
static int Graphics_buffer_win32_button_callback(
	unsigned int *button_event, struct Graphics_buffer *graphics_buffer,
	WPARAM wParam, LPARAM lParam)
/*******************************************************************************
LAST MODIFIED : 11 July 2002

DESCRIPTION :
The callback for mouse button input in the graphics_buffer window. The
resulting behaviour depends on the <graphics_buffer> input_mode. In Transform mode
mouse clicks and drags are converted to transformation; in Select mode OpenGL
picking is performed with picked objects and mouse click and drag information
returned to the scene.
==============================================================================*/
{
	int input_modifier, return_code;
	struct Graphics_buffer_input input;

	ENTER(Graphics_buffer_win32_button_callback);
	sharedContext
	return_code = 1;
	input.type = GRAPHICS_BUFFER_INVALID_INPUT;
	switch(*button_event)
	{
		case WM_LBUTTONDOWN:
		{
			input.button_number = 1;
			input.type = GRAPHICS_BUFFER_BUTTON_PRESS;
		} break;
		case WM_LBUTTONUP:
		{
			input.button_number = 1;
			input.type = GRAPHICS_BUFFER_BUTTON_RELEASE;
		} break;
		case WM_RBUTTONDOWN:
		{
			input.button_number = 3;
			input.type = GRAPHICS_BUFFER_BUTTON_PRESS;
		} break;
		case WM_RBUTTONUP:
		{
			input.button_number = 3;
			input.type = GRAPHICS_BUFFER_BUTTON_RELEASE;
		} break;
		case WM_MBUTTONDOWN:
		{
			input.button_number = 2;
			input.type = GRAPHICS_BUFFER_BUTTON_PRESS;
		} break;
		case WM_MBUTTONUP:
		{
			input.button_number = 2;
			input.type = GRAPHICS_BUFFER_BUTTON_RELEASE;
		} break;
		case WM_MOUSEMOVE:
		{
			input.button_number = 0;
			input.type = GRAPHICS_BUFFER_MOTION_NOTIFY;
		} break;
		default:
		{
			display_message(ERROR_MESSAGE,
				"Graphics_buffer_button_button_callback.  Unknown button event");
			return_code=0;
			/* This event type is not being passed on */
		} break;
	}
	input.key_code = 0;
	input.position_x = GET_X_LPARAM(lParam);
	input.position_y = GET_Y_LPARAM(lParam);
	input_modifier = 0;
	if (MK_SHIFT & wParam)
	{
		input_modifier |= GRAPHICS_BUFFER_INPUT_MODIFIER_SHIFT;
	}
	if (MK_CONTROL & wParam)
	{
		input_modifier |= GRAPHICS_BUFFER_INPUT_MODIFIER_CONTROL;
	}
	if (GetKeyState(VK_MENU) < 0)
	{
		input_modifier |= GRAPHICS_BUFFER_INPUT_MODIFIER_ALT;
	}
/*
	if (MK_XBUTTON1 == wParam)
	{
		input_modifier |= GRAPHICS_BUFFER_INPUT_MODIFIER_BUTTON1;
	}
*/
	input.input_modifier = static_cast<enum Graphics_buffer_input_modifier>
		(input_modifier);
	if (return_code)
	{
		CMZN_CALLBACK_LIST_CALL(Graphics_buffer_input_callback)(
			graphics_buffer->input_callback_list, graphics_buffer, &input);
	}
	LEAVE;

	return(TRUE);
} /* Graphics_buffer_win32_button_callback */
#endif /* defined (WIN32_USER_INTERFACE) */

#if defined (GTK_USER_INTERFACE)
static gboolean Graphics_buffer_gtkglarea_key_callback(GtkWidget *widget,
	GdkEventKey *key_event, gpointer graphics_buffer_structure)
/*******************************************************************************
LAST MODIFIED : 11 July 2002

DESCRIPTION :
The callback for key input in the graphics_buffer window. The
resulting behaviour depends on the <graphics_buffer> input_mode. In Transform mode
mouse clicks and drags are converted to transformation; in Select mode OpenGL
picking is performed with picked objects and mouse click and drag information
returned to the scene.
==============================================================================*/
{
	int input_modifier, return_code;
	struct Graphics_buffer *graphics_buffer;
	struct Graphics_buffer_input input;

	ENTER(Graphics_buffer_gtkglarea_key_callback);
	USE_PARAMETER(widget);
	if ((graphics_buffer=(struct Graphics_buffer *)graphics_buffer_structure)
		&& key_event)
	{
		return_code = 1;
		input.type = GRAPHICS_BUFFER_INVALID_INPUT;
		switch(key_event->type)
		{
			case GDK_KEY_PRESS:
			{
				input.type = GRAPHICS_BUFFER_KEY_PRESS;
			} break;
			case GDK_KEY_RELEASE:
			{
				input.type = GRAPHICS_BUFFER_KEY_RELEASE;
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"Graphics_buffer_gtkglarea_key_callback.  Unknown key event");
				return_code=0;
				/* This event type is not being passed on */
			} break;
		}
		input.button_number = 0;
		input.key_code = 0;
		input.position_x = 0;
		input.position_y = 0;
		input_modifier = (enum Graphics_buffer_input_modifier)0;
		if (GDK_SHIFT_MASK&(key_event->state))
		{
			input_modifier |= GRAPHICS_BUFFER_INPUT_MODIFIER_SHIFT;
		}
		if (GDK_CONTROL_MASK&(key_event->state))
		{
			input_modifier |= GRAPHICS_BUFFER_INPUT_MODIFIER_CONTROL;
		}
		if (GDK_MOD1_MASK&(key_event->state))
		{
			input_modifier |= GRAPHICS_BUFFER_INPUT_MODIFIER_ALT;
		}
		if (GDK_BUTTON1_MASK&(key_event->state))
		{
			input_modifier |= GRAPHICS_BUFFER_INPUT_MODIFIER_BUTTON1;
		}
		if (return_code)
		{
			CMZN_CALLBACK_LIST_CALL(Graphics_buffer_input_callback)(
				graphics_buffer->input_callback_list, graphics_buffer, &input);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_buffer_gtkglarea_key_callback.  Invalid argument(s)");
	}
	LEAVE;

	return(TRUE);
} /* Graphics_buffer_gtkglarea_key_callback */
#endif /* defined (GTK_USER_INTERFACE) */

#if defined (GTK_USER_INTERFACE)
static gboolean Graphics_buffer_gtkglarea_motion_notify_callback(
	GtkWidget *widget,
	GdkEventMotion *motion_event, gpointer graphics_buffer_structure)
/*******************************************************************************
LAST MODIFIED : 11 July 2002

DESCRIPTION :
The callback for mouse button input in the graphics_buffer window. The
resulting behaviour depends on the <graphics_buffer> input_mode. In Transform mode
mouse clicks and drags are converted to transformation; in Select mode OpenGL
picking is performed with picked objects and mouse click and drag information
returned to the scene.
==============================================================================*/
{
	GdkModifierType state;
	int input_modifier, return_code;
	struct Graphics_buffer *graphics_buffer;
	struct Graphics_buffer_input input;

	ENTER(Graphics_buffer_gtkglarea_motion_notify_callback);
	USE_PARAMETER(widget);
	if ((graphics_buffer=(struct Graphics_buffer *)graphics_buffer_structure)
		&& motion_event)
	{
		return_code = 1;
		input.button_number = 0;
		input.type = GRAPHICS_BUFFER_MOTION_NOTIFY;
		input.key_code = 0;
		if (motion_event->is_hint)
		{
			gdk_window_get_pointer(motion_event->window, &input.position_x,
				&input.position_y, &state);
		}
		else
		{
			input.position_x = static_cast<int>(motion_event->x);
			input.position_y = static_cast<int>(motion_event->y);
			state = static_cast<GdkModifierType>(motion_event->state);
		}
		input_modifier = 0;
		if (GDK_SHIFT_MASK&(state))
		{
			input_modifier |= GRAPHICS_BUFFER_INPUT_MODIFIER_SHIFT;
		}
		if (GDK_CONTROL_MASK&(state))
		{
			input_modifier |= GRAPHICS_BUFFER_INPUT_MODIFIER_CONTROL;
		}
		if (GDK_MOD1_MASK&(state))
		{
			input_modifier |= GRAPHICS_BUFFER_INPUT_MODIFIER_ALT;
		}
		if (GDK_BUTTON1_MASK&(state))
		{
			input_modifier |= GRAPHICS_BUFFER_INPUT_MODIFIER_BUTTON1;
		}
		input.input_modifier = static_cast<enum Graphics_buffer_input_modifier>
			(input_modifier);
		if (return_code)
		{
			CMZN_CALLBACK_LIST_CALL(Graphics_buffer_input_callback)(
				graphics_buffer->input_callback_list, graphics_buffer, &input);
		}

	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_buffer_gtkglarea_motion_notify_callback.  Invalid argument(s)");
	}
	LEAVE;

	return(TRUE);
} /* Graphics_buffer_gtkglarea_motion_notify_callback */
#endif /* defined (GTK_USER_INTERFACE) */

/*
Global functions
-
*/

struct Graphics_buffer_app_package *CREATE(Graphics_buffer_app_package)(
	struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 6 May 2004

DESCRIPTION :
Creates a Graphics_buffer_package which enables Graphics_buffers created from
it to share graphics contexts.
==============================================================================*/
{
	struct Graphics_buffer_app_package *package;

	USE_PARAMETER(user_interface);
	if (ALLOCATE(package, struct Graphics_buffer_app_package, 1))
	{
		package->core_package = CREATE(Graphics_buffer_package)();

#if defined (GTK_USER_INTERFACE)
#  if defined (GTK_USE_GTKGLAREA)
		package->share_glarea = (GtkWidget *)NULL;
#  else /* defined (GTK_USE_GTKGLAREA) */
		package->share_glcontext = (GdkGLContext *)NULL;
#  endif /* defined (GTK_USE_GTKGLAREA) */
#endif /* defined (GTK_USER_INTERFACE) */
#if defined (WX_USER_INTERFACE)
		package->wxSharedCanvas = NULL;
		package->sharedFrame = (wxFrame*)NULL;
#endif /* defined (WX_USER_INTERFACE) */
#if defined (WIN32_USER_INTERFACE)
		package->user_interface = user_interface;
		package->wgl_shared_context = (HGLRC)NULL;
		package->hidden_accelerated_window = (HWND)NULL;
		package->hidden_graphics_buffer = (Graphics_buffer *)NULL;
		package->pbuffer_support_available = GRAPHICS_BUFFER_PBUFFER_SUPPORT_UNKNOWN;
		package->hidden_graphics_package = (Graphics_buffer_package *)NULL;
#if defined (WIN32_SYSTEM)
		char env_buffer[1024];
		char *cmiss_intel_single_context_force_clipping = NULL;
		int force_clipping_int;
		if (GetEnvironmentVariable("CMZN_INTEL_SINGLE_CONTEXT_FORCE_CLIPPING",
			env_buffer, sizeof(env_buffer))
			&& (cmiss_intel_single_context_force_clipping = env_buffer))
#else /* defined (WIN32_SYSTEM) */
		cmiss_intel_single_context_force_clipping = getenv("CMZN_INTEL_SINGLE_CONTEXT_FORCE_CLIPPING");
		if (cmiss_intel_single_context_force_clipping)
#endif /* defined (WIN32_SYSTEM) */
		{
			if (sscanf(cmiss_intel_single_context_force_clipping, "%d", &force_clipping_int))
			{
				package->intel_single_context_force_clipping = force_clipping_int;
			}
			else
				package->intel_single_context_force_clipping = 0;
		}
		else
			package->intel_single_context_force_clipping = 0;
#endif /* defined (WIN32_USER_INTERFACE) */
	}
	else
	{
		display_message(ERROR_MESSAGE,"CREATE(Graphics_buffer_package). "
			"Unable to allocate package structure");
		package = 0;
	}

	LEAVE;
	return (package);
} /* CREATE(Graphics_buffer_package) */

int DESTROY(Graphics_buffer_app_package)(struct Graphics_buffer_app_package **package_ptr)
/*******************************************************************************
LAST MODIFIED : 6 May 2004

DESCRIPTION :
Closes the Graphics buffer package
==============================================================================*/
{
	int return_code;
	struct Graphics_buffer_app_package *package;

	if (package_ptr && (package = *package_ptr))
	{
		return_code=1;
#if defined (GTK_USER_INTERFACE)
#  if ! defined (GTK_USE_GTKGLAREA)
		if (package->share_glcontext)
		{
			gdk_gl_context_destroy(package->share_glcontext);
		}
#  endif /* ! defined (GTK_USE_GTKGLAREA) */
#endif /* defined (GTK_USER_INTERFACE) */
#if defined (WIN32_USER_INTERFACE)
		if (package->hidden_graphics_buffer)
		{
			DEACCESS(Graphics_buffer)(&package->hidden_graphics_buffer);
		}
		if (package->hidden_accelerated_window)
		{
			DestroyWindow(package->hidden_accelerated_window);
		}
		/* Destroy the shared_glx_context as we did not destroy it when closing
			it's buffer */
		if (package->wgl_shared_context)
		{
			void Set_wx_SharedContext()
			{
				if (!sharedCanvas)
				{
					graphics_buffer->package->wxSharedCanvas = this;
				}
			}
			wglDeleteContext(package->wgl_shared_context);
		}
		if (package->hidden_graphics_package)
		{
			DESTROY(Graphics_buffer_package)(&package->hidden_graphics_package);
		}
#endif /* defined (WIN32_USER_INTERFACE) */
#if defined (WX_USER_INTERFACE)
	if (package->sharedFrame)
	{
		delete package->sharedFrame;
		package->sharedFrame = 0;
	}
#endif /* defined (WX_USER_INTERFACE) */

		DESTROY(Graphics_buffer_package)(&(package->core_package));
		DEALLOCATE(*package_ptr);
		*package_ptr = 0;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(Graphics_buffer_package).  Missing package");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Graphics_buffer_package) */

int Graphics_buffer_package_set_override_visual_id(
	struct Graphics_buffer_package *graphics_buffer_package,
	int override_visual_id)
/*******************************************************************************
LAST MODIFIED : 21 May 2004

DESCRIPTION :
Sets a particular visual to be used by all graphics buffers.
==============================================================================*/
{
	int return_code;

	ENTER(Graphics_buffer_package_set_override_visual_id);
	if (graphics_buffer_package)
	{
		graphics_buffer_package->override_visual_id = override_visual_id;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"Graphics_buffer_package_set_override_visual_id.  "
			"Invalid argument");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Graphics_buffer_package_set_override_visual_id */

struct Graphics_buffer_app *create_Graphics_buffer_offscreen(Graphics_buffer_app_package *graphics_buffer_package,
	int width, int height,
	enum Graphics_buffer_buffering_mode buffering_mode,
	enum Graphics_buffer_stereo_mode stereo_mode,
	int minimum_colour_buffer_depth, int minimum_depth_buffer_depth,
	int minimum_accumulation_buffer_depth)
/*******************************************************************************
LAST MODIFIED : 6 May 2004

DESCRIPTION :
==============================================================================*/
{
	struct Graphics_buffer_app *buffer;

	ENTER(create_Graphics_buffer_offscreen);

	buffer = CREATE(Graphics_buffer_app)(graphics_buffer_package,
		GRAPHICS_BUFFER_INVALID_TYPE, buffering_mode, stereo_mode);
	if (buffer != NULL)
	{
		USE_PARAMETER(width);
		USE_PARAMETER(height);
		USE_PARAMETER(buffering_mode);
		USE_PARAMETER(stereo_mode);
		USE_PARAMETER(minimum_colour_buffer_depth);
		USE_PARAMETER(minimum_depth_buffer_depth);
		USE_PARAMETER(minimum_accumulation_buffer_depth);
		if (buffer->core_buffer->type == GRAPHICS_BUFFER_INVALID_TYPE)
		{
#if defined (DEBUG_CODE)
			display_message(ERROR_MESSAGE,"create_Graphics_buffer_offscreen.  "
				"Unable to create offscreen graphics buffer.");
#endif /* defined (DEBUG_CODE) */
			DESTROY(Graphics_buffer_app)(&buffer);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"create_Graphics_buffer_offscreen.  "
			"Unable to create generic Graphics_buffer.");
		buffer = 0;
	}
	LEAVE;

	return (buffer);
} /* create_Graphics_buffer_offscreen */

struct Graphics_buffer_app *create_Graphics_buffer_shared_offscreen(Graphics_buffer_app_package *graphics_buffer_package,
	int width, int height,
	enum Graphics_buffer_buffering_mode buffering_mode,
	enum Graphics_buffer_stereo_mode stereo_mode,
	int minimum_colour_buffer_depth, int minimum_depth_buffer_depth,
	int minimum_accumulation_buffer_depth)
/*******************************************************************************
LAST MODIFIED : 6 May 2004

DESCRIPTION :
==============================================================================*/
{
	struct Graphics_buffer_app *buffer;

	ENTER(create_Graphics_buffer_offscreen);

	buffer = CREATE(Graphics_buffer_app)(graphics_buffer_package,
		GRAPHICS_BUFFER_INVALID_TYPE, buffering_mode, stereo_mode);
	if (buffer != NULL)
	{
		USE_PARAMETER(width);
		USE_PARAMETER(height);
		USE_PARAMETER(buffering_mode);
		USE_PARAMETER(stereo_mode);
		USE_PARAMETER(minimum_colour_buffer_depth);
		USE_PARAMETER(minimum_depth_buffer_depth);
		USE_PARAMETER(minimum_accumulation_buffer_depth);
		if (buffer->core_buffer->type == GRAPHICS_BUFFER_INVALID_TYPE)
		{
#if defined (DEBUG_CODE)
			display_message(ERROR_MESSAGE,"create_Graphics_buffer_offscreen.  "
				"Unable to create offscreen graphics buffer.");
#endif /* defined (DEBUG_CODE) */
			DESTROY(Graphics_buffer_app)(&buffer);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"create_Graphics_buffer_offscreen.  "
			"Unable to create generic Graphics_buffer.");
		buffer = 0;
	}
	LEAVE;

	return (buffer);
} /* create_Graphics_buffer_offscreen */

struct Graphics_buffer_app *create_Graphics_buffer_offscreen_from_buffer(
	int width, int height, struct Graphics_buffer_app *buffer_to_match)
/*******************************************************************************
LAST MODIFIED : 6 May 2004

DESCRIPTION :
==============================================================================*/
{
	struct Graphics_buffer_app *buffer;

	ENTER(create_Graphics_buffer_offscreen_from_buffer);

	buffer = CREATE(Graphics_buffer_app)(buffer_to_match->package,
		buffer_to_match->core_buffer->type, buffer_to_match->core_buffer->buffering_mode, buffer_to_match->core_buffer->stereo_mode);
	if (buffer != NULL)
	{
#if defined (WX_USER_INTERFACE)
		buffer->core_buffer->type = GRAPHICS_BUFFER_WX_OFFSCREEN_TYPE;
#if defined (OPENGL_API) && (GL_EXT_framebuffer_object)
		if (Graphics_library_load_extension("GL_EXT_framebuffer_object"))
		{
			buffer->core_buffer->type = GRAPHICS_BUFFER_GL_EXT_FRAMEBUFFER_TYPE;
		}
#endif
		Graphics_buffer_create_buffer_wx(buffer, buffer_to_match->package,
			NULL, GRAPHICS_BUFFER_ANY_BUFFERING_MODE,
			GRAPHICS_BUFFER_ANY_STEREO_MODE,
			0, 0, 0, width, height,
			buffer_to_match);
#else /* defined (WX_USER_INTERFACE) */
		USE_PARAMETER(width);
		USE_PARAMETER(height);
		USE_PARAMETER(buffer_to_match);
#endif /* defined (WX_USER_INTERFACE) */
		if (buffer->core_buffer->type == GRAPHICS_BUFFER_INVALID_TYPE)
		{
#if defined (DEBUG_CODE)
			display_message(ERROR_MESSAGE,"create_Graphics_buffer_offscreen_from_buffer.  "
				"Unable to create offscreen_from_buffer graphics buffer.");
			buffer = (struct Graphics_buffer_app *)NULL;
#endif /* defined (DEBUG_CODE) */
			DESTROY(Graphics_buffer_app)(&buffer);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"create_Graphics_buffer_offscreen_from_buffer.  "
			"Unable to create generic Graphics_buffer.");
		buffer = (struct Graphics_buffer_app *)NULL;
	}
	LEAVE;

	return (buffer);
} /* create_Graphics_buffer_offscreen_from_buffer */

#if defined (ENABLE_GTK_PBUFFER_RENDERING)
#if defined (GTK_USER_INTERFACE)
static int Graphics_buffer_gtk_reallocate_offscreen_size(
	struct Graphics_buffer *buffer)
/*******************************************************************************
LAST MODIFIED : 10 March 2008

DESCRIPTION :
Resizes the offscreen pbuffer used for rendering with windowless mode.
==============================================================================*/
{
	int return_code;

	ENTER(Graphics_buffer_gtk_reallocate_pbuffer_size);
	if (buffer)
	{
		return_code = 0;
		/* We never bother to reduce the size */
		if (!buffer->offscreen_width || (buffer->offscreen_width < buffer->width)
			|| !buffer->offscreen_height || (buffer->offscreen_height < buffer->height))
		{
			/* 256x256 is the current minimum. Also allocate powers of two just to be
				conservative with graphics card drivers,
				and ensure we are using 4 byte aligned buffers */
			int required_width = 256;
			int required_height = 256;
			while (required_width < buffer->width)
			{
				required_width *= 2;
			}
			while (required_height < buffer->height)
			{
				required_height *= 2;
			}

#if defined (GL_EXT_framebuffer_object)
			if (Graphics_library_check_extension(GL_EXT_framebuffer_object))
			{

			GenRenderbuffersEXT(1, &buffer->framebuffer_object);

			// Enable render-to-texture
			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, buffer->framebuffer_object);

			// Set up color_tex and depth_rb for render-to-texture
			glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
									  GL_COLOR_ATTACHMENT0_EXT,
									  GL_TEXTURE_2D, color_tex, 0);
			glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT,
										 GL_DEPTH_ATTACHMENT_EXT,
										 GL_RENDERBUFFER_EXT, depth_rb);

			// Check framebuffer completeness at the end of initialization.
			CHECK_FRAMEBUFFER_STATUS();

			<draw to the texture and renderbuffer>

			// Re-enable rendering to the window
			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);



					/* Should be selecting with our standard rules and minimums here */
					int colour_bits = 32;
					int alpha_bits = 8;
					int depth_bits = 24;
					GLint pixel_format;
					unsigned int number_of_formats;

					const int pbuffer_attributes[]=
						{WGL_DRAW_TO_PBUFFER_ARB, 1,
						 WGL_COLOR_BITS_ARB, colour_bits,
						 WGL_ALPHA_BITS_ARB, alpha_bits,
						 WGL_DEPTH_BITS_ARB, depth_bits,
						 0};
					const float float_pbuffer_attributes[]={
						0};

#if defined (DEBUG_CODE)
					printf("Trying pbuffer\n");
#endif /* defined (DEBUG_CODE) */

					/* Only get the first valid format */
					if(wglChoosePixelFormatARB(buffer->package->hidden_graphics_buffer->hDC,
							pbuffer_attributes, float_pbuffer_attributes, 1,
							&pixel_format, &number_of_formats))
					{
						const int pbuffer_attrib[] = {0};

						if (buffer->pbuffer=wglCreatePbufferARB(
								 buffer->package->hidden_graphics_buffer->hDC, pixel_format,
								 required_width, required_height, pbuffer_attrib))
						{
							if (buffer->hDC = wglGetPbufferDCARB(buffer->pbuffer))
							{
								buffer->type = GRAPHICS_BUFFER_WIN32_COPY_PBUFFER_TYPE;
								buffer->pixel_format = pixel_format;
								return_code = 1;
							}
							else
							{
								return_code = 0;
							}
						}
						else
						{
							return_code = 0;
						}
					}
					else
					{
						return_code = 0;
					}
				}
				else
				{
					/* Try non accelerated bitmap OpenGL instead */
					return_code = 1;
				}
			}
#endif /* defined (WGL_ARB_pixel_format) && (WGL_ARB_pbuffer) */
			/* In either case we need a device independent bitmap matching the
				onscreen hdc.  Either for copying the pbuffer pixels or for rendering
				directly if we cannot get a pbuffer. */
			if (buffer->device_independent_bitmap)
			{
				DeleteObject(buffer->device_independent_bitmap);
			}
			if (buffer->device_independent_bitmap_hdc)
			{
				DeleteDC(buffer->device_independent_bitmap_hdc);
			}
			{
				BITMAPINFO bmi;

				bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
				bmi.bmiHeader.biWidth = required_width;
				bmi.bmiHeader.biHeight= required_height;
				bmi.bmiHeader.biPlanes = 1;
				if (GRAPHICS_BUFFER_RENDER_OFFSCREEN_AND_BLEND == buffer->buffering_mode)
				{
					bmi.bmiHeader.biBitCount = 32;
				}
				else
				{
					bmi.bmiHeader.biBitCount = 24;
				}
				if (GRAPHICS_BUFFER_WIN32_COPY_PBUFFER_TYPE != buffer->type)
				{
					/* If the onscreen size is 32 then the setpixelformat seems to
						want this even if rendering without alpha */
					int onscreen_bits = GetDeviceCaps(onscreen_hdc, BITSPIXEL);
					if (bmi.bmiHeader.biBitCount < onscreen_bits)
					{
						bmi.bmiHeader.biBitCount = onscreen_bits;
					}
				}

#if defined (DEBUG_CODE)
				printf("Bitmap bit count %d\n", bmi.bmiHeader.biBitCount);
#endif /* defined (DEBUG_CODE) */

				bmi.bmiHeader.biCompression = BI_RGB;
				bmi.bmiHeader.biSizeImage = 0;
				bmi.bmiHeader.biXPelsPerMeter = 0;
				bmi.bmiHeader.biYPelsPerMeter = 0;
				bmi.bmiHeader.biClrUsed = 0;
				bmi.bmiHeader.biClrImportant = 0;

				buffer->device_independent_bitmap_hdc = CreateCompatibleDC(onscreen_hdc);

				buffer->device_independent_bitmap =
					CreateDIBSection(onscreen_hdc,
						&bmi,
						DIB_RGB_COLORS,
						(void **)&buffer->device_independent_bitmap_pixels,
						0,
						0);
				SelectObject(buffer->device_independent_bitmap_hdc,
					buffer->device_independent_bitmap);

#if defined (DEBUG_CODE)
				printf ("Made dib\n");
#endif /* defined (DEBUG_CODE) */

				return_code = 1;

				if (buffer->type != GRAPHICS_BUFFER_WIN32_COPY_PBUFFER_TYPE)
				{
					buffer->type = GRAPHICS_BUFFER_WIN32_COPY_BITMAP_TYPE;
					/* We use the bitmap directly as the OpenGL rendering surface */
					buffer->hDC = buffer->device_independent_bitmap_hdc;

					PIXELFORMATDESCRIPTOR pfd;
					SetPixelFormat( buffer->hDC, buffer->pixel_format, &pfd );
					buffer->hRC = wglCreateContext( buffer->hDC );
					if(!wglMakeCurrent(buffer->hDC,buffer->hRC))
					{
						display_message(ERROR_MESSAGE,"Graphics_buffer_gtk_reallocate_pbuffer_size.  "
							"Bitmap make current failed");
						return_code = 0;
					}
				}

			}
			if (return_code)
			{
				buffer->offscreen_width = required_width;
				buffer->offscreen_height = required_height;
				buffer->offscreen_render_required = 1;
			}
		}
		else
		{
			/* Already large enough so nothing to do */
			return_code = 1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Graphics_buffer_gtk_reallocate_pbuffer_size.  "
			"Missing graphics_buffer parameter.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Graphics_buffer_gtk_reallocate_pbuffer_size */
#endif /* defined (GTK_USER_INTERFACE) */
#endif /* defined (ENABLE_GTK_PBUFFER_RENDERING) */

#if defined (GTK_USER_INTERFACE)
#if defined (GTK_USE_GTKGLAREA)
struct Graphics_buffer *create_Graphics_buffer_gtkgl(
	struct Graphics_buffer_package *graphics_buffer_package,
	GtkContainer *parent,
	enum Graphics_buffer_buffering_mode buffering_mode,
	enum Graphics_buffer_stereo_mode stereo_mode,
	int minimum_colour_buffer_depth, int minimum_depth_buffer_depth,
	int minimum_accumulation_buffer_depth)
/*******************************************************************************
LAST MODIFIED : 2 June 2004

DESCRIPTION :
==============================================================================*/
{
#define MAX_GL_ATTRIBUTES (50)
	GtkGLArea *share;
	int accumulation_colour_size, attribute_list[MAX_GL_ATTRIBUTES], *attribute_ptr;
	struct Graphics_buffer *buffer;

	ENTER(create_Graphics_buffer_gtkglarea);
	if (gdk_gl_query() == TRUE)
	{
		if (buffer = CREATE(Graphics_buffer)(graphics_buffer_package))
		{
			if (parent)
			{
				attribute_ptr = attribute_list;

				*attribute_ptr = GDK_GL_RGBA;
				attribute_ptr++;
				if (buffering_mode == GRAPHICS_BUFFER_DOUBLE_BUFFERING)
				{
					*attribute_ptr = GDK_GL_DOUBLEBUFFER;
					attribute_ptr++;
					buffer->buffering_mode = GRAPHICS_BUFFER_DOUBLE_BUFFERING;
				}
				else
				{
					/* GRAPHICS_BUFFER_ANY_BUFFERING_MODE so don't specify it */
					buffer->buffering_mode = GRAPHICS_BUFFER_SINGLE_BUFFERING;
				}
			case GDK_BUTTON_PRESS:
				if (stereo_mode == GRAPHICS_BUFFER_STEREO)
				{
					*attribute_ptr = GDK_GL_STEREO;
					attribute_ptr++;
					buffer->stereo_mode = GRAPHICS_BUFFER_STEREO;
				}
				else
				{
					/* GRAPHICS_BUFFER_ANY_STEREO_MODE so don't specify it */
					buffer->stereo_mode = GRAPHICS_BUFFER_MONO;
				}
				if (minimum_colour_buffer_depth)
				{
					*attribute_ptr = GDK_GL_BUFFER_SIZE;
					attribute_ptr++;
					*attribute_ptr = minimum_colour_buffer_depth;
					attribute_ptr++;
				}
				if (minimum_depth_buffer_depth)
				{
					*attribute_ptr = GDK_GL_DEPTH_SIZE;
					attribute_ptr++;
					*attribute_ptr = minimum_depth_buffer_depth;
					attribute_ptr++;
				}
				if (minimum_accumulation_buffer_depth)
				{
					accumulation_colour_size = minimum_accumulation_buffer_depth / 4;
					*attribute_ptr = GDK_GL_ACCUM_RED_SIZE;
					attribute_ptr++;
					*attribute_ptr = accumulation_colour_size;
					attribute_ptr++;
					*attribute_ptr = GDK_GL_ACCUM_GREEN_SIZE;
					attribute_ptr++;
					*attribute_ptr = accumulation_colour_size;
					attribute_ptr++;
					*attribute_ptr = GDK_GL_ACCUM_BLUE_SIZE;
					attribute_ptr++;
					*attribute_ptr = accumulation_colour_size;
					attribute_ptr++;
					*attribute_ptr = GDK_GL_ACCUM_ALPHA_SIZE;
					attribute_ptr++;
					*attribute_ptr = accumulation_colour_size;
					attribute_ptr++;
				}
				*attribute_ptr = GDK_GL_NONE;
				attribute_ptr++;
				if (graphics_buffer_package->share_glarea)
				{
					share = GTK_GL_AREA(graphics_buffer_package->share_glarea);
				}
				else
				{
					share = (GtkGLArea *)NULL;
				}
				if (buffer->glarea = gtk_gl_area_share_new(attribute_list, share))
				{
					if (!graphics_buffer_package->share_glarea)
					{
						graphics_buffer_package->share_glarea = buffer->glarea;
					}
					buffer->type = GRAPHICS_BUFFER_GTKGLAREA_TYPE;
					gtk_widget_set_events(GTK_WIDGET(buffer->glarea),
						GDK_EXPOSURE_MASK|GDK_POINTER_MOTION_MASK|GDK_POINTER_MOTION_HINT_MASK|
						GDK_BUTTON_PRESS_MASK|GDK_BUTTON_RELEASE_MASK|
						GDK_KEY_PRESS_MASK|GDK_KEY_RELEASE_MASK);
#if GTK_MAJOR_VERSION >= 2
					g_signal_connect(G_OBJECT(buffer->glarea), "realize",
						G_CALLBACK(Graphics_buffer_gtkglarea_initialise_callback),
						(gpointer)buffer);
					g_signal_connect(G_OBJECT(buffer->glarea), "size-allocate",
						G_CALLBACK(Graphics_buffer_gtkglarea_resize_callback),
						(gpointer)buffer);
					g_signal_connect(G_OBJECT(buffer->glarea), "expose-event",
						G_CALLBACK(Graphics_buffer_gtkglarea_expose_callback),
						(gpointer)buffer);
					g_signal_connect(G_OBJECT(buffer->glarea), "button-press-event",
						G_CALLBACK(Graphics_buffer_gtkglarea_button_callback),
						(gpointer)buffer);
					g_signal_connect(G_OBJECT(buffer->glarea), "button-release-event",
						G_CALLBACK(Graphics_buffer_gtkglarea_button_callback),
						(gpointer)buffer);
					g_signal_connect(G_OBJECT(buffer->glarea), "key-press-event",
						G_CALLBACK(Graphics_buffer_gtkglarea_key_callback),
						(gpointer)buffer);
					g_signal_connect(G_OBJECT(buffer->glarea), "key-release-event",
						G_CALLBACK(Graphics_buffer_gtkglarea_key_callback),
						(gpointer)buffer);
					g_signal_connect(G_OBJECT(buffer->glarea), "motion-notify-event",
						G_CALLBACK(Graphics_buffer_gtkglarea_motion_notify_callback),
						(gpointer)buffer);
#else /* GTK_MAJOR_VERSION >= 2 */
					gtk_signal_connect(GTK_OBJECT(buffer->glarea), "realize",
						GTK_SIGNAL_FUNC(Graphics_buffer_gtkglarea_initialise_callback),
						(gpointer)buffer);
					gtk_signal_connect(GTK_OBJECT(buffer->glarea), "size-allocate",
						GTK_SIGNAL_FUNC(Graphics_buffer_gtkglarea_resize_callback),
						(gpointer)buffer);
					gtk_signal_connect(GTK_OBJECT(buffer->glarea), "expose-event",
						GTK_SIGNAL_FUNC(Graphics_buffer_gtkglarea_expose_callback),
						(gpointer)buffer);
					gtk_signal_connect(GTK_OBJECT(buffer->glarea), "button-press-event",
						GTK_SIGNAL_FUNC(Graphics_buffer_gtkglarea_button_callback),
						(gpointer)buffer);
					gtk_signal_connect(GTK_OBJECT(buffer->glarea), "button-release-event",
						GTK_SIGNAL_FUNC(Graphics_buffer_gtkglarea_button_callback),
						(gpointer)buffer);
					gtk_signal_connect(GTK_OBJECT(buffer->glarea), "key-press-event",
						GTK_SIGNAL_FUNC(Graphics_buffer_gtkglarea_key_callback),
						(gpointer)buffer);
					gtk_signal_connect(GTK_OBJECT(buffer->glarea), "key-release-event",
						GTK_SIGNAL_FUNC(Graphics_buffer_gtkglarea_key_callback),
						(gpointer)buffer);
					gtk_signal_connect(GTK_OBJECT(buffer->glarea), "motion-notify-event",
						GTK_SIGNAL_FUNC(Graphics_buffer_gtkglarea_motion_notify_callback),
						(gpointer)buffer);
#endif /* GTK_MAJOR_VERSION >= 2 */
					gtk_container_add(parent, GTK_WIDGET(buffer->glarea));
				}
				else
				{
					display_message(ERROR_MESSAGE,"create_Graphics_buffer_gtkglarea.  "
						"Unable to create gtk gl area.");
					DEACCESS(Graphics_buffer)(&buffer);
					buffer = (struct Graphics_buffer *)NULL;
				}
			}
#if defined (ENABLE_GTK_PBUFFER_RENDERING)
			else if ((GRAPHICS_BUFFER_RENDER_OFFSCREEN_AND_COPY == buffering_mode) ||
				(GRAPHICS_BUFFER_RENDER_OFFSCREEN_AND_BLEND == buffering_mode))
			{
				buffer->minimum_colour_buffer_depth = minimum_colour_buffer_depth;
				buffer->minimum_depth_buffer_depth = minimum_depth_buffer_depth;
				buffer->minimum_accumulation_buffer_depth = minimum_accumulation_buffer_depth;

				buffer->buffering_mode = buffering_mode;

				/* If we don't use the pbuffer below we need a SINGLE BUFFERING OpenGL
					when we search pixel formats */
				buffering_mode = GRAPHICS_BUFFER_SINGLE_BUFFERING;
				if (Graphics_buffer_gtk_reallocate_offscreen_size(buffer))
				{
					if (GRAPHICS_BUFFER_GTK_COPY_PBUFFER_TYPE == buffer->type)
					{
	#if defined (DEBUG_CODE)
						printf("Using pbuffer\n");
	#endif /* defined (DEBUG_CODE) */
						use_pbuffer = 1;
					}
					else
					{
						/* Try to get some alpha planes although I think they don't work anyway */
						minimum_alpha_buffer_depth = 1;
					}
				}
				else
				{
					DEACCESS(Graphics_buffer)(&buffer);
					buffer = (struct Graphics_buffer *)NULL;
				}

			}
#endif /* defined (ENABLE_GTK_PBUFFER_RENDERING) */
			else
			{
				display_message(ERROR_MESSAGE,"create_Graphics_buffer_gtkglarea.  "
					"Missing gtk container and not an render and copy display mode.");
				buffer = (struct Graphics_buffer *)NULL;

			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"create_Graphics_buffer_gtkglarea.  "
				"Unable to create generic Graphics_buffer.");
			buffer = (struct Graphics_buffer *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"create_Graphics_buffer_gtkglarea.  "
			"Gdk Open GL not supported.");
		buffer = (struct Graphics_buffer *)NULL;
	}
	LEAVE;

	return (buffer);
} /* create_Graphics_buffer_gtkglarea */
#endif /* defined (GTK_USE_GTKGLAREA) */
#endif /* defined (GTK_USER_INTERFACE) */

#if defined (GTK_USER_INTERFACE)
#if ! defined (GTK_USE_GTKGLAREA)
struct Graphics_buffer *create_Graphics_buffer_gtkgl(
	struct Graphics_buffer_package *graphics_buffer_package,
	GtkContainer *parent,
	enum Graphics_buffer_buffering_mode buffering_mode,
	enum Graphics_buffer_stereo_mode stereo_mode,
	int minimum_colour_buffer_depth, int minimum_depth_buffer_depth,
	int minimum_accumulation_buffer_depth)
/*******************************************************************************
LAST MODIFIED : 2 June 2004

DESCRIPTION :
==============================================================================*/
{
#define MAX_GL_ATTRIBUTES (50)
	GdkGLConfig *glconfig;
	GtkWidget *glarea;
	int accumulation_colour_size, attribute_list[MAX_GL_ATTRIBUTES], *attribute_ptr,
		selection_level;
	struct Graphics_buffer *buffer;

	ENTER(create_Graphics_buffer_gtkglarea);

	if (gdk_gl_query_extension() == TRUE)
	{
		if (parent)
		{
			glarea = gtk_drawing_area_new();
			if (glarea)
			{
				buffer = CREATE(Graphics_buffer)(graphics_buffer_package);
				if (buffer)
				{
					glconfig = (GdkGLConfig *)NULL;
					selection_level = 2;
					while (!glconfig && (selection_level > 0))
					{
						attribute_ptr = attribute_list;

						*attribute_ptr = GDK_GL_RGBA;
						attribute_ptr++;
						if (buffering_mode == GRAPHICS_BUFFER_DOUBLE_BUFFERING)
						{
							*attribute_ptr = GDK_GL_DOUBLEBUFFER;
							attribute_ptr++;
						}
						/* else GRAPHICS_BUFFER_ANY_BUFFERING_MODE so don't specify it */
						if (stereo_mode == GRAPHICS_BUFFER_STEREO)
						{
							*attribute_ptr = GDK_GL_STEREO;
							attribute_ptr++;
						}
						/* else GRAPHICS_BUFFER_ANY_STEREO_MODE so don't specify it */
						if (minimum_colour_buffer_depth)
						{
							*attribute_ptr = GDK_GL_BUFFER_SIZE;
							attribute_ptr++;
							*attribute_ptr = minimum_colour_buffer_depth;
							attribute_ptr++;
						}
						if (selection_level > 1)
						{
							*attribute_ptr = GDK_GL_ALPHA_SIZE;
							attribute_ptr++;
							*attribute_ptr = 1;
							attribute_ptr++;
						}
						if (minimum_depth_buffer_depth)
						{
							*attribute_ptr = GDK_GL_DEPTH_SIZE;
							attribute_ptr++;
							*attribute_ptr = minimum_depth_buffer_depth;
							attribute_ptr++;
						}
						if (minimum_accumulation_buffer_depth)
						{
							accumulation_colour_size = minimum_accumulation_buffer_depth / 4;
							*attribute_ptr = GDK_GL_ACCUM_RED_SIZE;
							attribute_ptr++;
							*attribute_ptr = accumulation_colour_size;
							attribute_ptr++;
							*attribute_ptr = GDK_GL_ACCUM_GREEN_SIZE;
							attribute_ptr++;
							*attribute_ptr = accumulation_colour_size;
							attribute_ptr++;
							*attribute_ptr = GDK_GL_ACCUM_BLUE_SIZE;
							attribute_ptr++;
							*attribute_ptr = accumulation_colour_size;
							attribute_ptr++;
							*attribute_ptr = GDK_GL_ACCUM_ALPHA_SIZE;
							attribute_ptr++;
							*attribute_ptr = accumulation_colour_size;
							attribute_ptr++;
						}
						*attribute_ptr = GDK_GL_ATTRIB_LIST_NONE;
						attribute_ptr++;
						glconfig = gdk_gl_config_new(attribute_list);

						selection_level--;
					}
					if (glconfig &&
						gtk_widget_set_gl_capability(glarea, glconfig,
							graphics_buffer_package->share_glcontext,
							TRUE, GDK_GL_RGBA_TYPE))
					{
						buffer->glarea = glarea;
						buffer->glconfig = gtk_widget_get_gl_config(glarea);
						buffer->type = GRAPHICS_BUFFER_GTKGLEXT_TYPE;
						gtk_widget_set_events(buffer->glarea,
							GDK_EXPOSURE_MASK|GDK_POINTER_MOTION_MASK|GDK_POINTER_MOTION_HINT_MASK|
							GDK_BUTTON_PRESS_MASK|GDK_BUTTON_RELEASE_MASK|
							GDK_KEY_PRESS_MASK|GDK_KEY_RELEASE_MASK);
						buffer->initialise_handler_id =
							g_signal_connect(G_OBJECT(buffer->glarea), "realize",
							G_CALLBACK(Graphics_buffer_gtkglarea_initialise_callback),
							(gpointer)buffer);
					buffer->resize_handler_id =
						g_signal_connect(G_OBJECT(buffer->glarea), "size-allocate",
							G_CALLBACK(Graphics_buffer_gtkglarea_resize_callback),
							(gpointer)buffer);
					buffer->expose_handler_id =
						g_signal_connect(G_OBJECT(buffer->glarea), "expose-event",
							G_CALLBACK(Graphics_buffer_gtkglarea_expose_callback),
							(gpointer)buffer);
					buffer->button_press_handler_id =
						g_signal_connect(G_OBJECT(buffer->glarea), "button-press-event",
							G_CALLBACK(Graphics_buffer_gtkglarea_button_callback),
							(gpointer)buffer);
					buffer->button_release_handler_id =
						g_signal_connect(G_OBJECT(buffer->glarea), "button-release-event",
							G_CALLBACK(Graphics_buffer_gtkglarea_button_callback),
							(gpointer)buffer);
					buffer->key_press_handler_id =
						g_signal_connect(G_OBJECT(buffer->glarea), "key-press-event",
							G_CALLBACK(Graphics_buffer_gtkglarea_key_callback),
							(gpointer)buffer);
					buffer->key_release_handler_id =
						g_signal_connect(G_OBJECT(buffer->glarea), "key-release-event",
							G_CALLBACK(Graphics_buffer_gtkglarea_key_callback),
							(gpointer)buffer);
					buffer->motion_handler_id =
						g_signal_connect(G_OBJECT(buffer->glarea), "motion-notify-event",
							G_CALLBACK(Graphics_buffer_gtkglarea_motion_notify_callback),
							(gpointer)buffer);
						gtk_container_add(parent, buffer->glarea);
					}
					else
					{
						display_message(ERROR_MESSAGE,"create_Graphics_buffer_gtkgl.  "
							"Unable to add opengl capability.");
						DEACCESS(Graphics_buffer)(&buffer);
						buffer = (struct Graphics_buffer *)NULL;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,"create_Graphics_buffer_gtkgl.  "
						"Unable to create generic Graphics_buffer.");
					buffer = (struct Graphics_buffer *)NULL;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,"create_Graphics_buffer_gtkgl.  "
					"Could not create drawing area widget.");
				buffer = (struct Graphics_buffer *)NULL;
			}
		}
#if defined (ENABLE_GTK_PBUFFER_RENDERING)
		else if ((GRAPHICS_BUFFER_RENDER_OFFSCREEN_AND_COPY == buffering_mode) ||
			(GRAPHICS_BUFFER_RENDER_OFFSCREEN_AND_BLEND == buffering_mode))
		{
			buffer->minimum_colour_buffer_depth = minimum_colour_buffer_depth;
			buffer->minimum_depth_buffer_depth = minimum_depth_buffer_depth;
			buffer->minimum_accumulation_buffer_depth = minimum_accumulation_buffer_depth;

			buffer->buffering_mode = buffering_mode;

			/* If we don't use the pbuffer below we need a SINGLE BUFFERING OpenGL
				when we search pixel formats */
			buffering_mode = GRAPHICS_BUFFER_SINGLE_BUFFERING;
			if (Graphics_buffer_gtk_reallocate_offscreen_size(buffer))
			{
				if (GRAPHICS_BUFFER_GTK_COPY_PBUFFER_TYPE == buffer->type)
				{
#if defined (DEBUG_CODE)
					printf("Using pbuffer\n");
#endif /* defined (DEBUG_CODE) */
					use_pbuffer = 1;
				}
				else
				{
					/* Try to get some alpha planes although I think they don't work anyway */
					minimum_alpha_buffer_depth = 1;
				}
			}
			else
			{
				DEACCESS(Graphics_buffer)(&buffer);
				buffer = (struct Graphics_buffer *)NULL;
			}

		}
#endif /* defined (ENABLE_GTK_PBUFFER_RENDERING) */
		else
		{
			display_message(ERROR_MESSAGE,"create_Graphics_buffer_gtkglarea.  "
				"Missing gtk container and not an render and copy display mode.");
			buffer = (struct Graphics_buffer *)NULL;

		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"create_Graphics_buffer_gtkgl.  "
			"Gdk Open GL EXT not supported.");
		buffer = (struct Graphics_buffer *)NULL;
	}
	LEAVE;

	return (buffer);
} /* create_Graphics_buffer_gtkgl */
#endif /* ! defined (GTK_USE_GTKGLAREA) */
#endif /* defined (GTK_USER_INTERFACE) */

#if defined (WIN32_USER_INTERFACE)
static int Graphics_buffer_win32_create_context(struct Graphics_buffer *buffer)
/*******************************************************************************
Actually make the OpenGL context.
==============================================================================*/
{
	int i, return_code;

	ENTER(Graphics_buffer_win32_create_context);
	if (buffer)
	{
		return_code = 1;
		/* Find a valid OpenGL context for the hdc */
		if ((GRAPHICS_BUFFER_WIN32_COPY_PBUFFER_TYPE != buffer->type) && buffer->hDC)
		{
			int number_of_formats = DescribePixelFormat(buffer->hDC, 1, 0, NULL);
			int best_selection_level = 0;
			int format = 0;
			int selection_level;
			PIXELFORMATDESCRIPTOR pfd;

			for (i = 0 ; i < number_of_formats ; i++)
			{
				/* set the pixel format for the DC */
				ZeroMemory( &pfd, sizeof( PIXELFORMATDESCRIPTOR ) );
				pfd.nSize = sizeof( pfd );
				pfd.nVersion = 1;

				DescribePixelFormat(buffer->hDC, i, pfd.nSize, &pfd);

				if ((pfd.dwFlags & PFD_SUPPORT_OPENGL)
					&& (pfd.iPixelType == PFD_TYPE_RGBA))
				{
					selection_level = 10;
				}
				else
				{
					/* No good */
					selection_level = 0;
				}

				if (pfd.dwFlags & PFD_GENERIC_ACCELERATED)
				{
					/* Windows driver accelerated OpenGL */
					selection_level -= 2;
				}
				else
				{
					if (pfd.dwFlags & PFD_GENERIC_FORMAT)
					{
						/* Software OpenGL */
						selection_level -= 4;
					}
					else
					{
						/* Independent driver accelerated OpenGL */
						/* Best */
					}
				}

				if (!buffer->hWnd)
				{
					if (!(pfd.dwFlags & PFD_DRAW_TO_BITMAP))
					{
						selection_level = 0;
					}
				}
				else
				{
					if (!(pfd.dwFlags & PFD_DRAW_TO_WINDOW))
					{
						selection_level = 0;
					}
				}

				if (pfd.cColorBits >= 24)
				{
					/* Best */
				}
				else if (pfd.cColorBits >= buffer->minimum_colour_buffer_depth)
				{
					/* Satisfactory */
					selection_level--;
				}
				else
				{
					/* Poor */
					selection_level-=2;
				}

				if (pfd.cDepthBits >= 24)
				{
					/* Best */
				}
				else if (pfd.cDepthBits >= buffer->minimum_depth_buffer_depth)
				{
					/* Satisfactory */
					selection_level--;
				}
				else
				{
					/* Poor */
					selection_level-=2;
				}

				if (pfd.cAlphaBits >= buffer->minimum_alpha_buffer_depth)
				{
					/* Best */
				}
				else if (pfd.cAlphaBits > 0)
				{
					/* Satisfactory */
					selection_level--;
				}
				else
				{
					/* Poor */
					selection_level-=2;
				}

				if (pfd.cAccumBits >= buffer->minimum_accumulation_buffer_depth)
				{
					/* Best */
				}
				else if (pfd.cAccumBits > 0)
				{
					/* Satisfactory */
					selection_level--;
				}
				else
				{
					/* Poor */
					selection_level-=2;
				}

				switch (buffer->buffering_mode)
				{
					case GRAPHICS_BUFFER_SINGLE_BUFFERING:
					{
						if (pfd.dwFlags & PFD_DOUBLEBUFFER)
						{
							selection_level = 0;
						}
					} break;
					case GRAPHICS_BUFFER_DOUBLE_BUFFERING:
					{
						if (!(pfd.dwFlags & PFD_DOUBLEBUFFER))
						{
							selection_level = 0;
						}
					} break;
					default: /* GRAPHICS_BUFFER_ANY_BUFFERING_MODE: */
					{
					} break;
				}
				switch (buffer->stereo_mode)
				{
					case GRAPHICS_BUFFER_MONO:
					{
						if (pfd.dwFlags & PFD_STEREO)
						{
							selection_level = 0;
						}
					} break;
					case GRAPHICS_BUFFER_STEREO:
					{
						if (!(pfd.dwFlags & PFD_STEREO))
						{
							selection_level = 0;
						}
					} break;
					default: /* GRAPHICS_BUFFER_ANY_STEREO_MODE: */
					{
					} break;
				}

				if (selection_level > best_selection_level)
				{
					format = i;
					best_selection_level = selection_level;
				}
			}
			if (!format)
			{
			  /* Try the automatic chooser */
			  memset(&pfd,0, sizeof(PIXELFORMATDESCRIPTOR)) ;
			  pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
			  pfd.nVersion = 1 ;
			  pfd.dwFlags =  PFD_SUPPORT_OPENGL ;
			  pfd.iPixelType = PFD_TYPE_RGBA ;
			  pfd.cColorBits = 24 ;
			  pfd.cDepthBits = 32 ;
			  pfd.iLayerType = PFD_MAIN_PLANE ;
			  // Choose the pixel format.
			  format = ChoosePixelFormat(buffer->hDC, &pfd);
			  best_selection_level = -1;  /* Set a value to show that it was selected by ChoosePixelFormat */
			}

			if (format)
			{
#if defined (DEBUG_CODE)
				printf ("Trying format %d, selection level %d\n",
					format, best_selection_level);
#endif /* defined (DEBUG_CODE) */
				if(SetPixelFormat( buffer->hDC, format, &pfd ))
				{
#if defined (DEBUG_CODE)
					printf ("SetPixelFormat %d success\n", format);
#endif /* defined (DEBUG_CODE) */
					buffer->pixel_format = format;
				}
				else
				{
					DWORD error = GetLastError();
					display_message(ERROR_MESSAGE,"create_Graphics_buffer_win32.  "
						"Unable to set pixel format. Microsoft error code: %d", error);
					return_code = 0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,"create_Graphics_buffer_win32.  "
					"No valid pixel formats found.");
				return_code = 0;
			}
		}
		if (return_code)
		{
			/* A work around for Intel GMA 900 cards on windows where compilation
				of textures only seems to work on the first context of a
				share group.  On these cards only use a single
				graphics context for all graphics buffers.
				The vendor string will not be defined the first time around,
				although when using multiple instances in the same memory
				space (such as with zinc) it could be set for the first
				in a share group. */
			const char *vendor_string = (const char *)glGetString(GL_VENDOR);
			if (!buffer->package->wgl_shared_context ||
				!vendor_string || strcmp("Intel", vendor_string))
			{
				/* Default normal implementation,
					different context for each buffer with shared lists */
				/* create and enable the render context (RC) */
				if(buffer->hRC = wglCreateContext( buffer->hDC ))
				{
					if (buffer->package->wgl_shared_context)
					{
						wglShareLists(buffer->package->wgl_shared_context,
							buffer->hRC);
					}
					else
					{
						buffer->package->wgl_shared_context = buffer->hRC;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,"create_Graphics_buffer_win32.  "
						"Unable to create the render context.");
					return_code = 0;
				}
			}
			else
			{
				/* Use a single context for all graphics buffers */
				buffer->hRC = buffer->package->wgl_shared_context;
			}
			if (return_code)
			{
				if(wglMakeCurrent(buffer->hDC,buffer->hRC))
				{
#if defined (DEBUG_CODE)
					printf("Made new context %p %p\n", buffer->hDC, buffer->hRC);

					PIXELFORMATDESCRIPTOR pfd;
					ZeroMemory( &pfd, sizeof( PIXELFORMATDESCRIPTOR ) );
					pfd.nSize = sizeof( pfd );
					pfd.nVersion = 1;

					DescribePixelFormat(buffer->hDC, buffer->pixel_format, pfd.nSize, &pfd);

					printf("Pixel format %d\n", buffer->pixel_format);
					const char *vendor_string = (const char *)glGetString(GL_VENDOR);
					printf("OpenGL vendor string %s\n", vendor_string);
					if (pfd.dwFlags & PFD_GENERIC_ACCELERATED)
					{
						/* Windows driver accelerated OpenGL */
						printf("Windows driver accelerated OpenGL\n");
					}
					else
					{
						if (pfd.dwFlags & PFD_GENERIC_FORMAT)
						{
							/* Windows software OpenGL */
							printf("Software OpenGL\n");
						}
						else
						{
							/* Independent driver accelerated OpenGL */
							printf("Independent driver accelerated OpenGL\n");
						}
					}
					printf("Colour depth %d\n",  pfd.cColorBits);
					printf("Z depth %d\n",  pfd.cDepthBits);
					printf("Alpha depth %d\n",  pfd.cAlphaBits);
					printf("Accumulation depth %d\n",  pfd.cAccumBits);
#endif /* defined (DEBUG_CODE) */
				}
				else
				{
					display_message(ERROR_MESSAGE,"Graphics_buffer_win32_create_context.  "
						"Unable enable the render context.");
					return_code = 0;
				}
			}
		}
#if defined (DEBUG_CODE)
		fflush(stdout);
#endif /* defined (DEBUG_CODE) */
	}
	else
	{
		display_message(ERROR_MESSAGE,"Graphics_buffer_win32_create_context.  "
			"Missing graphics_buffer parameter.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Graphics_buffer_win32_create_context */
#endif /* defined (WIN32_USER_INTERFACE) */

#if defined (WIN32_USER_INTERFACE)
static int Graphics_buffer_win32_reallocate_offscreen_size(
	struct Graphics_buffer *buffer, HDC onscreen_hdc)
/*******************************************************************************
LAST MODIFIED : 10 March 2008

DESCRIPTION :
Resizes the offscreen pbuffer used for rendering with windowless mode.
==============================================================================*/
{
	int return_code;

	ENTER(Graphics_buffer_win32_reallocate_pbuffer_size);
	if (buffer)
	{
		return_code = 0;
#if defined (DEBUG_CODE)
		printf("Graphics_buffer_win32_reallocate_offscreen_size %p checking size %d %d current %d %d\n",
				buffer, buffer->width, buffer->height, buffer->offscreen_width, buffer->offscreen_height);
#endif // defined (DEBUG_CODE)

		/* We never bother to reduce the size */
		if (!buffer->offscreen_width || (buffer->offscreen_width < buffer->width)
			|| !buffer->offscreen_height || (buffer->offscreen_height < buffer->height))
		{
			/* 64x64 is the current minimum.  Allocate powers of two just to be
				conservative with graphics card drivers,
				and ensure we are using 4 byte aligned buffers and rows.
				(Also see work around for glReadPixels on ATI graphics.
				https://tracker.physiomeproject.org/show_bug.cgi?id=2697) */
			int required_width = 64;
			int required_height = 64;
			while (required_width < buffer->width)
			{
				required_width *= 2;
			}
			while (required_height < buffer->height)
			{
				required_height *= 2;
			}

#if defined (WGL_ARB_pixel_format) && (WGL_ARB_pbuffer)
			{
				/* Must create and select the offscreen buffer before testing extensions
				 otherwise default windows implementation will just respond unavailable. */
				if (!buffer->package->hidden_graphics_buffer)
				{
					BOOL win32_return_code;
					static const char *class_name="Hidden window";
					WNDCLASS class_information;

					/* check if the class is registered */
					win32_return_code=GetClassInfo(User_interface_get_instance(
						buffer->package->user_interface),
						class_name,&class_information);

					if (win32_return_code==FALSE)
					{
						/* register class */
						class_information.cbClsExtra=0;
						class_information.cbWndExtra=sizeof(struct Graphics_window *);
						class_information.hbrBackground=(HBRUSH)(COLOR_WINDOW+1);
						class_information.hCursor=LoadCursor(NULL,IDC_ARROW);
						class_information.hIcon=LoadIcon(
							User_interface_get_instance(buffer->package->user_interface),
							"Command_window_icon");
						class_information.hInstance=User_interface_get_instance(
							buffer->package->user_interface);
						class_information.lpfnWndProc=DefWindowProc;
						class_information.lpszClassName=class_name;
						class_information.style=CS_OWNDC;
						class_information.lpszMenuName=NULL;
						if (RegisterClass(&class_information))
						{
							win32_return_code=TRUE;
						}
					}

					/* Need to get an accelerated rendering context before
						we can try these functions!!! */
					buffer->package->hidden_accelerated_window =CreateWindow(class_name, "Hidden",
						WS_CAPTION | WS_POPUPWINDOW | WS_SIZEBOX,
						0, 0, 100, 100,
						NULL, NULL, User_interface_get_instance(buffer->package->user_interface), NULL);

					if (!buffer->package->hidden_graphics_package)
						buffer->package->hidden_graphics_package = CREATE(Graphics_buffer_package)(buffer->package->user_interface);

					buffer->package->hidden_graphics_buffer = create_Graphics_buffer_win32(
						buffer->package->hidden_graphics_package,
						buffer->package->hidden_accelerated_window, (HDC)NULL,
						GRAPHICS_BUFFER_ANY_BUFFERING_MODE, GRAPHICS_BUFFER_ANY_STEREO_MODE,
						buffer->minimum_colour_buffer_depth, buffer->minimum_depth_buffer_depth,
						buffer->minimum_accumulation_buffer_depth);
				}

				Graphics_buffer_make_current(buffer->package->hidden_graphics_buffer);

#if defined (DEBUG_CODE)
				printf("Made hidden window current\n");
#endif /* defined (DEBUG_CODE) */

				if (Window_system_extensions_check_wgl_extension(WGL_ARB_pixel_format) &&
					Window_system_extensions_check_wgl_extension(WGL_ARB_pbuffer))
				{
					/* Release the previous pbuffer */
					if (buffer->pbuffer && buffer->hDC)
					{
						wglReleasePbufferDCARB(buffer->pbuffer, buffer->hDC);
					}
					if (buffer->pbuffer)
					{
						wglDestroyPbufferARB(buffer->pbuffer);
					}

					/* Should be selecting with our standard rules and minimums here */
					int colour_bits = 32;
					int alpha_bits = 8;
					int depth_bits = 24;
					GLint pixel_format;
					unsigned int number_of_formats;

					const int pbuffer_attributes[]=
						{WGL_DRAW_TO_PBUFFER_ARB, 1,
						 WGL_COLOR_BITS_ARB, colour_bits,
						 WGL_ALPHA_BITS_ARB, alpha_bits,
						 WGL_DEPTH_BITS_ARB, depth_bits,
						 0};
					const float float_pbuffer_attributes[]={
						0};

#if defined (DEBUG_CODE)
					printf("Trying pbuffer\n");
#endif /* defined (DEBUG_CODE) */

					/* Only get the first valid format */
					if(wglChoosePixelFormatARB(buffer->package->hidden_graphics_buffer->hDC,
							pbuffer_attributes, float_pbuffer_attributes, 1,
							&pixel_format, &number_of_formats))
					{
						const int pbuffer_attrib[] = {0};

						if (buffer->pbuffer=wglCreatePbufferARB(
								 buffer->package->hidden_graphics_buffer->hDC, pixel_format,
								 required_width, required_height, pbuffer_attrib))
						{
							if (buffer->hDC = wglGetPbufferDCARB(buffer->pbuffer))
							{
								buffer->type = GRAPHICS_BUFFER_WIN32_COPY_PBUFFER_TYPE;
								buffer->pixel_format = pixel_format;
								return_code = 1;
#if defined (DEBUG_CODE)
								printf("Using pbuffer\n");
#endif // defined (DEBUG_CODE)
							}
							else
							{
								return_code = 0;
							}
						}
						else
						{
							return_code = 0;
						}
					}
					else
					{
						return_code = 0;
					}
				}
				else
				{
					/* Try non accelerated bitmap OpenGL instead */
					return_code = 1;
#if defined (DEBUG_CODE)
					printf("Using non accelerated bitmap\n");
#endif // defined (DEBUG_CODE)
				}
			}
#endif /* defined (WGL_ARB_pixel_format) && (WGL_ARB_pbuffer) */
			/* In either case we need a device independent bitmap matching the
				onscreen hdc.  Either for copying the pbuffer pixels or for rendering
				directly if we cannot get a pbuffer. */
			if (buffer->device_independent_bitmap)
			{
				DeleteObject(buffer->device_independent_bitmap);
			}
			if (buffer->device_independent_bitmap_hdc)
			{
				DeleteDC(buffer->device_independent_bitmap_hdc);
			}
			{
				BITMAPINFO bmi;

				bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
				bmi.bmiHeader.biWidth = required_width;
				bmi.bmiHeader.biHeight= required_height;
				bmi.bmiHeader.biPlanes = 1;
				if (GRAPHICS_BUFFER_RENDER_OFFSCREEN_AND_BLEND == buffer->buffering_mode)
				{
					bmi.bmiHeader.biBitCount = 32;
				}
				else
				{
					bmi.bmiHeader.biBitCount = 24;
				}
				if (GRAPHICS_BUFFER_WIN32_COPY_PBUFFER_TYPE != buffer->type)
				{
					/* If the onscreen size is 32 then the setpixelformat seems to
						want this even if rendering without alpha */
					int onscreen_bits = GetDeviceCaps(onscreen_hdc, BITSPIXEL);
					if (bmi.bmiHeader.biBitCount < onscreen_bits)
					{
						bmi.bmiHeader.biBitCount = onscreen_bits;
					}
				}

#if defined (DEBUG_CODE)
				printf("Bitmap bit count %d %d\n", bmi.bmiHeader.biBitCount,
					   GetDeviceCaps(onscreen_hdc, BITSPIXEL));
#endif /* defined (DEBUG_CODE) */

				bmi.bmiHeader.biCompression = BI_RGB;
				bmi.bmiHeader.biSizeImage = 0;
				bmi.bmiHeader.biXPelsPerMeter = 0;
				bmi.bmiHeader.biYPelsPerMeter = 0;
				bmi.bmiHeader.biClrUsed = 0;
				bmi.bmiHeader.biClrImportant = 0;

				buffer->device_independent_bitmap_hdc = CreateCompatibleDC(onscreen_hdc);
				if (!buffer->device_independent_bitmap_hdc)
				{
					buffer->device_independent_bitmap_hdc = CreateCompatibleDC(NULL);
				}
				buffer->device_independent_bitmap =
					CreateDIBSection(buffer->device_independent_bitmap_hdc,
						&bmi,
						DIB_RGB_COLORS,
						(void **)&buffer->device_independent_bitmap_pixels,
						0,
						0);
				SelectObject(buffer->device_independent_bitmap_hdc,
					buffer->device_independent_bitmap);

#if defined (DEBUG_CODE)
				printf("Made dib %p %p %d\n",
					   buffer->device_independent_bitmap_hdc,
					   buffer->device_independent_bitmap, buffer->type);
#endif /* defined (DEBUG_CODE) */

				return_code = 1;

				if (buffer->type != GRAPHICS_BUFFER_WIN32_COPY_PBUFFER_TYPE)
				{
					buffer->type = GRAPHICS_BUFFER_WIN32_COPY_BITMAP_TYPE;
					/* We use the bitmap directly as the OpenGL rendering surface */
					buffer->hDC = buffer->device_independent_bitmap_hdc;
#if defined (DEBUG_CODE)
					printf("buffer->hDC %p\n", buffer->hDC);
#endif /* defined (DEBUG_CODE) */
				}

			}
			if (return_code)
			{
				buffer->offscreen_width = required_width;
				buffer->offscreen_height = required_height;
				buffer->offscreen_render_required = 1;

				if (buffer->hRC)
				{
					wglMakeCurrent(NULL, NULL);
					/* Only delete it here if it isn't the share context, otherwise we
						will destroy it when the graphics buffer package is destroyed.
						In the intel workaround case we only want to destroy it at the end. */
					if (buffer->hRC && (buffer->hRC != buffer->package->wgl_shared_context))
					{
#if defined (DEBUG_CODE)
						printf("Graphics_buffer_win32_reallocate_offscreen_size %p wglDeleteContext %p\n", buffer, buffer->hRC);
#endif //defined (DEBUG_CODE)
						wglDeleteContext(buffer->hRC);
					}
				}

				return_code = Graphics_buffer_win32_create_context(buffer);
			}
		}
		else
		{
			/* Already large enough so nothing to do */
			return_code = 1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Graphics_buffer_win32_reallocate_pbuffer_size.  "
			"Missing graphics_buffer parameter.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Graphics_buffer_win32_reallocate_pbuffer_size */
#endif /* defined (WIN32_USER_INTERFACE) */

#if defined (WIN32_USER_INTERFACE)
struct Graphics_buffer *create_Graphics_buffer_win32(
	struct Graphics_buffer_package *graphics_buffer_package,
	HWND hWnd, HDC hDC,
	enum Graphics_buffer_buffering_mode buffering_mode,
	enum Graphics_buffer_stereo_mode stereo_mode,
	int minimum_colour_buffer_depth, int minimum_depth_buffer_depth,
	int minimum_accumulation_buffer_depth)
/*******************************************************************************
LAST MODIFIED : 9 August 2004

DESCRIPTION :
Creates a Graphics buffer on the specified <hWnd> window handle.
If the <hDC> is specified it is used to render.
Alternatively if <hWnd> is NULL and <hDC> is specified then no window functions
are performed but the graphics window will render into the supplied device context.
==============================================================================*/
{
	int render_offscreen;
	struct Graphics_buffer *buffer;

	ENTER(create_Graphics_buffer_win32);
	if (buffer = CREATE(Graphics_buffer)(graphics_buffer_package))
	{
		/* Hardcode this for the meantime so as not to change function interface */
		int minimum_alpha_buffer_depth = 8;

		render_offscreen = 0;

		buffer->type = GRAPHICS_BUFFER_WIN32_TYPE;
		buffer->buffering_mode = buffering_mode;
		buffer->stereo_mode = stereo_mode;
		buffer->minimum_colour_buffer_depth = minimum_colour_buffer_depth;
		buffer->minimum_depth_buffer_depth = minimum_depth_buffer_depth;
		buffer->minimum_accumulation_buffer_depth = minimum_accumulation_buffer_depth;
		buffer->minimum_alpha_buffer_depth = minimum_alpha_buffer_depth;

		if (hWnd)
		{
			buffer->hWnd = hWnd;

			SetWindowLongPtr(hWnd, GWL_WNDPROC,
				(LONG)Graphics_buffer_callback_proc);
			SetWindowLongPtr(hWnd, GWL_USERDATA, (LONG)buffer);

			if (hDC)
			{
				buffer->hDC = hDC;
			}
			else
			{
				buffer->hDC=GetDC(hWnd);
			}
		}
		else if ((GRAPHICS_BUFFER_RENDER_OFFSCREEN_AND_COPY == buffering_mode) ||
			(GRAPHICS_BUFFER_RENDER_OFFSCREEN_AND_BLEND == buffering_mode))
		{
			/* We should always use single buffering with an offscreen render */
			buffering_mode = GRAPHICS_BUFFER_SINGLE_BUFFERING;

			render_offscreen = 1;
		}
#if defined (DEBUG_CODE)
		printf ("create_Graphics_buffer_win32 %p %p %p\n",
			hWnd, hDC, buffer->hDC);
#endif /* defined (DEBUG_CODE) */

		/* Defer creating the graphics context for offscreen rendering until we actually need it
		 * and will have the correct initial sizes.
		 */
		if (buffer && !render_offscreen)
		{
			if (!Graphics_buffer_win32_create_context(buffer))
			{
				DEACCESS(Graphics_buffer)(&buffer);
				buffer = (struct Graphics_buffer *)NULL;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"create_Graphics_buffer_win32.  "
			"Unable to create generic Graphics_buffer.");
		buffer = (struct Graphics_buffer *)NULL;
	}
	LEAVE;

	return (buffer);
} /* create_Graphics_buffer_win32 */
#endif /* defined (WIN32_USER_INTERFACE) */

#if defined (WIN32_USER_INTERFACE) && (defined (__CYGWIN__) || defined (__MINGW__)) \
	&& ((__W32API_MAJOR_VERSION < 3) || (__W32API_MAJOR_VERSION == 3) && (__W32API_MINOR_VERSION < 7))
/* This is a relatively recent addition (version 3.7 2006-04-07) to the free W32API headers
   so declaring here if the headers are too old,
   however the actual function and dll was available in windows 98 */
extern "C" {
  WINGDIAPI BOOL  WINAPI AlphaBlend(HDC,int,int,int,int,HDC,int,int,int,int,BLENDFUNCTION);
}
#endif /* defined (WIN32_USER_INTERFACE) && (defined (__CYGWIN__) || defined (__MINGW__)) */

#if defined (WIN32_USER_INTERFACE)
int Graphics_buffer_handle_windows_event(struct Graphics_buffer *buffer,
	UINT message_identifier,WPARAM first_message,LPARAM second_message)
/*******************************************************************************
LAST MODIFIED : 5 June 2007

DESCRIPTION:
Handle an external windows event.  Used to support windowless plugin
mode with zinc.
==============================================================================*/
{
  int return_code;

  ENTER(Graphics_buffer_handle_windows_event);

  switch (message_identifier)
  {
	  case WM_CREATE:
	  {
		  return_code=1;
	  }
	  case WM_PAINT:
	  {
		  // We need to just paint what we are told otherwise we will mess up the
		  // compositing
		  HDC hdc = (HDC)first_message;
		  RECT * drc = (RECT *)second_message;

#if defined (DEBUG_CODE)
		  printf("Graphics_buffer_handle_windows_event paint %p\n",
					buffer);
		  printf ("Graphics_buffer_handle_windows_event WMPAINT\nhdc %p left %ld right %ld top %ld bottom %ld\n",
				  hdc, drc->left, drc->right, drc->top, drc->bottom);
		  fflush(stdout);
#endif // defined (DEBUG_CODE)

		  Graphics_buffer_win32_reallocate_offscreen_size(buffer, hdc);

		  if (buffer->offscreen_render_required)
		  {
#if defined (DEBUG_CODE)
			printf("Graphics_buffer_handle_windows_event render required %p\n",
					buffer);
#endif // defined (DEBUG_CODE)
			  wglMakeCurrent( buffer->hDC, buffer->hRC );

			  if (buffer->offscreen_render_required == 1)
			  {
				  if (buffer->package->intel_single_context_force_clipping)
				  {
					  glScissor(0, 0, buffer->offscreen_width, buffer->offscreen_height);
					  glEnable(GL_SCISSOR_TEST);
				  }

				  Graphics_buffer_expose_data expose_data;
				  CMZN_CALLBACK_LIST_CALL(Graphics_buffer_callback)(
					  buffer->expose_callback_list, buffer, &expose_data);
			  }
			  if (buffer->type == GRAPHICS_BUFFER_WIN32_COPY_PBUFFER_TYPE)
			  {
#if defined (DEBUG_CODE)
				  printf("Graphics_buffer_handle_windows_event opengl update %p\n",
					 buffer);
#endif // defined (DEBUG_CODE)

				  glPixelStorei(GL_PACK_ROW_LENGTH, buffer->offscreen_width);
				  if (GRAPHICS_BUFFER_RENDER_OFFSCREEN_AND_BLEND == buffer->buffering_mode)
				  {
					  /* Packing problem below isn't a problem here as pixel packets are all
					   * 4 bytes so width is necessarily 4 byte aligned.
					   */
					  glReadPixels(0, 0,
						  buffer->width, buffer->height, GL_BGRA,
						  GL_UNSIGNED_BYTE,
						  (unsigned char *)buffer->device_independent_bitmap_pixels +
						  4 * buffer->offscreen_width * (buffer->offscreen_height - buffer->height));

				  }
				  else
				  {
					  /* Avoid a packing bug in ATI graphics.
					   * https://tracker.physiomeproject.org/show_bug.cgi?id=2697
					   * Expand the width read to a 4 byte aligned boundary.  This is always
					   * possible as the offscreen width is a power of two and therefore 4
					   * byte aligned.  It just results in reading a few extra pixels.
					   */
					  int aligned_buffer_width = buffer->width + (4 - (buffer->width % 4)) % 4;
					  glReadPixels(0, 0,
						  aligned_buffer_width, buffer->height, GL_BGR,
						  GL_UNSIGNED_BYTE,
						  (unsigned char *)buffer->device_independent_bitmap_pixels +
						  3 * buffer->offscreen_width * (buffer->offscreen_height - buffer->height));
				  }
				  glPixelStorei(GL_PACK_ROW_LENGTH, 0);
			  }
			  buffer->offscreen_render_required = 0;
		  }

		  /* Work around bug in firefox.  See definition of mouse_x.
		   */
		  if (((drc->left == buffer->x) || (drc->left == buffer->x - 1))
			&& ((drc->top == buffer->y) || (drc->top == buffer->y - 1)) &&
			((drc->right - drc->left) >= buffer->width) && ((drc->bottom - drc->top) >= buffer->height))
		  {
			  buffer->mouse_x = buffer->x;
			  buffer->mouse_y = buffer->y;
		  }

		  {
			  int x = drc->left;
			  int y = drc->top;

			  x -= buffer->x;
			  y -= buffer->y;

			  if (x < buffer->x)
			  {
				  x = buffer->x;
			  }
			  if (y < buffer->y)
			  {
				  y = buffer->y;
			  }

			  int right = drc->right;
			  int bottom = drc->bottom;

			  right -= buffer->x;
			  bottom -= buffer->y;

			  if (right > buffer->width + buffer->x)
			  {
				  right = buffer->width + buffer->x;
			  }
			  if (bottom > buffer->height + buffer->y)
			  {
				  bottom = buffer->height + buffer->y;
			  }

			  x = buffer->x;
			  y = buffer->y;
			  right = buffer->width + buffer->x;
			  bottom = buffer->height + buffer->y;

			  int width = right - x;
			  int height = bottom - y;

			  if ((width > 0) && (height > 0))
			  {
#if defined (DEBUG_CODE)
				  const char *vendor_string = (const char *)glGetString(GL_VENDOR);
				  printf ("Rendering with %s\n", vendor_string);
#endif /* defined (DEBUG_CODE) */

				  switch (buffer->type)
				  {
					  case GRAPHICS_BUFFER_WIN32_COPY_BITMAP_TYPE:
					  {
						  if (GRAPHICS_BUFFER_RENDER_OFFSCREEN_AND_BLEND == buffer->buffering_mode)
						  {
#if defined (DEBUG_CODE)
							  printf ("Going to blend %d %d %d %d (bgra %d %d %d %d)\n",
								  x - buffer->x, buffer->offscreen_height - height - y + buffer->y,
								  width, height,
								  ((unsigned char *)buffer->device_independent_bitmap_pixels)
								  [4 * buffer->offscreen_width * (buffer->offscreen_height - height - y + buffer->y)],
								  ((unsigned char *)buffer->device_independent_bitmap_pixels)
								  [4 * buffer->offscreen_width * (buffer->offscreen_height - height - y + buffer->y) + 1],
								  ((unsigned char *)buffer->device_independent_bitmap_pixels)
								  [4 * buffer->offscreen_width * (buffer->offscreen_height - height - y + buffer->y) + 2],
								  ((unsigned char *)buffer->device_independent_bitmap_pixels)
								  [4 * buffer->offscreen_width * (buffer->offscreen_height - height - y + buffer->y) + 3]);
#endif /* defined (DEBUG_CODE) */

							  BLENDFUNCTION blendfunction =
								  {
									  AC_SRC_OVER,
									  0,
									  255,
									  AC_SRC_ALPHA
								  };

							  AlphaBlend(hdc, x, y, width, height,
								 buffer->hDC, x - buffer->x,
								 buffer->offscreen_height - height - y + buffer->y, width, height,
								 blendfunction);

						  }
						  else
						  {
#if defined (DEBUG_CODE)
#if defined (DRAW_A_TEST_LINE)
							  {
								  int i;
								  for (i = 0 ; i < 50 ; i++)
									  if (ERROR_INVALID_PARAMETER ==SetPixel(buffer->hDC,
										  x - buffer->x + i,
										  buffer->offscreen_height - height - y + buffer->y + i, RGB(255, 200, 10)))
										  printf("Error writing pixel to %p", buffer->hDC);
							  }
#endif // defined (DRAW_A_TEST_LINE)

							  printf ("Going to bitblt %d %d %d %d (bgra %d %d %d %d)\n",
								  x - buffer->x, buffer->offscreen_height - height - y + buffer->y,
								  width, height,
								  ((unsigned char *)buffer->device_independent_bitmap_pixels)
								  [4 * buffer->offscreen_width * (buffer->offscreen_height - height - y + buffer->y)],
								  ((unsigned char *)buffer->device_independent_bitmap_pixels)
								  [4 * buffer->offscreen_width * (buffer->offscreen_height - height - y + buffer->y) + 1],
								  ((unsigned char *)buffer->device_independent_bitmap_pixels)
								  [4 * buffer->offscreen_width * (buffer->offscreen_height - height - y + buffer->y) + 2],
								  ((unsigned char *)buffer->device_independent_bitmap_pixels)
								  [4 * buffer->offscreen_width * (buffer->offscreen_height - height - y + buffer->y) + 3]);

#if defined (WRITE_EVERY_PIXEL)
							  {
								  int i, j;
								  unsigned int repeat = 0;
								  unsigned long current, previous = 0;
								  for (i = 0 ; i < buffer->offscreen_width ; i++)
									  for (j = 0 ; j < buffer->offscreen_height ; j++)
									  {
										  current = ((unsigned long *)buffer->device_independent_bitmap_pixels)
											 [i + buffer->offscreen_width * j];
										  if (current == previous)
										  {
											  repeat++;
										  }
										  else
										  {
											  if (repeat > 0)
												  printf("   Pixels %0lX %d\n", previous, repeat);
											  previous = current;
											  repeat = 1;
										  }
									  }
								  if (repeat > 0)
									  printf("   Pixels %0lX %d\n", previous, repeat);
							  }
#endif // defined (WRITE_EVERY_PIXEL)
#endif /* defined (DEBUG_CODE) */


							  BitBlt(hdc, x, y, width, height,
								  buffer->hDC, x - buffer->x,
								  buffer->offscreen_height - height - y + buffer->y,
								  SRCCOPY);

#if defined (DRAW_A_TEST_LINE)
							  {
								  int i;
								  for (i = 0 ; i < 50 ; i++)
									  if (ERROR_INVALID_PARAMETER == SetPixel(hdc, x+i, y+i, RGB(200, 20, 200)))
										  printf("Error writing pixel to %p", hdc);
							  }
 #endif /* defined (DRAW_A_TEST_LINE) */
						  }
					  } break;
					  case GRAPHICS_BUFFER_WIN32_COPY_PBUFFER_TYPE:
					  {
						  if (GRAPHICS_BUFFER_RENDER_OFFSCREEN_AND_BLEND == buffer->buffering_mode)
						  {
							  BLENDFUNCTION blendfunction =
								  {
									  AC_SRC_OVER,
									  0,
									  255,
									  AC_SRC_ALPHA
								  };

#if defined (DEBUG_CODE)
							  printf ("Copied pixels %d %d %d %d (rgba %d %d %d %d)\n",
								  x - buffer->x, y - buffer->y,
								  width, height,
								  ((unsigned char *)buffer->device_independent_bitmap_pixels)
								  [4 * buffer->offscreen_width * (buffer->offscreen_height - height)],
								  ((unsigned char *)buffer->device_independent_bitmap_pixels)
								  [4 * buffer->offscreen_width * (buffer->offscreen_height - height) + 1],
								  ((unsigned char *)buffer->device_independent_bitmap_pixels)
								  [4 * buffer->offscreen_width * (buffer->offscreen_height - height) + 2],
								  ((unsigned char *)buffer->device_independent_bitmap_pixels)
								  [4 * buffer->offscreen_width * (buffer->offscreen_height - height) + 3]);
							  printf ("Going to alpha blend %ld %d %d %ld %d %d\n",
								  drc->left, buffer->x, buffer->width,
								  drc->top, buffer->y, buffer->height );
#endif /* defined (DEBUG_CODE) */

							  AlphaBlend(hdc, x, y, width, height,
								  buffer->device_independent_bitmap_hdc, 0, 0, width, height,
								  blendfunction);
						  }
						  else
						  {
#if defined (DEBUG_CODE)
#if defined (DRAW_A_TEST_LINE)
							  for (int i = 0 ; i < 128 ; i++)
								  memset(((char *)buffer->device_independent_bitmap_pixels) + 10 + i * 3 + i * 3 * buffer->offscreen_width, 128,
										  10);
#endif // defined (DRAW_A_TEST_LINE)

							  printf ("Copied pixels %d %d %d %d (rgb %d %d %d)\n",
								  x - buffer->x, y - buffer->y,
								  width, height,
								  ((unsigned char *)buffer->device_independent_bitmap_pixels)
								  [3 * buffer->offscreen_width * (buffer->offscreen_height - height)],
								  ((unsigned char *)buffer->device_independent_bitmap_pixels)
								  [3 * buffer->offscreen_width * (buffer->offscreen_height - height) + 1],
								  ((unsigned char *)buffer->device_independent_bitmap_pixels)
								  [3 * buffer->offscreen_width * (buffer->offscreen_height - height) + 2]);
							  printf ("Going to blt %ld %d %d %ld %d %d\n",
								  drc->left, buffer->x, buffer->width,
								  drc->top, buffer->y, buffer->height );

#if defined (DRAW_A_TEST_LINE)
							  {
								  int i;
								  for (i = 0 ; i < 50 ; i++)
									  if (ERROR_INVALID_PARAMETER ==SetPixel(buffer->device_independent_bitmap_hdc,
										  i, i, RGB(255, 200, 10)))
										  printf("Error writing pixel to %p", buffer->device_independent_bitmap_hdc);
							  }
#endif // defined (DRAW_A_TEST_LINE)
#endif /* defined (DEBUG_CODE) */

							  BitBlt(hdc, x, y, width, height,
								  buffer->device_independent_bitmap_hdc, 0, 0,
								  SRCCOPY);
						  }
					  } break;
					  default:
					  {
						  display_message(ERROR_MESSAGE,"Graphics_buffer_handle_windows_event.  "
							  "Unsupported buffer type.");
						  return_code = 0;
					  }
				  }

#if defined (DEBUG_CODE)
				  printf ("BitBlt %d %d %d %d\n", x, y, width, height);
				  fflush(stdout);
#endif /* defined (DEBUG_CODE) */
			  }
#if defined (DEBUG_CODE)
			  else
			  {
				  printf ("Nothing to render %d %d, %d %d %d %d\n", width, height, x, y, right, bottom);
				  fflush(stdout);
			  }
#endif /* defined (DEBUG_CODE) */
		  }
		  return_code=1;
	  } break;
	  case WM_SIZING:
	  {
#if defined (DEBUG_CODE)
		  printf ("Graphics_buffer_handle_windows_event WM_SIZING\n");
#endif /* defined (DEBUG_CODE) */

		  CMZN_CALLBACK_LIST_CALL(Graphics_buffer_callback)(
			  buffer->resize_callback_list, buffer, NULL);
		  return_code=1;
	  } break;
	  case WM_MOUSEMOVE:
	  {
		  //SetCursor(LoadCursor(NULL, IDC_ARROW));
	  } // No break, fall through
	  case WM_LBUTTONDOWN:
	  case WM_LBUTTONUP:
	  case WM_RBUTTONDOWN:
	  case WM_RBUTTONUP:
	  case WM_MBUTTONDOWN:
	  case WM_MBUTTONUP:
	  {
#if defined (DEBUG_CODE)
		  printf ("Graphics_buffer_handle_windows_event WM_BUTTON %d %d %d %d\n",
				  GET_X_LPARAM(second_message), GET_Y_LPARAM(second_message),
				  buffer->mouse_x, buffer->mouse_y);
#endif /* defined (DEBUG_CODE) */

		  LPARAM offset_coordinates;

		  offset_coordinates = MAKELPARAM(
			  GET_X_LPARAM(second_message) - buffer->mouse_x,
			  GET_Y_LPARAM(second_message) - buffer->mouse_y);

		  return_code = Graphics_buffer_win32_button_callback(&message_identifier,
			  buffer, first_message, offset_coordinates);
	  } break;
	  case WM_SETCURSOR:
	  {
		/* This message does not seem to propagate through to zinc so
		 setting cursor on WM_MOUSEMOVE above instead*/
#if defined (DEBUG_CODE)
		  printf ("Graphics_buffer_handle_windows_event WM_SETCURSOR\n");
#endif /* defined (DEBUG_CODE) */
//	          SetCursor(LoadCursor(NULL, IDC_ARROW));
		  return_code=1;
	  } break;
	  default:
	  {
	  } break;
  }

  return (return_code);
} /* Graphics_buffer_handle_windows_event */
#endif /* defined (WIN32_USER_INTERFACE) */

#if defined (WIN32_USER_INTERFACE)
int Graphics_buffer_win32_set_window_size(struct Graphics_buffer *buffer,
	int width, int height, int x, int y)
/*******************************************************************************
LAST MODIFIED : 14 September 2007

DESCRIPTION :
Sets the maximum extent of the graphics window within which individual paints
will be requested with handle_windows_event.
==============================================================================*/
{
	int return_code;

	ENTER(Graphics_buffer_win32_set_window_size);
	if (buffer)
	{
		if ((buffer->width != width) || (buffer->height != height))
		{
		   buffer->offscreen_render_required = 1;
		}
		buffer->width = width;
		buffer->height = height;
		buffer->x = x;
		buffer->y = y;
#if defined (DEBUG_CODE)
		printf("Graphics_buffer_win32_set_window_size width %d height %d x %d y %d\n",
			width, height, x, y);
#endif /* defined (DEBUG_CODE) */
	}
	else
	{
		display_message(ERROR_MESSAGE,"cmzn_graphics_buffer_win32_set_window_size.  "
			"Missing graphics_buffer parameter.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* cmzn_graphics_buffer_win32_set_window_size */
#endif /* defined (WIN32_USER_INTERFACE) */

#if defined (WIN32_USER_INTERFACE)
static LRESULT CALLBACK Graphics_buffer_callback_proc(HWND window,
	UINT message_identifier,WPARAM first_message,LPARAM second_message)
/*******************************************************************************
LAST MODIFIED : 9 August 2004

DESCRIPTION:
==============================================================================*/
{
	LRESULT return_code;
	PAINTSTRUCT ps;

	ENTER(Graphics_buffer_callback_proc);

	return_code=FALSE;
	struct Graphics_buffer *graphics_buffer =
		(struct Graphics_buffer *)GetWindowLongPtr(window, GWL_USERDATA);

#if defined (DEBUG_CODE)
	printf("window callback %d\n", message_identifier);
#endif /* defined (DEBUG_CODE) */

	switch (message_identifier)
	{
		case WM_CREATE:
		{
			return_code=TRUE;
		}
		/* CS I don't think handling this message is necessary,
		   worse still sending a WM_QUIT kills the application */
		/*
		case WM_DESTROY:
		{
			PostQuitMessage(0);
			return_code=TRUE;
		} break;
		*/
		case WM_PAINT:
		{
#if defined (DEBUG_CODE)
			printf("WM_PAINT\n");
#endif /* defined (DEBUG_CODE) */
			BeginPaint(window, &ps);
			CMZN_CALLBACK_LIST_CALL(Graphics_buffer_callback)(
				graphics_buffer->expose_callback_list, graphics_buffer, NULL);
			EndPaint(window, &ps);
			return_code=TRUE;
		} break;
		case WM_SIZING:
		{
			BeginPaint(window, &ps);
			CMZN_CALLBACK_LIST_CALL(Graphics_buffer_callback)(
				graphics_buffer->resize_callback_list, graphics_buffer, NULL);
			EndPaint(window, &ps);
			return_code=TRUE;
		} break;
		case WM_LBUTTONDOWN:
		{
			Graphics_buffer_win32_button_callback(&message_identifier,
				graphics_buffer, first_message, second_message);
			SetCapture(window);
			return_code=TRUE;
		} break;
		case WM_LBUTTONUP:
		{
			Graphics_buffer_win32_button_callback(&message_identifier,
				graphics_buffer, first_message, second_message);
			ReleaseCapture();
			return_code=TRUE;
		} break;
		case WM_RBUTTONDOWN:
		{
			Graphics_buffer_win32_button_callback(&message_identifier,
				graphics_buffer, first_message, second_message);
			SetCapture(window);
			return_code=TRUE;
		} break;
		case WM_RBUTTONUP:
		{
			Graphics_buffer_win32_button_callback(&message_identifier,
				graphics_buffer, first_message, second_message);
			ReleaseCapture();
			return_code=TRUE;
		} break;
		case WM_MBUTTONDOWN:
		{
			Graphics_buffer_win32_button_callback(&message_identifier,
				graphics_buffer, first_message, second_message);
			SetCapture(window);
			return_code=TRUE;
		} break;
		case WM_MBUTTONUP:
		{
			Graphics_buffer_win32_button_callback(&message_identifier,
				graphics_buffer, first_message, second_message);
			ReleaseCapture();
			return_code=TRUE;
		} break;
		case WM_MOUSEMOVE:
		{
			Graphics_buffer_win32_button_callback(&message_identifier,
				graphics_buffer, first_message, second_message);
			return_code=TRUE;
		} break;
		case WM_ERASEBKGND:
		{
#if defined (DEBUG_CODE)
			printf("WM_ERASEBKGND\n");
#endif /* defined (DEBUG_CODE) */
			return_code=TRUE;
		} break;
		case WM_WINDOWPOSCHANGED:
		{
#if defined (DEBUG_CODE)
			printf("WM_WINDOWPOSCHANGED\n");
#endif /* defined (DEBUG_CODE) */
			return_code=TRUE;
		} break;
		case WM_MOVE:
		{
#if defined (DEBUG_CODE)
			printf("WM_MOVE\n");
#endif /* defined (DEBUG_CODE) */
			return_code=0;
		} break;
		case WM_SIZE:
		{
#if defined (DEBUG_CODE)
			printf("WM_SIZE\n");
#endif /* defined (DEBUG_CODE) */
			return_code=0;
		} break;
		default:
		{
			return_code=DefWindowProc(window,message_identifier,first_message,
				second_message);
		} break;
	}
	LEAVE;

	return (return_code);
} /* Graphics_buffer_callback_proc */
#endif /* defined (WIN32_USER_INTERFACE) */

#if defined (WIN32_USER_INTERFACE)
int Graphics_buffer_win32_use_font_bitmaps(struct Graphics_buffer *buffer,
	HFONT font, int first_bitmap, int number_of_bitmaps, int display_list_offset)
/*******************************************************************************
LAST MODIFIED : 17 November 2005

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Graphics_buffer_win32_use_font_bitmaps);

	if (buffer)
	{
		switch (buffer->type)
		{
			case GRAPHICS_BUFFER_WIN32_TYPE:
			case GRAPHICS_BUFFER_WIN32_COPY_PBUFFER_TYPE:
			case GRAPHICS_BUFFER_WIN32_COPY_BITMAP_TYPE:
			{
				SelectObject(buffer->hDC, font);
				BOOL result = wglUseFontBitmaps(buffer->hDC, first_bitmap, number_of_bitmaps,
					display_list_offset);
				if (!result)
					result = wglUseFontBitmaps(buffer->hDC, first_bitmap, number_of_bitmaps,
						display_list_offset);
				return_code = 1;
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,"Graphics_buffer_win32_use_font_bitmaps.  "
					"This function should only be used with WIN32 type buffers.");
				return_code = 0;
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Graphics_buffer_win32_use_font_bitmaps.  "
			"Graphics_bufffer missing.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Graphics_buffer_win32_use_font_bitmaps */
#endif /* defined (WIN32_USER_INTERFACE) */

#if defined (CARBON_USER_INTERFACE)
static OSStatus Graphics_buffer_mouse_Carbon_callback(
	EventHandlerCallRef handler, EventRef event,
	void* buffer_void)
/*******************************************************************************
LAST MODIFIED : 23 November 2006

DESCRIPTION :
==============================================================================*/
{
	int input_modifier, return_code;
	EventMouseButton button_number;
	HIPoint location;
	struct Graphics_buffer *buffer;
	struct Graphics_buffer_input input;
	UInt32 modifier_keys, event_type;
	Rect global_bounds;

	ENTER(Graphics_buffer_mouse_Carbon_callback);
	USE_PARAMETER(handler);
	if ((buffer=(struct Graphics_buffer *)buffer_void)
		&& event)
	{
		// Figure out if the event is ours by looking at the location
		GetEventParameter (event, kEventParamMouseLocation,
			typeHIPoint, NULL, sizeof(HIPoint), NULL, &location);

#if defined (DEBUG_CODE)
		printf("  Graphics_buffer_mouse_Carbon_callback %lf %lf\n",
			location.x, location.y);
#endif // defined (DEBUG_CODE)

		// Convert from global screen coordinates to window coordinates
		GetWindowBounds(buffer->theWindow,
			kWindowGlobalPortRgn, &global_bounds);

#if defined (DEBUG_CODE)
		printf("    global bounds %d %d\n", global_bounds.top, global_bounds.left);
#endif // defined (DEBUG_CODE)

		location.x -= global_bounds.left;
		location.y -= global_bounds.top;

#if defined (DEBUG_CODE)
		printf("    local coordinates %lf %lf\n", location.x, location.y);
#endif // defined (DEBUG_CODE)

#if defined (DEBUG_CODE)
		printf("    graphics buffer coordinates %lf %lf\n", location.x, location.y);
#endif // defined (DEBUG_CODE)
		printf("x, y, bufferwidth, bufferheight: %lf, %lf, %lf, %lf\n", location.x, location.y, buffer->clip_width, buffer->clip_height);
		if ((location.x < 0) ||
			(location.x > buffer->clip_width-2) ||
			(location.y < 0) ||
			(location.y > buffer->clip_height-2))
		{
			// If the event isn't ours, pass it on.
			OSStatus result = CallNextEventHandler(handler, event);
			return(result);
		}
		return_code = 1;
		input.type = GRAPHICS_BUFFER_INVALID_INPUT;
		event_type = GetEventKind(event);
		switch (event_type)
		{
			case kEventMouseDown:
			{
				input.type = GRAPHICS_BUFFER_BUTTON_PRESS;
			} break;
			case kEventMouseUp:
			{
				input.type = GRAPHICS_BUFFER_BUTTON_RELEASE;
			} break;
			case kEventMouseDragged:
			{
				input.type = GRAPHICS_BUFFER_MOTION_NOTIFY;
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"Graphics_buffer_gtkglarea_button_callback.  Unknown button event");
				return_code=0;
				/* This event type is not being passed on */
			} break;
		}
		input.key_code = 0;

		GetEventParameter (event, kEventParamMouseButton,
			typeMouseButton, NULL, sizeof(EventMouseButton), NULL, &button_number);
		input.button_number = button_number;

		input.position_x = (int)location.x;
		input.position_y = (int)location.y;
		input_modifier = 0;

		GetEventParameter (event, kEventParamKeyModifiers,
				   typeUInt32, NULL,
				   sizeof(modifier_keys), NULL,
				   &modifier_keys);

		if (modifier_keys & shiftKey)
		{
			input_modifier |= GRAPHICS_BUFFER_INPUT_MODIFIER_SHIFT;
		}
		if (modifier_keys & controlKey)
		{
			input_modifier |= GRAPHICS_BUFFER_INPUT_MODIFIER_CONTROL;
		}
		if (modifier_keys & optionKey)
		{
			input_modifier |= GRAPHICS_BUFFER_INPUT_MODIFIER_ALT;
		}
		if (button_number == kEventMouseButtonPrimary)
		{
			input_modifier |= GRAPHICS_BUFFER_INPUT_MODIFIER_BUTTON1;
		}
		if (return_code)
		{
			input.input_modifier = static_cast<enum Graphics_buffer_input_modifier>
				(input_modifier);
			CMZN_CALLBACK_LIST_CALL(Graphics_buffer_input_callback)(
				buffer->input_callback_list, buffer, &input);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_buffer_gtkglarea_button_callback.  Invalid argument(s)");
	}
	LEAVE;

	return (TRUE);
} /* Graphics_buffer_mouse_Carbon_callback */
#endif /* defined (CARBON_USER_INTERFACE) */

#if defined (CARBON_USER_INTERFACE)
static OSStatus Graphics_buffer_expose_Carbon_callback(
	EventHandlerCallRef handler, EventRef event,
	void* buffer_void)
/*******************************************************************************
LAST MODIFIED : 23 November 2006

DESCRIPTION :
==============================================================================*/
{
	struct Graphics_buffer *graphics_buffer;

	USE_PARAMETER(handler);
	USE_PARAMETER(event);

	OSStatus result = eventNotHandledErr;

	if (graphics_buffer = (struct Graphics_buffer *)buffer_void)
	{
		CMZN_CALLBACK_LIST_CALL(Graphics_buffer_callback)(
			graphics_buffer->expose_callback_list, graphics_buffer, NULL);

		result = noErr;

	}
	LEAVE;

	return (result);
} /* Graphics_buffer_expose_Carbon_callback */
#endif /* defined (CARBON_USER_INTERFACE) */

#if defined (CARBON_USER_INTERFACE)
static OSStatus Graphics_buffer_resize_Carbon_callback(
	EventHandlerCallRef handler, EventRef event,
	void* buffer_void)
/*******************************************************************************
LAST MODIFIED : 23 November 2006

DESCRIPTION :
==============================================================================*/
{
	struct Graphics_buffer *graphics_buffer;

	USE_PARAMETER(handler);
	USE_PARAMETER(event);

	OSStatus result = eventNotHandledErr;

	if (graphics_buffer = (struct Graphics_buffer *)buffer_void)
	{
		aglSetWindowRef(graphics_buffer->aglContext, graphics_buffer->theWindow);
		CMZN_CALLBACK_LIST_CALL(Graphics_buffer_callback)(
			graphics_buffer->resize_callback_list, graphics_buffer, NULL);
		CMZN_CALLBACK_LIST_CALL(Graphics_buffer_callback)(
			graphics_buffer->expose_callback_list, graphics_buffer, NULL);

		result = noErr;

	}
	LEAVE;

	return (result);
} /* Graphics_buffer_expose_Carbon_callback */
#endif /* defined (CARBON_USER_INTERFACE) */

#if defined (CARBON_USER_INTERFACE)
struct Graphics_buffer *create_Graphics_buffer_Carbon(
	struct Graphics_buffer_package *graphics_buffer_package,
	WindowRef windowIn,
	enum Graphics_buffer_buffering_mode buffering_mode,
	enum Graphics_buffer_stereo_mode stereo_mode,
	int minimum_colour_buffer_depth, int minimum_depth_buffer_depth,
	int minimum_accumulation_buffer_depth)
/*******************************************************************************
LAST MODIFIED : 21 November 2006

DESCRIPTION :
==============================================================================*/
{
	GLint attributes[] = {
		AGL_MINIMUM_POLICY,
		AGL_RGBA,
		AGL_DOUBLEBUFFER,
		AGL_RED_SIZE, 1,
		AGL_GREEN_SIZE, 1,
		AGL_BLUE_SIZE, 1,
		AGL_ALPHA_SIZE, 1,
		AGL_ACCUM_RED_SIZE, 1,
		AGL_ACCUM_GREEN_SIZE, 1,
		AGL_ACCUM_BLUE_SIZE, 1,
		AGL_ACCUM_ALPHA_SIZE, 1,
		AGL_DEPTH_SIZE, 1,
		AGL_STENCIL_SIZE, 1,
		AGL_NONE};
	/* int accumulation_colour_size; */
	struct Graphics_buffer *buffer;

	ENTER(create_Graphics_buffer_Carbon);
	USE_PARAMETER(buffering_mode);
	USE_PARAMETER(stereo_mode);
	USE_PARAMETER(minimum_colour_buffer_depth);
	USE_PARAMETER(minimum_depth_buffer_depth);
	USE_PARAMETER(minimum_accumulation_buffer_depth);
	if (buffer = CREATE(Graphics_buffer)(graphics_buffer_package))
	{
		buffer->type = GRAPHICS_BUFFER_CARBON_TYPE;
		buffer->theWindow = windowIn;
		Rect contentBounds;
		GetWindowBounds (buffer->theWindow, kWindowContentRgn, &contentBounds);
		buffer->width = contentBounds.right - contentBounds.left;
		buffer->height = contentBounds.bottom - contentBounds.top;
		buffer->clip_width = contentBounds.right - contentBounds.left;
		buffer->clip_height = contentBounds.bottom - contentBounds.top;

		if (buffer->aglPixelFormat = aglChoosePixelFormat(NULL, 0, attributes))
		{
			if (buffer->aglContext = aglCreateContext(buffer->aglPixelFormat, NULL))
			{

				if(aglSetWindowRef(buffer->aglContext, windowIn))
				{

					if (aglSetCurrentContext(buffer->aglContext))
					{
						EventHandlerUPP mouse_handler_UPP,
							expose_handler_UPP, resize_handler_UPP;

						EventTypeSpec expose_event_list[] = {
					{kEventClassWindow, kEventWindowDrawContent}};
						EventTypeSpec resize_event_list[] = {
					{kEventClassWindow, kEventWindowBoundsChanged}};
						EventTypeSpec mouse_event_list[] = {
							{kEventClassMouse, kEventMouseDragged},
							{kEventClassMouse, kEventMouseUp},
							{kEventClassMouse, kEventMouseDown}};

						aglEnable(buffer->aglContext, AGL_BUFFER_RECT);
						//aglEnable(buffer->aglContext, AGL_SWAP_RECT);

						expose_handler_UPP =
							NewEventHandlerUPP (Graphics_buffer_expose_Carbon_callback);

						InstallWindowEventHandler(windowIn,
							expose_handler_UPP, GetEventTypeCount(expose_event_list),
							expose_event_list, buffer, &buffer->expose_handler_ref);

						resize_handler_UPP =
							NewEventHandlerUPP (Graphics_buffer_resize_Carbon_callback);

						InstallWindowEventHandler(windowIn,
							resize_handler_UPP, GetEventTypeCount(resize_event_list),
							resize_event_list, buffer, &buffer->resize_handler_ref);

						mouse_handler_UPP =
							NewEventHandlerUPP (Graphics_buffer_mouse_Carbon_callback);

						InstallWindowEventHandler(windowIn,
							mouse_handler_UPP, GetEventTypeCount(mouse_event_list),
							mouse_event_list, buffer, &buffer->mouse_handler_ref);

					}
					else
					{
						display_message(ERROR_MESSAGE,"create_Graphics_buffer_win32.  "
							"Unable enable the render context.");
						DEACCESS(Graphics_buffer)(&buffer);
						buffer = (struct Graphics_buffer *)NULL;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,"create_Graphics_buffer_win32.  "
						"Unable associate port with context.");
					DEACCESS(Graphics_buffer)(&buffer);
					buffer = (struct Graphics_buffer *)NULL;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,"create_Graphics_buffer_win32.  "
					"Unable to create the render context.");
				DEACCESS(Graphics_buffer)(&buffer);
				buffer = (struct Graphics_buffer *)NULL;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"create_Graphics_buffer_win32.  "
				"Unable to set pixel format.");
			DEACCESS(Graphics_buffer)(&buffer);
			buffer = (struct Graphics_buffer *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"create_Graphics_buffer_win32.  "
			"Unable to create generic Graphics_buffer.");
		buffer = (struct Graphics_buffer *)NULL;
	}
	LEAVE;

	return (buffer);
} /* create_Graphics_buffer_Carbon */
#endif /* defined (CARBON_USER_INTERFACE) */

#if defined (CARBON_USER_INTERFACE)
int Graphics_buffer_carbon_set_window_size(struct Graphics_buffer *buffer,
	int width, int height, int clip_width, int clip_height)
/*******************************************************************************
LAST MODIFIED : 16 February 2007

DESCRIPTION :
Sets the coordinates within the graphics port which the graphics_buffer should
respect.
==============================================================================*/
{
	int return_code;

	ENTER(cmzn_graphics_buffer_get_near_and_far_plane);
	if (buffer)
	{
		buffer->width = width;
		buffer->height = height;
		buffer->clip_width = clip_width;
		buffer->clip_height = clip_height;
	}
	else
	{
		display_message(ERROR_MESSAGE,"cmzn_graphics_buffer_carbon_set_window_size.  "
			"Missing graphics_buffer parameter.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* cmzn_graphics_buffer_carbon_set_window_size */
#endif /* defined (CARBON_USER_INTERFACE) */

struct Graphics_buffer *Graphics_buffer_app_get_core_buffer(struct Graphics_buffer_app *buffer)
{
	return buffer->core_buffer;
}

#if defined (WX_USER_INTERFACE)
Graphics_buffer_app *create_Graphics_buffer_wx(
	struct Graphics_buffer_app_package *graphics_buffer_package,
	wxPanel *parent,
	enum Graphics_buffer_buffering_mode buffering_mode,
	enum Graphics_buffer_stereo_mode stereo_mode,
	int minimum_colour_buffer_depth, int minimum_depth_buffer_depth,
	int minimum_accumulation_buffer_depth,
	struct Graphics_buffer_app  *buffer_to_match)
{
	 struct Graphics_buffer_app *buffer;

	ENTER(create_Graphics_buffer_wx);
	buffer = CREATE(Graphics_buffer_app)(graphics_buffer_package,
		GRAPHICS_BUFFER_ONSCREEN_TYPE, buffering_mode, stereo_mode);
	if (buffer != NULL)
	{
		 Graphics_buffer_create_buffer_wx(buffer, graphics_buffer_package,
				parent, buffering_mode, stereo_mode, minimum_colour_buffer_depth,
				minimum_depth_buffer_depth, minimum_accumulation_buffer_depth, 0, 0,
				buffer_to_match);
	}
	else
	{
		display_message(ERROR_MESSAGE,"create_Graphics_buffer_wx.  "
			"Unable to create generic Graphics_buffer.");
		buffer = (struct Graphics_buffer_app *)NULL;
	}
	LEAVE;

	return (buffer);
}

#endif /* defined (WX_USER_INTERFACE) */

int Graphics_buffer_app_make_current(struct Graphics_buffer_app *buffer)
/*******************************************************************************
LAST MODIFIED : 2 July 2002

DESCRIPTION :
==============================================================================*/
{
	int return_code = 0;

	ENTER(Graphics_buffer_app_make_current);

	if (buffer)
	{
#if defined (DEBUG_CODE)
		printf("Graphics_buffer_app_make_current\n");
#endif /* defined (DEBUG_CODE) */
		switch (buffer->core_buffer->type)
		{
#if defined (GTK_USER_INTERFACE)
#if defined (GTK_USE_GTKGLAREA)
			case GRAPHICS_BUFFER_GTKGLAREA_TYPE:
			{
				gtk_gl_area_make_current(GTK_GL_AREA(buffer->glarea));
				return_code = 1;
			} break;
#else /* defined (GTK_USE_GTKGLAREA) */
			case GRAPHICS_BUFFER_GTKGLEXT_TYPE:
			{
#if defined (DEBUG_CODE)
				printf("Graphics_buffer_app_make_current %p %p\n",
					buffer->gldrawable, buffer->glcontext);
#endif /* defined (DEBUG_CODE) */
				gdk_gl_drawable_make_current(buffer->gldrawable, buffer->glcontext);
				return_code = 1;
			} break;
#endif /* defined (GTK_USE_GTKGLAREA) */
#endif /* defined (GTK_USER_INTERFACE) */
#if defined (WIN32_USER_INTERFACE)
			case GRAPHICS_BUFFER_WIN32_COPY_PBUFFER_TYPE:
			{
				/* If this context has been made current then we need to copy the pixels
				 * before the next paint.
				 */
				if (!buffer->offscreen_render_required)
					buffer->offscreen_render_required = 2;
			} /* no break */
			case GRAPHICS_BUFFER_WIN32_TYPE:
			case GRAPHICS_BUFFER_WIN32_COPY_BITMAP_TYPE:
			{
#if defined (DEBUG_CODE)
				printf("Graphics_buffer_app_make_current %p %p\n",
					buffer->hDC, buffer->hRC);
#endif /* defined (DEBUG_CODE) */
				wglMakeCurrent( buffer->hDC, buffer->hRC );
				return_code = 1;
			} break;
#endif /* defined (WIN32_USER_INTERFACE) */
#if defined (CARBON_USER_INTERFACE)
			case GRAPHICS_BUFFER_CARBON_TYPE:
			{
				GLint parms[4] ;
				Rect port_bounds;
				GetWindowBounds (buffer->theWindow, kWindowContentRgn, &port_bounds);
				buffer->clip_width = port_bounds.right - port_bounds.left;
				buffer->clip_height = port_bounds.bottom - port_bounds.top;
				parms[0] = 0;
				parms[1] = 0;
				parms[2] = buffer->clip_width;
				parms[3] = buffer->clip_height;
				aglSetInteger(buffer->aglContext, AGL_BUFFER_RECT, parms) ;
				aglUpdateContext(buffer->aglContext);
				aglSetCurrentContext(buffer->aglContext);
				return_code = 1;
			} break;
#endif /* defined (CARBON_USER_INTERFACE) */
#if defined (WX_USER_INTERFACE)
			case GRAPHICS_BUFFER_ONSCREEN_TYPE:
			{
				 if (buffer->canvas)
				 {
						buffer->canvas->SetCurrent();
						return_code = 1;
				 }
				 else
				 {
						return_code = 0;
				 }
			} break;
			 case GRAPHICS_BUFFER_WX_OFFSCREEN_TYPE:
			{
				return_code = 0;
			} break;
			case GRAPHICS_BUFFER_GL_EXT_FRAMEBUFFER_TYPE:
			{
#if defined (OPENGL_API) && defined (GL_EXT_framebuffer_object)
				Graphics_buffer_bind_framebuffer(buffer->core_buffer);
#endif
			} break;
#endif /* defined (WX_USER_INTERFACE) */
			default:
			{
				display_message(ERROR_MESSAGE,"Graphics_buffer_app_make_current.  "
					"Graphics_bufffer type unknown or not supported.");
				return_code = 0;
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Graphics_buffer_app_make_current.  "
			"Graphics_bufffer missing.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Graphics_buffer_app_make_current */

int Graphics_buffer_get_visual_id(Graphics_buffer_app *buffer, int *visual_id)
/*******************************************************************************
LAST MODIFIED : 19 September 2002

DESCRIPTION :
Returns the visual id used by the graphics buffer.
==============================================================================*/
{
	int return_code;

	ENTER(Graphics_buffer_get_visual_id);

	if (buffer)
	{
		switch (buffer->core_buffer->type)
		{
#if defined (GTK_USER_INTERFACE)
#if defined (GTK_USE_GTKGLAREA)
			case GRAPHICS_BUFFER_GTKGLAREA_TYPE:
			{
				*visual_id = 0;
				return_code = 0;
			} break;
#else /* defined (GTK_USE_GTKGLAREA) */
			case GRAPHICS_BUFFER_GTKGLEXT_TYPE:
			{
				*visual_id = 0;
				return_code = 0;
			} break;
#endif /* defined (GTK_USE_GTKGLAREA) */
#endif /* defined (GTK_USER_INTERFACE) */
#if defined (WIN32_USER_INTERFACE)
			case GRAPHICS_BUFFER_WIN32_TYPE:
			case GRAPHICS_BUFFER_WIN32_COPY_PBUFFER_TYPE:
			case GRAPHICS_BUFFER_WIN32_COPY_BITMAP_TYPE:
			{
				*visual_id = buffer->pixel_format;
				return_code = 0;
			} break;
#endif /* defined (WIN32_USER_INTERFACE) */
#if defined (CARBON_USER_INTERFACE)
			case GRAPHICS_BUFFER_CARBON_TYPE:
			{
				*visual_id = 0;
				return_code = 0;
			} break;
#endif /* defined (CARBON_USER_INTERFACE) */
#if defined (WX_USER_INTERFACE)
			case GRAPHICS_BUFFER_ONSCREEN_TYPE:
			{
#if defined (__WXGTK__)
				*visual_id = (int)((XVisualInfo*)buffer->canvas->GetXVisualInfo())
					->visualid;
				return_code = 1;
#else /* if defined (__WXGTK__) */
				*visual_id = 0;
				return_code = 0;
#endif /* if defined (__WXGTK__) */
			} break;
#endif /* defined (WX_USER_INTERFACE) */
			default:
			{
				display_message(ERROR_MESSAGE,"Graphics_buffer_get_visual_id.  "
					"Graphics_bufffer type unknown or not supported.");
				return_code = 0;
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Graphics_buffer_get_visual_id.  "
			"Graphics_bufffer missing.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Graphics_buffer_get_visual_id */

int Graphics_buffer_get_colour_buffer_depth(struct Graphics_buffer_app *buffer,
	int *colour_buffer_depth)
/*******************************************************************************
LAST MODIFIED : 19 September 2002

DESCRIPTION :
Returns the depth of the colour buffer used by the graphics buffer.
==============================================================================*/
{
	int return_code = 0;

	ENTER(Graphics_buffer_get_colour_buffer_depth);
	if (buffer)
	{
		return_code = 1;
		switch (buffer->core_buffer->type)
		{
#if defined (WX_USER_INTERFACE)
			 case GRAPHICS_BUFFER_ONSCREEN_TYPE:
			{
				 GLint colour_buffer_bits;
				 glGetIntegerv(GL_RED_BITS, &colour_buffer_bits);
				 *colour_buffer_depth = colour_buffer_bits;
				 glGetIntegerv(GL_BLUE_BITS, &colour_buffer_bits);
				 *colour_buffer_depth += colour_buffer_bits;
				 glGetIntegerv(GL_GREEN_BITS, &colour_buffer_bits);
				 *colour_buffer_depth += colour_buffer_bits;
				 glGetIntegerv(GL_ALPHA_BITS, &colour_buffer_bits);
				 *colour_buffer_depth += colour_buffer_bits;
				 return_code = 1;
			} break;
#endif /* defined (WX_USER_INTERFACE) */
#if defined (GTK_USER_INTERFACE)
#if defined (GTK_USE_GTKGLAREA)
			case GRAPHICS_BUFFER_GTKGLAREA_TYPE:
			{
				*colour_buffer_depth = 0;
				return_code = 0;
			} break;
#else /* defined (GTK_USE_GTKGLAREA) */
			case GRAPHICS_BUFFER_GTKGLEXT_TYPE:
			{
				gdk_gl_config_get_attrib(buffer->glconfig, GDK_GL_BUFFER_SIZE,
					colour_buffer_depth);
				return_code = 1;
			} break;
#endif /* defined (GTK_USE_GTKGLAREA) */
#endif /* defined (GTK_USER_INTERFACE) */
#if defined (WIN32_USER_INTERFACE)
			case GRAPHICS_BUFFER_WIN32_TYPE:
			case GRAPHICS_BUFFER_WIN32_COPY_PBUFFER_TYPE:
			case GRAPHICS_BUFFER_WIN32_COPY_BITMAP_TYPE:
			{
				PIXELFORMATDESCRIPTOR pfd;
				ZeroMemory( &pfd, sizeof( PIXELFORMATDESCRIPTOR ) );
				pfd.nSize = sizeof( pfd );
				pfd.nVersion = 1;

				DescribePixelFormat(buffer->hDC, buffer->pixel_format, pfd.nSize, &pfd);

				*colour_buffer_depth = pfd.cColorBits;
			} break;
#endif /* defined (WIN32_USER_INTERFACE) */
#if defined (CARBON_USER_INTERFACE)
			case GRAPHICS_BUFFER_CARBON_TYPE:
			{



				 aglDescribePixelFormat (buffer->aglPixelFormat, AGL_RED_SIZE, &colour_buffer_bits);



				 aglDescribePixelFormat (buffer->aglPixelFormat, AGL_GREEN_SIZE, &colour_buffer_bits);



				 aglDescribePixelFormat (buffer->aglPixelFormat, AGL_BLUE_SIZE, &colour_buffer_bits);



				 aglDescribePixelFormat (buffer->aglPixelFormat, AGL_ALPHA_SIZE, &colour_buffer_bits);

				 return_code = 1;
			} break;
#endif /* defined (CARBON_USER_INTERFACE) */
			default:
			{
				display_message(ERROR_MESSAGE,"Graphics_buffer_get_colour_buffer_depth.  "
					"Graphics_buffer type unknown or not supported.");
				*colour_buffer_depth = 0;
				return_code = 0;
			} break;
		}

	}
	else
	{
		display_message(ERROR_MESSAGE,"Graphics_buffer_get_colour_buffer_depth.  "
			"Graphics_bufffer missing.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Graphics_buffer_get_colour_buffer_depth */

int Graphics_buffer_get_depth_buffer_depth(struct Graphics_buffer_app *buffer,
	int *depth_buffer_depth)
{
	int return_code = 0;
	ENTER(Graphics_buffer_get_depth_buffer_depth);

	if (buffer)
	{
		return_code = 1;
		switch (buffer->core_buffer->type)
		{
#if defined (WX_USER_INTERFACE)
			 case GRAPHICS_BUFFER_ONSCREEN_TYPE:
			{
				 GLint depth_bits;
				 glGetIntegerv(GL_DEPTH_BITS, &depth_bits);
				 *depth_buffer_depth = depth_bits;
				 return_code = 1;
			} break;
#endif /* defined (WX_USER_INTERFACE) */
#if defined (GTK_USER_INTERFACE)
#if defined (GTK_USE_GTKGLAREA)
			case GRAPHICS_BUFFER_GTKGLAREA_TYPE:
			{
				*depth_buffer_depth = 0;
				return_code = 0;
			} break;
#else /* defined (GTK_USE_GTKGLAREA) */
			case GRAPHICS_BUFFER_GTKGLEXT_TYPE:
			{
				gdk_gl_config_get_attrib(buffer->glconfig, GDK_GL_DEPTH_SIZE,
					depth_buffer_depth);
				return_code = 1;
			} break;
#endif /* defined (GTK_USE_GTKGLAREA) */
#endif /* defined (GTK_USER_INTERFACE) */
#if defined (WIN32_USER_INTERFACE)
			case GRAPHICS_BUFFER_WIN32_TYPE:
			case GRAPHICS_BUFFER_WIN32_COPY_PBUFFER_TYPE:
			case GRAPHICS_BUFFER_WIN32_COPY_BITMAP_TYPE:
			{
				PIXELFORMATDESCRIPTOR pfd;
				ZeroMemory( &pfd, sizeof( PIXELFORMATDESCRIPTOR ) );
				pfd.nSize = sizeof( pfd );
				pfd.nVersion = 1;

				DescribePixelFormat(buffer->hDC, buffer->pixel_format, pfd.nSize, &pfd);

				*depth_buffer_depth = pfd.cDepthBits;
			} break;
#endif /* defined (WIN32_USER_INTERFACE) */
#if defined (CARBON_USER_INTERFACE)
			case GRAPHICS_BUFFER_CARBON_TYPE:
			{
				 GLint depth_bits;
				 aglDescribePixelFormat(buffer->aglPixelFormat, AGL_DEPTH_SIZE, &depth_bits);
				 *depth_buffer_depth = depth_bits;
				 return_code = 1;
			} break;
#endif /* defined (CARBON_USER_INTERFACE) */
			default:
			{
				display_message(ERROR_MESSAGE,"Graphics_buffer_get_depth_buffer_depth.  "
					"Graphics_bufffer type unknown or not supported.");
				*depth_buffer_depth = 0;
				return_code = 0;
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Graphics_buffer_get_depth_buffer_depth.  "
			"Graphics_bufffer missing.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Graphics_buffer_get_depth_buffer_depth */

int Graphics_buffer_get_accumulation_buffer_depth(struct Graphics_buffer_app *buffer,
	int *accumulation_buffer_depth)
/*******************************************************************************
LAST MODIFIED : 19 September 2002

DESCRIPTION :
Returns the depth of the accumulation buffer used by the graphics buffer.
==============================================================================*/
{
	int return_code = 0;
	ENTER(Graphics_buffer_get_accumulation_buffer_depth);

	if (buffer)
	{
		return_code = 1;
		switch (buffer->core_buffer->type)
		{
#if defined (WX_USER_INTERFACE)
			 case GRAPHICS_BUFFER_ONSCREEN_TYPE:
			{
				 GLint accumulation_buffer_bits;
				 glGetIntegerv(GL_ACCUM_RED_BITS, &accumulation_buffer_bits);
				 *accumulation_buffer_depth = accumulation_buffer_bits;
				 glGetIntegerv(GL_ACCUM_BLUE_BITS, &accumulation_buffer_bits);
				 *accumulation_buffer_depth += accumulation_buffer_bits;
				 glGetIntegerv(GL_ACCUM_GREEN_BITS, &accumulation_buffer_bits);
				 *accumulation_buffer_depth += accumulation_buffer_bits;
				 glGetIntegerv(GL_ACCUM_ALPHA_BITS, &accumulation_buffer_bits);
				 *accumulation_buffer_depth += accumulation_buffer_bits;
				 return_code = 1;
			} break;
#endif /* defined (WX_USER_INTERFACE) */
#if defined (GTK_USER_INTERFACE)
#if defined (GTK_USE_GTKGLAREA)
			case GRAPHICS_BUFFER_GTKGLAREA_TYPE:
			{
				 *accumulation_buffer_depth = 0;
				return_code = 0;
			} break;
#else /* defined (GTK_USE_GTKGLAREA) */
			case GRAPHICS_BUFFER_GTKGLEXT_TYPE:
			{
				int accum_red_size, accum_green_size, accum_blue_size, accum_alpha_size;
				gdk_gl_config_get_attrib(buffer->glconfig, GDK_GL_ACCUM_RED_SIZE,
					&accum_red_size);
				gdk_gl_config_get_attrib(buffer->glconfig, GDK_GL_ACCUM_GREEN_SIZE,
					&accum_green_size);
				gdk_gl_config_get_attrib(buffer->glconfig, GDK_GL_ACCUM_BLUE_SIZE,
					&accum_blue_size);
				gdk_gl_config_get_attrib(buffer->glconfig, GDK_GL_ACCUM_ALPHA_SIZE,
					&accum_alpha_size);
				*accumulation_buffer_depth = accum_red_size + accum_green_size +
					accum_blue_size + accum_alpha_size;
				return_code = 1;
			} break;
#endif /* defined (GTK_USE_GTKGLAREA) */
#endif /* defined (GTK_USER_INTERFACE) */
#if defined (WIN32_USER_INTERFACE)
			case GRAPHICS_BUFFER_WIN32_TYPE:
			case GRAPHICS_BUFFER_WIN32_COPY_PBUFFER_TYPE:
			case GRAPHICS_BUFFER_WIN32_COPY_BITMAP_TYPE:
			{
				PIXELFORMATDESCRIPTOR pfd;
				ZeroMemory( &pfd, sizeof( PIXELFORMATDESCRIPTOR ) );
				pfd.nSize = sizeof( pfd );
				pfd.nVersion = 1;

				DescribePixelFormat(buffer->hDC, buffer->pixel_format, pfd.nSize, &pfd);

				*accumulation_buffer_depth = pfd.cAccumBits;
			} break;
#endif /* defined (WIN32_USER_INTERFACE) */
#if defined (CARBON_USER_INTERFACE)
			case GRAPHICS_BUFFER_CARBON_TYPE:
			{
				 GLint accumulation_buffer_bits;
				 aglDescribePixelFormat (buffer->aglPixelFormat, AGL_ACCUM_RED_SIZE, &accumulation_buffer_bits);
				 *accumulation_buffer_depth = accumulation_buffer_bits;
				 aglDescribePixelFormat (buffer->aglPixelFormat, AGL_ACCUM_GREEN_SIZE, &accumulation_buffer_bits);
				 *accumulation_buffer_depth += accumulation_buffer_bits;
				 aglDescribePixelFormat (buffer->aglPixelFormat, AGL_ACCUM_BLUE_SIZE, &accumulation_buffer_bits);
				 *accumulation_buffer_depth += accumulation_buffer_bits;
				 aglDescribePixelFormat (buffer->aglPixelFormat, AGL_ACCUM_ALPHA_SIZE, &accumulation_buffer_bits);
				 *accumulation_buffer_depth += accumulation_buffer_bits;
				 return_code = 1;
			} break;
#endif /* defined (CARBON_USER_INTERFACE) */
			default:
			{
				display_message(ERROR_MESSAGE,"Graphics_buffer_get_accumulation_buffer_depth.  "
					"Graphics_bufffer type unknown or not supported.");
				*accumulation_buffer_depth = 0;
				return_code = 0;
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Graphics_buffer_get_accumulation_buffer_depth.  "
			"Graphics_bufffer missing.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Graphics_buffer_get_accumulation_buffer_depth */

int Graphics_buffer_get_buffering_mode(struct Graphics_buffer_app *buffer,
	enum Graphics_buffer_buffering_mode *buffering_mode)
/*******************************************************************************
LAST MODIFIED : 19 September 2002

DESCRIPTION :
Returns the buffering mode used by the graphics buffer.
==============================================================================*/
{
	int return_code;

	ENTER(Graphics_buffer_get_buffering_mode);
#if defined (GTK_USER_INTERFACE)
	USE_PARAMETER(buffering_mode);
#endif /* defined (GTK_USER_INTERFACE) */

	if (buffer)
	{
		switch (buffer->core_buffer->type)
		{
#if defined (GTK_USER_INTERFACE)
#if defined (GTK_USE_GTKGLAREA)
			case GRAPHICS_BUFFER_GTKGLAREA_TYPE:
			{

			} break;
#else /* defined (GTK_USE_GTKGLAREA) */
			case GRAPHICS_BUFFER_GTKGLEXT_TYPE:
			{
				if (gdk_gl_config_is_double_buffered(buffer->glconfig))
				{
					*buffering_mode = GRAPHICS_BUFFER_DOUBLE_BUFFERING;
				}
				else
				{
					*buffering_mode = GRAPHICS_BUFFER_SINGLE_BUFFERING;
				}
				return_code = 1;
			} break;
#endif /* defined (GTK_USE_GTKGLAREA) */
#endif /* defined (GTK_USER_INTERFACE) */
#if defined (WIN32_USER_INTERFACE)
			case GRAPHICS_BUFFER_WIN32_TYPE:
			case GRAPHICS_BUFFER_WIN32_COPY_PBUFFER_TYPE:
			case GRAPHICS_BUFFER_WIN32_COPY_BITMAP_TYPE:
			{
				PIXELFORMATDESCRIPTOR pfd;
				int iPixelFormat;
				iPixelFormat = GetPixelFormat(buffer->hDC);
				DescribePixelFormat(buffer->hDC, iPixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &pfd);
				if(pfd.dwFlags & PFD_DOUBLEBUFFER)
				{
					*buffering_mode = GRAPHICS_BUFFER_DOUBLE_BUFFERING;
				}
				else
				{
					*buffering_mode = GRAPHICS_BUFFER_SINGLE_BUFFERING;
				}
				return_code = 1;
			} break;
#endif /* defined (WIN32_USER_INTERFACE) */
#if defined (CARBON_USER_INTERFACE)
			case GRAPHICS_BUFFER_CARBON_TYPE:
			{
				GLint value;

				if (aglDescribePixelFormat (buffer->aglPixelFormat,
						AGL_DOUBLEBUFFER, &value))
				{
					if (value)
					{
						*buffering_mode = GRAPHICS_BUFFER_DOUBLE_BUFFERING;
					}
					else
					{
						*buffering_mode = GRAPHICS_BUFFER_SINGLE_BUFFERING;
					}
					return_code = 1;
				}
				else
				{
					display_message(ERROR_MESSAGE,"Graphics_buffer_get_buffering_mode.  "
						"Invalid Pixel Format Query.");
					return_code = 0;
				}
			} break;
#endif /* defined (CARBON_USER_INTERFACE) */
#if defined (WX_USER_INTERFACE)
			case GRAPHICS_BUFFER_ONSCREEN_TYPE:
			{
				*buffering_mode = GRAPHICS_BUFFER_DOUBLE_BUFFERING;
				return_code = 1;
			} break;
#endif /* defined (WX_USER_INTERFACE) */
			default:
			{
				display_message(ERROR_MESSAGE,"Graphics_buffer_get_buffering_mode.  "
					"Graphics_bufffer type unknown or not supported.");
				return_code = 0;
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Graphics_buffer_get_buffering_mode.  "
			"Graphics_bufffer missing.");
		return_code = 0;
	}
	LEAVE;


	return (return_code);
} /* Graphics_buffer_get_buffering_mode */

int Graphics_buffer_get_stereo_mode(struct Graphics_buffer_app *buffer,
	enum Graphics_buffer_stereo_mode *stereo_mode)
/*******************************************************************************
LAST MODIFIED : 19 September 2002

DESCRIPTION :
Returns the stereo mode used by the graphics buffer.
==============================================================================*/
{
	int return_code = 0;
	ENTER(Graphics_buffer_get_stereo_mode);
#if defined (GTK_USER_INTERFACE)
	USE_PARAMETER(stereo_mode);
#endif /* defined (GTK_USER_INTERFACE) */

	if (buffer)
	{
		switch (buffer->core_buffer->type)
		{
#if defined (GTK_USER_INTERFACE)
#if defined (GTK_USE_GTKGLAREA)
			case GRAPHICS_BUFFER_GTKGLAREA_TYPE:
			{

			} break;
#else /* defined (GTK_USE_GTKGLAREA) */
			case GRAPHICS_BUFFER_GTKGLEXT_TYPE:
			{
				if (gdk_gl_config_is_stereo(buffer->glconfig))
				{
					*stereo_mode = GRAPHICS_BUFFER_STEREO;
				}
				else
				{
					*stereo_mode = GRAPHICS_BUFFER_MONO;
				}
				return_code = 1;
			} break;
#endif /* defined (GTK_USE_GTKGLAREA) */
#endif /* defined (GTK_USER_INTERFACE) */
#if defined (WIN32_USER_INTERFACE)
			case GRAPHICS_BUFFER_WIN32_TYPE:
			case GRAPHICS_BUFFER_WIN32_COPY_PBUFFER_TYPE:
			case GRAPHICS_BUFFER_WIN32_COPY_BITMAP_TYPE:
			{
				PIXELFORMATDESCRIPTOR pfd;
				int iPixelFormat;
				iPixelFormat = GetPixelFormat(buffer->hDC);
				DescribePixelFormat(buffer->hDC, iPixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &pfd);
				if(pfd.dwFlags & PFD_STEREO)
				{
					*stereo_mode = GRAPHICS_BUFFER_STEREO;
				}
				else
				{
					*stereo_mode = GRAPHICS_BUFFER_MONO;
				}
				return_code = 1;
			} break;
#endif /* defined (WIN32_USER_INTERFACE) */
#if defined (CARBON_USER_INTERFACE)
			case GRAPHICS_BUFFER_CARBON_TYPE:
			{
				GLint value;

				if (aglDescribePixelFormat (buffer->aglPixelFormat,
						AGL_STEREO, &value))
				{
					if (value)
					{
						*stereo_mode = GRAPHICS_BUFFER_STEREO;
					}
					else
					{
						*stereo_mode = GRAPHICS_BUFFER_MONO;
					}
					return_code = 1;
				}
				else
				{
					display_message(ERROR_MESSAGE,"Graphics_buffer_get_buffering_mode.  "
						"Invalid Pixel Format Query.");
					return_code = 0;
				}
			} break;
#endif /* defined (CARBON_USER_INTERFACE) */
#if defined (WX_USER_INTERFACE)
			case GRAPHICS_BUFFER_ONSCREEN_TYPE:
			{
				*stereo_mode = GRAPHICS_BUFFER_MONO;
				return_code = 1;
			} break;
#endif /* defined (WX_USER_INTERFACE) */

			default:
			{
				display_message(ERROR_MESSAGE,"Graphics_buffer_get_stereo_mode.  "
					"Graphics_bufffer type unknown or not supported.");
				return_code = 0;
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Graphics_buffer_get_stereo_mode.  "
			"Graphics_bufffer missing.");
		return_code = 0;
	}

	LEAVE;

	return (return_code);
} /* Graphics_buffer_get_stereo_mode */

int Graphics_buffer_app_swap_buffers(struct Graphics_buffer_app *buffer)
/*******************************************************************************
LAST MODIFIED : 2 July 2002

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Graphics_buffer_swap_buffers);

	if (buffer)
	{
		switch (buffer->core_buffer->type)
		{
#if defined (GTK_USER_INTERFACE)
#if defined (GTK_USE_GTKGLAREA)
			case GRAPHICS_BUFFER_GTKGLAREA_TYPE:
			{
				gtk_gl_area_swapbuffers(GTK_GL_AREA(buffer->glarea));
				return_code = 1;
			} break;
#else /* defined (GTK_USE_GTKGLAREA) */
			case GRAPHICS_BUFFER_GTKGLEXT_TYPE:
			{
				gdk_gl_drawable_swap_buffers(buffer->gldrawable);
				return_code = 1;
			} break;
#endif /* defined (GTK_USE_GTKGLAREA) */
#endif /* defined (GTK_USER_INTERFACE) */
#if defined (WIN32_USER_INTERFACE)
			case GRAPHICS_BUFFER_WIN32_TYPE:
			case GRAPHICS_BUFFER_WIN32_COPY_PBUFFER_TYPE:
			case GRAPHICS_BUFFER_WIN32_COPY_BITMAP_TYPE:
			{
				/* Seems to help the OpenGL context get updated to the right place in Vista */
				SetWindowPos(buffer->hWnd, HWND_TOP, 0, 0, 0, 0,
					SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
				SwapBuffers(buffer->hDC);
				return_code = 1;
			} break;
#endif /* defined (WIN32_USER_INTERFACE) */
#if defined (WX_USER_INTERFACE)
			case GRAPHICS_BUFFER_ONSCREEN_TYPE:
			{
				buffer->canvas->SwapBuffers();
				return_code = 1;
			} break;
#endif /* defined (WX_USER_INTERFACE) */
#if defined (CARBON_USER_INTERFACE)
			case GRAPHICS_BUFFER_CARBON_TYPE:
			{
				aglSwapBuffers(buffer->aglContext);
				return_code = 1;
			} break;
#endif /* defined (CARBON_USER_INTERFACE) */
			default:
			{
				display_message(ERROR_MESSAGE,"Graphics_buffer_swap_buffers.  "
					"Graphics_bufffer type unknown or not supported.");
				return_code = 0;
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Graphics_buffer_swap_buffers.  "
			"Graphics_bufffer missing.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Graphics_buffer_swap_buffers */

int Graphics_buffer_make_read_current(struct Graphics_buffer_app *buffer)
/*******************************************************************************
LAST MODIFIED : 28 May 2004

DESCRIPTION :
Sets this buffer to be the GLX source and the current ThreeDWindow (the one last
made current) to be the GLX destination.
==============================================================================*/
{
	int return_code = 0;

	ENTER(Graphics_buffer_make_read_current);

	if (buffer)
	{

		display_message(ERROR_MESSAGE,"Graphics_buffer_make_read_current.  "
			"Graphics_bufffer type unknown or not supported.");

	}
	else
	{
		display_message(ERROR_MESSAGE,"Graphics_buffer_make_read_current.  "
			"Graphics_bufffer missing.");
	}
	LEAVE;

	return (return_code);
} /* Graphics_buffer_make_read_current */

int Graphics_buffer_get_width(struct Graphics_buffer_app *buffer)


{
	int width;

	ENTER(Graphics_buffer_get_width);

	if (buffer)
	{
		switch (buffer->core_buffer->type)
		{
#if defined (GTK_USER_INTERFACE)
#if defined (GTK_USE_GTKGLAREA)
			case GRAPHICS_BUFFER_GTKGLAREA_TYPE:
			{
				width = buffer->glarea->allocation.width;
			} break;
#else /* defined (GTK_USE_GTKGLAREA) */
			case GRAPHICS_BUFFER_GTKGLEXT_TYPE:
			{
				width = buffer->glarea->allocation.width;
			} break;
#endif /* defined (GTK_USE_GTKGLAREA) */
#endif /* defined (GTKGLAREA) */
#if defined (WIN32_USER_INTERFACE)
			case GRAPHICS_BUFFER_WIN32_TYPE:
			case GRAPHICS_BUFFER_WIN32_COPY_PBUFFER_TYPE:
			case GRAPHICS_BUFFER_WIN32_COPY_BITMAP_TYPE:
			{
				if (buffer->hWnd)
				{
					RECT rect;
					if(GetClientRect(buffer->hWnd, &rect))
					{
						width = rect.right - rect.left;
					}
					else
					{
						display_message(ERROR_MESSAGE,"Graphics_buffer_get_width.  "
							"Failed to get window rectangle");
						width = 0;
					}
				}
				else
				{
					width = buffer->width;
				}
			} break;
#endif /* defined (WIN32_USER_INTERFACE) */
#if defined (WX_USER_INTERFACE)
			case GRAPHICS_BUFFER_ONSCREEN_TYPE:
			{
				int height;
				buffer->canvas->GetClientSize(&width, &height);
			} break;
#endif /* defined (WX_USER_INTERFACE) */
#if defined (CARBON_USER_INTERFACE)
			case GRAPHICS_BUFFER_CARBON_TYPE:
			{
				// Respect the values we have been given
				Rect portRect;
				GetWindowBounds (buffer->theWindow, kWindowContentRgn, &portRect);
				width = portRect.right - portRect.left;
			} break;
#endif /* defined (CARBON_USER_INTERFACE) */
			default:
			{
				display_message(ERROR_MESSAGE,"Graphics_buffer_get_width.  "
					"Graphics_bufffer type unknown or not supported.");
				width = 0;
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_buffer_get_width.  Invalid buffer");
		width = 0;
	}
	LEAVE;

	return (width);
} /* Graphics_buffer_get_width */


int Graphics_buffer_set_width(struct Graphics_buffer_app *buffer, int width)
{
	int return_code = 0;

	ENTER(Graphics_buffer_set_width);
	if (buffer)
	{
		switch (buffer->core_buffer->type)
		{
#if defined (WX_USER_INTERFACE)
			case GRAPHICS_BUFFER_ONSCREEN_TYPE:
			{
				int old_width, height;
				buffer->canvas->GetClientSize(&old_width, &height);
				buffer->canvas->SetClientSize(width, height);
				return_code = 1;
			} break;
#else

#endif /* defined (WX_USER_INTERFACE) */
			default:
			{
				display_message(ERROR_MESSAGE,"Graphics_buffer_set_width.  "
					"Graphics_bufffer type unknown or not supported.");
				return_code = 0;
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_buffer_set_width.  Invalid buffer");
	}
	LEAVE;

	return (return_code);
} /* Graphics_buffer_set_width */


int Graphics_buffer_get_height(struct Graphics_buffer_app *buffer)
{

	int height;

	ENTER(Graphics_buffer_get_height);
	if (buffer)
	{
		switch (buffer->core_buffer->type)
		{
#if defined (GTK_USER_INTERFACE)
#if defined (GTK_USE_GTKGLAREA)
			case GRAPHICS_BUFFER_GTKGLAREA_TYPE:
			{
				height = buffer->glarea->allocation.height;
			} break;
#else /* defined (GTK_USE_GTKGLAREA) */
			case GRAPHICS_BUFFER_GTKGLEXT_TYPE:
			{
				height = buffer->glarea->allocation.height;
			} break;
#endif /* defined (GTK_USE_GTKGLAREA) */
#endif /* defined (GTKGLAREA) */
#if defined (WIN32_USER_INTERFACE)
			case GRAPHICS_BUFFER_WIN32_TYPE:
			case GRAPHICS_BUFFER_WIN32_COPY_PBUFFER_TYPE:
			case GRAPHICS_BUFFER_WIN32_COPY_BITMAP_TYPE:
			{
				if (buffer->hWnd)
				{
					RECT rect;
					if(GetClientRect(buffer->hWnd, &rect))
					{
						height = rect.bottom - rect.top;
					}
					else
					{
						display_message(ERROR_MESSAGE,"Graphics_buffer_get_height.  "
							"Failed to get window rectangle");
						height = 0;
					}
				}
				else
				{

			} break;
#endif /* defined (WIN32_USER_INTERFACE) */
#if defined (WX_USER_INTERFACE)
			case GRAPHICS_BUFFER_ONSCREEN_TYPE:
			{
				int width;
				buffer->canvas->GetClientSize(&width, &height);
			} break;
#endif /* defined (WX_USER_INTERFACE) */
#if defined (CARBON_USER_INTERFACE)
			case GRAPHICS_BUFFER_CARBON_TYPE:
			{
				// Respect the values we have been given
				Rect portRect;
				GetWindowBounds (buffer->theWindow, kWindowContentRgn, &portRect);
				height = portRect.bottom - portRect.top;
			} break;
#endif /* defined (CARBON_USER_INTERFACE) */
			default:
			{
				display_message(ERROR_MESSAGE,"Graphics_buffer_get_height.  "
					"Graphics_bufffer type unknown or not supported.");
				height = 0;
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_buffer_get_height.  Invalid buffer");
		height = 0;
	}
	LEAVE;

	return (height);
} /* Graphics_buffer_get_height */

int Graphics_buffer_set_height(struct Graphics_buffer_app *buffer, int height)
{
	int return_code = 0;

	ENTER(Graphics_buffer_set_height);
	USE_PARAMETER(height);

	if (buffer)
	{
		switch (buffer->core_buffer->type)
		{
#if defined (WX_USER_INTERFACE)
			case GRAPHICS_BUFFER_ONSCREEN_TYPE:
			{
				int width, old_height;
				buffer->canvas->GetClientSize(&width, &old_height);
				buffer->canvas->SetClientSize(width, height);
				return_code = 1;
			} break;
#endif /* defined (WX_USER_INTERFACE) */
			default:
			{
				display_message(ERROR_MESSAGE,"Graphics_buffer_set_height.  "
					"Graphics_bufffer type unknown or not supported.");
				return_code = 0;
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_buffer_set_height.  Invalid buffer");
		return_code = 1;
	}
	LEAVE;

	return (return_code);
} /* Graphics_buffer_set_height */

int Graphics_buffer_is_visible(struct Graphics_buffer_app *buffer)
/*******************************************************************************
LAST MODIFIED : 1 July 2002

DESCRIPTION :
Returns 1 if the <buffer> is visible.  If the scene viewer gets zero from this
routine it will not bother rendering into it, allowing us to avoid rendering
into unmanaged or invisible widgets.
==============================================================================*/
{
	int return_code;

	ENTER(Graphics_buffer_is_visible);

	if (buffer)
	{
		return_code = 0;

		switch (buffer->core_buffer->type)
		{
#if defined (GTK_USER_INTERFACE)
#if defined (GTK_USE_GTKGLAREA)
			case GRAPHICS_BUFFER_GTKGLAREA_TYPE:
			{
				return_code = 1;
			} break;
#else /* defined (GTK_USE_GTKGLAREA) */
			case GRAPHICS_BUFFER_GTKGLEXT_TYPE:
			{
				return_code = 1;
			} break;
#endif /* defined (GTK_USE_GTKGLAREA) */
#endif /* defined (GTK_USER_INTERFACE) */
#if defined (WIN32_USER_INTERFACE)
			case GRAPHICS_BUFFER_WIN32_TYPE:
			case GRAPHICS_BUFFER_WIN32_COPY_PBUFFER_TYPE:
			case GRAPHICS_BUFFER_WIN32_COPY_BITMAP_TYPE:
			{
				return_code = 1;
			} break;
#endif /* defined (WIN32_USER_INTERFACE) */
#if defined (WX_USER_INTERFACE)
			case GRAPHICS_BUFFER_ONSCREEN_TYPE:
			{
				return_code = 1;
			} break;
#endif /* defined (WX_USER_INTERFACE) */
#if defined (CARBON_USER_INTERFACE)
			case GRAPHICS_BUFFER_CARBON_TYPE:

#endif /* defined (CARBON_USER_INTERFACE) */
			default:
			{
				display_message(ERROR_MESSAGE,"Graphics_buffer_is_visible.  "
					"Graphics_bufffer type unknown or not supported.");
				return_code = 0;
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_buffer_is_visible.  Invalid buffer");
		return_code=GRAPHICS_BUFFER_INVALID_TYPE;
	}
	LEAVE;

	return (return_code);
} /* Graphics_buffer_is_visible */

int Graphics_buffer_app_awaken(struct Graphics_buffer_app *buffer)
/*******************************************************************************
LAST MODIFIED : 1 July 2002

DESCRIPTION :
Activates the graphics <buffer>.
==============================================================================*/
{
	int return_code = 0;

	ENTER(Graphics_buffer_awaken);
	if (buffer)
	{
		switch (buffer->core_buffer->type)
		{
#if defined (GTK_USER_INTERFACE)
#if defined (GTK_USE_GTKGLAREA)
			case GRAPHICS_BUFFER_GTKGLAREA_TYPE:
			{
				gtk_widget_show(buffer->glarea);
				return_code = 1;
			} break;
#else /* defined (GTK_USE_GTKGLAREA) */
			case GRAPHICS_BUFFER_GTKGLEXT_TYPE:
			{
				gtk_widget_show(buffer->glarea);
				return_code = 1;
			} break;
#endif /* defined (GTK_USE_GTKGLAREA) */
#endif /* defined (GTK_USER_INTERFACE) */
#if defined (WIN32_USER_INTERFACE)
			case GRAPHICS_BUFFER_WIN32_TYPE:
			case GRAPHICS_BUFFER_WIN32_COPY_PBUFFER_TYPE:
			case GRAPHICS_BUFFER_WIN32_COPY_BITMAP_TYPE:
			{
				return_code = 1;
			} break;
#endif /* defined (WIN32_USER_INTERFACE) */
#if defined (WX_USER_INTERFACE)
			case GRAPHICS_BUFFER_ONSCREEN_TYPE:
			{
				return_code = 1;
			} break;
#endif /* defined (WX_USER_INTERFACE) */
#if defined (CARBON_USER_INTERFACE)
			case GRAPHICS_BUFFER_CARBON_TYPE:
			{
				return_code = 1;
			} break;
#endif /* defined (CARBON_USER_INTERFACE) */
			default:
			{
				display_message(ERROR_MESSAGE,"Graphics_buffer_awaken.  "
					"Graphics_bufffer type unknown or not supported.");
				return_code = 0;
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_buffer_awaken.  Invalid buffer");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Graphics_buffer_awaken */

int Graphics_buffer_add_initialise_callback(struct Graphics_buffer_app *buffer,
	CMZN_CALLBACK_FUNCTION(Graphics_buffer_callback) initialise_callback, void *user_data)
/*******************************************************************************
LAST MODIFIED : 1 July 2002

DESCRIPTION :
Adds an initialise callback to the graphics <buffer>.
==============================================================================*/
{
	int return_code;

	ENTER(Graphics_buffer_awaken);
	if (buffer)
	{
		return_code = CMZN_CALLBACK_LIST_ADD_CALLBACK(Graphics_buffer_callback)(
			buffer->initialise_callback_list, initialise_callback,
			user_data);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_buffer_add_initialise_callback.  Invalid buffer");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Graphics_buffer_add_initialise_callback */

int Graphics_buffer_add_resize_callback(struct Graphics_buffer_app *buffer,
	CMZN_CALLBACK_FUNCTION(Graphics_buffer_callback) resize_callback, void *user_data)
/*******************************************************************************
LAST MODIFIED : 1 July 2002

DESCRIPTION :
Adds an resize callback to the graphics <buffer>.
==============================================================================*/
{
	int return_code;

	ENTER(Graphics_buffer_awaken);
	if (buffer)
	{
		return_code = CMZN_CALLBACK_LIST_ADD_CALLBACK(Graphics_buffer_callback)(
			buffer->resize_callback_list, resize_callback,
			user_data);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_buffer_add_resize_callback.  Invalid buffer");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Graphics_buffer_add_resize_callback */

int Graphics_buffer_app_add_expose_callback(struct Graphics_buffer_app *buffer,
	CMZN_CALLBACK_FUNCTION(Graphics_buffer_callback) expose_callback, void *user_data)
/*******************************************************************************
LAST MODIFIED : 1 July 2002

DESCRIPTION :
Adds an expose callback to the graphics <buffer>.
==============================================================================*/
{
	int return_code;

	ENTER(Graphics_buffer_awaken);
	if (buffer)
	{
		return_code = CMZN_CALLBACK_LIST_ADD_CALLBACK(Graphics_buffer_callback)(
			buffer->expose_callback_list, expose_callback,
			user_data);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_buffer_add_expose_callback.  Invalid buffer");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Graphics_buffer_add_expose_callback */

int Graphics_buffer_app_add_input_callback(struct Graphics_buffer_app *buffer,
	CMZN_CALLBACK_FUNCTION(Graphics_buffer_input_callback) input_callback,
	void *user_data)
/*******************************************************************************
LAST MODIFIED : 1 July 2002

DESCRIPTION :
Adds an input callback to the graphics <buffer>.
==============================================================================*/
{
	int return_code;

	ENTER(Graphics_buffer_awaken);
	if (buffer)
	{
		return_code = CMZN_CALLBACK_LIST_ADD_CALLBACK(Graphics_buffer_input_callback)(
			buffer->input_callback_list, input_callback,
			user_data);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_buffer_add_input_callback.  Invalid buffer");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Graphics_buffer_add_input_callback */


int DESTROY(Graphics_buffer_app)(struct Graphics_buffer_app **buffer_ptr)
/*******************************************************************************
LAST MODIFIED : 1 July 2002

DESCRIPTION :
Closes a Graphics buffer instance
x==============================================================================*/
{
	int return_code;
	struct Graphics_buffer_app *buffer;

	ENTER(DESTROY(Graphics_buffer_app));

	if (buffer_ptr && (buffer = *buffer_ptr))
	{
		return_code=1;
#if defined (GTK_USER_INTERFACE)
#if ! defined (GTK_USE_GTKGLAREA)
			if (buffer->glconfig)
			{
				g_object_unref(buffer->glconfig);
				buffer->glconfig = NULL;
			}

			g_signal_handler_disconnect(GTK_OBJECT(buffer->glarea),
				buffer->initialise_handler_id);
			g_signal_handler_disconnect(GTK_OBJECT(buffer->glarea),
				buffer->resize_handler_id);
			g_signal_handler_disconnect(GTK_OBJECT(buffer->glarea),
				buffer->expose_handler_id);
			g_signal_handler_disconnect(GTK_OBJECT(buffer->glarea),
				buffer->button_press_handler_id);
			g_signal_handler_disconnect(GTK_OBJECT(buffer->glarea),
				buffer->button_release_handler_id);
			g_signal_handler_disconnect(GTK_OBJECT(buffer->glarea),
				buffer->key_press_handler_id);
			g_signal_handler_disconnect(GTK_OBJECT(buffer->glarea),
				buffer->key_release_handler_id);
			g_signal_handler_disconnect(GTK_OBJECT(buffer->glarea),
				buffer->motion_handler_id);
#endif /* defined (GTK_USE_GTKGLAREA) */
#endif /* defined (GTK_USER_INTERFACE) */

		if (buffer->initialise_callback_list)
		{
			DESTROY(LIST(CMZN_CALLBACK_ITEM(Graphics_buffer_callback)))(
				&buffer->initialise_callback_list);
		}
		if (buffer->resize_callback_list)
		{
			DESTROY(LIST(CMZN_CALLBACK_ITEM(Graphics_buffer_callback)))(
				&buffer->resize_callback_list);
		}
		if (buffer->expose_callback_list)
		{
			DESTROY(LIST(CMZN_CALLBACK_ITEM(Graphics_buffer_callback)))(
				&buffer->expose_callback_list);
		}
		if (buffer->input_callback_list)
		{
			DESTROY(LIST(CMZN_CALLBACK_ITEM(Graphics_buffer_input_callback)))(
				&buffer->input_callback_list);
		}
#if defined (WIN32_USER_INTERFACE)
		wglMakeCurrent(NULL, NULL);
		/* Only delete it here if it isn't the share context, otherwise we
			will destroy it when the graphics buffer package is destroyed.
			In the intel workaround case we only want to destroy it at the end. */
		if (buffer->hRC && (buffer->hRC != buffer->package->wgl_shared_context))
		{
#if defined (DEBUG_CODE)
			printf("wglDeleteContext %p\n", buffer->hRC);
#endif //defined (DEBUG_CODE)
			wglDeleteContext(buffer->hRC);
		}
		switch (buffer->type)
		{
			case GRAPHICS_BUFFER_WIN32_TYPE:
			{
				if (buffer->hDC)
				{
					ReleaseDC(buffer->hWnd, buffer->hDC);
				}
			} break;
#ifdef WGL_ARB_pbuffer
			case GRAPHICS_BUFFER_WIN32_COPY_PBUFFER_TYPE:
			{
				Graphics_buffer_make_current(buffer->package->hidden_graphics_buffer);
				if (buffer->pbuffer && buffer->hDC)
				{
					wglReleasePbufferDCARB(buffer->pbuffer, buffer->hDC);
				}
				if (buffer->pbuffer)
				{
					wglDestroyPbufferARB(buffer->pbuffer);
				}
			} /* no break */
#endif /* defined (WGL_ARB_pbuffer) */
			case GRAPHICS_BUFFER_WIN32_COPY_BITMAP_TYPE:
			{
				if (buffer->device_independent_bitmap)
				{
					DeleteObject(buffer->device_independent_bitmap);
				}
				if (buffer->device_independent_bitmap_hdc)
				{
					DeleteDC(buffer->device_independent_bitmap_hdc);
				}
			} break;
		}
#endif /* defined (WIN32_USER_INTERFACE) */
#if defined (CARBON_USER_INTERFACE)
		aglDisable(buffer->aglContext, AGL_BUFFER_RECT);
		if (buffer->expose_handler_ref)
		{
			RemoveEventHandler(buffer->expose_handler_ref);
		}
		if (buffer->mouse_handler_ref)
		{
			RemoveEventHandler(buffer->mouse_handler_ref);
		}
		if (buffer->resize_handler_ref)
		{
			RemoveEventHandler(buffer->resize_handler_ref);
		}
#endif /* defined (CARBON_USER_INTERFACE) */
#if defined (WX_USER_INTERFACE)
		/* Remove reference to this object in wxGraphicsBuffer */
		if (buffer->canvas)
		{
			buffer->canvas->ClearGraphicsBufferReference();
			delete buffer->canvas;
		}

#endif /* defined (WX_USER_INTERFACE) */

		DEACCESS(Graphics_buffer)(&(buffer->core_buffer));
		DEALLOCATE(*buffer_ptr);
		*buffer_ptr = 0;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DEACCESS(Graphics_buffer).  Missing buffer");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DEACCESS(Graphics_buffer) */

int Graphics_buffer_app_is_visible(struct Graphics_buffer_app *buffer)
{
	return Graphics_buffer_is_visible(buffer->core_buffer);
}

int Graphics_buffer_app_add_initialise_callback(struct Graphics_buffer_app *buffer,
	CMZN_CALLBACK_FUNCTION(Graphics_buffer_callback) initialise_callback, void *user_data)
/*******************************************************************************
LAST MODIFIED : 1 July 2002

DESCRIPTION :
Adds an initialise callback to the graphics <buffer>.
==============================================================================*/
{
	int return_code;

	ENTER(Graphics_buffer_awaken);
	if (buffer)
	{
		return_code = CMZN_CALLBACK_LIST_ADD_CALLBACK(Graphics_buffer_callback)(
			buffer->initialise_callback_list, initialise_callback,
			user_data);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_buffer_add_initialise_callback.  Invalid buffer");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Graphics_buffer_add_initialise_callback */

int Graphics_buffer_app_add_resize_callback(struct Graphics_buffer_app *buffer,
	CMZN_CALLBACK_FUNCTION(Graphics_buffer_callback) resize_callback, void *user_data)
/*******************************************************************************
LAST MODIFIED : 1 July 2002

DESCRIPTION :
Adds an resize callback to the graphics <buffer>.
==============================================================================*/
{
	int return_code;

	if (buffer)
	{
		return_code = CMZN_CALLBACK_LIST_ADD_CALLBACK(Graphics_buffer_callback)(
			buffer->resize_callback_list, resize_callback,
			user_data);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_buffer_add_resize_callback.  Invalid buffer");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Graphics_buffer_add_resize_callback */

