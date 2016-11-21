#ifndef __SPRITE_H__
#define __SPRITE_H__

#include <SDL.h>

#define MAX_POINTS 8

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

#endif