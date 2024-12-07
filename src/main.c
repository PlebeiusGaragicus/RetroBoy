#include <stdint.h>
#include <stdio.h>
// #include <rand.h>
#include <types.h>  // for fixed type

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


// int16_t PlayerX = 0;
// int16_t PlayerY = 0;

// int16_t old_PlayerX = 0;
// int16_t old_PlayerY = 0;


fixed PlayerPos[2];    // [0] is X, [1] is Y
fixed old_PlayerPos[2];

// Keep velocities as int8_t since they're small increments
int8_t VelX = 0;
int8_t VelY = 0;

#define TOP_SPEED 60
#define SPEED_CHANGE 3

// Helper function to reduce velocity by ~30%
// int8_t reduce_velocity(int8_t vel) {
//     int8_t half = vel >> 1;    // divide by 2
//     int8_t eighth = vel >> 3;   // divide by 8
//     int8_t sixteenth = vel >> 4; // divide by 16
//     return half + eighth + sixteenth;
// }

// Helper function to reduce velocity by ~70%
int8_t reduce_velocity(int8_t vel) {
    int8_t quarter = vel >> 2;    // divide by 4
    int8_t sixteenth = vel >> 4;  // divide by 16
    return quarter + sixteenth;
}

void update_physics() {
    uint8_t key = joypad();

    // Movement logic with fixed point adjustments
    if (key & J_LEFT) {
        if (VelX < -TOP_SPEED) {
            VelX = -TOP_SPEED;
            if (VelY != 0) {
                VelY += (VelY > 0) ? -1 : 1;
            }
        } else {
            VelX -= SPEED_CHANGE;
        }
    }
    else if (key & J_RIGHT) {
        if (VelX > TOP_SPEED) {
            VelX = TOP_SPEED;
            if (VelY != 0) {
                VelY += (VelY > 0) ? -1 : 1;
            }
        } else {
            VelX += SPEED_CHANGE;
        }
    }

    if (key & J_UP) {
        if (VelY < -TOP_SPEED) {
            VelY = -TOP_SPEED;
            if (VelX != 0) {
                VelX += (VelX > 0) ? -1 : 1;
            }
        } else {
            VelY -= SPEED_CHANGE;
        }
    }
    else if (key & J_DOWN) {
        if (VelY > TOP_SPEED) {
            VelY = TOP_SPEED;
            if (VelX != 0) {
                VelX += (VelX > 0) ? -1 : 1;
            }
        } else {
            VelY += SPEED_CHANGE;
        }
    }

    // Store old position for trail effect
    old_PlayerPos[0].w = PlayerPos[0].w;
    old_PlayerPos[1].w = PlayerPos[1].w;

    // Update position using 16-bit math
    // Convert VelX/VelY to fixed point by shifting left 4 bits
    PlayerPos[0].w += ((int16_t)VelX << 4);
    PlayerPos[1].w += ((int16_t)VelY << 4);

    // Screen boundary collision using the high byte
    if (PlayerPos[0].h < MIN_X) {
        PlayerPos[0].w = ((int16_t)MIN_X << 8);  // Reset both high and low bytes
        // VelX = -VelX;
        VelX = -reduce_velocity(VelX);
    }
    if (PlayerPos[0].h > MAX_X) {
        PlayerPos[0].w = ((int16_t)MAX_X << 8);
        // VelX = -VelX;
        VelX = -reduce_velocity(VelX);
    }
    if (PlayerPos[1].h < MIN_Y) {
        PlayerPos[1].w = ((int16_t)MIN_Y << 8);
        // VelY = -VelY;
        VelY = -reduce_velocity(VelY);
    }
    if (PlayerPos[1].h > MAX_Y) {
        PlayerPos[1].w = ((int16_t)MAX_Y << 8);
        // VelY = -VelY;
        VelY = -reduce_velocity(VelY);
    }

    // Move sprites using only the high byte (integer portion)
    move_sprite(0, PlayerPos[0].h, PlayerPos[1].h);
    move_sprite(1, old_PlayerPos[0].h, old_PlayerPos[1].h);
}

void ready_start() {
    uint8_t x, y;
    uint8_t key;
    
    while(1) {
        key = joypad();
        if (key & (J_START | J_A | J_B))
            break;
        
        if(sys_time % 20 == 0) {
            x = random(MIN_X, MAX_X);
            y = random(MIN_Y, MAX_Y);
            move_sprite(0, x, y);
        }
        vsync();
    }
    
    // Initialize fixed-point position
    PlayerPos[0].h = x;
    PlayerPos[0].l = 0;
    PlayerPos[1].h = y;
    PlayerPos[1].l = 0;
    
    game_over = FALSE;
}

// #####################################################################################
void main()
{
    // show_Trump();
    init();
    // DMG_PALETTE; //???
    // delay(1250);
    seed_prng();
    clear_screen();

    set_sprite_data(0, 1, Snek_head);
    set_sprite_tile(0,0);
    set_sprite_data(1, 1, Snek_body);
    set_sprite_tile(1,1);
    set_sprite_data(2, 1, Snek_tail);
    set_sprite_tile(2,2);
    // set_sprite_data(1, 5, Ready);




    
    while (1) {
        ready_start();
        // set_sprite_data(0, 1, Gooby);


        // PlayerX = random(MIN_X, MAX_X);
        // PlayerY = random(MIN_Y, MAX_Y);

        while(!game_over) {
    
            update_physics();
            // move_sprite(0, PlayerX, PlayerY);
            // move_sprite(1, old_PlayerX, old_PlayerY);
            move_sprite(0, PlayerPos[0].h, PlayerPos[1].h);
            move_sprite(1, old_PlayerPos[0].h, old_PlayerPos[1].h);

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