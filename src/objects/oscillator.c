#include "oscillator.h"

#include <stdio.h>
#include <math.h>
#include "object.h"
#include "types.h"
#include "ui.h"
#include "audio.h"

float osc_sine   [AUDIO_WAVETABLE_SIZE+1];
float osc_square [AUDIO_WAVETABLE_SIZE+1];
float osc_saw    [AUDIO_WAVETABLE_SIZE+1];

UIRect osc_type_rect;

void Oscillator_draw_object(Object *o, bool is_selected, UIPoint mouse, ALLEGRO_EVENT *ev) {
	Oscillator *osc = o->data;

	char attack[20], decay[20], sustain[20], release[20], type[20];

	sprintf(attack,  "Attack: %.2f",  osc->attack);
	sprintf(decay,   "Decay: %.2f",   osc->decay);
	sprintf(sustain, "Sustain: %.2f", osc->sustain);
	sprintf(release, "Release: %.2f", osc->release);
    
    switch (osc->shape) {
        case Sine:   strcpy(type, "Type: Sine");   break;
        case Saw:    strcpy(type, "Type: Saw");    break;
        case Square: strcpy(type, "Type: Square"); break;
    }

	if (is_selected)
		UI_draw_border(UIRect_points(UIPoint_margin, o->ui->size), UI_COLOR_HIGHLIGHT);
	
	UI_draw_box(UIRect_points(UIPoint_margin, o->ui->size), UI_COLOR_WHITE);
	
	bool is_clicking = ev->type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN;
	
	if (UI_is_point_near_io(mouse, o->ui, true)) {
		UI_draw_circle(UIPoint_add(UIPoint_margin, UI_io_point_for_drawable(o->ui, true)), UI_IO_SIZE*1.5, UI_COLOR_BLACK);
		if (is_clicking) UI_set_target(o, UITargetInput);
	}
	else if (UI_is_point_near_io(mouse, o->ui, false)) {
		UI_draw_circle(UIPoint_add(UIPoint_margin, UI_io_point_for_drawable(o->ui, false)), UI_IO_SIZE*1.5, UI_COLOR_BLACK);
		if (is_clicking)
			UI_set_target(o, UITargetOutput);
	}
	else if (UI_is_point_within_rect(mouse, osc_type_rect)) {
        UI_draw_box(osc_type_rect, UI_COLOR_HIGHLIGHT);
		if (is_clicking) osc->shape = (osc->shape + 1) % 3;
	}
	else {
		if (is_clicking) UI_set_target(o, UITargetObject);
	}
	
	UI_draw_title(UIPoint_offset(UIPoint_margin, OSC_WIDTH/2, 10),  "Oscillator");
	UI_draw_label(UIPoint_offset(UIPoint_margin, OSC_WIDTH/2, 70),  type);
	UI_draw_label(UIPoint_offset(UIPoint_margin, OSC_WIDTH/2, 100), attack);
	UI_draw_label(UIPoint_offset(UIPoint_margin, OSC_WIDTH/2, 130), decay);
	UI_draw_label(UIPoint_offset(UIPoint_margin, OSC_WIDTH/2, 160), sustain);
	UI_draw_label(UIPoint_offset(UIPoint_margin, OSC_WIDTH/2, 190), release);
	
	// Draw IO nodes
	UI_draw_circle(UIPoint_add(UIPoint_margin, UI_io_point_for_drawable(o->ui, true)),  UI_IO_SIZE, UI_COLOR_CONTROL);
	UI_draw_circle(UIPoint_add(UIPoint_margin, UI_io_point_for_drawable(o->ui, false)), UI_IO_SIZE, UI_COLOR_AUDIO);
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

void Oscillator_process(Object *o, AudioPhaseDataList *phase_data_head, unsigned long frames, float *buffer) {
    Oscillator *osc = o->data;
    float (*wavetable)[]; // Wavetable to use
    
    switch (osc->shape) {
        case Sine:   wavetable = &osc_sine;   break;
        case Saw:    wavetable = &osc_saw;    break;
        case Square: wavetable = &osc_square; break;
    }
    
	AudioPhaseDataList *phase_data;

	// Sum phase data
	float out_left, out_right;

    for (int i = 0; i < frames; i++) {
		phase_data = phase_data_head;
		out_left = out_right = 0.0;

		while (phase_data != NULL) {
			out_left  += AudioPhaseDataList_get_value(phase_data->left,  wavetable);
			out_right += AudioPhaseDataList_get_value(phase_data->right, wavetable);

			phase_data->left  += phase_data->increment;
			phase_data->right += phase_data->increment;// * 1.5f; /* fifth above */
			phase_data->time++;

			if (phase_data->release_time > 0)
				phase_data->release_time++;

			if (phase_data->left >= 1.0f)  phase_data->left  -= 1.0f;
			if (phase_data->right >= 1.0f) phase_data->right -= 1.0f;

			phase_data = phase_data->next;
		}

		*buffer++ = out_left;
		*buffer++ = out_right;
    }
}

void Oscillator_init() {
    // UI components
    osc_type_rect = UIRect_xy(UI_MARGIN, UI_MARGIN+70, OSC_WIDTH, 35);
    
    // Initialise wavetables
	for (int i = 0; i < AUDIO_WAVETABLE_SIZE; i++) {
		osc_sine[i]   = 0.9f * (float) sin( ((double)i/(double)AUDIO_WAVETABLE_SIZE) * M_PI * 2. );
        osc_saw[i]    = 0.9f * (((double)i/(double)AUDIO_WAVETABLE_SIZE) - floor(((double)i/(double)AUDIO_WAVETABLE_SIZE)));
        osc_square[i] = signbit(osc_sine[i]) ? 0.9f : -0.9f;
	}

    osc_sine[AUDIO_WAVETABLE_SIZE] = osc_saw[AUDIO_WAVETABLE_SIZE] = osc_square[AUDIO_WAVETABLE_SIZE] = 0;
    
    // Set-up module
	obj_set_type_params(
		TYPE_OSCILLATOR,
		&Oscillator_process,
		&Oscillator_alloc,
		&Oscillator_draw_object,
		UIPoint_xy(OSC_WIDTH, OSC_HEIGHT),
		IOControl,
		IOPCM
	);
}
