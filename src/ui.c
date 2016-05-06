#include "ui.h"

#include <ctype.h>
#include <math.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>

#include "object.h"
#include "types.h"
#include "audio.h"
#include "keyboard.h"

// Private Properties

ALLEGRO_DISPLAY *ui_display = NULL;
ALLEGRO_EVENT_QUEUE *ui_event_queue = NULL;

UIPoint UIPoint_margin = {UI_MARGIN, UI_MARGIN};
UIPoint ui_mouse; // Cursor position

UIRect ui_toolbar[TYPE_COUNT];

// Object being manipulated (dragged)
struct {
	UIDrawable  *item;
    bool         is_selected;
	UIPoint      offset;
    UITargetType type;
} ui_target;

// Items on the canvas
UIDrawable *ui_items[UI_MAX_OBJECTS];

void UI_add_object(Object *o, UIPoint loc) {
	// Find the first free space in ui_items
	size_t i = 0;
	while (ui_items[i] != NULL) i++;
	
	// Insert our object in this position, as a UIDrawable with the given location
	ui_items[i] = UIDrawable_with(o, loc);
	o->ui = ui_items[i]; // Make sure there is a circular reference back to the UIDrawable
	
	// If our object is a source (i.e. it has no input nodes), then notify audio module that
	// there is a new source
	if (obj_get_input(o->type) == IONone)
		Audio_set_source(o);
}

void UI_remove_object(Object *o) {
	// More complicated.. need to reorder the draw list slightly so that it will
	// draw all the items when this is removed.
	// The strategy for doing that efficiently is just to find the last item in the list and then
	// swap it with this one before deleting
	// We're also going to need to scan the list for any item that emits into this one, and remove
	// the reference
	// And finally we're going to need to search for replacement control sources if this item is the
	// head of the audio chain
	
	// Reset source
	Audio_set_source(NULL);
	
	size_t current_idx = 0; // Where the object currently sits in the array
	size_t i; // Put this outside the for-loop so we can retain the reference to find where the last item is
	for (i = 0; ui_items[i] != NULL; i++) {
		// Find where the object is at the moment
		if (ui_items[i]->object == o) {
			current_idx = i;
			continue;
		}
		
		// Clear any references to this item
		if (ui_items[i]->object->next == o)
			ui_items[i]->object->next = NULL;
		
		// Check if it's a source and make it the head source if it is
		if (obj_get_input(ui_items[i]->object->type) == IONone)
			Audio_set_source(ui_items[i]->object);
	}
	
	// Free all the data relating to the object
	free(o->data); // First the module field
	free(o); // Then the object
	al_destroy_bitmap(ui_items[current_idx]->bitmap);
	free(ui_items[current_idx]); // Then the drawable
	ui_items[current_idx] = NULL;
	
	// Now move the item at the end of the list to the location occupied by the previous object
	UIDrawable *last = ui_items[i-1];
	ui_items[i-1] = NULL;
	ui_items[current_idx] = last;
}

void UI_init() {
	
    ui_display = al_create_display(UI_WIDTH, UI_HEIGHT);
	ui_event_queue = al_create_event_queue();

	al_register_event_source(ui_event_queue, al_get_display_event_source(ui_display));
	al_register_event_source(ui_event_queue, al_get_mouse_event_source());
	al_register_event_source(ui_event_queue, al_get_keyboard_event_source());

	al_init_ttf_addon();

	UI_add_object(Object_with(TYPE_KEYBOARD),   UIPoint_xy(100, 100));
	UI_add_object(Object_with(TYPE_OSCILLATOR), UIPoint_xy(300, 300));
	UI_add_object(Object_with(TYPE_LFO),        UIPoint_xy(300, 700));

	ui_items[0]->object->next = ui_items[1]->object;
	ui_items[1]->object->next = ui_items[2]->object;
}

void UI_close() {
	al_destroy_display(ui_display);
}

