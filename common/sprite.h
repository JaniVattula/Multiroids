#ifndef __SPRITE_H__
#define __SPRITE_H__

#include <SDL.h>

#define MAX_POINTS 8

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
} input_state_t;

typedef struct player_state_t
{
    point_t position;
    point_t velocity;
    float angle;
} player_state_t;

typedef struct sprite_t
{
    point_t position;
    point_t velocity;
    point_t size;
    point_t points[MAX_POINTS];
    size_t count;
    float angle;
    int alive;
} sprite_t;

sprite_t* get_sprite(sprite_t* sprites, int count);
void free_sprite(sprite_t* sprite);

void translate_sprites(sprite_t* sprites, int count);
void render_sprites(SDL_Renderer* renderer, sprite_t* sprites, int count);

sprite_t players[MAX_PLAYERS];

void init_sprites();
sprite_t* create_ship(float x, float y, float width, float height, float angle);

inline float to_rad(float deg)
{
    return deg * (float)M_PI / 180.0f;
}

inline float to_deg(float rad)
{
    return rad * 180.0f / (float)M_PI;
}



#endif