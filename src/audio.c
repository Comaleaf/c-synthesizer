#include "audio.h"

#include "object.h"
#include <portaudio.h>
#include <math.h>

// Private functions

static int Audio_pa_buffer_callback(const void *, void *, unsigned long, const PaStreamCallbackTimeInfo*,
                                    PaStreamCallbackFlags, void *);

// Private properties

AudioPhaseDataList *audio_phase_data = NULL;
PaStream *audio_stream = NULL;
Object   *audio_source = NULL;

float audio_buffer[AUDIO_BUFFER_SIZE*2] = {0};

// Implementation

void Audio_set_source(struct Object *o) {
	audio_source = o;
}

void Audio_init() {
    Pa_Initialize();
    
    PaStreamParameters params;
    
    params.device = Pa_GetDefaultOutputDevice();
    params.channelCount = 2; // Stereo
    params.sampleFormat = paFloat32;
    params.suggestedLatency = Pa_GetDeviceInfo(params.device)->defaultLowOutputLatency;
    params.hostApiSpecificStreamInfo = NULL;
    
    Pa_OpenStream(
        &audio_stream,
        NULL, // No input
        &params,
        AUDIO_SAMPLE_RATE,
        AUDIO_BUFFER_SIZE,
        paClipOff,
        Audio_pa_buffer_callback,
        &audio_phase_data);
	
	audio_phase_data = NULL;
	audio_source = NULL;
	
    Pa_StartStream(audio_stream);
}

void Audio_close() {
    Pa_StopStream(audio_stream);
}

void Audio_enable_note(int n) {
	AudioPhaseDataList **phase_data = &audio_phase_data;

	while (*phase_data != NULL) {
		if ((*phase_data)->tag == n) {
			(*phase_data)->time = 0;
			(*phase_data)->release_time = 0;
			return; // Already enabled
		}
		else
			phase_data = &((*phase_data)->next);
	}

	AudioPhaseDataList *new_phase = calloc(1, sizeof(AudioPhaseDataList));
	new_phase->tag = n;
	new_phase->increment = (440.0f * pow(1.059463094359, n-9)) / (float)AUDIO_SAMPLE_RATE; // -9 for middle C (440Hz is A above)

	*phase_data = new_phase;
}

void Audio_disable_note(int n) {
	if (!audio_phase_data) return;
	
	AudioPhaseDataList **phase_data = &audio_phase_data;
	AudioPhaseDataList *node_to_delete;

	while (phase_data != NULL && *phase_data != NULL) {
		if ((*phase_data)->tag == n) {
			// Found it, delete it
			node_to_delete = *phase_data;
			*phase_data = node_to_delete->next;
			free(node_to_delete);
			//(*phase_data)->release_time = 1;
			return;
		}
		else
			phase_data = &((*phase_data)->next); // Look at next item
	}
}

float AudioPhaseDataList_get_value(float phase, float (*wavetable)[]) {
	// Calculate the offset into the wavetable
    float offset = phase * AUDIO_WAVETABLE_SIZE;
	// Get the natural part of the offset (which can be used as an index)
    int index = (int) offset;
	// Get the rational remainder of the offset
    float remainder = offset - index;
	// Interpolate between two points, proportional to the remainder
    float lower = (*wavetable)[index];
    float upper = (*wavetable)[index+1];
    return lower + remainder*(upper-lower);
}

/*
#define AUDIO_SUSTAIN      (0.8f)
#define AUDIO_ATTACK       (1.0f)
#define AUDIO_ATTACK_TIME  (2000)
#define AUDIO_DECAY_TIME   (5000)
#define AUDIO_RELEASE_TIME (2000)

float Audio_ADSR(int time, int release_time, float data) {
	if (release_time > 0) {
		if (release_time < AUDIO_RELEASE_TIME)
			return (data * AUDIO_SUSTAIN) * ( (AUDIO_RELEASE_TIME - release_time)/(float)AUDIO_RELEASE_TIME );
		else 
			return .0f;
	}
	else {
		if (time < AUDIO_ATTACK_TIME)
			return data * AUDIO_ATTACK * ((float)time/(float)AUDIO_ATTACK_TIME);
		else if (time < AUDIO_ATTACK_TIME+AUDIO_DECAY_TIME)
			return data * (AUDIO_ATTACK - (AUDIO_ATTACK-AUDIO_SUSTAIN) * (time-AUDIO_ATTACK_TIME)/AUDIO_DECAY_TIME);
		else
			return data * AUDIO_SUSTAIN;
	}
}*/



static int Audio_pa_buffer_callback(const void *input, void *output, unsigned long frames,
                                    const PaStreamCallbackTimeInfo* time, PaStreamCallbackFlags flags, void *data) {
	Object *o = audio_source;

    while (o != NULL) {
		obj_get_process(o->type)(o, audio_phase_data, frames, audio_buffer);
		o = o->next;
	}

	// Output buffer contents
	float *out = output;
	for (int i = 0; i < frames; i++) {
		*out++ = audio_buffer[(i<<1)];
		*out++ = audio_buffer[(i<<1) + 1];
	}
	
	// Clear the buffer for next time
	memset(audio_buffer, 0, AUDIO_BUFFER_SIZE*2);
	
	return paContinue;
}

