/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** x11.c
*/

/*
$Author: lidl $
$Id: x11.c,v 1.1.1.1 1995/02/01 00:25:48 lidl Exp $
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "malloc.h"
#include <stdio.h>
#ifdef SVR4
#include <string.h>
#else
#ifndef SYSV
#include <strings.h>
#endif
#endif
#include <ctype.h>
#include "sysdep.h"
#include "graphics.h"
#include <X11/Xos.h>
#include <X11/Xutil.h>			/* X11 stuff must come last */
#include <X11/Xatom.h>
#include <X11/Xresource.h>
#ifdef KEYPAD_DETECT
#include <X11/keysym.h>
#endif
#include "proto.h"

#ifdef lint
  struct _XRegion {
	  int x;
  };							/* avoid lint complaint */
#endif

#ifdef DEBUG_SYNC
Boolean un_stingy = TRUE;

#else /* DEBUG_SYNC */
Boolean un_stingy = FALSE;

#endif


#ifdef NEED_AUX_FONT

#ifndef MAXPATHLEN
#define MAXPATHLEN 1024
#endif
extern char fontdir[], pathname[];

#endif

extern char *program_name;

char *get_default();
int get_num_default();

#ifdef BATCH_LINES
#ifdef BATCH_COLOR_LINES
XSegment lineBatch[MAX_COLORS][BATCHDEPTH];
int linesBatched[MAX_COLORS] =
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int lineBatchFunc = -20;
int lineBatchWin = -20;

#else /* BATCH_COLOR_LINES */
XSegment lineBatch[BATCHDEPTH];
int linesBatched = 0;
int lineBatchColor = -1;		/* Must be a non-color */
int lineBatchFunc = -20;
int lineBatchWin = -20;

#endif /* BATCH_COLOR_LINES */
#endif
#ifdef BATCH_POINTS
XPoint pointBatch[MAX_COLORS][BATCHPDEPTH];
int pointsBatched[MAX_COLORS] =
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int pointBatchFunc = -20;
int pointBatchWin = -20;

#endif

#define icon_width 64
#define icon_height 14
static unsigned char icon_bits[] =
{
	0x07, 0x98, 0xff, 0xc7, 0x03, 0x0f, 0xe7, 0x30, 0x0f, 0xbc, 0xff, 0xef,
	0x03, 0x1f, 0xef, 0x79, 0x1b, 0x2e, 0x01, 0xe4, 0x06, 0x23, 0x6b, 0x5d,
	0x36, 0x17, 0xde, 0x73, 0x05, 0x2b, 0x6b, 0x6f, 0xec, 0x0b, 0x58, 0xb0,
	0x0b, 0x5b, 0x6b, 0x37, 0xd8, 0x05, 0x58, 0xb8, 0x0a, 0x5b, 0x6b, 0x1b,
	0xb0, 0x02, 0x58, 0xd8, 0x17, 0xab, 0x6b, 0x0d, 0x70, 0x03, 0x58, 0xdc,
	0x17, 0xab, 0x6b, 0x06, 0xb8, 0x06, 0x58, 0x0c, 0x20, 0x4b, 0x6b, 0x0d,
	0xdc, 0x0d, 0x58, 0xee, 0x2f, 0x4b, 0x6b, 0x1b, 0x2e, 0x1b, 0x58, 0x16,
	0x50, 0x8b, 0x6a, 0x35, 0x17, 0x36, 0x58, 0x17, 0x50, 0x8b, 0x6a, 0x69,
	0x0b, 0x2c, 0x58, 0x0b, 0xa0, 0x0b, 0x69, 0x51, 0x06, 0x38, 0x70, 0x0e,
	0xe0, 0x0e, 0xcf, 0x61
};

static char *curptr;

static char *color_titles[MAX_COLORS] =
{
	"background",				/* background */
	"foreground",				/* foreground */
	"color.red",				/* Red Team */
	"color.orange",				/* Orange Team */
	"color.yellow",				/* Yellow Team */
	"color.green",				/* Green Team */
	"color.blue",				/* Blue Team */
	"color.violet",				/* Violet Team */
	"color.neutral",			/* Neutral Team */
	"color.cursor",				/* Cursor, tink, commentator, radar */
	"color.dashed",				/* dashed line for mono tubes */
};

static char *color_classes[MAX_COLORS] =
{
	"BackGround",				/* background */
	"ForeGround",				/* foreground */
	"Color.Team",				/* Red Team */
	"Color.Team",				/* Orange Team */
	"Color.Team",				/* Yellow Team */
	"Color.Team",				/* Green Team */
	"Color.Team",				/* Blue Team */
	"Color.Team",				/* Violet Team */
	"Color.Team",				/* Neutral Team */
	"Color.Cursor",				/* Cursor, tink, commentator, radar */
	"Color.Team",				/* dashed line color */
};

static char *colorname[MAX_COLORS] =
{
	"Black",					/* background */
	"White",					/* foreground */
	"Red",						/* Red Team */
	"#ffa144",				/* Orange Team */
	"Yellow",					/* Yellow Team */
	"Green",					/* Green Team */
#if !defined(_IBMR2)
	"#879dff",				/* Blue Team */
#else
	"Blue",						/* IBM sucks rocks */
#endif
	"orchid",					/* Violet Team */
	"grey71",					/* Neutral Team */
	"cyan",						/* Cursor, tink, commentator, radar */
	"White",					/* dashed line color */
};

int team_color[] =
{GREY, RED, ORANGE, YELLOW, GREEN, BLUE, VIOLET};
int team_color_bright[] =
{WHITE, RED, ORANGE, YELLOW, GREEN, BLUE, VIOLET};

static char *fontname[MAX_FONTS] =
{
#ifdef X11R2FONTS
	"5x8", "6x10", "courb14", "courb18", "courb24",
#else
#ifdef NEED_AUX_FONTS
	"-misc-fixed-medium-r-*-*-8-*", "6x10",
#else
/*
	"-misc-fixed-medium-r-*-*-8-*", "-misc-fixed-medium-r-*-*-10-*",
*/
	"-misc-fixed-medium-r-*-*-8-*", "6x10",
#endif /* def NEED_AUX_FONTS */
	"*-courier-bold-r-*-14-*", "*-courier-bold-r-*-18-*",
	"*-courier-bold-r-*-24-*"
#endif /* def X11R2FONTS */
};

static int fwidth[MAX_FONTS], fheight[MAX_FONTS], fascent[MAX_FONTS];

Video *vid;

/******************
** Error Handler **
******************/

char video_error_str[256];

#define video_error(identifier,arg) \
  sprintf(video_error_str,identifier,arg)

/*********************
** Window functions **
*********************/

/*
 * * Initializes everything dealing with graphics.
 */
open_graphics()
{
	int liteXerror();

	XSetErrorHandler(liteXerror);
}

/*
 * * Frees all allocated storage for graphics.
 */
close_graphics()
{
}

/*
 * * Reset the current video.
 */
reset_video()
{
	vid->last_expose_frame = 0;
}

