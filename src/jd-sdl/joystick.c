 /*
  * UAE - The Un*x Amiga Emulator
  *
  * SDL Joystick code
  *
  * Copyright 1997 Bernd Schmidt
  * Copyright 1998 Krister Walfridsson
  * Copyright 2003-2005 Richard Drummond
  */

#include "sysconfig.h"
#include "sysdeps.h"

#include "options.h"
#include "inputdevice.h"
#include <SDL.h>

#if defined(GEKKO)
# include <ogc/system.h>
# include <wiiuse/wpad.h> 
#endif

extern int gui_is_active;

unsigned int nr_joysticks;
static int initialized;

struct joyinfo {
    SDL_Joystick *joy;
    unsigned int axles;
    unsigned int buttons;
};

struct joyinfo joys[MAX_INPUT_DEVICES];

//Wiimote Rumble
#ifdef GEKKO
void Rumble(unsigned int nr, unsigned int i, int bs)
{
		static Uint32 last_ticks[2];
		Uint32 cur_ticks;
		static bool rumble_on[2];
		static bool fire_pressed[2];
		static int joystickbutton_fire[2]={-1,-1,-1,-1,-1,-1};
		int kc;
		
		if (nr>1) return;
		
		cur_ticks = SDL_GetTicks();
		
		kc = currprefs.joystick_settings[1][nr].eventid[i][0];
		
			if (bs && (kc == INPUTEVENT_JOY2_FIRE_BUTTON || kc == INPUTEVENT_JOY1_FIRE_BUTTON) && !rumble_on[nr] && !fire_pressed[nr])  
			{			
				WPAD_Rumble(nr, true);
				last_ticks[nr]= cur_ticks;
				rumble_on[nr]=true;	
				fire_pressed[nr]=true;
				joystickbutton_fire[nr]=i;
			}
		
			if (joystickbutton_fire[nr] == i)
			{
				if (!bs && (kc == INPUTEVENT_JOY2_FIRE_BUTTON || kc == INPUTEVENT_JOY1_FIRE_BUTTON) && rumble_on[nr] && fire_pressed[nr])
				{			
					rumble_on[nr]=true;	
					fire_pressed[nr]=false;
				}
			
				if (((cur_ticks - last_ticks[nr] > 120) && rumble_on[nr] && !fire_pressed[nr]) \
				||(!bs && (kc == INPUTEVENT_JOY2_FIRE_BUTTON || kc == INPUTEVENT_JOY1_FIRE_BUTTON) && !rumble_on[nr] && fire_pressed[nr]))
				{
					WPAD_Rumble(nr, false);
					rumble_on[nr]=false;
					fire_pressed[nr]=false;
					joystickbutton_fire[nr]=-1;
				}
				if ((cur_ticks - last_ticks[nr] > 120) && rumble_on[nr] && fire_pressed[nr])
				{
					WPAD_Rumble(nr, false);
					rumble_on[nr]=false;
					fire_pressed[nr]=true;	
				}
			}		
}		
#endif

static void read_joy (unsigned int nr)
{
    unsigned int num, i, axes, axis, hats;
    SDL_Joystick *joy;

    if (currprefs.input_selected_setting == 0) {
	if (jsem_isjoy (0, &currprefs) != (int)nr && jsem_isjoy (1, &currprefs) != (int)nr)
	    return;
    }
    joy = joys[nr].joy;
    axes = SDL_JoystickNumAxes (joy);
    for (i = 0; i < axes; i++) {
	axis = SDL_JoystickGetAxis (joy, i);
	setjoystickstate (nr, i, axis, 32767);
    }

    /* Handle hat - used e.g., for the Wii */
    hats = SDL_JoystickNumHats (joy); 
    for (i = 0; i < hats; i++) {
    	Uint8 v = SDL_JoystickGetHat (joy, i);
    	int x = 0, y = 0;

    	if (v & SDL_HAT_UP)
    		y = -1;
    	if (v & SDL_HAT_DOWN)
    		y = 1;
    	if (v & SDL_HAT_LEFT)
    		x = -1;
    	if (v & SDL_HAT_RIGHT)
    		x = 1;

        setjoystickstate (nr, axes + i * 2, x, 1);
        setjoystickstate (nr, axes + i * 2 + 1, y, 1);
    }

    num = SDL_JoystickNumButtons (joy);
    for (i = 0; i < num; i++) {
	int bs = SDL_JoystickGetButton (joy, i) ? 1 : 0;
	setjoybuttonstate (nr, i, bs);
	#ifdef GEKKO
	if ((nr==0) && (currprefs.rumble[0])) Rumble (0,i, bs);
	if ((nr==1) && (currprefs.rumble[1])) Rumble (1,i, bs);	
	#endif
    }
}

