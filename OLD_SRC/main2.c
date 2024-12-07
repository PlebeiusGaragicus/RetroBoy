#include <stdint.h>
#include <stdio.h>
// #include <rand.h>

#include <gb/gb.h>
#include <gb/cgb.h>
#include <gb/drawing.h>

#include "sprites.h"
#include "util.h"
#include "intro.h"



// #####################################################################################

// Screen boundaries
#define MIN_X 8
#define MAX_X 160
#define MIN_Y 16
#define MAX_Y 152



int16_t PlayerX = 0;
int16_t PlayerY = 0;

int8_t VelX = 0;
int8_t VelY = 0;

#define ACCELERATION 1
#define MAX_VELOCITY 4
#define FRICTION 1
#define BOUNCE_FACTOR -3







// #####################################################################################
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




// #####################################################################################
void main()
{
    // show_Trump();
    init();
    // delay(1250);
    seed_prng();
    clear_screen();
    // color(DKGREY, WHITE, SOLID);

    set_sprite_data(0, 16, Gooby);
    
    PlayerX = random(MIN_X, MAX_X);
    PlayerY = random(MIN_Y, MAX_Y);
    
    while (1) {
        update_physics();
        move_sprite(0, PlayerX, PlayerY);
        vsync();
    }
}
