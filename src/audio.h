#ifndef AUDIO_H_
#define AUDIO_H_

#include <portaudio.h>

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

typedef struct AudioPhaseDataList {
	int   tag;
	int   time;
	int   release_time;
	float increment;
	float left;
	float right;
	struct AudioPhaseDataList *next;
} AudioPhaseDataList;

typedef void (*AudioProcessCallback)(struct Object *, AudioPhaseDataList *, unsigned long, float *);

// AudioPhaseDataList

float AudioPhaseDataList_get_value(float phase, float (*wavetable)[]);

#endif