void UI_set_target(struct Object *o, UITargetType target_type) {
	ui_target.item = o->ui;
	ui_target.is_selected = true;
    ui_target.type   = target_type;
	
	if (target_type == UITargetObject)
		ui_target.offset = UIPoint_subtract(o->ui->loc, ui_mouse);
	
	else {
		ui_target.offset = UIPoint_add(o->ui->loc, UI_io_point_for_drawable(o->ui, target_type == UITargetInput));
		
		// If it's an input, clear any node that connects to this input
		if (target_type == UITargetInput) {
			for (size_t i = 0; ui_items[i] != NULL; i++) {
				if (ui_items[i]->object->next == o)
					ui_items[i]->object->next = NULL;
			}
		}
		else {
			// It's an output, so clear any node that this connects to
			o->next = NULL;
		}
	}
}

static void UI_update_bitmap_for_drawable(UIDrawable *d, ALLEGRO_EVENT *ev) {
	al_set_target_bitmap(d->bitmap);
	al_clear_to_color(UI_COLOR_ALPHA); // Clear to transparent background
	obj_get_draw(d->object->type)(
        d->object,
		d == ui_target.item && ui_target.is_selected && ui_target.type == UITargetObject,
        UIPoint_subtract(ui_mouse, UIPoint_subtract(d->loc, UIPoint_margin)),
        ev
    );
}

