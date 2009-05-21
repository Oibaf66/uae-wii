/*********************************************************************
 *
 * Copyright (C) 2004, 2008,  Simon Kagstrom
 *
 * Filename:      menu.h
 * Author:        Simon Kagstrom <simon.kagstrom@gmail.com>
 * Description:
 *
 * $Id$
 *
 ********************************************************************/
#ifndef __MENU_H__
#define __MENU_H__

#include <SDL.h>
#include <SDL_ttf.h>
#include <stdint.h>

#define KEY_UP         1
#define KEY_DOWN       2
#define KEY_LEFT       4
#define KEY_RIGHT      8
#define KEY_SELECT    16
#define KEY_ESCAPE    32
#define KEY_PAGEDOWN  64
#define KEY_PAGEUP   128
#define KEY_HELP     256

void menu_print_font(SDL_Surface *screen, int r, int g, int b, int x, int y, const char *msg);
void menu_print_font64(SDL_Surface *screen, int r, int g, int b, int x, int y, const char *msg);

/* Various option selects */
int menu_select_title(const char *title, const char **pp_msgs, int *p_submenus);
int menu_select(const char **pp_msgs, int *p_submenus);
const char *menu_select_file(const char *dir_path);
const char *menu_select_file_start(const char *dir_path, const char **d64_name);

uint32_t menu_wait_key_press(void);

extern int msgKill(SDL_Rect *rc);
extern int msgInfo(char *text, int duration, SDL_Rect *rc);

extern int msgYesNo(char *text, int def,int x, int y);

void menu_init(SDL_Surface *screen);

int menu_is_inited(void);

#endif /* !__MENU_H__ */