/*
 * * Sets the current video.
 */
set_video(video)
Video *video;
{
	vid = video;
}

#define punt() \
  { close_video(video); \
    return (Video *) NULL; }

/*
 * * Makes a video structure, initializes it. * Returns a pointer to the
 * video or NULL if video could not be initialized.
 */
Video *make_video(name)
char *name;
{
	Video *video;
	int i;

	video = (Video *) malloc(sizeof(Video));

	video->dpy = XOpenDisplay(name);
	/***********************************/
	/* 40 times slower, but debugable: */
	/* XSynchronize(video->dpy, 1);    */
	/***********************************/
#ifdef DEBUG_SYNC
	XSynchronize(video->dpy, 1);
#endif
	(void) strcpy(video->display_name, name);
	if (video->dpy == NULL) {
		video_error("Could not open display named %s", name);
		free((char *) video);
		return (Video *) NULL;
	}
	set_video(video);

	/* Make the gcs, cursors, and parent window for the display */
	if (make_gcs())
		punt();
	if (make_cursors())
		punt();
	if (make_parent())
		punt();

	video->num_windows = 0;
	video->num_pixids = 0;
	video->escher_width = get_num_default("escherWidth", "Width", 10);

	/*
	 * Intended for debugging, probably no reason to advertise
	 * that these can be set by resource                  -ane
	 */

	DEST_WALL = get_num_default("color.dest_wall_index", "Color.Team",
								(vid->planes > 1) ? GREY : DASHED);

	if (DEST_WALL < BLACK || DEST_WALL >= MAX_COLORS)
		DEST_WALL = GREY;

	RDF_SAFE = get_num_default("color.green_rdf", "Color.Team",
							   (vid->planes > 1) ? GREEN : DASHED);

	if (RDF_SAFE < BLACK || DEST_WALL >= MAX_COLORS)
		RDF_SAFE = GREEN;


	/* Do these with the tri-nary because they are bitfields and */
	/* I don't want to deal with having some moron setting thier values */
	/* to something other than 0 or 1 */
	i = get_num_default("beep", "Beep", 1);
	video->beep_flag = (i == 0) ? 0 : 1;

	i = get_num_default("drawName", "Drawname", 1);
	video->display_names_flag = (i == 0) ? 0 : 1;

	video->last_expose_frame = 0;
	return video;
}

/*
 * * Makes a 1024x864 or 640x400 parent window for the application.
 */
make_parent()
{
	Window rw;
	XSizeHints size;
	Pixmap icon;
	XWMHints wmhints;
	XClassHint classHint;


	size.flags = PPosition | PSize;
	size.x = 0;
	size.y = 0;

#ifdef  S1024x864
	size.min_width = 1024;
	size.min_height = 864;
#else /* S1024x864 */
	/* S640x400 better be defined */
	size.min_width = 640;
	size.min_height = 400;
#endif /* S1024x864 */

	size.max_width = DisplayWidth(vid->dpy, DefaultScreen(vid->dpy));
	size.max_height = DisplayHeight(vid->dpy, DefaultScreen(vid->dpy));

	size.width = MIN(size.min_width, size.max_width);
	size.height = MIN(size.min_height, size.max_height);

	/* Let user know that it doesn't fit on screen, but don't quit */
	if ((size.width < size.min_width) || (size.height < size.min_height))
		fprintf(stderr, "Warning: Parent window does not fit on screen.");

	rw = RootWindow(vid->dpy, DefaultScreen(vid->dpy));
	vid->parent_id = XCreateSimpleWindow(vid->dpy, rw,
										 size.x, size.y, size.width,
										 size.height, 0, vid->fg, vid->bg);
	if (vid->parent_id == NULL) {
		video_error("Could not open parent window %s", "");
		return 1;
	}
	icon = XCreateBitmapFromData(vid->dpy, rw, icon_bits, icon_width,
								 icon_height);
	XSetStandardProperties(vid->dpy, vid->parent_id, program_name,
						   program_name, icon, NULL, 0, &size);

	classHint.res_name = "xtank";
	classHint.res_class = "XTank";
	XSetClassHint(vid->dpy, vid->parent_id, &classHint);

	wmhints.input = TRUE;
	wmhints.flags = InputHint | IconPixmapHint;
	wmhints.icon_pixmap = icon;

#if 0
	XSetWMProperties(vid->dpy, vid->parent_id, program_name, program_name,
					 NULL, 0, NULL, &wmhints, NULL);
#endif

	wmhints.input = TRUE;
	wmhints.flags = InputHint;
	XSetWMHints(vid->dpy, vid->parent_id, &wmhints);

	return 0;
}

/*
 * * Destroys the specified video.
 */
close_video(video)
Video *video;
{
	XCloseDisplay(video->dpy);
	free((char *) video);
}

/*
 * * Makes a window with the specified characteristics.
 */
make_window(w, x, y, width, height, border)
int w, x, y, width, height, border;
{
	Window id;

	id = XCreateSimpleWindow(vid->dpy, vid->parent_id, x, y, width, height,
							 border, vid->fg, vid->bg);
	if (id == (KeySym) NULL) {
		video_error("Could not open window #%d", w);
		return 1;
	}
	vid->input_mask = ButtonPressMask | KeyPressMask | ExposureMask;
	XSelectInput(vid->dpy, id, vid->input_mask);

	/* Make the internal window structure */
	vid->win[w].id = id;
	vid->win[w].width = width;
	vid->win[w].height = height;
	vid->win[w].flags = 0;

	/* Make sure the number of windows is high enough */
	vid->num_windows = MAX(vid->num_windows, w + 1);

	return 0;
}

/*
 * * Beep
 */
beep_window()
{
	if (vid->beep_flag) {
		XBell(vid->dpy, 0);
	}
}

/*
 * * Displays the window on the screen.
 */
map_window(w)
int w;
{
	/* Map the parent window just before mapping the first child */
	if (w == 0)
		XMapWindow(vid->dpy, vid->parent_id);
	XMapWindow(vid->dpy, vid->win[w].id);
	vid->win[w].flags |= WIN_mapped;
}

/*
 * * Removes the window from the screen.
 */
unmap_window(w)
int w;
{
	XUnmapWindow(vid->dpy, vid->win[w].id);
	vid->win[w].flags &= ~WIN_mapped;
}

/*
 * * Clears the window.
 */
clear_window(w)
int w;
{
	XClearWindow(vid->dpy, vid->win[w].id);
}

/***********************
** Graphics functions **
***********************/

/* draw text in an arbitrary position in a window */

draw_text_left(window, x, y, str, font, func, color)
int window;
int x, y;						/* upper left corner */
char *str;
int font, func, color;
{
	XDrawString(vid->dpy, vid->win[window].id, vid->text_gc[font][func][color],
				x, y + fascent[font],
				str, strlen(str));
}

/* Use this routine to draw centered text in an arbitrary position in a window.
   The x coordinate specifies the center of the string to be drawn.  The y
   coordinate is at the top of the string.  */

