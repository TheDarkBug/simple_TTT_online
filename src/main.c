
#include "include/main.h"
#include "../lib/raylib/src/raylib.h"
#include "include/bot.h"
#include "include/client.h"
#include "include/gameplay.h"
#include "include/gui.h"
#include "include/server.h"
#include "include/shapes.h"
#include <pthread.h>
#if defined(__linux__) || defined(__EMSCRIPTEN__)
	#include <arpa/inet.h>
	#include <netdb.h>
#elif _WIN32
// #include <winsock2.h>
#endif
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef ANDROID
	#include <jni.h>
struct android_app *app;
#endif

int game_running = 0;

char *get_ip_addr(int id) {
	char *ip = NULL;
	if (id == 0) { // local ip
#ifndef ANDROID
	#ifndef _WIN32
		char hostbuffer[256];
		struct hostent *host_entry;
		int hostname = gethostname(hostbuffer, sizeof(hostbuffer));
		if (hostname != -1) {
			host_entry = gethostbyname(hostbuffer);
			if (host_entry)
				ip = inet_ntoa(*((struct in_addr *)host_entry->h_addr_list[0]));
		}
	#endif
#else
		FILE *ip_fp = popen("ifconfig $(getprop wifi.interface) | grep -Eo 'addr:[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}' | sed \"s/addr://g\"", "r");
		if (ip_fp == NULL) {
			LOGE("Failed to get external ip");
			return NULL;
		}
		ip = calloc(1, 48);
		fgets(ip, 48, ip_fp);
		pclose(ip_fp);
		// for some reason, on android a \n gets added
		char *ipp = strchr(ip, '\n');
		while (ipp) {
			*ipp = ' ';
			ipp	 = strchr(ipp, '\n');
		};
#endif
	} else { // external ip
		FILE *ip_fp = popen("curl -s ipinfo.io/ip", "r");
		if (ip_fp == NULL) {
			LOGE("Failed to get external ip");
			return NULL;
		}
		ip = calloc(1, 48);
		fgets(ip, 48, ip_fp);
		pclose(ip_fp);
	}
	return ip;
}

void init_game(struct client_data *data, struct server_data *server, struct nk_context *ctx) {
#ifdef ANDROID
	SCR_WIDTH  = GetScreenWidth();
	SCR_HEIGHT = GetScreenHeight();
#endif
	// resetting game data
	data->click_position[0] = -1;
	data->click_position[1] = -1;
	data->uid				= -1;
	data->local_ip			= calloc(1, 128);
	server->PORT			= 5555;
	data->game_mode			= join_window(server->IP_ADDRESS, &server->PORT, data, ctx);
	/*
	 * 0 = single player
	 * 1 = join online
	 * 2 = host online
	 */
	if (data->game_mode < 0) {
		CloseWindow();
		UnloadNuklear(ctx);
		exit(0);
	}
#ifdef ANDROID
	toggleKeyboard(false);
#endif
	pthread_t tid[4];
	// if (data->game_mode == 0 || data->game_mode == 2)
	pthread_create(&tid[2], 0, server_main, server);
	if (data->game_mode == 0)
		sprintf(data->username, "Me");
	while (client_connect(server->IP_ADDRESS, server->PORT, &data->sockfd))
		LOGE("%s:%d", server->IP_ADDRESS, server->PORT);
	pthread_create(&tid[0], 0, client_comm, data);
	if (data->game_mode == 0)
		pthread_create(&tid[3], 0, bot_main, NULL);
	else {
		// get addresses
		char *local_ip	  = get_ip_addr(0);
		char *external_ip = get_ip_addr(1);
		if (local_ip != NULL) {
			sprintf(data->local_ip, "IP: %s", local_ip);
			if (external_ip)
				sprintf(data->local_ip, "%s / %s", data->local_ip, external_ip);
		} else {
			if (external_ip)
				sprintf(data->local_ip, "IP: %s", external_ip);
		}
		free(external_ip);
#ifdef ANDROID
		free(local_ip);
#endif
	}
}

void main_window(struct client_data *data, struct nk_context *ctx) {
	// main window
	initHitBox();
	SetWindowTitle(TextFormat("Simple TTT - %s", data->username));
	while (!WindowShouldClose() && game_running) {
		UpdateNuklear(ctx);
		if (data->is_game_over == 1)
			end_client_game(data, ctx);
		place(data);
		BeginDrawing();
		ClearBackground(BG_COLOR);
		grid();
		for (int i = 0; i < 3; i++)
			for (int j = 0; j < 3; j++)
				shape((int[2]){i, j}, data->game_grid[i][j]);
		matchInfo(data);
		DrawNuklear(ctx);
		EndDrawing();
	}
}

void sigpipe_handler(int signum) {
	LOGE("Client disconnected: %d", signum);
	exit(1);
}

int main() {
#ifndef _WIN32
	signal(SIGPIPE, sigpipe_handler);
#endif
#ifdef ANDROID
	app = GetAndroidApp();
#endif
	struct nk_context *ctx	   = NULL;
	struct client_data *data   = calloc(1, sizeof(struct client_data));
	struct server_data *server = calloc(1, sizeof(struct server_data));
	SetTraceLogLevel(LOG_NONE);
	ctx = InitNuklear(STTT_TEXT_SIZE);
	SetConfigFlags(FLAG_MSAA_4X_HINT);
	InitWindow(SCR_WIDTH, SCR_HEIGHT, "Game Mode Selection");
	SetTargetFPS(GetMonitorRefreshRate(0));
	init_game(data, server, ctx);
	main_window(data, ctx);
	// end of the program
	game_running = 0;
	CloseWindow();
	UnloadNuklear(ctx);
	close(data->sockfd);
	return 0;
}
