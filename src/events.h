#ifndef EVENTS_H__
#define EVENTS_H__

#include <allegro5/allegro.h>

typedef unsigned int EventType;

enum {
	EVENT_COUNT = 64 // Must be higher than Allegro event codes - (which end at 61 as of Allegro5) include/allegro5/events.h
};

void Event_notify(EventType t, ALLEGRO_EVENT *ev);

void Event_register_callback(EventType t, void (*f)(ALLEGRO_EVENT *ev));

#endif
