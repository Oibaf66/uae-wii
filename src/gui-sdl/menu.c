/*********************************************************************
*
* Copyright (C) 2004,2008,  Simon Kagstrom
*
* Filename:      menu.c
* Author:        Simon Kagstrom <simon.kagstrom@gmail.com>
* Description:   Code for menus (originally for Mophun)
*
* $Id$
*
********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>

#include "sysconfig.h"
#include "menu.h"
#include "VirtualKeyboard.h"

struct joyinfo {
    SDL_Joystick *joy;
    unsigned int axles;
    unsigned int buttons;
};

extern unsigned int nr_joysticks;
extern struct joyinfo joys[];

typedef struct
{
	int n_entries;
	int index;
	int sel;
} submenu_t;

typedef struct
{
	char title[256];
	const char **pp_msgs;
	TTF_Font  *p_font;
	int        x1,y1;
	int        x2,y2;
	int        text_w;
	int        text_h;

	int        n_submenus;
	submenu_t *p_submenus;

	int        cur_sel; /* Main selection */
	int        start_entry_visible;
	int        n_entries;
} menu_t;

static SDL_Surface *real_screen;

#define IS_SUBMENU(p_msg) ( (p_msg)[0] == '^' )
#define IS_TEXT(p_msg) ( (p_msg)[0] == '#' || (p_msg)[0] == ' ' )
#define IS_MARKER(p_msg) ( (p_msg)[0] == '@' )

static int is_inited = 0;
static TTF_Font *menu_font16, *menu_font20;
static TTF_Font *menu_font_alt16;
#if defined(GEKKO)
#define FONT_PATH "/apps/uae/FreeMono.ttf"
#define FONT_ALT_PATH "/apps/uae/Smaller.ttf"
#else
#define FONT_PATH "FreeMono.ttf"
#define FONT_ALT_PATH "Smaller.ttf"
#endif

int fh, fw;

int msgInfo(char *text, int duration, SDL_Rect *irc)
{
	int len = strlen(text);
	int X, Y, wX, wY;
	SDL_Rect src;
	SDL_Rect rc;
	SDL_Rect brc;

	X = (FULL_DISPLAY_X /2) - (len / 2 + 1)*12;
	Y = (FULL_DISPLAY_Y /2) - 24;

	brc.x = FULL_DISPLAY_X/2-2*12; 
	brc.y=Y+42;
	brc.w=48;
	brc.h=20;

	rc.x = X; 
	rc.y=Y;
	rc.w=12*(len + 2);
	rc.h=duration > 0 ? 48 : 80;

	src.x=rc.x+4;
	src.y=rc.y+4;
	src.w=rc.w;
	src.h=rc.h;


	if (irc)
	{
		irc->x=rc.x;
		irc->y=rc.y;
		irc->w=src.w;
		irc->h=src.h;
	}
	SDL_FillRect(real_screen, &src, SDL_MapRGB(real_screen->format, 0, 96, 0));	
	SDL_FillRect(real_screen, &rc, SDL_MapRGB(real_screen->format, 128, 128, 128));
	menu_print_font(real_screen, 255,255,255, X+12, Y+12, text,20);
	SDL_UpdateRect(real_screen, src.x, src.y, src.w, src.h);
	SDL_UpdateRect(real_screen, rc.x, rc.y, rc.w,rc.h);
	if (duration > 0)
		SDL_Delay(duration);
	else if (duration < 0)
	{
		SDL_FillRect(real_screen, &brc, SDL_MapRGB(real_screen->format, 0x00, 0x80, 0x00));
		menu_print_font(real_screen, 0,0,0, FULL_DISPLAY_X/2-12, Y+42, "OK",20);
		SDL_UpdateRect(real_screen, brc.x, brc.y, brc.w, brc.h);
		while (!(KEY_SELECT & menu_wait_key_press())) {}

	}

	return 1;
}

/*
void msgKill(SDL_Rect *rc)
{
	SDL_UpdateRect(real_screen, rc->x, rc->y, rc->w,rc->h);
}
*/

