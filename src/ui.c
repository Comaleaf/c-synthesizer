#include "ui.h"
#include "audio.h"

#include "ui_keys.h"

int note_to_index(char n) {
	switch (n) {
		case 'a': return 0;
		case 'w': return 1;
		case 's': return 2;
		case 'e': return 3;
		case 'd': return 4;
		case 'f': return 5;
		case 't': return 6;
		case 'g': return 7;
		case 'y': return 8;
		case 'h': return 9;
		case 'u': return 10;
		case 'j': return 11;
	}

	return -1;
}

char index_to_note(int i) {
	switch (i) {
		case  0: return 'a';
		case  1: return 'w';
		case  2: return 's';
		case  3: return 'e';
		case  4: return 'd';
		case  5: return 'f';
		case  6: return 't';
		case  7: return 'g';
		case  8: return 'y';
		case  9: return 'h';
		case 10: return 'u';
		case 11: return 'j';
	}

	return '\0';
}

// Properties

ALLEGRO_FONT *_font;

UIObject *_objects[100];
UIPoint _mouse;

UIPoint drag_offset;
UIObject *drag_object;

void (*_draw_handlers[UITYPE_COUNT])(void *obj, UIPoint loc);

void UI_init() {
	_draw_handlers[UITYPE_KEYBOARD] = &UI_draw_keys;

	_objects[0] = UIObject_with(UIPoint_with_coords(100, 100), 0, "Oscillator");
	_objects[1] = UIObject_with(UIPoint_with_coords(100, 300), 0, "Filter");
	_objects[2] = UIObject_with(UIPoint_with_coords(100, 500), 0, "Delay");
	_objects[3] = UIObject_with(UIPoint_with_coords(100, 700), 0, "Boat");

	_objects[0]->next = _objects[1];
	_objects[1]->next = _objects[2];

	initwindow(1024, 768);
	initfont();
	initmouse();
	initkeyboard();

	al_init_ttf_addon();
	_font = al_load_ttf_font("data/Roboto-Medium.ttf", 35, 0);
	 
	if (!_font) {
		exit(-1);
	}

    create_event_queue();

	reg_display_events();
	reg_mouse_events();
	reg_keyboard_events();
}

void UI_close() {
	closemouse();
	closegraph();
}

bool UI_next() {
	if (!check_if_event()) return true;

	wait_for_event();

	// Event Phase
	if (event_close_display())
		return false;

	if (event_mouse_position_changed()) {
		get_mouse_coordinates();
		_mouse.x = XMOUSE;
		_mouse.y = YMOUSE;
	}

	for (int i = 0; _objects[i] != NULL; i++) {
		_objects[i]->is_focused = UI_is_point_within_object(_mouse, _objects[i]);
	}

	if (event_mouse_button_down() && event_mouse_left_button_down()) {
		// Select
		if (drag_object == NULL) {
			for (int i = 0; _objects[i] != NULL; i++) {
				if (_objects[i]->is_focused) {
					drag_object = _objects[i];
					drag_offset = UIPoint_with_offset(_objects[i]->loc, -_mouse.x, -_mouse.y);
				}
			}
		}
		// Deselect
		else 
			drag_object = NULL;
	}
	else if (drag_object != NULL)
		// DRAGGING
		drag_object->loc = UIPoint_with_offset(drag_offset, _mouse.x, _mouse.y);
	// Keyboard
	for (int i = 0; i < 12; i++) {
		if (event_key(index_to_note(i))) {
			if (event_key_down())
				Audio_enable_note(i);
			else if (event_key_up())
				Audio_disable_note(i);
		}
	}

	// Draw Phase
	al_clear_to_color(al_map_rgb(0xFF, 0xFF, 0xFF));

	for (int i = 0; _objects[i] != NULL; i++) {
		_draw_handlers[UITYPE_KEYBOARD](_objects[i], _mouse);
	}

	al_flip_display();

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

UIObject* UIObject_with(UIPoint loc, int z_order, char* label) {
	UIObject *o = malloc(sizeof(UIObject));

	o->loc        = loc;
	o->is_focused = false;
	o->z_order    = z_order;
	o->label      = label;
	o->size       = UISize_with_bounds(200, 400);
	o->next       = NULL;
	o->type       = UITYPE_KEYBOARD;

	return o;
}

void UI_draw_box(UIPoint p, UISize s, unsigned fill_color) {
	filled_rectangle(p.x, p.y, p.x+s.width, p.y+s.height, fill_color);
	setcolor(BLACK);
	rectangle(p.x, p.y, p.x+s.width, p.y+s.height, 4);
}

void UI_draw_line(UIPoint p1, UIPoint p2) {
	setcolor(BLACK);
	line(p1.x, p1.y, p2.x, p2.y, 2);
}

void UI_draw_object(UIObject *o) {
	unsigned color;

	if (o == drag_object)
		color = RED;
	else if (o->is_focused)
		color = YELLOW;
	else
		color = WHITE;

	UI_draw_box(o->loc, o->size, color); 

	// There is a connection
	if (o->next != NULL)
		UI_draw_line(UIPoint_with_offset(o->loc, o->size.width, 30), UIPoint_with_offset(o->next->loc, 0, 30));

	// Connecting nodes
	UI_draw_box(UIPoint_with_offset(o->loc, -10, +20),              UISize_with_bounds(20, 20), GREEN);
	UI_draw_box(UIPoint_with_offset(o->loc, o->size.width-10, +20), UISize_with_bounds(20, 20), GREEN);

	setcolor(BLACK);
	al_draw_text(_font, al_map_rgb(0, 0, 0), o->loc.x+o->size.width/2, o->loc.y+10, ALLEGRO_ALIGN_CENTRE, o->label);
}


bool UI_is_point_within_object(UIPoint p, UIObject *o) {
	return (WITHIN(p.x, o->loc.x, o->loc.x+o->size.width) &&
            WITHIN(p.y, o->loc.y, o->loc.y+o->size.height));
}

