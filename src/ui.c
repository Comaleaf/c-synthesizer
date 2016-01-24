#include "ui.h"

#include <ctype.h>
#include <math.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>

#include "object.h"
#include "types.h"
#include "audio.h"
#include "keyboard.h"

// Private Properties

ALLEGRO_DISPLAY *ui_display = NULL;
ALLEGRO_EVENT_QUEUE *ui_event_queue = NULL;

UIPoint ui_mouse; // Cursor position

// Object being manipulated (dragged)
struct {
	UIDrawable *target;
	UIPoint offset;
} ui_drag;

// Items on the canvas
UIDrawable *ui_items[UI_MAX_OBJECTS];

void UI_init() {
    ui_display = al_create_display(1024, 768);
	ui_event_queue = al_create_event_queue();

	al_register_event_source(ui_event_queue, al_get_display_event_source(ui_display));
	al_register_event_source(ui_event_queue, al_get_mouse_event_source());
	al_register_event_source(ui_event_queue, al_get_keyboard_event_source());

	al_init_ttf_addon();

	ui_items[0] = UIDrawable_with(Object_with(TYPE_KEYBOARD),   0, UIPoint_with_coords(100, 100));
	ui_items[1] = UIDrawable_with(Object_with(TYPE_OSCILLATOR), 0, UIPoint_with_coords(300, 300));
	ui_items[2] = UIDrawable_with(Object_with(TYPE_LFO),        0, UIPoint_with_coords(300, 700));

	ui_items[0]->object->ui = ui_items[0];
	ui_items[1]->object->ui = ui_items[1];
	ui_items[2]->object->ui = ui_items[2];

	ui_items[0]->object->next = ui_items[1]->object;
	ui_items[1]->object->next = ui_items[2]->object;
}

void UI_close() {
	al_destroy_display(ui_display);
}

void UI_set_drag_object(struct Object *o) {
	ui_drag.target = o->ui;
	ui_drag.offset = UIPoint_with_offset(o->ui->loc, -ui_mouse.x, -ui_mouse.y);
}

bool UI_next() {
	ALLEGRO_EVENT ev;
	ALLEGRO_TIMEOUT timeout;
	
	al_init_timeout(&timeout, 0.06);

	if (!al_wait_for_event_until(ui_event_queue, &ev, &timeout))
		return true;
	
	UIEventMask event_mask = 0;
	
	// Event Phase
	switch (ev.type) {
		case ALLEGRO_EVENT_DISPLAY_CLOSE: return false;
		case ALLEGRO_EVENT_MOUSE_AXES:
			event_mask |= UI_Event_Mouse_Move;
			ui_mouse.x = ev.mouse.x;
			ui_mouse.y = ev.mouse.y;
			break;
		case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN: event_mask |= UI_Event_Mouse_Down; break;
		case ALLEGRO_EVENT_MOUSE_BUTTON_UP:   event_mask |= UI_Event_Mouse_Up;   break;
		case ALLEGRO_EVENT_KEY_DOWN:          event_mask |= UI_Event_Key_Down;   break;
		case ALLEGRO_EVENT_KEY_UP:            event_mask |= UI_Event_Key_Up;     break;
	}

	for (int i = 0; ui_items[i] != NULL; i++) {
		ui_items[i]->is_focused = UI_is_point_within_drawable(ui_mouse, ui_items[i]);
	}

	if (ui_drag.target != NULL) {
		// DRAGGING
		ui_drag.target->loc = UIPoint_with_offset(ui_drag.offset, ui_mouse.x, ui_mouse.y);

		if (event_mask & UI_Event_Mouse_Up)
			ui_drag.target = NULL;
	}

	/*
	// Keyboard
	for (int i = 0; i < 12; i++) {
		if (event_key(index_to_note(i))) {
			if (event_key_down())
				Audio_enable_note(i);
			else if (event_key_up())
				Audio_disable_note(i);
		}
	}*/

	UIEventMask mask_for_object;

	for (int i = 0; ui_items[i] != NULL; i++) {
		ObjType t = ui_items[i]->object->type;

		mask_for_object = UI_is_point_within_drawable(ui_mouse, ui_items[i]) ? event_mask : event_mask & ~(UI_Event_Mouse_Down | UI_Event_Mouse_Up);
		if (obj_get_event_mask(t) & event_mask) {
			al_set_target_bitmap(ui_items[i]->bitmap);
			al_clear_to_color(al_map_rgba(0, 0, 0, 0));
			obj_get_draw(t)(ui_items[i]->object, mask_for_object, ui_mouse, &ev);
		}
	}

	// Draw Phase
	al_set_target_bitmap(al_get_backbuffer(ui_display)); // Go back to drawing to the display

	// Now draw each bitmap and connections
	for (int i = 0; ui_items[i] != NULL; i++) {
		al_draw_bitmap(ui_items[i]->bitmap, ui_items[i]->loc.x-UI_BITMAP_BUFFER, ui_items[i]->loc.y-UI_BITMAP_BUFFER, 0x00);

		// Connection
		if (ui_items[i]->object->next != NULL) {
			UI_draw_line(
				UIPoint_with_offset(ui_items[i]->loc, ui_items[i]->size.width, ui_items[i]->size.height/2),
				UIPoint_with_offset(ui_items[i]->object->next->ui->loc, 0, ui_items[i]->object->next->ui->size.height/2)
			);
		}
	}

	// Draw IO boxes
	for (int i = 0; ui_items[i] != NULL; i++) {
		ObjType t = ui_items[i]->object->type;

		if (obj_get_input(t) != IONone) {
			UI_draw_box(
				ui_items[i]->loc.x - UI_IO_SIZE/2,
				ui_items[i]->loc.y + ui_items[i]->size.height/2 - UI_IO_SIZE/2,
				UISize_with_bounds(UI_IO_SIZE, UI_IO_SIZE),
				obj_get_input(t) == IOControl ? UI_COLOR_CONTROL : UI_COLOR_AUDIO
			);
		}
		else {
			// Must be an audio source if there is no input node, so tell audio module
			// @todo Move this to somewhere more obvious
			Audio_set_source(ui_items[i]->object);
		}

		if (obj_get_output(t) != IONone)
			UI_draw_box(
				ui_items[i]->loc.x + ui_items[i]->size.width - UI_IO_SIZE/2,
				ui_items[i]->loc.y + ui_items[i]->size.height/2 - UI_IO_SIZE/2,
				UISize_with_bounds(UI_IO_SIZE, UI_IO_SIZE),
				obj_get_output(t) == IOControl ? UI_COLOR_CONTROL : UI_COLOR_AUDIO
			);
	}

	al_flip_display();
	al_clear_to_color(UI_COLOR_WHITE); // Clear after flip - that way the buffer is cleared ready, lower latency

	return true;
}

