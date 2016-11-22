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

#endif