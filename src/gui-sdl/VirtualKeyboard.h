/*********************************************************************
 *
 * Copyright (C) 2009,  Simon Kagstrom
 * Copyright (C) 2014,  Fabio Olimpieri
 *
 * Filename:      VirtualKeyboard.c
 * Author:        Simon Kagstrom <simon.kagstrom@gmail.com>, Fabio Olimpieri
 * Description:   A virtual keyboard
 *
 * $Id$
 *
 ********************************************************************/
#include <SDL.h>
#include <SDL_ttf.h>

typedef struct virtkey
{
	const char *name;
	const char *ev_name;
	int sdl_code;
	int is_done;
} virtkey_t;

typedef struct Virtual_Keyboard
{
	SDL_Surface *screen;
	int x;
	int y;
	char buf[255];
	
} VirtualKeyboard_struct;

void VirtualKeyboard_init(SDL_Surface *surf);
void VirtualKeyboard_fini(void);
extern struct virtkey *virtkbd_get_key(void);
void flip_VKB();
extern int kbd_is_active;