static unsigned int get_joystick_num (void)
{
    return nr_joysticks;
}

static unsigned int get_joystick_widget_num (unsigned int joy)
{
    return joys[joy].axles + joys[joy].buttons;
}

static int get_joystick_widget_type (unsigned int joy, unsigned int num, char *name, uae_u32 *code)
{
    if (num >= joys[joy].axles && num < joys[joy].axles + joys[joy].buttons) {
	if (name)
	    sprintf (name, "Button %d", num + 1 - joys[joy].axles);
	return IDEV_WIDGET_BUTTON;
    } else if (num < joys[joy].axles) {
	if (name)
	    sprintf (name, "Axis %d", num + 1);
	return IDEV_WIDGET_AXIS;
    }
    return IDEV_WIDGET_NONE;
}

static int get_joystick_widget_first (unsigned int joy, int type)
{
    switch (type) {
	case IDEV_WIDGET_BUTTON:
	    return joys[joy].axles;
	case IDEV_WIDGET_AXIS:
	    return 0;
    }
    return -1;
}

static const char *get_joystick_name (unsigned int joy)
{
    return SDL_JoystickName (joy);
}

static void read_joysticks (void)
{
    if (get_joystick_num ()) {
	unsigned int i;
	//IR wiimote fix
	#ifndef GEKKO
	SDL_JoystickUpdate ();
	#endif
	for (i = 0; i < get_joystick_num (); i++)
	    read_joy (i);
    }
}

static int init_joysticks (void)
{
    int success = 0;

    if (!initialized) {
	if (SDL_InitSubSystem (SDL_INIT_JOYSTICK) == 0) {
	    unsigned int i;

	    nr_joysticks = SDL_NumJoysticks ();
	    write_log ("Found %d joystick(s)\n", nr_joysticks);

	    if (nr_joysticks > MAX_INPUT_DEVICES)
		nr_joysticks = MAX_INPUT_DEVICES;

	    for (i = 0; i < get_joystick_num (); i++) {
		joys[i].joy     = SDL_JoystickOpen (i);
		joys[i].axles   = SDL_JoystickNumAxes (joys[i].joy);
		joys[i].buttons = SDL_JoystickNumButtons (joys[i].joy);
	    }
		//IR wiimote fix 
		#ifdef GEKKO
		SDL_JoystickEventState(SDL_ENABLE);
		#endif
	    success = initialized = 1;
	} else
	    write_log ("Failed to initialize joysticks\n");
    }

    return success;
}

static void close_joysticks (void)
{
    unsigned int i;
    for (i = 0; i < nr_joysticks; i++) {
	SDL_JoystickClose (joys[i].joy);
	joys[i].joy = 0;
    }
    nr_joysticks = 0;

    if (initialized) {
	SDL_QuitSubSystem (SDL_INIT_JOYSTICK);
	initialized = 0;
    }
}

static int acquire_joy (unsigned int num, int flags)
{
    return num < get_joystick_num ();
}

static void unacquire_joy (unsigned int num)
{
}

struct inputdevice_functions inputdevicefunc_joystick = {
    init_joysticks,
    close_joysticks,
    acquire_joy,
    unacquire_joy,
    read_joysticks,
    get_joystick_num,
    get_joystick_name,
    get_joystick_widget_num,
    get_joystick_widget_type,
    get_joystick_widget_first
};

/*
 * Set default inputdevice config for SDL joysticks
 */
void input_get_default_joystick (struct uae_input_device *uid)
{
    unsigned int i, port;

    for (i = 0; i < nr_joysticks; i++) {
        port = i & 1;
        uid[i].eventid[ID_AXIS_OFFSET + 0][0]   = port ? INPUTEVENT_JOY2_HORIZ : INPUTEVENT_JOY1_HORIZ;
        uid[i].eventid[ID_AXIS_OFFSET + 1][0]   = port ? INPUTEVENT_JOY2_VERT  : INPUTEVENT_JOY1_VERT;
        uid[i].eventid[ID_BUTTON_OFFSET + 0][0] = port ? INPUTEVENT_JOY2_FIRE_BUTTON : INPUTEVENT_JOY1_FIRE_BUTTON;
        uid[i].eventid[ID_BUTTON_OFFSET + 1][0] = port ? INPUTEVENT_JOY2_2ND_BUTTON  : INPUTEVENT_JOY1_2ND_BUTTON;
        uid[i].eventid[ID_BUTTON_OFFSET + 2][0] = port ? INPUTEVENT_JOY2_3RD_BUTTON  : INPUTEVENT_JOY1_3RD_BUTTON;
    }
    uid[0].enabled = 1;
}