draw_text(w, x, y, str, font, func, color)
int w;
int x, y;
char *str;
int font, func, color;
{
	int len;

#ifndef NO_TEXT_CLIP

/*
 * Only draws text that (some of) would end up in the window
 */

	int bottom_y, right_x, left_x;

	len = strlen(str);
	bottom_y = y + fheight[font];
	left_x = x - fwidth[font] * len / 2;
	right_x = x + fwidth[font] * len / 2;
	if (left_x < vid->win[w].width
		&& right_x >= 0
		&& y < vid->win[w].height
		&& bottom_y >= 0)
		XDrawString(vid->dpy, vid->win[w].id, vid->text_gc[font][func][color],
					left_x, y + fascent[font], str, len);

#else /* NO_TEXT_CLIP */

	len = strlen(str);
	XDrawString(vid->dpy, vid->win[w].id, vid->text_gc[font][func][color],
				x - fwidth[font] * len / 2,
				y + fascent[font],
				str, len);

#endif /* NO_TEXT_CLIP */

}

int should_disp_name()
{
	int ret = vid->display_names_flag;

	return (ret);
}

/*
 * * Use this routine to write text in a text type window * The x and y
 * values in the word structure are interpreted as * a row and column in the
 * window.
 */
draw_text_rc(w, x, y, str, font, color)
int w, x, y;
char *str;
int font, color;
{
	XDrawImageString(vid->dpy, vid->win[w].id,
					 vid->text_gc[font][DRAW_COPY][color],
					 LEFT_BORDER + fwidth[font] * x,
					 TOP_BORDER + fheight[font] * y + fascent[font],
					 str, strlen(str));
}

/*
 * * Clears a rectangle of text rows and columns in a specified window.
 */
clear_text_rc(w, x, y, width, height, font)
int w, x, y, width, height, font;
{
	draw_filled_rect(w,
					 LEFT_BORDER + x * fwidth[font],
					 TOP_BORDER + y * fheight[font],
					 width * fwidth[font],
					 height * fheight[font],
					 DRAW_COPY, BLACK);
}

/******************
** I/O functions **
******************/

/*
 * * Performs all graphics operations that have been queued up.
 */
flush_output()
{
#ifdef USED_BATCHED_POINTS
	flush_point_batch;
#endif
#ifdef USED_BATCHED_LINES
	flush_line_batch;
#endif
	XFlush(vid->dpy);
}

/*
 * * Performs all graphics operations and waits until all input is received.
 */
sync_output(discard)
Boolean discard;
{
	XSync(vid->dpy, discard);
}

#ifdef MULTI_SYNC
/*
 * * Synchronizes all the given video displays. * Currently the number of
 * videos is limited to 10.
 */
multi_sync(video, num_videos, discard)
Video *video[];
int num_videos;
Boolean discard;

{
	Display *dpys[MAX_TERMINALS];
	int i;

	for (i = 0; i < num_videos; i++)
		dpys[i] = video[i]->dpy;

	XMultiSync(dpys, num_videos, discard);
}

#endif

#ifdef KEYPAD_DETECT
/***************************************************************************
 *
 * The following two functions are supplied to ``fix'' the way that the SUN
 * X Server deals with the numeric/application keypad. 
 *
 ***************************************************************************/

/****************************************************************************
 *
 * Sun Keypad KeySym Decoder, returns char of decoded Numeric Keypad 
 *
 ****************************************************************************/

char
  XDecodeSunKeypad(keysym)
KeySym keysym;
{
  /*
   * Only decodes Numeric Keys
   */
  char val;
  
  if (keysym >= XK_KP_0 && keysym <= XK_KP_9) 
    {
      val = (char) (keysym - XK_KP_0 + (int) '0');
    }
  else
    val = (char) 0;
  return (val);
}

/****************************************************************************
 *
 * Sun Keypad Handler -- translates SUN KeySyms to Numberpad KeySyms.
 *
 ****************************************************************************/

KeySym
  XMapSunKeypad(event)
XKeyEvent *event;
{
  typedef struct 
    {
      KeySym original;
      KeySym mapped;
    } translation;

  static translation SunTranslationTable[] = {
    XK_Insert, XK_KP_0,
    XK_R1,     XK_F1,
    XK_R2,     XK_F2,
    XK_R3,     XK_F3,
    XK_R4,     XK_KP_Equal,
    XK_R5,     XK_KP_Divide,
    XK_R6,     XK_KP_Multiply,
    XK_R7,     XK_KP_7,
    XK_Up,     XK_KP_8,
    XK_R9,     XK_KP_9,
    XK_Left,   XK_KP_4,
    XK_R11,    XK_KP_5,
    XK_Right,  XK_KP_6,
    XK_R13,    XK_KP_1,
    XK_Down,   XK_KP_2,
    XK_R15,    XK_KP_3,
    (KeySym) NULL, (KeySym) NULL
    };
  
  KeySym inkeysym;
  int loop;
  char buffer[2];
  
  inkeysym = XLookupKeysym(event, 0);
  
  for (loop = 0; SunTranslationTable[loop].original != NULL; loop++)
    if (SunTranslationTable[loop].original == inkeysym)
      {
	event->keycode = XKeysymToKeycode(vid->dpy,
					  SunTranslationTable[loop].mapped);
	
	inkeysym = SunTranslationTable[loop].mapped;
	
	break;
      }
#ifdef DEBUG  
  fprintf(stderr,
	  "XMapSunKeyPad: KeySym = %#04x (%s),\t Keypad is %s, \
Char = `%c' (%3d)\n",
	  inkeysym, XKeysymToString(inkeysym), 
	  IsKeypadKey(inkeysym) ? "True" : "False",
	  event->keycode,  event->keycode);

  if ((XLookupString(event, buffer, 1, NULL, NULL) == 0)
      && (buffer[0] = XDecodeSunKeypad(inkeysym)) == 0)
    fprintf(stderr, "failed looking up key\n");
  else
    {
      fprintf(stderr, "succeeded looking up key\n");
      
      fprintf(stderr, "Key (%c) is %sprintable\n",
	      buffer[0],
	      (!isprint(buffer[0]) && buffer[0] != '\r' && buffer[0] != 127)
	      ? "not " : "");
    }
#endif /* DEBUG */
  return (inkeysym);
}

#endif /* KEYPAD_DETECT */

/* Gets up to *num_events input events, handling the expose events by setting
   the WIN_expose flag.  Events are stored in the event array, with num_events
   holding the number of events placed there.  */

