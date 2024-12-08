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

#include "level.h"


const BOOLEAN TESTING = TRUE;


// #####################################################################################
#define MIN_X 8
#define MAX_X 160
#define MIN_Y 16
#define MAX_Y 152

BOOLEAN KEY_B_PRESSED = FALSE;




#define TOP_SPEED 600
#define SPEED_CHANGE 8
#define PLAYER_SPEED_FRICTION 2

// [0] is X, [1] is Y
fixed PlayerPos[2];
int16_t VelX = 0;
int16_t VelY = 0;
uint8_t PlayerSpriteIndex = 0;

fixed Coin[2];
int16_t coin_vel_x = 0;
int16_t coin_vel_y = 0;





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
    if (key & J_A) {
        PlayerPos[0].w += VelX >> 1;
        PlayerPos[1].w += VelY >> 1;
    } else {
        PlayerPos[0].w += VelX;
        PlayerPos[1].w += VelY;
    }

    // Screen boundary collision using the high byte
    if (PlayerPos[0].b.h < MIN_X) {
        PlayerPos[0].w = ((uint16_t)MIN_X << 8);
        VelX = -reduce_velocity(VelX);
    }
    if (PlayerPos[0].b.h > MAX_X) {
        PlayerPos[0].w = ((uint16_t)MAX_X << 8);
        VelX = -reduce_velocity(VelX);
    }
    if (PlayerPos[1].b.h < MIN_Y) {
        PlayerPos[1].w = ((uint16_t)MIN_Y << 8);
        VelY = -reduce_velocity(VelY);
    }
    if (PlayerPos[1].b.h > MAX_Y) {
        PlayerPos[1].w = ((uint16_t)MAX_Y << 8);
        VelY = -reduce_velocity(VelY);
    }

    // Move sprite using only the high byte (integer portion)
    move_sprite(0, PlayerPos[0].b.h, PlayerPos[1].b.h);
}


// #####################################################################################

// #####################################################################################

// #####################################################################################



// ... include files and player variables above...

#define ATTRACTION_FORCE 200    // Force of attraction to target (coin/player)
#define SEPARATION_FORCE 80   // Force to keep enemies apart
#define SEPARATION_RADIUS 15   // Distance at which separation starts

#define COIN_CHASER_SPEED 90    // Slower, methodical movement
#define PLAYER_CHASER_SPEED 100 // Medium speed, persistent follower
#define WANDERER_SPEED 150      // Faster but erratic movement

#define MAX_ENEMIES 4
#define ENEMY_TYPE_COIN_CHASER 0
#define ENEMY_TYPE_PLAYER_CHASER 1
#define ENEMY_TYPE_WANDERER 2

const uint8_t ENEMY_SPAWN_CONFIG[MAX_ENEMIES] = {
    ENEMY_TYPE_COIN_CHASER,    // Enemy 0
    ENEMY_TYPE_PLAYER_CHASER,    // Enemy 1
    ENEMY_TYPE_COIN_CHASER,    // Enemy 2
    ENEMY_TYPE_PLAYER_CHASER,  // Enemy 3
    ENEMY_TYPE_PLAYER_CHASER,  // Enemy 4
    ENEMY_TYPE_WANDERER,       // Enemy 5
    ENEMY_TYPE_WANDERER        // Enemy 6
};



// Define how many of each type we want
// #define NUM_COIN_CHASERS 3
// #define NUM_PLAYER_CHASERS 2
// #define NUM_WANDERERS 2

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



void calculate_separation_force(Enemy* current_enemy, int16_t* force_x, int16_t* force_y) {
    *force_x = 0;
    *force_y = 0;
    
    for(uint8_t i = 0; i < active_enemies; i++) {
        // Skip self
        if (&enemies[i] == current_enemy) continue;
        
        int16_t dx = current_enemy->pos[0].b.h - enemies[i].pos[0].b.h;
        int16_t dy = current_enemy->pos[1].b.h - enemies[i].pos[1].b.h;
        int16_t dist = abs(dx) + abs(dy); // Manhattan distance for simplicity
        
        if (dist < SEPARATION_RADIUS) {
            // Add separation force inversely proportional to distance
            if (dx != 0) *force_x += (SEPARATION_FORCE * dx) / dist;
            if (dy != 0) *force_y += (SEPARATION_FORCE * dy) / dist;
        }
    }
}




