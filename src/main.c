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
#define MIN_X 8
#define MAX_X 160
#define MIN_Y 16
#define MAX_Y 152

BOOLEAN KEY_B_PRESSED = FALSE;

BOOLEAN game_over = FALSE;
uint8_t level = 0;
uint8_t score = 0;



#define TOP_SPEED 700
#define SPEED_CHANGE 12
#define PLAYER_SPEED_FRICTION 2

// [0] is X, [1] is Y
fixed PlayerPos[2];
int16_t VelX = 0;
int16_t VelY = 0;
uint8_t PlayerSpriteIndex = 0;

fixed Coin[2];
int16_t coin_vel_x = 0;
int16_t coin_vel_y = 0;



#define COIN_CHASER_SPEED 75    // Slower, methodical movement
#define PLAYER_CHASER_SPEED 100 // Medium speed, persistent follower
#define WANDERER_SPEED 150      // Faster but erratic movement

#define MAX_ENEMIES 3
#define ENEMY_TYPE_COIN_CHASER 0
#define ENEMY_TYPE_PLAYER_CHASER 1
#define ENEMY_TYPE_WANDERER 2

#define SPAWN_DELAY 60          // Frames between enemy spawns
#define MIN_SPAWN_DISTANCE 32   // Minimum distance from player for spawns
#define MIN_COIN_DISTANCE 24    // Minimum distance for coin respawns

uint8_t active_enemies = 0;     // Track number of currently active enemies
uint8_t spawn_timer = 0;        // Timer for spawning enemies

typedef struct {
    fixed pos[2];        // Position (x,y)
    uint8_t sprite_id;   // Sprite index
    uint8_t type;        // Enemy behavior type
    int16_t vel_x;       // Velocity X
    int16_t vel_y;       // Velocity Y
    uint8_t speed;       // Movement speed
} Enemy;

Enemy enemies[MAX_ENEMIES];



BOOLEAN is_too_close_to_player(fixed pos_x, fixed pos_y) {
    int16_t x_dist = pos_x.h - PlayerPos[0].h;
    int16_t y_dist = pos_y.h - PlayerPos[1].h;
    if (x_dist < 0) x_dist = -x_dist;
    if (y_dist < 0) y_dist = -y_dist;
    return (x_dist < MIN_SPAWN_DISTANCE && y_dist < MIN_SPAWN_DISTANCE);
}

BOOLEAN is_too_close_to_enemies(fixed pos_x, fixed pos_y) {
    for(uint8_t i = 0; i < active_enemies; i++) {
        int16_t x_dist = pos_x.h - enemies[i].pos[0].h;
        int16_t y_dist = pos_y.h - enemies[i].pos[1].h;
        if (x_dist < 0) x_dist = -x_dist;
        if (y_dist < 0) y_dist = -y_dist;
        if (x_dist < MIN_SPAWN_DISTANCE && y_dist < MIN_SPAWN_DISTANCE)
            return TRUE;
    }
    return FALSE;
}

void get_safe_position(fixed* pos_x, fixed* pos_y) {
    do {
        pos_x->w = (uint16_t)random(MIN_X, MAX_X) << 8;
        pos_y->w = (uint16_t)random(MIN_Y, MAX_Y) << 8;
    } while(is_too_close_to_player(*pos_x, *pos_y) || 
            is_too_close_to_enemies(*pos_x, *pos_y));
}



void init_enemy(uint8_t index, uint8_t type) {

    set_sprite_prop(index + 2, S_PRIORITY); // Make sure sprite is visible


    fixed pos_x, pos_y;
    get_safe_position(&pos_x, &pos_y);
    
    enemies[index].pos[0] = pos_x;
    enemies[index].pos[1] = pos_y;
    enemies[index].sprite_id = index + 2;
    enemies[index].type = type;
    enemies[index].vel_x = 0;
    enemies[index].vel_y = 0;

    switch(type) {
        case ENEMY_TYPE_COIN_CHASER:
            set_sprite_tile(enemies[index].sprite_id, 2);
            enemies[index].speed = COIN_CHASER_SPEED;
            break;
        case ENEMY_TYPE_PLAYER_CHASER:
            set_sprite_tile(enemies[index].sprite_id, 3);
            enemies[index].speed = PLAYER_CHASER_SPEED;
            break;
        case ENEMY_TYPE_WANDERER:
            set_sprite_tile(enemies[index].sprite_id, 4);
            enemies[index].speed = WANDERER_SPEED;
            break;
        default:
            set_sprite_tile(enemies[index].sprite_id, 2);
            enemies[index].speed = COIN_CHASER_SPEED;
            break;
    }
    
    move_sprite(enemies[index].sprite_id, 
                enemies[index].pos[0].h, 
                enemies[index].pos[1].h);
}