int msgYesNo(char *text, int default_opt, int x, int y)
{
	int len = strlen(text);
	int X, Y, wX, wY;
	SDL_Rect src;
	SDL_Rect rc;
	SDL_Rect brc;
	uint32_t key;
	//int old;

	//old = default_opt;

	if (x < 0)
		X = (FULL_DISPLAY_X /2) - (len / 2 + 1)*12;
	else
		X = x;

	if (y < 0)	
		Y = (FULL_DISPLAY_Y /2) - 48;
	else
		Y = y;

	rc.x=X; 
	rc.y=Y;
	rc.w=12*(len + 2);
	rc.h=80;

	src.x=rc.x+4;
	src.y=rc.y+4;
	src.w=rc.w;
	src.h=rc.h;

	while (1)
	{
		SDL_FillRect(real_screen, &src, SDL_MapRGB(real_screen->format, 0, 96, 0));	
		SDL_FillRect(real_screen, &rc, SDL_MapRGB(real_screen->format, 128, 128, 128));
		menu_print_font(real_screen, 255,255,255, X+12, Y+12, text,20);

		if (default_opt)
		{
			brc.x=rc.x + rc.w/2-5*12; 
			brc.y=rc.y+42;
			brc.w=12*3;
			brc.h=20;
			SDL_FillRect(real_screen, &brc, SDL_MapRGB(real_screen->format, 0x00, 0x80, 0x00));
		}
		else
		{
			brc.x=rc.x + rc.w/2+5*12-2*12-6; 
			brc.y=rc.y+42;
			brc.w=12*3;
			brc.h=20;
			SDL_FillRect(real_screen, &brc, SDL_MapRGB(real_screen->format, 0x80, 0x00, 0x00));
		}
	
		menu_print_font(real_screen, 255,255,255, rc.x + rc.w/2-5*12, Y+42, "YES",20);
		menu_print_font(real_screen, 255,255,255, rc.x + rc.w/2-5*12+8*12, Y+42, "NO",20);
		
		SDL_UpdateRect(real_screen, src.x, src.y, src.w, src.h);
		SDL_UpdateRect(real_screen, rc.x, rc.y, rc.w,rc.h);
		SDL_UpdateRect(real_screen, brc.x, brc.y, brc.w,brc.h);

		//SDL_Flip(real_screen);
		key = menu_wait_key_press();
		if (key & KEY_SELECT)
		{
			return default_opt;
		}
		else if (key & KEY_ESCAPE)
		{
			return 0;
		}
		else if (key & KEY_LEFT)
		{
			default_opt = !default_opt;
		}
		else if (key & KEY_RIGHT)
		{
			default_opt = !default_opt;
		}
	}
}



static int cmpstringp(const void *p1, const void *p2)
{
	const char *p1_s = *(const char**)p1;
	const char *p2_s = *(const char**)p2;

	/* Put directories first */
	if (*p1_s == '[' && *p2_s != '[')
		return -1;
	if (*p1_s != '[' && *p2_s == '[')
		return 1;
	return strcasecmp(* (char * const *) p1, * (char * const *) p2);
}

/* Return true if name ends with ext (for filenames) */
int ext_matches(const char *name, const char *ext)
{
	int len = strlen(name);
	int ext_len = strlen(ext);

	if (len <= ext_len)
		return 0;
	return (strcmp(name + len - ext_len, ext) == 0);
}

static int ext_matches_list(const char *name, const char **exts)
{
	const char **p;

	for (p = exts; *p; p++)
	{
		if (ext_matches(name, *p))
			return 1;
	}

	return 0;
}

