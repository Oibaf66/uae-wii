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
#include "sounddep/sound.h"

#define ID_BUTTON_OFFSET 0
#define ID_AXIS_OFFSET 32

/* Uncomment for debugging output */
//#define DEBUG
#ifdef DEBUG
#define DEBUG_LOG write_log
#else
#define DEBUG_LOG(...) do ; while(0)
#endif

extern int usbismount, smbismount, sdismount;

int gui_is_active;

static void default_config(void);

static const char *main_menu_messages[] = {
		/*00*/		"Insert floppy",
		/*01*/		"^|df0|df1|df2|df3",
		/*02*/		"States",
		/*03*/		"^|Load|Save|Delete",
		/*04*/		"#1-------------------------------------",
		/*05*/		"Wiimote configuration",
		/*06*/		"^|Wiimote1|Wiimote2",
		/*07*/		"Virtual keyboard",
		/*08*/		"Hardware options",
		/*09*/		"Emulation options",
		/*10*/		"Other options",
		/*11*/		"Save confs",
		/*12*/		"Reset UAE",
		/*13*/		"Help",
		/*14*/		"Quit",
		NULL
};

static const  char *input_messages[] = {
		/*00*/		"Bind key to Wiimote",
		/*01*/		"^|1|2|-|+",
		/*02*/		"  ",
		/*03*/		"Bind key to Nunchuk",
		/*04*/		"^|Z|C",
		/*05*/		"  ",
		/*06*/		"Bind key to Classic",
		/*07*/		"^|a|b|x|y|L|R|Zl|Zr|-|+",
		/*08*/		"  ",
		/*09*/		"Mario kart wheel (horizontal only)",
		/*10*/		"^|On|Off",
		/*11*/		"  ",
		/*12*/		"Mouse emulation",
		/*13*/		"^|On|Off",
		NULL,
};

static const char *hardware_messages[] = {
		/*00*/		"Amiga model",
		/*01*/		"^|A1000|A500|A600|A1200|Custom",
		/*02*/		"  ",
		/*03*/		"Video system chipset",
		/*04*/		"^|PAL|NTSC",
		/*05*/		"  ",
		/*06*/		"CPU/Chipset options",
		/*07*/		"  ",
		/*08*/		"Memory options",
		/*09*/		"  ",
		/*10*/		"Change ROM",
		NULL
};

static const char *memory_messages[] = {
		/*00*/		"Chip mem",
		/*01*/		"^|512K|1M|2M",
		/*02*/		"  ",
		/*03*/		"Slow mem",
		/*04*/		"^|None|256K|512K|1M|1.8M",
		/*05*/		"  ",
		/*06*/		"Fast mem",
		/*07*/		"^|None|1M|2M|4M|8M",
		/*08*/		"  ",
		/*09*/		"Zorro3 mem",
		/*10*/		"^|None|1M|2M|4M|8M|16M|32M",		
		NULL
};

static const int chipmem_size_table[] = { 512 * 1024, 1024 * 1024, 2048 * 1024 };
static const int slowmem_size_table[] = { 0, 256 * 1024, 512 * 1024, 1024 * 1024, 1792 * 1024 };
static const int fastmem_size_table[] = { 0, 1024 * 1024, 2048 * 1024, 4096 * 1024, 8192 * 1024 };	
static const int z3fastmem_size_table[] = { 0, 1024 * 1024, 2048 * 1024, 4096 * 1024, 8192 * 1024, 16384 * 1024, 32768 * 1024};	

static const char *cpu_chipset_messages[] = {
		/*00*/		"CPU type",
		/*01*/		"^|68000|68010|68020|68020/68881|68040|68060",
		/*02*/		"  ",
		/*03*/		"Chipset type",
		/*04*/		"^|OCS|ECS AGNUS|ECS|AGA",
		NULL
};

static const int cpu_level_table[] = { 0, 1, 2, 3, 4, 6};
static const int chipset_mask_table[] = {0, CSMASK_ECS_AGNUS,
		CSMASK_ECS_AGNUS | CSMASK_ECS_DENISE,
		CSMASK_ECS_AGNUS | CSMASK_ECS_DENISE | CSMASK_AGA};


