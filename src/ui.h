#ifndef __UI_H_
#define __UI_H_

#define WITHIN(x, min, max) (x > min && x < max)
#define UI_MAX_OBJECTS   100
#define UI_BITMAP_BUFFER 10    // How much of a buffer to give bitmap drawing (in pixels)
#define UI_IO_SIZE       20

#include "types.h"
#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>

// Forward declarations (needed to avoid inclusion dependency cycle

struct Object;

// UI

void UI_init();
void UI_close();
bool UI_next();

// Types

typedef enum {
	UI_Event_Mouse_Move = (1 << 0),
	UI_Event_Mouse_Down = (1 << 1),
	UI_Event_Mouse_Up   = (1 << 2),
	UI_Event_Key_Down   = (1 << 3),
	UI_Event_Key_Up     = (1 << 4)
} UIEventMask;

typedef struct {
	int x, y;
} UIPoint;

typedef struct {
	int width;
	int height;
} UISize;

typedef struct UIDrawable {
	struct Object *object;
	ALLEGRO_BITMAP *bitmap;
	int     z_order;
	bool    is_focused;
	UISize  size;
	UIPoint loc;
} UIDrawable;

typedef void (*UIDrawCallback)(struct Object *, UIEventMask, UIPoint, ALLEGRO_EVENT *);

// Object constructors

UIPoint UIPoint_with_coords(int x, int y);
UIPoint UIPoint_with_offset(UIPoint origin, int x, int y);
UIPoint UIPoint_with_size_offset(UIPoint origin, UISize s);

UISize UISize_with_bounds(int width, int height);

UIDrawable *UIDrawable_with(struct Object *o, int z_order, UIPoint loc);

// UI Behaviour

void UI_set_drag_object(struct Object *o);

// Drawing tools

void UI_draw_title   (int x, int y, char *text);
void UI_draw_label   (int x, int y, char *text);
void UI_draw_ellipse (int x, int y, UISize s, ALLEGRO_COLOR fill_color);
void UI_draw_box     (int x, int y, UISize s, ALLEGRO_COLOR fill_color);
void UI_draw_line    (UIPoint p1, UIPoint p2);

// Utilities

bool UI_is_point_within_drawable(UIPoint p, UIDrawable *d);

// Colors

#define UI_COLOR_WHITE  (al_map_rgb(0xFF, 0xFF, 0xFF))
#define UI_COLOR_GRAY   (al_map_rgb(0x80, 0x80, 0x80))
#define UI_COLOR_BLACK  (al_map_rgb(0x00, 0x00, 0x00))
#define UI_COLOR_GREEN  (al_map_rgb(0x00, 0xFF, 0x00))
#define UI_COLOR_RED    (al_map_rgb(0xFF, 0x00, 0x00))
#define UI_COLOR_PURPLE (al_map_rgb(0xFF, 0x00, 0xFF))
#define UI_COLOR_YELLOW (al_map_rgb(0xFF, 0xFF, 0x00))

#define UI_COLOR_CONTROL   UI_COLOR_PURPLE
#define UI_COLOR_AUDIO     UI_COLOR_GREEN
#define UI_COLOR_HIGHLIGHT UI_COLOR_YELLOW
#define UI_COLOR_ACTIVE    UI_COLOR_RED

#endif
