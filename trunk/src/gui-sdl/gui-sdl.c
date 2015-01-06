 /*
  * UAE - The Un*x Amiga Emulator
  *
  * WII Interface
  *
  * Copyright 2014 Fabio Olimpieri
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
#include "audio.h"
#include "inputdevice.h"
#include "filesys.h"

#define ID_BUTTON_OFFSET 0
#define ID_AXIS_OFFSET 32

/* Uncomment for debugging output */
//#define DEBUG_VK
#ifdef DEBUG_VK
#define DEBUG_LOG write_log
#else
#define DEBUG_LOG(...) do {} while(0)
#endif

extern int usbismount, smbismount, sdismount;

int gui_is_active;

static void default_config(void);

static const char *main_menu_messages[] = {
		/*00*/		"Insert floppy",
		/*01*/		"^|df0|df1|df2|df3",
		/*02*/		"States",
		/*03*/		"^|Load|Save|Delete",
		/*04*/		"#1-----------------------------------------------",
		/*05*/		"Wiimote configuration",
		/*06*/		"^|Wiimote1|Wiimote2",
		/*07*/		"Hardware options",
		/*08*/		"Emulation options",
		/*09*/		"Audio options",
		/*10*/		"Other options",
		/*11*/		"HD emulation",
		/*12*/		"Save confs",
		/*13*/		"Load confs",
		/*14*/		"Reset UAE",
		/*15*/		"Quit",
		NULL
};

static const  char *input_messages[] = {
		/*00*/		"Bind key to Wiimote",
		/*01*/		"^|1|2|-",
		/*02*/		"Bind key to Nunchuk",
		/*03*/		"^|Z|C",
		/*04*/		"Bind key to Classic",
		/*05*/		"^|a|b|x|y|L|R|Zl|Zr|-",
		/*06*/		"Mario kart wheel (horizontal only)",
		/*07*/		"^|On|Off",
		/*08*/		"Mouse emulation",
		/*09*/		"^|On|Off",
		/*10*/		"Rumble",
		/*11*/		"^|on|off",
		/*12*/		"Autofire delay",
		/*13*/		"^|3|6|9|12|15",
		NULL
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
		/*01*/		"^|512K|1M|2M|4M|8M",
		/*02*/		"  ",
		/*03*/		"Slow mem",
		/*04*/		"^|None|256K|512K|1M|1.8M",
		/*05*/		"  ",
		/*06*/		"Fast mem",
		/*07*/		"^|None|1M|2M|4M|8M",
		/*08*/		"  ",
		/*09*/		"Zorro3 mem",
		/*10*/		"^|None|1M|2M|4M|8M",
		/*11*/		"  ",
		/*12*/		"Picasso96 mem",
		/*13*/		"^|None|1M|2M|4M|8M|16M",
		NULL
};

static const int chipmem_size_table[] = { 512 * 1024, 1024 * 1024, 2048 * 1024, 4096 * 1024, 8192 * 1024};
static const int slowmem_size_table[] = { 0, 256 * 1024, 512 * 1024, 1024 * 1024, 1792 * 1024 };
static const int fastmem_size_table[] = { 0, 1024 * 1024, 2048 * 1024, 4096 * 1024, 8192 * 1024 };	
static const int z3fastmem_size_table[] = { 0, 1024 * 1024, 2048 * 1024, 4096 * 1024, 8192 * 1024};
static const int picasso96_size_table[] = { 0, 1024 * 1024, 2048 * 1024, 4096 * 1024, 8192 * 1024, 16384 * 1024};	

static const char *cpu_chipset_messages[] = {
		/*00*/		"CPU type",
		/*01*/		"^|68000|68010|68020|68020/68881|68040|68060",
		/*02*/		"  ",
		/*03*/		"CPU address space",
		/*04*/		"^|24 bit|32 bit",
		/*05*/		"  ",
		/*06*/		"Chipset type",
		/*07*/		"^|OCS|ECS AGNUS|ECS|AGA",
		NULL
};

static const int cpu_level_table[] = { 0, 1, 2, 3, 4, 6};
static const int chipset_mask_table[] = {0, CSMASK_ECS_AGNUS,
		CSMASK_ECS_AGNUS | CSMASK_ECS_DENISE,
		CSMASK_ECS_AGNUS | CSMASK_ECS_DENISE | CSMASK_AGA};


static const char *emulation_messages[] = {
		/*00*/		"CPU emulation accuracy",
		/*01*/		"^|Fast|Compatible|Cycle-exact",
		/*02*/		"CPU to chipset speed",
		/*03*/		"^|real|max|90%|80%|60%|40%|20%|0%",
		/*04*/		"Framerate",
		/*05*/		"^|100%|50%|33%|25%|12%|custom",
		/*06*/		"Refresh rate (Hz)",
		/*07*/		"^|off|30|35|40|45|50|60|custom",		
		/*08*/		"Collision level",
		/*09*/		"^|none|sprites|playfields|full",		
		/*10*/		"Immediate blits",
		/*11*/		"^|on|off",
		/*12*/		"Blitter cycle exact",
		/*13*/		"^|on|off",
		/*12*/		"Vsync",
		/*13*/		"^|on|off",
		NULL
};

static const char *audio_messages[] = {
		/*00*/		"Sound ouput",
		/*01*/		"^|none|normal|exact",
		/*02*/		"  ",
		/*03*/		"Sound stereo separation",
		/*04*/		"^|0|20%|40%|60%|80%|100%",
		/*05*/		"  ",
		/*06*/		"Sound stereo delay",
		/*07*/		"^|0|2|4|6|8|10",
		/*08*/		"  ",
		/*09*/		"Sound interpolation",
		/*10*/		"^|none|rh|crux|sinc",
		/*11*/		"  ",
		/*12*/		"Floppy sound",
		/*13*/		"^|on|off",
		NULL
};

