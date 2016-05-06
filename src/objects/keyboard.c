#include "keyboard.h"

#include "object.h"
#include "types.h"
#include "ui.h"
#include "audio.h"

int Keyboard_note_to_index(char n) {
	switch (n) {
		case ALLEGRO_KEY_Q: return 0;
		case ALLEGRO_KEY_2: return 1;
		case ALLEGRO_KEY_W: return 2;
		case ALLEGRO_KEY_3: return 3;
		case ALLEGRO_KEY_E: return 4;
		case ALLEGRO_KEY_R: return 5;
		case ALLEGRO_KEY_5: return 6;
		case ALLEGRO_KEY_T: return 7;
		case ALLEGRO_KEY_6: return 8;
		case ALLEGRO_KEY_Y: return 9;
		case ALLEGRO_KEY_7: return 10;
		case ALLEGRO_KEY_U: return 11;
	}

	return -1;
}

char* Keyboard_note_to_char(int index) {
	switch (index) {
		case 0:  return "Q";
		case 1:  return "2";
		case 2:  return "W";
		case 3:  return "3";
		case 4:  return "E";
		case 5:  return "R";
		case 6:  return "5";
		case 7:  return "T";
		case 8:  return "6";
		case 9:  return "Y";
		case 10: return "7";
		case 11: return "U";
	}
	
	return "";
}

static inline bool Keyboard_is_black_key(int k) {
    return (k & 0x1) != (k > 4); // Logical xor
}

static inline ALLEGRO_COLOR Keyboard_color_for_key(Keyboard *kb, int k) {
    if (kb->key_pressed[k])
        return KEYBOARD_COLOR_PRESSED;
    else {
        if (Keyboard_is_black_key(k))
            return UI_COLOR_GRAY;
        else
            return UI_COLOR_WHITE;
    }
}

void Keyboard_draw_object(Object *o, bool is_selected, UIPoint mouse, ALLEGRO_EVENT *ev) {
	Keyboard *kb = o->data;
	int note;
	bool is_hover_output = UI_is_point_near_io(mouse, o->ui, false);
	
	switch (ev->type) {
		case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN:
			if (UI_is_point_near_io(mouse, o->ui, false))
				UI_set_target(o, UITargetOutput);
			else
				UI_set_target(o, UITargetObject);
			break;
		case ALLEGRO_EVENT_KEY_DOWN:
		case ALLEGRO_EVENT_KEY_UP:
			note = Keyboard_note_to_index(ev->keyboard.keycode);
			if (note >= 0)
				kb->key_pressed[note] = (ev->type == ALLEGRO_EVENT_KEY_DOWN);
	}
	
	if (is_selected)
		UI_draw_border(UIRect_points(UIPoint_margin, o->ui->size), UI_COLOR_HIGHLIGHT);
	
	// Draw keys
	int key_width  = KEYBOARD_WIDTH  / 2;
	int key_height = KEYBOARD_HEIGHT / 7;
	
	UIRect key_rect = UIRect_points(UIPoint_margin, UIPoint_xy(KEYBOARD_WIDTH, key_height));
	
	// Draw white keys first
	for (int i = 11; i >= 0; i--) {
		if (Keyboard_is_black_key(i))
			continue;
		
        UI_draw_box(key_rect, Keyboard_color_for_key(kb, i));
		UI_draw_label(UIPoint_offset(key_rect.loc, key_width * 1.5f, 6), Keyboard_note_to_char(i));
		key_rect.loc.y += key_height;
	}
	
	// Then draw blacks over the top
	// (It would be faster to do it in order, but then the user wouldn't see half of the keys)
	key_rect.loc.y = UI_MARGIN + key_height/2;
	key_rect.size = UIPoint_xy(key_width, key_height);
	for (int i = 11; i >= 0; i--) {
		if (!Keyboard_is_black_key(i))
			continue;
		
        UI_draw_box(key_rect, Keyboard_color_for_key(kb, i));
		UI_draw_label(UIPoint_offset(key_rect.loc, key_width/2, 6), Keyboard_note_to_char(i));
		key_rect.loc.y += key_height;
		
		// Make sure to leave a gap where there's no black key
		if (!Keyboard_is_black_key(i-2))
			key_rect.loc.y += key_height;
	}
	
	if (is_hover_output)
		UI_draw_circle(UIPoint_add_margin(UI_io_point_for_drawable(o->ui, false)), UI_IO_SIZE*1.5, UI_COLOR_BLACK);

	// Draw IO nodes
	UI_draw_circle(UIPoint_add_margin(UI_io_point_for_drawable(o->ui, false)), UI_IO_SIZE, UI_COLOR_CONTROL);
}

void *Keyboard_alloc() {
	return calloc(1, sizeof(Keyboard));
}

void Keyboard_process(Object *o, AudioPhaseDataList *phase_data, unsigned long frames, float *buffer) {
	Keyboard *kb = o->data;

	for (int i = 0; i < 12; i++) {
		if (kb->key_pressed[i])
			Audio_enable_note(i);
		else
			Audio_disable_note(i);
	}
}

void Keyboard_init() {
	obj_set_type_params(
		TYPE_KEYBOARD,
		"Keyboard",
		&Keyboard_process,
		&Keyboard_alloc,
		&Keyboard_draw_object,
		UIPoint_xy(KEYBOARD_WIDTH, KEYBOARD_HEIGHT),
		IONone,
		IOControl
	);
}
