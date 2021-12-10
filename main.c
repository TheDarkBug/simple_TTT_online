#include "include/main.h"
#include "include/bot.h"
#include "include/client.h"
#include "include/gameplay.h"
#include "include/gui.h"
#include "include/server.h"
#include "include/shapes.h"
#include "lib/raylib/src/raylib.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int game_running = 0;
pthread_t tid[4];

int main() {
	struct client_data data	  = (struct client_data){0};
	struct server_data server = (struct server_data){0};
	data.click_position[0]	  = -1;
	data.click_position[1]	  = -1;
	data.user_id			  = -1;
	server.PORT				  = 5555;
	data.game_mode			  = join_window(server.IP_ADDRESS, &server.PORT, &data);

	if (data.game_mode < 0)
		return 0;
	else if (data.game_mode == 1 || data.game_mode == 2) {
		pthread_create(&tid[2], 0, server_main, &server);
		usleep(100000);
		while (client_connect(server.IP_ADDRESS, server.PORT, &data.sockfd))
			;
		int ret = 0;
		pthread_join(tid[2], (void *)&ret);
		exit(ret);
	}
	if (data.game_mode == 2)
		sprintf(data.users[0], "Me");
	game_running = 1;
	pthread_create(&tid[0], 0, client_comm, &data);
	// pthread_create(&tid[1], 0, window_main, &data); // android does not like a separate thread to draw to the screen
	if (data.game_mode == 2)
		pthread_create(&tid[3], 0, bot_main, NULL);
	// pthread_join(tid[1], NULL);
	window_main(&data);
	return 0;
}

void *window_main(void *arg) {
	struct client_data *data = (struct client_data *)arg;
#ifdef ANDROID
#else
	SetTraceLogLevel(LOG_NONE);
	SetConfigFlags(FLAG_MSAA_4X_HINT);
	InitWindow(SCR_WIDTH, SCR_HEIGHT, TextFormat("Simple TTT - %s", data->users[0]));
	SetTargetFPS(GetMonitorRefreshRate(0));
#endif

	initHitBox();
	// main game loop
	while (!WindowShouldClose() && game_running) {
		place(data);
		BeginDrawing();
		ClearBackground(RAYWHITE);
		grid();
		for (int i = 0; i < 3; i++)
			for (int j = 0; j < 3; j++)
				shape((int[2]){i, j}, &data->game_grid[i][j]);
		if (data->is_game_over == 1)
			end_client_game(data);
		matchInfo(data);
		EndDrawing();
	}
	game_running = 0;
	// end of the program
	CloseWindow();
	// close(data->sockfd);
	return 0;
}