static const char *other_messages[] = {
		
		/*00*/		"Floppy speed",
		/*01*/		"^|normal|turbo|400%|800%",
		/*02*/		"Number of floppies",
		/*03*/		"^| 0 | 1 | 2 | 3 | 4 ",
		/*04*/		"Correct aspect ratio",
		/*05*/		"^|off|100%|95%|93%|90%|custom",
		/*06*/		"Resolution",
		/*07*/		"^|320X240|640X480",
		/*08*/		"Scanlines",
		/*09*/		"^|on|off",
		/*10*/		"Leds",
		/*11*/		"^|on|off",
		/*12*/		"Port",
		/*13*/		"^|DEFAULT|SD|USB|SMB",
		NULL
};

static const char *save_conf_messages[] = {
		/*00*/		"Save conf file to load at start (uaerc.saved)",
		/*01*/		" ",
		/*02*/		"Save confs file 1",
		/*03*/		" ",
		/*04*/		"Save confs file 2",
		/*05*/		" ",
		/*06*/		"Save confs file 3",
		/*07*/		" ",
		/*08*/		"Save confs file 4",
		/*09*/		" ",
		/*10*/		"Save confs file 5",
		/*11*/		" ",
		/*12*/		"Delete uaerc.saved",
		NULL
};

static const char *load_conf_messages[] = {
		/*00*/		"Load default conf file (uaerc)",
		/*01*/		" ",
		/*02*/		"Load confs file 1",
		/*03*/		" ",
		/*04*/		"Load confs file 2",
		/*05*/		" ",
		/*06*/		"Load confs file 3",
		/*07*/		" ",
		/*08*/		"Load confs file 4",
		/*09*/		" ",
		/*10*/		"Load confs file 5",
		NULL
};

static const char *hd_emulation_messages[] = {
		/*00*/		"Mount virtual filesystem",
		/*01*/		" ",
		/*02*/		"Mount RDB hard disk",
		/*03*/		" ",
		/*04*/		"Mount hard drive partition",
		/*05*/		" ",
		/*06*/		"Unmount device",
		/*07*/		" ",
		/*08*/		"Make hardfile",
		/*09*/		" ",
		/*10*/		"Delete hardfile",
		NULL
};

static const char *VFS_configuration_messages[] = {
		/*00*/		"Device Name",
		/*01*/		"^|DH0|DH1|DH2|DH3|DH4|DH5",
		/*02*/		"Volume name",
		/*03*/		"^|VFS0|VFS1|VFS2|VFS3|VFS4",
		/*04*/		"Access",
		/*05*/		"^| RW | RO ",
		/*06*/		"Boot priority",
		/*07*/		"^| 0 | 1 | 2 | 3 |127|-128",
		NULL
};

static const char *partition_configuration_messages[] = {
		/*00*/		"Device Name",
		/*01*/		"^|DH0|DH1|DH2|DH3|DH4|DH5",
		/*02*/		"Access",
		/*03*/		"^| RW | RO ",
		/*04*/		"Sectors per track",
		/*05*/		"^|32|64|128|256",
		/*06*/		"Surfaces/heads",
		/*07*/		"^| 1 | 2 | 4 | 8 | 16 ",
		/*08*/		"Reserved blocks",
		/*09*/		"^| 0 | 1 | 2 | 3 | 4 ",
		/*10*/		"Block size",
		/*11*/		"^|512|1024|2048|4096",
		/*12*/		"Boot priority",
		/*13*/		"^| 0 | 1 | 2 | 3 |127|-128",
		NULL
};

static const char *make_hdf_messages[] = {
		/*00*/		"Name",
		/*01*/		"^|HDF0|HDF1|HDF2|HDF3|HDF4|HDF5",
		/*01*/		"Size",
		/*02*/		"^|10M|20M|50M|100M|200M|500M",
		/*03*/		"Sectors per track",
		/*04*/		"^|32|64|128|256",
		/*05*/		"Surfaces/heads",
		/*06*/		"^| 1 | 2 | 4 | 8 | 16 ",
		/*07*/		"Block size",
		/*08*/		"^|512|1024|2048|4096",
		NULL
};

static const char *RDB_configuration_messages[] = {
		/*04*/		"Access",
		/*05*/		"^| RW | RO ",
		NULL
};

static const char *device_table[] = {"DH0","DH1","DH2","DH3","DH4","DH5"};
static const char *volume_table[] = {"VFS0","VFS1","VFS2","VFS3","VFS4"};
static const char *hdf_name_table[] = {"HDF0","HDF1","HDF2","HDF3","HDF4","HDF5"};
static const int sector_table[] = {32, 64, 128, 256};
static const int surface_table[] = {1, 2, 4, 8, 16};
static const int blocksize_table[] = {512, 1024, 2048, 4096};
static const int boot_priority_table[] = {0, 1, 2, 3, 127, -128};
static const int hdf_size_table[] = {10*1024*1024,20*1024*1024,50*1024*1024,100*1024*1024,10*1024*1024, 200*1024*1024,500*1024*1024 };
static const int correct_aspect_table[] = {0,100,95,93,90};
static const int cpu_to_chipset_table[] = {0,-1,512*2,512*4, 512*8, 512*12, 512*16, 512*20};
static const int floppy_table[] = {100, 0, 400, 800};
static const int framerate_table[] = {1, 2, 3, 4, 8};
static const int refreshrate_table[] = {0, 30, 35, 40, 45, 50, 60};

