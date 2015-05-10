#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
//#include<sys/socket.h>
//#include<fcntl.h>
#include<netinet/in.h>
#include<arpa/inet.h>
//#include<curses.h>
#include "transmission.h"
#define TRUE 1
#define FALSE 0

#define DEVICE_FILE "/dev/blindbf"
#define ID_MAXLEN 256
#define SCREEN_NUM 16
#define EVENT_MAXNUM 256
#define SCREEN_WIDTH 20
#define SCREEN_HEIGHT 15
#define PROGRAM_MAXNUM 100
#define LISTEN_PORT 2011
#define BUF_SIZE 65536
#define LOCAL_ESCAPE_KEY 27

/* iostreams:
	standard input: keyboard (F1~F12 as control keys, chars as input to program)
	standard output: program message
	device file: current status of screen
	socket: input from library
*/

/* control keys definition
	F1~F5: click on lines 1~5
	Enter: OK to current message
	Arrow keys: move the window left, right, up and down
	PageUp, PageDown, Home, End: as it suggests
	Alt+Arrow: Alter screens within the same program
	Alt+Tab: Alter program windows
	Esc: Return to main menu
	Ctrl+C: Close program (SIGINT)
	Ctrl+K: Kill program (SIGKILL)
*/
/*
typedef struct screen {
	int width;
	int height;
	int posx;
	int posy;
	char *data;
} screen;

typedef struct focus_area {
	int screen;
	int x;
	int y;
	int width;
	int height;
	int id_length;
	char* id;
} focus_area;

typedef struct program {
	int pid;
	int curr_screen;
	screen scr[SCREEN_NUM];
	int focus_num;
	focus_area f[EVENT_MAXNUM];
} program;

program prog[PROGRAM_MAXNUM];
int curr_program = 0;
int fd[PROGRAM_MAXNUM];
int program_num = 0;

int sockfd;
struct sockaddr_in server_addr;
struct sockaddr_in client_addr;
socklen_t sin_size;
int max_socket = 0;

fd_set readfds;
struct timeval tv;
char buf[BUF_SIZE] = {0};
char curr_device[SCREEN_WIDTH * SCREEN_HEIGHT] = {0};

int send_key(int ch) {
	// type = KeyPressed, keycode, id_length = 0
	int buf[3] = {2, ch, 0};
	if (fd[curr_program]) {
		send(fd[curr_program], buf, 3*4, 0);
		return TRUE;
	}
	return FALSE; // fd not found
}

int write_device(char *data) {
	FILE *fp = fopen(DEVICE_FILE, "wb+");
	if (!fp) {
		return FALSE;
	}
	fwrite(data, sizeof(char), SCREEN_HEIGHT * SCREEN_WIDTH, fp);
	fclose(fp);
	return TRUE;
}

void update_device() {
	int i, j, flag = TRUE;
	char new_device[SCREEN_WIDTH * SCREEN_HEIGHT];
	program *p = &prog[curr_program];
	screen *scr = &p->scr[p->curr_screen];
	
	for (i=0; i<(SCREEN_HEIGHT > scr->height ? scr->height : SCREEN_HEIGHT); i++)
		for (j=0; j<(SCREEN_WIDTH > scr->width ? scr->width : SCREEN_WIDTH); j++) {
			new_device[i * SCREEN_WIDTH + j] = *((char *)scr->data + (scr->posx + i) * scr->width + (scr->posy + j));
			if (new_device[i * SCREEN_WIDTH + j] != curr_device[i * scr->width + j])
				flag = FALSE;
		}
	
	if (flag) {
		memcpy(curr_device, new_device, SCREEN_WIDTH * SCREEN_HEIGHT);
		write_device(curr_device);
	}
}

char* extract_pid(char *buf, program *p) {
	memcpy(&p->pid, buf, 4); // the first 4 bytes is PID
	return buf + 4;
}

char* update_screen(char *buf, program *p) {
	int i;
	for (i=0; i<SCREEN_NUM; i++) {
		screen *s = &p->scr[i];
		
		memcpy(&s->width, buf, 4);
		buf += 4;
		memcpy(&s->height, buf, 4);
		buf += 4;

		s->data = malloc(s->width * s->height);
		memcpy(s->data, buf, s->width * s->height);
		buf += s->width * s->height;
	}
	return buf; // postion after chunk of data
}

char* update_focus(char *buf, program *p) {
	int i;
	p->focus_num = *buf;
	buf += 4; // pointer move forward 4 bytes
	for (i=0; i<p->focus_num; i++) {
		// copy screen, x, y, width, height, id_length
		memcpy(&p->f[i], buf, 6 * 4);
		buf += 6 * 4;
		// store
		p->f[i].id = malloc(p->f[i].id_length);
		memcpy(p->f[i].id, buf, p->f[i].id_length);
		buf += p->f[i].id_length;
	}
	return buf;
}

void switch_screen() {
	prog[curr_program].curr_screen++;
	if (prog[curr_program].curr_screen > SCREEN_NUM) {
		prog[curr_program].curr_screen = 0;
	}
}

void move_cursor(int dx, int dy) {
	screen *scr = &prog[curr_program].scr[ prog[curr_program].curr_screen ];
	scr->posx += dx;
	// note that elseif cannot be used since scr->height might be less than SCRREN_HEIGHT
	if (scr->posx + SCREEN_HEIGHT > scr->height)
		scr->posx = scr->height - SCREEN_HEIGHT;
	if (scr->posx < 0)
		scr->posx = 0;

	if (scr->posy + SCREEN_WIDTH > scr->width)
		scr->width = scr->width - SCREEN_WIDTH;
	if (scr->posy < 0)
		scr->posy = 0;
}

void cursor_home() {
	screen *scr = &prog[curr_program].scr[ prog[curr_program].curr_screen ];
	scr->posx = 0;
	scr->posy = 0;
}

void cursor_end() {
	screen *scr = &prog[curr_program].scr[ prog[curr_program].curr_screen ];
	scr->posx = scr->height - SCREEN_HEIGHT;
	if (scr->posx < 0)
		scr->posx = 0;
	scr->posy = scr->width - SCREEN_WIDTH;
	if (scr->posy < 0)
		scr->posy = 0;
}

void init_addr(struct sockaddr_in *addr) {
	char sin_addr[4] = {127, 0, 0, 1}; // 127.0.0.1
	addr->sin_family = AF_INET;
	addr->sin_port = LISTEN_PORT;
	memcpy(&addr->sin_addr, sin_addr, 4);
	memset(&addr->sin_zero, 0, 8);
}

void init_curses() {
	initscr();
	cbreak();
	nonl();
	noecho();
	intrflush(stdscr, FALSE);
	keypad(stdscr, TRUE);
	refresh();
}

void init_fd() {
	int yes = 1; // no actual use
	
	// readfds are global
	init_addr(&server_addr);

	if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
		perror("Error Creating Socket");
		exit(1);
	}

	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
		perror("Error Setting Sock Options");
		exit(1);
	}

	if (bind(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1) {
		perror("Error Binding Port");
		exit(1);
	}

	if (listen(sockfd, PROGRAM_MAXNUM) == -1) {
		perror("Error Listening Port");
		exit(1);
	}

	FD_ZERO(&readfds);
	FD_SET(0, &readfds); // add standard input to readfds
	tv.tv_sec = 60;
	tv.tv_usec = 0;
}

void show_client() {
	int i, flag = FALSE;
	printw("------------------\n[Client] Socket ");
	refresh();
	for (i=0; i<PROGRAM_MAXNUM; i++) 
		if (fd[i] != 0) {
			printw("[%d] %d", i, fd[i]);
			flag = TRUE;
		}
	if (!flag)
		printw("NO CLIENT");
	printw("\n");
	refresh();	
}

void close_all_sock() {
	int i;
	for (i=0; i<PROGRAM_MAXNUM; i++) 
		if (fd[i] != 0) 
			close(fd[i]);
}
*/
void save(char *str) {
	FILE *fp;
	fp = fopen("/dev/blindbf", "w");
	fprintf(fp, "%s", str);
	fclose(fp);
}

