#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <SDL.h>
#include <WinSock2.h>
#include <stdio.h>
#include <stdbool.h>
#include <enet\enet.h>

#define SERVER "127.0.0.1"
#define PORT 8888

const int WINDOW_WIDTH = 640;
const int WINDOW_HEIGHT = 360;

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

	SDL_Event polled_event;
	bool running = true;

	ENetHost* client;
	client = enet_host_create(NULL, 1, 2, 0, 0);

	if (client == NULL)
	{
		printf("Failed to create an ENet client host.\n");
		exit(EXIT_FAILURE);
	}

	while (running)
	{
		while (SDL_PollEvent(&polled_event))
		{
			SDL_RenderClear(renderer);
			SDL_RenderCopy(renderer, texture, NULL, NULL);
			SDL_RenderPresent(renderer);

			if (polled_event.type == SDL_QUIT)
			{
				running = false;
			}

			else if (polled_event.type == SDL_KEYDOWN)
			{
				switch (polled_event.key.keysym.sym)
				{
				case SDLK_w:
					printf("W ");
					break;
				case SDLK_s:
					printf("S ");
					break;
				case SDLK_a:
					printf("A ");
					break;
				case SDLK_d:
					printf("D ");
					break;
				}
			}
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