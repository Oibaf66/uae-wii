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
#include "uae.h"
#include "menu.h"

extern SDL_Surface *screen;

static const char *main_menu_messages[] = {
		/*00*/		"Floppy",
		/*01*/		"^|df0|df1|df2|df3",
		/*02*/		"States",
		/*03*/		"^|Load|Save|Delete",
		/*04*/		"Keyboard",
		/*05*/		"^|Type|Macro|Bind",
		/*06*/		"#1-------------------------------------",
		/*07*/		"Reset UAE",
		/*08*/		"Networking",
		/*09*/		"Options",
		/*10*/		"Advanced Options",
		/*11*/		"Help",
		/*12*/		"Quit",
		NULL
};


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

static void insert_floppy(int which)
{
	const char *name = menu_select_file(prefs_get_attr("floppy_path"));

	if (name != NULL)
		strcpy (changed_prefs.df[which], name);
}

void gui_display(int shortcut)
{
	static int is_inited = 0;
	int submenus[3];
	int opt;

	memset(submenus, 0, sizeof(submenus));
	printf("Initing gui with %d\n", shortcut);
	printf("Al-mibb: Gui is display!\n");

	if (!is_inited)
	{
		menu_init(screen);
		is_inited = 1;
	}
	opt = menu_select_title("Main menu", main_menu_messages, submenus);
	switch(opt)
	{
	case 0:
		/* Insert floppy */
		insert_floppy(submenus[0]);
		break;
	case 7:
		uae_reset(1);
		break;
	case 12:
		uae_quit();
		break;
	default:
		break;
	}
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
