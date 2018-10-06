#include "library/syscalls.h"
#include "library/user-io.h"
#include "library/widgets.h"

// Draws the border with the colors specified
int draw_border(int x, int y, int w, int h, int thickness, int screen_d_w, int screen_d_h, int r, int g, int b)
{
	// Check that dimensions line up 
	if(x + w > screen_d_w || y + h > screen_d_h) {
		return -1;
	}
	// Color the border appropriately
	draw_color(r, b, g);

	// Draw 4 rectangles to represent the border
	draw_rect(0, 0, w, thickness);
	draw_rect(0, 0, thickness, h);
	draw_rect(w - thickness, 0, thickness, h);
	draw_rect(0, h - thickness, w, 5);

	return 0;
}