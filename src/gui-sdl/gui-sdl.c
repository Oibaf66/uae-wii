 /*
  * UAE - The Un*x Amiga Emulator
  *
  * Interface to the Tcl/Tk GUI
  *
  * Copyright 1996 Bernd Schmidt
  */

#include <sys/types.h>
#include <sys/stat.h>

#include "sysconfig.h"
#include "sysdeps.h"
#include "savestate.h"
#include "custom.h"

#include "options.h"
#include "gui.h"
#include "uae.h"
#include "menu.h"
#include "VirtualKeyboard.h"

static void default_config(void);

static const char *main_menu_messages[] = {
		/*00*/		"Insert floppy",
		/*01*/		"^|df0|df1|df2|df3",
		/*02*/		"States",
		/*03*/		"^|Load|Save|Delete",
		/*04*/		"#1-------------------------------------",
		/*05*/		"Reset UAE",
		/*06*/		"Amiga options",
		/*07*/		"Keyboard bindings",
		/*08*/		"Options",
		/*09*/		"Help",
		/*10*/		"Quit",
		/*04*/		"#1-------------------------------------",
		/*04*/		"#21 - back,  2/A - select",
		NULL
};

static const  char *keyboard_messages[] = {
		/*00*/		"Wiimote",
		/*01*/		"^|1|-|+",
		/*02*/		"Nunchuk",
		/*03*/		"^|C",
		/*04*/		"Classic",
		/*05*/		"^|b|x|y|L|R|Zl|Zr|-|+",
		NULL,
};

static const char *amiga_model_messages[] = {
		/*00*/		"Amiga model",
		/*01*/		"^|A1000|A500|A600|A1200|Custom",
		/*02*/		"Emulation accuracy",
		/*03*/		"^|Fast|Compatible|Cycle-exact",
		/*04*/		"Memory options",
		/*05*/		"CPU/Chipset options",
		/*06*/		"Change ROM",
		NULL
};

static const char *memory_messages[] = {
		/*00*/		"Chip mem",
		/*01*/		"^|512K|1M|2M",
		/*02*/		"Slow mem",
		/*03*/		"^|None|256K|512K|1M|1.8M",
		/*04*/		"Fast mem",
		/*05*/		"^|None|1M|2M|4M|8M",
		/*04*/		"Zorro3 mem",
		/*05*/		"^|None|1M|2M|4M|8M|16M|32M",		
		NULL
};

static const char *cpu_chipset_messages[] = {
		/*00*/		"CPU type",
		/*01*/		"^|68000|68010|68020|68030|68040|68060",
		/*03*/		"Chipset type",
		/*04*/		"^|OCS|ECS AGNUS|ECS|AGA",
		NULL
};

static const char *options_messages[] = {
		/*00*/		"CPU to chipset speed",
		/*01*/		"^|max|0%|34%|51%|68%|84%|100% Chipset",
		/*02*/		"Frameskip",
		/*03*/		"^|none|2|3|4|8|custom",
		/*04*/		"Floppy speed",
		/*05*/		"^|normal|turbo|400%|800%",
		/*06*/		"Correct aspect",
		/*07*/		"^|true|false",
		/*08*/		"Leds",
		/*09*/		"^|on|off",
		NULL
};

static const char *help_messages[] = {
		/*00*/		"#2HOME enters the menu system, where arrow",
		/*01*/		"#2keys and +/- are used to navigate up and down.",
		/*02*/		"#2You can bind keyboard keys to the wiimote",
		/*03*/		"#2buttons in the 'keyboard bindings' menu and",
		/*04*/		"#2change emulation options in the Amiga menu.",
		/*05*/		"#2 ",
		/*06*/		"#2Kickstart roms should be named kick.rom,",
		/*07*/		"#2kick10.rom, kick12.rom, kick13.rom, kick20.rom,",
		/*08*/		"#2kick30.rom, and kick31.rom for different Amiga",
		/*09*/		"#2models selectable in the Amiga menu.",
		/*10*/		"#2 ",
		/*11*/		"#2More information is available on the wiki:",
		/*12*/		"#2   http://wiibrew.org/wiki/UAE_Wii",
		/*13*/		"#2 ",
		/*14*/		"OK",
		NULL,
};


static int find_index_by_val(int val, const int vec[], int vec_size, int default_val)
{
	int i;

	for (i = 0; i < vec_size; i++)
	{
		if (val == vec[i])
			return i;
	}

	/* Some default */
	return default_val;
}

