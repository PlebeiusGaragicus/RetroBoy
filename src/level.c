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

BOOLEAN game_over = FALSE;
uint8_t level = 0;
uint8_t player_score = 0;
uint8_t enemy_score = 0;


void load_number_tiles() {
    // Load number tiles into background memory starting at index 128
    // (to avoid conflicts with other tiles)
    set_bkg_data(128, 10, NumberTiles);
}

void display_scores() {
    // Player score (top right)
    uint8_t player_ones = (player_score % 10) + 128;
    uint8_t player_tens = ((player_score / 10) % 10) + 128;
    set_bkg_tiles(17, 1, 1, 1, &player_tens);
    set_bkg_tiles(18, 1, 1, 1, &player_ones);
    
    // Enemy score (bottom left)
    uint8_t enemy_ones = (enemy_score % 10) + 128;
    uint8_t enemy_tens = ((enemy_score / 10) % 10) + 128;
    set_bkg_tiles(1, 16, 1, 1, &enemy_tens);
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
