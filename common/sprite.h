#ifndef __SPRITE_H__
#define __SPRITE_H__

#include <SDL.h>

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

#define MAX_POINTS 8
#define PLAYER_SPR_SIZE 16
#define PLAYER_SPR_POINTS 5

#define MAX_PLAYERS 8
#define MAX_ASTEROIDS 32
#define MAX_BULLETS 256
#define MAX_SPEED 4

#define ACCELERATION 0.0675f

extern const double physics_step;
extern const double network_step;

typedef struct point_t
{
    float x;
    float y;
} point_t;

typedef struct input_state_t
{
    uint32_t sequence;
    uint8_t id : 4;
    uint8_t thrust : 1;
    uint8_t left : 1;
    uint8_t right : 1;
    uint8_t shoot : 1;
} input_state_t;

typedef struct player_state_t
{
    uint32_t sequence;
    point_t position;
    point_t velocity;
    float angle;
} player_state_t;

typedef struct world_state_t
{
    uint32_t sequence;
    player_state_t players[MAX_PLAYERS];
    uint8_t alive;
} world_state_t;

void translate_world(world_state_t* world);
void render_world(SDL_Renderer* renderer, world_state_t* world);

inline float to_rad(float deg)
{
    return deg * (float)M_PI / 180.0f;
}

inline float to_deg(float rad)
{
    return rad * 180.0f / (float)M_PI;
}

inline int is_player_alive(world_state_t* world, int player)
{
    return world->alive & 1 << player;
}

inline void set_player_alive(world_state_t* world, int player)
{
    world->alive = world->alive | 1 << player;
}

inline void set_player_free(world_state_t* world, int player)
{
    world->alive = world->alive & ~(1 << player);
}

inline int get_free_player(world_state_t* world)
{
    for (int i = 0; i < MAX_PLAYERS; i++)
    {
        if (is_player_alive(world, i))
            continue;

        set_player_alive(world, i);

        return i;
    }

    return -1;
}

extern const point_t player_sprite[PLAYER_SPR_POINTS];
extern const SDL_Color player_colors[MAX_PLAYERS];
#endif