/* From PSPUAE (implementation is different though!) */
static void maybe_load_kick_rom(const char *name)
{
	const char *dir = prefs_get_attr("rom_path");
	char buf[255];
	struct stat st;

	if (!dir)
		dir="";
	snprintf(buf, 255, "%s/%s", dir, name);
	if (stat(buf, &st) < 0)
		return;

	/* Setup the new kickstart ROM */
	if (!S_ISDIR(st.st_mode))
		strncpy(changed_prefs.romfile, buf, 255);
}

/* All this is taken directly from PSPUAE */
static void A500_config(void)
{
	default_config();

	changed_prefs.cpu_level = 0; //68000
	changed_prefs.fastmem_size = 0; //OFF
	changed_prefs.chipmem_size = 512 * 1024; //512
	changed_prefs.bogomem_size = 512 * 1024; //512
	changed_prefs.chipset_mask = 1; //OCS

	maybe_load_kick_rom("kick13.rom");
}

static void A600_config(void)
{
	default_config();

	changed_prefs.cpu_level = 1; //68010
	changed_prefs.fastmem_size = 0; //OFF
	changed_prefs.chipmem_size = 1024 * 1024; //1024
	changed_prefs.bogomem_size = 0; //OFF
	changed_prefs.chipset_mask = 2; //ECS Agnus

	maybe_load_kick_rom("kick205.rom");
}

static void A1000_config(void)
{
	default_config();

	changed_prefs.cpu_level = 0; //68000
	changed_prefs.fastmem_size = 0; //OFF
	changed_prefs.chipmem_size = 256 * 1024; //512
	changed_prefs.bogomem_size = 0; //OFF
	changed_prefs.chipset_mask = 0; //OCS

	maybe_load_kick_rom("kick12.rom");
}

static void A1200_config(void)
{
	default_config();

	changed_prefs.cpu_level = 2; //68020
	changed_prefs.fastmem_size = 0; //OFF
	changed_prefs.chipmem_size = 1024 * 2048; //2048
	changed_prefs.bogomem_size = 0;
	changed_prefs.chipset_mask = 3; //AGA

	maybe_load_kick_rom("kick31.rom");
}

static void default_config(void)
{
	/* "good enough" and "fast enough" on the Wii */
	changed_prefs.gfx_framerate = 2;
}

static int prefs_has_changed;

static void insert_floppy(int which)
{
	const char *name = menu_select_file(prefs_get_attr("floppy_path"));

	if (name != NULL)
	{
		if (strcmp(name, "None") == 0)
			changed_prefs.df[which][0] = '\0';
		else
			strcpy (changed_prefs.df[which], name);
		free((void*)name);
	}
	else
		changed_prefs.df[which][0] = '\0';
}

static void insert_rom(void)
{
	const char *name = menu_select_file(prefs_get_attr("rom_path"));

	/* None or NULL means no change */
	if (name != NULL)
	{
		if (strcmp(name, "None") == 0)
			return;
		strcpy (changed_prefs.romfile, name);
		free((void*)name);
	}
}

static void cpu_chipset_options(void)
{
	const int cpu_levels[] = { 0, 1, 2, 3, 4, 6};
	const int chipset_masks[] = {0, 1, 2, 3};
	int submenus[2], opt;

	submenus[0] = find_index_by_val(changed_prefs.cpu_level, cpu_levels,
			sizeof(cpu_levels) / sizeof(cpu_levels[0]), 0);
	submenus[1] = find_index_by_val(changed_prefs.chipset_mask, chipset_masks,
			sizeof(chipset_masks) / sizeof(chipset_masks[0]), 0);

	opt = menu_select_title("CPU/Chipset options menu",
			cpu_chipset_messages, submenus);
	if (opt < 0)
		return;
	changed_prefs.cpu_level = cpu_levels[submenus[0]];
	changed_prefs.chipset_mask = chipset_masks[submenus[1]];

	prefs_has_changed = 1;
}

