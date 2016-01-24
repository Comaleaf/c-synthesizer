#ifndef __OSCILLATOR_H_
#define __OSCILLATOR_H_

#define OSC_HEIGHT 240
#define OSC_WIDTH  240

typedef struct Oscillator {
	enum {Sine, Saw, Square} shape;
	float attack;
	float decay;
	float sustain;
	float release;
} Oscillator;

void Oscillator_init();

#endif
