#include "common.h"

const double physics_step = 1 / 60.0;
const double network_step = 1 / 10.0;

void translate_world(world_state_t* world)
{
    for (int i = 0; i < MAX_PLAYERS; i++)
    {
        if (!is_player_alive(world, i))
            continue;

        world->players[i].position.x += world->players[i].velocity.x;
        world->players[i].position.y += world->players[i].velocity.y;
    }
}

void render_world(SDL_Renderer* renderer, world_state_t* world)
{
    for (int i = 0; i < MAX_PLAYERS; i++)
    {
        if (!is_player_alive(world, i))
            continue;

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

void translate_bullets(bullet_state_t* bullets, size_t count)
{
    for (int i = 0; i < count; i++)
    {
        bullets[i].position.x += bullets[i].velocity.x;
        bullets[i].position.y += bullets[i].velocity.y;
    }
}

void render_bullets(SDL_Renderer* renderer, bullet_state_t* bullets, size_t count)
{
    SDL_Point point;

    for (int i = 0; i < count; i++)
    {
        point.x = (int)bullets[i].position.x;
        point.y = (int)bullets[i].position.y;

        SDL_SetRenderDrawColor(renderer,
            player_colors[bullets[i].owner].r,
            player_colors[bullets[i].owner].g,
            player_colors[bullets[i].owner].b,
            player_colors[bullets[i].owner].a);

        SDL_RenderDrawPoint(renderer, point.x, point.y);
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
    255, 0,   255, 255,
    255, 255, 0,   255,
    0,   255, 255, 255,
    255, 128, 128, 255,
    128, 128, 255, 255
};