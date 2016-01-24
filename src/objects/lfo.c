#include "lfo.h"

#include "object.h"
#include "types.h"
#include "ui.h"

void LFO_draw_object(Object *o, UIEventMask event_mask, UIPoint loc, ALLEGRO_EVENT *ev) {
	if (event_mask & UI_Event_Mouse_Down) {
		//o->is_focused = true;
		UI_set_drag_object(o);
	}/*
	else if (event_mask & UI_Event_Mouse_Up)
		o->is_focused = false;*/

	UI_draw_ellipse(UI_BITMAP_BUFFER, UI_BITMAP_BUFFER, o->ui->size, UI_COLOR_WHITE); 
	UI_draw_title(UI_BITMAP_BUFFER + LFO_WIDTH/2, UI_BITMAP_BUFFER+50, "LFO");
	UI_draw_label(UI_BITMAP_BUFFER + LFO_WIDTH/2, UI_BITMAP_BUFFER+100, "Mod: Volume");
}

void *LFO_alloc() {
	return NULL;
}

void LFO_process(Object *o, const void *input_buffer, const void *output_buffer, unsigned long frames_per_buffer,
                 const PaStreamCallbackTimeInfo* time_info, PaStreamCallbackFlags status_flags, 
                 void *user_data) {
	// do nothing;
}

void LFO_init() {
	obj_set_type_params(
		TYPE_LFO,
		&LFO_process,
		&LFO_alloc,
		&LFO_draw_object,
		UI_Event_Mouse_Down | UI_Event_Mouse_Up,
		UISize_with_bounds(LFO_WIDTH, LFO_HEIGHT),
		IOPCM,
		IOPCM
	);
}

