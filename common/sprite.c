#include "sprite.h"

void translate_world(world_state_t* world)
{
    for (int i = 0; i < world->player_count; i++)
    {
        world->players[i].position.x += world->players[i].velocity.x;
        world->players[i].position.y += world->players[i].velocity.y;
    }
}

void render_world(SDL_Renderer* renderer, world_state_t* world)
{
    for (int i = 0; i < world->player_count; i++)
    {
        static SDL_Point points[PLAYER_SPR_POINTS];

        float s = sinf(world->players[i].angle);
        float c = cosf(world->players[i].angle);

        for (int j = 0; j < PLAYER_SPR_POINTS; j++)
        {
            float x = player_sprite[j].x * c - player_sprite[j].y * s;
            float y = player_sprite[j].x * s + player_sprite[j].y * c;

            points[j].x = (int)(x + world->players[i].position.x);
            points[j].y = (int)(y + world->players[i].position.y);
        }

        SDL_SetRenderDrawColor(renderer, 
            player_colors[i].r,
            player_colors[i].g,
            player_colors[i].b,
            player_colors[i].a);

        SDL_RenderDrawLines(renderer, points, PLAYER_SPR_POINTS);
    }
}

const point_t player_sprite[PLAYER_SPR_POINTS] = {
    -PLAYER_SPR_SIZE, -PLAYER_SPR_SIZE,
     PLAYER_SPR_SIZE, 0.0f,
    -PLAYER_SPR_SIZE,  PLAYER_SPR_SIZE,
    -PLAYER_SPR_SIZE * 0.5f, 0.0f,
    -PLAYER_SPR_SIZE, -PLAYER_SPR_SIZE
};

const SDL_Color player_colors[MAX_PLAYERS] = {
    255, 0,   0,   255,
    0,   255, 0,   255,
    0,   0,   255, 255,
    255, 0,   255, 255
};