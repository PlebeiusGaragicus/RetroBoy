#include <gb/gb.h>
#include <gb/drawing.h>

#include <stdint.h>
#include <stdio.h>
#include <rand.h>


uint8_t PlayerX = 0;
uint8_t PlayerY = 0;


unsigned char CrossSprite[] =
{
    0x00,0x00,0x00,0x00,0x10,0x10,0x38,0x38,0x10,0x10,0x00,0x00,0x00,0x00,0x00,0x00,
};



int random(int min, int max) {
	return (rand() % (max - min + 1)) + min;
}



void init()
{
	SHOW_BKG;
	SHOW_SPRITES;
	DISPLAY_ON;
}


void seed_prng()
{
        printf(" \n\n\n\n\n\n\n\n    PRESS START!\n");
        // abuse user input for seed generation
        waitpad(J_START);
        uint16_t seed = LY_REG;
        seed |= (uint16_t)DIV_REG << 8;
        initrand(seed);

		printf("\n\nLucky number: %d\n\n", seed);
		delay(250);
		// clear_screen();

		uint8_t countdown = 20;
        while(countdown-- > 0)
        {
            uint8_t r = ((uint8_t)rand()) % (uint8_t)2;
            printf("%d", r);

			vsync();
        }
}


void clear_screen() {
	uint8_t j, f;
	for (j = 0; j < 20; j++) { // GB screen is 20 columns wide and 18 columns tall
		for (f = 0; f < 18; f++) { 
			gotogxy(j, f);
			wrtchr(' '); // Use wrtchr to place a character when using the drawing library
		}
	}
}


void main()
{
	init();
	seed_prng();
	clear_screen();

	set_sprite_data(0, 16, CrossSprite);
	color(DKGREY, WHITE, SOLID);


	PlayerX = random(10, 190);
	PlayerY = random(10, 190);

	uint8_t key;
	while (1) {
		key = joypad();

		if (key & J_UP)
			PlayerY--;
		else if (key & J_DOWN)
			PlayerY++;

		if (key & J_LEFT)
			PlayerX--;
		else if (key & J_RIGHT)
			PlayerX++;

		move_sprite(0, PlayerX, PlayerY);

		vsync();
	}
}
