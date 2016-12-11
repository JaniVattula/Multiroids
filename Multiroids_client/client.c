#include <SDL.h>
#include <stdio.h>
#include <enet\enet.h>
#include <math.h>
#include "common.h"

#define MAX_INPUTS 128

int running = 1;
int input_count = 0;
int bullet_count = 0;

double interpolation = 0.0;

input_state_t input;
world_state_t world_state;
world_state_t prev_state;
world_state_t new_state;
input_state_t inputs[MAX_INPUTS];
bullet_state_t bullets[MAX_BULLETS];
player_state_t* player;

ENetHost* client = NULL;
ENetPeer* peer = NULL;

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;

char buffer[256];

void connect_to_host(char* ip, int port)
{
    ENetAddress address;
    ENetEvent net_event;

    enet_address_set_host(&address, ip);
    address.port = port;

    peer = enet_host_connect(client, &address, 2, 0);

    if (peer == NULL)
    {
        printf("No available peers for connection.\n");
        exit(EXIT_FAILURE);
    }

    if (enet_host_service(client, &net_event, 5000) > 0 && 
        net_event.type == ENET_EVENT_TYPE_CONNECT)
    {
        enet_address_get_host_ip(&peer->address, buffer, sizeof(buffer));
        printf("Connection to %s:%d established.\n", buffer, peer->address.port);

        if (enet_host_service(client, &net_event, 5000) > 0 && 
            net_event.type == ENET_EVENT_TYPE_RECEIVE)
        {
            int id = 0;
            memcpy(&id, net_event.packet->data, net_event.packet->dataLength);

            enet_packet_destroy(net_event.packet);

            input.id = id;
            input.sequence = 0;
            input.type = PACKET_INPUT;

            printf("I am player %d!\n", input.id);
        }
    }
    else
    {
        enet_peer_reset(peer);
        printf("Connection to %s:%d failed.\n", ip, port);
        exit(EXIT_FAILURE);
    }
}

void init(char* ip, int port)
{
    printf("SIZE: %zd\n", sizeof(world_state));
    memset(&input, 0, sizeof(input));

    if (enet_initialize() != 0)
    {
        printf("Failed to initialize ENet.\n");
        exit(EXIT_FAILURE);
    }

    SDL_Init(SDL_INIT_VIDEO);

    window = SDL_CreateWindow(
        "Multiroids", 
        SDL_WINDOWPOS_CENTERED, 
        SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, 
        WINDOW_HEIGHT, 
        SDL_WINDOW_SHOWN);

    renderer = SDL_CreateRenderer(window, -1, 
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    client = enet_host_create(NULL, 1, 2, 0, 0);

    if (client == NULL)
    {
        printf("Failed to create an ENet client host.\n");
        exit(EXIT_FAILURE);
    }

    connect_to_host(ip, port);

    char title[256];

    sprintf(title, "Multiroids: Player %d", input.id + 1);

    player = &world_state.players[input.id];

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
            case SDLK_UP:
                input.thrust = 1;
                break;
            case SDLK_LEFT:
                input.left = 1;
                break;
            case SDLK_RIGHT:
                input.right = 1;
                break;
            case SDLK_LCTRL:
            case SDLK_RCTRL:
                input.shoot = 1;
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
            case SDLK_UP:
                input.thrust = 0;
                break;
            case SDLK_LEFT:
                input.left = 0;
                break;
            case SDLK_RIGHT:
                input.right = 0;
                break;
            case SDLK_LCTRL:
            case SDLK_RCTRL:
                input.shoot = 0;
                break;
            }
        }
    }
}

void interpolate_world()
{
    float t = (float)(interpolation / network_step);

    t = t > 1.0f ? 1.0f : t;

    for (int i = 0; i < MAX_PLAYERS; i++)
    {
        if (i == input.id || !is_player_alive(&world_state, i))
            continue;

        world_state.players[i].angle = prev_state.players[i].angle + t * (new_state.players[i].angle - prev_state.players[i].angle);
        world_state.players[i].position.x = prev_state.players[i].position.x + t * (new_state.players[i].position.x - prev_state.players[i].position.x);
        world_state.players[i].position.y = prev_state.players[i].position.y + t * (new_state.players[i].position.y - prev_state.players[i].position.y);
    }
}

void update()
{
    if (input.right)
    {
        player->angle += 0.1f;
    }

    if (input.left)
    {
        player->angle -= 0.1f;
    }

    if (input.thrust)
    {
        player->velocity.x += cosf(player->angle) * ACCELERATION;
        player->velocity.y += sinf(player->angle) * ACCELERATION;

        float speed = sqrtf(
            powf(player->velocity.x, 2.0f) +
            powf(player->velocity.y, 2.0f));

        if (speed > MAX_SPEED)
        {
            player->velocity.x *= MAX_SPEED / speed;
            player->velocity.y *= MAX_SPEED / speed;
        }
    }

    interpolate_world();
    translate_world(&world_state);
    translate_bullets(bullets, bullet_count);
}

void network_stuff()
{
    world_state_t* state;
    uint8_t* packet_type = NULL;

    ENetEvent net_event;
    ENetPacket* packet = enet_packet_create(
        &input, 
        sizeof(input), 
        ENET_PACKET_FLAG_RELIABLE);

    enet_peer_send(peer, 0, packet);

    while (enet_host_service(client, &net_event, 0) > 0)
    {
        switch (net_event.type)
        {
        case ENET_EVENT_TYPE_CONNECT:
            break;
        case ENET_EVENT_TYPE_RECEIVE:
            packet_type = (uint8_t*)net_event.packet->data;

            switch (*packet_type)
            {
            case PACKET_WORLD:
                state = (world_state_t*)packet_type;

                prev_state = world_state;
                world_state = new_state = *state;

                interpolation = 0.0;

                int j = 0;

                for (int i = 0; i < input_count; i++)
                {
                    if (inputs[i].sequence > player->sequence)
                    {
                        inputs[j++] = input = inputs[i];

                        update();
                    }
                }

                input_count = j;
                break;
            case PACKET_BULLET:
                bullets[bullet_count++] = *(bullet_state_t*)packet_type;
                break;
            }

            enet_packet_destroy(net_event.packet);

            break;
        case ENET_EVENT_TYPE_DISCONNECT:
            break;
        }
    }
}

void render()
{
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);

    render_world(renderer, &world_state);
    render_bullets(renderer, bullets, bullet_count);

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

    FILE* file = NULL;
    char ip[255];
    int port = 0;

    if (file = fopen("config.txt", "r"))
    {
        fscanf(file, "%s %d", ip, &port);
        fclose(file);
    }
    else
    {
        printf("Config file not found!\n");
        printf("Type server ip and address(format: 127.0.0.1 8888)\n");
        scanf("%s %d", ip, &port);
    }

    init(ip, port);

    double accumulator = 0.0;
    double current_time = 0.0;

	while (running)
	{
        double new_time = SDL_GetTicks() / 1000.0;
        double frame_time = new_time - current_time;

        current_time = new_time;
        accumulator += frame_time;

        poll_events();

        while (accumulator >= physics_step)
        {
            update();


            input.sequence++;
            inputs[input_count++] = input;

            network_stuff();
            accumulator -= physics_step;
            interpolation += physics_step;
        }

        render();
	}

    enet_peer_disconnect(peer, 0);

    network_stuff();

    deinit();

	return 0;
}