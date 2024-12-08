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



BOOLEAN KEY_B_PRESSED = FALSE;




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

    move_coin_to_safe_position();

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

            update_player_physics(key);
            handle_player_coin_collision();

            update_enemies();
            handle_enemy_collisions();

            vsync();
        }
    }
}
