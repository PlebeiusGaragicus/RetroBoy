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

BOOLEAN game_over = FALSE;
uint8_t level = 0;
uint8_t player_score = 0;
uint8_t enemy_score = 0;

uint8_t current_level = 0;
uint16_t level_timer = 0;
uint8_t enemies_spawned = 0;

// Level configurations
const LevelConfig LEVEL_CONFIGS[MAX_LEVEL] = {
    // Level 1: Easy - Just wanderers
    {
        .max_enemies = 3,
        .spawn_interval = 180,    // 3 seconds between spawns
        // .level_duration = 3600    // 1 minute
    },
    // Level 2: Medium - Wanderers and coin chasers
    {
        .max_enemies = 5,
        .spawn_interval = 120,    // 2 seconds between spawns
        // .level_duration = 3600    // 1 minute
    },
    // Level 3: Hard - All enemy types
    {
        .max_enemies = 7,
        .spawn_interval = 60,     // 1 second between spawns
        // .level_duration = 3600    // 1 minute
    }
};

uint8_t get_enemy_type(uint8_t level, uint8_t spawn_index) {
    switch(level) {
        case 0:  // Level 1
            return ENEMY_TYPE_WANDERER;

        case 1:  // Level 2
            return (spawn_index % 2 == 0) ? 
                   ENEMY_TYPE_WANDERER : 
                   ENEMY_TYPE_COIN_CHASER;
            
        case 2:  // Level 3
            if (spawn_index < 2) return ENEMY_TYPE_WANDERER;
            if (spawn_index < 4) return ENEMY_TYPE_COIN_CHASER;
            if (spawn_index < 6) return ENEMY_TYPE_PLAYER_CHASER;
            return ENEMY_TYPE_WANDERER;
            
        default:
            return ENEMY_TYPE_WANDERER;
    }
}







void init_level() {
    // current_level = level;
    level_timer = 0;
    enemies_spawned = 0;
    hide_all_enemies();
    // active_enemies = 0;
}

void handle_level_events() {
    const LevelConfig* config = &LEVEL_CONFIGS[current_level];

    // Increment level timer
    level_timer++;

    // Check if level is complete
    if (player_score > 3) {
        if (current_level < MAX_LEVEL - 1) {
            beedledo();
            player_score = 0;
            current_level++;
            init_level();
            // splash_screen();
            level_intro_cutscene();
        } else {
            // Game complete!
            game_over = TRUE;
        }
        return;
    }

    // Handle enemy spawning
    if (level_timer % config->spawn_interval == 0 && 
        // active_enemies < config->max_enemies && 
        enemies_spawned < config->max_enemies) {

        init_enemy(enemies_spawned);
        enemies_spawned++;
    }
}

void load_number_tiles() {
    // Load number tiles into background memory starting at index 128
    // (to avoid conflicts with other tiles)
    set_bkg_data(128, 10, NumberTiles);
}

void display_scores() {
    // Player score (top right)
    uint8_t player_ones = (player_score % 10) + 128;
    // uint8_t player_tens = ((player_score / 10) % 10) + 128;
    // set_bkg_tiles(17, 1, 1, 1, &player_tens);
    set_bkg_tiles(18, 1, 1, 1, &player_ones);

    // Enemy score (bottom left)
    uint8_t enemy_ones = (enemy_score % 10) + 128;
    // uint8_t enemy_tens = ((enemy_score / 10) % 10) + 128;
    // set_bkg_tiles(1, 16, 1, 1, &enemy_tens);
    set_bkg_tiles(2, 16, 1, 1, &enemy_ones);
}


void load_sprites()
{
    show_screen_border();
    load_number_tiles();


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
    load_number_tiles();
    SHOW_SPRITES;
}

void reset_score() {
    player_score = 0;
    enemy_score = 0;
    display_scores();
}
