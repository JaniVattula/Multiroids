#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <SDL.h>
#include <WinSock2.h>
#include <stdio.h>

#define BUFLEN 64
#define PORT 8888

int initializeSockets();
void shutdownSockets();

int main(int argc, char* argv[])
{
	(void)argc;
	(void)argv;

	if (initializeSockets() == 0)
	{
		printf("Failed to initialize sockets.");
		return 0;
	}

	SOCKET s;
	struct sockaddr_in addr_server, addr_client;
	char recv_buf[BUFLEN];
	int recv_len, slen;

	s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	slen = sizeof(addr_client);

	addr_server.sin_family = AF_INET;
	addr_server.sin_addr.s_addr = inet_addr("127.0.0.1");
	addr_server.sin_port = htons(PORT);
	bind(s, (struct sockaddr*)&addr_server, sizeof(addr_server));

	int running = 1;
	DWORD nonBlocking = 1;

	if (ioctlsocket(s, FIONBIO, &nonBlocking) != 0)
	{
		printf("Failed to set non-blocking!\n");
		running = 0;
	}

	while (running)
	{
		if (recv_buf[0] == 0)
		{
			printf("Address: %s:%d\nWaiting for data...\n", inet_ntoa(addr_server.sin_addr), PORT);
			fflush(stdout);
		}
		memset(recv_buf, 0, BUFLEN);
		recv_len = recvfrom(s, recv_buf, BUFLEN, 0, (struct sockaddr*)&addr_client, &slen);
		
		printf("%s", recv_buf);

		if (recv_buf[0] != 0)
		{
			switch (recv_buf[0])
			{
			case 1:
				printf("Player from %s:%d is moving up!\n", inet_ntoa(addr_client.sin_addr), ntohs(addr_client.sin_port));
				break;
			case 2:
				printf("Player from %s:%d is moving down!\n", inet_ntoa(addr_client.sin_addr), ntohs(addr_client.sin_port));
				break;
			case 3:
				printf("Player from %s:%d is moving left!\n", inet_ntoa(addr_client.sin_addr), ntohs(addr_client.sin_port));
				break;
			case 4:
				printf("Player from %s:%d is moving right!\n", inet_ntoa(addr_client.sin_addr), ntohs(addr_client.sin_port));
				break;
			}
		}

		sendto(s, recv_buf, recv_len, 0, (struct sockaddr*)&addr_client, slen);
		Sleep(50);
		system("cls");
	}
	
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