/*
static const char *help_messages[] = {
		*00*		"#2HOME enters the menu system, where arrow keys",
		*01*		"#2and nunchuck are used to navigate up and down.",
		*02*		"#2You can bind keyboard keys to the wiimote",
		*03*		"#2buttons in the 'keyboard bindings' menu and",
		*04*		"#2change emulation options in the hardware menu.",
		*05*		"#2 ",
		*06*		"#2Kickstart roms should be named kick.rom,",
		*07*		"#2kick10.rom, kick12.rom, kick13.rom, kick20.rom,",
		*08*		"#2kick30.rom, and kick31.rom for different Amiga",
		*09*		"#2models selectable in the Amiga menu.",
		*10*		"#2 ",
		*11*		"#2More information is available on the wiki:",
		*12*		"#2   http://wiibrew.org/wiki/UAE_Wii",
		*13*		"#2 ",fixe
		*14*		"OK",
		NULL,
};
*/

void fix_options_menu_sdl (int printmsg)
{
	if ((changed_prefs.cpu_level < 2)&&(changed_prefs.address_space_24==0))
	{
	changed_prefs.address_space_24 = 1;	
		if (printmsg) msgInfo ("With 68000/68010 address space is 24 bit",4000, NULL);
	}
	if ((changed_prefs.cpu_level >= 4)&&(changed_prefs.address_space_24==1))
	{	
	changed_prefs.address_space_24 = 0;
		if (printmsg) msgInfo ("With 68040/060 address space is 32 bit",4000, NULL);
	} 
	
	if ((changed_prefs.cpu_level != 0)&&(changed_prefs.cpu_compatible == 1))
	{	
	changed_prefs.cpu_compatible = 0;
		if (printmsg) msgInfo (" Compatible emulation only for 68000 ",4000, NULL);	 
	}
	
	if ((changed_prefs.cpu_level != 0)&&(changed_prefs.cpu_cycle_exact == 1))
	{	
	changed_prefs.cpu_cycle_exact = 0;		
		if (printmsg) msgInfo (" Cycle-exact emulation only for 68000 ",4000, NULL);	 
	}
	
	if ((changed_prefs.chipmem_size & (changed_prefs.chipmem_size - 1)) != 0
	|| changed_prefs.chipmem_size < 0x40000
	|| changed_prefs.chipmem_size > 0x800000)
    {
	changed_prefs.chipmem_size = 0x80000;
		if (printmsg) msgInfo ("Unsupported chipmem size",4000, NULL);
    }
	
	if ((changed_prefs.chipmem_size > 0x80000) && (!(changed_prefs.chipset_mask & CSMASK_ECS_AGNUS)))
	{
	changed_prefs.chipmem_size = 0x80000;
		if (printmsg) msgInfo ("No more than 512KB chipmem with OCS",4000, NULL);
    }
	
	if ((changed_prefs.bogomem_size > 0x100000) && ((changed_prefs.chipset_mask & CSMASK_AGA)))
	{
	changed_prefs.bogomem_size = 0x100000;
		if (printmsg) msgInfo ("No more than 1MB slowmem with AGA",4000, NULL);
    }
	
    if ((changed_prefs.fastmem_size & (changed_prefs.fastmem_size - 1)) != 0
	|| (changed_prefs.fastmem_size != 0 && (changed_prefs.fastmem_size < 0x100000 || changed_prefs.fastmem_size > 0x800000)))
    {
	changed_prefs.fastmem_size = 0;
		if (printmsg) msgInfo ("Unsupported fastmem size",4000, NULL);
    }
	
    if ((changed_prefs.gfxmem_size & (changed_prefs.gfxmem_size - 1)) != 0
	|| (changed_prefs.gfxmem_size != 0 && (changed_prefs.gfxmem_size < 0x100000 || changed_prefs.gfxmem_size > 0x2000000)))
    {
		if (printmsg) msgInfo ("Unsupported graphics card memory size",4000, NULL);
	changed_prefs.gfxmem_size = 0;
    }
	
    if ((changed_prefs.z3fastmem_size & (changed_prefs.z3fastmem_size - 1)) != 0
	|| (changed_prefs.z3fastmem_size != 0 && (changed_prefs.z3fastmem_size < 0x100000 || changed_prefs.z3fastmem_size > 0x20000000)))
    {
	changed_prefs.z3fastmem_size = 0;
		if (printmsg) msgInfo ("Unsupported Zorro III fastmem size",4000, NULL);
    }
	
    if (changed_prefs.address_space_24 && (changed_prefs.gfxmem_size != 0)) {
	changed_prefs.gfxmem_size = 0;
		if (printmsg) msgInfo ("Can't use graphic mem with 24 bit address",4000, NULL);
	}
	
	if (changed_prefs.address_space_24 && (changed_prefs.z3fastmem_size != 0)) {
	changed_prefs.z3fastmem_size = 0;
		if (printmsg) msgInfo (" Can't use Zorro III with 24 bit address ",4000, NULL);
    }
	
    if (changed_prefs.bogomem_size != 0 && changed_prefs.bogomem_size != 0x80000 && changed_prefs.bogomem_size != 0x100000 && changed_prefs.bogomem_size != 0x1C0000)
    {
	changed_prefs.bogomem_size = 0;
		if (printmsg) msgInfo ("Unsupported slowmem size",4000, NULL);
    }

    if (changed_prefs.chipmem_size > 0x200000 && changed_prefs.fastmem_size != 0) {
		if (printmsg) msgInfo ("Can't use fastmem and more than 2MB chipmem",4000, NULL);
	changed_prefs.fastmem_size = 0;
    }
	
    if (changed_prefs.produce_sound < 0 || changed_prefs.produce_sound > 3) {
		if (printmsg) msgInfo ("Value must be within 0..3",4000, NULL);
	changed_prefs.produce_sound = 0;
    }

    if (changed_prefs.cpu_level < 2 && changed_prefs.z3fastmem_size > 0) {
		if (printmsg) msgInfo ("Z3 fast memory requires 68020 emulation",4000, NULL);
	changed_prefs.z3fastmem_size = 0;
    }
	
    if (changed_prefs.gfxmem_size > 0 && (changed_prefs.cpu_level < 2 || changed_prefs.address_space_24)) {
		if (printmsg) msgInfo ("Picasso96 requires 68020 emulation",4000, NULL);
	changed_prefs.gfxmem_size = 0;
    }
	
#ifndef BSDSOCKET
    if (changed_prefs.socket_emu) {
		if (printmsg) msgInfo ("Compile-time option of BSDSOCKET was not enabled. You can't use bsd-socket emulation",4000, NULL);
	changed_prefs.socket_emu = 0;
    }
#endif

    if (changed_prefs.collision_level < 0 || changed_prefs.collision_level > 3) {
		if (printmsg) msgInfo ("Invalid collision support level. Using Sprite level.",4000, NULL);
	changed_prefs.collision_level = 1;
    }
	
    fixup_prefs_dimensions (&changed_prefs);

#ifdef CPU_68000_ONLY
    changed_prefs.cpu_level = 0;
#endif
#ifndef CPUEMU_0
    changed_prefs.cpu_compatible = 1;
    changed_prefs.address_space_24 = 1;
#endif
#if !defined(CPUEMU_5) && !defined (CPUEMU_6)
    changed_prefs.cpu_compatible = 0;
    changed_prefs.address_space_24 = 0;
#endif
#if !defined (CPUEMU_6)
    changed_prefs.cpu_cycle_exact = changed_prefs.blitter_cycle_exact = 0;
#endif
#ifndef AGA
    changed_prefs.chipset_mask &= ~CSMASK_AGA;
#endif
#ifndef AUTOCONFIG
    changed_prefs.z3fastmem_size = 0;
    changed_prefs.fastmem_size = 0;
    changed_prefs.gfxmem_size = 0;
#endif
#if !defined (BSDSOCKET)
    changed_prefs.socket_emu = 0;
#endif
#if !defined (SCSIEMU)
    changed_prefs.scsi = 0;
#ifdef _WIN32
    changed_prefs.win32_aspi = 0;
#endif
#endif
}

