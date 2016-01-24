#ifndef __AUDIO_H_
#define __AUDIO_H_

#include <amio_lib.h>

#define AUDIO_BUFFER_SIZE    (128)
#define AUDIO_WAVETABLE_SIZE (256)
#define AUDIO_MIN_FREQ       (100.0f)
#define AUDIO_SAMPLE_RATE    (44100)

// Forward declarations
struct Object;

// Audio module

void Audio_init();
void Audio_close();
void Audio_enable_note(int n);
void Audio_disable_note(int n);
void Audio_set_source(struct Object *o);

// Types

typedef void (*AudioProcessCallback)(struct Object *o, const void *input_buffer, const void *output_buffer, unsigned long frames_per_buffer,
		                             const PaStreamCallbackTimeInfo* time_info, PaStreamCallbackFlags status_flags,
									 void *user_data);

typedef struct _AudioLinkedPhase {
	int tag;
	int time;
	int release_time;
	float increment;
	float left;
	float right;
	struct _AudioLinkedPhase *next;
} _AudioLinkedPhase;

typedef struct {
    float sine[AUDIO_WAVETABLE_SIZE+1];
	_AudioLinkedPhase *phase_data;
} AudioBufferData;

// AudioBufferData

float AudioBufferData_get_value(AudioBufferData *data, float phase);

#endif
