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

void Keyboard_draw_object(Object *o, UIEventMask event_mask, UIPoint loc, ALLEGRO_EVENT *ev) {
	Keyboard *kb = o->data;

	if (event_mask & UI_Event_Mouse_Down) {
		UI_set_drag_object(o);
	}
	else if (event_mask & UI_Event_Key_Down) {
		int note = Keyboard_note_to_index(ev->keyboard.keycode);
		if (note >= 0) 
			kb->key_pressed[note] = true;
	}
	else if (event_mask & UI_Event_Key_Up) {
		int note = Keyboard_note_to_index(ev->keyboard.keycode);
		if (note >= 0) 
			kb->key_pressed[note] = false;
	}
	
	// Draw outline
	UI_draw_box(UI_BITMAP_BUFFER, UI_BITMAP_BUFFER, o->ui->size, UI_COLOR_WHITE);

	// Draw keys
	int key_width  = KEYBOARD_WIDTH  / 2;
	int key_height = KEYBOARD_HEIGHT / 7;
	UISize white_key_size = UISize_with_bounds(KEYBOARD_WIDTH, key_height);
	UISize black_key_size = UISize_with_bounds(key_width, key_height);

	UI_draw_box(UI_BITMAP_BUFFER, UI_BITMAP_BUFFER+ 0*key_height, white_key_size, KEYBOARD_COLOR_RESOLVE(11, kb, UI_COLOR_WHITE));
	UI_draw_box(UI_BITMAP_BUFFER, UI_BITMAP_BUFFER+ 1*key_height, white_key_size, KEYBOARD_COLOR_RESOLVE(9, kb, UI_COLOR_WHITE)); 
	UI_draw_box(UI_BITMAP_BUFFER, UI_BITMAP_BUFFER+ 2*key_height, white_key_size, KEYBOARD_COLOR_RESOLVE(7, kb, UI_COLOR_WHITE));
	UI_draw_box(UI_BITMAP_BUFFER, UI_BITMAP_BUFFER+ 3*key_height, white_key_size, KEYBOARD_COLOR_RESOLVE(5, kb, UI_COLOR_WHITE));
	UI_draw_box(UI_BITMAP_BUFFER, UI_BITMAP_BUFFER+ 4*key_height, white_key_size, KEYBOARD_COLOR_RESOLVE(4, kb, UI_COLOR_WHITE));
	UI_draw_box(UI_BITMAP_BUFFER, UI_BITMAP_BUFFER+ 5*key_height, white_key_size, KEYBOARD_COLOR_RESOLVE(2, kb, UI_COLOR_WHITE));
	UI_draw_box(UI_BITMAP_BUFFER, UI_BITMAP_BUFFER+ 6*key_height, white_key_size, KEYBOARD_COLOR_RESOLVE(0, kb, UI_COLOR_WHITE));

	UI_draw_box(UI_BITMAP_BUFFER, UI_BITMAP_BUFFER+ 0.5*key_height, black_key_size, KEYBOARD_COLOR_RESOLVE(10, kb, UI_COLOR_GRAY));
	UI_draw_box(UI_BITMAP_BUFFER, UI_BITMAP_BUFFER+ 1.5*key_height, black_key_size, KEYBOARD_COLOR_RESOLVE(8, kb, UI_COLOR_GRAY));
	UI_draw_box(UI_BITMAP_BUFFER, UI_BITMAP_BUFFER+ 2.5*key_height, black_key_size, KEYBOARD_COLOR_RESOLVE(6, kb, UI_COLOR_GRAY));
	UI_draw_box(UI_BITMAP_BUFFER, UI_BITMAP_BUFFER+ 4.5*key_height, black_key_size, KEYBOARD_COLOR_RESOLVE(3, kb, UI_COLOR_GRAY));
	UI_draw_box(UI_BITMAP_BUFFER, UI_BITMAP_BUFFER+ 5.5*key_height, black_key_size, KEYBOARD_COLOR_RESOLVE(1, kb, UI_COLOR_GRAY));

	UI_draw_label(UI_BITMAP_BUFFER+ key_width*1.5f, UI_BITMAP_BUFFER+5+ 6.f*key_height, "Q");
	UI_draw_label(UI_BITMAP_BUFFER+ key_width*0.5f, UI_BITMAP_BUFFER+5+ 5.5f*key_height, "2");
	UI_draw_label(UI_BITMAP_BUFFER+ key_width*1.5f, UI_BITMAP_BUFFER+5+ 5.f*key_height, "W");
	UI_draw_label(UI_BITMAP_BUFFER+ key_width*0.5f, UI_BITMAP_BUFFER+5+ 4.5f*key_height, "3");
	UI_draw_label(UI_BITMAP_BUFFER+ key_width*1.5f, UI_BITMAP_BUFFER+5+ 4.f*key_height, "E");
	UI_draw_label(UI_BITMAP_BUFFER+ key_width*1.5f, UI_BITMAP_BUFFER+5+ 3.f*key_height, "R");
	UI_draw_label(UI_BITMAP_BUFFER+ key_width*0.5f, UI_BITMAP_BUFFER+5+ 2.5f*key_height, "5");
	UI_draw_label(UI_BITMAP_BUFFER+ key_width*1.5f, UI_BITMAP_BUFFER+5+ 2.f*key_height, "T");
	UI_draw_label(UI_BITMAP_BUFFER+ key_width*0.5f, UI_BITMAP_BUFFER+5+ 1.5f*key_height, "6");
	UI_draw_label(UI_BITMAP_BUFFER+ key_width*1.5f, UI_BITMAP_BUFFER+5+ 1.f*key_height, "Y");
	UI_draw_label(UI_BITMAP_BUFFER+ key_width*0.5f, UI_BITMAP_BUFFER+5+ 0.5f*key_height, "7");
	UI_draw_label(UI_BITMAP_BUFFER+ key_width*1.5f, UI_BITMAP_BUFFER+5, "U");
}

void *Keyboard_alloc() {
	return calloc(1, sizeof(Keyboard));
}

void Keyboard_process(Object *o, const void *input_buffer, const void *output_buffer, unsigned long frames_per_buffer,
                      const PaStreamCallbackTimeInfo* time_info, PaStreamCallbackFlags status_flags, 
                      void *user_data) {
	Keyboard *kb = o->data;

	for (int i = 0; i < 12; i++) {
		if (kb->key_pressed[i]) {
			Audio_enable_note(i);
		}
		else
			Audio_disable_note(i);
	}
}

void Keyboard_init() {
	obj_set_type_params(
		TYPE_KEYBOARD,
		&Keyboard_process,
		&Keyboard_alloc,
		&Keyboard_draw_object,
		UI_Event_Key_Down | UI_Event_Key_Up | UI_Event_Mouse_Down | UI_Event_Mouse_Up,
		UISize_with_bounds(KEYBOARD_WIDTH, KEYBOARD_HEIGHT),
		IONone,
		IOControl
	);
}