get_events(num_events, event)
int *num_events;
Event event[];
{
	XEvent xevent;
	XAnyEvent *any_xevent;
	XKeyEvent *key_xevent;
	XButtonEvent *button_xevent;
	XMotionEvent *motion_xevent;
	KeySym sunkeysym;
	Event *e;
	char buf[2];
	int max_events;
	int w, tf;
	int iExposeEvent = FALSE;
	extern int frame;
	extern Boolean game_running;

	max_events = *num_events;
	*num_events = 0;
	while ((*num_events < max_events) && (!game_running || XPending(vid->dpy))) {
		XNextEvent(vid->dpy, &xevent);
		any_xevent = &xevent.xany;
		for (w = 0; w < vid->num_windows; w++) {
			if (any_xevent->window == vid->win[w].id) {
				switch ((int) xevent.type) {
				  case Expose:
					  /* Set the exposed bit in the window flags if win is
		       		exposed */
					  vid->win[w].flags |= WIN_exposed;
					  vid->last_expose_frame = frame;
					  iExposeEvent = TRUE;
					  break;
				  case KeyPress:
					  /* Figure out which key was pressed (if any) */
					  key_xevent = &xevent.xkey;
				
#ifdef KEYPAD_DETECT
#ifdef sun
					  /*
					   * Check for SUN keypad keysyms
					   * and map them to Standard keysyms.
					   */
					  sunkeysym=
					    XMapSunKeypad(key_xevent); 
#else
					  sunkeysym =
					    XLookupKeysym(key_xevent, 0); 
#endif /* sun */
#endif /* KEYPAD_DETECT */
					  
					  if ((XLookupString(key_xevent,
							     buf, 1, 
							     NULL, NULL) 
					       == 0)
					      && ((buf[0] =
						   XDecodeSunKeypad(sunkeysym))
						  == 0))
					    break;
					  /* Changed 127 to DEL_CHAR (HAK) */
					  if (!isprint(buf[0]) 
					      && buf[0] !='\r' 
					      && buf[0] != '\t'
					      && buf[0] != 0x08
					      && buf[0] != 0x7f) 
					    break;
					  
					  /* Add a key event to the array */
					  e = &event[(*num_events)++];
					  e->win = w;
					  e->type = EVENT_KEY;
					  e->key = buf[0];
					  e->x = key_xevent->x;
					  e->y = key_xevent->y;
#ifdef KEYPAD_DETECT
					  /* assumes most people will have wants_keypad set */
					  e->keypad =
					    vid->kludge.wants_keypad
					      * IsKeypadKey(sunkeysym);
#endif
					  break;
				  case ButtonPress:
				  case ButtonRelease:
					  /* Add a button event to the array */
					  e = &event[(*num_events)++];
					  e->win = w;

					  button_xevent = &xevent.xbutton;
					  tf = (xevent.type == ButtonPress);
					  switch (button_xevent->button) {
						case Button1:
							e->type = (tf ? EVENT_LBUTTON : EVENT_LBUTTONUP);
							break;
						case Button2:
							e->type = (tf ? EVENT_MBUTTON : EVENT_MBUTTONUP);
							break;
						case Button3:
							e->type = (tf ? EVENT_RBUTTON : EVENT_RBUTTONUP);
							break;
					  }
					  e->x = button_xevent->x;
					  e->y = button_xevent->y;
					  break;
				  case MotionNotify:
 				  /* 
 				   * Only take the last motion event when
 				   * we get a sequence of motion events.
 				   *
 				   * Each player only gets to handle
 				   * `max_events' X Events so we don't want
 				   * to waste time reading Motion Events
 				   * that are only used to set direction
 				   * and/or relative drive (mouse_speed).
 				   */
				  while 
 				    (XCheckTypedWindowEvent(vid->dpy,
 							    vid->win[w].id,
 							    MotionNotify,
 							    &xevent));
					  /* Add a moved event to the array */
					  e = &event[(*num_events)++];
					  e->win = w;

					  motion_xevent = &xevent.xmotion;
					  e->type = EVENT_MOVED;
					  e->x = motion_xevent->x;
					  e->y = motion_xevent->y;
					  break;
				}
			}
		}

		if (iExposeEvent) {
			break;
		}
	}
}

/*
 * * Causes EVENT_MOVED to be enabled or disabled in get_events().
 */
follow_mouse(w, status)
int w;
Boolean status;
{
	if (status == TRUE)
		vid->input_mask |= PointerMotionMask;
	else
		vid->input_mask &= ~PointerMotionMask;

	XSelectInput(vid->dpy, vid->win[w].id, vid->input_mask);
}

/*
 * * Causes EVENT_*BUTTONUP to be enabled or disabled in get_events().
 */
button_up(w, status)
int w;
Boolean status;
{
	if (status == TRUE)
		vid->input_mask |= ButtonReleaseMask;
	else
		vid->input_mask &= ~ButtonReleaseMask;

	XSelectInput(vid->dpy, vid->win[w].id, vid->input_mask);
}

/****************************
** Cursors, fonts, pixmaps **
****************************/

/* Global because I am being lazy */
#define SENSIBLE    10
XrmDatabase xrmdb;
char *rmdbstring;
XrmName res_name[SENSIBLE];
XrmClass res_class[SENSIBLE];
XrmRepresentation res_type;
XrmValue xrmv;

/*
 * * Gets default resources, allocates colors, loads in fonts and makes the
 * gcs.
 */
