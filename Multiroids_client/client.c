#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <SDL.h>
#include <WinSock2.h>
#include <stdio.h>
#include <math.h>

#define SERVER "127.0.0.1"
#define BUFLEN 64
#define PORT 8888

#define MAX_POINTS 8
#define MAX_PLAYERS 4

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
    point_t size;
    point_t points[MAX_POINTS];
    size_t count;
    float angle;
} sprite_t;

int initializeSockets();
void shutdownSockets();

void render_sprites(SDL_Renderer* renderer, sprite_t* sprites, size_t count);
sprite_t create_ship(float x, float y, float width, float height, float angle);

const int WINDOW_WIDTH = 640;
const int WINDOW_HEIGHT = 480;

int main(int argc, char* argv[])
{
	(void)argc;
	(void)argv;

	if (initializeSockets() == 0)
	{
		printf("Failed to initialize sockets.");
		return 0;
	}

	if (SDL_Init(SDL_INIT_VIDEO) != 0)
	{
		printf("SDL_Init error: %s\n", SDL_GetError());
		return 1;
	}

	SDL_Window* window = SDL_CreateWindow("Hell no world!", 100, 100, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
	if (window == NULL)
	{
		printf("SDL_CreateWindow error: %s\n", SDL_GetError());
		SDL_Quit();
		return 1;
	}

	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (renderer == NULL)
	{
		SDL_DestroyWindow(window);
		printf("SDL_CreateRenderer error: %s\n", SDL_GetError());
		SDL_Quit();
		return 1;
	}

	SDL_Surface* bmp = SDL_LoadBMP("../assets/hello.bmp");
	if (bmp == NULL)
	{
		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);
		printf("SDL_LoadBMP error: %s\n", SDL_GetError());
		SDL_Quit();
		return 1;
	}

	SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, bmp);
	SDL_FreeSurface(bmp);
	if (texture == NULL)
	{
		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);
		printf("SDL_CreateTextureFromSurface error: %s\n", SDL_GetError());
		SDL_Quit();
		return 1;
	}


    sprite_t players[MAX_PLAYERS];
    players[0] = create_ship(128.0f, 128.0f, 64.0f, 64.0f, to_rad(45.0f));
    players[1] = create_ship(512.0f, 32.0f, 32.0f, 32.0f, 0.0f);
    players[2] = create_ship(128.0f, 256.0f, 128.0f, 128.0f, to_rad(45.0f));
    players[3] = create_ship(256.0f, 256.0f, 64.0f, 64.0f, to_rad(180.0f));

	//----------------------------------------------------

	SDL_Event polled_event;
	int running = 1;

	SOCKET s;
	struct sockaddr_in addr_server;
	char recv_buf[BUFLEN];
	char msg_buf[BUFLEN];
	int slen;

	s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	slen = sizeof(addr_server);

	addr_server.sin_family = AF_INET;
	addr_server.sin_addr.s_addr = inet_addr("127.0.0.1");
	addr_server.sin_port = htons(PORT);

	// Turn on non-blocking mode for listening.
	DWORD nonBlocking = 1;
	if (ioctlsocket(s, FIONBIO, &nonBlocking) != 0)
	{
		printf("Failed to set non-blocking!\n");
		running = 0;
	}

    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);

	while (running)
	{
        memset(msg_buf, 0, BUFLEN);

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
					printf("W ");
					break;
				case SDLK_s:
					msg_buf[0] = 2;
					printf("S ");
					break;
				case SDLK_a:
					msg_buf[0] = 3;
                    players[1].angle -= to_rad(45);
					printf("A ");
					break;
				case SDLK_d:
					msg_buf[0] = 4;
                    players[1].angle += to_rad(45.0f);
					printf("D ");
					break;
                case SDLK_ESCAPE:
                    running = 0;
                    break;
				}
			}
		}

        players[0].angle += 0.025f;

        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);

        render_sprites(renderer, players, MAX_PLAYERS);

        SDL_RenderPresent(renderer);

        if (msg_buf[0] != 0)
        {
            sendto(s, msg_buf, BUFLEN, 0, (struct sockaddr*)&addr_server, slen);

            printf("Sending to %s:%d\n", inet_ntoa(addr_server.sin_addr), ntohs(addr_server.sin_port));

            memset(recv_buf, 0, BUFLEN);
            recvfrom(s, recv_buf, BUFLEN, 0, (struct sockaddr*)&addr_server, &slen);

            Sleep(50);

            printf("Server is receiving data!\n");
        }
	}

	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();

	closesocket(s);
	shutdownSockets();
	return 0;
}

int initializeSockets()
{
	WSADATA WsaData;
	return WSAStartup(MAKEWORD(2, 2), &WsaData) == NO_ERROR;
}

void shutdownSockets()
{
	WSACleanup();
}

void render_sprites(SDL_Renderer* renderer, sprite_t* sprites, size_t count)
{
    static SDL_Point points[MAX_POINTS];

    for (size_t i = 0; i < count; i++)
    {
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

sprite_t create_ship(float x, float y, float width, float height, float angle)
{
    sprite_t ship;
    ship.angle = angle;
    ship.count = 5;
    ship.position.x = x;
    ship.position.y = y;
    ship.size.x = width;
    ship.size.y = height;

    ship.points[0].x = -0.5f;   ship.points[0].y = 0.5f;
    ship.points[1].x = 0.0f;    ship.points[1].y = -0.5f;
    ship.points[2].x = 0.5f;    ship.points[2].y = 0.5f;
    ship.points[3].x = 0.0f;    ship.points[3].y = 0.25f;
    ship.points[4].x = -0.5f;   ship.points[4].y = 0.5f;

    return ship;
}