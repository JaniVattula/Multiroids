#include <SDL.h>
#include <stdio.h>
#include <enet\enet.h>
#include "sprite.h"

#define PORT 8888
#define MAX_INPUTS 256

ENetHost* server = NULL;
ENetPeer* peers[MAX_PLAYERS];

char buffer[256];

input_state_t inputs[MAX_INPUTS];
world_state_t world_state;

int input_count = 0;
int running = 1;

void init()
{
    if (enet_initialize() != 0)
    {
        printf("Failed to initialize ENet.\n");
        exit(EXIT_FAILURE);
    }

    ENetAddress address;

    enet_address_set_host(&address, "127.0.0.1");
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

    world_state.sequence = 0;
}

void receive_packets()
{
    ENetEvent net_event;

    while (enet_host_service(server, &net_event, 0) > 0)
    {
        switch (net_event.type)
        {
        case ENET_EVENT_TYPE_CONNECT:
            enet_address_get_host_ip(&net_event.peer->address, buffer, sizeof(buffer));
            printf("A new client connected from %s:%u\n", buffer, net_event.peer->address.port);

            char id = get_free_player(&world_state);

            if (id != -1)
            {
                peers[id] = net_event.peer;

                world_state.players[id].position.x = WINDOW_WIDTH / 2.0f;
                world_state.players[id].position.y = WINDOW_HEIGHT / 2.0f;
                world_state.players[id].velocity.x = 0.0f;
                world_state.players[id].velocity.y = 0.0f;
                world_state.players[id].angle = (float)M_PI * 1.5f;
                world_state.players[id].sequence = 0;

                ENetPacket* packet = enet_packet_create(&id, sizeof(id), 0);

                enet_peer_send(peers[id], 0, packet);

                net_event.peer->data = malloc(sizeof(char));
                *(char*)net_event.peer->data = id;
            }

            break;
        case ENET_EVENT_TYPE_RECEIVE:
            memcpy(&inputs[input_count++], net_event.packet->data, net_event.packet->dataLength);
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

void update()
{   
    for (int i = 0; i < input_count; i++)
    {
        player_state_t* player = &world_state.players[inputs[i].id];

        if (inputs[i].right)
        {
            player->angle += 0.1f;
        }

        if (inputs[i].left)
        {
            player->angle -= 0.1f;
        }

        if (inputs[i].thrust)
        {
            player->velocity.x += cosf(player->angle) * ACCELERATION;
            player->velocity.y += sinf(player->angle) * ACCELERATION;

            float speed = sqrtf(powf(player->velocity.x, 2.0f) + powf(player->velocity.y, 2.0f));

            if (speed > MAX_SPEED)
            {
                player->velocity.x *= MAX_SPEED / speed;
                player->velocity.y *= MAX_SPEED / speed;
            }
        }

        player->sequence = inputs[i].sequence;
    }

    input_count = 0;

    translate_world(&world_state);
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