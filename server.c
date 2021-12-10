#include <stdio.h>
#include <unistd.h>
#ifdef __linux__
	#include <netinet/in.h>
	#include <sys/socket.h>
#elif _WIN32
	#include <winsock2.h>
	#include <ws2tcpip.h>
#endif
#include "include/gameplay.h"
#include "include/main.h"
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include <sys/types.h>
#define MAX_CLI 4

int ready_check[MAX_CLI];
extern int game_running;

void *communication(void *);

void *server_main(void *arg) {
	struct server_data *servdata = (struct server_data *)arg;
	pthread_t t[MAX_CLI];
	SOCK listenfd;
	int opt = 1;
	struct sockaddr_in servaddr;
	servaddr.sin_family		 = AF_INET;
	servaddr.sin_addr.s_addr = INADDR_ANY;
	servaddr.sin_port		 = htons(servdata->PORT);
	memset(servaddr.sin_zero, 0, sizeof(servaddr.sin_zero));

	if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		LOGE("Socket creation failed!");
		return (void *)-1;
	}
	setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const char *)&opt, sizeof(opt));
	if (bind(listenfd, (const struct sockaddr *)&servaddr, sizeof(struct sockaddr_in)) < 0) {
		LOGE("Binding failed!");
		return (void *)-1;
	}
	if (listen(listenfd, 10) < 0) {
		LOGE("Error while calling listen()!");
		return (void *)-1;
	}
	for (int i = 0; i < 2 /* MAX_CLI */; i++) {
		servdata->clifd[i]	= accept(listenfd, NULL, NULL);
		servdata->thread_id = i;
		if (servdata->clifd[i] < 0) {
			LOGE("Failed to accept client %i!", i);
			return (void *)-1;
		}
		pthread_create(&t[i], 0, communication, servdata);
	}
	pthread_join(t[0], NULL);
	close(listenfd);
	return 0;

	/* struct server_data *server_data = (struct server_data *)arg;
	server_data->server_tid			= pthread_self();
	// creating socket and connecting to it
	#ifdef _WIN32
	WSADATA Data;
	WSAStartup(MAKEWORD(2, 2), &Data);
	#elif defined(__linux__) || defined(ANDROID)
	struct sockaddr_in address;
	struct sockaddr client_addr[4];
	int server_fd, opt = 1, addrlen = sizeof(address);
	#endif
	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if ((server_fd < 0))
		return NULL;

		usleep(1000);
	// setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (const char *)&opt, sizeof(opt));
	address.sin_family		= AF_INET;
	address.sin_addr.s_addr = htonl(INADDR_ANY);
	address.sin_port		= htons(server_data->PORT);
	if (bind(server_fd, (const struct sockaddr *)&address, sizeof(struct sockaddr_in))) {
		LOGE("Error binding to socket");
		exit((void*)-1);
	}

	srand(time(NULL));
	client_count = rand() % 2;
	listen(server_fd, 10);

	// accepting clients
	clientfd[0] = accept(server_fd, NULL, NULL); // also this fails on android because the port is not 5555
	if (clientfd[0] < 0) {
		LOGE("Error in accepting client 1");
		exit((void*)-1);
	}
	LOGI("Client %i connected", clientfd[0]);
	read(clientfd[0], (char *)&server_data->data.users[1], sizeof(server_data->data.users[1]));

	clientfd[1] = accept(server_fd, NULL, NULL);
	if (clientfd[1] < 0) {
		LOGE("Error in accepting client 2");
		exit((void*)-1);
	}
	read(clientfd[1], (char *)&server_data->data.users[2], sizeof(server_data->data.users[2]));
	LOGI("Client %i connected", clientfd[1]);

	// exit(0);

	// initializing connection
	write(clientfd[0], (char *)&server_data->data.users, sizeof(server_data->data.users));
	write(clientfd[0], (char *)&client_count, sizeof(client_count));
	client_count = !client_count;
	write(clientfd[1], (char *)&server_data->data.users, sizeof(server_data->data.users));
	write(clientfd[1], (char *)&client_count, sizeof(client_count));

	// creating and joining threads
	while (server_data->thread_id <= 1) {
		server_data->client_running = 0;
		pthread_create(&server_tid[server_data->thread_id], 0, communication, server_data);
		while (server_data->client_running == 0)
			;
		server_data->thread_id++;
	}
	for (int i = 0; i <= 1; i++)
		pthread_join(server_tid[i], NULL);
	return 0; */
}

void *communication(void *arg) {
	struct server_data *servdata = (struct server_data *)arg;
	int clid					 = servdata->thread_id;
	servdata->client_running	 = 1;
	char buf[1000]				 = "";
	if (send(servdata->clifd[clid], "Ciao!", strlen("Ciao!"), 0) < 0) {
		LOGE("Error sending to client %i, disconnected!", clid);
		return (void *)-1;
	}
	if (recv(servdata->clifd[clid], buf, sizeof(buf), 0) < 0) {
		LOGE("Error receiving from client %i, disconnected!", clid);
		return (void *)-1;
	}
	if (strcmp(buf, "yes") != 0) {
		LOGE("Connection with the client failed!");
		return (void *)-1;
	}
	close(servdata->clifd[clid]);
	return 0;
	// communicating server_data->data with the client (mostly sending)
	/* struct server_data *server_data = (struct server_data *)arg;
	int client_id					= server_data->thread_id;
	server_data->client_running		= 1;
	while (game_running) {
		// server_data->data.turn = !server_data->data.turn;
		server_data->data.winner = checkwinner(&server_data->data);

		// write server_data->data to client
		write(clientfd[client_id], (const char *)&server_data->data.is_game_over, sizeof(server_data->data.is_game_over));
		write(clientfd[client_id], (const char *)&server_data->data.turn, sizeof(server_data->data.turn));
		write(clientfd[client_id], (const char *)&server_data->data.winsP, sizeof(server_data->data.winsP));
		write(clientfd[client_id], (const char *)&server_data->data.winner, sizeof(server_data->data.winner));
		write(clientfd[client_id], (const char *)&server_data->data.game_grid, sizeof(server_data->data.game_grid));

		// read client events
		read(clientfd[client_id], (char *)&server_data->data.click_position, sizeof(server_data->data.click_position));
		read(clientfd[client_id], (char *)&ready_check[client_id], sizeof(ready_check));

		// check if client has permission to play
		if (server_data->data.turn == client_id) {
			server_data->data.click_position[0] = (void*)-1;
			server_data->data.click_position[1] = (void*)-1;
		}																																																												   // accepting click_position only from player's turn client
		if (server_data->data.click_position[0] != (void*)-1 && server_data->data.click_position[1] != (void*)-1 && server_data->data.game_grid[server_data->data.click_position[0]][server_data->data.click_position[1]] == 0 && server_data->data.is_game_over == 0) { // handling click_positions
			if (server_data->data.turn)
				server_data->data.game_grid[server_data->data.click_position[0]][server_data->data.click_position[1]] = 1;
			else
				server_data->data.game_grid[server_data->data.click_position[0]][server_data->data.click_position[1]] = 2;
			server_data->data.turn = !server_data->data.turn;
		}
		if (ready_check[0] && ready_check[1])
			end_server_game(server_data->data.winner, &server_data->data); // checks if all clients are ready to continue
	}
	close(clientfd[client_id]);
	pthread_exit(NULL); */
}