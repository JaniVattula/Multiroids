#include "common.h"
#include <stdio.h>

const double physics_step = 1 / 60.0;
const double network_step = 1 / 10.0;

void translate_world(world_state_t* world)
{
    for (int i = 0; i < MAX_PLAYERS; i++)
    {
        if (!is_player_connected(world, i))
            continue;

        world->players[i].position.x += world->players[i].velocity.x;
        world->players[i].position.y += world->players[i].velocity.y;

		if (world->players[i].position.x > WINDOW_WIDTH + BUFFER_ZONE)
			world->players[i].position.x = -BUFFER_ZONE;
		else if (world->players[i].position.x < -BUFFER_ZONE)
			world->players[i].position.x = WINDOW_WIDTH + BUFFER_ZONE;

		if (world->players[i].position.y > WINDOW_HEIGHT + BUFFER_ZONE)
			world->players[i].position.y = -BUFFER_ZONE;
		else if (world->players[i].position.y < -BUFFER_ZONE)
			world->players[i].position.y = WINDOW_HEIGHT + BUFFER_ZONE;
    }
}

void render_world(SDL_Renderer* renderer, world_state_t* world)
{
    for (int i = 0; i < MAX_PLAYERS; i++)
    {
        if (!is_player_connected(world, i) || !world->players[i].alive)
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

void translate_bullets(bullet_state_t* bullets, int count)
{
    for (int i = 0; i < count; i++)
    {
        bullets[i].position.x += bullets[i].velocity.x;
        bullets[i].position.y += bullets[i].velocity.y;
    }
}

void render_bullets(SDL_Renderer* renderer, bullet_state_t* bullets, int count)
{
    SDL_Rect rect;

    for (int i = 0; i < count; i++)
    {
        rect.x = (int)bullets[i].position.x - BULLET_SIZE / 2;
        rect.y = (int)bullets[i].position.y - BULLET_SIZE / 2;
        rect.w = BULLET_SIZE;
        rect.h = BULLET_SIZE;

        SDL_SetRenderDrawColor(renderer,
            player_colors[bullets[i].owner].r,
            player_colors[bullets[i].owner].g,
            player_colors[bullets[i].owner].b,
            player_colors[bullets[i].owner].a);

        SDL_RenderDrawRect(renderer, &rect);
    }
}

void clean_bullets(bullet_state_t* bullets, int* count)
{
    int new_count = 0;
    
    for (int i = 0; i < *count; i++)
    {
        if (bullets[i].alive)
            bullets[new_count++] = bullets[i];
    }

    *count = new_count;
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