static const char **get_file_list(const char *base_dir)
{
	DIR *d = opendir(base_dir);
	const char **file_list;
	int cur = 0;
	struct dirent *de;
	int cnt = 16;

	if (!d)
		return NULL;

	file_list = (const char**)malloc(cnt * sizeof(char*));
	file_list[cur++] = strdup("None"); 
	file_list[cur] = NULL;

	for (de = readdir(d);
	de;
	de = readdir(d))
	{
		char buf[255];
		//ipf files are not enabled in UAE Wii
		const char *exts[] = {".adf", ".ADF", ".adz", ".ADZ", ".zip",".ZIP",".dms", ".DMS",
				".sav", ".SAV", ".uss", ".USS", ".rom", ".ROM", NULL};
		struct stat st;

		snprintf(buf, 255, "%s/%s", base_dir, de->d_name);
		if (stat(buf, &st) < 0)
			continue;
		if (S_ISDIR(st.st_mode)&&strcmp(".", de->d_name))
		{
			char *p;
			size_t len = strlen(de->d_name) + 4;

			p = (char*)malloc( len );
			snprintf(p, len, "[%s]", de->d_name);
			file_list[cur++] = p;
			file_list[cur] = NULL;
		}
		else if (ext_matches_list(de->d_name, exts))
		{
			char *p;

			p = strdup(de->d_name);
			file_list[cur++] = p;
			file_list[cur] = NULL;
		}

		if (cur > cnt - 2)
		{
			cnt = cnt + 32;
			file_list = (const char**)realloc(file_list, cnt * sizeof(char*));
			if (!file_list)
				return NULL;
		}
	}
	closedir(d);
        qsort(&file_list[1], cur-1, sizeof(const char *), cmpstringp);

        return file_list;
}


static submenu_t *find_submenu(menu_t *p_menu, int index)
{
	int i;

	for (i=0; i<p_menu->n_submenus; i++)
	{
		if (p_menu->p_submenus[i].index == index)
			return &p_menu->p_submenus[i];
	}

	return NULL;
}

void menu_print_font(SDL_Surface *screen, int r, int g, int b,
		int x, int y, const char *msg, int font_size)
{
#define _MAX_STRING 64
	SDL_Surface *font_surf;
	SDL_Rect dst = {x, y,  0, 0};
	SDL_Color color = {r, g, b};
	char buf[255];
	unsigned int i;

	memset(buf, 0, sizeof(buf));
	strncpy(buf, msg, 254);
	if (buf[0] != '|' && buf[0] != '^' && buf[0] != '.'
		&& buf[0] != '-' && buf[0] != ' ' && !strstr(buf, "  \""))
	{
		if (strlen(buf)>_MAX_STRING)
		{
			//buf[_MAX_STRING-3] = '.';
			//buf[_MAX_STRING-2] = '.';
			//buf[_MAX_STRING-1] = '.';
			buf[_MAX_STRING] = '\0';
		}
	}
	/* Fixup multi-menu option look */
	for (i = 0; i < strlen(buf) ; i++)
	{
		if (buf[i] == '^' || buf[i] == '|')
			buf[i] = ' ';
	}

	if (font_size == 16) font_surf = TTF_RenderUTF8_Blended(menu_font16, buf, color);
		else font_surf = TTF_RenderUTF8_Blended(menu_font20, buf, color);		
	if (!font_surf)
	{
		fprintf(stderr, "%s\n", TTF_GetError());
		exit(1);
	}

	SDL_BlitSurface(font_surf, NULL, screen, &dst);
	SDL_FreeSurface(font_surf);
}

void menu_print_font_alt(SDL_Surface *screen, int r, int g, int b, int x, int y, const char *msg)
{
#undef _MAX_STRING
#define _MAX_STRING 64
	SDL_Surface *font_surf;
	SDL_Rect dst = {x, y,  0, 0};
	SDL_Color color = {r, g, b};
	char buf[255];
	unsigned int i;

	memset(buf, 0, sizeof(buf));
	strncpy(buf, msg, 254);
	if (buf[0] != '|' && buf[0] != '^' && buf[0] != '.'
		&& buf[0] != '#' && buf[0] != ' ' )
	{
		if (strlen(buf)>_MAX_STRING)
		{
			buf[_MAX_STRING-3] = '.';
			buf[_MAX_STRING-2] = '.';
			buf[_MAX_STRING-1] = '.';
			buf[_MAX_STRING] = '\0';
		}
	}

	/* Fixup multi-menu option look */
	for (i = 0; i < strlen(buf) ; i++)
	{
		if (buf[i] == '^' || buf[i] == '|')
			buf[i] = ' ';
	}

	font_surf = TTF_RenderText_Blended(menu_font_alt16, buf, color);
	if (!font_surf)
	{
		fprintf(stderr, "%s\n", TTF_GetError());
		exit(1);
	}

	SDL_BlitSurface(font_surf, NULL, screen, &dst);
	SDL_FreeSurface(font_surf);
}