make_gcs()
{
	Window rw;
	XGCValues values;
	XCharStruct *cs;
	Colormap cmap;
	XColor scol, ecol;
	int scr;
	unsigned long black, white;
	char *fc, *bc, *rv, *cptr;
	long mask;
	int i, j, k;
	char dummy;
	static Boolean firsttime = TRUE;

	dummy = '\0';

	/* Get default info about screen */
	scr = DefaultScreen(vid->dpy);
	cmap = DefaultColormap(vid->dpy, scr);
	vid->planes = DisplayPlanes(vid->dpy, scr);
	black = BlackPixel(vid->dpy, scr);
	white = WhitePixel(vid->dpy, scr);

	/* Only needs to be done *once* */
	XrmInitialize();
	rmdbstring = XResourceManagerString(vid->dpy);
	if (!rmdbstring)
		fprintf(stderr, "Someone has no x resources...\n");
	xrmdb = XrmGetStringDatabase(rmdbstring ? rmdbstring : "");
	res_name[0] = XrmStringToName("xtank");
	res_class[0] = XrmStringToClass("XTank");
	res_name[2] = res_class[2] = NULLQUARK;

	/* Get default info from resources */
	strncpy(vid->kludge.player_name,
			get_default("playerName", "PlayerName", &dummy),
			MAX_STRING);
	strncpy(vid->kludge.tank_name,
			get_default("tankName", "TankName", &dummy),
			MAX_STRING);
	vid->kludge.mouse_speed = get_num_default("mouseSpeed","MouseSpeed", 0);
	vid->kludge.mouse_drive = get_num_default("mouseDrive","MouseDrive", 0); 
	vid->kludge.mouse_heat = get_num_default("mouseHeat","MouseHeat", 0); 
	strncpy(vid->kludge.keymap,
			get_default("keyMap", "KeyMap", &dummy),
			MAX_STRING);
#ifdef KEYPAD_DETECT
	vid->kludge.wants_keypad = get_num_default("keypad", "Keypad", 1);
#endif
	fc = get_default("foreground", "Foreground", (char *) NULL);
	bc = get_default("background", "Background", (char *) NULL);
	rv = get_default("reverseVideo", "ReverseViedo", (char *) NULL);

	/* Get foreground and background pixel values */
	if (rv != NULL &&
		(!strcmp(rv, "true") || !strcmp(rv, "on") || !strcmp(rv, "yes"))) {
		vid->fg = black;
		vid->bg = white;
		curptr = color_titles[BLACK];
	} else {
		vid->fg = white;
		vid->bg = black;
		curptr = color_titles[WHITE];
	}

	if (vid->planes > 1) {
		if (fc != NULL) {
			if (XAllocNamedColor(vid->dpy, cmap, fc, &scol, &ecol)) {
				vid->fg = scol.pixel;
			}
		}
		if (bc != NULL) {
			if (XAllocNamedColor(vid->dpy, cmap, bc, &scol, &ecol)) {
				vid->bg = scol.pixel;
			}
		}
	}
	/* Get array of pixel values */
	for (i = 0; i < MAX_COLORS; i++)
		vid->color[i] = (i == BLACK) ? vid->bg : vid->fg;

	if (vid->planes > 1) {
		for (i = 2; i < MAX_COLORS; i++) {
			cptr = get_default(color_titles[i], color_classes[i],
							   colorname[i]);
			if (XParseColor(vid->dpy, cmap, cptr, &scol))
				if (XAllocColor(vid->dpy, cmap, &scol))
					vid->color[i] = scol.pixel;
				else
					fprintf(stderr, "Couldn't allocate color %s\n", cptr);
			else
				fprintf(stderr, "Couldn't parse color %s\n", cptr);
		}
	}
	/* Load in all the fonts */

#ifdef NEED_AUX_FONT
	{
		char fontpath[MAXPATHLEN];

		sprintf(fontpath, "%s/%s/", pathname, fontdir);
		set_font_path(fontpath);
	}
#endif

	for (i = 0; i < MAX_FONTS; i++) {
		vid->fs[i] = XLoadQueryFont(vid->dpy, fontname[i]);
		if (!vid->fs[i])
			vid->fs[i] = XLoadQueryFont(vid->dpy, "fixed");
		if (!vid->fs[i]) {
			video_error("Cannot open font named %s or fixed", fontname[i]);
			return 1;
		}
	}

	/* If first time, cache the widths, heights and ascents of the fonts */
	if (firsttime == TRUE) {
		firsttime = FALSE;
		for (i = 0; i < MAX_FONTS; i++) {
			cs = &vid->fs[i]->max_bounds;
			fwidth[i] = cs->width;
			fheight[i] = cs->ascent + cs->descent - (i > 0);
			fascent[i] = cs->ascent;
		}
	}
	rw = RootWindow(vid->dpy, scr);

	/* * Turn off graphics exposures, to avoid getting an event for each *
       XCopyArea with a pixmap.  X is totally brain damaged. */
	values.graphics_exposures = False;
	for (i = 0; i < MAX_COLORS; i++) {
		for (j = 0; j < MAX_DRAW_FUNCS; j++) {
			/*
	     * Note that the following ONLY apply if
	     * the mask is set, ie, these are normally
	     * NOT copied into the GC, they are used only
	     * if the color is DASHED        -ane
	     */

			values.dashes = 1;
			values.line_style = LineOnOffDash;

			switch (j) {
			  case DRAW_XOR:
				  /* Works on PsudoColor, what about DirrectColor? */
				  values.background = 0;
				  values.function = GXxor;
				  values.foreground = vid->color[i] ^ vid->bg;
				  break;
			  case DRAW_COPY:
				  values.function = GXcopy;
				  values.background = vid->bg;	/* Moved JMO */
				  values.foreground = vid->color[i];
				  /* printf("fg %d, bg %d\n", values.background,
		   values.foreground); */
				  break;
			}

			/*
	     * Switching GC's is one of the more expensive
	     * operations, both on the server and the client.
	     *
	     * If we are on a mono tube, create only necessary GC's and
	     * make everything else point to them.    -ane
	     */

			if (vid->planes > 1) {

				/* Make a gc for each font in this color,func */
				mask = GCForeground | GCBackground | GCFont | GCFunction |
				  GCGraphicsExposures;
				for (k = 0; k < MAX_FONTS; k++) {
					values.font = vid->fs[k]->fid;
					vid->text_gc[k][j][i] = XCreateGC(vid->dpy, rw, mask, &values);

				}

				/* Make a gc for drawing in this color,func */
				mask = GCForeground | GCBackground | GCFunction |
				  GCGraphicsExposures;

				/*
		 * if this color is DASHED, set the line style  -ane
		 */
				if (i == DASHED)
					mask |= GCDashList | GCLineStyle;

				vid->graph_gc[j][i] = XCreateGC(vid->dpy, rw, mask, &values);

			} else {

/*
 * Warning, code depends on WHITE being numerically less than
 * all other colors                                     -ane
 */

				if (i == WHITE || i == BLACK || i == DASHED) {

					/* Make a gc for each font in this color,func */
					mask = GCForeground | GCBackground | GCFont | GCFunction |
					  GCGraphicsExposures;
					for (k = 0; k < MAX_FONTS; k++) {
						values.font = vid->fs[k]->fid;
						vid->text_gc[k][j][i] = XCreateGC(vid->dpy, rw, mask, &values);
					}

					/* Make a gc for drawing in this color,func */
					mask = GCForeground | GCBackground | GCFunction |
					  GCGraphicsExposures;

					/*
		 * if this color is DASHED, set the line style  -ane
		 */

					if (i == DASHED)
						mask |= GCDashList | GCLineStyle;

					vid->graph_gc[j][i] = XCreateGC(vid->dpy, rw, mask, &values);

				} else {

					for (k = 0; k < MAX_FONTS; k++)
						vid->text_gc[k][j][i] = vid->text_gc[k][j][WHITE];

					vid->graph_gc[j][i] = vid->graph_gc[j][WHITE];

				}
			}
		}
	}
	return 0;
}

#ifdef NEED_AUX_FONT
/* This sets up the font path to contain the directories that have the
 * fonts this program needs.
 */
set_font_path(fontdir)
char *fontdir;
{
	int i, font_length;
	char **font_path = XGetFontPath(vid->dpy, &font_length);

	for (i = 0; (i < font_length) && strcmp(font_path[i], fontdir); i++) ;

	if (i >= font_length) {
		char **new_font_path;

		if (new_font_path = (char **) malloc((font_length + 1) * sizeof(char *))) {
			for (i = 0; i < font_length; i++)
				new_font_path[i] = font_path[i];
			new_font_path[i] = fontdir;
			XSetFontPath(vid->dpy, new_font_path, font_length + 1);
			free(new_font_path);
		}
	}
	if (font_path)
		XFreeFontPath(font_path);
}

#endif

#define cross_width 16
#define cross_height 16
#define cross_x_hot 7
#define cross_y_hot 7
static unsigned char cross_bits[] =
{
	0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0xc0, 0x01, 0x80, 0x00,
	0x10, 0x04, 0x3f, 0x7e, 0x10, 0x04, 0x80, 0x00, 0xc0, 0x01, 0x80, 0x00,
	0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x00, 0x00};
