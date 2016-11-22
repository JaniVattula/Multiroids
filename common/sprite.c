#include "sprite.h"


sprite_t* get_sprite(sprite_t* sprites, int count)
{
    for (int i = 0; i < count; i++)
    {
        if (sprites[i].alive == 0)
        {
            sprites[i].alive = 1;

            return &sprites[i];
        }
    }

    return NULL;
}

void free_sprite(sprite_t* sprite)
{
    sprite->alive = 0;
}

void translate_sprites(sprite_t* sprites, int count)
{
    for (int i = 0; i < count; i++)
    {
        if (sprites[i].alive == 0)
            continue;

        sprites[i].position.x += sprites[i].velocity.x;
        sprites[i].position.y += sprites[i].velocity.y;
    }
}

void render_sprites(SDL_Renderer* renderer, sprite_t* sprites, int count)
{
    static SDL_Point points[MAX_POINTS];

    for (int i = 0; i < count; i++)
    {
        if (sprites[i].alive == 0)
            continue;

        float s = sinf(sprites[i].angle);
        float c = cosf(sprites[i].angle);
        float x = 0;
        float y = 0;

        for (size_t j = 0; j < sprites[i].count; j++)
        {
            x = sprites[i].points[j].x * c - sprites[i].points[j].y * s;
            y = sprites[i].points[j].x * s + sprites[i].points[j].y * c;

            x *= sprites[i].size.x;
            y *= sprites[i].size.y;

            points[j].x = (int)(x + sprites[i].position.x);
            points[j].y = (int)(y + sprites[i].position.y);
        }

        SDL_RenderDrawLines(renderer, points, (int)sprites[i].count);
    }
}