bool UI_next() {
	static ALLEGRO_EVENT ev = {0};
	//static UIDrawable *focused = NULL;
	static bool is_mouse_event = false;

	// Clear before updating
	al_set_target_bitmap(al_get_backbuffer(ui_display));
	al_clear_to_color(al_map_rgb(0xDD, 0xDD, 0xDD));

	// Update all objects
	for (size_t i = 0; ui_items[i] != NULL; i++) {
		// Some special actions need to happen for mouse events as they can cause items to be
		// moved around the screen.
		if (is_mouse_event) {
    		// Determine if item should be the current target
    		if (ui_target.item == NULL) {
				if (UI_is_point_within_drawable(ui_mouse, ui_items[i])) {
    				// It's the item under the cursor
    				ui_target.item = ui_items[i];
    				ui_target.type = UITargetObject;
    				ui_target.is_selected = false; // Only the actual object can determine whether it is selected (whether it can be dragged) - this is so that it can manage its own click events for controls on the object
    			}
    		}
			// Otherwise consider the case where it already is the current target
			else if (ui_target.type == UITargetObject && ui_target.item == ui_items[i]) {
				// Is the item being dragged?
				if (ui_target.is_selected) {
					// It is, so it either needs to be moved or dropped
					if (ev.type == ALLEGRO_EVENT_MOUSE_AXES)
						ui_target.item->loc = UIPoint_offset(ui_target.offset, ui_mouse.x, ui_mouse.y);
					else {
						ui_target.is_selected = false; // Drop the item
						ev.type = 0; // Stop event propagating to object, otherwise it'll just reselect the item
					}
				}
				
				// If the mouse is no longer within target bounds, we need to clear the target
				if (!UI_is_point_within_drawable(ui_mouse, ui_items[i]))
					ui_target.item = NULL;
			}
			// Finally, consider the case where the IO nodes are being dragged
			else if (ui_target.type != UITargetObject) {
				if (ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN &&
					UI_is_point_within_drawable(ui_mouse, ui_items[i])) {
					// The user is clicking on a particular object -- adjust the IO chain and release the target if succesful
					if (Object_connect(ui_target.item->object, ui_items[i]->object, ui_target.type == UITargetInput))
						ui_target.item = NULL;
				}
			}
			
    		// Can skip updating this if it's not the target, as it won't have changed on a mouse event
    		if (ui_items[i] != ui_target.item || ui_target.type != UITargetObject)
    			continue;
		}
		
		// Update bitmap for item
		UI_update_bitmap_for_drawable(ui_items[i], &ev);
	}
	
	// Draw Phase
	al_set_target_bitmap(al_get_backbuffer(ui_display)); // Go back to drawing to the display

	// Now draw each bitmap and connections
	for (size_t i = 0; ui_items[i] != NULL; i++) {
		// Draw IO edges between object and next objects
		// Don't bother drawing the edge if there's no successor node for the object
		if (ui_items[i]->object->next != NULL) {
        	UI_draw_line(
    			UIPoint_add(ui_items[i]->loc, UI_io_point_for_drawable(ui_items[i], false)),
    			UIPoint_add(ui_items[i]->object->next->ui->loc, UI_io_point_for_drawable(ui_items[i]->object->next->ui, true))
    		);
		}
		
		al_draw_bitmap(ui_items[i]->bitmap, ui_items[i]->loc.x-UI_MARGIN, ui_items[i]->loc.y-UI_MARGIN, 0);
	}
	
	// Draw toolbar
	UIRect  toolbar_rect = UIRect_xy(-10, -10, UI_WIDTH+20, UI_TOOLBAR_HEIGHT+10);
	UIRect button_rect = UIRect_points(UIPoint_xy(15, 15), UIPoint_xy((UI_WIDTH - 15) / TYPE_COUNT - 15, UI_TOOLBAR_HEIGHT - 30));
	UI_draw_box(toolbar_rect, UI_COLOR_WHITE);
	
	for (ObjType t = 0; t < TYPE_COUNT; t++) {
		bool mouse_over = UI_is_point_within_rect(ui_mouse, button_rect);
		
		// If there's a click, add the new item under the cursor
		if (mouse_over && ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
			Object *new = Object_with(t);
			UI_add_object(new, ui_mouse);
			UI_set_target(new, UITargetObject);
		}
		
		UI_draw_box(button_rect, mouse_over ? UI_COLOR_HIGHLIGHT : UI_COLOR_WHITE);
		UI_draw_label(UIPoint_offset(button_rect.loc, button_rect.size.x/2, 9), obj_get_name(t));
		button_rect.loc.x += button_rect.size.x + 15;
	}
	
	// Draw an unconnected node edge if we are currently in the process of changing one
	if (ui_target.item != NULL && ui_target.type != UITargetObject) {
		UI_draw_line_thick(ui_mouse, ui_target.offset);
	}
	
	// Send output buffer to display
	al_flip_display();
	
	// Wait for next event
	al_wait_for_event(ui_event_queue, &ev);

	is_mouse_event = false;
	switch (ev.type) {
		case ALLEGRO_EVENT_DISPLAY_CLOSE:
			return false; // Stop the run-loop in main execution thread (close app)
		case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN:
		case ALLEGRO_EVENT_MOUSE_AXES:
			is_mouse_event = true;
    		ui_mouse.x = ev.mouse.x;
    		ui_mouse.y = ev.mouse.y;
			break;
		case ALLEGRO_EVENT_KEY_DOWN:
			// Clear any target
			if (ev.keyboard.keycode == ALLEGRO_KEY_ESCAPE)
				ui_target.item = NULL;
			// Delete any target
			else if (ev.keyboard.keycode == ALLEGRO_KEY_DELETE ||
					 ev.keyboard.keycode == ALLEGRO_KEY_BACKSPACE) {
				if (ui_target.item != NULL && ui_target.is_selected) {
					UI_remove_object(ui_target.item->object);
					ui_target.item = NULL;
				}
			}
		case ALLEGRO_EVENT_KEY_UP:
			break;
		default: ev.type = 0; // Don't forward events we don't recognise
	}

	return true;
}

UIPoint UI_io_point_for_drawable(UIDrawable *d, bool is_input) {
    if (is_input)
        return UIPoint_xy(0, d->size.y/2);
    else
        return UIPoint_xy(d->size.x, d->size.y/2);
}

UIRect UIRect_points(UIPoint origin, UIPoint size) {
    UIRect r = {origin, size};
    return r;
}

extern inline UIRect UIRect_xy(int x, int y, int width, int height) {
    UIPoint s = {width, height};
    UIPoint p = {x, y};
    return UIRect_points(p, s);
}

UIPoint UIPoint_xy(int x, int y) {
	UIPoint p = {x, y};
	return p;
}

UIPoint UIPoint_offset(UIPoint origin, int x, int y) {
	return UIPoint_xy(origin.x+x, origin.y+y);
}

UIPoint UIPoint_add(UIPoint p1, UIPoint p2) {
    return UIPoint_xy(p1.x + p2.x, p1.y + p2.y);
}