static const char *emulation_messages[] = {
		/*00*/		"Emulation accuracy",
		/*01*/		"^|Fast|Compatible|Cycle-exact",
		/*02*/		"CPU to chipset speed",
		/*03*/		"^|real|max|90%|80%|60%|40%|20%|0%",
		/*04*/		"Framerate",
		/*05*/		"^|100%|50%|33%|25%|12%|custom",
		/*06*/		"Floppy speed",
		/*07*/		"^|normal|turbo|400%|800%",
		/*08*/		"Sound interpolation",
		/*09*/		"^|none|rh|crux|sinc",
		/*10*/		"Collision level",
		/*11*/		"^|none|sprites|playfields|full",		
		/*12*/		"Immediate blits",
		/*13*/		"^|on|off",
		NULL
};

static const char *graphic_messages[] = {
		
		/*00*/		"Correct aspect",
		/*01*/		"^|off|100%|95%|93%|90%|custom",
		/*02*/		"Scanlines",
		/*03*/		"^|on|off",
		/*04*/		"Leds",
		/*05*/		"^|on|off",
		/*06*/		"Floppy sound",
		/*07*/		"^|on|off",
		/*08*/		"Port",
		/*09*/		"^|DEFAULT|SD|USB|SMB",
		/*10*/		"Rumble",
		/*11*/		"^|on|off",
		NULL
};

static const int correct_aspect_table[] = {0,100,95,93,90};
static const int cpu_to_chipset_table[] = {0,-1,512*2,512*4, 512*8, 512*12, 512*16, 512*20};
static const int floppy_table[] = {100, 0, 400, 800};
static const int framerate_table[] = {1, 2, 3, 4, 8};


