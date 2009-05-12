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

static const char *main_menu_messages[] = {
		/*00*/		"Floppy",
		/*01*/		"^|df0|df1|df2|df3",
		/*02*/		"States",
		/*03*/		"^|Load|Save|Delete",
		/*04*/		"Keyboard",
		/*05*/		"^|Type|Macro|Bind",
		/*06*/		"#1-------------------------------------",
		/*07*/		"Reset UAE",
		/*08*/		"Options",
		/*09*/		"Advanced Options",
		/*10*/		"Help",
		/*11*/		"Quit",
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
	int submenus[3];
	int opt;

	memset(submenus, 0, sizeof(submenus));
	printf("gui_display: %d\n", shortcut);

	opt = menu_select_title("Main menu", main_menu_messages, submenus);
	switch(opt)
	{
	case 0:
		/* Insert floppy */
		insert_floppy(submenus[0]);
		break;
	case 2:
		/* States */
		if (0)
		{
			const char *dir = prefs_get_attr("savestate_path");
			switch(submenus[1])
			{
			case 0: /* Load state */
				break;
			case 1: /* Save state */
				save_state(prefs_get_attr("floppy0"), "Descr");
				break;
			case 2: /* Delete state */
				break;
			default:
					break;
			}
		}
		msgYesNo("This is not implemented", 0, 320, 200);
		break;
	case 7:
		uae_reset(1);
		break;
	case 11:
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

       printf("Al-mibb: Gui message!\n");

       va_start (parms,format);
       vsprintf ( msg, format, parms);
       va_end (parms);
       msgYesNo(msg, 0, 24, 24);

       write_log (msg);
}