int file_exists (const char *name)
{
    FILE *f;
	f = fopen(name,"rb");
    if (!f)
	return 0;
    fclose (f);
    return 1;
}


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
	changed_prefs.address_space_24 = 1;
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
	changed_prefs.address_space_24 = 1;	
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
	changed_prefs.address_space_24 = 1;	
	changed_prefs.fastmem_size = 0; //OFF
	changed_prefs.chipmem_size = 512 * 1024; //512
	changed_prefs.bogomem_size = 0; //OFF
	changed_prefs.chipset_mask = 0; //OCS

	maybe_load_kick_rom_list(roms);
}

static void A1200_config(void)
{
	const char *roms[] = {"amiga-os-310.rom", "kick31.rom", NULL};
	default_config();

	changed_prefs.cpu_level = 2; //68020
	changed_prefs.address_space_24 = 0;	
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

static void mount_harddisk(void)
{
#ifdef FILESYS

	int submenus[1], opt, access;

	submenus[0] = 0; //RW
	

	opt = menu_select_title("RDB configuration menu",
			RDB_configuration_messages, submenus);
	if (opt < 0)
		return;
	access = submenus[0];

	char *name, *err_msg;
	char *ptr_file_name;
	char dir[255];
	strncpy(dir,prefs_get_attr("hardfile_path"),255);
	name = (char *) menu_select_file(dir, NULL, 0);
	
	if (name != NULL)
	{
		ptr_file_name = strrchr(name,'/');
		if (ptr_file_name) ptr_file_name++; else ptr_file_name = name;
		if (strcmp(ptr_file_name, "None") == 0)
			return;
		
		err_msg= (char *) add_filesys_unit (currprefs.mountinfo, NULL,NULL , name, access, 0, 0, 0, 0, 0,NULL, 0);
			
		if (err_msg) msgInfo(err_msg, 3000, NULL);
		
		free((void*)name);
	}
#endif	
}

void mount_partition(void)
{
#ifdef FILESYS
	int submenus[7], opt; 
	int access, sector, surfaces, reserved, blocksize, priority;
	const char *device;

	submenus[0] = 0; //device 
	submenus[1] = 0; //access
	submenus[2] = 0; //sector	
	submenus[3] = 0; //surfacces
	submenus[4] = 2; //reserved
	submenus[5] = 0; //blocksize
	submenus[6] = 0; //priority
	

	opt = menu_select_title("HD partition configuration menu",
			partition_configuration_messages, submenus);
	if (opt < 0)
		return;
	device = device_table[submenus[0]];
	access = submenus[1];
	sector = sector_table[submenus[2]];
	surfaces = surface_table[submenus[3]];
	reserved = submenus[4];
	blocksize = blocksize_table[submenus[5]];
	priority = boot_priority_table[submenus[6]];

	char *name, *err_msg;
	char *ptr_file_name;
	char dir[255];
	strncpy(dir,prefs_get_attr("hardfile_path"),255);
	name = (char *) menu_select_file(dir, NULL, 0);
	
	if (name != NULL)
	{
		ptr_file_name = strrchr(name,'/');
		if (ptr_file_name) ptr_file_name++; else ptr_file_name = name;
		if (strcmp(ptr_file_name, "None") == 0)
			return;
		
		err_msg= (char *) add_filesys_unit (currprefs.mountinfo, device ,NULL , name, access, sector, surfaces, reserved, blocksize, priority,NULL, 0);
			
		if (err_msg) msgInfo(err_msg, 3000, NULL);
		
		free((void*)name);
	}
#endif
}


static void mount_virtual_file_system(void)
{
#ifdef FILESYS
int submenus[4], opt; 
	int access,  priority;
	const char *device, *volume;

	submenus[0] = 0; //device 
	submenus[1] = 0; //volume
	submenus[2] = 0; //access
	submenus[3] = 5; //priority
	

	opt = menu_select_title("VFS configuration menu",
			VFS_configuration_messages, submenus);
	if (opt < 0)
		return;
	device = device_table[submenus[0]];
	volume = volume_table[submenus[1]];
	access = submenus[2];
	priority = boot_priority_table[submenus[3]];

	char *err_msg;
	err_msg= (char *) add_filesys_unit (currprefs.mountinfo, device,volume , "/uae/harddir", 0, 0, 0, 0, 0, priority, NULL, 0);	
	if (err_msg) msgInfo(err_msg, 3000, NULL);
#endif			
}

static void unmount_device()
{
#ifdef FILESYS

	int dev_to_unmount=menu_select_devices();
	if (dev_to_unmount==-1) return;
	
	write_log("Unmounting device: %d\n",dev_to_unmount );
	if (kill_filesys_unit (currprefs.mountinfo, dev_to_unmount) == -1)
	msgInfo("Volume does not exist", 3000, NULL);	
#endif			
}

extern int make_hdf(unsigned int size, const char *hdf_path, int blocks_per_track, int surfaces, int block_size);

void make_hardfile(void)
{
#ifdef FILESYS
	int submenus[5], opt; 
	int sector, surfaces, blocksize, size;
	const char *device, *hdf_name;

	submenus[0] = 0; //name
	submenus[1] = 0; //size
	submenus[2] = 0; //sector	
	submenus[3] = 0; //surfacces
	submenus[4] = 0; //blocksize
	

	opt = menu_select_title("Make hardfile menu",
			make_hdf_messages, submenus);
	if (opt < 0)
		return;
	
	hdf_name = hdf_name_table[submenus[0]];
	size = hdf_size_table[submenus[1]];
	sector = sector_table[submenus[2]];
	surfaces = surface_table[submenus[3]];
	blocksize = blocksize_table[submenus[4]];
	
	
	const char *dir = prefs_get_attr("hardfile_path");
	char hdf_path[256];

	snprintf(hdf_path, 255, "%s/%s.hdf", dir, hdf_name);
	
	if (file_exists(hdf_path)) 
	{
		if (msgYesNo("Overwrite the existing file?", 0, FULL_DISPLAY_X /2-180/RATIO, FULL_DISPLAY_Y /2-48/RATIO))
		unlink (hdf_path); else return;
	}
	
	msgInfo("Creating file",1,NULL);	
		
	if (!make_hdf((unsigned int) size, hdf_path, sector, surfaces, blocksize)) 
		msgInfo("Hardfile created",2000,NULL);
	else
		msgInfo("Failed to create hardfile",4000,NULL);	
#endif
}

void delete_hardfile(void)
{
	char *name;
	char dir[255];
	strncpy(dir,prefs_get_attr("hardfile_path"),255);
	name = (char *) menu_select_file(dir, NULL, 0);
	if (name && msgYesNo("Are you sure to delete the hardfile?", 0, FULL_DISPLAY_X /2-200/RATIO, FULL_DISPLAY_Y /2-48/RATIO))
		{unlink (name); msgInfo("Hardfile deleted",3000,NULL);}
}

static void cpu_chipset_options(void)
{
	int submenus[3], opt;

	submenus[0] = find_index_by_val(changed_prefs.cpu_level, cpu_level_table,
			SDL_arraysize(cpu_level_table), 0);
	submenus[1] = !changed_prefs.address_space_24;		
	submenus[2] = find_index_by_val(changed_prefs.chipset_mask, chipset_mask_table,
			SDL_arraysize(chipset_mask_table), 0);

	opt = menu_select_title("CPU/Chipset options menu",
			cpu_chipset_messages, submenus);
	if (opt < 0)
		return;
	changed_prefs.cpu_level = cpu_level_table[submenus[0]];
	changed_prefs.address_space_24 = !submenus[1];
	changed_prefs.chipset_mask = chipset_mask_table[submenus[2]];
	
	fix_options_menu_sdl(1);
}

static void memory_options(void)
{
	int submenus[5], opt;

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
	submenus[4] = find_index_by_val(changed_prefs.gfxmem_size, picasso96_size_table,
			SDL_arraysize(picasso96_size_table), 0);		

	opt = menu_select_title("Memory options menu",
			memory_messages, submenus);
	if (opt < 0)
		return;
	/* And update with the new settings */
	changed_prefs.chipmem_size = chipmem_size_table[submenus[0]];
	changed_prefs.bogomem_size = slowmem_size_table[submenus[1]];
	changed_prefs.fastmem_size = fastmem_size_table[submenus[2]];
	changed_prefs.z3fastmem_size = z3fastmem_size_table[submenus[3]];
	changed_prefs.gfxmem_size = picasso96_size_table[submenus[4]];
	
	fix_options_menu_sdl(1);
	
	if (changed_prefs.chipmem_size != currprefs.chipmem_size ||
	changed_prefs.bogomem_size != currprefs.bogomem_size||
	changed_prefs.fastmem_size != currprefs.fastmem_size||
	changed_prefs.z3fastmem_size != currprefs.z3fastmem_size||
	changed_prefs.gfxmem_size != currprefs.gfxmem_size) uae_reset(1);
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

static int get_dfxclick(void);
static void set_dfxclick(int);

static int get_floppy_number(void)
{
	int i;

	for (i=0; (changed_prefs.dfxtype[i]!=-1) && (i<4); i++);

	return i;
	
}

static void set_floppy_number(int which)
{
	int i;
	
	set_dfxclick (get_dfxclick()); //Trick to be sure that all floppies have sound on or off

	for (i=0; i<4; i++)
	{
	if (which == 0) changed_prefs.dfxtype[i] = -1; //disable all floppies
	if (i<which) changed_prefs.dfxtype[i] = 0; //3.5 inches double-density
	else changed_prefs.dfxtype[i] = -1; //disable floppy
	}
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

static void set_gfx_refreshrate(int which)
{
	/* Custom setting - don't touch! */
	if (which > SDL_arraysize(refreshrate_table)-1)
		return;
	changed_prefs.gfx_refreshrate = refreshrate_table[which];
}

static int get_gfx_refreshrate(void)
{
	return find_index_by_val(changed_prefs.gfx_refreshrate, refreshrate_table,
			SDL_arraysize(refreshrate_table), 7);

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
		changed_prefs.cpu_compatible = 0;
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
	if ((changed_prefs.chipset_mask&CSMASK_ECS_DENISE)&&(changed_prefs.chipset_mask&CSMASK_ECS_AGNUS)&&!(changed_prefs.chipset_mask&CSMASK_AGA)&&(changed_prefs.cpu_level == 0)) /* A600 */
		return 2;
	if ((changed_prefs.chipset_mask&CSMASK_ECS_AGNUS)&&!(changed_prefs.chipset_mask&CSMASK_AGA)&&!(changed_prefs.chipset_mask&CSMASK_ECS_DENISE)&&(changed_prefs.cpu_level == 0)) /* A500 */
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
			msgInfo("SD is not mounted",4000,NULL);
		break;
	case PORT_USB:
		if (usbismount) {
			prefs_set_attr ("floppy_path",    strdup(TARGET_USB_PATH));
			changed_prefs.Port = which;
			currprefs.Port = changed_prefs.Port;}
		else
			msgInfo("USB is not mounted",4000,NULL);
		break;
	case PORT_SMB:
		if (smbismount) {
			prefs_set_attr ("floppy_path",    strdup(TARGET_SMB_PATH));
			changed_prefs.Port = which;
			currprefs.Port = changed_prefs.Port;}
		else
			msgInfo("SMB is not mounted",4000,NULL);
		break;
	default:
		break;		
	}	
}

static void save_configurations(const char *ext)
{
char user_options[255] = "";

if (ext==NULL) return;

#ifdef OPTIONS_IN_HOME
char *home = getenv ("HOME");

if (home != NULL && strlen (home) < 240)
{
	strcpy (user_options, home);
	strcat (user_options, "/");
}
#endif

strcat(user_options, OPTIONSFILENAME);
strcat(user_options, ext);

cfgfile_save(&changed_prefs, user_options, 0);
msgInfo("Configurations saved",3000,NULL);
}

static int load_configurations(const char *ext)
{
char user_options[255] = "";
FILE* optionfile; 

if (ext==NULL) return (1);

#ifdef OPTIONS_IN_HOME
char *home = getenv ("HOME");

if (home != NULL && strlen (home) < 240)
{
	strcpy (user_options, home);
	strcat (user_options, "/");
}
#endif

strcat(user_options, OPTIONSFILENAME);
if (strcmp(ext, "default")) strcat(user_options, ext); //not uarec file

optionfile = fopen (user_options, "r");
    
if (!optionfile) {msgInfo("File not found",4000,NULL);return 1;}
	
msgInfo("Configurations loaded",2500,NULL);
default_prefs (&changed_prefs, 0);
cfgfile_load(&changed_prefs, user_options, 0);
fix_options_menu_sdl(0);
currprefs.Port = changed_prefs.Port;
currprefs.leds_on_screen = changed_prefs.leds_on_screen;
currprefs.rumble[0] = changed_prefs.rumble[0];
currprefs.rumble[1] = changed_prefs.rumble[1];

return 0;
}

static void delete_conf_file()
{
char user_options[255] = "";

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

unlink (user_options);

msgInfo("File deleted",3000,NULL);
}


static int get_dfxclick(void)
{
	int sounddf_on = 0;

	sounddf_on = (changed_prefs.dfxclick[0]!=0)||(changed_prefs.dfxclick[1]!=0)||
	(changed_prefs.dfxclick[2]!=0)||(changed_prefs.dfxclick[3]!=0);
	
	return sounddf_on;
}

static void set_dfxclick(int sounddf_on)
{
	int i;
	for (i=0; i < 4; i++)
	    changed_prefs.dfxclick[i] = sounddf_on; //Floppy sounds are all off or all on
}

static void emulation_options(void)
{
	int submenus[8], old_sub_3;
	int opt;
	
	memset(submenus, 0, sizeof(submenus));
	
	submenus[0] = get_emulation_accuracy();
	submenus[1] = get_cpu_to_chipset_speed();
	submenus[2] = get_gfx_framerate();
	submenus[3] = old_sub_3 = get_gfx_refreshrate();
	submenus[4] = changed_prefs.collision_level;
	submenus[5] = !changed_prefs.immediate_blits;
	submenus[6] = !changed_prefs.blitter_cycle_exact;
	submenus[7] = !changed_prefs.gfx_vsync;
	
	opt = menu_select_title("Emulation options menu",
			emulation_messages, submenus);
	if (opt < 0)
		return;
	
	if ((submenus[3] != old_sub_3)&&(submenus[7]))
		{msgInfo("You must set vsync on",3000,0);
		submenus[3]=0;}
		
	
	/* Cycle-exact or not? */
	set_emulation_accuracy(submenus[0]);
	set_cpu_to_chipset_speed(submenus[1]);
	set_gfx_framerate(submenus[2]);
	set_gfx_refreshrate(submenus[3]);
	changed_prefs.collision_level = submenus[4];	
	changed_prefs.immediate_blits = !submenus[5];
	changed_prefs.blitter_cycle_exact = !submenus[6];
	changed_prefs.gfx_vsync = !submenus[7];
	
	fix_options_menu_sdl(1);
}

static void audio_options(void)
{
	
	int submenus[5];
	int opt;
	
	memset(submenus, 0, sizeof(submenus));
	
	submenus[0] = changed_prefs.produce_sound-1;
	submenus[1] = changed_prefs.sound_stereo_separation/2;
	submenus[2] = changed_prefs.sound_mixed_stereo/2;
	submenus[3] = changed_prefs.sound_interpol;
	submenus[4] = !get_dfxclick();
	
	opt = menu_select_title("Audio options menu",
			audio_messages, submenus);
	if (opt < 0)
		return;
	
	changed_prefs.produce_sound = submenus[0]+1;
	changed_prefs.sound_stereo_separation = submenus[1]*2;
	changed_prefs.sound_mixed_stereo = submenus[2]*2;
	changed_prefs.sound_interpol = submenus[3];
	set_dfxclick(!submenus[4]);
	
	fix_options_menu_sdl(1);
}

static void set_gfx_resolution (int res)
{
	if (res) //640X480
	{
		changed_prefs.gfx_width_win = 640;
		changed_prefs.gfx_height_win = 480;
		changed_prefs.gfx_lores = 0;
		changed_prefs.gfx_linedbl = 1;
		currprefs.input_mouse_speed = 100;
	}
	else //320X240
	{
		changed_prefs.gfx_width_win = 320;
		changed_prefs.gfx_height_win = 240;
		changed_prefs.gfx_lores = 1;
		changed_prefs.gfx_linedbl = 0;
		currprefs.input_mouse_speed = 200;
	}
}

extern int screen_is_picasso;

static void other_options(void)
{
	int submenus[7];
	int opt, floppy_n, old_sub_3;
	
	memset(submenus, 0, sizeof(submenus));
	
	floppy_n=get_floppy_number();
	
	submenus[0] = get_floppy_speed();
	submenus[1] = floppy_n;
	submenus[2] = get_gfx_aspect_ratio();
	submenus[3] =  old_sub_3 = (changed_prefs.gfx_width_win == 640) ;
	submenus[4] = !(changed_prefs.gfx_linedbl == 2) ;
	submenus[5] = !changed_prefs.leds_on_screen;
	submenus[6] = changed_prefs.Port;

	opt = menu_select_title("Other options menu",
			other_messages, submenus);
	if (opt < 0)
		return;

	set_floppy_speed(submenus[0]);
	set_floppy_number(submenus[1]);
	set_gfx_aspect_ratio(submenus[2]);
	if (old_sub_3 != submenus[3])
		{
		if (screen_is_picasso) msgInfo("No gfx change with Picasso",3000,NULL);
		else set_gfx_resolution(submenus[3]);
		}
	if (changed_prefs.gfx_width_win == 640) changed_prefs.gfx_linedbl = submenus[4] ? 1 : 2;
	changed_prefs.leds_on_screen = !submenus[5];
	set_Port(submenus[6]);
	
	currprefs.leds_on_screen = changed_prefs.leds_on_screen;
	
	fix_options_menu_sdl(1);
	
	if (floppy_n != submenus[1]) uae_reset(1);
}

static void save_conf_file_menu(void)
{
	int opt;

	opt = menu_select_title("Configuration file save menu",
			save_conf_messages, NULL);
	if (opt < 0)
		return;
	
	switch(opt)
		{
		case 0:
			save_configurations(".saved");
			break;
		case 2:
			save_configurations(".user1");
			break;
		case 4:
			save_configurations(".user2");
			break;
		case 6:
			save_configurations(".user3");
			break;	
		case 8:
			save_configurations(".user4");
			break;
		case 10:
			save_configurations(".user5");
			break;	
		case 12:
			delete_conf_file();
			break;	
		default:
			break;
		}
}

static void load_conf_file_menu(void)
{
	int opt;

	opt = menu_select_title("Configuration file load menu",
			load_conf_messages, NULL);
	if (opt < 0)
		return;
	
	switch(opt)
		{
		case 0:
			if(!load_configurations("default")) uae_reset(1);
			break;
		case 2:
			if(!load_configurations(".user1")) uae_reset(1);
			break;
		case 4:
			if(!load_configurations(".user2")) uae_reset(1);
			break;
		case 6:
			if(!load_configurations(".user3")) uae_reset(1);
			break;	
		case 8:
			if(!load_configurations(".user4")) uae_reset(1);
			break;
		case 10:
			if(!load_configurations(".user5")) uae_reset(1);
			break;	
		default:
			break;
		}
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
	int submenus[7];
	int opt, autofire;
	struct virtkey *virtualkey;

	memset(submenus, 0, sizeof(submenus));
	submenus[3] = !changed_prefs.joystick_settings[1][joy].eventid[ID_AXIS_OFFSET + 6][0];
	submenus[4] = (changed_prefs.mouse_settings[1][joy].enabled == 0);
	submenus[5] = !changed_prefs.rumble[joy];
	submenus[6] = changed_prefs.input_autofire_framecnt/3-1;
	if (submenus[6]<0) submenus[6] = 0;

	opt = menu_select_title("Input menu",
			input_messages, submenus);
	if (opt < 0)
		return;
		
	if (opt == 6) //Mario Kart Wheel
	{	
		if (!submenus[3]){
			if (!joy) insert_keyboard_map("JOY2_HORIZ","input.1.joystick.%d.axis.6", 0);
			else insert_keyboard_map("JOY1_HORIZ" , "input.1.joystick.%d.axis.6", 1);}
		else{
				changed_prefs.joystick_settings[1][joy].eventid[ID_AXIS_OFFSET + 6][0] = 0;
				inputdevice_config_change();
			}
		return;
	}

	if (opt == 8) //Mouse emulation
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
	
	if (opt == 10) //Rumble
	{
		changed_prefs.rumble[joy] = !submenus[5];
		currprefs.rumble[joy] = changed_prefs.rumble[joy];
		return;
	}
	
	if (opt == 12) //Autofire delay
	{
		changed_prefs.input_autofire_framecnt = (submenus[6]+1)*3;
		currprefs.input_autofire_framecnt = changed_prefs.input_autofire_framecnt;
		return;
	}
	
	virtualkey = virtkbd_get_key();
	if (virtualkey == NULL)
		return;
	if (virtualkey->ev_name == NULL)
		return;
	key = virtualkey->ev_name;
	
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
	if (!strcmp(key,"JOY_FIRE_BUTTON"))
			key= joy ? "JOY1_FIRE_BUTTON.0": "JOY2_FIRE_BUTTON.0";
			
	if (!strcmp(key,"JOY_2ND_BUTTON"))
			key= joy ? "JOY1_2ND_BUTTON": "JOY2_2ND_BUTTON";
			
	if (!strcmp(key,"JOY_3RD_BUTTON"))
			key= joy ? "JOY1_3RD_BUTTON": "JOY2_3RD_BUTTON";
	
	if (!strcmp(key,"JOY_LEFT"))
			key= joy ? "JOY1_LEFT": "JOY2_LEFT";
	
	if (!strcmp(key,"JOY_RIGHT"))
			key= joy ? "JOY1_RIGHT": "JOY2_RIGHT";
			
	if (!strcmp(key,"JOY_UP"))
			key= joy ? "JOY1_UP": "JOY2_UP";
			
	if (!strcmp(key,"JOY_DOWN"))
			key= joy ? "JOY1_DOWN": "JOY2_DOWN";
			
	if (!strcmp(key,"JOY_AUTOFIRE_BUTTON"))
	{
			key= joy ? "JOY1_FIRE_BUTTON.1": "JOY2_FIRE_BUTTON.1";		
	}		
		
	setup_joystick(joy, key, sdl_key);
	
}

static void virtual_keyboard(void)
{
	int key_code;
	
	virtkey_t *key =virtkbd_get_key();  
	if (key) {key_code = key->sdl_code;} else return;
	if (!key_code) return;
	
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
	} while (opt == 6);

	fix_options_menu_sdl(0);

	/* Reset the Amiga if the model or ROM has changed */
	if ((cur_model != submenus[0])||(strcmp (currprefs.romfile, changed_prefs.romfile)))
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
/*
static void help(void)
{
	menu_select_title("UAE-wii help",
			help_messages, NULL);
}
*/


void hd_emulation()
{
int opt;

	opt = menu_select_title("HD emulation menu",
			hd_emulation_messages, NULL);
	if (opt < 0)
		return;
	
	switch(opt)
		{
		case 0:
			mount_virtual_file_system();
			break;
		case 2:
			mount_harddisk();
			break;
		case 4:
			mount_partition();
			break;
		case 6:
			unmount_device();
			break;
		case 8:
			make_hardfile();
			break;
		case 10:
			delete_hardfile();
			break;	
		default:
			break;
		}
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

int gui_update (void)
{
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
	int exit = 0;
	audio_pause();
	
	memset(submenus, 0, sizeof(submenus));

	if (shortcut==-1) {do //Enter Menu
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
			hardware_options();
			break;	
		case 8:
			emulation_options();
			break;
		case 9:
			audio_options();
			break;	
		case 10:
			other_options();
			break;
		case 11:
			hd_emulation();
			break;
		case 12:	
			save_conf_file_menu();
			break;
		case 13:	
			load_conf_file_menu();
			break;	
		case 14:
			uae_reset(1);
			break;	
		case 15:
			if (msgYesNo("Are you sure to quit?", 0, FULL_DISPLAY_X /2-138/RATIO, FULL_DISPLAY_Y /2-48/RATIO)) 
				{currprefs.rumble[0]=0; currprefs.rumble[1]=0;uae_quit(); exit = 1;}
			break;
		default:
			break;
		}
	} while ((opt == 0 || opt == 5 || opt == 8 || opt == 9 || opt == 10 || opt == 11 || opt == 15)&&!exit);
	if (!exit) flip_screen();}
	
	if (shortcut==6) {virtual_keyboard(); notice_screen_contents_lost ();}//Enter Virtual Keyboard
	
	audio_resume();
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
