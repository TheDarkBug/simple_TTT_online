#include <stdio.h>
#include <string.h>
#include <unistd.h>
#ifdef __linux__
	#include <sys/socket.h>
	#include <arpa/inet.h>
#elif _WIN32
	#include <winsock2.h>
#endif

#define IP_ADDRESS	"25.64.37.243"
#define PORT		5555

extern int click_position;
extern int game_grid[9];
extern int is_game_over;
extern int turn;
extern int winsP1;
extern int winsP2;
extern int winner;
#ifdef __linux__
int sock;			//-6108638100957931777 1.16.5 x:-16 y:73 z:76
#elif _WIN32
unsigned int sock;
#endif
extern int ready;

int client_connect() {
	#ifdef _WIN32
		WSADATA Data;
		WSAStartup(MAKEWORD(2, 2), &Data);
	#endif
	struct sockaddr_in server_id;
	if (sock == -1)	return 1;
	server_id.sin_addr.s_addr = inet_addr(IP_ADDRESS);
	server_id.sin_family = AF_INET;
	server_id.sin_port = htons(PORT);
	sock = socket(AF_INET, SOCK_STREAM, 0);

	//Connect to remote server_id
	if (connect(sock , (struct sockaddr *) &server_id , sizeof(server_id)) < 0) return 1;
	return 0;
}

void client_comm() {
	listen(sock, 1);
	// read game data
	recv(sock, (char *) &is_game_over, 4, 0);
	recv(sock, (char *) &turn, 4, 0);
	recv(sock, (char *) &winsP1, 4, 0);
	recv(sock, (char *) &winsP2, 4, 0);
	recv(sock, (char *) &winner, 4, 0);
	for (int i = 0; i < 9; i++) {
		recv(sock, (char *) &game_grid[i], 4, 0);
	}

	// write events
	send(sock, (char *) &click_position, 4, 0);
	send(sock, (char *) &ready, 4, 0);
	if (click_position != -1 || is_game_over == 1) {
		click_position = -1;
	}
	if (ready == 1 && is_game_over == 0) {
		ready = 0;
	}
}