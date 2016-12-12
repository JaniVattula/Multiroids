#include <SDL.h>
#include <stdio.h>
#include <enet\enet.h>
#include "common.h"

#define PORT 8888
#define MAX_INPUTS 256

ENetHost* server = NULL;
ENetPeer* peers[MAX_PLAYERS];

char buffer[256];

double last_shots[MAX_PLAYERS];

bullet_state_t bullets[MAX_BULLETS];
input_state_t inputs[MAX_INPUTS];
world_state_t world_state;

int input_count = 0;
int bullet_count = 0;
int running = 1;

void init()
{
    SDL_Init(SDL_INIT_TIMER);

    if (enet_initialize() != 0)
    {
        printf("Failed to initialize ENet.\n");
        exit(EXIT_FAILURE);
    }

    ENetAddress address;

    address.host = ENET_HOST_ANY;
    address.port = PORT;

    server = enet_host_create(&address, MAX_PLAYERS, 2, 0, 0);

    if (server == NULL)
    {
        printf("Failed to create an ENet server host.\n");
        exit(EXIT_FAILURE);
    }

    enet_address_get_host_ip(&address, buffer, sizeof(buffer));
    printf("Launching new server at %s:%u\n", buffer, address.port);

    memset(&peers, 0, sizeof(peers));

    world_state.type = PACKET_WORLD;
    world_state.sequence = 0;
}

void receive_packets()
{
    ENetEvent net_event;
    uint8_t* packet_type;

    while (enet_host_service(server, &net_event, 0) > 0)
    {
        switch (net_event.type)
        {
        case ENET_EVENT_TYPE_CONNECT:
            enet_address_get_host_ip(&net_event.peer->address, buffer, sizeof(buffer));

            uint8_t id = get_free_player(&world_state);

            if (id != -1)
            {
                peers[id] = net_event.peer;

                world_state.players[id].position.x = WINDOW_WIDTH / 2.0f;
                world_state.players[id].position.y = WINDOW_HEIGHT / 2.0f;
                world_state.players[id].velocity.x = 0.0f;
                world_state.players[id].velocity.y = 0.0f;
                world_state.players[id].angle = (float)M_PI * 1.5f;
                world_state.players[id].sequence = 0;

                ENetPacket* packet = enet_packet_create(&id, sizeof(id), ENET_PACKET_FLAG_RELIABLE);

                enet_peer_send(peers[id], 0, packet);

                net_event.peer->data = malloc(sizeof(uint8_t));
                *(uint8_t*)net_event.peer->data = id;

                printf("Player %d connected from %s:%u\n", id, buffer, net_event.peer->address.port);
            }

            break;
        case ENET_EVENT_TYPE_RECEIVE:
            packet_type = (uint8_t*)net_event.packet->data;

            switch (*packet_type)
            {
            case PACKET_INPUT:
                inputs[input_count++] = *(input_state_t*)net_event.packet->data;
                break;
            }

            enet_packet_destroy(net_event.packet);

            break;
        case ENET_EVENT_TYPE_DISCONNECT:
            printf("Player %d disconnected.\n", *(char*)net_event.peer->data);

            set_player_free(&world_state, *(char*)net_event.peer->data);

            free(net_event.peer->data);

            break;
        }
    }
}

void check_bullet_collisions_to_borders(bullet_state_t* bullets, int count)
{
    for (int i = 0; i < count; i++)
    {
        if (bullets[i].position.x > WINDOW_WIDTH + BULLET_SIZE ||
            bullets[i].position.x < -BULLET_SIZE ||
            bullets[i].position.y > WINDOW_HEIGHT + BULLET_SIZE ||
            bullets[i].position.y < -BULLET_SIZE)
        {
            bullets[i].alive = 0;

            bullet_dead_t dead;
            dead.id = i;
            dead.type = PACKET_BULLET_REMOVE;

            ENetPacket* packet = enet_packet_create(&dead, sizeof(dead), ENET_PACKET_FLAG_RELIABLE);
            enet_host_broadcast(server, 1, packet);
        }
    }
}

