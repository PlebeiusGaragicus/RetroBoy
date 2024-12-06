#include <stdint.h>
#include <stdio.h>
#include <rand.h>

#include <gb/gb.h>
// #include <nes/nes.h>
#include <gb/drawing.h>

#include "assets/sprites.h"

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

typedef uint8_t palette_color_t;
#define RGB_TO_NES(c) \
    (c == 0x00) ? 0x1D : \
    (c == 0x01) ? 0x06 : \
    (c == 0x02) ? 0x17 : \
    (c == 0x03) ? 0x16 : \
    (c == 0x04) ? 0x19 : \
    (c == 0x05) ? 0x18 : \
    (c == 0x06) ? 0x17 : \
    (c == 0x07) ? 0x27 : \
    (c == 0x08) ? 0x2A : \
    (c == 0x09) ? 0x29 : \
    (c == 0x0A) ? 0x28 : \
    (c == 0x0B) ? 0x27 : \
    (c == 0x0C) ? 0x2A : \
    (c == 0x0D) ? 0x29 : \
    (c == 0x0E) ? 0x29 : \
    (c == 0x0F) ? 0x28 : \
    (c == 0x10) ? 0x01 : \
    (c == 0x11) ? 0x04 : \
    (c == 0x12) ? 0x15 : \
    (c == 0x13) ? 0x15 : \
    (c == 0x14) ? 0x1C : \
    (c == 0x15) ? 0x00 : \
    (c == 0x16) ? 0x15 : \
    (c == 0x17) ? 0x26 : \
    (c == 0x18) ? 0x2B : \
    (c == 0x19) ? 0x2A : \
    (c == 0x1A) ? 0x10 : \
    (c == 0x1B) ? 0x26 : \
    (c == 0x1C) ? 0x2B : \
    (c == 0x1D) ? 0x2A : \
    (c == 0x1E) ? 0x39 : \
    (c == 0x1F) ? 0x38 : \
    (c == 0x20) ? 0x02 : \
    (c == 0x21) ? 0x13 : \
    (c == 0x22) ? 0x14 : \
    (c == 0x23) ? 0x14 : \
    (c == 0x24) ? 0x11 : \
    (c == 0x25) ? 0x13 : \
    (c == 0x26) ? 0x10 : \
    (c == 0x27) ? 0x25 : \
    (c == 0x28) ? 0x2C : \
    (c == 0x29) ? 0x10 : \
    (c == 0x2A) ? 0x3D : \
    (c == 0x2B) ? 0x36 : \
    (c == 0x2C) ? 0x2C : \
    (c == 0x2D) ? 0x3B : \
    (c == 0x2E) ? 0x3A : \
    (c == 0x2F) ? 0x37 : \
    (c == 0x30) ? 0x12 : \
    (c == 0x31) ? 0x13 : \
    (c == 0x32) ? 0x14 : \
    (c == 0x33) ? 0x24 : \
    (c == 0x34) ? 0x12 : \
    (c == 0x35) ? 0x22 : \
    (c == 0x36) ? 0x23 : \
    (c == 0x37) ? 0x24 : \
    (c == 0x38) ? 0x21 : \
    (c == 0x39) ? 0x22 : \
    (c == 0x3A) ? 0x32 : \
    (c == 0x3B) ? 0x34 : \
    (c == 0x3C) ? 0x2C : \
    (c == 0x3D) ? 0x3C : \
    (c == 0x3E) ? 0x3C : \
    (c == 0x3F) ? 0x20 : \
                  0xFF // out-of-range value - set to 0xFF

#define RGB8(r,g,b)       RGB_TO_NES((((r) >> 6) | (((g) >> 6) << 2) | (((b) >> 6) << 4)))
void set_bkg_palette(uint8_t first_palette, uint8_t nb_palettes, const palette_color_t *rgb_data) NO_OVERLAY_LOCALS;

void init()
{
    // Define 4 colors for one palette (from lightest to darkest)
    palette_color_t palette[] = {
        RGB8(255, 255, 255), // White (Color 0)
        RGB8(170, 100, 170), // Light grey (Color 1)
        RGB8(85, 0, 85),    // Dark grey (Color 2)
        RGB8(0, 50, 0)        // Black (Color 3)
    };

    // Set the background palette
    set_bkg_palette(0, 1, palette);  // Use palette 0, set 1 palette, use our palette data

	SHOW_BKG;
	SHOW_SPRITES;
	DISPLAY_ON;

    color(DKGREY, WHITE, SOLID);
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
    show_Trump();
    init();
    delay(1250);
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
