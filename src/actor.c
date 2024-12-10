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




// uint8_t active_enemies = 0;     // Track number of currently active enemies
uint8_t spawn_timer = 0;        // Timer for spawning enemies


Enemy enemies[MAX_ENEMIES];
Player player;  // Global player instance
// uint8_t PlayerSpriteIndex = 0;

fixed Coin[2];
int16_t coin_vel_x = 0;
int16_t coin_vel_y = 0;






BOOLEAN is_too_close_to_player(fixed pos_x, fixed pos_y) {
    int16_t x_dist = pos_x.b.h - player.pos[0].b.h;
    int16_t y_dist = pos_y.b.h - player.pos[1].b.h;
    if (x_dist < 0) x_dist = -x_dist;
    if (y_dist < 0) y_dist = -y_dist;
    return (x_dist < MIN_SPAWN_DISTANCE && y_dist < MIN_SPAWN_DISTANCE);
}

BOOLEAN is_too_close_to_enemies(fixed pos_x, fixed pos_y) {
    for(uint8_t i = 0; i < enemies_spawned; i++) {
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
        enemy->vel_x = -enemy->vel_x;
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


void hide_all_enemies() {
    for(uint8_t i = 0; i < MAX_ENEMIES; i++) {
        move_sprite(i + 2, 0, 0);  // Move sprites off-screen
    }
}



void update_player_physics(uint8_t key) {

    if (key & J_LEFT) {
        if (player.vel_x > -TOP_SPEED) {
            player.vel_x -= SPEED_CHANGE;
        }
    }
    else if (key & J_RIGHT) {
        if (player.vel_x < TOP_SPEED) {
            player.vel_x += SPEED_CHANGE;
        }
    }
    else // Apply friction when no input
    {
        if (player.vel_x > 0)
            player.vel_x -= PLAYER_SPEED_FRICTION;
        else if (player.vel_x < 0)
            player.vel_x += PLAYER_SPEED_FRICTION;
    }

    if (key & J_UP) {
        if (player.vel_y > -TOP_SPEED) {
            player.vel_y -= SPEED_CHANGE;
        }
    }
    else if (key & J_DOWN) {
        if (player.vel_y < TOP_SPEED) {
            player.vel_y += SPEED_CHANGE;
        }
    }
    else // Apply friction when no input
    {
        if (player.vel_y > 0)
            // player.vel_y--;
            player.vel_y -= PLAYER_SPEED_FRICTION;
        else if (player.vel_y < 0)
            // player.vel_y++;
            player.vel_y += PLAYER_SPEED_FRICTION;
    }

    // Update position using sub-pixel movement
    if (key & J_A) {
        player.pos[0].w += player.vel_x >> 1;
        player.pos[1].w += player.vel_y >> 1;
    } else {
        player.pos[0].w += player.vel_x;
        player.pos[1].w += player.vel_y;
    }

    // Screen boundary collision using the high byte
    if (player.pos[0].b.h < MIN_X) {
        player.pos[0].w = ((uint16_t)MIN_X << 8);
        player.vel_x = -reduce_velocity(player.vel_x);
    }
    if (player.pos[0].b.h > MAX_X) {
        player.pos[0].w = ((uint16_t)MAX_X << 8);
        player.vel_x = -reduce_velocity(player.vel_x);
    }
    if (player.pos[1].b.h < MIN_Y) {
        player.pos[1].w = ((uint16_t)MIN_Y << 8);
        player.vel_y = -reduce_velocity(player.vel_y);
    }
    if (player.pos[1].b.h > MAX_Y) {
        player.pos[1].w = ((uint16_t)MAX_Y << 8);
        player.vel_y = -reduce_velocity(player.vel_y);
    }

    // Move sprite using only the high byte (integer portion)
    move_sprite(0, player.pos[0].b.h, player.pos[1].b.h);
}

void move_coin_to_safe_position() {
    fixed new_coin_x, new_coin_y;
    get_safe_position(&new_coin_x, &new_coin_y);
    Coin[0] = new_coin_x;
    Coin[1] = new_coin_y;
    move_sprite(1, Coin[0].b.h, Coin[1].b.h);
}


void handle_player_coin_collision() {
    int16_t x_dist = player.pos[0].b.h - Coin[0].b.h;
    int16_t y_dist = player.pos[1].b.h - Coin[1].b.h;
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
    // Move toward coin with acceleration
    if (Coin[0].b.h > enemy->pos[0].b.h) {
        enemy->vel_x += enemy->top_speed/16;
    } else {
        enemy->vel_x -= enemy->top_speed/16;
    }
    
    if (Coin[1].b.h > enemy->pos[1].b.h) {
        enemy->vel_y += enemy->top_speed/16;
    } else {
        enemy->vel_y -= enemy->top_speed/16;
    }

    // Cap velocity at speed
    if (enemy->vel_x > enemy->top_speed) enemy->vel_x = enemy->top_speed;
    if (enemy->vel_x < -enemy->top_speed) enemy->vel_x = -enemy->top_speed;
    if (enemy->vel_y > enemy->top_speed) enemy->vel_y = enemy->top_speed;
    if (enemy->vel_y < -enemy->top_speed) enemy->vel_y = -enemy->top_speed;

    enemy->pos[0].w += enemy->vel_x;
    enemy->pos[1].w += enemy->vel_y;

    constrain_to_boundaries(enemy);
}

void update_player_chaser(Enemy* enemy) {
    // Move toward player
    if (player.pos[0].b.h > enemy->pos[0].b.h) enemy->pos[0].w += enemy->top_speed;
    if (player.pos[0].b.h < enemy->pos[0].b.h) enemy->pos[0].w -= enemy->top_speed;
    if (player.pos[1].b.h > enemy->pos[1].b.h) enemy->pos[1].w += enemy->top_speed;
    if (player.pos[1].b.h < enemy->pos[1].b.h) enemy->pos[1].w -= enemy->top_speed;
}

void update_wanderer(Enemy* enemy) {
    if (sys_time % 60 == 0) {
        // Change direction randomly
        enemy->vel_x = random(-enemy->top_speed, enemy->top_speed);
        enemy->vel_y = random(-enemy->top_speed, enemy->top_speed);
    }
    enemy->pos[0].w += enemy->vel_x;
    enemy->pos[1].w += enemy->vel_y;
    constrain_to_boundaries(enemy);
}



void handle_enemy_collisions() {
    // Check each enemy against others within a reasonable distance
    for(uint8_t i = 0; i < enemies_spawned; i++) {
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
        int16_t enemy_x_dist = abs(player.pos[0].b.h - enemies[i].pos[0].b.h);
        int16_t enemy_y_dist = abs(player.pos[1].b.h - enemies[i].pos[1].b.h);

        if (enemy_x_dist < 6 && enemy_y_dist < 6) {
            switch(enemies[i].type) {
                case ENEMY_TYPE_WANDERER:
                case ENEMY_TYPE_COIN_CHASER:
                    // Store original velocities
                    int16_t orig_player_vel_x = player.vel_x;
                    int16_t orig_player_vel_y = player.vel_y;
                    int16_t orig_enemy_vel_x = enemies[i].vel_x;
                    int16_t orig_enemy_vel_y = enemies[i].vel_y;

                    // Simple collision response - exchange velocities
                    if (player.pos[0].b.h < enemies[i].pos[0].b.h) {
                        player.vel_x = -abs(orig_player_vel_x);
                        enemies[i].vel_x = abs(orig_enemy_vel_x);
                        player.pos[0].w -= 256;
                        enemies[i].pos[0].w += 256;
                    } else {
                        player.vel_x = abs(orig_player_vel_x);
                        enemies[i].vel_x = -abs(orig_enemy_vel_x);
                        player.pos[0].w += 256;
                        enemies[i].pos[0].w -= 256;
                    }

                    if (player.pos[1].b.h < enemies[i].pos[1].b.h) {
                        player.vel_y = -abs(orig_player_vel_y);
                        enemies[i].vel_y = abs(orig_enemy_vel_y);
                        player.pos[1].w -= 256;
                        enemies[i].pos[1].w += 256;
                    } else {
                        player.vel_y = abs(orig_player_vel_y);
                        enemies[i].vel_y = -abs(orig_enemy_vel_y);
                        player.pos[1].w += 256;
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
        for(uint8_t j = i + 1; j < enemies_spawned; j++) {
            int16_t dx = abs(enemies[i].pos[0].b.h - enemies[j].pos[0].b.h);
            int16_t dy = abs(enemies[i].pos[1].b.h - enemies[j].pos[1].b.h);

            // Only process if they're close enough (8x8 sprite size)
            if (dx < 8 && dy < 8) {
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
    for(uint8_t i = 0; i < enemies_spawned; i++) {
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
        
        // Update sprite position
        move_sprite(enemies[i].sprite_id, 
                   enemies[i].pos[0].b.h, 
                   enemies[i].pos[1].b.h);
    }
}

void init_enemy(uint8_t index) {
    uint8_t type = get_enemy_type(current_level, enemies_spawned);

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
            enemies[index].top_speed = COIN_CHASER_SPEED;
            break;
        case ENEMY_TYPE_PLAYER_CHASER:
            set_sprite_tile(enemies[index].sprite_id, 3);
            enemies[index].top_speed = PLAYER_CHASER_SPEED;
            break;
        case ENEMY_TYPE_WANDERER:
            set_sprite_tile(enemies[index].sprite_id, 4);
            enemies[index].top_speed = WANDERER_SPEED;
            break;
    }
    
    move_sprite(enemies[index].sprite_id, 
                enemies[index].pos[0].b.h, 
                enemies[index].pos[1].b.h);
}