static void menu_draw(SDL_Surface *screen, menu_t *p_menu, int sel, int font_size)
{
	int font_height = TTF_FontHeight(p_menu->p_font);
	int line_height = (font_height + font_height / 4);
	int x_start = p_menu->x1;
	int y_start = p_menu->y1 + line_height;
	SDL_Rect r;
	int entries_visible = (p_menu->y2 - p_menu->y1) / line_height - 1;

	int i, y;
	char pTemp[256];

	if ( p_menu->n_entries * line_height > p_menu->y2 )
		y_start = p_menu->y1 + line_height;

	if (p_menu->cur_sel - p_menu->start_entry_visible > entries_visible)
	{
		while (p_menu->cur_sel - p_menu->start_entry_visible > entries_visible)
		{
			p_menu->start_entry_visible ++;
			if (p_menu->start_entry_visible > p_menu->n_entries)
			{
				p_menu->start_entry_visible = 0;
				break;
			}
		}
	}
	else if ( p_menu->cur_sel < p_menu->start_entry_visible )
		p_menu->start_entry_visible = p_menu->cur_sel;

	if (strlen(p_menu->title))
	{
		r.x = p_menu->x1;
		r.y = p_menu->y1;
		r.w = p_menu->x2 - p_menu->x1;
		r.h = line_height-1;
		if (sel < 0)
			SDL_FillRect(screen, &r, SDL_MapRGB(screen->format, 0x40, 0x00, 0x00));
		else
			SDL_FillRect(screen, &r, SDL_MapRGB(screen->format, 0x00, 0xe7, 0xe7));
		menu_print_font(screen, 0,0,0, p_menu->x1, p_menu->y1, p_menu->title, font_size);
	}

	for (i = p_menu->start_entry_visible; i <= p_menu->start_entry_visible + entries_visible; i++)
	{
		const char *msg = p_menu->pp_msgs[i];

		if (i >= p_menu->n_entries)
			break;
		if (IS_MARKER(msg))
			p_menu->cur_sel = atoi(&msg[1]);
		else
		{
			y = (i - p_menu->start_entry_visible) * line_height;

			if (sel < 0)
				menu_print_font(screen, 0x40,0x40,0x40,
						x_start, y_start + y, msg, font_size);
			else if (p_menu->cur_sel == i) /* Selected - color */
				menu_print_font(screen, 0,200,0,
						x_start, y_start + y, msg, font_size);
			else if (IS_SUBMENU(msg))
			{
				if (p_menu->cur_sel == i-1)
					menu_print_font(screen, 0,200,0,
							x_start, y_start + y, msg, font_size);
				else
					menu_print_font(screen, 0x40,0x40,0x40,
							x_start, y_start + y, msg, font_size);
			}
			else if (msg[0] == '#')
			{
				switch (msg[1])
				{
				case '1':
					menu_print_font(screen, 0,0,255,
							x_start, y_start + y, msg+2, font_size);
					break;
				case '2':
					menu_print_font(screen, 0x40,0x40,0x40,
							x_start, y_start + y, msg+2, font_size);
					break;
				default:
					menu_print_font(screen, 0x40,0x40,0x40,
							x_start, y_start + y, msg, font_size);
					break;							
				}
			}
			else /* Otherwise white */
				menu_print_font(screen, 0x40,0x40,0x40,
						x_start, y_start + y, msg, font_size);
			if (IS_SUBMENU(msg))
			{
				submenu_t *p_submenu = find_submenu(p_menu, i);
				int n_pipe = 0;
				int n;

				for (n=0; msg[n] != '\0'; n++)
				{
					/* Underline the selected entry */
					if (msg[n] == '|')
					{
						int16_t n_chars;

						for (n_chars = 1; msg[n+n_chars] && msg[n+n_chars] != '|'; n_chars++);

						n_pipe++;
						if (p_submenu->sel == n_pipe-1)
						{
							int w;
							int h;

							if (TTF_SizeText(p_menu->p_font, "X", &w, &h) < 0)
							{
								fw = w;
								fh = h;
								fprintf(stderr, "%s\n", TTF_GetError());
								exit(1);
							}

							r = (SDL_Rect){ x_start + (n+1) * w-1, y_start + (i+ 1 - p_menu->start_entry_visible) * ((h + h/4)) -3, (n_chars - 1) * w, 2};
							if (p_menu->cur_sel == i-1)
								SDL_FillRect(screen, &r,
										SDL_MapRGB(screen->format, 255,0,0));
							else
								SDL_FillRect(screen, &r,
										SDL_MapRGB(screen->format, 0x40,0x40,0x40));
							break;
						}
					}
				}
			}
		}
	}
}

