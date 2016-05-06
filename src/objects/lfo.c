#include "lfo.h"

#include "math.h"
#include "object.h"
#include "types.h"
#include "ui.h"

void LFO_draw_object(Object *o, bool is_selected, UIPoint mouse, ALLEGRO_EVENT *ev) {
	// Determine the state of user interaction with the object
	bool is_clicking     = ev->type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN;
	bool is_hover_input  = UI_is_point_near_io(mouse, o->ui, true);
	bool is_hover_output = !is_hover_input && UI_is_point_near_io(mouse, o->ui, false);
	
	// Draw selection border if necessary
	if (is_selected)
		UI_draw_border(UIRect_points(UIPoint_margin, o->ui->size), UI_COLOR_HIGHLIGHT);
	
	// Background
	UI_draw_circle(UIPoint_offset_margin(LFO_WIDTH/2, LFO_WIDTH/2), LFO_WIDTH/2, UI_COLOR_WHITE);
	
	// Labels
	UI_draw_title(UIPoint_offset_margin(LFO_WIDTH/2,  50), ObjParam(o, name));
	UI_draw_label(UIPoint_offset_margin(LFO_WIDTH/2, 100), "Mod: Volume");
	
	// Dispatch necessary actions on click events
	if (is_clicking) {
		if      (is_hover_input)  UI_set_target(o, UITargetInput);
		else if (is_hover_output) UI_set_target(o, UITargetOutput);
		else                      UI_set_target(o, UITargetObject);
	}

	// Update UI in response to hover events
	if (is_hover_input)
		UI_draw_circle(UIPoint_add_margin(UI_io_point_for_drawable(o->ui, true)), UI_IO_SIZE*1.5, UI_COLOR_BLACK);
	else if (is_hover_output)
		UI_draw_circle(UIPoint_add_margin(UI_io_point_for_drawable(o->ui, false)), UI_IO_SIZE*1.5, UI_COLOR_BLACK);
	
	//UI_draw_io(o)
	// Draw IO nodes
	UI_draw_circle(UIPoint_add_margin(UI_io_point_for_drawable(o->ui, true)),  UI_IO_SIZE, UI_COLOR_AUDIO);
	UI_draw_circle(UIPoint_add_margin(UI_io_point_for_drawable(o->ui, false)), UI_IO_SIZE, UI_COLOR_AUDIO);
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
		"LFO",
		&LFO_process,
		&LFO_alloc,
		&LFO_draw_object,
		UIPoint_xy(LFO_WIDTH, LFO_HEIGHT),
		IOPCM,
		IOPCM
	);
}

