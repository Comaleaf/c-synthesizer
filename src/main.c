#include "audio.h"
#include "ui.h"
#include "keyboard.h"
#include "oscillator.h"
#include "lfo.h"

#include <stdlib.h>
#include <stdio.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>

int main(void) {

	if (!al_init() || !al_install_mouse() || !al_install_keyboard() ||
	    !al_init_primitives_addon()) {
		fprintf(stderr, "Could not initialise Allegro library.\n");
		return EXIT_FAILURE;
	}

	// Initialise different modules (you need to do this before they are used or you will segfault)
	Audio_init();
	Keyboard_init();
	Oscillator_init();
	LFO_init();
	UI_init();
	
	while (UI_next());
	
	UI_close();
	Audio_close();

	al_uninstall_mouse();
	al_uninstall_keyboard();

	return EXIT_SUCCESS;
}

