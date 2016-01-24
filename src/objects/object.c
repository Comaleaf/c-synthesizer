#include "object.h"
#include "ui.h"
#include "keyboard.h"
#include "oscillator.h"

// Private types / properties

typedef struct {
	AudioProcessCallback process;
	ObjectAllocCallback alloc;
	UIDrawCallback draw;
	UIEventMask event_mask;
	UISize size;
	IOType input;
	IOType output;
} ObjTypeParams;

ObjTypeParams obj_type_params[TYPE_COUNT];

void obj_set_type_params(ObjType t, AudioProcessCallback process, ObjectAllocCallback alloc, UIDrawCallback draw, UIEventMask event_mask, UISize size, IOType input, IOType output) {
	ObjTypeParams p = {process, alloc, draw, event_mask, size, input, output};
	obj_type_params[t] = p;
}

AudioProcessCallback obj_get_process    (ObjType t) { return obj_type_params[t].process; }
ObjectAllocCallback  obj_get_alloc      (ObjType t) { return obj_type_params[t].alloc; }
UIDrawCallback       obj_get_draw       (ObjType t) { return obj_type_params[t].draw; }
UIEventMask          obj_get_event_mask (ObjType t) { return obj_type_params[t].event_mask; }
UISize               obj_get_size       (ObjType t) { return obj_type_params[t].size; }
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
