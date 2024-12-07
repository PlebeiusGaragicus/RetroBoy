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

BOOLEAN game_over = FALSE;


int16_t PlayerX = 0;
int16_t PlayerY = 0;

int16_t old_PlayerX = 0;
int16_t old_PlayerY = 0;

int8_t VelX = 0;
int8_t VelY = 0;

#define ACCELERATION 2
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
    old_PlayerX = PlayerX;
    old_PlayerY = PlayerY;
    PlayerX += VelX * 2;
    PlayerY += VelY * 2;

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


void ready_start() {
    uint8_t x, y;
    // volatile uint8_t frame_count = 0;
    uint8_t key;

    while(1) {
        key = joypad();
        if (key & (J_START | J_A | J_B))
            break;

        // every 20 frames
        // if( frame_count % 20 == 0 )
        if( sys_time % 20 == 0 )
        {
            x = random(MIN_X, MAX_X);
            y = random(MIN_Y, MAX_Y);
            move_sprite(0, x, y);
        }

        // frame_count++;
        vsync();
    }

    // hide sprite
    // move_sprite(1, 0, 0);

    PlayerX = x;
    PlayerY = y;

    game_over = FALSE;
    // clear_screen(); // NOTE: this is slow...
}

// #####################################################################################
void main()
{
    // show_Trump();
    init();
    // delay(1250);
    seed_prng();
    clear_screen();

    set_sprite_data(0, 1, Snek_head);
    set_sprite_data(1, 1, Snek_body);
    set_sprite_data(2, 1, Snek_tail);
    // set_sprite_data(1, 5, Ready);




    
    while (1) {
        ready_start();
        // set_sprite_data(0, 1, Gooby);


        // PlayerX = random(MIN_X, MAX_X);
        // PlayerY = random(MIN_Y, MAX_Y);

        while(!game_over) {
    
            update_physics();
            move_sprite(0, PlayerX, PlayerY);
            move_sprite(1, old_PlayerX, old_PlayerY);

            vsync();
        }
    }
}






/*

fixed player[2];
...
// Modify player position using its 16 bit representation
player[0].w += player_speed_x;
player[1].w += player_speed_y;
...
// Use only the upper 8 bits for setting the sprite position
move_sprite(0, player[0].h ,player[1].h);

*/


/*
20.72.5.10 sys_time volatile uint16_t sys_time [extern]
Global Time Counter in VBL periods (60Hz)
Increments once per Frame
Will wrap around every∼18 minutes (unsigned 16 bits = 65535 / 60 / 60 = 18.2)
*/