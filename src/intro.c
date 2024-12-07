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
#include "Justin.h"




// typedef uint8_t palette_color_t;


// Define 4 colors for one palette (from lightest to darkest)
const palette_color_t palette[] = {
    0x00,
    0xFF,
    0x7F,
    0x3F
};


// // Declare a global variable to store the frame count
// volatile uint8_t frame_count = 0;


// // VBlank interrupt handler
// void vblank_interrupt() {
//     frame_count++;
// }

// // Initialize interrupts (call this once at the start of your program)
// void init_interrupts() {
//     // enable_interrupt(VBLANK_INTERRUPT);
//     enable_interrupt();
// }


void splash_screen() {
    init();
    seed_prng();
    clear_screen();
}


void init()
{
    // Load tileset into GB memory
    set_bkg_data(0, Justin_tileset_size, Justin_tileset);

     // Fill screen with splashscreen map
    set_bkg_tiles(0, 0, 20, 18, Justin_tilemap);

	SHOW_BKG;
	SHOW_SPRITES;
	DISPLAY_ON;

    audio_init();

    performantdelay(50);

    beedledo();

    // waitpad(J_START);
    while(TRUE) {
        if (joypad()) {
            boop();
            break;
        }
        vsync();
    }


    // DMG_PALETTE; //???
}




// https://stackoverflow.com/questions/66105001/how-can-you-get-a-random-number-on-gbdk
void seed_prng()
{
    // printf("Build: %d", BUILD);
    printf("%s\n", VERSION);
    printf(" \n\n\n\n\n\n\n\n    PRESS START!\n");
    waitpad(J_START);
    boop();
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


// void show_Trump() {

//     // Load tileset into GB memory
//      set_bkg_data(0, TrumpFace_tileset_size, TrumpFace_tileset);

//      // Fill screen with splashscreen map
//      set_bkg_tiles(0, 0, 20, 18, TrumpFace_tilemap);

//     //  delay(1250);
// }


void show_screen_border() {

    // Load tileset into GB memory
    set_bkg_data(0, 9, ScreenBorderTiles);

    // Fill screen with splashscreen map
    set_bkg_tiles(0, 0, 20, 18, ScreenBorderMap);
}