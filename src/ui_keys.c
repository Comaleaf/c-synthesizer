#include "ui_keys.h"

void UIKeys_draw_object(UIObject *obj, UIPoint loc) {
	UIObject *o = obj;

	// Draw outline
	UI_draw_box(o->loc, o->size, WHITE);

	// Draw keys
	
	int key_width   = o->size.width / 2;
	int key_height  = o->size.height / 7;
	UISize white_key_size = UISize_with_bounds(o->size.width, key_height);
	UISize black_key_size = UISize_with_bounds(key_width, key_height);

	UI_draw_box(UIPoint_with_offset(o->loc, 0, 0*key_height), white_key_size, WHITE);
	UI_draw_box(UIPoint_with_offset(o->loc, 0, 1*key_height), white_key_size, WHITE);
	UI_draw_box(UIPoint_with_offset(o->loc, 0, 2*key_height), white_key_size, WHITE);
	UI_draw_box(UIPoint_with_offset(o->loc, 0, 3*key_height), white_key_size, WHITE);
	UI_draw_box(UIPoint_with_offset(o->loc, 0, 4*key_height), white_key_size, WHITE);
	UI_draw_box(UIPoint_with_offset(o->loc, 0, 5*key_height), white_key_size, WHITE);
	UI_draw_box(UIPoint_with_offset(o->loc, 0, 6*key_height), white_key_size, WHITE);

	UI_draw_box(UIPoint_with_offset(o->loc, 0, 0.5*key_height), black_key_size, DARKGRAY);
	UI_draw_box(UIPoint_with_offset(o->loc, 0, 1.5*key_height), black_key_size, DARKGRAY);
	UI_draw_box(UIPoint_with_offset(o->loc, 0, 3.5*key_height), black_key_size, DARKGRAY);
	UI_draw_box(UIPoint_with_offset(o->loc, 0, 4.5*key_height), black_key_size, DARKGRAY);
	UI_draw_box(UIPoint_with_offset(o->loc, 0, 5.5*key_height), black_key_size, DARKGRAY);

	/*unsigned color;

	if (o == drag_object)
		color = RED;
	else if (o->is_focused)
		color = YELLOW;
	else
		color = WHITE;*/

	// There is a connection
	if (o->next != NULL)
		UI_draw_line(UIPoint_with_offset(o->loc, o->size.width, 30), UIPoint_with_offset(o->next->loc, 0, 30));

	// Connecting nodes
	UI_draw_box(UIPoint_with_offset(o->loc, -10, +20),              UISize_with_bounds(20, 20), GREEN);
	UI_draw_box(UIPoint_with_offset(o->loc, o->size.width-10, +20), UISize_with_bounds(20, 20), GREEN);

	setcolor(BLACK);
	//al_draw_text(_font, al_map_rgb(0, 0, 0), o->loc.x+o->size.width/2, o->loc.y+10, ALLEGRO_ALIGN_CENTRE, o->label);
}
