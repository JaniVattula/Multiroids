#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <SDL.h>
#include <WinSock2.h>
#include <stdio.h>

#define SERVER "127.0.0.1"
#define BUFLEN 64
#define PORT 8888

typedef struct shape_t
{
    SDL_Point* points;
    size_t count;
} shape_t;

int initializeSockets();
void shutdownSockets();

void rotate_shape(const shape_t* const source, shape_t* const dest, float angle, SDL_Point pivot);

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

    size_t count = 5;

    shape_t ship;
    ship.count = count;
    ship.points = malloc(sizeof(SDL_Point) * count);
    ship.points[0].x = 32; ship.points[0].y = 96;
    ship.points[1].x = 64; ship.points[1].y = 32;
    ship.points[2].x = 96; ship.points[2].y = 96;
    ship.points[3].x = 64; ship.points[3].y = 80;
    ship.points[4].x = 32; ship.points[4].y = 96;

    shape_t rotated;
    rotated.count = count;
    rotated.points = malloc(sizeof(SDL_Point) * count);

    float angle = 0.0f;

    SDL_Point anchor = { 64, 64 };

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
					printf("A ");
					break;
				case SDLK_d:
					msg_buf[0] = 4;
					printf("D ");
					break;
                case SDLK_ESCAPE:
                    running = 0;
                    break;
				}
			}
		}

        rotate_shape(&ship, &rotated, angle, anchor);

        angle += 0.025f;

        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);

        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_RenderDrawLines(renderer, rotated.points, (int)rotated.count);

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

    free(ship.points);
    free(rotated.points);

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

void rotate_shape(const shape_t* const source, shape_t* const dest, float angle, SDL_Point pivot)
{
    float s = sinf(angle);
    float c = cosf(angle);
    float x = 0;
    float y = 0;

    for (size_t i = 0; i < source->count; i++)
    {
        dest->points[i] = source->points[i];
        dest->points[i].x -= pivot.x;
        dest->points[i].y -= pivot.y;

        x = dest->points[i].x * c - dest->points[i].y * s;
        y = dest->points[i].x * s + dest->points[i].y * c;

        dest->points[i].x = (int)x + pivot.x;
        dest->points[i].y = (int)y + pivot.y;
    }
}
