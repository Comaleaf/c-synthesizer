#ifndef __UI_H_
#define __UI_H_

#include <graphics_lib.h>
#include <ctype.h>
#include <math.h>

#define WITHIN(x, min, max) (x > min && x < max)

// Allegro stuff

#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>

// UI

void UI_init();
void UI_close();
bool UI_next();

// Types

typedef enum {
	UITYPE_KEYBOARD,
	UITYPE_OSCILLATOR,
	UITYPE_DELAY,
	UITYPE_REVERB,

	UITYPE_COUNT // Should be the number of different UI types
} UIType;

typedef struct {
	int x, y;
} UIPoint;

typedef struct {
	int width;
	int height;
} UISize;

typedef struct UIObject {
	UIType  type;
	UIPoint loc;
	UISize  size;
	bool    is_focused;
	int     z_order;
	char*   label;

	struct UIObject *next;
} UIObject;

// Object constructors

UIPoint UIPoint_with_coords(int x, int y);
UIPoint UIPoint_with_offset(UIPoint origin, int x, int y);
UIPoint UIPoint_with_size_offset(UIPoint origin, UISize s);

UISize UISize_with_bounds(int width, int height);

UIObject* UIObject_with(UIPoint loc, int z_order, char* label);

// Drawing tools

void UI_draw_box(UIPoint p, UISize s, unsigned fill_color);
void UI_draw_line(UIPoint p1, UIPoint p2);
void UI_draw_object(UIObject *o);

// Utilities

bool UI_is_point_within_object(UIPoint p, UIObject *o);

#endif
