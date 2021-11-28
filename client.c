#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#ifdef __linux__
	#include <arpa/inet.h>
	#include <sys/socket.h>
#elif _WIN32
	#include <winsock2.h>
#endif
#include "include/main.h"

extern int game_running;
// SOCK;
// int user_id = -1;

int client_connect(char *IP_ADDRESS, int PORT, SOCK *sock) { // connect to the sock
#ifdef _WIN32
	WSADATA Data;
	WSAStartup(MAKEWORD(2, 2), &Data);
#endif
	struct sockaddr_in server_id;
	server_id.sin_addr.s_addr = inet_addr(IP_ADDRESS);
	server_id.sin_family	  = AF_INET;
	server_id.sin_port		  = htons(PORT);
	*sock					  = socket(AF_INET, SOCK_STREAM, 0);
	return connect(*sock, (struct sockaddr *)&server_id, sizeof(server_id));
}

void *client_comm(void *arg) { // communicating data with the server (mostly receiving)
	// initializing the game
	struct client_data *data = (struct client_data *)arg;
	listen(data->sock, 1);
	write(data->sock, (char *)&data->users[0], sizeof(data->users[0]));
	read(data->sock, (char *)&data->users, sizeof(data->users));
	read(data->sock, (char *)&data->user_id, sizeof(data->user_id));

	// communication loop
	while (game_running) {
		// read game data
		read(data->sock, (char *)&data->is_game_over, sizeof(data->is_game_over));
		read(data->sock, (char *)&data->turn, sizeof(data->turn));
		read(data->sock, (char *)&data->winsP, sizeof(data->winsP));
		read(data->sock, (char *)&data->winner, sizeof(data->winner));
		read(data->sock, (char *)&data->game_grid, sizeof(data->game_grid));

		// checking if client has permission to play and sending data
		if (data->turn == data->user_id)
			data->click_position = -1; // set click only if it's client's turn
		write(data->sock, (char *)&data->click_position, sizeof(data->click_position));
		if (data->click_position >= 0 || data->is_game_over)
			data->click_position = -1;
		write(data->sock, (char *)&data->ready, sizeof(data->ready));
		if (data->ready == 1 && data->is_game_over == 0)
			data->ready = 0;
	}
	pthread_exit(NULL);
}