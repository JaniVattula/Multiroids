#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <SDL.h>
#include <WinSock2.h>
#include <stdio.h>
#include <stdbool.h>

#define SERVER "127.0.0.1"
#define BUFLEN 64
#define PORT 8888

bool initializeSockets();
void shutdownSockets();

const int WINDOW_WIDTH = 640;
const int WINDOW_HEIGHT = 360;

int main(int argc, char* argv[])
{
	(void)argc;
	(void)argv;

	if (initializeSockets() == false)
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

	//----------------------------------------------------

	SDL_Event polled_event;
	bool running = true;

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
		running = false;
	}

	while (running)
	{
		while (SDL_PollEvent(&polled_event))
		{
			SDL_RenderClear(renderer);
			SDL_RenderCopy(renderer, texture, NULL, NULL);
			SDL_RenderPresent(renderer);

			memset(msg_buf, 0, BUFLEN);

			if (polled_event.type == SDL_QUIT)
			{
				running = false;
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
				}
			}

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
	}

	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();

	closesocket(s);
	shutdownSockets();
	return 0;
}

bool initializeSockets()
{
	WSADATA WsaData;
	return WSAStartup(MAKEWORD(2, 2), &WsaData) == NO_ERROR;
}

void shutdownSockets()
{
	WSACleanup();
}
