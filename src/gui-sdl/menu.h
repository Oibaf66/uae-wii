/*********************************************************************
 *
 * Copyright (C) 2004, 2008,  Simon Kagstrom
 * Copyright (C) 2010,2014,  Fabio Olimpieri
 *
 * Filename:      menu.h
 * Author:        Simon Kagstrom <simon.kagstrom@gmail.com>, Fabio Olimpieri
 * Description:
 *
 * $Id$
 *
 ********************************************************************/
#ifndef __MENU_H__
#define __MENU_H__

#include <SDL.h>
#include <SDL_ttf.h>
#include<SDL/SDL_image.h>
#include <stdint.h>

#if defined(__cplusplus)
extern "C" {
#endif

#define KEY_UP         1
#define KEY_DOWN       2
#define KEY_LEFT       4
#define KEY_RIGHT      8
#define KEY_SELECT    16
#define KEY_ESCAPE    32
#define KEY_PAGEDOWN  64
#define KEY_PAGEUP   128
#define KEY_HELP     256
#define  MAX_DEVICE_ITEM 32

extern int FULL_DISPLAY_X; //640
extern int FULL_DISPLAY_Y; //480
extern int RATIO;


void menu_print_font(SDL_Surface *screen, int r, int g, int b, int x, int y, const char *msg, int font_size);

/* Various option selects */
int menu_select_title(const char *title, const char **pp_msgs, int *p_submenus);
int menu_select(const char **pp_msgs, int *p_submenus);
const char *menu_select_file(const char *dir_path,const char *selected_file, int which);
const char *menu_select_file_start(const char *dir_path, const char **d64_name);

uint32_t menu_wait_key_press(void);

void msgKill(SDL_Rect *rc);
int msgInfo(char *text, int duration, SDL_Rect *rc);

int msgYesNo(char *text, int def,int x, int y);

void menu_init(SDL_Surface *screen);

void menu_deinit(void);

int menu_is_inited(void);

int ext_matches(const char *name, const char *ext);

void flip_screen (void);

int menu_select_devices(void);

void play_click(int sound);

#if defined(__cplusplus)
}
#endif

#endif /* !__MENU_H__ */
