#ifndef LEVEL_H
#define LEVEL_H

#include <gb/gb.h>


#define MIN_X 8
#define MAX_X 160
#define MIN_Y 16
#define MAX_Y 152



extern BOOLEAN game_over;
extern uint8_t level;
extern uint8_t player_score;
extern uint8_t enemy_score;


void load_number_tiles();
void display_scores();
void load_sprites();
void pause_screen();
void reset_score();


#endif