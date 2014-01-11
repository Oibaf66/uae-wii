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
#include<SDL/SDL_image.h>


#ifdef GEKKO
#include <wiiuse/wpad.h>
#endif

#include "menu.h"
#include "VirtualKeyboard.h"
#include "writelog.h"
#include "target.h"
#include "gensound.h"

static SDL_Surface *image_kbd, *tmp_surface ;
static int vkb_is_init;
//static int key_code;

static VirtualKeyboard_struct VirtualKeyboard;

int kbd_is_active;


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

#define KEY_COLS 21
#define KEY_ROWS 7

static virtkey_t keys[KEY_COLS * KEY_ROWS] = {
  N("Esc","ESC",27), K("F1",282),K("F2",283),K("F3",284),K("F4",285),K("F5",286),K("F6",287),K("F7",288),K("F8",289),K("F9",290),K("F10",291), KNL(),KNL(),KNL(),KNL(),KNL(),KNL(),KNL(),KNL(),KNL(),KNL(),
  N("~`","BACKQUOTE",96),K("1",49),K("2",50), K("3",51), K("4",52), K("5",53), K("6",54), K("7",55), K("8",56), K("9",57), K("0",48), N("-","SUB",45),N("=","EQUALS",61), N("\\","KEY_BACKSLASH",92),N("BS","BACKSPACE",8),N("Del", "DELETE",127),N("Help", "HELP",277),K("LEFTPAREN",278), K("RIGHTPAREN",279),K("KP_DIVIDE",267),K("KP_MULTIPLY",268),
  N("Tab", "TAB", 9), K("Q",113), K("W",119), K("E",101), K("R",114), K("T",116), K("Y",121), K("U",117), K("I",105), K("O",111), K("P",112),N("[","LEFTBRACKET",91),N("]","RIGHTBRACKET",93),N("Ret","RETURN",13),KNL(), K("KP7",263), K("KP8",264), K("KP9",265), K("KP_MINUS",269),KNL(),KNL(),
  N("Ctrl","CTRL",306),N("Capslock","CAPSLOCK",301), K("A",97), K("S",115), K("D",100), K("F",102), K("G",103), K("H",104), K("J",106), K("K",107), K("L",108),N(";","SEMICOLON",59),N("'","SINGLEQUOTE",96),N("Ret","RETURN",13),KNL(),N("Up","CURSOR_UP",273),KNL(),K("KP4",260), K("KP5",261), K("KP6",262),K("KP_PLUS",270),
  N("Sft","SHIFT_LEFT",304),K("Z",122),K("X",120),K("C",99), K("V",118), K("B",98), K("N",110), K("M",109),N(",","COMMA",44),N(".","PERIOD",46), N("/","SLASH",47), N("Sft","SHIFT_RIGHT",303), N("Lft","CURSOR_LEFT",276), N("Dwn","CURSOR_DOWN",274),  N("Rgt", "CURSOR_RIGHT",275),K("KP1",257),K("KP2",258), K("KP3",259), N("Enter","KP_ENTERENTER",271),KNL(),KNL(),
  N("Alt","ALT_LEFT",308), N("Amg","AMIGA_LEFT",310),N("space", "SPACE",32),N("Amg","AMIGA_RIGHT",309),N("Alt","ALT_RIGHT",307),KNL(),K("KP0",256),N(".","KP_PERIOD",266),N("Enter","KP_ENTER",271),KNL(),KNL(),KNL(),KNL(),KNL(),KNL(),KNL(),KNL(),KNL(),KNL(),KNL(),KNL(),
 
  NJ("Fire","JOY_FIRE_BUTTON"),NJ("Joy 2nd button","JOY_2ND_BUTTON"),NJ("Joy 3rd button","JOY_3RD_BUTTON"), NJ("Joy left","JOY_LEFT"),NJ("Joy right","JOY_RIGHT"),NJ("Joy up","JOY_UP"),NJ("Joy down","JOY_DOWN"),D("None"),KNL(),KNL(),KNL(),KNL(),KNL(),KNL(),KNL(),KNL(),KNL(),KNL(),KNL(),KNL(), KNL()};

static int rows_margins[KEY_ROWS+1] = {26, 49, 74, 98, 122, 145, 172, 198};

