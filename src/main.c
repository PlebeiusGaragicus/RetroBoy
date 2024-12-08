#include <stdint.h>
#include <stdio.h>
#include <types.h>  // for fixed type

#include <gb/gb.h>
#include <gb/cgb.h>
#include <gb/drawing.h>

#include "sprites.h"
#include "util.h"
#include "intro.h"
#include "audio.h"

// #####################################################################################


// TODO double check these
#define MIN_X 8
#define MAX_X 160
#define MIN_Y 8
#define MAX_Y 152

BOOLEAN KEY_B_PRESSED = FALSE;
// BOOLEAN KEY_START_PRESSED = FALSE;

BOOLEAN game_over = FALSE;
uint8_t level = 0;



#define TOP_SPEED 900
#define SPEED_CHANGE 15

// [0] is X, [1] is Y
fixed PlayerPos[2];
int16_t VelX = 0;
int16_t VelY = 0;
uint8_t PlayerSpriteIndex = 0;

fixed Coin[2];
int16_t coin_vel_x = 0;
int16_t coin_vel_y = 0;

#define ENEMY_SPEED_STARTING 100

fixed Enemy[2];
int16_t enemy_vel_x = 0;
int16_t enemy_vel_y = 0;
uint8_t EnemyMaxSpeed = ENEMY_SPEED_STARTING;


// Helper function to reduce velocity by ~70%
// int8_t reduce_velocity(int8_t vel) {
//     int8_t quarter = vel >> 2;    // divide by 4
//     int8_t sixteenth = vel >> 4;  // divide by 16
//     return quarter + sixteenth;
// }
int16_t reduce_velocity(int16_t vel) {
    return (vel * 3) >> 2;  // Multiply by 0.75
}


void update_physics(uint8_t key) {

    if (key & J_LEFT) {
        if (VelX > -TOP_SPEED) {
            VelX -= SPEED_CHANGE;
        }
    }
    else if (key & J_RIGHT) {
        if (VelX < TOP_SPEED) {
            VelX += SPEED_CHANGE;
        }
    }
    else // Apply friction when no input
    {
        if (VelX > 0)
            VelX -= 1;
        else if (VelX < 0)
            VelX += 1;
    }

    if (key & J_UP) {
        if (VelY > -TOP_SPEED) {
            VelY -= SPEED_CHANGE;
        }
    }
    else if (key & J_DOWN) {
        if (VelY < TOP_SPEED) {
            VelY += SPEED_CHANGE;
        }
    }
    else // Apply friction when no input
    {
        if (VelY > 0)
            // VelY--;
            VelY -= 1;
        else if (VelY < 0)
            // VelY++;
            VelY += 1;
    }

    // Update position using sub-pixel movement
    PlayerPos[0].w += VelX;
    PlayerPos[1].w += VelY;

    // Screen boundary collision using the high byte
    if (PlayerPos[0].h < MIN_X) {
        PlayerPos[0].w = ((uint16_t)MIN_X << 8);
        VelX = -reduce_velocity(VelX);
    }
    if (PlayerPos[0].h > MAX_X) {
        PlayerPos[0].w = ((uint16_t)MAX_X << 8);
        VelX = -reduce_velocity(VelX);
    }
    if (PlayerPos[1].h < MIN_Y) {
        PlayerPos[1].w = ((uint16_t)MIN_Y << 8);
        VelY = -reduce_velocity(VelY);
    }
    if (PlayerPos[1].h > MAX_Y) {
        PlayerPos[1].w = ((uint16_t)MAX_Y << 8);
        VelY = -reduce_velocity(VelY);
    }

    // Move sprite using only the high byte (integer portion)
    move_sprite(0, PlayerPos[0].h, PlayerPos[1].h);
}


void update_enemy() {
    // Calculate direction to coin
    int16_t dx = Coin[0].h - Enemy[0].h;
    int16_t dy = Coin[1].h - Enemy[1].h;
    
    // Update enemy position
    if (dx > 0) Enemy[0].w += ENEMY_SPEED;
    if (dx < 0) Enemy[0].w -= ENEMY_SPEED;
    if (dy > 0) Enemy[1].w += ENEMY_SPEED;
    if (dy < 0) Enemy[1].w -= ENEMY_SPEED;
    
    // Screen boundary collision
    if (Enemy[0].h < MIN_X) Enemy[0].w = ((uint16_t)MIN_X << 8);
    if (Enemy[0].h > MAX_X) Enemy[0].w = ((uint16_t)MAX_X << 8);
    if (Enemy[1].h < MIN_Y) Enemy[1].w = ((uint16_t)MIN_Y << 8);
    if (Enemy[1].h > MAX_Y) Enemy[1].w = ((uint16_t)MAX_Y << 8);
    
    // Update sprite position
    move_sprite(2, Enemy[0].h, Enemy[1].h);
}