static const char *help_messages[] = {
		/*00*/		"#2HOME enters the menu system, where arrow keys",
		/*01*/		"#2and nunchuck are used to navigate up and down.",
		/*02*/		"#2You can bind keyboard keys to the wiimote",
		/*03*/		"#2buttons in the 'keyboard bindings' menu and",
		/*04*/		"#2change emulation options in the hardware menu.",
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
static int maybe_load_kick_rom(const char *name)
{
	const char *dir = prefs_get_attr("rom_path");
	char buf[255];
	struct stat st;

	if (!dir)
		dir="";
	snprintf(buf, 255, "%s/%s", dir, name);
	if (stat(buf, &st) < 0)
		return 1;

	/* Setup the new kickstart ROM */
	if (!S_ISDIR(st.st_mode))
		strncpy(changed_prefs.romfile, buf, 255);

	return 0;
}

static int maybe_load_kick_rom_list(const char *roms[])
{
	const char **p;

	for (p = roms; p; p++)
	{
		/* OK to open? */
		if (maybe_load_kick_rom(*p) == 0)
			return 0;
	}

	return 1;
}

/* All this is taken directly from PSPUAE */
static void A500_config(void)
{
	const char *roms[] = {"amiga-os-130.rom", "kick13.rom", NULL};
	default_config();

	changed_prefs.cpu_level = 0; //68000
	changed_prefs.fastmem_size = 0; //OFF
	changed_prefs.chipmem_size = 512 * 1024; //512
	changed_prefs.bogomem_size = 512 * 1024; //512
	changed_prefs.chipset_mask = CSMASK_ECS_AGNUS; // A500 are OCS/ECS

	maybe_load_kick_rom_list( roms );
}

static void A600_config(void)
{
	const char *roms[] = {"amiga-os-205.rom", "kick205.rom", NULL};
	default_config();

	changed_prefs.cpu_level = 0; //68000
	changed_prefs.fastmem_size = 0; //OFF
	changed_prefs.chipmem_size = 1024 * 1024; //1024
	changed_prefs.bogomem_size = 0; //OFF
	changed_prefs.chipset_mask = CSMASK_ECS_AGNUS | CSMASK_ECS_DENISE; //ECS Agnus

	maybe_load_kick_rom_list(roms);
}

static void A1000_config(void)
{
	const char *roms[] = {"amiga-os-120.rom", "kick12.rom", NULL};
	default_config();

	changed_prefs.cpu_level = 0; //68000
	changed_prefs.fastmem_size = 0; //OFF
	changed_prefs.chipmem_size = 256 * 1024; //512
	changed_prefs.bogomem_size = 0; //OFF
	changed_prefs.chipset_mask = 0; //OCS

	maybe_load_kick_rom_list(roms);
}

static void A1200_config(void)
{
	const char *roms[] = {"amiga-os-310.rom", "kick31.rom", NULL};
	default_config();

	changed_prefs.cpu_level = 2; //68020
	changed_prefs.fastmem_size = 0; //OFF
	changed_prefs.chipmem_size = 1024 * 2048; //2048
	changed_prefs.bogomem_size = 0;
	changed_prefs.chipset_mask = CSMASK_ECS_AGNUS | CSMASK_ECS_DENISE | CSMASK_AGA; //AGA

	maybe_load_kick_rom_list(roms);
}

static void default_config(void)
{
	/* "good enough" and "fast enough" on the Wii */
	changed_prefs.gfx_framerate = 2;
}


static void insert_floppy(int which)
{
	const char *selected_file=changed_prefs.df[which];
	char *name;
	char *ptr_file_name;
	char dir[255];
	strncpy(dir,prefs_get_attr("floppy_path"),255);
	name = (char *) menu_select_file(dir, selected_file, which);
	
	if (name != NULL)
	{
		ptr_file_name = strrchr(name,'/');
		if (ptr_file_name) ptr_file_name++; else ptr_file_name = name;
		if (strcmp(ptr_file_name, "None") == 0)
			changed_prefs.df[which][0] = '\0';
		else strcpy(changed_prefs.df[which], name);
			
		ptr_file_name = strrchr(name,'/');
		if (ptr_file_name)
			{
				*ptr_file_name=0; //extract the dir from the path
				if (strrchr(name,'/')==NULL) {*ptr_file_name='/'; *(ptr_file_name+1)=0;} //check if it was root
			}
		prefs_set_attr("floppy_path", strdup(name));
		free((void*)name);
	}
}

static void insert_rom(void)
{
	const char *name = menu_select_file(prefs_get_attr("rom_path"),NULL, -1);

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
	int submenus[2], opt;

	submenus[0] = find_index_by_val(changed_prefs.cpu_level, cpu_level_table,
			SDL_arraysize(cpu_level_table), 0);
	submenus[1] = find_index_by_val(changed_prefs.chipset_mask, chipset_mask_table,
			SDL_arraysize(chipset_mask_table), 0);

	opt = menu_select_title("CPU/Chipset options menu",
			cpu_chipset_messages, submenus);
	if (opt < 0)
		return;
	changed_prefs.cpu_level = cpu_level_table[submenus[0]];
	changed_prefs.chipset_mask = chipset_mask_table[submenus[1]];

}

static void memory_options(void)
{
	//FOL - GFXCard no point in this yet, until GFX and HDD are working properly, we can then use Picasso screen modes.
	//const int gfxcard_size[] = { 0, 1024 * 1024, 2048 * 1024, 4096 * 1024, 8192 * 1024, 16384 * 1024 };
	int submenus[4], opt;

	memset(submenus, 0, sizeof(submenus));

	/* Setup current values */
	submenus[0] = find_index_by_val(changed_prefs.chipmem_size, chipmem_size_table,
			SDL_arraysize(chipmem_size_table), 0);
	submenus[1] = find_index_by_val(changed_prefs.bogomem_size, slowmem_size_table,
			SDL_arraysize(slowmem_size_table), 0);
	submenus[2] = find_index_by_val(changed_prefs.fastmem_size, fastmem_size_table,
			SDL_arraysize(fastmem_size_table), 0);
	submenus[3] = find_index_by_val(changed_prefs.z3fastmem_size, z3fastmem_size_table,
			SDL_arraysize(z3fastmem_size_table), 0);

	opt = menu_select_title("Memory options menu",
			memory_messages, submenus);
	if (opt < 0)
		return;
	/* And update with the new settings */
	changed_prefs.chipmem_size = chipmem_size_table[submenus[0]];
	changed_prefs.bogomem_size = slowmem_size_table[submenus[1]];
	changed_prefs.fastmem_size = fastmem_size_table[submenus[2]];
	changed_prefs.z3fastmem_size = z3fastmem_size_table[submenus[3]];

}

static int get_cpu_to_chipset_speed(void)
{
	return find_index_by_val(changed_prefs.m68k_speed, cpu_to_chipset_table,
			SDL_arraysize(cpu_to_chipset_table), 0);
}

static void set_cpu_to_chipset_speed(int which)
{
	changed_prefs.m68k_speed = cpu_to_chipset_table[which];
}

static int get_floppy_speed(void)
{
	return find_index_by_val(changed_prefs.floppy_speed, floppy_table,
			SDL_arraysize(floppy_table), 0);
}

static void set_floppy_speed(int which)
{
	changed_prefs.floppy_speed = floppy_table[which];
}

static void set_gfx_framerate(int which)
{
	/* Custom setting - don't touch! */
	if (which > SDL_arraysize(framerate_table)-1)
		return;
	changed_prefs.gfx_framerate = framerate_table[which];
}

static int get_gfx_framerate(void)
{
	return find_index_by_val(changed_prefs.gfx_framerate, framerate_table,
			SDL_arraysize(framerate_table), 5);

}

static void set_gfx_aspect_ratio(int which)
{
	if (!which) {changed_prefs.gfx_correct_aspect = 0; return;}
	/* Custom setting or correct aspect off - don't touch! */
	if (which > SDL_arraysize(correct_aspect_table)-1)
		return;
	changed_prefs.gfx_correct_aspect = 1;
	changed_prefs.gfx_correct_ratio = correct_aspect_table[which];
}

static int get_gfx_aspect_ratio(void)
{	
	if (!changed_prefs.gfx_correct_aspect) return 0; 
	else
		return find_index_by_val(changed_prefs.gfx_correct_ratio, correct_aspect_table,
			SDL_arraysize(correct_aspect_table), 5);
}

/* Helpers to determine the accuracy */
static int get_emulation_accuracy(void)
{
	if (changed_prefs.cpu_compatible == 0 &&
			changed_prefs.cpu_cycle_exact == 0)
		return 0;
	if (changed_prefs.cpu_compatible == 1 &&
			changed_prefs.cpu_cycle_exact == 0)
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
	
	if ((changed_prefs.chipset_mask&CSMASK_AGA)&&(changed_prefs.cpu_level == 2)) /* A1200 */
		return 3;
	if ((changed_prefs.chipset_mask&CSMASK_ECS_DENISE)&&(changed_prefs.cpu_level == 0)) /* A600 */
		return 2;
	if ((changed_prefs.chipset_mask&CSMASK_ECS_AGNUS)&&(changed_prefs.cpu_level == 0)) /* A500 */
		return 1;
	if ((changed_prefs.chipset_mask == 0)&&(changed_prefs.cpu_level == 0)) /* A1000 */
		return 0;
		
	/* Custom */
	return 4;
}


static void set_Port(int which)
{
	switch (which)
	{
	case PORT_DEFAULT:
		prefs_set_attr ("floppy_path",    strdup(TARGET_FLOPPY_PATH));
		changed_prefs.Port = which;
		currprefs.Port = changed_prefs.Port;
		break;
	case PORT_SD:
		if (sdismount) {
		prefs_set_attr ("floppy_path",    strdup(TARGET_SD_PATH));
		changed_prefs.Port = which;
		currprefs.Port = changed_prefs.Port;}
		else
			msgInfo("SD is not mounted",3000,NULL);
		break;
	case PORT_USB:
		if (usbismount) {
			prefs_set_attr ("floppy_path",    strdup(TARGET_USB_PATH));
			changed_prefs.Port = which;
			currprefs.Port = changed_prefs.Port;}
		else
			msgInfo("USB is not mounted",3000,NULL);
		break;
	case PORT_SMB:
		if (smbismount) {
			prefs_set_attr ("floppy_path",    strdup(TARGET_SMB_PATH));
			changed_prefs.Port = which;
			currprefs.Port = changed_prefs.Port;}
		else
			msgInfo("SMB is not mounted",3000,NULL);
		break;
	default:
		break;		
	}	
}

static void save_configurations(void)
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
strcat(user_options, ".saved");

cfgfile_save(&changed_prefs, user_options, 0);
msgInfo("Configurations saved",3000,NULL);
}	

