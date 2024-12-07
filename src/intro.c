#include <stdint.h>
#include <stdio.h>
#include <rand.h>

#include <gb/gb.h>
#include <gb/cgb.h>
#include <gb/drawing.h>

#include "VERSION.h"
#include "util.h"
#include "audio.h"
#include "intro.h"
#include "sprites.h"
#include "faces.h"


// DMG_PALETTE; //??? TODO:

void clear_screen() {
	// uint8_t j, f;
	// for (j = 0; j < 20; j++) { // GB screen is 20 columns wide and 18 columns tall
	// 	for (f = 0; f < 18; f++) { 
	// 		gotogxy(j, f);
	// 		wrtchr(' '); // Use wrtchr to place a character when using the drawing library
	// 	}
	// }

    vsync();
    HIDE_BKG;

	// Load blank sprite tile into VRAM
	set_bkg_data(0, 1, BLANK);
	// Fill entire screen with 1st tile
	init_bkg(0);

    SHOW_BKG;
}

void load_justin() {
    HIDE_BKG;

    // Load tileset into GB memory
    set_bkg_data(0, Justin_tileset_size, Justin_tileset);
    // Fill screen with splashscreen map
    set_bkg_tiles(0, 0, 20, 18, Justin_tilemap);

	SHOW_BKG;

}

void load_trump() {
    HIDE_BKG;

    // Load tileset into GB memory
    set_bkg_data(0, TrumpFace_tileset_size, TrumpFace_tileset);
    // Fill screen with splashscreen map
    set_bkg_tiles(0, 0, 20, 18, TrumpFace_tilemap);

	SHOW_BKG;

}

void load_cybertruck() {
    HIDE_BKG;

    // Load tileset into GB memory
    set_bkg_data(0, CyberTruck_tileset_size, CyberTruck_tileset);
    // Fill screen with splashscreen map
    set_bkg_tiles(0, 0, 20, 18, CyberTruck_tilemap);

	SHOW_BKG;

}

void show_screen_border() {
    HIDE_BKG;

    // Load tileset into GB memory
    set_bkg_data(0, 9, ScreenBorderTiles);
    // Fill screen with splashscreen map
    set_bkg_tiles(0, 0, 20, 18, ScreenBorderMap);

    SHOW_BKG;
}






void splash_screen() {
	SHOW_BKG;
	SHOW_SPRITES;
	DISPLAY_ON;
    audio_init();

    seed_prng();

    load_justin();
    // load_trump();
    // load_cybertruck();

    performantdelay(50);

    while(TRUE) {
        if (joypad()) break;
        vsync();
    }

    beedledo();
    clear_screen();
}



// https://stackoverflow.com/questions/66105001/how-can-you-get-a-random-number-on-gbdk
void seed_prng()
{
    // printf("Build: %d", BUILD);
    printf("--------------------");
    printf("|                  |");
    printf("|    Retro  Boy    |");
    printf("|                  |");
    printf("--------------------");
    printf("\n\n\n");
    printf("    PRESS START!");
    printf("\n\n\n\n\n");
    printf("  Compiled on:\n\n");
    printf("%s", VERSION);

    waitpad(J_START);
    boop();

    uint16_t seed = LY_REG;
    seed |= (uint16_t)DIV_REG << 8;
    initrand(seed);

    printf("\nLucky Number: %d", seed);
    // delay(150);
    performantdelay(10);
    clear_screen();

    // uint8_t countdown = 360; // 360 characters fit on the whole screen
    // while(countdown-- > 0)
    // {
    //     uint8_t r = ((uint8_t)rand()) % (uint8_t)2;
    //     printf("%d", r);

    //     vsync();
    // }

    // delay(100);
    // vsync();
}
