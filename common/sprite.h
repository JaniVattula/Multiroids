#ifndef __SPRITE_H__
#define __SPRITE_H__

#include <SDL.h>

#define MAX_POINTS 8
#define PLAYER_SPR_SIZE 32
#define PLAYER_SPR_POINTS 5

#define MAX_PLAYERS 4
#define MAX_ASTEROIDS 32
#define MAX_BULLETS 256
#define MAX_SPEED 4

typedef struct point_t
{
    float x;
    float y;
} point_t;

typedef struct input_state_t
{
    unsigned char thrust : 1;
    unsigned char left : 1;
    unsigned char right : 1;
    unsigned char shoot : 1;
} input_state_t;

typedef struct player_state_t
{
    point_t position;
    point_t velocity;
    float angle;
} player_state_t;

typedef struct world_state_t
{
    player_state_t players[MAX_PLAYERS];
    char player_count;
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

extern const point_t player_sprite[PLAYER_SPR_POINTS];
extern const SDL_Color player_colors[MAX_PLAYERS];
#endif