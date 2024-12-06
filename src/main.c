#include <stdint.h>
#include <stdio.h>
#include <rand.h>

#include <gb/gb.h>
#include <gb/cgb.h>
#include <gb/drawing.h>

#include "assets/sprites.h"

#define BUILD 124

// Position variables (using fixed-point for smoother movement)
int16_t PlayerX = 0;
int16_t PlayerY = 0;
// Velocity variables
int8_t VelX = 0;
int8_t VelY = 0;
// Constants
#define ACCELERATION 1
#define MAX_VELOCITY 4
#define FRICTION 1
#define BOUNCE_FACTOR -3

// Screen boundaries
#define MIN_X 8
#define MAX_X 160
#define MIN_Y 16
#define MAX_Y 152




int random(int min, int max) {
	return (rand() % (max - min + 1)) + min;
}


// Define 4 colors for one palette (from lightest to darkest)
uint8_t palette[] = {
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

void show_Trump() {

    // Load tileset into GB memory
     set_bkg_data(0, TrumpFace_tileset_size, TrumpFace_tileset);

     // Fill screen with splashscreen map
     set_bkg_tiles(0, 0, 20, 18, TrumpFace_tilemap);

    //  delay(1250);
}

void seed_prng()
{
    // https://stackoverflow.com/questions/66105001/how-can-you-get-a-random-number-on-gbdk
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


void clear_screen() {
	uint8_t j, f;
	for (j = 0; j < 20; j++) { // GB screen is 20 columns wide and 18 columns tall
		for (f = 0; f < 18; f++) { 
			gotogxy(j, f);
			wrtchr(' '); // Use wrtchr to place a character when using the drawing library
		}
	}
}



void update_physics() {
    uint8_t key = joypad();
    
    if (key & J_LEFT) {
        VelX -= ACCELERATION;
        if (VelX < -MAX_VELOCITY) VelX = -MAX_VELOCITY;
    }
    else if (key & J_RIGHT) {
        VelX += ACCELERATION;
        if (VelX > MAX_VELOCITY) VelX = MAX_VELOCITY;
    }
    else {
        // Apply friction when no input
        if (VelX > 0) VelX -= FRICTION;
        else if (VelX < 0) VelX += FRICTION;
    }

    if (key & J_UP) {
        VelY -= ACCELERATION;
        if (VelY < -MAX_VELOCITY) VelY = -MAX_VELOCITY;
    }
    else if (key & J_DOWN) {
        VelY += ACCELERATION;
        if (VelY > MAX_VELOCITY) VelY = MAX_VELOCITY;
    }
    else {
        // Apply friction when no input
        if (VelY > 0) VelY -= FRICTION;
        else if (VelY < 0) VelY += FRICTION;
    }

    // Update position
    PlayerX += VelX;
    PlayerY += VelY;

    // Screen boundary collision
    if (PlayerX < MIN_X) {
        PlayerX = MIN_X;
        VelX *= BOUNCE_FACTOR;
    }
    if (PlayerX > MAX_X) {
        PlayerX = MAX_X;
        VelX *= BOUNCE_FACTOR;
    }
    if (PlayerY < MIN_Y) {
        PlayerY = MIN_Y;
        VelY *= BOUNCE_FACTOR;
    }
    if (PlayerY > MAX_Y) {
        PlayerY = MAX_Y;
        VelY *= BOUNCE_FACTOR;
    }
}

void main()
{
    // show_Trump();
    init();
    // delay(1250);
    seed_prng();
    clear_screen();
    
    set_sprite_data(0, 16, Gooby);
    // color(DKGREY, WHITE, SOLID);
    
    PlayerX = random(MIN_X, MAX_X);
    PlayerY = random(MIN_Y, MAX_Y);
    
    while (1) {
        update_physics();
        move_sprite(0, PlayerX, PlayerY);
        vsync();
    }
}
