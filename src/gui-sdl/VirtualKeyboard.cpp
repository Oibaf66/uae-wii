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


struct virtkey;

class VirtualKeyboard
{
public:
	VirtualKeyboard(SDL_Surface *screen, TTF_Font *font);
	const char* get_key();

private:
	struct virtkey *get_key_internal();
	void draw();
	void select_next(int dx, int dy);
	void toggle_shift();

	SDL_Surface *screen;
	TTF_Font *font;
	int sel_x;
	int sel_y;

	char buf[255];
};

typedef struct virtkey
{
	const char *name;
	const char *ev_name;
	bool is_done;
} virtkey_t;

#define K(name) \
  { name, "KEY_"name, false }
#define N(name, key_name) \
  { name, "KEY_"key_name, false }
#define D(name) \
  { name, "None", true }
#define KNL() \
  { NULL, NULL, false }

#define KEY_COLS 14
#define KEY_ROWS 7

static virtkey_t keys[KEY_COLS * KEY_ROWS] = {
  N("Esc", "ESC"), KNL(), K("F1"),K("F2"),K("F3"),K("F4"),K("F5"),K("F6"),K("F7"),K("F8"),K("F9"),K("F10"), N("Del","DEL"),N("Help", "HELP"),
  N("~`","BACKQUOTE"),KNL(),K("1"), K("2"), K("3"), K("4"), K("5"), K("6"), K("7"), K("8"), K("9"), K("0"), N("-", "SUB"),N("+", "PLUS"),
  N("Tab", "TAB"), KNL(), K("Q"), K("W"), K("E"), K("R"), K("T"), K("Y"), K("U"), K("I"), K("O"), K("P"),N("[", "LEFTBRACKET"),N("]","RIGHTBRACKET"),
  N("Sft","SHIFT_LEFT"),KNL(), K("A"), K("S"), K("D"), K("F"), K("G"), K("H"), K("J"), K("K"), K("L"), N(":;", "SEMICOLON"), N("@#", "??"), N("Sft", "SHIFT_RIGHT"),
  N("Ctrl","CTRL"),KNL(),K("Z"),K("X"), K("C"), K("V"), K("B"), K("N"), K("M"),N("<,", "COMMA"),N(">.", "PERIOD"), N("\\","KEY_BACKSLASH"), N("/", "SLASH"),N("Ret", "RETURN"),
  N("Alt","ALT_LEFT"),KNL(), N("Amg","AMIGA_LEFT"),KNL(),N("space", "SPACE"),KNL(),KNL(),KNL(), N("Up", "CURSOR_UP"),KNL(),KNL(),N("Amg","AMIGA_RIGHT"),KNL(),N("Alt","ALT_RIGHT"), 
  D("None"), KNL(), KNL(), KNL(), KNL(), KNL(), N("Lft", "CURSOR_LEFT"),KNL(), N("Dwn", "CURSOR_DOWN"), KNL(), N("Rgt", "CURSOR_RIGHT"),KNL(), KNL(),
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
			SDL_MapRGB(screen->format, 0x00, 0x80, 0x80));

	for (int y = 0; y < KEY_ROWS; y++ )
	{
		for (int x = 0; x < KEY_COLS; x++ )
		{
			int which = y * KEY_COLS + x;
			virtkey_t key = keys[which];
			int r = 255, g = 255, b = 255;
			const char *what = key.name;

			/* Skip empty positions */
			if (key.name == NULL)
				continue;

			if ( key.is_done )
				r = 0;
			if ( (x == this->sel_x && y == this->sel_y))
				b = 0;

			menu_print_font(this->screen, r, g, b,
					x * key_w + border_x, y * key_h + border_y,
					what);
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

const char* VirtualKeyboard::get_key()
{
	virtkey_t *key;

	SDL_FillRect(this->screen, 0, SDL_MapRGB(screen->format, 0x00, 0x80, 0x80));

	key = this->get_key_internal();
	if (key == NULL)
		return NULL;

	return key->ev_name;
}

/* C interface */
static VirtualKeyboard *virtual_keyboard;
void virtkbd_init(SDL_Surface *surf, TTF_Font *fnt)
{
	virtual_keyboard = new VirtualKeyboard(surf, fnt);
}

const char *virtkbd_get_key(void)
{
	return virtual_keyboard->get_key();
}
