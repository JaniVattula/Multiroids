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

        SDL_RenderDrawLines(renderer, points, PLAYER_SPR_POINTS);
    }
}

void init_sprites()
{
    player_sprite[0].x = -PLAYER_SPR_SIZE;
    player_sprite[0].y = -PLAYER_SPR_SIZE;
    player_sprite[1].x = PLAYER_SPR_SIZE;
    player_sprite[1].y = 0.0f;
    player_sprite[2].x = -PLAYER_SPR_SIZE;
    player_sprite[2].y = PLAYER_SPR_SIZE;
    player_sprite[3].x = -PLAYER_SPR_SIZE * 0.5f;
    player_sprite[3].y = 0.0f;
    player_sprite[4].x = -PLAYER_SPR_SIZE;
    player_sprite[4].y = -PLAYER_SPR_SIZE;
}