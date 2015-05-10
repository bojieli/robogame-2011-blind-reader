#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/socket.h>
#include<fcntl.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<signal.h>
#define TRUE 1
#define FALSE 0

#define MONITOR_FILE "./monitor"
#define PROGRAM_MAXNUM 100
#define SCREEN_NUM 16
#define DAEMON_PORT 40000
#define MONITOR_PORT 40001
#define CONTROL_MAXNUM 1000
#define PIXEL_SIZE 20

typedef struct {
	int screen;
	int x;
	int y;
	int width;
	int height;
	int data_length;
	char *data;
} control;
typedef struct {
	pid_t pid;
	int sock;
	int curr_screen;
	int ctrl_num;
	control ctrl[CONTROL_MAXNUM];
} program;
program prog[PROGRAM_MAXNUM];
int program_num = 0;

int curr_program;

struct sockaddr_in daemon_addr, monitor_addr, client_addr;
int daemon_fd, sockfd; // sockfd is monitor_fd
fd_set readfds;
struct timeval tv;
int max_socket = 0;
socklen_t sin_size;

FILE *monitor_file;

void init_addr(struct sockaddr_in *addr, int listen_port) {
	char sin_addr[4] = {127, 0, 0, 1}; // 127.0.0.1
	addr->sin_family = AF_INET;
	addr->sin_port = htons(listen_port);
	memcpy(&addr->sin_addr, sin_addr, 4);
	memset(&addr->sin_zero, 0, 8);
}

int connect_daemon() {
	init_addr(&daemon_addr, DAEMON_PORT);
	daemon_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (daemon_fd == -1) {
		perror("Error Connecting to Daemon");
		exit(1);
	}
	fcntl(daemon_fd, F_GETFL, O_NONBLOCK);
	if (connect(daemon_fd, (struct sockaddr *)&daemon_addr, sizeof(daemon_addr)) == -1)
		return FALSE;
	FD_SET(daemon_fd, &readfds);
	return TRUE;
}

void init_fd() {
	int yes = 1; // no actual use
	
	init_addr(&monitor_addr, MONITOR_PORT);

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("Error Creating Socket");
		exit(1);
	}

	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
		perror("Error Setting Sock Options");
		exit(1);
	}

	if (bind(sockfd, (struct sockaddr *) &monitor_addr, sizeof(monitor_addr)) == -1) {
		perror("Error Binding Port");
		exit(1);
	}

	if (listen(sockfd, PROGRAM_MAXNUM) == -1) {
		perror("Error Listening Port");
		exit(1);
	}

	FD_ZERO(&readfds);
	tv.tv_sec = 600; // wait for 10 minutes and automatically exit
	tv.tv_usec = 0;
}

void recv_prog(int prog_no) {
	// (screen, x, y, width, height, data_length, data)[num]
	int sock = prog[prog_no].sock;
	int i;
	recv(sock, (char *)&prog[prog_no].ctrl_num, sizeof(int), 0);
	for (i=0; i<prog[prog_no].ctrl_num; i++) {
		// receive screen, x, y, width, height, data_length in byte order
		recv(sock, (char *)&prog[prog_no].ctrl[i], sizeof(int) * 6, 0);
		char *buf = malloc(prog[prog_no].ctrl[i].data_length);
		// receive data in this controller
		recv(sock, buf, prog[prog_no].ctrl[i].data_length, 0);
		prog[prog_no].ctrl[i].data = buf;
	}
}

void update_monitor() {
	int i, flag = FALSE;
	monitor_file = fopen(MONITOR_FILE, "wb+");
	fprintf(monitor_file, "<style>div { position:absolute; font-size:%dpx; border:1px solid }</style>", PIXEL_SIZE);
	for (i=0; i<prog[curr_program].ctrl_num; i++) {
		if (prog[curr_program].curr_screen == prog[curr_program].ctrl[i].screen) {
			control *c = &prog[curr_program].ctrl[i];
			fprintf(monitor_file, "<div id=\"%d\" style=\"left: %dpx; top:%dpx; width:%dpx; height:%dpx;\">", i, c->x * PIXEL_SIZE, c->y * PIXEL_SIZE, c->width * PIXEL_SIZE, c->height * PIXEL_SIZE);
			fprintf(monitor_file, "%s", c->data);
			fprintf(monitor_file, "</div>\n");
			flag = TRUE;
		}
	}
	if (!flag)
		fprintf(monitor_file, "<p>BLANK SCREEN");
	fclose(monitor_file); // free write lock
}

int main() {
	init_fd();
	connect_daemon();

	while (TRUE) {
		int ret, i;
		struct timeval tmp_tv = tv;

		// select a new client
		ret = select(max_socket + 1, &readfds, NULL, NULL, &tmp_tv);
		if (ret <= 0) {
			exit(1);
		}

		// poll sockets
		for (i=0; i<program_num; i++)
			if (prog[i].pid != 0 && FD_ISSET(prog[i].sock, &readfds)) {
				recv_prog(i);
				if (curr_program == i)
					update_monitor();
			}

		// accept new connection
		if (FD_ISSET(sockfd, &readfds)) {
			int new_fd;
			pid_t pid;
			new_fd = accept(sockfd, (struct sockaddr *)&client_addr, &sin_size);
			if (new_fd <= 0)
				continue;
			fcntl(new_fd, F_GETFL, O_NONBLOCK);
			if (program_num >= PROGRAM_MAXNUM)
				exit(1);

			recv(new_fd, &pid, sizeof(pid_t), 0);
			if (pid <= 0) {
				close(new_fd);
				continue;
			}
			
			prog[program_num].sock = new_fd;
			prog[program_num].pid = pid;
			prog[program_num].curr_screen = 0;
			prog[program_num].ctrl_num = 0;
			program_num++;

			if (new_fd > max_socket)
				max_socket = new_fd;
		}

		// handle daemon messages
		if (FD_ISSET(daemon_fd, &readfds)) {
			pid_t pid;
			int curr_screen;
			recv(daemon_fd, (char *)&pid, sizeof(pid_t), 0);
			recv(daemon_fd, (char *)&curr_screen, sizeof(int), 0);
			if (curr_screen < 0 || curr_screen >= 16) {
				printf("Internal Error: pid %d, screen %d\n", pid, curr_screen);
				continue;
			}
			for (i=0; i<program_num; i++)
				if (prog[i].pid == pid) {
					prog[i].curr_screen = curr_screen;
					update_monitor();
					break;
				}
		}
	}
}
