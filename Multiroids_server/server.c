#include <SDL.h>
#include <WinSock2.h>
#include <stdio.h>
#include <stdbool.h>
#include <enet\enet.h>

#define PORT 8888

int main(int argc, char* argv[])
{
	(void)argc;
	(void)argv;

	if (enet_initialize() != 0)
	{
		printf("Failed to initialize ENet.\n");
		exit(EXIT_FAILURE);
	}

	//---------------------------------------------

	ENetAddress address;
	ENetHost* server;
	ENetEvent net_event;

	enet_address_set_host(&address, "127.0.0.1");
	address.port = PORT;

	// Create the server (host address, number of clients,
	// number of channels, incoming bandwith, outgoing bandwith).
	server = enet_host_create(&address, 4, 2, 0, 0);
	
	if (server == NULL)
	{
		printf("Failed to create an ENet server host.\n");
		exit(EXIT_FAILURE);
	}

	printf("Launching new server at %x:%u\n", address.host, address.port);

	bool running = true;

	while (running)
	{
		while (enet_host_service(server, &net_event, 0) > 0)
		{
			printf("Listening to incoming connections.\n");
			switch (net_event.type)
			{
			case ENET_EVENT_TYPE_CONNECT:
				printf("A new client connected from %x:%u\n", net_event.peer->address.host, net_event.peer->address.port);
				net_event.peer->data = "Player 1";
				break;
			case ENET_EVENT_TYPE_RECEIVE:
				printf("A packet was received!\nSize: %u\nData: %s\nFrom: %s\nChannel: %u\n\n", (uint32_t)net_event.packet->dataLength, (char*)net_event.packet->data, (char*)net_event.peer->data, net_event.channelID);
				enet_packet_destroy(net_event.packet);
				break;
			case ENET_EVENT_TYPE_DISCONNECT:
				printf("%s disconnected.\n", (char*)net_event.peer->data);
				net_event.peer->data = NULL;
				break;
			}
		}
	}
	
	enet_host_destroy(server);
	enet_deinitialize();

	return 0;
}