BOOLEAN is_too_close_to_player(fixed pos_x, fixed pos_y) {
    int16_t x_dist = pos_x.b.h - PlayerPos[0].b.h;
    int16_t y_dist = pos_y.b.h - PlayerPos[1].b.h;
    if (x_dist < 0) x_dist = -x_dist;
    if (y_dist < 0) y_dist = -y_dist;
    return (x_dist < MIN_SPAWN_DISTANCE && y_dist < MIN_SPAWN_DISTANCE);
}

BOOLEAN is_too_close_to_enemies(fixed pos_x, fixed pos_y) {
    for(uint8_t i = 0; i < active_enemies; i++) {
        int16_t x_dist = pos_x.b.h - enemies[i].pos[0].b.h;
        int16_t y_dist = pos_y.b.h - enemies[i].pos[1].b.h;
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


void handle_player_coin_collision() {
    int16_t x_dist = PlayerPos[0].b.h - Coin[0].b.h;
    int16_t y_dist = PlayerPos[1].b.h - Coin[1].b.h;
    if (x_dist < 0) x_dist = -x_dist;
    if (y_dist < 0) y_dist = -y_dist;
    if (x_dist < 6 && y_dist < 6) {
        fixed new_coin_x, new_coin_y;
        get_safe_position(&new_coin_x, &new_coin_y);
        Coin[0] = new_coin_x;
        Coin[1] = new_coin_y;
        move_sprite(1, Coin[0].b.h, Coin[1].b.h);
        player_score++;
        display_scores();
        boop();
    }
}





void init_enemy(uint8_t index) {
    uint8_t type = ENEMY_SPAWN_CONFIG[index];
    
    set_sprite_prop(index + 2, S_PRIORITY);

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
    }

    move_sprite(enemies[index].sprite_id,
                enemies[index].pos[0].b.h,
                enemies[index].pos[1].b.h);
}


void hide_all_enemies() {
    for(uint8_t i = 0; i < MAX_ENEMIES; i++) {
        move_sprite(i + 2, 0, 0);  // Move sprites off-screen
    }
}


// void update_coin_chaser(Enemy* enemy) {
//     // Move toward coin
//     if (Coin[0].b.h > enemy->pos[0].b.h) enemy->pos[0].w += enemy->speed;
//     if (Coin[0].b.h < enemy->pos[0].b.h) enemy->pos[0].w -= enemy->speed;
//     if (Coin[1].b.h > enemy->pos[1].b.h) enemy->pos[1].w += enemy->speed;
//     if (Coin[1].b.h < enemy->pos[1].b.h) enemy->pos[1].w -= enemy->speed;
// }
// void update_coin_chaser(Enemy* enemy) {
//     // Calculate direction to coin
//     if (Coin[0].b.h > enemy->pos[0].b.h) 
//         enemy->vel_x += enemy->speed/16;
//     else if (Coin[0].b.h < enemy->pos[0].b.h)
//         enemy->vel_x -= enemy->speed/16;

//     if (Coin[1].b.h > enemy->pos[1].b.h)
//         enemy->vel_y += enemy->speed/16;
//     else if (Coin[1].b.h < enemy->pos[1].b.h)
//         enemy->vel_y -= enemy->speed/16;
    
//     // Cap velocity
//     if (enemy->vel_x > enemy->speed) enemy->vel_x = enemy->speed;
//     if (enemy->vel_x < -enemy->speed) enemy->vel_x = -enemy->speed;
//     if (enemy->vel_y > enemy->speed) enemy->vel_y = enemy->speed;
//     if (enemy->vel_y < -enemy->speed) enemy->vel_y = -enemy->speed;
    
//     // Apply velocity
//     enemy->pos[0].w += enemy->vel_x;
//     enemy->pos[1].w += enemy->vel_y;
// }
// void update_player_chaser(Enemy* enemy) {
//     // Move toward player
//     if (PlayerPos[0].b.h > enemy->pos[0].b.h) enemy->pos[0].w += enemy->speed;
//     if (PlayerPos[0].b.h < enemy->pos[0].b.h) enemy->pos[0].w -= enemy->speed;
//     if (PlayerPos[1].b.h > enemy->pos[1].b.h) enemy->pos[1].w += enemy->speed;
//     if (PlayerPos[1].b.h < enemy->pos[1].b.h) enemy->pos[1].w -= enemy->speed;
// }
// void update_wanderer(Enemy* enemy) {
//     // Random movement
//     if(sys_time % 60 == 0) {
//         enemy->vel_x = random(-enemy->speed, enemy->speed);
//         enemy->vel_y = random(-enemy->speed, enemy->speed);
//     }
//     enemy->pos[0].w += enemy->vel_x;
//     enemy->pos[1].w += enemy->vel_y;
// }
// void update_wanderer(Enemy* enemy) {
//     // Random movement
//     if(sys_time % 60 == 0) {
//         // Initialize with some velocity if it doesn't have any
//         if (enemy->vel_x == 0 && enemy->vel_y == 0) {
//             enemy->vel_x = random(-enemy->speed, enemy->speed);
//             enemy->vel_y = random(-enemy->speed, enemy->speed);
//         } else {
//             // Randomly adjust existing velocity
//             enemy->vel_x += random(-enemy->speed/2, enemy->speed/2);
//             enemy->vel_y += random(-enemy->speed/2, enemy->speed/2);
            
//             // Cap velocity
//             if (enemy->vel_x > enemy->speed) enemy->vel_x = enemy->speed;
//             if (enemy->vel_x < -enemy->speed) enemy->vel_x = -enemy->speed;
//             if (enemy->vel_y > enemy->speed) enemy->vel_y = enemy->speed;
//             if (enemy->vel_y < -enemy->speed) enemy->vel_y = -enemy->speed;
//         }
//     }
    
//     // Apply velocity
//     enemy->pos[0].w += enemy->vel_x;
//     enemy->pos[1].w += enemy->vel_y;
// }

void update_coin_chaser(Enemy* enemy) {
    // Calculate attraction to coin
    int16_t dx = Coin[0].b.h - enemy->pos[0].b.h;
    int16_t dy = Coin[1].b.h - enemy->pos[1].b.h;
    
    // Calculate separation force
    int16_t sep_force_x, sep_force_y;
    calculate_separation_force(enemy, &sep_force_x, &sep_force_y);

    // Combine forces
    enemy->vel_x += (dx * ATTRACTION_FORCE) / 256 + sep_force_x;
    enemy->vel_y += (dy * ATTRACTION_FORCE) / 256 + sep_force_y;

    // Cap velocity
    if (enemy->vel_x > enemy->speed) enemy->vel_x = enemy->speed;
    if (enemy->vel_x < -enemy->speed) enemy->vel_x = -enemy->speed;
    if (enemy->vel_y > enemy->speed) enemy->vel_y = enemy->speed;
    if (enemy->vel_y < -enemy->speed) enemy->vel_y = -enemy->speed;

    // Apply velocity
    enemy->pos[0].w += enemy->vel_x;
    enemy->pos[1].w += enemy->vel_y;
}

void update_player_chaser(Enemy* enemy) {
    // Calculate attraction to player
    int16_t dx = PlayerPos[0].b.h - enemy->pos[0].b.h;
    int16_t dy = PlayerPos[1].b.h - enemy->pos[1].b.h;
    
    // Calculate separation force
    int16_t sep_force_x, sep_force_y;
    calculate_separation_force(enemy, &sep_force_x, &sep_force_y);
    
    // Combine forces
    enemy->vel_x += (dx * ATTRACTION_FORCE) / 256 + sep_force_x;
    enemy->vel_y += (dy * ATTRACTION_FORCE) / 256 + sep_force_y;
    
    // Cap velocity
    if (enemy->vel_x > enemy->speed) enemy->vel_x = enemy->speed;
    if (enemy->vel_x < -enemy->speed) enemy->vel_x = -enemy->speed;
    if (enemy->vel_y > enemy->speed) enemy->vel_y = enemy->speed;
    if (enemy->vel_y < -enemy->speed) enemy->vel_y = -enemy->speed;
    
    // Apply velocity
    enemy->pos[0].w += enemy->vel_x;
    enemy->pos[1].w += enemy->vel_y;
}
void update_wanderer(Enemy* enemy) {
    // Random movement as before
    if(sys_time % 60 == 0) {
        enemy->vel_x += random(-enemy->speed/2, enemy->speed/2);
        enemy->vel_y += random(-enemy->speed/2, enemy->speed/2);
    }
    
    // Add separation force
    int16_t sep_force_x, sep_force_y;
    calculate_separation_force(enemy, &sep_force_x, &sep_force_y);
    enemy->vel_x += sep_force_x;
    enemy->vel_y += sep_force_y;
    
    // Cap velocity
    if (enemy->vel_x > enemy->speed) enemy->vel_x = enemy->speed;
    if (enemy->vel_x < -enemy->speed) enemy->vel_x = -enemy->speed;
    if (enemy->vel_y > enemy->speed) enemy->vel_y = enemy->speed;
    if (enemy->vel_y < -enemy->speed) enemy->vel_y = -enemy->speed;
    
    // Apply velocity
    enemy->pos[0].w += enemy->vel_x;
    enemy->pos[1].w += enemy->vel_y;
}









void constrain_to_boundaries(Enemy* enemy) {
    if (enemy->pos[0].b.h < MIN_X) {
        enemy->pos[0].w = ((uint16_t)MIN_X << 8);
        enemy->vel_x = -enemy->vel_x;  // Bounce off walls for wanderer
    }
    if (enemy->pos[0].b.h > MAX_X) {
        enemy->pos[0].w = ((uint16_t)MAX_X << 8);
        enemy->vel_x = -enemy->vel_x;
    }
    if (enemy->pos[1].b.h < MIN_Y) {
        enemy->pos[1].w = ((uint16_t)MIN_Y << 8);
        enemy->vel_y = -enemy->vel_y;
    }
    if (enemy->pos[1].b.h > MAX_Y) {
        enemy->pos[1].w = ((uint16_t)MAX_Y << 8);
        enemy->vel_y = -enemy->vel_y;
    }
}

void update_enemies() {
    for(uint8_t i = 0; i < active_enemies; i++) {
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
                   enemies[i].pos[0].b.h, 
                   enemies[i].pos[1].b.h);
    }
}



