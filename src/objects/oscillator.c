#include "oscillator.h"

#include <stdio.h>
#include "object.h"
#include "types.h"
#include "ui.h"

void Oscillator_draw_object(Object *o, UIEventMask event_mask, UIPoint loc, ALLEGRO_EVENT *ev) {
	Oscillator *osc = o->data;

	if (event_mask & UI_Event_Mouse_Down) {
		UI_set_drag_object(o);
	}

	char attack[20], decay[20], sustain[20], release[20];

	sprintf(attack, "Attack: %.2f", osc->attack);
	sprintf(decay, "Decay: %.2f", osc->decay);
	sprintf(sustain, "Sustain: %.2f", osc->sustain);
	sprintf(release, "Release: %.2f", osc->release);

	UI_draw_box(UI_BITMAP_BUFFER, UI_BITMAP_BUFFER, o->ui->size, o->ui->is_focused ? UI_COLOR_HIGHLIGHT : UI_COLOR_WHITE); 
	UI_draw_title(UI_BITMAP_BUFFER + OSC_WIDTH/2, UI_BITMAP_BUFFER+10, "Oscillator");
	UI_draw_label(UI_BITMAP_BUFFER + OSC_WIDTH/2, UI_BITMAP_BUFFER+70, "Type: Sine");
	UI_draw_label(UI_BITMAP_BUFFER + OSC_WIDTH/2, UI_BITMAP_BUFFER+100, attack);
	UI_draw_label(UI_BITMAP_BUFFER + OSC_WIDTH/2, UI_BITMAP_BUFFER+130, decay);
	UI_draw_label(UI_BITMAP_BUFFER + OSC_WIDTH/2, UI_BITMAP_BUFFER+160, sustain);
	UI_draw_label(UI_BITMAP_BUFFER + OSC_WIDTH/2, UI_BITMAP_BUFFER+190, release);
}

void *Oscillator_alloc() {
	Oscillator *o = calloc(1, sizeof(Oscillator));
	o->shape   = Sine;
	o->attack  = 1.0;
	o->decay   = 1.0;
	o->sustain = 1.0;
	o->release = 1.0;
	return o;
}

void Oscillator_process(Object *o, const void *input_buffer, const void *output_buffer, unsigned long frames_per_buffer,
                        const PaStreamCallbackTimeInfo* time_info, PaStreamCallbackFlags status_flags, 
                        void *user_data) {
	// do nothing;
}

void Oscillator_init() {
	obj_set_type_params(
		TYPE_OSCILLATOR,
		&Oscillator_process,
		&Oscillator_alloc,
		&Oscillator_draw_object,
		UI_Event_Mouse_Down | UI_Event_Mouse_Up,
		UISize_with_bounds(OSC_WIDTH, OSC_HEIGHT),
		IOControl,
		IOPCM
	);
}
