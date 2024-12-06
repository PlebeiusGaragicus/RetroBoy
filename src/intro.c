#include <stdint.h>
#include <stdio.h>
#include <rand.h>

#include <gb/gb.h>
#include <gb/cgb.h>
#include <gb/drawing.h>

#include "sprites.h"
#include "intro.h"

#include "VERSION.h"



// typedef uint8_t palette_color_t;


// Define 4 colors for one palette (from lightest to darkest)
const palette_color_t palette[] = {
    0x00,
    0xFF,
    0x7F,
    0x3F
};



void init()
{
    // Use palette 0, set 1 palette, use our palette data
    set_bkg_palette(0, 1, palette);
    // set_bkg_palette_entry(0, 0, palette[0]);

	SHOW_BKG;
	SHOW_SPRITES;
	DISPLAY_ON;

    // color(DKGREY, WHITE, SOLID);
}




// https://stackoverflow.com/questions/66105001/how-can-you-get-a-random-number-on-gbdk
void seed_prng()
{
    printf("Build: %d", BUILD);
    printf(" \n\n\n\n\n\n\n\n    PRESS START!\n");
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


void show_Trump() {

    // Load tileset into GB memory
     set_bkg_data(0, TrumpFace_tileset_size, TrumpFace_tileset);

     // Fill screen with splashscreen map
     set_bkg_tiles(0, 0, 20, 18, TrumpFace_tilemap);

    //  delay(1250);
}