int get_dfxclick(void)
{
	int sounddf_on = 0; 
	int i;

	for (i=0; i < 4; i++)
	 if (changed_prefs.dfxclick[i]&&(changed_prefs.dfxtype[i]>=0)) sounddf_on =1;
	
	return sounddf_on;
}

void set_dfxclick(int sounddf_on)
{
	int i;
	for (i=0; i < 4; i++)
	 if ((changed_prefs.dfxtype[i]>=0)&&(changed_prefs.dfxclick[i]!=sounddf_on))
	    changed_prefs.dfxclick[i] = sounddf_on;
}

static void emulation_options(void)
{
	int submenus[7];
	int opt;
	
	memset(submenus, 0, sizeof(submenus));
	
	submenus[0] = get_emulation_accuracy();
	submenus[1] = get_cpu_to_chipset_speed();
	submenus[2] = get_gfx_framerate();
	submenus[3] = get_floppy_speed();
	submenus[4] = changed_prefs.sound_interpol;
	submenus[5] = changed_prefs.collision_level;
	submenus[6] = !changed_prefs.immediate_blits;
	
	opt = menu_select_title("Emulation options menu",
			emulation_messages, submenus);
	if (opt < 0)
		return;
	/* Cycle-exact or not? */
	set_emulation_accuracy(submenus[0]);
	
	set_cpu_to_chipset_speed(submenus[1]);
	set_gfx_framerate(submenus[2]);
	set_floppy_speed(submenus[3]);
	changed_prefs.sound_interpol = submenus[4];
	changed_prefs.collision_level = submenus[5];	
	changed_prefs.immediate_blits = !submenus[6];
	
}