static int get_next_seq_y(menu_t *p_menu, int v, int dy, int cicle)
{
	if (v + dy < 0)
	 {if (cicle) return (p_menu->n_entries - 1); else return 0;}
	 
	if (v + dy > p_menu->n_entries - 1)
		{if (cicle) return 0; else return (p_menu->n_entries - 1);}
	return v + dy;
}

static void select_next(menu_t *p_menu, int dx, int dy, int cicle)
{
	int next;
	char buffer[256];

	p_menu->cur_sel = get_next_seq_y(p_menu, p_menu->cur_sel, dy, cicle);
	next = get_next_seq_y(p_menu, p_menu->cur_sel, dy + 1, cicle);
	if (p_menu->pp_msgs[p_menu->cur_sel][0] == ' ' ||
			p_menu->pp_msgs[p_menu->cur_sel][0] == '#' ||
			IS_SUBMENU(p_menu->pp_msgs[p_menu->cur_sel]) )
		select_next(p_menu, dx, dy, cicle);

	/* If the next is a submenu */
	if (dx != 0 && IS_SUBMENU(p_menu->pp_msgs[next]))
	{
		submenu_t *p_submenu = find_submenu(p_menu, next);

		p_submenu->sel = (p_submenu->sel + dx) < 0 ? p_submenu->n_entries - 1 :
		(p_submenu->sel + dx) % p_submenu->n_entries;
	}
	else if (dx == -1 && !strcmp(p_menu->pp_msgs[0], "[..]"))
		p_menu->cur_sel = 0;
}

static void select_one(menu_t *p_menu, int sel)
{
	if (sel >= p_menu->n_entries)
		sel = 0;
	p_menu->cur_sel = sel;
	if (p_menu->pp_msgs[p_menu->cur_sel][0] == ' ' ||
			p_menu->pp_msgs[p_menu->cur_sel][0] == '#' ||
			IS_SUBMENU(p_menu->pp_msgs[p_menu->cur_sel]))
		select_next(p_menu, 0, 1 , 1);
}

/*
static int is_submenu_title(menu_t *p_menu, int n)
{
	if (n+1 >= p_menu->n_entries)
		return 0;
	else
		return IS_SUBMENU(p_menu->pp_msgs[n+1]);
}
*/

static void menu_init_internal(menu_t *p_menu, const char *title,
		TTF_Font *p_font, const char **pp_msgs,
		int16_t x1, int16_t y1, int16_t x2, int16_t y2)
{
	int submenu;
	int i;
	int j;

	memset(p_menu, 0, sizeof(menu_t));

	p_menu->pp_msgs = pp_msgs;
	p_menu->p_font = p_font;
	p_menu->x1 = x1;
	p_menu->y1 = y1;
	p_menu->x2 = x2;
	p_menu->y2 = y2;

	p_menu->text_w = 0;
	p_menu->n_submenus = 0;
	strcpy(p_menu->title, title);

	for (p_menu->n_entries = 0; p_menu->pp_msgs[p_menu->n_entries]; p_menu->n_entries++)
	{
		int text_w_font;

		/* Is this a submenu? */
		if (IS_SUBMENU(p_menu->pp_msgs[p_menu->n_entries]))
		{
			p_menu->n_submenus++;
			continue; /* Length of submenus is unimportant */
		}

		if (TTF_SizeText(p_font, p_menu->pp_msgs[p_menu->n_entries], &text_w_font, NULL) != 0)
		{
			fprintf(stderr, "%s\n", TTF_GetError());
			exit(1);
		}
		if (text_w_font > p_menu->text_w)
			p_menu->text_w = text_w_font;
	}
	if (p_menu->text_w > p_menu->x2 - p_menu->x1)
		p_menu->text_w = p_menu->x2 - p_menu->x1;

	if ( !(p_menu->p_submenus = (submenu_t *)malloc(sizeof(submenu_t) * p_menu->n_submenus)) )
	{
		perror("malloc failed!\n");
		exit(1);
	}

	j=0;
	submenu = 0;
	for (; j < p_menu->n_entries; j++)
	{
		if (IS_SUBMENU(p_menu->pp_msgs[j]))
		{
			int n;

			p_menu->p_submenus[submenu].index = j;
			p_menu->p_submenus[submenu].sel = 0;
			p_menu->p_submenus[submenu].n_entries = 0;
			for (n=0; p_menu->pp_msgs[j][n] != '\0'; n++)
			{
				if (p_menu->pp_msgs[j][n] == '|')
					p_menu->p_submenus[submenu].n_entries++;
			}
			submenu++;
		}
	}
	p_menu->text_h = p_menu->n_entries * (TTF_FontHeight(p_font) + TTF_FontHeight(p_font) / 4);
}