static unsigned char cross_mask[] =
{
	0xc0, 0x01, 0xc0, 0x01, 0xc0, 0x01, 0xc0, 0x01, 0xe0, 0x03, 0xd0, 0x05,
	0xbf, 0x7e, 0x7f, 0x7f, 0xbf, 0x7e, 0xd0, 0x05, 0xe0, 0x03, 0xc0, 0x01,
	0xc0, 0x01, 0xc0, 0x01, 0xc0, 0x01, 0x00, 0x00};

#define plus_width 16
#define plus_height 16
#define plus_x_hot 8
#define plus_y_hot 7
static unsigned char plus_bits[] =
{
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x03, 0x80, 0x03, 0x80, 0x03,
	0xf0, 0x1f, 0xf0, 0x1f, 0xf0, 0x1f, 0x80, 0x03, 0x80, 0x03, 0x80, 0x03,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static unsigned char plus_mask[] =
{
	0x00, 0x00, 0x00, 0x00, 0xc0, 0x07, 0xc0, 0x07, 0xc0, 0x07, 0xf8, 0x3f,
	0xf8, 0x3f, 0xf8, 0x3f, 0xf8, 0x3f, 0xf8, 0x3f, 0xc0, 0x07, 0xc0, 0x07,
	0xc0, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

#define ul_width 16
#define ul_height 16
#define ul_x_hot 8
#define ul_y_hot 7
static unsigned char ul_bits[] =
{
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x80, 0x3f, 0x80, 0x3f, 0x80, 0x3f, 0x80, 0x03, 0x80, 0x03, 0x80, 0x03,
	0x80, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static unsigned char ul_mask[] =
{
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x7f,
	0xc0, 0x7f, 0xc0, 0x7f, 0xc0, 0x7f, 0xc0, 0x7f, 0xc0, 0x07, 0xc0, 0x07,
	0xc0, 0x07, 0xc0, 0x07, 0x00, 0x00, 0x00, 0x00};

#define lr_width 16
#define lr_height 16
#define lr_x_hot 8
#define lr_y_hot 7
static unsigned char lr_bits[] =
{
	0x00, 0x00, 0x00, 0x00, 0x80, 0x03, 0x80, 0x03, 0x80, 0x03, 0x80, 0x03,
	0xf8, 0x03, 0xf8, 0x03, 0xf8, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static unsigned char lr_mask[] =
{
	0x00, 0x00, 0xc0, 0x07, 0xc0, 0x07, 0xc0, 0x07, 0xc0, 0x07, 0xfc, 0x07,
	0xfc, 0x07, 0xfc, 0x07, 0xfc, 0x07, 0xfc, 0x07, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

/*
 * * Makes all cursors.
 */
make_cursors()
{
	int ret = 0;

	/* Create all the cursors */
	ret += make_cursor(CROSS_CURSOR, cross_width, cross_height, cross_x_hot,
					   cross_y_hot, cross_bits, cross_mask);
	ret += make_cursor(PLUS_CURSOR, plus_width, plus_height, plus_x_hot,
					   plus_y_hot, plus_bits, plus_mask);
	ret += make_cursor(UL_CURSOR, ul_width, ul_height, ul_x_hot, ul_y_hot,
					   ul_bits, ul_mask);
	ret += make_cursor(LR_CURSOR, lr_width, lr_height, lr_x_hot, lr_y_hot,
					   lr_bits, lr_mask);
	return ret;
}

/*
 * * Makes a cursor.
 */
make_cursor(c, width, height, xhot, yhot, bits, mask)
int c, width, height, xhot, yhot;
char bits[], mask[];
{
	XColor fcol, bcol, junk_col;
	Colormap cmap;
	Pixmap bpix, mpix;
	char *cptr;

	/* Make the pixmap for the cursor and make the cursor */
	bpix = XCreateBitmapFromData(vid->dpy,
							  RootWindow(vid->dpy, DefaultScreen(vid->dpy)),
								 bits, width, height);
	mpix = XCreateBitmapFromData(vid->dpy,
							  RootWindow(vid->dpy, DefaultScreen(vid->dpy)),
								 mask, width, height);
	if (bpix == NULL || mpix == NULL) {
		video_error("Could not create cursor pixmap #%d", c);
		return 1;
	}
	/* Get foreground and background colors */
	cmap = DefaultColormap(vid->dpy, DefaultScreen(vid->dpy));
	if (vid->planes > 1) {
		cptr = color_titles[CUR_COLOR];
	} else {
		cptr = curptr;
	}
	cptr = get_default(cptr, color_classes[CUR_COLOR], colorname[CUR_COLOR]);
	if (XAllocNamedColor(vid->dpy, cmap, cptr, &fcol, &junk_col)) {
		vid->color[CUR_COLOR] = fcol.pixel;
	} else {
		fprintf(stderr, "Cursor color failed\n");
	}
	/* fcol.pixel = vid->fg; */
	XQueryColor(vid->dpy, cmap, &fcol);
	bcol.pixel = vid->bg;
	XQueryColor(vid->dpy, cmap, &bcol);

	vid->cursor[c] = XCreatePixmapCursor(vid->dpy, bpix, mpix, &fcol, &bcol,
										 xhot, yhot);
	XFreePixmap(vid->dpy, bpix);
	XFreePixmap(vid->dpy, mpix);
	return 0;
}

/*
 * * Sets the current cursor.
 */
set_cursor(c)
int c;
{
	int i;

	if (c < 0 || c >= MAX_CURSORS)
		return;

	/* Define the cursor for all the windows */
	for (i = 0; i < vid->num_windows; i++)
		XDefineCursor(vid->dpy, vid->win[i].id, vid->cursor[c]);
}

/*
 * * Returns height of the specified font.
 */
font_height(font)
int font;
{
	return (fheight[font]);
}

/*
 * * Returns width of the string in the specified font.
 */
font_string_width(str, font)
char *str;
int font;
{
	return (fwidth[font] * strlen(str));
}

static int printed_openwindows_warning = 1;  /* We've seen it plenty (HAK) */

Video *oldvid = NULL;   /* It's a global now.  Temporary hack (HAK 2/93) */
                        /* We need remove_player to be able to set this */
                        /* to null, if it's display is going to be */
                        /* closed.  Otherwise, make_picture() will crash */
                        /* on the invalid oldvid when a new term is */
                        /* added.  */

/*
 * Makes the picture from the bitmap and the size given in the picture.
 */
make_picture(pic, bitmap)
Picture *pic;
char *bitmap;
{
	Pixmap temp, pix;
	unsigned int w, h;
	static GC tempgc;

	/* static int zerogc = -1; */
	/* static Video *oldvid = NULL; */ /* changed to global (HAK 2/93) */

	temp = XCreateBitmapFromData(vid->dpy,
							  RootWindow(vid->dpy, DefaultScreen(vid->dpy)),
								 bitmap, pic->width, pic->height);
	if (temp == NULL) {
		video_error("Could not store pixmap #%d", vid->num_pixids);
		return 1;
	}
	if (oldvid != vid) {
		if (oldvid != NULL)
			XFreeGC(oldvid->dpy, tempgc);
		oldvid = vid;

		tempgc = XCreateGC(vid->dpy, temp, 0, NULL);
		XSetForeground(vid->dpy, tempgc, 0L);

#if 0
		if (0 == BlackPixel(vid->dpy, DefaultScreen(vid->dpy)))
			zerogc = BLACK;
		if (0 == WhitePixel(vid->dpy, DefaultScreen(vid->dpy)))
			zerogc = WHITE;
		if (-1 == zerogc) {
			video_error("Black and white != %d", 1);
			return (1);
		}
#endif
	}
	/* 319 */
	if (!XQueryBestSize(vid->dpy, TileShape,
						RootWindow(vid->dpy, DefaultScreen(vid->dpy)),
						pic->width, pic->height, &w, &h)) {
		video_error("Could not find size for pixmap#%d", vid->num_pixids);
	}
	if (w < pic->width || h < pic->height) {
		if (printed_openwindows_warning == 0) {
			printed_openwindows_warning = 1;

			fprintf(stderr, "The XServer gave bogus size for XQueryBestSize!\n");
			fprintf(stderr,
					"It gave new size (%d, %d) smaller than the original (%d,%d).\n\n",
					w, h, pic->width, pic->height);
			fprintf(stderr, "We are thus going to ignore the bogofied results!\n");
			fprintf(stderr, "This is a known OpenWindows bug (what do you get when\n");
			fprintf(stderr, "you open windows?  BUGS!)\n");
			fprintf(stderr, "Send your XServer vendor hate mail!\n");
		}
		w = pic->width;
		h = pic->height;
	}
	pix = XCreatePixmap(vid->dpy,
						RootWindow(vid->dpy, DefaultScreen(vid->dpy)),
						w, h, 1);
	if (!pix) {
		video_error("Could not create 2nd pixmap #%d", vid->num_pixids);
		return 1;
	}
	XFillRectangle(vid->dpy, pix, tempgc, 0, 0, w, h);
	XCopyArea(vid->dpy, temp, pix, tempgc, 0, 0,
			  pic->width, pic->height, 0, 0);

	/* XFreeGC(vid->dpy, tempgc); */
	XFreePixmap(vid->dpy, temp);
	/*
      if (w != pic->width || h != pic->height) printf("PM#%d %dx%d -> %dx%d\n",
      vid->num_pixids+1, pic->width, pic->height, w, h);
      */

	if (vid->num_pixids == MAX_PIXMAPS - 1) {
		video_error("MAX_PIXMAPS too small (MoL%d)", 42);
		return 1;
	}
	pic->pixmap = vid->num_pixids++;
	vid->width[pic->pixmap] = w;
	vid->height[pic->pixmap] = h;
	vid->pixid[pic->pixmap] = pix;
	return 0;
}

/*
 * * Frees the picture.
 */
free_picture(pic)
Picture *pic;
{
	XFreePixmap(vid->dpy, vid->pixid[pic->pixmap]);
}

/*
 * * Puts a copy of pic rotated 90 degrees into rot_pic.
 */
Byte *rotate_pic_90(pic, rot_pic, bitmap)
Picture *pic, *rot_pic;
Byte *bitmap;
{
	Byte *rot_bitmap;
	int by_line, rot_by_line, num_bytes, rot_num_bytes;

/* #define WEIRD_SWITCH_DO */

#ifndef WEIRD_SWITCH_DO
	int itemp1, itemp2;

#endif
	register Byte *s_ptr, *d_ptr, *s_ptr_begin, *d_ptr_begin;
	register Byte d_mask;
	register int s_bit_begin, d_bit;

	rot_pic->width = pic->height;
	rot_pic->height = pic->width;
	rot_pic->offset_x = pic->height - pic->offset_y - 1;
	rot_pic->offset_y = pic->offset_x;

	by_line = (pic->width + 7) >> 3;
	num_bytes = by_line * pic->height;
	rot_by_line = (rot_pic->width + 7) >> 3;
	rot_num_bytes = rot_by_line * rot_pic->height;
	rot_bitmap = (Byte *) calloc((unsigned) (rot_num_bytes + 4), sizeof(Byte));

	/* Scan across each source scanline in the bitmap starting from the bottom
       right corner and working left and up. */
	s_ptr_begin = bitmap + num_bytes - 1;
	s_bit_begin = (pic->width - 1) & 0x7;
	d_ptr_begin = rot_bitmap + rot_num_bytes - rot_by_line;
	d_bit = 0;
	s_ptr = s_ptr_begin;
	do {
		d_ptr = d_ptr_begin;
		d_mask = 1 << d_bit;
		s_ptr_begin -= by_line;

#ifndef WEIRD_SWITCH_DO
		itemp1 = -99;
		itemp2 = s_bit_begin;
		do {
			if (itemp1 != -99)
				d_ptr -= rot_by_line;	/* happens after case 0 */

			for (itemp1 = itemp2; itemp1 >= 0; itemp1--) {
				if (*s_ptr & (1 << itemp1)) {
					*d_ptr |= d_mask;
				}
				if (itemp1) {
					d_ptr -= rot_by_line;	/* happens after case 0 */
				} else {
					itemp2 = 7;
				}
			}
		}
		while (--s_ptr > s_ptr_begin);
#else
		switch (s_bit_begin) {
			  do {
				  d_ptr -= rot_by_line;	/* happens after case 0 */
		  case 7:
				  if (*s_ptr & 1 << 7)
					  *d_ptr |= d_mask;
				  d_ptr -= rot_by_line;
		  case 6:
				  if (*s_ptr & 1 << 6)
					  *d_ptr |= d_mask;
				  d_ptr -= rot_by_line;
		  case 5:
				  if (*s_ptr & 1 << 5)
					  *d_ptr |= d_mask;
				  d_ptr -= rot_by_line;
		  case 4:
				  if (*s_ptr & 1 << 4)
					  *d_ptr |= d_mask;
				  d_ptr -= rot_by_line;
		  case 3:
				  if (*s_ptr & 1 << 3)
					  *d_ptr |= d_mask;
				  d_ptr -= rot_by_line;
		  case 2:
				  if (*s_ptr & 1 << 2)
					  *d_ptr |= d_mask;
				  d_ptr -= rot_by_line;
		  case 1:
				  if (*s_ptr & 1 << 1)
					  *d_ptr |= d_mask;
				  d_ptr -= rot_by_line;
		  case 0:
				  if (*s_ptr & 1 << 0)
					  *d_ptr |= d_mask;
			  } while (--s_ptr > s_ptr_begin);
		}
#endif

		if (++d_bit & 1 << 3) {
			d_bit = 0;
			d_ptr_begin++;
		}
	} while (s_ptr >= bitmap);

	return rot_bitmap;
}

/* Lookup table to reverse the bits in a byte */
unsigned char reverse_byte[256] =
{
	0x00, 0x80, 0x40, 0xc0, 0x20, 0xa0, 0x60, 0xe0, 0x10, 0x90, 0x50, 0xd0, 0x30, 0xb0, 0x70,
	0xf0, 0x08, 0x88, 0x48, 0xc8, 0x28, 0xa8, 0x68, 0xe8, 0x18, 0x98, 0x58, 0xd8, 0x38, 0xb8,
	0x78, 0xf8, 0x04, 0x84, 0x44, 0xc4, 0x24, 0xa4, 0x64, 0xe4, 0x14, 0x94, 0x54, 0xd4, 0x34,
	0xb4, 0x74, 0xf4, 0x0c, 0x8c, 0x4c, 0xcc, 0x2c, 0xac, 0x6c, 0xec, 0x1c, 0x9c, 0x5c, 0xdc,
	0x3c, 0xbc, 0x7c, 0xfc, 0x02, 0x82, 0x42, 0xc2, 0x22, 0xa2, 0x62, 0xe2, 0x12, 0x92, 0x52,
	0xd2, 0x32, 0xb2, 0x72, 0xf2, 0x0a, 0x8a, 0x4a, 0xca, 0x2a, 0xaa, 0x6a, 0xea, 0x1a, 0x9a,
	0x5a, 0xda, 0x3a, 0xba, 0x7a, 0xfa, 0x06, 0x86, 0x46, 0xc6, 0x26, 0xa6, 0x66, 0xe6, 0x16,
	0x96, 0x56, 0xd6, 0x36, 0xb6, 0x76, 0xf6, 0x0e, 0x8e, 0x4e, 0xce, 0x2e, 0xae, 0x6e, 0xee,
	0x1e, 0x9e, 0x5e, 0xde, 0x3e, 0xbe, 0x7e, 0xfe, 0x01, 0x81, 0x41, 0xc1, 0x21, 0xa1, 0x61,
	0xe1, 0x11, 0x91, 0x51, 0xd1, 0x31, 0xb1, 0x71, 0xf1, 0x09, 0x89, 0x49, 0xc9, 0x29, 0xa9,
	0x69, 0xe9, 0x19, 0x99, 0x59, 0xd9, 0x39, 0xb9, 0x79, 0xf9, 0x05, 0x85, 0x45, 0xc5, 0x25,
	0xa5, 0x65, 0xe5, 0x15, 0x95, 0x55, 0xd5, 0x35, 0xb5, 0x75, 0xf5, 0x0d, 0x8d, 0x4d, 0xcd,
	0x2d, 0xad, 0x6d, 0xed, 0x1d, 0x9d, 0x5d, 0xdd, 0x3d, 0xbd, 0x7d, 0xfd, 0x03, 0x83, 0x43,
	0xc3, 0x23, 0xa3, 0x63, 0xe3, 0x13, 0x93, 0x53, 0xd3, 0x33, 0xb3, 0x73, 0xf3, 0x0b, 0x8b,
	0x4b, 0xcb, 0x2b, 0xab, 0x6b, 0xeb, 0x1b, 0x9b, 0x5b, 0xdb, 0x3b, 0xbb, 0x7b, 0xfb, 0x07,
	0x87, 0x47, 0xc7, 0x27, 0xa7, 0x67, 0xe7, 0x17, 0x97, 0x57, 0xd7, 0x37, 0xb7, 0x77, 0xf7,
	0x0f, 0x8f, 0x4f, 0xcf, 0x2f, 0xaf, 0x6f, 0xef, 0x1f, 0x9f, 0x5f, 0xdf, 0x3f, 0xbf, 0x7f,
	0xff
};

/*
 * * Puts a copy of pic rotated 180 degrees into rot_pic.
 */
Byte *rotate_pic_180(pic, rot_pic, bitmap)
Picture *pic, *rot_pic;
Byte *bitmap;
{
	Byte *rot_bitmap;
	int num_bytes;
	register Byte *d_ptr;
	register Byte rev, dest;
	register int rshift, lshift;

	rot_pic->width = pic->width;
	rot_pic->height = pic->height;
	rot_pic->offset_x = pic->width - pic->offset_x - 1;
	rot_pic->offset_y = pic->height - pic->offset_y - 1;

	num_bytes = ((pic->width + 7) >> 3) * pic->height;
	rot_bitmap = (Byte *) malloc((unsigned) (num_bytes * sizeof(Byte)));

	/* * Scan across each source scanline in the bitmap starting * from the
	   bottom right corner and working left and up. * Reverse the source
	   bytes, shift, and OR into the destination. */
	lshift = pic->width & 0x7;
	rshift = 8 - lshift;
	d_ptr = rot_bitmap;

	/* Special case aligned rotation */
	if (lshift == 0) {
		while (--num_bytes >= 0)
			*d_ptr++ = reverse_byte[bitmap[num_bytes]];
	} else {
		dest = reverse_byte[bitmap[--num_bytes]] >> rshift;
		while (--num_bytes >= 0) {
			rev = reverse_byte[bitmap[num_bytes]];
			dest |= rev << lshift;
			*d_ptr++ = dest;
			dest = rev >> rshift;
		}
		*d_ptr = dest;
	}

	return rot_bitmap;
}

int liteXerror(dpy, err)
Display *dpy;
XErrorEvent *err;
{
	char buf[1024];

	printf("Error on %s\n", XDisplayName(buf));
	XGetErrorText(dpy, err->error_code, buf, 1024);
	printf("Error: %s\n", buf);
	XGetErrorDatabaseText(dpy, "xtank", "XRequest", "???", buf, 1024);
	printf("Serial %d/%d, Opcode %d/%d (%s), id $%lx\n", err->serial, 0,
		   err->request_code, err->minor_code, buf, err->resourceid);

	/* Dump core... */
	abort();
}

/*******************************/
/* Get User Defaults           */
/*******************************/
char *get_default(itemname, itemclass, defaultstr)
char *itemname, *itemclass, *defaultstr;
{
	char *ptr;

	/* Assume res_{name,class}[0] are set-up... */

	XrmStringToNameList(itemname, res_name + 1);
	XrmStringToClassList(itemclass, res_class + 1);

	/* ptr = XGetDefault(vid->dpy, program_name, itemname); */

	if (XrmQGetResource(xrmdb, res_name, res_class, &res_type, &xrmv)) {
		ptr = xrmv.addr;
	} else {
		ptr = defaultstr;
	}

	return (ptr);
}

int get_num_default(itemname, itemclass, defaultnum)
char *itemname, *itemclass;
int defaultnum;
{
	char *ptr;
	int retval;

	/* ptr = XGetDefault(vid->dpy, program_name, itemname); */
	ptr = get_default(itemname, itemclass, (char *) NULL);
	if (!ptr) {
		retval = defaultnum;
	} else {
		retval = atoi(ptr);
	}

	return (retval);
}
