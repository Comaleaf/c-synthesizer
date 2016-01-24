#include "events.h"

typedef struct EventHandlerList {
	void (*function)(ALLEGRO_EVENT *e);
	struct EventHandlerList *next;
} EventHandlerList;

EventHandlerList* handlers[EVENT_COUNT];

void Event_notify(EventType t, ALLEGRO_EVENT *ev) {
	EventHandlerList *handler = handlers[t];

	// Invoke the callback then continue to the next cell in the list
	while (handler != NULL) {
		handler->function(ev);
		handler = handler->next;
	}
}

void Event_register_callback(EventType t, void (*f)(ALLEGRO_EVENT *ev)) {
	// Get the head of the list for this particular event type
	EventHandlerList **head = &handlers[t];
	
	// Find the end of the list
	while (*head != NULL)
		*head = (*head).next;

	// Create a new cell
	*head = malloc(sizeof(EventHandlerList);
	(*head)->function = f;
	(*head)->next = NULL;
}
