#include "include/main.h"
#include "include/bot.h"
#include "include/client.h"
#include "include/gameplay.h"
#include "include/gui.h"
#include "include/server.h"
#include "include/shapes.h"
#include "lib/raylib/include/raylib.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int block		 = SCR_WIDTH / 3;
int game_running = 0;
Rectangle game[9];
pthread_t tid[4];

int main() {
	struct client_data data	  = (struct client_data){0};
	struct server_data server = (struct server_data){0};
	data.click_position		  = -1;
	data.user_id			  = -1;
	server.PORT				  = 5555;
	data.game_mode			  = join_window(server.IP_ADDRESS, &server.PORT, &data);
	if (data.game_mode < 0)
		return 0;
	else if (data.game_mode == 1 || data.game_mode == 2) {
		pthread_create(&tid[2], 0, server_main, &server);
		while (client_connect(server.IP_ADDRESS, server.PORT, &data.sock))
			;
	}
	if (data.game_mode == 2)
		sprintf(data.users[0], "Me");
	pthread_create(&tid[0], 0, client_comm, &data);
	pthread_create(&tid[1], 0, window_main, &data);
	if (data.game_mode == 2)
		pthread_create(&tid[3], 0, bot_main, NULL);
	for (int i = 0; i < 3; i++)
		pthread_join(tid[i], NULL);
	return 0;
}

void *window_main(void *arg) {
	struct client_data *data = (struct client_data *)arg;
	SetTraceLogLevel(LOG_NONE);
	SetConfigFlags(FLAG_MSAA_4X_HINT);
	InitWindow(SCR_WIDTH, SCR_HEIGHT, TextFormat("Simple TTT - %s", data->users[0]));
	SetTargetFPS(GetMonitorRefreshRate(0));
	initHitBox();
	// main game loop
	while (!WindowShouldClose() && game_running) {
		place(data);
		BeginDrawing();
		ClearBackground(RAYWHITE);
		grid();
		for (int i = 0; i < 9; i++)
			shape(game, &i, &data->game_grid[i]);
		if (data->is_game_over == 1)
			end_client_game(data);
		matchInfo(data);
		// DrawFPS(10, 10);
		EndDrawing();
	}
	game_running = 0;
	// end of the program
	CloseWindow();
	close(data->sock);
	exit(0);
}