// used when player bounces off a wall
// int16_t reduce_velocity(int16_t vel) {
//     return (vel * 3) >> 2;  // Multiply by 0.75
// }

int16_t reduce_velocity(int16_t vel) {
    return (vel >> 2);  // Multiply by 0.5
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
            VelX -= PLAYER_SPEED_FRICTION;
        else if (VelX < 0)
            VelX += PLAYER_SPEED_FRICTION;
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
            VelY -= PLAYER_SPEED_FRICTION;
        else if (VelY < 0)
            // VelY++;
            VelY += PLAYER_SPEED_FRICTION;
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


void update_coin_chaser(Enemy* enemy) {
    // Move toward coin
    if (Coin[0].h > enemy->pos[0].h) enemy->pos[0].w += enemy->speed;
    if (Coin[0].h < enemy->pos[0].h) enemy->pos[0].w -= enemy->speed;
    if (Coin[1].h > enemy->pos[1].h) enemy->pos[1].w += enemy->speed;
    if (Coin[1].h < enemy->pos[1].h) enemy->pos[1].w -= enemy->speed;
}

void update_player_chaser(Enemy* enemy) {
    // Move toward player
    if (PlayerPos[0].h > enemy->pos[0].h) enemy->pos[0].w += enemy->speed;
    if (PlayerPos[0].h < enemy->pos[0].h) enemy->pos[0].w -= enemy->speed;
    if (PlayerPos[1].h > enemy->pos[1].h) enemy->pos[1].w += enemy->speed;
    if (PlayerPos[1].h < enemy->pos[1].h) enemy->pos[1].w -= enemy->speed;
}

void update_wanderer(Enemy* enemy) {
    // Random movement
    if(sys_time % 60 == 0) {
        enemy->vel_x = random(-enemy->speed, enemy->speed);
        enemy->vel_y = random(-enemy->speed, enemy->speed);
    }
    enemy->pos[0].w += enemy->vel_x;
    enemy->pos[1].w += enemy->vel_y;
}

void constrain_to_boundaries(Enemy* enemy) {
    if (enemy->pos[0].h < MIN_X) {
        enemy->pos[0].w = ((uint16_t)MIN_X << 8);
        enemy->vel_x = -enemy->vel_x;  // Bounce off walls for wanderer
    }
    if (enemy->pos[0].h > MAX_X) {
        enemy->pos[0].w = ((uint16_t)MAX_X << 8);
        enemy->vel_x = -enemy->vel_x;
    }
    if (enemy->pos[1].h < MIN_Y) {
        enemy->pos[1].w = ((uint16_t)MIN_Y << 8);
        enemy->vel_y = -enemy->vel_y;
    }
    if (enemy->pos[1].h > MAX_Y) {
        enemy->pos[1].w = ((uint16_t)MAX_Y << 8);
        enemy->vel_y = -enemy->vel_y;
    }
}

void update_enemies() {
    for(uint8_t i = 0; i < active_enemies; i++) {
    // for(uint8_t i = 0; i < MAX_ENEMIES; i++) {
        switch(enemies[i].type) {
            case ENEMY_TYPE_COIN_CHASER:
                update_coin_chaser(&enemies[i]);
                break;
            case ENEMY_TYPE_PLAYER_CHASER:
                update_player_chaser(&enemies[i]);
                break;
            case ENEMY_TYPE_WANDERER:
                update_wanderer(&enemies[i]);
                break;
        }
        
        // Apply boundary constraints
        constrain_to_boundaries(&enemies[i]);
        
        // Update sprite position
        move_sprite(enemies[i].sprite_id, 
                   enemies[i].pos[0].h, 
                   enemies[i].pos[1].h);
    }
}



// void ready_start() {
//     uint8_t x, y;
//     uint8_t key;
//     uint8_t counter = 120;

//     // while(1) {
//     while(counter--) {
//         // key = joypad();
//         // if (key & (J_START | J_A | J_B))
//         // if (key & (J_A | J_B))
//             // break;

//         if(sys_time % 5 == 0) {
//             x = random(MIN_X, MAX_X);
//             y = random(MIN_Y, MAX_Y);
//             move_sprite(0, x, y);
//         }
//         vsync();
//     }

//     boop();

//     // Initialize fixed-point position with proper sub-pixel precision
//     PlayerPos[0].w = (uint16_t)x << 8;
//     PlayerPos[1].w = (uint16_t)y << 8;

//     // Initialize coin position
//     Coin[0].w = (uint16_t)random(MIN_X, MAX_X) << 8;
//     Coin[1].w = (uint16_t)random(MIN_Y, MAX_Y) << 8;
//     move_sprite(1, Coin[0].h, Coin[1].h);

//     init_enemy(0, ENEMY_TYPE_COIN_CHASER);
//     init_enemy(1, ENEMY_TYPE_PLAYER_CHASER);
//     init_enemy(2, ENEMY_TYPE_WANDERER);

//     // Reset velocities
//     VelX = 0;
//     VelY = 0;
    
//     game_over = FALSE;
// }


void ready_start() {
    uint8_t x, y;
    uint8_t counter = 50;

    while(counter--) {
        if(sys_time % 5 == 0) {
            x = random(MIN_X, MAX_X);
            y = random(MIN_Y, MAX_Y);
            move_sprite(0, x, y);
        }
        vsync();
    }

    boop();

    PlayerPos[0].w = (uint16_t)x << 8;
    PlayerPos[1].w = (uint16_t)y << 8;

    // Initialize coin position safely
    fixed coin_x, coin_y;
    get_safe_position(&coin_x, &coin_y);
    Coin[0] = coin_x;
    Coin[1] = coin_y;
    move_sprite(1, Coin[0].h, Coin[1].h);

    // Reset enemy spawning
    active_enemies = 0;
    spawn_timer = SPAWN_DELAY;

    // Reset velocities
    VelX = 0;
    VelY = 0;
    
    game_over = FALSE;
}

void hide_all_enemies() {
    for(uint8_t i = 0; i < MAX_ENEMIES; i++) {
        move_sprite(i + 2, 0, 0);  // Move sprites off-screen
    }
}


void load_sprites()
{
    show_screen_border();


    set_sprite_data(0, 1, Smiles);
    // set_sprite_data(0, 1, Pointer);
    set_sprite_tile(0, 0);

    set_sprite_data(1, 1, CoinSprite_light);
    set_sprite_tile(1, 1);
    //TODO: research this later...
    // set_sprite_prop(1,0);

    // Coin chaser sprite
    set_sprite_data(2, 1, Spoky);
    set_sprite_tile(2, 2);

    // Player chaser sprite
    set_sprite_data(3, 1, Frowns);
    set_sprite_tile(3, 3);

    // Wanderer sprite
    set_sprite_data(4, 1, Gooby);
    set_sprite_tile(4, 4);
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

            // Add this section for enemy spawning
            if (active_enemies < MAX_ENEMIES) {
                if (spawn_timer > 0) {
                    spawn_timer--;
                } else {
                    init_enemy(active_enemies, active_enemies); // Use index as type
                    active_enemies++;
                    spawn_timer = SPAWN_DELAY;
                }
            }

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
                fixed new_coin_x, new_coin_y;
                get_safe_position(&new_coin_x, &new_coin_y);
                Coin[0] = new_coin_x;
                Coin[1] = new_coin_y;
                move_sprite(1, Coin[0].h, Coin[1].h);
                boop();
            }
            move_sprite(0, PlayerPos[0].h, PlayerPos[1].h);


            update_enemies();
            // for(uint8_t i = 0; i < MAX_ENEMIES; i++) {
            for(uint8_t i = 0; i < active_enemies; i++) {
                int16_t enemy_x_dist = PlayerPos[0].h - enemies[i].pos[0].h;
                int16_t enemy_y_dist = PlayerPos[1].h - enemies[i].pos[1].h;
                
                // Make distances positive if negative
                if (enemy_x_dist < 0) enemy_x_dist = -enemy_x_dist;
                if (enemy_y_dist < 0) enemy_y_dist = -enemy_y_dist;
                
                if (enemy_x_dist < 6 && enemy_y_dist < 6) {
                    game_over = TRUE;
                    hide_all_enemies();
                    beedledo();
                    break;
                }

                // Check enemy-coin collisions
                int16_t enemy_coin_x_dist = enemies[i].pos[0].h - Coin[0].h;
                int16_t enemy_coin_y_dist = enemies[i].pos[1].h - Coin[1].h;
                
                if (enemy_coin_x_dist < 0) enemy_coin_x_dist = -enemy_coin_x_dist;
                if (enemy_coin_y_dist < 0) enemy_coin_y_dist = -enemy_coin_y_dist;
                
                if (enemy_coin_x_dist < 6 && enemy_coin_y_dist < 6) {
                    fixed new_coin_x, new_coin_y;
                    get_safe_position(&new_coin_x, &new_coin_y);
                    Coin[0] = new_coin_x;
                    Coin[1] = new_coin_y;
                    move_sprite(1, Coin[0].h, Coin[1].h);
                    beedledo();
                }
            }

            vsync();
        }

        // Game over screen
    }
}