static void menu_fini(menu_t *p_menu)
{
	free(p_menu->p_submenus);
}

uint32_t menu_wait_key_press(void)
{
	SDL_Event ev;
	uint32_t keys = 0;

	while (1)
	{
		int i, hats, nr;
		SDL_Joystick *joy;
		static int joy_keys_changed;
		static int joy_keys_last;

		/* Wii-specific, sorry */
		for (nr = 0; nr < nr_joysticks; nr++) {
			joy = joys[nr].joy;
			if (!joy)
				continue;

			hats = SDL_JoystickNumHats (joy); 
			for (i = 0; i < hats; i++) {
				Uint8 v = SDL_JoystickGetHat (joy, i);
				int x = 0, y = 0;

				if (v & SDL_HAT_UP)
					keys |= KEY_UP;
				if (v & SDL_HAT_DOWN)
					keys |= KEY_DOWN;
				if (v & SDL_HAT_LEFT)
					keys |= KEY_LEFT;
				if (v & SDL_HAT_RIGHT)
					keys |= KEY_RIGHT;
			}
			
			Sint16 axis0 = SDL_JoystickGetAxis(joy, 0);
			Sint16 axis1 = SDL_JoystickGetAxis(joy, 1);
			
			if ( axis0 < -15000 )  keys |= KEY_LEFT;
			else if (axis0 > 15000 )  keys |= KEY_RIGHT;
			
			if (axis1 < -15000 )  keys |= KEY_UP;
			else if( axis1 > 15000 )  keys |= KEY_DOWN;	
				
			
			if (SDL_JoystickGetButton(joy, 0) != 0 ||      /* A */
					SDL_JoystickGetButton(joy, 3) != 0 ||  /* 2 */
					SDL_JoystickGetButton(joy, 9) != 0 ||  /* CA */
					SDL_JoystickGetButton(joy, 10) != 0)   /* CB */
				keys |= KEY_SELECT;
			if (SDL_JoystickGetButton(joy, 2) != 0 ||      /* 1 */
					SDL_JoystickGetButton(joy, 11) != 0 || /* CX */
					SDL_JoystickGetButton(joy, 12) != 0)   /* CY */
				keys |= KEY_ESCAPE;
			if (SDL_JoystickGetButton(joy, 5) != 0 ||      /* + */
					SDL_JoystickGetButton(joy, 18) != 0)   /* C+ */
				keys |= KEY_PAGEUP;
			if (SDL_JoystickGetButton(joy, 4) != 0 ||      /* + */
					SDL_JoystickGetButton(joy, 17) != 0)   /* C+ */
				keys |= KEY_PAGEDOWN;
		}
		joy_keys_changed = keys != joy_keys_last;
		joy_keys_last = keys;
		if (!joy_keys_changed)
			keys = 0;

		if (SDL_PollEvent(&ev))
		{
			switch(ev.type)
			{
			case SDL_KEYDOWN:
				switch (ev.key.keysym.sym)
				{
				case SDLK_UP:
					keys |= KEY_UP;
					break;
				case SDLK_DOWN:
					keys |= KEY_DOWN;
					break;
				case SDLK_LEFT:
					keys |= KEY_LEFT;
					break;
				case SDLK_RIGHT:
					keys |= KEY_RIGHT;
					break;
				case SDLK_PAGEDOWN:
					keys |= KEY_PAGEDOWN;
					break;
				case SDLK_PAGEUP:
					keys |= KEY_PAGEUP;
					break;
				case SDLK_RETURN:
				case SDLK_SPACE:
					keys |= KEY_SELECT;
					break;
				case SDLK_HOME:
				case SDLK_ESCAPE:
					keys |= KEY_ESCAPE;
					break;
				default:
					break;
				}
				break;
				case SDL_QUIT:
					exit(0);
					break;
				default:
					break;

			}
			break;
		}

		if (keys != 0)
			break;
		SDL_Delay(100);
	}
	return keys;
}


