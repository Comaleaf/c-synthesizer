#include "audio.h"
#include "ui.h"

int main(void)
{
	Audio_init();
	UI_init();

	while (UI_next());
	
	UI_close();
	Audio_close();

	return 0;
}

