#include <SDL.h>
#include <stdio.h>
#include <enet\enet.h>

#define SERVER "172.31.16.47"
#define PORT 8888

const int WINDOW_WIDTH = 640;
const int WINDOW_HEIGHT = 360;

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

			if (polled_event.type == SDL_QUIT)
			{
				running = 0;
			}

			else if (polled_event.type == SDL_KEYDOWN)
			{
				switch (polled_event.key.keysym.sym)
				{
				case SDLK_w:
					enet_peer_send(peer, 0, packet);
					enet_address_get_host_ip(&peer->address, ip, sizeof(ip));
					printf("Sending packet to %s:%u\n", ip, peer->address.port);
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