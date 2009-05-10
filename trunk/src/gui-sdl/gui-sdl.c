 /*
  * UAE - The Un*x Amiga Emulator
  *
  * Interface to the Tcl/Tk GUI
  *
  * Copyright 1996 Bernd Schmidt
  */

#include "sysconfig.h"
#include "sysdeps.h"

#include "options.h"
#include "gui.h"
#include "menu.h"

extern SDL_Surface *screen;


void gui_init (int argc, char **argv)
{
}

int gui_open (void)
{
    return -1;
}

void gui_notify_state (int state)
{
}

int gui_update (void)
{
	printf("Al-mibb: Gui is updejted!\n");
    return 0;
}

void gui_exit (void)
{
}

void gui_fps (int fps, int idle)
{
    gui_data.fps  = fps;
    gui_data.idle = idle;
}

void gui_led (int led, int on)
{
}

void gui_hd_led (int led)
{
    static int resetcounter;

    int old = gui_data.hd;

    if (led == 0) {
	resetcounter--;
	if (resetcounter > 0)
	    return;
    }

    gui_data.hd = led;
    resetcounter = 6;
    if (old != gui_data.hd)
	gui_led (5, gui_data.hd);
}

void gui_cd_led (int led)
{
    static int resetcounter;

    int old = gui_data.cd;
    if (led == 0) {
	resetcounter--;
	if (resetcounter > 0)
	    return;
    }

    gui_data.cd = led;
    resetcounter = 6;
    if (old != gui_data.cd)
	gui_led (6, gui_data.cd);
}

void gui_filename (int num, const char *name)
{
}

void gui_handle_events (void)
{
}

static const char *main_menu_messages[] = {
		/*02*/		"Floppy",
		/*03*/		"^|df0|df1|df2|df3",
		/*04*/		"States",
		/*05*/		"^|Load|Save|Delete",
		/*06*/		"Keyboard",
		/*07*/		"^|Type|Macro|Bind",
		/*08*/		"#1-------------------------------------",
		/*09*/		"Reset the C=64",
		/*10*/		"Networking",
		/*11*/		"Options",
		/*12*/		"Advanced Options",
		/*13*/		"Help",
		/*15*/		"Quit",
		NULL
};


void gui_display(int shortcut)
{
	static int is_inited = 0;
	int submenus[3];
	int opt;

	printf("Initing gui with %d\n", shortcut);
	printf("Al-mibb: Gui is display!\n");

	if (!is_inited)
	{
		menu_init(screen);
		is_inited = 1;
	}
	opt = menu_select_title("Main menu", main_menu_messages, submenus);
	if (opt == 12)
		uae_quit();
}

void gui_message (const char *format,...)
{
       char msg[2048];
       va_list parms;

       printf("Al-mibb: Gui is al-message!\n");

       va_start (parms,format);
       vsprintf ( msg, format, parms);
       va_end (parms);

       write_log (msg);
}
