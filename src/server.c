#include <stdio.h>
#include <unistd.h>
#if defined(__linux__) || defined(__EMSCRIPTEN__)
	#include <netinet/in.h>
	#include <sys/socket.h>
	#include <netdb.h>
	#include <netinet/tcp.h>
#elif _WIN32
	#include <winsock2.h>
	#include <ws2tcpip.h>
#endif
#include "include/gameplay.h"
#include "include/main.h"
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

extern int game_running;

void *communication(void *arg) { // communicating servdata->data with the client (mostly sending)
#ifndef __EMSCRIPTEN__
	struct server_data *servdata = (struct server_data *)arg;
	int client_id				 = servdata->client_count;
	servdata->client_running	 = 1;
	int recv_flags				 = 0;
	int send_flags				 = 0;
	while (game_running) {
		servdata->data.winner = checkwinner(&servdata->data);

		// send servdata->data to client
		send(servdata->clifd[client_id], &servdata->data.users, sizeof(servdata->data.users), send_flags);
		send(servdata->clifd[client_id], &servdata->data.is_game_over, sizeof(servdata->data.is_game_over), send_flags);
		send(servdata->clifd[client_id], &servdata->data.turn, sizeof(servdata->data.turn), send_flags);
		send(servdata->clifd[client_id], &servdata->data.winsP, sizeof(servdata->data.winsP), send_flags);
		send(servdata->clifd[client_id], &servdata->data.winner, sizeof(servdata->data.winner), send_flags);
		send(servdata->clifd[client_id], &servdata->data.game_grid, sizeof(servdata->data.game_grid), send_flags);

		// recv client events
		recv(servdata->clifd[client_id], &servdata->data.click_position, sizeof(servdata->data.click_position), recv_flags);
		recv(servdata->clifd[client_id], &servdata->ready_check[client_id], sizeof(servdata->ready_check[client_id]), recv_flags);

		// check if client has permission to play
		if (servdata->data.turn == client_id) {
			servdata->data.click_position[0] = -1;
			servdata->data.click_position[1] = -1;
		}
		// accepting click_position only from player's turn client
		if (servdata->data.click_position[0] != -1 &&
			servdata->data.click_position[1] != -1 &&
			servdata->data.is_game_over == 0)																	   // handling click_positions
			if (servdata->data.game_grid[servdata->data.click_position[0]][servdata->data.click_position[1]] == 0) // if positions exist, then check for the position being free
				servdata->data.game_grid[servdata->data.click_position[0]][servdata->data.click_position[1]] = (servdata->data.turn = !servdata->data.turn) + 1;
		if (servdata->ready_check[0] && servdata->ready_check[1])
			end_server_game(servdata->data.winner, &servdata->data); // checks if all clients are recvy to continue
	}
	close(servdata->clifd[client_id]);
#endif
	return NULL;
}

void *server_main(void *arg) {
#ifndef __EMSCRIPTEN__
	struct server_data *servdata = (struct server_data *)arg;
	pthread_t tid[128];

	// creating socket and connecting to it
	#ifdef _WIN32
	WSADATA Data;
	WSAStartup(MAKEWORD(2, 2), &Data);
	#endif
	struct sockaddr_in address;
	int flags = 1, addrlen = sizeof(address);
	SOCK servfd = socket(AF_INET, SOCK_STREAM, 6); // 6 = tcp protocol
	setsockopt(servfd, SOL_SOCKET, SO_REUSEADDR, &flags, sizeof(flags));
	// setsockopt(servfd, IPPROTO_TCP, TCP_NODELAY, &flags, sizeof(flags));
	address.sin_family		= AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port		= htons(servdata->PORT);
	if (bind(servfd, (struct sockaddr *)&address, sizeof(address)) < 0)
		return NULL;
	listen(servfd, 10);

	// accepting clients and creating and joining threads
	servdata->client_count = 0;
	while (servdata->client_count < 2) {
		servdata->clifd[servdata->client_count] = accept(servfd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
		recv(servdata->clifd[servdata->client_count], &servdata->data.users[servdata->client_count], sizeof(servdata->data.users[servdata->client_count]), 0);
		send(servdata->clifd[servdata->client_count], &servdata->client_count, sizeof(servdata->client_count), 0);
		servdata->client_running = 0;
		pthread_create(&tid[servdata->client_count], 0, communication, servdata);
		while (servdata->client_running == 0)
			;
		servdata->client_count++;
	}
	for (int i = 0; i <= 1; i++)
		pthread_join(tid[i], NULL);
#endif
	return 0;
}
