#include <SDL.h>
#include <stdio.h>
#include <enet\enet.h>

#define PORT 8888
#define PACKET_SIZE 4

ENetHost* server = NULL;
ENetAddress address;
ENetEvent net_event;

char buffer[256];
char packet_data[4];
int running = 1;

void init()
{
    if (enet_initialize() != 0)
    {
        printf("Failed to initialize ENet.\n");
        exit(EXIT_FAILURE);
    }

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

    enet_address_get_host_ip(&address, buffer, sizeof(buffer));
    printf("Launching new server at %s:%u\n", buffer, address.port);
}

void receive_packets()
{
    while (enet_host_service(server, &net_event, 0) > 0)
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
			for (int i = 0; i < PACKET_SIZE; i++)
			{
				packet_data[i] = net_event.packet->data[i];
			}
            printf("A packet was received!\nSize: %u\nThrust: %d | Left: %d | Right: %d\nFrom: %s\nChannel: %u\n\n", (uint32_t)net_event.packet->dataLength, (int)packet_data[0] - 48, (int)packet_data[1] - 48, (int)packet_data[2] - 48, (char*)net_event.peer->data, net_event.channelID);
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

}

void send_packets()
{

}

void deinit()
{
    enet_host_destroy(server);
    enet_deinitialize();
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

        receive_packets();

        while (accumulator >= delta_time)
        {
            update();
            accumulator -= delta_time;
        }

        send_packets();
    }

    deinit();
	return 0;
}