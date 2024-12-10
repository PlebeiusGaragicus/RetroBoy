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
    uint8_t counter = 60;

    while(counter--) {
        if(sys_time % 20 == 0) {
            x = random(MIN_X, MAX_X);
            y = random(MIN_Y, MAX_Y);
            move_sprite(0, x, y);
            bap();
        }
        vsync();
    }

    performantdelay(20);

    boop();

    PlayerPos[0].w = (uint16_t)x << 8;
    PlayerPos[1].w = (uint16_t)y << 8;

    move_coin_to_safe_position();

    // Reset enemy spawning
    // active_enemies = 0;
    enemies_spawned = 0;
    spawn_timer = SPAWN_DELAY;

    // Reset velocities
    VelX = 0;
    VelY = 0;

    reset_score();
    display_scores();


    game_over = FALSE;
    level_timer = 0;
}



void main() {
    // https://www.youtube.com/watch?v=nziu1O_cj1w&list=PLrW43fNmjaQVmjvIj3Ho3rzW46GEw14F9&index=5
	SHOW_BKG;
	SHOW_SPRITES;
	DISPLAY_ON;

    audio_init();

    // This is the "intro" screen when the game first starts
    seed_prng();




    uint8_t key;

    while(TRUE) {
        level_intro_cutscene();
        init_level();
        ready_start();

        while(!game_over) {
            key = joypad();

            // if (active_enemies < MAX_ENEMIES) {
            //     if (spawn_timer > 0) {
            //         spawn_timer--;
            //     } else {
            //         init_enemy(active_enemies); // Use index as type
            //         active_enemies++;
            //         spawn_timer = SPAWN_DELAY;
            //     }
            // }

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

            handle_level_events();  // Handle level-specific events including enemy spawning


            update_enemies();
            handle_enemy_collisions();

            vsync();
        }
    }
}
