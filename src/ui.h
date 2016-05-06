#ifndef __UI_H_
#define __UI_H_

#define WITHIN(x, min, max) (x > min && x < max)
#define UI_MAX_OBJECTS   100
#define UI_MARGIN        50    // How much of a buffer to give bitmap drawing (in pixels)
#define UI_IO_SIZE       10
#define UI_TOOLBAR_HEIGHT 80

#define UI_WIDTH  1024
#define UI_HEIGHT 768

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

typedef struct {
	int x, y;
} UIPoint;

typedef struct {
    UIPoint loc;
    UIPoint  size;
} UIRect;

typedef struct UIDrawable {
	struct Object *object;
	ALLEGRO_BITMAP *bitmap;
	UIPoint size;
	UIPoint loc;
} UIDrawable;

typedef enum {
    UITargetObject,
    UITargetOutput,
    UITargetInput
} UITargetType;

typedef void (*UIDrawCallback)(struct Object *, bool is_selected, UIPoint, ALLEGRO_EVENT *);

// Object constructors

UIRect UIRect_points(UIPoint p, UIPoint s);
UIRect UIRect_xy(int x, int y, int width, int height);

UIPoint UIPoint_xy(int x, int y);
UIPoint UIPoint_offset(UIPoint origin, int x, int y);
UIPoint UIPoint_add(UIPoint origin, UIPoint s);
UIPoint UIPoint_subtract(UIPoint p1, UIPoint p2);
extern UIPoint UIPoint_margin;
#define UIPoint_add_margin(p)       (UIPoint_add(UIPoint_margin, p))
#define UIPoint_offset_margin(x, y) (UIPoint_offset(UIPoint_margin, x, y))

UIPoint UIPoint_xy(int width, int height);

UIDrawable *UIDrawable_with(struct Object *o, UIPoint loc);

// UI Behaviour

void UI_set_target(struct Object *o, UITargetType target_type);

// Drawing tools

void UI_draw_title  (UIPoint loc, char *text);
void UI_draw_label  (UIPoint loc, char *text);
void UI_draw_circle (UIPoint loc, int r, ALLEGRO_COLOR fill_color);
void UI_draw_border (UIRect rect, ALLEGRO_COLOR color);
void UI_draw_box    (UIRect r, ALLEGRO_COLOR fill_color);
void UI_draw_line   (UIPoint p1, UIPoint p2);
void UI_draw_line_thick(UIPoint p1, UIPoint p2);

// Utilities

UIPoint UI_io_point_for_drawable(UIDrawable *d, bool is_input);
bool UI_is_point_near(UIPoint p1, UIPoint p2, int threshhold);
bool UI_is_point_near_io(UIPoint p, UIDrawable *d, bool is_input);
bool UI_is_point_within(UIPoint p, int x1, int x2, int y1, int y2);
bool UI_is_point_within_rect(UIPoint p, UIRect r);
bool UI_is_point_within_bounds(UIPoint p, UIPoint loc, UIPoint bounds);
bool UI_is_point_within_drawable(UIPoint p, UIDrawable *d);

// Colors

#define UI_COLOR_ALPHA  (al_map_rgba(0, 0, 0, 0))
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
