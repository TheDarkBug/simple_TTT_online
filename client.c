#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#ifdef __linux__
#include <sys/socket.h>
#include <arpa/inet.h>
#define SOCK int sock
#elif _WIN32
#include <winsock2.h>
#define SOCK unsigned int sock
#endif
#include "include/main.h"

extern int game_running;
// extern struct online_data cslient_data;
extern char user0[32];
extern char user1[32];
extern char user_name[32];

SOCK;
int user_id = -1;

int client_connect(char *IP_ADDRESS, int PORT)
{ // connect to the sock
#ifdef _WIN32
	WSADATA Data;
	WSAStartup(MAKEWORD(2, 2), &Data);
#endif
	struct sockaddr_in server_id;
	server_id.sin_addr.s_addr = inet_addr(IP_ADDRESS);
	server_id.sin_family = AF_INET;
	server_id.sin_port = htons(PORT);
	sock = socket(AF_INET, SOCK_STREAM, 0);
	return connect(sock, (struct sockaddr *)&server_id, sizeof(server_id));
}

void *client_comm(void *arg_data)
{ // communicating data with the server (mostly receiving)
	// initializing the game
	struct online_data *data = (struct online_data *)arg_data;
	listen(sock, 1);
	printf("cioao\n\n");
	send(sock, &user_name, sizeof(user_name), 0);
	sleep(10);
	recv(sock, &user0, sizeof(user0), 0);
	recv(sock, &user1, sizeof(user1), 0);
	recv(sock, &user_id, 4, 0);

	// communication loop
	while (game_running)
	{
		// read game data
		recv(sock, &data->is_game_over, sizeof(data->is_game_over), 0);
		recv(sock, &data->turn, sizeof(data->turn), 0);
		recv(sock, &data->winsP0, sizeof(data->winsP0), 0);
		recv(sock, &data->winsP1, sizeof(data->winsP1), 0);
		recv(sock, &data->winner, sizeof(data->winner), 0);
		for (int i = 0; i < 9; i++)
			recv(sock, &data->game_grid[i], sizeof(data->game_grid[i]), 0);

		// checking if client has permission to play and sending data
		if (data->turn == user_id)
			data->click_position = -1; // set click only if it's client's turn
		send(sock, &data->click_position, sizeof(data->click_position), 0);
		if (data->click_position >= 0 || data->is_game_over)
			data->click_position = -1;
		send(sock, &data->ready, sizeof(data->ready), 0);
		if (data->ready == 1 && data->is_game_over == 0)
			data->ready = 0;
	}
	pthread_exit(NULL);
}