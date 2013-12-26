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

#include "menu.h"
#include "VirtualKeyboard.h"


static	struct virtkey* get_key_internal();
static	void draw();
static	void select_next(int dx, int dy);

static	SDL_Surface *screen;
static	TTF_Font *font;
static	int sel_x;
static	int sel_y;

static	char buf[255];


#define K(name, sdl_code) \
  { name, "KEY_"name, sdl_code, 0 }
#define N(name, key_name, sdl_code) \
  { name, "KEY_"key_name, sdl_code, 0 }
#define D(name) \
  { name, "None", 0, 1 }
#define KNL() \
  { NULL, NULL, 0, 0 }
#define NJ(name, joy_name) \
  { name, joy_name, 0, 0 }

#define KEY_COLS 14
#define KEY_ROWS 8

static virtkey_t keys[KEY_COLS * KEY_ROWS] = {
  N("Esc","ESC",27), KNL(), K("F1",282),K("F2",283),K("F3",284),K("F4",285),K("F5",286),K("F6",287),K("F7",288),K("F8",289),K("F9",290),K("F10",291), N("BS","BACKSPACE",8),N("Help", "HELP",277),
  N("~`","BACKQUOTE",96),KNL(),K("1",49),K("2",50), K("3",51), K("4",52), K("5",53), K("6",54), K("7",55), K("8",56), K("9",57), K("0",48), N("-","SUB",45),N("=","EQUALS",61),
  N("Tab", "TAB", 9), KNL(), K("Q",113), K("W",119), K("E",101), K("R",114), K("T",116), K("Y",121), K("U",117), K("I",105), K("O",111), K("P",112),N("[","LEFTBRACKET",91),N("]","RIGHTBRACKET",93),
  N("Ctrl","CTRL",306),KNL(), K("A",97), K("S",115), K("D",100), K("F",102), K("G",103), K("H",104), K("J",106), K("K",107), K("L",108),N(":;","SEMICOLON",59),N("'","SINGLEQUOTE",96),N("Ret","RETURN",13),
  N("Sft","SHIFT_LEFT",304),KNL(),K("Z",122),K("X",120),K("C",99), K("V",118), K("B",98), K("N",110), K("M",109),N("<,","COMMA",44),N(">.","PERIOD",46), N("/","SLASH",47),N("\\","KEY_BACKSLASH",92), N("Sft","SHIFT_RIGHT",303),
  N("Alt","ALT_LEFT",308),KNL(), N("Amg","AMIGA_LEFT",310),KNL(),N("space", "SPACE",32),KNL(),KNL(),KNL(),N("Up","CURSOR_UP",273),KNL(),KNL(),N("Amg","AMIGA_RIGHT",309),KNL(),N("Alt","ALT_RIGHT",307), 
  D("None"), KNL(), KNL(), KNL(), KNL(), KNL(), N("Lft","CURSOR_LEFT",276),KNL(), N("Dwn","CURSOR_DOWN",274), KNL(), N("Rgt", "CURSOR_RIGHT",275),KNL(), N("Enter","ENTER",271),KNL(),
  NJ("Fire","JOY_FIRE_BUTTON"),KNL(),KNL(),NJ("Joy 2nd button","JOY_2ND_BUTTON"),KNL(),KNL(),KNL(),KNL(),KNL(),NJ("Joy 3rd button","JOY_3RD_BUTTON"),KNL(),KNL(),KNL(),KNL()
};


void draw()
{
	int screen_w = screen->w;
	int screen_h = screen->h;
	int key_w = 36;
	int key_h = 36;
	int border_x = (screen_w - (key_w * KEY_COLS)) / 2;
	int border_y = (screen_h - (key_h * KEY_ROWS)) / 2;
	int y;
	int x;
	
	SDL_Rect bg_rect = {border_x, border_y,
			key_w * KEY_COLS, key_h * KEY_ROWS};

	SDL_FillRect(screen, &bg_rect,
			SDL_MapRGB(screen->format, 0xff, 0xff, 0xff));

	for (y = 0; y < KEY_ROWS; y++ )
	{
		for (x = 0; x < KEY_COLS; x++ )
		{
			int which = y * KEY_COLS + x;
			virtkey_t key = keys[which];
			int r = 64, g = 64, b = 64;
			const char *what = key.name;

			/* Skip empty positions */
			if (key.name == NULL)
				continue;

			if ( key.is_done )
				r = 255;
			if ( (x == sel_x && y == sel_y))
				g = 200;

			menu_print_font(screen, r, g, b,
					x * key_w + border_x, y * key_h + border_y,
					what, 20);
		}
	}
}

void select_next(int dx, int dy)
{
	int next_x = (sel_x + dx) % KEY_COLS;
	int next_y = (sel_y + dy) % KEY_ROWS;
	virtkey_t key;

	if (next_x < 0)
		next_x = KEY_COLS + next_x;
	if (next_y < 0)
		next_y = KEY_ROWS + next_y;
	sel_x = next_x;
	sel_y = next_y;

	key = keys[ next_y * KEY_COLS + next_x ];

	/* Skip the empty spots */
	if (key.name == NULL)
	{
		if (dy != 0) /* Look left */
			select_next(-1, 0);
		else
			select_next(dx, dy);
	}
}

struct virtkey *get_key_internal()
{
	while(1)
	{
		uint32_t k;

		draw();
		SDL_Flip(screen);

		k = menu_wait_key_press();

		if (k & KEY_UP)
			select_next(0, -1);
		else if (k & KEY_DOWN)
			select_next(0, 1);
		else if (k & KEY_LEFT)
			select_next(-1, 0);
		else if (k & KEY_RIGHT)
			select_next(1, 0);
		else if (k & KEY_ESCAPE)
			return NULL;
		else if (k & KEY_SELECT)
		{
			virtkey_t *key = &keys[ sel_y * KEY_COLS + sel_x ];

			return key;
		}
	}

	return NULL;
}

struct virtkey* virtkbd_get_key(void)
{
	virtkey_t *key;
	SDL_Rect rect = {56, 80, FULL_DISPLAY_X-104, FULL_DISPLAY_Y-176};

	SDL_FillRect(screen, &rect, SDL_MapRGB(screen->format, 0xff, 0xff, 0xff));
	
	key = get_key_internal();

	return key;
}


void virtkbd_init(SDL_Surface *surf, TTF_Font *fnt)
{
	sel_x = 0;
	sel_y = 0;
	screen = surf;
	font = fnt;
	memset(buf, 0, sizeof(buf));
}

