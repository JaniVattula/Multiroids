#include <SDL.h>
#include <stdio.h>
#include <enet\enet.h>
#include "sprite.h"

#define PORT 8888

ENetHost* server = NULL;
ENetPeer* client = NULL;

char buffer[256];

input_state_t input;
player_state_t player;

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

    server = enet_host_create(&address, 2, 2, 57600 / 4, 14400 / 4);

    if (server == NULL)
    {
        printf("Failed to create an ENet server host.\n");
        exit(EXIT_FAILURE);
    }

    enet_address_get_host_ip(&address, buffer, sizeof(buffer));
    printf("Launching new server at %s:%u\n", buffer, address.port);

    create_ship(128.0f, 128.0f, 64.0f, 64.0f, to_rad(45.0f));

    memset(&input, 0, sizeof(input));
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
            client = net_event.peer;
            client->data = "Player 1";
            break;
        case ENET_EVENT_TYPE_RECEIVE:
            memcpy(&input, net_event.packet->data, net_event.packet->dataLength);

            enet_packet_destroy(net_event.packet);
            
            break;
        case ENET_EVENT_TYPE_DISCONNECT:
            printf("%s disconnected.\n", (char*)net_event.peer->data);
            net_event.peer->data = NULL;
            client = NULL;
            break;
        }
    }
}

void update()
{   
    if (input.right)
    {
        players[0].angle += 0.1f;
    }

    if (input.left)
    {
        players[0].angle -= 0.1f;
    }

    if (input.thrust)
    {
        players[0].velocity.x += cosf(players[0].angle) * 0.125f;
        players[0].velocity.y += sinf(players[0].angle) * 0.125f;

        float speed = sqrtf(powf(players[0].velocity.x, 2.0f) + powf(players[0].velocity.y, 2.0f));

        if (speed > MAX_SPEED)
        {
            players[0].velocity.x *= MAX_SPEED / speed;
            players[0].velocity.y *= MAX_SPEED / speed;
        }
    }

    translate_sprites(players, MAX_PLAYERS);
}

void send_packets()
{
    if (client)
    { 
        player.angle = players[0].angle;
        player.position = players[0].position;
        player.velocity = players[0].velocity;

        ENetPacket* packet = enet_packet_create(&player, sizeof(player), 0);

        enet_host_broadcast(server, 1, packet);
    }
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
    double delta_time = 1 / 60.0;

    while (running)
    {
        double new_time = SDL_GetTicks() / 1000.0;
        double frame_time = new_time - current_time;

        current_time = new_time;
        accumulator += frame_time;

        receive_packets();

        while (accumulator >= delta_time)
        {
            update();
            accumulator -= delta_time;
        }

        send_packets();
    }

    deinit();
	return 0;
}