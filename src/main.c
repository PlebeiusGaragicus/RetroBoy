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

    // Initialize player at random position
    player.pos[0].w = (uint16_t)x << 8;
    player.pos[1].w = (uint16_t)y << 8;
    player.vel_x = 0;
    player.vel_y = 0;
    player.top_speed = TOP_SPEED;

    move_coin_to_safe_position();

    // Reset enemy spawning
    enemies_spawned = 0;
    spawn_timer = SPAWN_DELAY;

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
        hide_all_enemies();
        init_level();
        ready_start();

        while(!game_over) {
            key = joypad();

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

            // Animate coin every 8 frames
            if (sys_time % 8 == 0) {
                update_coin_animation();
            }

            handle_level_events();  // Handle level-specific events including enemy spawning

            update_enemies();
            handle_enemy_collisions();

            vsync();
        }
    }
}