void check_bullet_collisions_to_players(world_state_t* world, bullet_state_t* bullets, int count)
{
    SDL_Rect player;
    SDL_Rect bullet;

    for (int i = 0; i < count; i++)
    {
        for (int j = 0; j < MAX_PLAYERS; j++)
        {
            if (!is_player_alive(world, j) || j == bullets[i].owner)
                continue;

            player.x = (int)(world->players[j].position.x - PLAYER_SPR_SIZE / 2);
            player.y = (int)(world->players[j].position.y - PLAYER_SPR_SIZE / 2);
            player.w = PLAYER_SPR_SIZE;
            player.h = PLAYER_SPR_SIZE;

            bullet.x = (int)(bullets[i].position.x - BULLET_SIZE / 2);
            bullet.y = (int)(bullets[i].position.y - BULLET_SIZE / 2);
            bullet.w = BULLET_SIZE;
            bullet.h = BULLET_SIZE;

            if (SDL_HasIntersection(&bullet, &player))
            {
                
            }
        }
    }
}

void update()
{   
    double current_time = SDL_GetTicks() / 1000.0;

    for (int i = 0; i < input_count; i++)
    {
        player_state_t* player = &world_state.players[inputs[i].id];

        if (inputs[i].right)
        {
            player->angle += PLAYER_TURN_SPEED;
        }

        if (inputs[i].left)
        {
            player->angle -= PLAYER_TURN_SPEED;
        }

        if (inputs[i].thrust)
        {
            player->velocity.x += cosf(player->angle) * PLAYER_ACCELERATION;
            player->velocity.y += sinf(player->angle) * PLAYER_ACCELERATION;

            float speed = sqrtf(powf(player->velocity.x, 2.0f) + powf(player->velocity.y, 2.0f));

            if (speed > PLAYER_MOVE_SPEED)
            {
                player->velocity.x *= PLAYER_MOVE_SPEED / speed;
                player->velocity.y *= PLAYER_MOVE_SPEED / speed;
            }
        }

        if (inputs[i].shoot && current_time - last_shots[inputs[i].id] > FIRE_INTERVAL)
        {
            last_shots[inputs[i].id] = current_time;

            bullet_state_t* bullet = &bullets[bullet_count++];
            bullet->type = PACKET_BULLET_ADD;
            bullet->alive = 1;
            bullet->sequence = inputs[i].sequence;
            bullet->owner = inputs[i].id;
            bullet->position.x = player->position.x + cosf(player->angle) * PLAYER_SPR_SIZE;
            bullet->position.y = player->position.y + sinf(player->angle) * PLAYER_SPR_SIZE;
            bullet->velocity.x = cosf(player->angle) * BULLET_SPEED;
            bullet->velocity.y = sinf(player->angle) * BULLET_SPEED;

            ENetPacket* packet = enet_packet_create(bullet, sizeof(bullet_state_t), ENET_PACKET_FLAG_RELIABLE);
            enet_host_broadcast(server, 1, packet);
        }

        player->sequence = inputs[i].sequence;
    }

    input_count = 0;

    translate_world(&world_state);
    translate_bullets(bullets, bullet_count);
    check_bullet_collisions_to_borders(bullets, bullet_count);
    check_bullet_collisions_to_players(&world_state, bullets, bullet_count);
    clean_bullets(bullets, &bullet_count);
}

void send_packets()
{
    ENetPacket* packet = enet_packet_create(&world_state, sizeof(world_state), 0);
    enet_host_broadcast(server, 1, packet);

    world_state.sequence++;
}

void deinit()
{
    enet_host_destroy(server);
    enet_deinitialize();

    SDL_Quit();
}

int main(int argc, char* argv[])
{
	(void)argc;
	(void)argv;

    init();

    double accumulator = 0.0;
    double current_time = 0.0;
    double send_time = 0.0;

    while (running)
    {
        double new_time = SDL_GetTicks() / 1000.0;
        double frame_time = new_time - current_time;

        current_time = new_time;
        accumulator += frame_time;

        receive_packets();

        while (accumulator >= physics_step)
        {
            update();
            accumulator -= physics_step;

            if (new_time - send_time >= network_step)
            {
                send_packets();
                send_time = new_time;
            }
        }
    }

    deinit();
	return 0;
}