UIPoint UIPoint_with_coords(int x, int y) {
	UIPoint p = {x, y};
	return p;
}

UIPoint UIPoint_with_offset(UIPoint origin, int x, int y) {
	return UIPoint_with_coords(origin.x+x, origin.y+y);
}

UIPoint UIPoint_with_size_offset(UIPoint origin, UISize s) {
	return UIPoint_with_offset(origin, s.width, s.height);
}

UISize UISize_with_bounds(int width, int height) {
	UISize s = {width, height};
	return s;
}

UIDrawable *UIDrawable_with(struct Object *o, int z_order, UIPoint loc) {
	UIDrawable *d = malloc(sizeof(UIDrawable));

	d->object  = o;
	d->z_order = z_order;
	d->loc     = loc;
	d->size    = obj_get_size(o->type); // Set the size to be the UISize defined by the module
	d->bitmap  = al_create_bitmap(d->size.width + UI_BITMAP_BUFFER*2, d->size.height + UI_BITMAP_BUFFER*2); // Derive bitmap size from retrieved UISize

	return d;
}

void UI_draw_title(int x, int y, char *text) {
	static ALLEGRO_FONT *font = NULL;
	
	if (!font)
		font = al_load_ttf_font("data/Roboto-Medium.ttf", 42, 0);
	
	al_draw_text(font, UI_COLOR_BLACK, x, y, ALLEGRO_ALIGN_CENTRE, text);
}

void UI_draw_label(int x, int y, char *text) {
	static ALLEGRO_FONT *font = NULL;
	
	if (!font)
		font = al_load_ttf_font("data/Roboto-Medium.ttf", 28, 0);
	
	al_draw_text(font, UI_COLOR_BLACK, x, y, ALLEGRO_ALIGN_CENTRE, text);
}

void UI_draw_ellipse(int x, int y, UISize s, ALLEGRO_COLOR fill_color) {
	float rx = s.width/2;
	float ry = s.height/2;
	float cx = x + rx;
	float cy = y + ry;
	al_draw_filled_ellipse(cx, cy, rx, ry, fill_color);
	al_draw_ellipse(cx, cy, rx, ry, UI_COLOR_BLACK, 4);
}

void UI_draw_box(int x, int y, UISize s, ALLEGRO_COLOR fill_color) {
	al_draw_filled_rectangle(x, y, x+s.width, y+s.height, fill_color);
	al_draw_rectangle(x, y, x+s.width, y+s.height, UI_COLOR_BLACK, 4);
}

void UI_draw_line(UIPoint p1, UIPoint p2) {
	al_draw_line(p1.x, p1.y, p2.x, p2.y, UI_COLOR_BLACK, 2);
}

bool UI_is_point_within_drawable(UIPoint p, UIDrawable *d) {
	return (WITHIN(p.x, d->loc.x, d->loc.x + d->size.width) &&
            WITHIN(p.y, d->loc.y, d->loc.y + d->size.height));
}
