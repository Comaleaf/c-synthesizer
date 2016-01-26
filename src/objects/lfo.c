#include "lfo.h"

#include "math.h"
#include "object.h"
#include "types.h"
#include "ui.h"

void LFO_draw_object(Object *o, bool is_selected, UIPoint loc, ALLEGRO_EVENT *ev) {
	if (ev->type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
		UI_set_target(o, UITargetObject);
	}
	
	// Background
	UI_draw_circle(UIPoint_offset(UIPoint_margin, LFO_WIDTH/2, LFO_WIDTH/2), LFO_WIDTH/2, UI_COLOR_WHITE);
	
	// Labels
	UI_draw_title(UIPoint_offset(UIPoint_margin, LFO_WIDTH/2,  50), "LFO");
	UI_draw_label(UIPoint_offset(UIPoint_margin, LFO_WIDTH/2, 100), "Mod: Volume");
	
	// Draw IO nodes
	UI_draw_circle(UIPoint_add(UIPoint_margin, UI_io_point_for_drawable(o->ui, true)),  UI_IO_SIZE, UI_COLOR_AUDIO);
	UI_draw_circle(UIPoint_add(UIPoint_margin, UI_io_point_for_drawable(o->ui, false)), UI_IO_SIZE, UI_COLOR_AUDIO);
}

void *LFO_alloc() {
	LFO *lfo = malloc(sizeof(LFO));
	lfo->modulation = Volume;
	lfo->theta = 0.0;
	return lfo;
}

/*
static float cubic_distort(float input) {
	float output, temp;
	if( input < 0.0 )
		{
   temp = input + 1.0f;
   output = (temp * temp * temp) - 1.0f;
   }
	else
		{
   temp = input - 1.0f;
   output = (temp * temp * temp) + 1.0f;
			
   }
	return output;
}*/

#define LFO_FREQUENCY 5.0f

void LFO_process(Object *o, AudioPhaseDataList *phase_data_head, unsigned long frames, float *buffer) {
	LFO *lfo = o->data;
	
	float period = (float)AUDIO_SAMPLE_RATE / (float)LFO_FREQUENCY; // How many frames need to pass for one cycle
	float increment = (1.0/(float)frames) * M_PI * 2.0f * ((float)frames / period);
	
	for (int i = 0; i < frames; i++) {
		lfo->theta += increment;
		
		if (lfo->theta > (M_PI * 2.0))
			lfo->theta -= (M_PI * 2.0);
	
		buffer[(i<<1)]   *= (sinf(lfo->theta) + 1.0);
		buffer[(i<<1)+1] *= (sinf(lfo->theta) + 1.0);
	}
}

void LFO_init() {
	obj_set_type_params(
		TYPE_LFO,
		&LFO_process,
		&LFO_alloc,
		&LFO_draw_object,
		UIPoint_xy(LFO_WIDTH, LFO_HEIGHT),
		IOPCM,
		IOPCM
	);
}