static void memory_options(void)
{
	const int chipmem_size[] = { 512 * 1024, 1024 * 1024, 2048 * 1024 };
	const int slowmem_size[] = { 0, 256 * 1024, 512 * 1024, 1024 * 1024, 1792 * 1024 };
	const int fastmem_size[] = { 0, 1024 * 1024, 2048 * 1024, 4096 * 1024, 8192 * 1024 };	
	const int z3fastmem_size[] = { 0, 1024 * 1024, 2048 * 1024, 4096 * 1024, 8192 * 1024, 16384 * 1024, 32768 * 1024};	
	//FOL - GFXCard no point in this yet, until GFX and HDD are working properly, we can then use Picasso screen modes.
	//const int gfxcard_size[] = { 0, 1024 * 1024, 2048 * 1024, 4096 * 1024, 8192 * 1024, 16384 * 1024 };
	int submenus[4], opt;

	memset(submenus, 0, sizeof(submenus));

	/* Setup current values */
	submenus[0] = find_index_by_val(changed_prefs.chipmem_size, chipmem_size,
			sizeof(chipmem_size) / sizeof(chipmem_size[0]), 0);
	submenus[1] = find_index_by_val(changed_prefs.bogomem_size, slowmem_size,
			sizeof(slowmem_size) / sizeof(slowmem_size[0]), 0);
	submenus[2] = find_index_by_val(changed_prefs.fastmem_size, fastmem_size,
			sizeof(fastmem_size) / sizeof(fastmem_size[0]), 0);
	submenus[3] = find_index_by_val(changed_prefs.z3fastmem_size, z3fastmem_size,
			sizeof(z3fastmem_size) / sizeof(z3fastmem_size[0]), 0);

	opt = menu_select_title("Memory options menu",
			memory_messages, submenus);
	if (opt < 0)
		return;
	/* And update with the new settings */
	changed_prefs.chipmem_size = chipmem_size[submenus[0]];
	changed_prefs.bogomem_size = slowmem_size[submenus[1]];
	changed_prefs.fastmem_size = fastmem_size[submenus[2]];
	changed_prefs.z3fastmem_size = z3fastmem_size[submenus[3]];

	prefs_has_changed = 1;
}

static int cpu_to_chipset_table[] = {-1,512,2560,5120,7168,8704,10240};

static int get_cpu_to_chipset_speed(void)
{
	return find_index_by_val(changed_prefs.m68k_speed, cpu_to_chipset_table,
			sizeof(cpu_to_chipset_table) / sizeof(cpu_to_chipset_table[0]), 0);
}

static void set_cpu_to_chipset_speed(int which)
{
	changed_prefs.m68k_speed = cpu_to_chipset_table[which];
}

static int get_floppy_speed(void)
{
	switch(changed_prefs.floppy_speed)
	{
	case 200:
		return 1;
	case 400:
		return 2;
	case 800:
		return 3;
	default: break; /* 100 */
	}
	return 0;
}

static void set_floppy_speed(int which)
{
	int table[] = {100, 0, 400, 800};

	changed_prefs.floppy_speed = table[which];
}

static void set_gfx_framerate(int which)
{
	int table[] = {1, 2, 3, 4, 8};

	/* Custom setting - don't touch! */
	if (which > sizeof(table) / sizeof(table[0]))
		return;
	changed_prefs.gfx_framerate = table[which];
}

static int get_gfx_framerate(void)
{
	int table[] = {1, 2, 3, 4, 8};

	return find_index_by_val(changed_prefs.gfx_framerate, table,
			sizeof(table) / sizeof(table[0]), 5);

}

static void general_options(void)
{
	int submenus[5];
	int opt;

	submenus[0] = get_cpu_to_chipset_speed();
	submenus[1] = get_gfx_framerate();
	submenus[2] = get_floppy_speed();
	submenus[3] = changed_prefs.gfx_correct_aspect == 0 ? 1 : 0;
	submenus[4] = currprefs.leds_on_screen == 0 ? 1 : 0;

	opt = menu_select_title("General options menu",
			options_messages, submenus);
	if (opt < 0)
		return;
	set_cpu_to_chipset_speed(submenus[0]);
	set_gfx_framerate(submenus[1]);
	set_floppy_speed(submenus[2]);

	changed_prefs.gfx_correct_aspect = !submenus[3];
	currprefs.gfx_correct_aspect = changed_prefs.gfx_correct_aspect;
	/* Floppy, Power, FPS, etc etc. */
	changed_prefs.leds_on_screen = !submenus[4];
	currprefs.leds_on_screen = changed_prefs.leds_on_screen;

	prefs_has_changed = 1;
}

/* There are a few unfortunate header problems, so I'll do like this for now */
struct uae_prefs;
void read_inputdevice_config (struct uae_prefs *pr, const char *option, const char *value);
void notice_screen_contents_lost (void);

