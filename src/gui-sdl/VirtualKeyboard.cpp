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


class VirtualKeyboard
{
public:
	VirtualKeyboard(SDL_Surface *screen, TTF_Font *font);
	struct virtkey* get_key();

private:
	struct virtkey* get_key_internal();
	void draw();
	void select_next(int dx, int dy);
	void toggle_shift();

	SDL_Surface *screen;
	TTF_Font *font;
	int sel_x;
	int sel_y;

	char buf[255];
};



#define K(name, sdl_code) \
  { name, "KEY_"name, sdl_code, false }
#define N(name, key_name, sdl_code) \
  { name, "KEY_"key_name, sdl_code, false }
#define D(name) \
  { name, "None", 0, true }
#define KNL() \
  { NULL, NULL, 0, false }
#define NJ(name, joy_name) \
  { name, joy_name, 0, false }

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

VirtualKeyboard::VirtualKeyboard(SDL_Surface *screen, TTF_Font *font)
{
	this->screen = screen;
	this->font = font;
	this->sel_x = 0;
	this->sel_y = 0;

	memset(this->buf, 0, sizeof(this->buf));
}

void VirtualKeyboard::draw()
{
	int screen_w = this->screen->w;
	int screen_h = this->screen->h;
	int key_w = 36;
	int key_h = 36;
	int border_x = (screen_w - (key_w * KEY_COLS)) / 2;
	int border_y = (screen_h - (key_h * KEY_ROWS)) / 2;
	SDL_Rect bg_rect = {border_x, border_y,
			key_w * KEY_COLS, key_h * KEY_ROWS};

	SDL_FillRect(this->screen, &bg_rect,
			SDL_MapRGB(screen->format, 0xff, 0xff, 0xff));

	for (int y = 0; y < KEY_ROWS; y++ )
	{
		for (int x = 0; x < KEY_COLS; x++ )
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
			if ( (x == this->sel_x && y == this->sel_y))
				g = 200;

			menu_print_font(this->screen, r, g, b,
					x * key_w + border_x, y * key_h + border_y,
					what, 20);
		}
	}
}

void VirtualKeyboard::select_next(int dx, int dy)
{
	int next_x = (this->sel_x + dx) % KEY_COLS;
	int next_y = (this->sel_y + dy) % KEY_ROWS;
	virtkey_t key;

	if (next_x < 0)
		next_x = KEY_COLS + next_x;
	if (next_y < 0)
		next_y = KEY_ROWS + next_y;
	this->sel_x = next_x;
	this->sel_y = next_y;

	key = keys[ next_y * KEY_COLS + next_x ];

	/* Skip the empty spots */
	if (key.name == NULL)
	{
		if (dy != 0) /* Look left */
			this->select_next(-1, 0);
		else
			this->select_next(dx, dy);
	}
}

struct virtkey *VirtualKeyboard::get_key_internal()
{
	while(1)
	{
		uint32_t k;

		this->draw();
		SDL_Flip(this->screen);

		k = menu_wait_key_press();

		if (k & KEY_UP)
			this->select_next(0, -1);
		else if (k & KEY_DOWN)
			this->select_next(0, 1);
		else if (k & KEY_LEFT)
			this->select_next(-1, 0);
		else if (k & KEY_RIGHT)
			this->select_next(1, 0);
		else if (k & KEY_ESCAPE)
			return NULL;
		else if (k & KEY_SELECT)
		{
			virtkey_t *key = &keys[ this->sel_y * KEY_COLS + this->sel_x ];

			return key;
		}
	}

	return NULL;
}

struct virtkey* VirtualKeyboard::get_key()
{
	virtkey_t *key;
	SDL_Rect rect = {32, 32, FULL_DISPLAY_X-64, FULL_DISPLAY_Y-96};

	SDL_FillRect(this->screen, &rect, SDL_MapRGB(screen->format, 0xff, 0xff, 0xff));
	
	key = this->get_key_internal();

	return key;
}

/* C interface */
static VirtualKeyboard *virtual_keyboard;
void virtkbd_init(SDL_Surface *surf, TTF_Font *fnt)
{
	virtual_keyboard = new VirtualKeyboard(surf, fnt);
}

struct virtkey *virtkbd_get_key(void)
{
	return virtual_keyboard->get_key();
}