static void graphic_options(void)
{
	int submenus[6];
	int opt;
	
	memset(submenus, 0, sizeof(submenus));
	
	
	submenus[0] = get_gfx_aspect_ratio();
	submenus[1] = !(changed_prefs.gfx_linedbl == 2) ;
	submenus[2] = !changed_prefs.leds_on_screen;
	submenus[3] = !get_dfxclick();
	submenus[4] = changed_prefs.Port;
	submenus[5] = !changed_prefs.rumble;

	opt = menu_select_title("Other options menu",
			graphic_messages, submenus);
	if (opt < 0)
		return;

	set_gfx_aspect_ratio(submenus[0]);
	changed_prefs.gfx_linedbl = submenus[1] ? 1 : 2;
	changed_prefs.leds_on_screen = !submenus[2];
	set_dfxclick(!submenus[3]);
	set_Port(submenus[4]);
	changed_prefs.rumble = !submenus[5];
	currprefs.leds_on_screen = changed_prefs.leds_on_screen;
	currprefs.rumble = changed_prefs.rumble;
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

	//printf("Mibb: %s:%s\n", buf, key);
	read_inputdevice_config (&changed_prefs, buf, key);

	inputdevice_config_change();
}


static void setup_joystick(int joy, const char *key, int sdl_key)
{
	if (!strcmp(key, "None")) 
	{
	changed_prefs.joystick_settings[1][joy].eventid[ID_BUTTON_OFFSET + sdl_key][0] = 0;
	inputdevice_config_change();
	}
	else
	insert_keyboard_map(key, "input.1.joystick.%d.button.%d", joy, sdl_key);
}

static void input_options(int joy)
{
	const int wiimote_to_sdl[] = {2, 3, 4, 5};
	const int nunchuk_to_sdl[] = {7, 8};
	const int classic_to_sdl[] = {9, 10, 11, 12, 13, 14, 15, 16, 17, 18};
	int sdl_key = 1;
	const char *key;
	int submenus[5];
	int opt;
	int i;
	struct virtkey *virtualkey;

	memset(submenus, 0, sizeof(submenus));
	submenus[3] = !changed_prefs.joystick_settings[1][joy].eventid[ID_AXIS_OFFSET + 6][0];
	submenus[4] = (changed_prefs.mouse_settings[1][joy].enabled == 0);

	opt = menu_select_title("Input menu",
			input_messages, submenus);
	if (opt < 0)
		return;
	/* Translate key to UAE key event name */
	
	if (opt == 9)
	{	
		if (!submenus[3]){
			if (!joy) insert_keyboard_map("JOY2_HORIZ","input.1.joystick.%d.axis.6", 0);
			else insert_keyboard_map("JOY1_HORIZ" ,"input.1.joystick.%d.axis.6", 1);}
		else{
				changed_prefs.joystick_settings[1][joy].eventid[ID_AXIS_OFFSET + 6][0] = 0;
				inputdevice_config_change();
			}
		return;
	}

	if (opt == 12)
	{
		if (submenus[4])
		{
		changed_prefs.mouse_settings[1][joy].enabled = 0;
		}
		else
		{
		changed_prefs.mouse_settings[1][joy].enabled = 1;
		}
		inputdevice_config_change();
		return;
	}
	
	virtualkey = virtkbd_get_key();
	if (virtualkey == NULL)
		return;
	key = virtualkey->ev_name;
	
	switch(opt)
		{
		case 0: /* wiimote */
			sdl_key = wiimote_to_sdl[submenus[0]]; break;
		case 3: /* nunchuk */
			sdl_key = nunchuk_to_sdl[submenus[1]]; break;
		case 6: /* classic */
			sdl_key = classic_to_sdl[submenus[2]]; break;
		default: /* can never happen */
			break;
		}
	if (!strcmp(key,"JOY_FIRE_BUTTON"))
			key= joy ? "JOY1_FIRE_BUTTON": "JOY2_FIRE_BUTTON";
			
	if (!strcmp(key,"JOY_2ND_BUTTON"))
			key= joy ? "JOY1_2ND_BUTTON": "JOY2_2ND_BUTTON";
			
	if (!strcmp(key,"JOY_3RD_BUTTON"))
			key= joy ? "JOY1_3RD_BUTTON": "JOY2_3RD_BUTTON";
		
	setup_joystick(joy, key, sdl_key);
	
}