static void insert_keyboard_map(const char *key, const char *fmt, ...)
{
	char buf[255];
	va_list ap;
	int r;

	va_start(ap, fmt);
	r = vsnprintf(buf, 255, fmt, ap);
	if (r >= 255)
		fprintf(stderr, "Too long string passed\n");
	va_end(ap);

	read_inputdevice_config (&changed_prefs, buf, key);
	read_inputdevice_config (&currprefs, buf, key);
}

static void setup_joystick_defaults(int joy)
{
	int fire_buttons[] = {3,7,9,10};
	int i;

	/* For some reason, the user uaerc removes these. The following
	 * lines should be removed when this is properly figured out */
	for (i = 0; i < 8; i++)
	{
		const char *what = "JOY2_HORIZ"; /* Assume port 1 */

		/* Odd - vertical */
		if (i % 2 != 0)
		{
			if (joy == 1)
				what = "JOY1_VERT";
			else
				what = "JOY2_VERT";
		}
		else if (joy == 1) /* Even - horizontal (and port 2) */
			what = "JOY1_HORIZ";

		insert_keyboard_map(what,
				"input.1.joystick.%d.axis.%d", joy, i);
	}

	insert_keyboard_map("SPC_ENTERGUI", "input.1.joystick.%d.button.6", joy);
	insert_keyboard_map("SPC_ENTERGUI", "input.1.joystick.%d.button.19", joy);

	for (i = 0; i < sizeof(fire_buttons) / sizeof(fire_buttons[0]); i++)
	{
		const char *btn = joy == 0 ? "JOY2_FIRE_BUTTON" : "JOY1_FIRE_BUTTON";

		insert_keyboard_map(btn, "input.1.joystick.%d.button.%d",
				joy, fire_buttons[i]);
	}
}

static void setup_joystick(int joy, const char *key, int sdl_key)
{
	insert_keyboard_map(key, "input.1.joystick.%d.button.%d", joy, sdl_key);
}

static void keyboard_options(void)
{
	const int wiimote_to_sdl[] = {2, 4, 5};
	const int nunchuk_to_sdl[] = {8};
	const int classic_to_sdl[] = {10, 11, 12, 13, 14, 15, 16, 17, 18};
	int sdl_key = 1;
	const char *key;
	int submenus[3];
	int opt;
	int i;

	memset(submenus, 0, sizeof(submenus));

	opt = menu_select_title("Keyboard menu",
			keyboard_messages, submenus);
	if (opt < 0)
		return;
	/* Translate key to UAE key event name */
	key = virtkbd_get_key();
	if (key == NULL)
		return;
	switch(opt)
	{
	case 0: /* wiimote */
		sdl_key = wiimote_to_sdl[submenus[0]]; break;
	case 2: /* nunchuk */
		sdl_key = nunchuk_to_sdl[submenus[1]]; break;
	case 4: /* classic */
		sdl_key = classic_to_sdl[submenus[2]]; break;
	default: /* can never happen */
		break;
	}

	for (i = 0; i < 2; i++)
		setup_joystick(i, key, sdl_key);

	prefs_has_changed = 1;
}


/* Helpers to determine the accuracy */
static int get_emulation_accuracy(void)
{
	if (currprefs.cpu_compatible == 0 &&
			currprefs.cpu_cycle_exact == 0)
		return 0;
	if (currprefs.cpu_compatible == 1 &&
			currprefs.cpu_cycle_exact == 0)
		return 1;
	return 2;
}

static void set_emulation_accuracy(int which)
{
	switch (which)
	{
	case 1:
		changed_prefs.cpu_compatible = 1;
		changed_prefs.cpu_cycle_exact = 0;
		break;
	case 2:
		changed_prefs.cpu_compatible = 1;
		changed_prefs.cpu_cycle_exact = 1;
		break;
	case 0:
	default:
		changed_prefs.cpu_compatible = 0;
		changed_prefs.cpu_cycle_exact = 0;
		break;
	}
}

static int get_model(void)
{
	if (changed_prefs.cpu_level == 1) /* 68010 - only on the A600 */
		return 2;
	if (changed_prefs.cpu_level == 2) /* 68020 - only on the A1200 */
		return 3;
	if (changed_prefs.cpu_level == 0) /* 68000 - A1000/A500 */
	{
		if (changed_prefs.chipmem_size == 256 * 1024) /* A1000 */
			return 0;

		/* A500 */
		return 1;
	}

	/* Custom */
	return 4;
}

