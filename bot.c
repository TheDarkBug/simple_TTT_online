#include "include/bot.h"
#include "include/client.h"
#include "include/main.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

extern int game_running;

void *bot_main() {
	struct client_data data = (struct client_data){0};
	pthread_t tid[3];
	data.click_position[0] = -1;
	data.click_position[1] = -1;
	data.user_id		   = -1;
	sprintf(data.users[0], "CPU");
	while (client_connect("127.0.0.1", 5555, &data.sock))
		;
	pthread_create(&tid[0], 0, client_comm, &data);
	pthread_create(&tid[1], 0, bot_ai, &data);
	pthread_join(tid[0], NULL);
	pthread_join(tid[1], NULL);
	return NULL;
}

void bot_easy(struct client_data *data) {
	for (int i = 0; i < 9 && data->click_position[0] < 0 && data->click_position[0] < 0; i++) {
		int rand_num = rand() % 8;
		if (data->game_grid[rand_num / 3][rand_num % 3] == 0) {
			data->click_position[0] = rand_num / 3;
			data->click_position[1] = rand_num % 3;
		}
	}
}
void bot_medium(struct client_data *data) {
	bot_easy(data);
	for (int i = 0; i < 3; i++) {
		// rows
		if (data->game_grid[i][0] == 2) {
			if (data->game_grid[i][1] == 0 && data->game_grid[i][2] == 0) {
				data->click_position[0] = i;
				data->click_position[1] = 1;
				return;
			} else if (data->game_grid[i][1] == 2 && data->game_grid[i][2] == 0) {
				data->click_position[0] = i;
				data->click_position[1] = 2;
				return;
			} else if (data->game_grid[i][1] == 0 && data->game_grid[i][2] == 2) {
				data->click_position[0] = i;
				data->click_position[1] = 1;
				return;
			}
		}
		if (data->game_grid[i][1] == 2) {
			if (data->game_grid[i][0] == 0 && data->game_grid[i][2] == 0) {
				data->click_position[0] = i;
				data->click_position[1] = 1;
				return;
			} else if (data->game_grid[i][0] == 2 && data->game_grid[i][2] == 0) {
				data->click_position[0] = i;
				data->click_position[1] = 1;
				return;
			} else if (data->game_grid[i][0] == 0 && data->game_grid[i][2] == 2) {
				data->click_position[0] = i;
				data->click_position[1] = 0;
				return;
			}
		}
		if (data->game_grid[i][2] == 2) {
			if (data->game_grid[i][0] == 0 && data->game_grid[i][1] == 0) {
				data->click_position[0] = i;
				data->click_position[1] = 0;
				return;
			} else if (data->game_grid[i][0] == 2 && data->game_grid[i][1] == 0) {
				data->click_position[0] = i;
				data->click_position[1] = 1;
				return;
			} else if (data->game_grid[i][0] == 0 && data->game_grid[i][1] == 2) {
				data->click_position[0] = i;
				data->click_position[1] = 0;
				return;
			}
		}

		//columns
		if (data->game_grid[0][i] == 2) {
			if (data->game_grid[1][i] == 0 && data->game_grid[2][i] == 0) {
				data->click_position[0] = 1;
				data->click_position[1] = i;
				return;
			} else if (data->game_grid[1][i] == 2 && data->game_grid[2][i] == 0) {
				data->click_position[0] = 2;
				data->click_position[1] = i;
				return;
			} else if (data->game_grid[1][i] == 0 && data->game_grid[2][i] == 2) {
				data->click_position[0] = 1;
				data->click_position[1] = i;
				return;
			}
		}
		if (data->game_grid[1][i] == 2) {
			if (data->game_grid[0][i] == 0 && data->game_grid[2][i] == 0) {
				data->click_position[0] = 0;
				data->click_position[1] = i;
				return;
			} else if (data->game_grid[0][i] == 2 && data->game_grid[2][i] == 0) {
				data->click_position[0] = 2;
				data->click_position[1] = i;
				return;
			} else if (data->game_grid[0][i] == 0 && data->game_grid[2][i] == 2) {
				data->click_position[0] = 0;
				data->click_position[1] = i;
				return;
			}
		}
		if (data->game_grid[2][i] == 2) {
			if (data->game_grid[0][i] == 0 && data->game_grid[1][i] == 0) {
				data->click_position[0] = 0;
				data->click_position[1] = i;
				return;
			} else if (data->game_grid[0][i] == 2 && data->game_grid[1][i] == 0) {
				data->click_position[0] = 1;
				data->click_position[1] = i;
				return;
			} else if (data->game_grid[0][i] == 0 && data->game_grid[1][i] == 2) {
				data->click_position[0] = 0;
				data->click_position[1] = i;
				return;
			}
		}
	}
}
void bot_hard(struct client_data *data) { bot_medium(data); }
void bot_impossible(struct client_data *data) { bot_hard(data); }

void *bot_ai(void *arg) {
	struct client_data *data = (struct client_data *)arg;
	srand(time(NULL));
	while (game_running) {
		if (!data->is_game_over && !data->turn) {
			usleep(rand() % 100000);
			do {
				switch (data->bot_hardness) {
				case 1:
					bot_medium(data);
					break;

				case 2:
					bot_hard(data);
					break;

				case 3:
					bot_impossible(data);
					break;

				default:
					bot_easy(data);
					break;
				}
				if (data->is_game_over)
					break;
			} while (data->game_grid[data->click_position[0]][data->click_position[1]] != 0);
		}
		data->ready = (data->is_game_over == 1);
	}
	game_running = 0;
	return NULL;
}