char smile[] = "";
char nongfu[] = "";
char nike[] = "";

int main() {

	int fd_serial;
	char c;
	char a[1000];
	printf("starting...  ");
	open_port(&fd_serial);

	printf("starting...  ");
	save(smile);
	c = (char)1; // smile
	write(fd_serial, &c, 1);

	sleep(10000);
	printf("started.\n");

	// Menu
	system("curl localhost/blind/lib_braille.php?str=Plot_Barcode_Snake_Reader_Search_Chat");
	c = (char)6;
	write(fd_serial, &c, 1);

	printf(" > ");
	getchar();
	// Plot
	system("curl localhost/blind/lib_braille.php?str=Plot");
	printf(" > ");
	scanf("%s", a);
	c = (char)4;
	write(fd_serial, &c, 1);
	system("curl localhost/blind/function.php?content=sin(x)");
	
	printf(" > ");
	scanf("%s", a);
	c = (char)2;
	write(fd_serial, &c, 1);
	system("curl localhost/blind/function.php?content=cos(x/3)");

	printf(" > ");
	getchar();

	// barcode
	system("curl localhost/blind/barcode.php");
	c = (char)1;
	write(fd_serial, &c, 1);
	
	sleep(5000);
	save(nongfu);
	c = (char)2;
	write(fd_serial, &c, 1);

	printf(" > ");
	getchar();

	system("curl localhost/blind/barcode.php");
	c = (char)1;
	write(fd_serial, &c, 1);

	sleep(5000);
	save(nike);
	c = (char)6;
	write(fd_serial, &c, 1);

	printf(" > ");
	getchar();

	system("curl localhost/blind/barcode.php");
	c = (char)2;
	write(fd_serial, &c, 1);

	printf(" > ");
	getchar();

	// Reader
	system("curl localhost/blind/lib_braille.php?str=Input_File_Name");
	printf(" > ");
	scanf("%s", a);
	system("curl localhost/blind/lib_braille.php?str=Read_Me_This_Is_The_Reader");
	c = (char)1;
	write(fd_serial, &c, 1);

	getchar();

	// search
	system("curl localhost/blind/lib_braille.php?str=Search");
	c = (char)2;
	write(fd_serial, &c, 1);
	printf(" > ");
	scanf("%s", a);
	system("curl localhost/blind/lib_braille.php?str=Search_Result_This_Is_Very_Good");
	c = (char)4;
	write(fd_serial, &c, 1);

	printf(" > ");
	getchar();
	
	// chat
	system("curl localhost/blind/lib_braille.php?str=Chat");
	c = (char)4;
	write(fd_serial, &c, 1);
	
	while(1) {
		FILE *fp;
		fp = fopen("/srv/gewu/blind/chat_blind.txt", "w");
		printf(" > ");
		scanf("%s", a);
		fprintf(fp, "%s", a);
		fclose(fp);
		system("curl localhost/blind/lib_chat.php");
		c = (char)2;
		write(fd_serial, &c, 1);
	}
/*
	// enter curses mode
	init_curses();
	// init file socket
	init_fd();

	printw("Waiting For Input...\n");
	refresh();

	// deprecated:
	// 	for each: fcntl(sock_fd, F_SETFL, O_NONBLOCK);
	while (TRUE) {
		int *data;
		int ret;
		int new_fd;
		int i;

		// select a new event
		ret = select(max_socket + 1, &readfds, NULL, NULL, &tv);
		if (ret < 0) {
			perror("Error in SELECT system call. Retry...\n");
			continue;
		} else if (ret == 0) {
			printw("Timeout. Retry...\n");
			refresh();
			continue;
		}

		// poll sockets
		for (i=0; i<program_num; i++) {
			// if new message comes
			if (FD_ISSET(fd[i], &readfds)) {
				ret = recv(fd[i], buf, BUF_SIZE, 0);
				if (ret <= 0) { // fail
					printw("client %d close\n", i);
					refresh();
					close(fd[i]);
					FD_CLR(fd[i], &readfds);
					fd[i] = 0;
				}
				else { // receive success
					char *buf_rear;
					if (ret < BUF_SIZE) {
						memset(&buf[ret], '\0', 1);
					}
					buf_rear = extract_pid(buf, &prog[i]);
					buf_rear = update_screen(buf, &prog[i]);
					update_focus(buf_rear, &prog[i]);
					update_device();
				}
			}
		}

		// accept new connection
		if (FD_ISSET(sockfd, &readfds)) {
			new_fd = accept(sockfd, (struct sockaddr *)&client_addr, &sin_size);
			if (new_fd <= 0) {
				perror("Error Accepting socket");
				continue;
			}
			if (program_num < PROGRAM_MAXNUM) {
				fd[program_num] = new_fd;
				prog[program_num].curr_screen = 0; // current screen is 0 by default
				program_num++;

				printw("New client %d %s : %d\n", program_num, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
				refresh();
				if (new_fd > max_socket) {
					max_socket = new_fd;
				}
			} else {
				printw("Too Many Connections! Connection refused.\n");
				refresh();
				close(new_fd);
				continue;
			}
		}

		// handle standard input with curses
		if (FD_ISSET(0, &readfds)) {
			int ch = getch();
			printw("Readin [%c] : %d\n", ch, ch);
			refresh();
			switch (ch) {
			case KEY_UP:
				move_cursor(-1, 0);
				break;
			case KEY_DOWN:
				move_cursor(1, 0);
				break;
			case KEY_LEFT:
				move_cursor(0, -1);
				break;
			case KEY_RIGHT:
				move_cursor(0, 1);
				break;
			case KEY_HOME:
				cursor_home();
				break;
			case KEY_END:
				cursor_end();
				break;
			case KEY_NPAGE:
				move_cursor(SCREEN_HEIGHT, 0);
				break;
			case KEY_PPAGE:
				move_cursor(-SCREEN_HEIGHT, 0);
				break;
			case '\t':
				switch_screen();
				break;
			default:
				break;
			}
			
			send_key(ch);
			update_device();
		}

		//show_client();
	}
	
	close_all_sock();
	// exit curses mode
	endwin();
*/
	return 0;
}
