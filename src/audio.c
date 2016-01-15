#include "audio.h"

#include <amio_lib.h>
#include <math.h>

AudioBufferData _data;
AUDIOSTREAM _stream;

static int _Audio_pa_buffer_callback(const void *inputBuffer,
                                     void *outputBuffer,
                                     unsigned long framesPerBuffer,
                                     const PaStreamCallbackTimeInfo* timeInfo,
                                     PaStreamCallbackFlags statusFlags,
                                     void *userData);

void Audio_init() {
    // Initialise sinusoidal wavetable
	for (int i = 0; i < AUDIO_WAVETABLE_SIZE; i++) {
		_data.sine[i] = 0.90f * (float) sin( ((double)i/(double)AUDIO_WAVETABLE_SIZE) * M_PI * 2. );
	}

	_data.sine[AUDIO_WAVETABLE_SIZE] = _data.sine[0]; /* set guard point. */
	_data.phase_data = NULL;

    _stream.stream          = NULL; // Will get value after openDefaultAudioStream call
    _stream.sampleRate      = AUDIO_SAMPLE_RATE; // Stream sample rate
    _stream.sampleFormat    = paFloat32; // Stream sample format (float 32-bit in this case)
    _stream.inChannels      = 0; // Output-only stream
    _stream.outChannels     = 2; // Stereo 
    _stream.framesPerBuffer = AUDIO_BUFFER_SIZE; // Number of frames of the buffers in the processing callback function

    initialiseAudioSystem();
	openDefaultAudioStream(&_stream, _Audio_pa_buffer_callback, &_data);

	startAudioStream(&_stream);
}

void Audio_close() {
	stopAudioStream(&_stream);
}

void Audio_enable_note(int n) {
	_AudioLinkedPhase **phase_data = &(_data.phase_data);

	while (*phase_data != NULL) {
		if ((*phase_data)->tag == n) {
			(*phase_data)->time = 0;
			(*phase_data)->release_time = 0;
			return; // Already enabled
		}
		else
			phase_data = &((*phase_data)->next);
	}

	_AudioLinkedPhase *new_phase = calloc(1, sizeof(_AudioLinkedPhase));
	new_phase->tag = n;
	new_phase->increment = (440.0f * pow(1.059463094359, n)) / (float)AUDIO_SAMPLE_RATE;

	*phase_data = new_phase;
}

void Audio_disable_note(int n) {
	_AudioLinkedPhase **phase_data = &(_data.phase_data);
	//_AudioLinkedPhase *node_to_delete;

	while (*phase_data != NULL) {
		if ((*phase_data)->tag == n) {
		/*
			// Found it, delete it
			node_to_delete = *phase_data;
			*phase_data = node_to_delete->next;
			free(node_to_delete);*/
			(*phase_data)->release_time = 1;
			return;
		}
		else
			phase_data = &((*phase_data)->next); // Look at next item
	}
}

float AudioBufferData_get_value(AudioBufferData *data, float phase) {
	// Calculate the offset into the wavetable
    float offset = phase * AUDIO_WAVETABLE_SIZE;
	// Get the natural part of the offset (which can be used as an index)
    int index = (int) offset;
	// Get the rational remainder of the offset
    float remainder = offset - index;
	// Interpolate between two points, proportional to the remainder
    float lower = data->sine[index];
    float upper = data->sine[index+1];
    return lower + remainder*(upper-lower);
}

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
}

static int _Audio_pa_buffer_callback(const void *inputBuffer, void *outputBuffer,
                                     unsigned long framesPerBuffer,
                                     const PaStreamCallbackTimeInfo* timeInfo,
                                     PaStreamCallbackFlags statusFlags,
                                     void *userData) {
    AudioBufferData *data = (AudioBufferData*)userData;
    float *out = (float*)outputBuffer;

    (void) timeInfo; // Prevent unused variable warnings.
    (void) statusFlags;
    (void) inputBuffer;

	// Sum phase data
	_AudioLinkedPhase *phase_data;
	float out_left, out_right;

    for(int i=0; i<framesPerBuffer; i++) {
		phase_data = data->phase_data;
		out_left = out_right = 0.0;

		while (phase_data != NULL) {
			out_left  += Audio_ADSR(phase_data->time, phase_data->release_time, AudioBufferData_get_value(data, phase_data->left));  /* left */
			out_right += Audio_ADSR(phase_data->time, phase_data->release_time, AudioBufferData_get_value(data, phase_data->right));  /* right */

			phase_data->left  += phase_data->increment;
			phase_data->right += phase_data->increment;// * 1.5f; /* fifth above */
			phase_data->time++;

			if (phase_data->release_time > 0)
				phase_data->release_time++;

			if (phase_data->left >= 1.0f)  phase_data->left  -= 1.0f;
			if (phase_data->right >= 1.0f) phase_data->right -= 1.0f;

			phase_data = phase_data->next;
		}

		*out++ = out_left;
		*out++ = out_right;
    }

    return paContinue;
}