void handle_enemy_collisions() {
    for(uint8_t i = 0; i < active_enemies; i++) {

        int16_t enemy_coin_x_dist = abs(enemies[i].pos[0].b.h - Coin[0].b.h);
        int16_t enemy_coin_y_dist = abs(enemies[i].pos[1].b.h - Coin[1].b.h);

        if (enemy_coin_x_dist < 6 && enemy_coin_y_dist < 6) {
            fixed new_coin_x, new_coin_y;
            get_safe_position(&new_coin_x, &new_coin_y);
            Coin[0] = new_coin_x;
            Coin[1] = new_coin_y;
            move_sprite(1, Coin[0].b.h, Coin[1].b.h);
            enemy_score++;
            display_scores();
            beedledo();
        }

        int16_t enemy_x_dist = abs(PlayerPos[0].b.h - enemies[i].pos[0].b.h);
        int16_t enemy_y_dist = abs(PlayerPos[1].b.h - enemies[i].pos[1].b.h);

        if (enemy_x_dist < 6 && enemy_y_dist < 6) {
            // Different behavior based on enemy type
            switch(enemies[i].type) {
                case ENEMY_TYPE_WANDERER:
                case ENEMY_TYPE_COIN_CHASER:
                    // Store original velocities
                    int16_t orig_player_vel_x = VelX;
                    int16_t orig_player_vel_y = VelY;
                    int16_t orig_enemy_vel_x = enemies[i].vel_x;
                    int16_t orig_enemy_vel_y = enemies[i].vel_y;

                    // Calculate average velocities
                    int16_t avg_vel_x = (orig_player_vel_x + orig_enemy_vel_x) >> 1;
                    int16_t avg_vel_y = (orig_player_vel_y + orig_enemy_vel_y) >> 1;

                    // Apply new velocities in opposite directions
                    VelX = avg_vel_x + ((orig_player_vel_x - avg_vel_x) >> 1);
                    VelY = avg_vel_y + ((orig_player_vel_y - avg_vel_y) >> 1);
                    
                    enemies[i].vel_x = avg_vel_x + ((orig_enemy_vel_x - avg_vel_x) >> 1);
                    enemies[i].vel_y = avg_vel_y + ((orig_enemy_vel_y - avg_vel_y) >> 1);

                    // Reverse directions based on collision position
                    if (PlayerPos[0].b.h < enemies[i].pos[0].b.h) {
                        VelX = -abs(VelX);
                        enemies[i].vel_x = abs(enemies[i].vel_x);
                        PlayerPos[0].w -= 256;
                        enemies[i].pos[0].w += 256;
                    } else {
                        VelX = abs(VelX);
                        enemies[i].vel_x = -abs(enemies[i].vel_x);
                        PlayerPos[0].w += 256;
                        enemies[i].pos[0].w -= 256;
                    }

                    if (PlayerPos[1].b.h < enemies[i].pos[1].b.h) {
                        VelY = -abs(VelY);
                        enemies[i].vel_y = abs(enemies[i].vel_y);
                        PlayerPos[1].w -= 256;
                        enemies[i].pos[1].w += 256;
                    } else {
                        VelY = abs(VelY);
                        enemies[i].vel_y = -abs(enemies[i].vel_y);
                        PlayerPos[1].w += 256;
                        enemies[i].pos[1].w -= 256;
                    }

                    bap();
                    break;

                case ENEMY_TYPE_PLAYER_CHASER:
                    // game_over = TRUE;
                    if (TESTING) {
                        boop();
                    } else {
                        game_over = TRUE;
                        hide_all_enemies();
                        beedledo();
                        break;
                    }
            }
        }
    }
}




void ready_start() {
    uint8_t x, y;
    uint8_t counter = 15;

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
    move_sprite(1, Coin[0].b.h, Coin[1].b.h);

    // Reset enemy spawning
    active_enemies = 0;
    spawn_timer = SPAWN_DELAY;

    // Reset velocities
    VelX = 0;
    VelY = 0;

    reset_score();

    game_over = FALSE;
}



void main() {
    splash_screen(TESTING);
    load_sprites();

    uint8_t key;
    while(TRUE) {
        ready_start();

        while(!game_over) {
            key = joypad();

            if (active_enemies < MAX_ENEMIES) {
                if (spawn_timer > 0) {
                    spawn_timer--;
                } else {
                    init_enemy(active_enemies); // Use index as type
                    active_enemies++;
                    spawn_timer = SPAWN_DELAY;
                }
            }

            if (key & J_START) {
                pause_screen();
                continue;
            }

            if (key & J_B) {
                if (KEY_B_PRESSED == FALSE) {
                    KEY_B_PRESSED = TRUE;
                    bap();
                }
            } else {
                KEY_B_PRESSED = FALSE;
            }
            
            update_physics(key);
            handle_player_coin_collision();

            update_enemies();
            handle_enemy_collisions();

            vsync();
        }
    }
}
