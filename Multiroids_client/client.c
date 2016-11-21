#include <SDL.h>
#include <stdio.h>
#include <enet\enet.h>
#include <math.h>

#define SERVER "172.31.16.47"
#define PORT 8888

#define PLAYER 0
#define ASTEROID 1
#define BULLET 2

#define MAX_POINTS 8
#define MAX_PLAYERS 4
#define MAX_ASTEROIDS 32
#define MAX_BULLETS 256
#define MAX_SPEED 4

int turn_left = 0;
int turn_right = 0;
int thrust = 0;

int current_sprite = 0;

float to_rad(float deg)
{
    return deg * (float)M_PI / 180.0f;
}

float to_deg(float rad)
{
    return rad * 180.0f / (float)M_PI;
}

typedef struct point_t
{
    float x;
    float y;
} point_t;

typedef struct sprite_t
{
    point_t position;
    point_t velocity;
    point_t size;
    point_t points[MAX_POINTS];
    size_t count;
    float angle;
    int alive;
} sprite_t;

sprite_t players[MAX_PLAYERS];

int initializeSockets();
void shutdownSockets();

sprite_t* get_sprite(sprite_t* sprites, int count);
void free_sprite(sprite_t* sprite);

void init_sprites();

void translate_sprites(sprite_t* sprites, int count);
void render_sprites(SDL_Renderer* renderer, sprite_t* sprites, int count);

sprite_t* create_ship(float x, float y, float width, float height, float angle);

const int WINDOW_WIDTH = 640;
const int WINDOW_HEIGHT = 480;

ENetPeer* connectToHost(ENetHost* client);

int main(int argc, char* argv[])
{
	(void)argc;
	(void)argv;

	if (enet_initialize() != 0)
	{
		printf("Failed to initialize ENet.\n");
		exit(EXIT_FAILURE);
	}

	SDL_Init(SDL_INIT_VIDEO);
	SDL_Window* window = SDL_CreateWindow("Hell no world!", 100, 100, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	SDL_Surface* bmp = SDL_LoadBMP("../assets/hello.bmp");
	SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, bmp);
	SDL_FreeSurface(bmp);

    create_ship(128.0f, 128.0f, 64.0f, 64.0f, to_rad(45.0f));
    create_ship(512.0f, 32.0f, 32.0f, 32.0f, 0.0f);
    create_ship(128.0f, 256.0f, 128.0f, 128.0f, to_rad(45.0f));
    create_ship(256.0f, 256.0f, 64.0f, 64.0f, to_rad(180.0f));

	//----------------------------------------------------

	SDL_Event polled_event;
	ENetPacket* packet;
	int running = 1;

	ENetHost* client;
	client = enet_host_create(NULL, 1, 2, 0, 0);

	if (client == NULL)
	{
		printf("Failed to create an ENet client host.\n");
		exit(EXIT_FAILURE);
	}

	ENetPeer* peer;
	peer = connectToHost(client);

	ENetEvent net_event;
	char ip[256];

    double delta_time = 1 / 60.0;
    double current_time = SDL_GetTicks() / 1000.0;
    double accumulator = 0.0;

	while (running)
	{
		while (SDL_PollEvent(&polled_event))
		{
			SDL_RenderClear(renderer);
			SDL_RenderCopy(renderer, texture, NULL, NULL);
			SDL_RenderPresent(renderer);

			packet = enet_packet_create("Packet data WOO!", 18, 0);

			while (enet_host_service(client, &net_event, 0) > 0)
			{
				printf("Listening to incoming connections.\n");
				switch (net_event.type)
				{
				case ENET_EVENT_TYPE_CONNECT:
					enet_address_get_host_ip(&net_event.peer->address, ip, sizeof(ip));
					printf("A new client connected from %s:%u\n", ip, net_event.peer->address.port);
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
                    msg_buf[0] = 1;
                    //printf("W ");
                    thrust = 1;
                    break;
                case SDLK_s:
                    msg_buf[0] = 2;
                    //printf("S ");
                    break;
                case SDLK_a:
                    msg_buf[0] = 3;
                    //printf("A ");
                    turn_left = 1;
                    break;
                case SDLK_d:
                    msg_buf[0] = 4;
                    //printf("D ");
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

        double new_time = SDL_GetTicks() / 1000.0f;
        double frame_time = new_time - current_time;
        current_time = new_time;

        accumulator += frame_time;

        while (accumulator >= delta_time)
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

            accumulator -= delta_time;
        }

        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);

        render_sprites(renderer, players, MAX_PLAYERS);

        SDL_RenderPresent(renderer);

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

        if (msg_buf[0] != 0)
        {
            sendto(s, msg_buf, BUFLEN, 0, (struct sockaddr*)&addr_server, slen);

            //printf("Sending to %s:%d\n", inet_ntoa(addr_server.sin_addr), ntohs(addr_server.sin_port));

            memset(recv_buf, 0, BUFLEN);
            recvfrom(s, recv_buf, BUFLEN, 0, (struct sockaddr*)&addr_server, &slen);

            //Sleep(50);

            //printf("Server is receiving data!\n");
        }
	}

	enet_host_destroy(client);
	enet_deinitialize();

	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
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

void shutdownSockets()
{
	WSACleanup();
}

sprite_t* get_sprite(sprite_t* sprites, int count)
{
    for (int i = 0; i < count; i++)
    {
        if (sprites[i].alive == 0)
        {
            sprites[i].alive = 1;
            
            return &sprites[i];
        }
    }

    return NULL;
}

void free_sprite(sprite_t* sprite)
{
    sprite->alive = 0;
}

void init_sprites()
{
    for (size_t i = 0; i < MAX_PLAYERS; i++)
    {
        players[i].alive = 0;
    }
}

void translate_sprites(sprite_t* sprites, int count)
{
    for (int i = 0; i < count; i++)
    {
        if (sprites[i].alive == 0)
            continue;

        sprites[i].position.x += sprites[i].velocity.x;
        sprites[i].position.y += sprites[i].velocity.y;
    }
}

void render_sprites(SDL_Renderer* renderer, sprite_t* sprites, int count)
{
    static SDL_Point points[MAX_POINTS];

    for (int i = 0; i < count; i++)
    {
        if (sprites[i].alive == 0)
            continue;

        float s = sinf(sprites[i].angle);
        float c = cosf(sprites[i].angle);
        float x = 0;
        float y = 0;

        for (size_t j = 0; j < sprites[i].count; j++)
        {
            x = sprites[i].points[j].x * c - sprites[i].points[j].y * s;
            y = sprites[i].points[j].x * s + sprites[i].points[j].y * c;

            x *= sprites[i].size.x;
            y *= sprites[i].size.y;

            points[j].x = (int)(x + sprites[i].position.x);
            points[j].y = (int)(y + sprites[i].position.y);
        }

        SDL_RenderDrawLines(renderer, points, (int)sprites[i].count);
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