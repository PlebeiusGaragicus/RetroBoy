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

#include "actor.h"


const uint8_t ENEMY_SPAWN_CONFIG[MAX_ENEMIES] = {
    ENEMY_TYPE_COIN_CHASER,    // Enemy 0
    ENEMY_TYPE_PLAYER_CHASER,  // Enemy 1
    ENEMY_TYPE_COIN_CHASER,    // Enemy 2
    ENEMY_TYPE_PLAYER_CHASER,  // Enemy 3
    ENEMY_TYPE_PLAYER_CHASER,  // Enemy 4
    ENEMY_TYPE_WANDERER,       // Enemy 5
    ENEMY_TYPE_WANDERER        // Enemy 6
};




uint8_t active_enemies = 0;     // Track number of currently active enemies
uint8_t spawn_timer = 0;        // Timer for spawning enemies


Enemy enemies[MAX_ENEMIES];


fixed PlayerPos[2];
int16_t VelX = 0;
int16_t VelY = 0;
uint8_t PlayerSpriteIndex = 0;


fixed Coin[2];
int16_t coin_vel_x = 0;
int16_t coin_vel_y = 0;





// #############################################################################
// INTERNAL FUNCTIONS
// #############################################################################


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

// void sort_enemies_by_x() {
//     // Simple bubble sort - for small number of enemies this is fine
//     for(uint8_t i = 0; i < active_enemies - 1; i++) {
//         for(uint8_t j = 0; j < active_enemies - i - 1; j++) {
//             if(enemies[j].pos[0].b.h > enemies[j + 1].pos[0].b.h) {
//                 Enemy temp;
//                 temp = enemies[j];
//                 enemies[j] = enemies[j + 1];
//                 enemies[j + 1] = temp;
//             }
//         }
//     }
// }

// #############################################################################
// External functions
// #############################################################################


void hide_all_enemies() {
    for(uint8_t i = 0; i < MAX_ENEMIES; i++) {
        move_sprite(i + 2, 0, 0);  // Move sprites off-screen
    }
}