void ready_start() {
    uint8_t x, y;
    uint8_t key;
    uint8_t counter = 120;

    // while(1) {
    while(counter--) {
        // key = joypad();
        // if (key & (J_START | J_A | J_B))
        // if (key & (J_A | J_B))
            // break;

        if(sys_time % 5 == 0) {
            x = random(MIN_X, MAX_X);
            y = random(MIN_Y, MAX_Y);
            move_sprite(0, x, y);
        }
        vsync();
    }

    boop();

    // Initialize fixed-point position with proper sub-pixel precision
    PlayerPos[0].w = (uint16_t)x << 8;
    PlayerPos[1].w = (uint16_t)y << 8;
    
    // Initialize coin position
    Coin[0].w = (uint16_t)random(MIN_X, MAX_X) << 8;
    Coin[1].w = (uint16_t)random(MIN_Y, MAX_Y) << 8;
    move_sprite(1, Coin[0].h, Coin[1].h);

    // Add this in ready_start() after coin initialization
    Enemy[0].w = (uint16_t)random(MIN_X, MAX_X) << 8;
    Enemy[1].w = (uint16_t)random(MIN_Y, MAX_Y) << 8;
    move_sprite(2, Enemy[0].h, Enemy[1].h);

    // Reset velocities
    VelX = 0;
    VelY = 0;
    
    game_over = FALSE;
}

void load_sprites()
{
    show_screen_border();


    set_sprite_data(0, 1, Smiles);
    // set_sprite_data(0, 1, Pointer);
    set_sprite_tile(0,0);

    set_sprite_data(1, 1, CoinSprite_light);
    set_sprite_tile(1,1);
    // set_sprite_prop(1,0); //TODO: figure this out.

    set_sprite_data(2, 1, Spoky);
    set_sprite_tile(2,2);
}

void pause_screen() {
    HIDE_SPRITES;
    load_justin();
    boop();

    // wait for start to be released
    while(joypad() & J_START)
        vsync();


    // wait for start to be pressed
    while(1) {
        if (joypad() & J_START)
            break;
        vsync();
    }

    while(joypad() & J_START)
        vsync();

    boop();
    show_screen_border();
    SHOW_SPRITES;
}

// #####################################################################################
void main()
{
    splash_screen();

    load_sprites();

    // show_screen_border();

    uint8_t key;
    while( TRUE ) {
        ready_start();

        while( !game_over ) {
            key = joypad();



            if ( key & J_START ) {
                // KEY_START_PRESSED = TRUE;
                pause_screen();
                continue;
            }



            if (key & J_B) {
                if (KEY_B_PRESSED == FALSE) {
                    KEY_B_PRESSED = TRUE;
                    bap();
                }
            } else {
                // If J_B is not pressed, make sure KEY_B_PRESSED is set to FALSE
                KEY_B_PRESSED = FALSE;
            }



            update_physics(key);
            int16_t x_dist = PlayerPos[0].h - Coin[0].h;
            int16_t y_dist = PlayerPos[1].h - Coin[1].h;
            if (x_dist < 0) x_dist = -x_dist;
            if (y_dist < 0) y_dist = -y_dist;
            if (x_dist < 6 && y_dist < 6)
            {
                Coin[0].h = random(MIN_X, MAX_X);
                Coin[1].h = random(MIN_Y, MAX_Y);
                move_sprite(1, Coin[0].h, Coin[1].h);
                boop();
            }
            move_sprite(0, PlayerPos[0].h, PlayerPos[1].h);




            update_enemy();
            // Add collision detection with enemy
            int16_t enemy_x_dist = PlayerPos[0].h - Enemy[0].h;
            int16_t enemy_y_dist = PlayerPos[1].h - Enemy[1].h;
            if (enemy_x_dist < 0) enemy_x_dist = -enemy_x_dist;
            if (enemy_y_dist < 0) enemy_y_dist = -enemy_y_dist;
            if (enemy_x_dist < 6 && enemy_y_dist < 6) {
                game_over = TRUE;
                beedledo();  // Assuming you have a death sound
            }


            // Enemy-coin collision detection
            int16_t enemy_coin_x_dist = Enemy[0].h - Coin[0].h;
            int16_t enemy_coin_y_dist = Enemy[1].h - Coin[1].h;
            if (enemy_coin_x_dist < 0) enemy_coin_x_dist = -enemy_coin_x_dist;
            if (enemy_coin_y_dist < 0) enemy_coin_y_dist = -enemy_coin_y_dist;
            if (enemy_coin_x_dist < 6 && enemy_coin_y_dist < 6)
            {
                // Enemy got the coin
                Coin[0].h = random(MIN_X, MAX_X);
                Coin[1].h = random(MIN_Y, MAX_Y);
                move_sprite(1, Coin[0].h, Coin[1].h);
                // bap();  // or whatever sound you prefer
                // boop();
                beedledo();

                // Optional: Increment enemy score or trigger game over
                // game_over = TRUE;  // if you want enemy catching coin to end game
            }


            vsync();
        }
    }
}