extern void PicDisplay(char *name, int off_x, int off_y, int wait);
extern const char **get_t64_list(char *t64);
extern const char **get_prg_list(char *t64);

extern char curdir[256];

static int menu_select_internal(SDL_Surface *screen,
		menu_t *p_menu, int *p_submenus, int sel,
		void (*select_next_cb)(menu_t *p, void *data),
		void *select_next_cb_data, int font_size)
{
	int ret = -1;
	int i;

	for (i = 0; i < p_menu->n_submenus; i++)
		p_menu->p_submenus[i].sel = p_submenus[i];

	while(1)
	{
		SDL_Rect r = {p_menu->x1, p_menu->y1,
				p_menu->x2 - p_menu->x1, p_menu->y2 - p_menu->y1};
		uint32_t keys;
		int sel_last = p_menu->cur_sel;

		SDL_FillRect(screen, &r, SDL_MapRGB(screen->format, 0xff, 0xff, 0xff));

		menu_draw(screen, p_menu, 0, font_size);
		SDL_Flip(screen);

		keys = menu_wait_key_press();

		if (keys & KEY_UP)
			select_next(p_menu, 0, -1, 1);
		else if (keys & KEY_DOWN)
			select_next(p_menu, 0, 1, 1);
		else if (keys & KEY_PAGEUP)
			select_next(p_menu, 0, -18, 0);
		else if (keys & KEY_PAGEDOWN)
			select_next(p_menu, 0, 18, 0);
		else if (keys & KEY_LEFT)
			select_next(p_menu, -1, 0 ,1);
		else if (keys & KEY_RIGHT)
			select_next(p_menu, 1, 0 ,1);
		else if (keys & KEY_ESCAPE)
			break;
		else if (keys & KEY_SELECT)
		{
			ret = p_menu->cur_sel;
			int i;

			for (i=0; i<p_menu->n_submenus; i++)
				p_submenus[i] = p_menu->p_submenus[i].sel;
			break;
		}
		/* Invoke the callback when an entry is selected */
		if (sel_last != p_menu->cur_sel &&
				select_next_cb != NULL)
			select_next_cb(p_menu, select_next_cb_data);
	}

	SDL_FillRect(screen, 0, SDL_MapRGB(screen->format, 0, 0, 0));
	return ret;
}

int menu_select_sized(const char *title, const char **msgs, int *submenus, int sel,
		int x, int y, int x2, int y2,
		void (*select_next_cb)(menu_t *p, void *data),
		void *select_next_cb_data, int font_size)

{
	menu_t menu;
	int out;
	/*
	int info;

	if (!strcmp(title, "Folder") || !strcmp(title, "Single File") ||
			!strcmp(title, "C-64 Disc") || !strcmp(title, "C-64 Tape") || sel < 0)
		info = 0;
	else
		info = 1;
	*/
	if (font_size == 16) menu_init_internal(&menu, title, menu_font16, msgs, x, y, x2, y2);
	else menu_init_internal(&menu, title, menu_font20, msgs, x, y, x2, y2);

	if (sel >= 0)
		select_one(&menu, sel);
	out = menu_select_internal(real_screen, &menu, submenus, sel,
			select_next_cb, select_next_cb_data, font_size);

	menu_fini(&menu);

	return out;
}

int menu_select_title(const char *title, const char **msgs, int *submenus)
{
	SDL_FillRect(real_screen, 0, SDL_MapRGB(real_screen->format, 0, 0, 0));
	return menu_select_sized(title, msgs, submenus, 0,
			32, 32, FULL_DISPLAY_X-32, FULL_DISPLAY_Y-32,
			NULL, NULL, 20);
}

