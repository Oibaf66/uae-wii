/*********************************************************************
 *
 * Copyright (C) 2009,  Simon Kagstrom
 *
 * Filename:      VirtualKeyboard.c
 * Author:        Simon Kagstrom <simon.kagstrom@gmail.com>
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

#if defined(__cplusplus)
extern "C" {
#endif

extern void virtkbd_init(SDL_Surface *surf, TTF_Font *fnt);
extern struct virtkey *virtkbd_get_key(void);

#if defined(__cplusplus)
};
#endif
