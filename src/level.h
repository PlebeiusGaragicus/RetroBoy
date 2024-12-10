#ifndef LEVEL_H
#define LEVEL_H

#include <gb/gb.h>
#include "actor.h"

#define MIN_X 8
#define MAX_X 160
#define MIN_Y 16
#define MAX_Y 152

#define MAX_LEVEL 3

// Level state
extern BOOLEAN game_over;
extern uint8_t level;
extern uint8_t player_score;
extern uint8_t enemy_score;
extern uint8_t current_level;
extern uint16_t level_timer;
extern uint8_t enemies_spawned;

// Level functions
void load_number_tiles();
void display_scores();
void load_sprites();
void pause_screen();
void reset_score();
void init_level();
void level_intro_cutscene();

uint8_t get_enemy_type(uint8_t level, uint8_t spawn_index);
void handle_level_events();

// Level definitions
typedef struct {
    uint8_t max_enemies;          // Maximum enemies for this level
    uint16_t spawn_interval;      // Time between enemy spawns
    uint8_t score_needed_to_win;
} LevelConfig;

extern const LevelConfig LEVEL_CONFIGS[MAX_LEVEL];

#endif