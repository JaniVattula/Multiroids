#include <SDL.h>
#include <stdio.h>
#include <enet\enet.h>
#include <math.h>
#include "sprite.h"

#define SERVER "127.0.0.1"
#define PORT 8888

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

int running = 1;

input_state_t input;
world_state_t world_state;

ENetHost* client = NULL;
ENetPeer* peer = NULL;

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;

char buffer[256];

void connect_to_host()
{
    ENetAddress address;
    ENetEvent net_event;

    enet_address_set_host(&address, SERVER);
    address.port = PORT;

    peer = enet_host_connect(client, &address, 2, 0);

    if (peer == NULL)
    {
        printf("No available peers for connection.\n");
        exit(EXIT_FAILURE);
    }

    if (enet_host_service(client, &net_event, 5000) > 0 && net_event.type == ENET_EVENT_TYPE_CONNECT)
    {
        enet_address_get_host_ip(&peer->address, buffer, sizeof(buffer));
        printf("Connection to %s:%d established.\n", buffer, peer->address.port);

        if (enet_host_service(client, &net_event, 5000) > 0 && net_event.type == ENET_EVENT_TYPE_RECEIVE)
        {
            int id = 0;
            memcpy(&id, net_event.packet->data, net_event.packet->dataLength);

            enet_packet_destroy(net_event.packet);

            input.id = id;

            printf("I am player %d!\n", input.id);
        }
    }
    else
    {
        enet_peer_reset(peer);
        printf("Connection to %s:%d failed.\n", SERVER, PORT);
        exit(EXIT_FAILURE);
    }
}

void init()
{
    memset(&input, 0, sizeof(input));

    if (enet_initialize() != 0)
    {
        printf("Failed to initialize ENet.\n");
        exit(EXIT_FAILURE);
    }

    SDL_Init(SDL_INIT_VIDEO);
    window = SDL_CreateWindow("Multiroids", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    client = enet_host_create(NULL, 1, 2, 0, 0);

    if (client == NULL)
    {
        printf("Failed to create an ENet client host.\n");
        exit(EXIT_FAILURE);
    }

    connect_to_host();

    char title[256];

    sprintf(title, "Multiroids: Player %d", input.id + 1);

    SDL_SetWindowTitle(window, title);
}

void poll_events()
{
    SDL_Event polled_event;

    while (SDL_PollEvent(&polled_event))
    {
        if (polled_event.type == SDL_QUIT)
        {
            running = 0;
        }
        else if (polled_event.type == SDL_KEYDOWN)
        {
            switch (polled_event.key.keysym.sym)
            {
            case SDLK_w:
                input.thrust = 1;
                break;
            case SDLK_a:
                input.left = 1;
                break;
            case SDLK_d:
                input.right = 1;
                break;
            case SDLK_ESCAPE:
                running = 0;
                break;
            }
        }
        else if (polled_event.type == SDL_KEYUP)
        {
            switch (polled_event.key.keysym.sym)
            {
            case SDLK_w:
                input.thrust = 0;
                break;
            case SDLK_a:
                input.left = 0;
                break;
            case SDLK_d:
                input.right = 0;
                break;
            }
        }
    }
}

void network_stuff()
{
    ENetEvent net_event;

    ENetPacket* packet = enet_packet_create(&input, sizeof(input), 0);

    enet_peer_send(peer, 0, packet);

    while (enet_host_service(client, &net_event, 0) > 0)
    {
        switch (net_event.type)
        {
        case ENET_EVENT_TYPE_CONNECT:
            break;
        case ENET_EVENT_TYPE_RECEIVE:
            memcpy(&world_state, net_event.packet->data, net_event.packet->dataLength);
            enet_packet_destroy(net_event.packet);

            break;
        case ENET_EVENT_TYPE_DISCONNECT:
            break;
        }
    }
}

void update()
{
}

void render()
{
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);

    render_world(renderer, &world_state);

    SDL_RenderPresent(renderer);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
}

void deinit()
{
    enet_host_destroy(client);
    enet_deinitialize();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
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

        poll_events();

        while (accumulator >= delta_time)
        {
            network_stuff();

            update();
            accumulator -= delta_time;
        }

        render();
	}

    deinit();

	return 0;
}