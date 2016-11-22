#include <SDL.h>
#include <stdio.h>
#include <enet\enet.h>
#include <math.h>
#include "sprite.h"

#define SERVER "127.0.0.1"
#define PORT 8888

#define PLAYER 0
#define ASTEROID 1
#define BULLET 2

#define MAX_POINTS 8
#define MAX_PLAYERS 4
#define MAX_ASTEROIDS 32
#define MAX_BULLETS 256
#define MAX_SPEED 4

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

sprite_t players[MAX_PLAYERS];

int turn_left = 0;
int turn_right = 0;
int thrust = 0;
int running = 1;

ENetHost* client = NULL;
ENetPeer* peer = NULL;
ENetPacket* packet = NULL;
ENetEvent net_event;

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;

char buffer[256];

float to_rad(float deg)
{
    return deg * (float)M_PI / 180.0f;
}

float to_deg(float rad)
{
    return rad * 180.0f / (float)M_PI;
}

void init_sprites()
{
    for (size_t i = 0; i < MAX_PLAYERS; i++)
    {
        players[i].alive = 0;
    }
}

sprite_t* create_ship(float x, float y, float width, float height, float angle)
{
    sprite_t* sprite = get_sprite(players, MAX_PLAYERS);

    if (sprite)
    {
        sprite->angle = angle;
        sprite->count = 5;
        sprite->position.x = x;
        sprite->position.y = y;
        sprite->size.x = width;
        sprite->size.y = height;

        sprite->points[0].x = -0.5f;    sprite->points[0].y = -0.5f;
        sprite->points[1].x = 0.5f;     sprite->points[1].y = 0.0f;
        sprite->points[2].x = -0.5f;    sprite->points[2].y = 0.5f;
        sprite->points[3].x = -0.25f;   sprite->points[3].y = 0.0f;
        sprite->points[4].x = -0.5f;    sprite->points[4].y = -0.5f;
    }

    return sprite;
}

ENetPeer* connectToHost(ENetHost* client)
{
    ENetAddress address;
    ENetEvent net_event;
    ENetPeer* peer;
    char ip[256];

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
        enet_address_get_host_ip(&peer->address, ip, sizeof(ip));
        printf("Connection to %s:%d established.\n", ip, peer->address.port);
    }
    else
    {
        enet_peer_reset(peer);
        printf("Connection to %s:%d failed.\n", SERVER, PORT);
        exit(EXIT_FAILURE);
    }

    return peer;
}

void init()
{
    if (enet_initialize() != 0)
    {
        printf("Failed to initialize ENet.\n");
        exit(EXIT_FAILURE);
    }

    SDL_Init(SDL_INIT_VIDEO);
    window = SDL_CreateWindow("Hell no world!", 100, 100, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    create_ship(128.0f, 128.0f, 64.0f, 64.0f, to_rad(45.0f));
    create_ship(512.0f, 32.0f, 32.0f, 32.0f, 0.0f);
    create_ship(128.0f, 256.0f, 128.0f, 128.0f, to_rad(45.0f));
    create_ship(256.0f, 256.0f, 64.0f, 64.0f, to_rad(180.0f));

    client = enet_host_create(NULL, 1, 2, 0, 0);

    if (client == NULL)
    {
        printf("Failed to create an ENet client host.\n");
        exit(EXIT_FAILURE);
    }

    peer = connectToHost(client);
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
                thrust = 1;
                break;
            case SDLK_s:
                break;
            case SDLK_a:
                turn_left = 1;
                break;
            case SDLK_d:
                turn_right = 1;
                break;
            case SDLK_f:
                free_sprite(&players[3]);
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
                thrust = 0;
                break;
            case SDLK_a:
                turn_left = 0;
                break;
            case SDLK_d:
                turn_right = 0;
                break;
            }
        }
    }
}

void network_stuff()
{
    packet = enet_packet_create("Packet data WOO!", 18, 0);

    enet_peer_send(peer, 0, packet);
    enet_address_get_host_ip(&peer->address, buffer, sizeof(buffer));
    //printf("Sending packet to %s:%u\n", ip, peer->address.port);

    while (enet_host_service(client, &net_event, 0) > 0)
    {
        printf("Listening to incoming connections.\n");
        switch (net_event.type)
        {
        case ENET_EVENT_TYPE_CONNECT:
            enet_address_get_host_ip(&net_event.peer->address, buffer, sizeof(buffer));
            printf("A new client connected from %s:%u\n", buffer, net_event.peer->address.port);
            net_event.peer->data = "Player 1";
            break;
        case ENET_EVENT_TYPE_RECEIVE:
            printf("A packet was received!\nSize: %u\nData: %s\n From: %s\nChannel: %u\n\n", (uint32_t)net_event.packet->dataLength, (char*)net_event.packet->data, (char*)net_event.peer->data, net_event.channelID);
            enet_packet_destroy(net_event.packet);
            break;
        case ENET_EVENT_TYPE_DISCONNECT:
            printf("%s disconnected.\n", (char*)net_event.peer->data);
            net_event.peer->data = NULL;
            break;
        }
    }
}

void update()
{
    if (turn_right)
    {
        players[1].angle += 0.1f;
    }

    if (turn_left)
    {
        players[1].angle -= 0.1f;
    }

    if (thrust)
    {
        players[1].velocity.x += cosf(players[1].angle) * 0.125f;
        players[1].velocity.y += sinf(players[1].angle) * 0.125f;

        float speed = sqrtf(powf(players[1].velocity.x, 2.0f) + powf(players[1].velocity.y, 2.0f));

        if (speed > MAX_SPEED)
        {
            players[1].velocity.x *= MAX_SPEED / speed;
            players[1].velocity.y *= MAX_SPEED / speed;
        }
    }

    players[0].angle += 0.025f;

    translate_sprites(players, MAX_PLAYERS);
}

void render()
{
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);

    render_sprites(renderer, players, MAX_PLAYERS);

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
        network_stuff();

        while (accumulator >= delta_time)
        {
            update();
            accumulator -= delta_time;
        }

        render();
	}

    deinit();

	return 0;
}