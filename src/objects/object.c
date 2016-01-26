#include "object.h"
#include "ui.h"
#include "keyboard.h"
#include "oscillator.h"

// Private types / properties

typedef struct {
	AudioProcessCallback process;
	ObjectAllocCallback alloc;
	UIDrawCallback draw;
	UIPoint size;
	IOType input;
	IOType output;
} ObjTypeParams;

ObjTypeParams obj_type_params[TYPE_COUNT];

void obj_set_type_params(ObjType t, AudioProcessCallback process, ObjectAllocCallback alloc, UIDrawCallback draw, UIPoint size, IOType input, IOType output) {
	ObjTypeParams p = {process, alloc, draw, size, input, output};
	obj_type_params[t] = p;
}

AudioProcessCallback obj_get_process    (ObjType t) { return obj_type_params[t].process; }
ObjectAllocCallback  obj_get_alloc      (ObjType t) { return obj_type_params[t].alloc; }
UIDrawCallback       obj_get_draw       (ObjType t) { return obj_type_params[t].draw; }
UIPoint              obj_get_size       (ObjType t) { return obj_type_params[t].size; }
IOType               obj_get_input      (ObjType t) { return obj_type_params[t].input; }
IOType               obj_get_output     (ObjType t) { return obj_type_params[t].output; }

Object* Object_with(ObjType t) {
	Object *o = malloc(sizeof(Object));

	o->next = NULL;
	o->type = t;
	o->ui   = NULL;
	o->data = obj_get_alloc(t)();

	return o;
}

bool Object_connect(Object *o1, Object *o2, bool is_o1_receiver) {
	Object *receiver, *emitter;
	
	// Re-order depending on what we're given
	receiver = is_o1_receiver ? o1 : o2;
	emitter  = is_o1_receiver ? o2 : o1;
	
	// Make sure they're not the same
	if (receiver == emitter)
		return false;
	
	// Make sure the IOTypes are compatible
	if (obj_get_input(receiver->type) != obj_get_output(emitter->type))
		return false;
	
	// Looks fine, connect them
	emitter->next = receiver;
	
	return true;
}