#ifndef __OBJECT_H_
#define __OBJECT_H_

#include <stdbool.h>
#include <allegro5/allegro.h>
#include "types.h"
#include "ui.h"
#include "audio.h"

typedef enum {
	IONone,
	IOControl,
	IOPCM
} IOType;

typedef void * (*ObjectAllocCallback)(void);

typedef struct Object {
	ObjType type;
	void *data;
	UIDrawable *ui;
	struct Object *next;
} Object;

// Object constructors

Object* Object_with(ObjType t);

// Modifiers
bool Object_connect(Object *o1, Object *o2, bool is_o1_receiver);

// ObjType/parameter constructors/accessors

void obj_set_type_params(ObjType t, AudioProcessCallback process, ObjectAllocCallback alloc, UIDrawCallback draw, UIPoint size, IOType input, IOType output);

AudioProcessCallback obj_get_process    (ObjType t);
ObjectAllocCallback  obj_get_alloc      (ObjType t);
UIDrawCallback       obj_get_draw       (ObjType t);
UIPoint              obj_get_size       (ObjType t);
IOType               obj_get_input      (ObjType t);
IOType               obj_get_output     (ObjType t);

#endif
