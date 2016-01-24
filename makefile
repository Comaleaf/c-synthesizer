CC=clang
CFLAGS += -Wall -Werror -std=c99

INCLUDES=-Iinclude -Isrc -Isrc/objects
OBJ=assignment.o

GRAPHLIBS=-lallegro -lallegro_main -lallegro_primitives -lallegro_image -lallegro_font -lallegro_ttf
GRAPHIMPL=include/graphics_lib_functions.c
MUSLIBS=-lsndfile -lportaudio -lportmidi
MUSIMPL=include/amio_lib_functions.c

%.o: src/%.c .deps
	$(CC) $(CFLAGS) $(INCLUDES) $(GRAPHLIBS) $(MUSLIBS) -o build/$@ $(MUSIMPL) src/audio.c src/objects/object.c src/objects/oscillator.c src/objects/lfo.c src/objects/keyboard.c src/ui.c $< -MMD -MP -MF .deps/$*.d

clean:
	rm -f build/*

# subtle: directories are listed as changed when entries are
# created, leading to spurious rebuilds.
.deps/stamp:
	mkdir .deps && touch/stamp

# Include dependencies
-include .deps/*.d