static void amiga_model_options(void)
{
	int opt;
	int cur_model;
	int submenus[2];

	do
	{
		cur_model = get_model();
		submenus[0] = cur_model;
		submenus[1] = get_emulation_accuracy();

		opt = menu_select_title("Amiga model menu",
				amiga_model_messages, submenus);
		if (opt < 0)
			return;
		if (submenus[0] != cur_model)
		{
			switch(submenus[0])
			{
			case 0:	A1000_config(); break;
			case 1:	A500_config(); break;
			case 2:	A600_config(); break;
			case 3:	A1200_config(); break;
			default: /* custom */
				break;
			}
		}

		switch(opt)
		{
		case 4:
			memory_options(); break;
		case 5:
			cpu_chipset_options(); break;
		case 6:
			insert_rom(); break;
		default:
			break;
		}
	} while (opt == 4 || opt == 5 || opt == 6);

	/* Reset the Amiga if the model has changed */
	if (cur_model != submenus[0])
		uae_reset(1);
	/* Cycle-exact or not? */
	set_emulation_accuracy(submenus[1]);

	prefs_has_changed = 1;
}

static void save_load_state(int which)
{
	const char *dir = prefs_get_attr("savestate_path");
	const char *floppy0 = currprefs.df[0];
	char db[256];
	char fb[81];

	/* Name (for saves) */
	if (floppy0 && strrchr(floppy0, '/'))
		strncpy(fb, strrchr(floppy0, '/') + 1, 80);
	else
		strcpy(fb, "unknown");

	switch(which)
	{
	case 2:
	case 0: /* Load state */
	{
		const char *name = menu_select_file(dir);

		if (!name)
			return;

		if (which == 0)
		{
			strcpy(savestate_fname, name);
			savestate_state = STATE_DORESTORE;
		}
		else /* Delete saved state */
			unlink(name);
		free((void*)name);
	} break;
	case 1: /* Save state */
		snprintf(db, 255, "%s/%s.uss", dir, fb);
		savestate_state = STATE_DOSAVE;
		save_state(db, floppy0);
		break;
	default:
		break;
	}
}

static void help(void)
{
	menu_select_title("UAE-wii help",
			help_messages, NULL);
}

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

extern int log_quiet;
int gui_update (void)
{
	log_quiet = 1;

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

void gui_display(int shortcut)
{
	int submenus[3];
	int opt;

	memset(submenus, 0, sizeof(submenus));
	prefs_has_changed = 0;

	do
	{
		opt = menu_select_title("Main menu", main_menu_messages, submenus);
		notice_screen_contents_lost ();
		if (opt < 0)
			break;

		switch(opt)
		{
		case 0:
			/* Insert floppy */
			insert_floppy(submenus[0]);
			break;
		case 2:
			/* States */
			save_load_state(submenus[1]);
			break;
		case 5:
			uae_reset(1);
			break;
		case 6:
			amiga_model_options();
			break;
		case 7:
			keyboard_options();
			break;
		case 8:
			general_options();
			break;
		case 9:
			help();
			break;
		case 10:
			uae_quit();
			break;
		default:
			break;
		}
	} while (opt == 0 || opt == 6 || opt == 7 || opt == 8);

	if (prefs_has_changed)
	{
		char user_options[255] = "";
		int i;

#ifdef OPTIONS_IN_HOME
		char *home = getenv ("HOME");
		if (home != NULL && strlen (home) < 240)
		{
			strcpy (user_options, home);
			strcat (user_options, "/");
		}
#endif
		strcat(user_options, OPTIONSFILENAME);
		strcat(user_options, ".user");

		for (i = 0; i < 2; i++)
			setup_joystick_defaults(i);

		cfgfile_save(&changed_prefs, user_options, 0);
	}
}

void gui_message (const char *format,...)
{
       char msg[2048];
       va_list parms;

       va_start (parms,format);
       vsprintf ( msg, format, parms);
       va_end (parms);

       if (!menu_is_inited())
       {
    	   /* Some error message at startup - just quit */
    	   SDL_Surface *screen = SDL_SetVideoMode(640, 480, 16, 0);
    	   if (!screen)
    		   return; /* Deep trouble! */
    	   menu_init(screen);
       }
       msgYesNo(msg, 0, 24, 24);

       write_log (msg);
}