UIPoint UIPoint_subtract(UIPoint p1, UIPoint p2) {
    return UIPoint_xy(p1.x - p2.x, p1.y - p2.y);
}

UIDrawable *UIDrawable_with(struct Object *o, UIPoint loc) {
	UIDrawable *d = malloc(sizeof(UIDrawable));

	d->object     = o;
	d->loc        = loc;
	d->size       = obj_get_size(o->type); // Set the size to be the UIPoint defined by the module
	d->bitmap     = al_create_bitmap(d->size.x + UI_MARGIN*2, d->size.y + UI_MARGIN*2); // Derive bitmap size from retrieved UIPoint

	return d;
}

void UI_draw_title(UIPoint loc, char *text) {
	static ALLEGRO_FONT *font = NULL;
	
	if (!font)
		font = al_load_ttf_font("Roboto-Medium.ttf", 42, 0);
	
	al_draw_text(font, UI_COLOR_BLACK, loc.x, loc.y, ALLEGRO_ALIGN_CENTRE, text);
}

void UI_draw_label(UIPoint loc, char *text) {
	static ALLEGRO_FONT *font = NULL;
	
	if (!font)
		font = al_load_ttf_font("Roboto-Medium.ttf", 28, 0);
	
	al_draw_text(font, UI_COLOR_BLACK, loc.x, loc.y, ALLEGRO_ALIGN_CENTRE, text);
}

void UI_draw_circle(UIPoint c, int r, ALLEGRO_COLOR fill_color) {
	al_draw_filled_circle(c.x, c.y, r, fill_color);
	al_draw_circle(c.x, c.y, r, UI_COLOR_BLACK, 4);
}

void UI_draw_border(UIRect rect, ALLEGRO_COLOR color) {
	al_draw_rectangle(rect.loc.x, rect.loc.y, rect.loc.x+rect.size.x, rect.loc.y+rect.size.y, color, 20);
}

void UI_draw_box(UIRect rect, ALLEGRO_COLOR fill_color) {
	al_draw_filled_rectangle(rect.loc.x, rect.loc.y, rect.loc.x+rect.size.x, rect.loc.y+rect.size.y, fill_color);
	al_draw_rectangle(rect.loc.x, rect.loc.y, rect.loc.x+rect.size.x, rect.loc.y+rect.size.y, UI_COLOR_BLACK, 4);
}

void UI_draw_line(UIPoint p1, UIPoint p2) {
	al_draw_line(p1.x, p1.y, p2.x, p2.y, UI_COLOR_BLACK, 3);
}

void UI_draw_line_thick(UIPoint p1, UIPoint p2) {
	al_draw_line(p1.x, p1.y, p2.x, p2.y, UI_COLOR_BLACK, 8);
}

bool UI_is_point_within(UIPoint p, int x1, int x2, int y1, int y2) {
    return (WITHIN(p.x, x1, x2) && WITHIN(p.y, y1, y2));
}

bool UI_is_point_near(UIPoint p1, UIPoint p2, int threshhold) {
    return (abs(p1.x - p2.x) <= threshhold) && (abs(p1.y - p2.y) <= threshhold);
}

bool UI_is_point_near_io(UIPoint p, UIDrawable *d, bool is_input) {
	return UI_is_point_near(p, UIPoint_add_margin(UI_io_point_for_drawable(d, is_input)), UI_IO_SIZE*2);
}
	
extern inline bool UI_is_point_within_bounds(UIPoint p, UIPoint loc, UIPoint bounds) {
    return UI_is_point_within(p, loc.x, loc.x + bounds.x, loc.y, loc.y + bounds.y);
}

extern inline bool UI_is_point_within_rect(UIPoint p, UIRect r) {
    return UI_is_point_within_bounds(p, r.loc, r.size);
}

extern inline bool UI_is_point_within_drawable(UIPoint p, UIDrawable *d) {
	// Include margin here so that the events will still fire when the mouse is being moved on and off IO nodes
    return UI_is_point_within_bounds(p, UIPoint_subtract(d->loc, UIPoint_margin), UIPoint_add(UIPoint_add(d->size, UIPoint_margin), UIPoint_margin));
}
