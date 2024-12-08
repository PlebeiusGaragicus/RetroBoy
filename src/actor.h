#ifndef ACTOR_H
#define ACTOR_H



#define TOP_SPEED 600
#define SPEED_CHANGE 8
#define PLAYER_SPEED_FRICTION 2

// [0] is X, [1] is Y
extern fixed PlayerPos[2];
extern int16_t VelX;
extern int16_t VelY;
extern uint8_t PlayerSpriteIndex;

extern fixed Coin[2];
extern int16_t coin_vel_x;
extern int16_t coin_vel_y;



#define ATTRACTION_FORCE 200    // Force of attraction to target (coin/player)
#define SEPARATION_FORCE 80   // Force to keep enemies apart
#define SEPARATION_RADIUS 15   // Distance at which separation starts

#define COIN_CHASER_SPEED 90    // Slower, methodical movement
#define PLAYER_CHASER_SPEED 100 // Medium speed, persistent follower
#define WANDERER_SPEED 150      // Faster but erratic movement

#define MAX_ENEMIES 7
#define ENEMY_TYPE_COIN_CHASER 0
#define ENEMY_TYPE_PLAYER_CHASER 1
#define ENEMY_TYPE_WANDERER 2


#define SPAWN_DELAY 60          // Frames between enemy spawns
#define MIN_SPAWN_DISTANCE 32   // Minimum distance from player for spawns
#define MIN_COIN_DISTANCE 24    // Minimum distance for coin respawns



extern const uint8_t ENEMY_SPAWN_CONFIG[MAX_ENEMIES];


extern uint8_t active_enemies;     // Track number of currently active enemies
extern uint8_t spawn_timer;        // Timer for spawning enemies

typedef struct {
    fixed pos[2];        // Position (x,y)
    uint8_t sprite_id;   // Sprite index
    uint8_t type;        // Enemy behavior type
    int16_t vel_x;       // Velocity X
    int16_t vel_y;       // Velocity Y
    uint8_t speed;       // Movement speed
} Enemy;

extern Enemy enemies[MAX_ENEMIES];


void hide_all_enemies();
void update_enemies();
void init_enemy(uint8_t index);

void move_coin_to_safe_position();

void update_player_physics(uint8_t key);
void handle_player_coin_collision();

void update_enemies();
void handle_enemy_collisions();




#endif