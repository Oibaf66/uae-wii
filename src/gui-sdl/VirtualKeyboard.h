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

#if defined(__cplusplus)
extern "C" {
#endif

extern void virtkbd_init(SDL_Surface *surf, TTF_Font *fnt);
extern const char *virtkbd_get_key(void);

#if defined(__cplusplus)
};
#endif
