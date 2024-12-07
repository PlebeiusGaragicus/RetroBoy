#include <rand.h>

#include <gb/gb.h>
#include <gb/drawing.h>



int random(int min, int max) {
	return (rand() % (max - min + 1)) + min;
}


//NOTE: this is not the best way to clear the screen and it's kinda slow
void clear_screen() {
	uint8_t j, f;
	for (j = 0; j < 20; j++) { // GB screen is 20 columns wide and 18 columns tall
		for (f = 0; f < 18; f++) { 
			gotogxy(j, f);
			wrtchr(' '); // Use wrtchr to place a character when using the drawing library
		}
	}
}



void performantdelay(uint8_t numloops)
{
    uint8_t i;
    for(i = 0; i < numloops; i++)
    {
        wait_vbl_done();
    }
}