static int buttons_margins[KEY_ROWS][KEY_COLS+1] = {
{14,44,79,110,143,175,216,257,289,321,353,386},
{14,47,73,99,125,151,177,203,229,255,281,307,333,359,385,417,462,509,541,567,593,620},
{14,62,88,114,140,166,192,218,244,270,296,322,348,374,412,515,541,567,593,620},
{14,42,67,93,119,145,171,197,223,249,275,301,327,353,412,449,476,515,541,567,593,620},
{14,80,106,132,158,184,210,236,262,288,314,340,417,449,476,515,541,567,593,620 },
{28,63,95,329,361,396,515,567,593,620},
{14,80,146,212,278,344,411,476,543}};



void VirtualKeyboard_init(SDL_Surface *screen)
{
	VirtualKeyboard.screen = screen;
	VirtualKeyboard.x = 3; //Where to print the keyboard
	VirtualKeyboard.y = 100;
	vkb_is_init = -1;
	
	char kbd_image[255];

    strcpy (kbd_image, "");

#ifdef OPTIONS_IN_HOME
	char *home;
    home = getenv ("HOME");
    if (home != NULL && strlen (home) < 240)
    {
	strcpy (kbd_image, home);
	strcat (kbd_image, "/");
    }
#endif

    strcat (kbd_image, KBDIMAGE);
	
	tmp_surface=IMG_Load(kbd_image);

	if (tmp_surface == NULL) {write_log("Impossible to load keyboard image\n"); return;}
	image_kbd=SDL_DisplayFormat(tmp_surface);
	SDL_FreeSurface (tmp_surface);
	
	memset(VirtualKeyboard.buf, 0, sizeof(VirtualKeyboard.buf));
	vkb_is_init = 1;
	kbd_is_active=0;
}


void draw_vk()
{
	SDL_Rect dst_rect = {VirtualKeyboard.x, VirtualKeyboard.y, 0, 0};
	SDL_BlitSurface(image_kbd, NULL, VirtualKeyboard.screen, &dst_rect); 	 
}

inline void flip_VKB()
{
	SDL_Flip(VirtualKeyboard.screen);	 
}


int get_index(int x, int y)

{
	
	int row, col;
	
	row=col=0;
	
	if ((x<0)||(x> 620)) return -1; 
	if ((y<0)||(y> 198)) return -1; 
	
	for (row=0; (rows_margins[row]<y)&&(row<KEY_ROWS+1); row++);
	
	if (row==0) return -1; //not valid
	if (rows_margins[row]<y) return -1; //not valid
	
	row--;
	
	for (col=0; (buttons_margins[row][col]<x)&&(row<KEY_COLS+1); col++);
	
	if (col==0) return -1; //not valid
	if (buttons_margins[row][col]<x) return -1; //not valid
	
	col--;
	
	return ((col) + (row) *KEY_COLS); 
	 
}



struct virtkey *get_key_internal()
{
	while(1)
	{
		uint32_t k;
		int x,y,xm, ym, i=0;
		int border_x = VirtualKeyboard.x;
		int border_y = VirtualKeyboard.y;

		draw_vk();
		
		SDL_ShowCursor(SDL_ENABLE);
		
		flip_VKB();

		k = menu_wait_key_press();
		
		SDL_ShowCursor(SDL_DISABLE);

		if (k & KEY_ESCAPE) return NULL;
		else if (k & KEY_SELECT)
		{
			
			SDL_GetMouseState(&xm, &ym);
			x = (xm-border_x);
			y = (ym-border_y);
			
			i = get_index(x,y);
			
			if (i==-1) continue;
			
			#ifdef GEKKO
			WPAD_Rumble(0, 1);
			SDL_Delay(90);
			WPAD_Rumble(0, 0);
			#endif
			
			virtkey_t *key = &keys[i];
			
			return key;
		}
	}

	return NULL;
}


struct virtkey* virtkbd_get_key()
{
	virtkey_t *key;
	
	if (vkb_is_init != 1) return NULL;
	
	pause_sound();
	
	kbd_is_active=1;
	
	key = get_key_internal();
	
	kbd_is_active=0;
	
	resume_sound();

	return key;
}