static void virtual_keyboard(void)
{
	int key_code;
	
	virtkey_t *key =virtkbd_get_key();  
	if (key) {key_code = key->sdl_code;} else return;
	
	SDL_Event event_key;
	
	event_key.type=SDL_KEYDOWN;
	event_key.key.keysym.sym=key_code;
	SDL_PushEvent(&event_key);
	DEBUG_LOG ("Push Event: keycode %d %s\n", key_code, "SDL_KEYDOWN");
	
	event_key.type=SDL_KEYUP;
	SDL_PushEvent(&event_key);
	DEBUG_LOG ("Push Event: keycode %d %s\n", key_code, "SDL_KEYUP");
	
}	


static void hardware_options(void)
{
	int opt;
	int cur_model;
	int submenus[2];

	do
	{
		cur_model = get_model();
		submenus[0] = cur_model;
		submenus[1] = changed_prefs.ntscmode;

		opt = menu_select_title("Hardware option menu",
				hardware_messages, submenus);
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
		
		changed_prefs.ntscmode = submenus[1];
		
		switch(opt)
		{
		case 6:
			cpu_chipset_options(); break;
		case 8:
			memory_options(); break;
		case 10:
			insert_rom(); break;
		default:
			break;
		}
	} while (opt == 6 || opt == 8 || opt == 10);

	/* Reset the Amiga if the model has changed */
	if (cur_model != submenus[0])
		uae_reset(1);
	

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
		const char *name = menu_select_file(dir, NULL,-1);

		if (!name)
			return;

		if (ext_matches(name, ".uss")|ext_matches(name, ".USS"))
		{
			if (which == 0)
			{
				strcpy(savestate_fname, name);
				savestate_state = STATE_DORESTORE;
			}
			else /* Delete saved state */
				unlink(name);
		}	
		free((void*)name);
	} break;
	case 1: /* Save state */
		snprintf(db, 255, "%s/%s.uss", dir, fb);
		savestate_state = STATE_DOSAVE;
		save_state(db, floppy0);
		msgInfo("State saved",3000,NULL);
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
	#ifndef GEKKO
	log_quiet = 1;
	#endif

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
	gui_is_active=1;
	pause_sound();
	
	memset(submenus, 0, sizeof(submenus));

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
			input_options(submenus[2]);
			break;
		case 7:
			virtual_keyboard();
			break;	
		case 8:
			hardware_options();
			break;	
		case 9:
			emulation_options();
			break;
		case 10:
			graphic_options();
			break;	
		case 11:	
			save_configurations();
			break;
		case 12:
			uae_reset(1);
			break;	
		case 13:
			help();
			break;
		case 14:
			if (msgYesNo("Are you sure to quit?", 0, FULL_DISPLAY_X /2-138, FULL_DISPLAY_Y /2-48)) 
				{currprefs.rumble=0; uae_quit();}	
			break;
		default:
			break;
		}
	} while (opt == 0 || opt == 5 || opt == 8 || opt == 9 || opt == 10 || opt == 13);
	
	resume_sound();
	gui_is_active=0;
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
       msgInfo(msg, -1, NULL);

       write_log (msg);
}