int menu_select(const char **msgs, int *submenus)
{
	return menu_select_title("", msgs, submenus);
}

static const char *menu_select_file_internal(const char *dir_path,
		int x, int y, int x2, int y2, const char *selected_file, int which)
{
	const char **file_list = get_file_list(dir_path);
	char *sel;
	char *out;
	const char *ptr_selected_file;
	int opt;
	int i;
	char buf[64];
	
	if (file_list == NULL)
		return NULL;

	if (selected_file) 
	{
		ptr_selected_file= strrchr(selected_file,'/');
		if (ptr_selected_file) ptr_selected_file++;
		else ptr_selected_file = selected_file;
		snprintf(buf,64,"df%d:%s",which, ptr_selected_file);
		opt = menu_select_sized(buf, file_list, NULL, 0, x, y, x2, y2, NULL, NULL, 16);
	}
	else opt = menu_select_sized("Select file", file_list, NULL, 0, x, y, x2, y2, NULL, NULL ,16);
	
	if (opt < 0)
		return NULL;
	sel = strdup(file_list[opt]);

	/* Cleanup everything - file_list is NULL-terminated */
        for ( i = 0; file_list[i]; i++ )
        	free((void*)file_list[i]);
        free(file_list);

	if (!sel)
		return NULL;
        /* If this is a folder, enter it recursively */
        if (sel[0] == '[')
        {
        	char buf[255];
        	int len = strlen(sel);
        	int s;

        	/* Remove trailing ] */
        	sel[len-1] = '\0';
        	s = snprintf(buf, 128, "%s/%s", dir_path, sel + 1);

        	/* We don't need this anymore */
        	free((void*)sel);
        	/* Too deep recursion! */
        	if (s >= sizeof(buf))
        		return NULL;
        	return menu_select_file(buf, selected_file, which);
        }

	out = (char*)malloc(strlen(dir_path) + strlen(sel) + 4);
	snprintf(out, strlen(dir_path) + strlen(sel) + 4,
			"%s/%s", dir_path, sel);

	free(sel);
        return out;
}

/*
const char *menu_select_file_start(const char *dir_path, const char **d64_name)
{
	const char *file = menu_select_file_internal(dir_path,
			32, 32, FULL_DISPLAY_X, FULL_DISPLAY_Y - 32);

	if (!file)
		return NULL;

	return file;
}
*/
const char *menu_select_file(const char *dir_path,const char *selected_file, int which)
{
	if (dir_path == NULL)
		dir_path = "";
	return menu_select_file_internal(dir_path,
			0, 32, FULL_DISPLAY_X, FULL_DISPLAY_Y - 32, selected_file, which);
}

static TTF_Font *read_font(const char *path, int font_size)
{
	TTF_Font *out;
	SDL_RWops *rw;
	Uint8 *data = (Uint8*)malloc(1 * 1024*1024);
	FILE *fp = fopen(path, "r");

	if (!data) {
		fprintf(stderr, "Malloc failed\n");
		exit(1);
	}
	if (!fp) {
		fprintf(stderr, "Could not open font\n");
		exit(1);
	}
	fread(data, 1, 1 * 1024 * 1024, fp);
	rw = SDL_RWFromMem(data, 1 * 1024 * 1024);
	if (!rw) 
	{
		fprintf(stderr, "Could not create RW: %s\n", SDL_GetError());
		exit(1);
	}
	out = TTF_OpenFontRW(rw, 1, font_size);
	if (!out)
	{
		fprintf(stderr, "Unable to open font %s\n", path);
		exit(1);
	}
	fclose(fp);

	return out;
}

void menu_init(SDL_Surface *screen)
{
	TTF_Init();

	menu_font16 = read_font(FONT_PATH, 16);
	menu_font20 = read_font(FONT_PATH, 20);
	
	menu_font_alt16 = read_font(FONT_ALT_PATH,16);

	real_screen = screen;
	virtkbd_init(screen, menu_font_alt16);
	is_inited = 1;
}

int menu_is_inited(void)
{
	return is_inited;
}