void update_player_physics(uint8_t key) {

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

void move_coin_to_safe_position() {
    fixed new_coin_x, new_coin_y;
    get_safe_position(&new_coin_x, &new_coin_y);
    Coin[0] = new_coin_x;
    Coin[1] = new_coin_y;
    move_sprite(1, Coin[0].b.h, Coin[1].b.h);
}


void handle_player_coin_collision() {
    int16_t x_dist = PlayerPos[0].b.h - Coin[0].b.h;
    int16_t y_dist = PlayerPos[1].b.h - Coin[1].b.h;
    if (x_dist < 0) x_dist = -x_dist;
    if (y_dist < 0) y_dist = -y_dist;
    if (x_dist < 6 && y_dist < 6) {
        move_coin_to_safe_position();
        player_score++;
        display_scores();
        boop();
    }
}


void update_coin_chaser(Enemy* enemy) {
    // Calculate direction to coin
    if (Coin[0].b.h > enemy->pos[0].b.h) 
        enemy->vel_x += enemy->speed/16;
    else if (Coin[0].b.h < enemy->pos[0].b.h)
        enemy->vel_x -= enemy->speed/16;

    if (Coin[1].b.h > enemy->pos[1].b.h)
        enemy->vel_y += enemy->speed/16;
    else if (Coin[1].b.h < enemy->pos[1].b.h)
        enemy->vel_y -= enemy->speed/16;
    
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
    // Move toward player
    if (PlayerPos[0].b.h > enemy->pos[0].b.h) enemy->pos[0].w += enemy->speed;
    if (PlayerPos[0].b.h < enemy->pos[0].b.h) enemy->pos[0].w -= enemy->speed;
    if (PlayerPos[1].b.h > enemy->pos[1].b.h) enemy->pos[1].w += enemy->speed;
    if (PlayerPos[1].b.h < enemy->pos[1].b.h) enemy->pos[1].w -= enemy->speed;
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



void handle_enemy_collisions() {
    // Check each enemy against others within a reasonable distance
    for(uint8_t i = 0; i < active_enemies; i++) {
        // Check coin collision
        int16_t enemy_coin_x_dist = abs(enemies[i].pos[0].b.h - Coin[0].b.h);
        int16_t enemy_coin_y_dist = abs(enemies[i].pos[1].b.h - Coin[1].b.h);

        if (enemy_coin_x_dist < 6 && enemy_coin_y_dist < 6) {
            move_coin_to_safe_position();
            enemy_score++;
            display_scores();
            beedledo();
        }

        // Check player collision
        int16_t enemy_x_dist = abs(PlayerPos[0].b.h - enemies[i].pos[0].b.h);
        int16_t enemy_y_dist = abs(PlayerPos[1].b.h - enemies[i].pos[1].b.h);

        if (enemy_x_dist < 6 && enemy_y_dist < 6) {
            switch(enemies[i].type) {
                case ENEMY_TYPE_WANDERER:
                case ENEMY_TYPE_COIN_CHASER:
                    // Store original velocities
                    int16_t orig_player_vel_x = VelX;
                    int16_t orig_player_vel_y = VelY;
                    int16_t orig_enemy_vel_x = enemies[i].vel_x;
                    int16_t orig_enemy_vel_y = enemies[i].vel_y;

                    // Simple collision response - exchange velocities
                    if (PlayerPos[0].b.h < enemies[i].pos[0].b.h) {
                        VelX = -abs(orig_player_vel_x);
                        enemies[i].vel_x = abs(orig_enemy_vel_x);
                        PlayerPos[0].w -= 256;
                        enemies[i].pos[0].w += 256;
                    } else {
                        VelX = abs(orig_player_vel_x);
                        enemies[i].vel_x = -abs(orig_enemy_vel_x);
                        PlayerPos[0].w += 256;
                        enemies[i].pos[0].w -= 256;
                    }

                    if (PlayerPos[1].b.h < enemies[i].pos[1].b.h) {
                        VelY = -abs(orig_player_vel_y);
                        enemies[i].vel_y = abs(orig_enemy_vel_y);
                        PlayerPos[1].w -= 256;
                        enemies[i].pos[1].w += 256;
                    } else {
                        VelY = abs(orig_player_vel_y);
                        enemies[i].vel_y = -abs(orig_enemy_vel_y);
                        PlayerPos[1].w += 256;
                        enemies[i].pos[1].w -= 256;
                    }

                    bap();
                    break;

                case ENEMY_TYPE_PLAYER_CHASER:
                    // if (TESTING) {
                        // boop();
                    // } else {
                        game_over = TRUE;
                        hide_all_enemies();
                        beedledo();
                    // }
                    break;
            }
        }

        // Check collisions with other enemies
        for(uint8_t j = i + 1; j < active_enemies; j++) {
            int16_t dx = abs(enemies[i].pos[0].b.h - enemies[j].pos[0].b.h);
            int16_t dy = abs(enemies[i].pos[1].b.h - enemies[j].pos[1].b.h);

            // Only process if they're close enough
            if (dx < 16 && dy < 8) {
                // Store original velocities
                int16_t orig_enemy1_vel_x = enemies[i].vel_x;
                int16_t orig_enemy1_vel_y = enemies[i].vel_y;
                int16_t orig_enemy2_vel_x = enemies[j].vel_x;
                int16_t orig_enemy2_vel_y = enemies[j].vel_y;

                // Simple collision response - push apart and exchange velocities
                if (enemies[i].pos[0].b.h < enemies[j].pos[0].b.h) {
                    enemies[i].vel_x = -abs(orig_enemy1_vel_x);
                    enemies[j].vel_x = abs(orig_enemy2_vel_x);
                    enemies[i].pos[0].w -= 256;
                    enemies[j].pos[0].w += 256;
                } else {
                    enemies[i].vel_x = abs(orig_enemy1_vel_x);
                    enemies[j].vel_x = -abs(orig_enemy2_vel_x);
                    enemies[i].pos[0].w += 256;
                    enemies[j].pos[0].w -= 256;
                }

                if (enemies[i].pos[1].b.h < enemies[j].pos[1].b.h) {
                    enemies[i].vel_y = -abs(orig_enemy1_vel_y);
                    enemies[j].vel_y = abs(orig_enemy2_vel_y);
                    enemies[i].pos[1].w -= 256;
                    enemies[j].pos[1].w += 256;
                } else {
                    enemies[i].vel_y = abs(orig_enemy1_vel_y);
                    enemies[j].vel_y = -abs(orig_enemy2_vel_y);
                    enemies[i].pos[1].w += 256;
                    enemies[j].pos[1].w -= 256;
                }
            }
        }
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
