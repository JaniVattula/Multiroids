#ifndef __COMMON_H__
#define __COMMON_H__

#include <SDL.h>

#define WINDOW_WIDTH 1440
#define WINDOW_HEIGHT 900
#define BUFFER_ZONE 20

#define PLAYER_SPR_SIZE 20
#define PLAYER_SPR_POINTS 5
#define PLAYER_ACCELERATION 0.10f
#define PLAYER_MOVE_SPEED 6
#define PLAYER_TURN_SPEED 0.08f

#define MAX_PLAYERS 8
#define MAX_BULLETS 256
#define FIRE_INTERVAL 0.4
#define RESPAWN_RATE 3.0

#define BULLET_SIZE 5
#define BULLET_SPEED 8

enum PACKET_TYPE
{
    PACKET_INPUT = 0,
    PACKET_WORLD,
    PACKET_BULLET_ADD,
    PACKET_PLAYER_HIT,
    PACKET_BULLET_REMOVE,
};

extern const double physics_step;
extern const double network_step;

typedef struct point_t
{
    float x;
    float y;
} point_t;

typedef struct input_state_t
{
    uint8_t type;
    uint32_t sequence;
    uint8_t id : 4;
    uint8_t thrust : 1;
    uint8_t left : 1;
    uint8_t right : 1;
    uint8_t shoot : 1;
} input_state_t;

typedef struct bullet_dead_t
{
    uint8_t type;
    uint32_t sequence;
    uint8_t id;
} bullet_dead_t;

typedef struct bullet_state_t
{
    uint8_t type;
    uint32_t sequence;
    uint8_t owner : 7;
    uint8_t alive : 1;
    point_t position;
    point_t velocity;
} bullet_state_t;

typedef struct player_state_t
{
    uint32_t sequence;
	uint8_t alive;
    point_t position;
    point_t velocity;
    float angle;
} player_state_t;

typedef struct world_state_t
{
    uint8_t type;
    uint32_t sequence;
    player_state_t players[MAX_PLAYERS];
    uint8_t connected;
} world_state_t;

void translate_world(world_state_t* world);
void render_world(SDL_Renderer* renderer, world_state_t* world);
void translate_bullets(bullet_state_t* bullets, int count);
void render_bullets(SDL_Renderer* renderer, bullet_state_t* bullets, int count);
void clean_bullets(bullet_state_t* bullets, int* count);

inline float to_rad(float deg)
{
    return deg * (float)M_PI / 180.0f;
}

inline float to_deg(float rad)
{
    return rad * 180.0f / (float)M_PI;
}

inline int is_player_connected(world_state_t* world, int player)
{
    return world->connected & 1 << player;
}

inline void set_player_connected(world_state_t* world, int player)
{
    world->connected = world->connected | 1 << player;
}

inline void set_player_free(world_state_t* world, int player)
{
    world->connected = world->connected & ~(1 << player);
}

inline int get_free_player(world_state_t* world)
{
    for (int i = 0; i < MAX_PLAYERS; i++)
    {
        if (is_player_connected(world, i))
            continue;

        set_player_connected(world, i);

        return i;
    }

    return -1;
}

extern const point_t player_sprite[PLAYER_SPR_POINTS];
extern const SDL_Color player_colors[MAX_PLAYERS];

#endif