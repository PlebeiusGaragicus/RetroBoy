#include <gb/gb.h>
#include <gb/drawing.h>

uint8_t x = 3, y = 6, i = 3, j, f;

void main() {
	color(DKGREY, WHITE, SOLID); //  forecolor, backcolor, mode [Colors: WHITE (0), LTGREY (1), DKGREY(2), BLACK (3)]
	// circle(100, 60, 30, M_FILL); // x, y, radius, style (M_FILL or M_NOFILL)
	// line(40, 40, 50, 50); // x1, y1, x2, y2
	// box(120, 125, 125, 135, M_NOFILL); // x1, y1, x2, y2, style (M_FILL or M_NOFILL)

	// plot_point(3, 4); // Plot a single pixel on the screen
	// gotogxy(18, 15); // Places you at these coordinates
	// gprintf("asdf"); // When using the drawing library you must use gprintf instead of printf

	uint8_t key;
	while (1) {
		key = joypad();

		// print("X: %d, Y: %d, Color: %d\n", x, y, i);
		// print(key);

		if (key & J_UP)
			y--;
		if (key & J_DOWN)
			y++;
		if (key & J_LEFT)
			x--;
		if (key & J_RIGHT)
			x++;
	
		// if (key == J_A) { // Change colors
		// 	waitpadup();
		// 	i++;
		// 	if (i > 3)
		// 		i = 0;
		// }

		// if (key == J_B) { // Function to 'clear' the screen
		// 	for (j = 0; j < 20; j++) { // GB screen is 20 columns wide and 18 columns tall
		// 		for (f = 0; f < 18; f++) { 
		// 			gotogxy(j, f);
		// 			wrtchr(' '); // Use wrtchr to place a character when using the drawing library
		// 		}
		// 	}
		// }
		plot(x, y, i, SOLID); // Draws a pixel on the screen (x, y, color, mode)
		// delay(10);

		vsync();
	}
}