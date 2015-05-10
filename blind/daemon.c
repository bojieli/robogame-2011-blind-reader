#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<sys/socket.h>
#include<fcntl.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<curses.h>
#include<unistd.h>
#include<signal.h>
#define TRUE 1
#define FALSE 0

#define DEVICE_FILE "/dev/blindfb"
#define ID_MAXLEN 256
#define SCREEN_NUM 16
#define EVENT_MAXNUM 2048
#define SCREEN_WIDTH 20
#define SCREEN_HEIGHT 15
#define SCREEN_BYTES 38
#define PROGRAM_MAXNUM 100
#define LISTEN_PORT 40000
#define BUF_SIZE 2097152 // 256 * 256 * 16 * 2
#define LOCAL_ESCAPE_KEY 27
#define MAX_WIDTH 256
#define MAX_HEIGHT 256
#define ID_MAXLENGTH 256

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

#define STATUS_INTERRUPTABLE 1
#define STATUS_UNINTERRUPTABLE 2
#define STATUS_EXIT 4
#define STATUS_ZOMBIE 8
#define STATUS_DISCONNECTED 16

typedef struct program {
	pid_t pid;
	short status;
	int curr_screen;
	screen scr[SCREEN_NUM];
	int focus_num;
	focus_area f[EVENT_MAXNUM];
} program;

program prog[PROGRAM_MAXNUM];
int curr_program = 0;
int prev_program = 0;
int fd[PROGRAM_MAXNUM] = {0};
int program_num = 1; // increment from 1, 0 is reserved for menu

int menu_fd = 0;
pid_t monitor_pid = 0;
int monitor_fd = 0;

int sockfd;
struct sockaddr_in server_addr;
struct sockaddr_in client_addr;
socklen_t sin_size;
int max_socket = 0;

fd_set readfds;
struct timeval tv;
char buf[BUF_SIZE];
char curr_device[SCREEN_BYTES] = {0};

void close_all_sock() {
	int i;
	for (i=0; i<PROGRAM_MAXNUM; i++) 
		if (fd[i] != 0)
			close(fd[i]);
}

void exit_daemon() {
	if (monitor_pid != 0)
		kill(monitor_pid, SIGINT);
	if (prog[0].pid != 0)
		kill(prog[0].pid, SIGINT);
	printw("\nKilling processes %d %d\n", monitor_pid, prog[0].pid);
	close_all_sock();
	printw("Press any key to exit daemon...");
	refresh();
	fcntl(0, F_SETFL, 0);
	getch();
	endwin();
	exit(0);
}

int send_key(int ch) {
	// type = KeyPressed, keycode, id_length = 0
	int buf[3] = {2, ch, 0};
	if (fd[curr_program]) {
		send(fd[curr_program], (char *)buf, 3 * sizeof(char), 0);
		return TRUE;
	}
	return FALSE; // fd not found
}

int press_button(int button) {
	// button is number of button in current screen numbered from 1, order by `y`, `x`
	// type = ButtonPressed (3), keycode, id_length, id
	int i, max, maxy, maxx;
	program *p = &prog[curr_program];
	screen *scr = &p->scr[p->curr_screen];
	short flag[p->focus_num];
	int left = scr->posx;
	int right = (scr->posx + SCREEN_WIDTH > scr->width) ? scr->width : scr->posx + SCREEN_WIDTH;
	int top = scr->posy;
	int bottom = (scr->posy + SCREEN_HEIGHT > scr->height) ? scr->height : scr->posy + SCREEN_HEIGHT;

	for (i=0; i < p->focus_num; i++)
		if (p->f[i].screen == p->curr_screen) {
			if (p->f[i].x >= left && p->f[i].x + p->f[i].width <= right && p->f[i].y >= top && p->f[i].y + p->f[i].height <= bottom)
				flag[i] = TRUE;
		}
		else flag[i] = FALSE;

	while (button > 0) {
		max = -1, maxy = 0, maxx = 0;
		button--;
		for (i=0; i < p->focus_num; i++) {
			if (flag[i]) { // rank first by y, then by x
				if (p->f[i].y > maxy || (p->f[i].y == maxy && p->f[i].x > maxx)) {
					max = i;
					maxx = p->f[i].x;
					maxy = p->f[i].y;
				}
			}
		}
		if (max != -1) {
			flag[max] = FALSE; // do not go into next turn
		}
	}
	if (max == -1) {
		return FALSE; // no button found in current screen
	}
	else {
		int type = 3, keycode = 0;
		send(fd[curr_program], &type, sizeof(int), 0);
		send(fd[curr_program], &keycode, sizeof(int), 0);
		send(fd[curr_program], &p->f[max].id_length, sizeof(int), 0);
		send(fd[curr_program], p->f[max].id, p->f[max].id_length, 0);
		return TRUE;
	}
}

int write_device(char *data) {
	int fd = open(DEVICE_FILE, O_WRONLY); // write only device file
	if (!fd) {
		return FALSE;
	}
	
	{
	int i;
	printw("Writing device: ");
	for (i=0; i<SCREEN_BYTES; i++)
		printw("%02x", data[i]);
	printw("\n");
	refresh();
	}
	write(fd, data, SCREEN_BYTES);
	close(fd);
	return TRUE;
}

void update_device() {
	int i, j, flag = FALSE;
	char new_device[SCREEN_BYTES] = {0};
	program *p = &prog[curr_program];
	screen *scr = &p->scr[p->curr_screen];
	
	for (i=0; i<(SCREEN_HEIGHT > scr->height ? scr->height : SCREEN_HEIGHT); i++)
		for (j=0; j<(SCREEN_WIDTH > scr->width ? scr->width : SCREEN_WIDTH); j++) {
			int offset = i * SCREEN_WIDTH + j;
			char binary = *((char *)scr->data + (scr->posx + i) * scr->width + (scr->posy + j));
			new_device[(offset + 7) >> 3] |= binary << ((offset + 7) % 8);
		}
	
	for (i=0; i<SCREEN_BYTES; i++)
		if (new_device[i] != curr_device[i])
			flag = TRUE;

	if (flag) {
		memcpy(curr_device, new_device, SCREEN_BYTES);
		write_device(curr_device);
	}
}

/*
char* extract_pid(char *buf, program *p) {
	memcpy(&p->pid, buf, sizeof(pid_t)); // the first 4 bytes is PID
	return buf + sizeof(pid_t);
}*/

pid_t receive_pid(int fd) {
	pid_t pid = 0;
	int status = recv(fd, &pid, sizeof(pid_t), 0);
	printw("Receive PID [%d] from SOCK [%d]\n", pid, fd);
	refresh();
	if (status <= 0 || pid < 0 || pid > 65536)
		return -1; // receive failure or not a valid pid
	return pid;
}

char* update_screen(char *buf, program *p) {
	int i;
	for (i=0; i<SCREEN_NUM; i++) {
		screen *s = &p->scr[i];
		int total_byte, byte, bit;

		if (*(int *)buf <= 0 || *(int *)buf > MAX_WIDTH) {
			return buf;
		}
		memcpy(&s->width, buf, 4);
		buf += 4;
		
		if (*(int *)buf <= 0 || *(int *)buf > MAX_HEIGHT) {
			return buf;
		}
		memcpy(&s->height, buf, 4);
		buf += 4;

		total_byte = (s->width * s->height + 7) >> 3;
		s->data = malloc(total_byte << 3);
		for (byte = 0; byte < total_byte; byte++) {
			for (bit = 0; bit < 8; bit++) {
				s->data[byte << 3 + 7 - bit] = buf[byte] & 1;
				buf[byte] >>= 1;
			}
		}
		buf += total_byte;

		printw("SCREEN [%d] %d * %d\n", i, s->width, s->height);
		{ int j,k;
		for (j=0;j<s->height;j++) {
			for (k=0;k<s->width;k++)
				printw("%d", s->data[j * s->width + k]);
			printw("\n");
		}
		}
		refresh();
	}
	return buf; // postion after chunk of data
}

char* update_focus(char *buf, program *p) {
	int i;
	if (*(int *)buf < 0 || *(int *)buf > EVENT_MAXNUM)
		return buf;
	p->focus_num = *(int *)buf;
	buf += sizeof(int); // pointer move forward 4 bytes
	for (i=0; i<p->focus_num; i++) {
		// copy screen, x, y, width, height, id_length
		memcpy(&p->f[i], buf, 6 * sizeof(int));
		buf += 6 * 4;
		// store
		if (p->f[i].id_length < 0 || p->f[i].id_length > ID_MAXLENGTH)
			return buf;
		p->f[i].id = malloc(p->f[i].id_length);
		memcpy(p->f[i].id, buf, p->f[i].id_length);
		buf += p->f[i].id_length;
	}
	return buf;
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

void notify_monitor() {
	// send: [pid screenid]
	if (monitor_fd == 0) return;
	send(monitor_fd, (char *)&prog[curr_program].pid, sizeof(int), MSG_DONTWAIT);
	send(monitor_fd, (char *)&prog[curr_program].curr_screen, sizeof(int), MSG_DONTWAIT);
}

int not_interruptable() {
	return (prog[curr_program].status == STATUS_UNINTERRUPTABLE);
}

int is_running(int program_no) {
	return (prog[curr_program].status == STATUS_INTERRUPTABLE || prog[curr_program].status == STATUS_UNINTERRUPTABLE);
}

int goto_program(int program_no) {
	if (not_interruptable())
		return FALSE;
	if (!is_running(program_no))
		return FALSE;
	
	prev_program = curr_program;
	curr_program = program_no;
	notify_monitor();
	return TRUE;
}

int switch_program() { // offset = 1
	if (not_interruptable())
		return FALSE;

	prev_program = curr_program;		
	// find next running program
	// program 0 (menu) is always running, hence the loop is finite
	do {
		curr_program++;
		if (curr_program == program_num)
			curr_program = 0;
	} while(!is_running(curr_program));
	notify_monitor();
	return TRUE;
}

int close_program(int program_no) {
	// menu program is not allowed to exit
	if (program_no <= 0 || program_no >= program_num)
		return FALSE;
	
	if (curr_program == program_no)
		switch_program();

	prog[program_no].status = STATUS_ZOMBIE;
	close(fd[program_no]); // close connection

	if (kill(prog[program_no].pid, 0) == 0) {
		prog[program_no].status = STATUS_EXIT;
		return TRUE;
	}
	else return FALSE;
}

void switch_screen(int offset) {
	prog[curr_program].curr_screen += offset;
	prog[curr_program].curr_screen %= SCREEN_NUM;
	notify_monitor();
}

void init_addr(struct sockaddr_in *addr) {
	char sin_addr[4] = {127, 0, 0, 1}; // 127.0.0.1
	addr->sin_family = AF_INET;
	addr->sin_port = htons(LISTEN_PORT);
	memcpy(&addr->sin_addr, sin_addr, 4);
	memset(&addr->sin_zero, 0, 8);
}

void init_curses() {
	initscr();
	cbreak();
	nonl();
	raw();
	noecho();
	scrollok(stdscr, TRUE);
	keypad(stdscr, TRUE);
	fcntl(0, F_SETFL, O_NONBLOCK);
	refresh();
}

void init_fd() {
	int yes = 1; // no actual use
	
	// readfds are global
	init_addr(&server_addr);

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		printw("Error Creating Socket");
		refresh();
		exit_daemon();
	}

	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
		printw("Error Setting Sock Options");
		refresh();
		exit_daemon();
	}

	if (bind(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1) {
		printw("Error Binding Port");
		refresh();
		exit_daemon();
	}

	if (listen(sockfd, PROGRAM_MAXNUM) == -1) {
		printw("Error Listening Port");
		refresh();
		exit_daemon();
	}

	max_socket = sockfd; // for select()
	tv.tv_sec = 600; // wait for 10 minutes and automatically exit
	tv.tv_usec = 0;
	
	{ // ignore SIGPIPE when trying to send to a disconnected socket
	struct sigaction sa;
	sa.sa_handler = SIG_IGN;
	sigaction( SIGPIPE, &sa, 0 );
	}
}

void show_client() {
	int i, flag = FALSE;
	printw("Client sockets: ");
	for (i=0; i<PROGRAM_MAXNUM; i++)
		if (fd[i] != 0) {
			printw("%d[%d]", i, fd[i]);
			flag = TRUE;
		}
	if (!flag)
		printw("NO CLIENT");
	printw("\n");
	refresh();	
}

void init_monitor() {
	pid_t pid = fork();
	if (pid > 0) { // father process
		monitor_pid = pid;
	} else {
		//system("./monitor");
		execl("./monitor", NULL);
		exit(0);
	}
}

void init_menu() {
	pid_t pid = fork();
	if (pid > 0) { // father process
		prog[0].pid = pid; // the menu is program zero
		fd[0] = 0; // init fd to illegal num
	}
	else { // child process
		execl("./menu", NULL);
		//system("./menu");
		exit(0);
	}
}

int poll_sockets() {
		int i, ret;
		for (i=0; i<program_num; i++) {
			// if new message comes
			if (fd[i] != 0 && FD_ISSET(fd[i], &readfds)) {
				printw("Trying to receive data from SOCK [%d]...\n", fd[i]);
				refresh();
				bzero(buf, BUF_SIZE);
				ret = recv(fd[i], buf, BUF_SIZE, MSG_DONTWAIT);
				if (ret <= 0) { // fail
					printw("client %d [socket %d] close\n", i, fd[i]);
					refresh();
					close(fd[i]);
					fd[i] = 0;
				}
				else { // receive success
					char *buf_rear;
					int j;
					printw("Receive %d bytes from [%d]: ", ret, i);
					for (j=0; j<ret; j++) {
						printw("%02x", *((char *)buf + j));
					}
					printw("\n");
					refresh();
					buf_rear = update_screen(buf, &prog[i]);
					update_focus(buf_rear, &prog[i]);
					update_device();
				}
			}
		}
		return FALSE;
}

int accept_connection() {
		int new_fd;
		if (FD_ISSET(sockfd, &readfds)) {
			new_fd = accept(sockfd, (struct sockaddr *)&client_addr, &sin_size);
			printw("New socket [%d]\n", new_fd);
			refresh();

			if (new_fd <= 0) {
				printw("Error Accepting socket\n");
				refresh();
				return TRUE;
			}
			if (program_num < PROGRAM_MAXNUM) {
				pid_t pid = receive_pid(new_fd);

				if (pid > 0) {

					if (new_fd > max_socket) {
						max_socket = new_fd;
					}

					if (pid == prog[0].pid) { // the menu process
						if (fd[0] == 0) { // the first connection for data
							fd[0] = new_fd;
							prog[0].curr_screen = 0;
							prog[0].status = STATUS_INTERRUPTABLE;
							printw("Menu screen connected: sock [%d]\n", fd[0]);
							refresh();
						} else { // the program controlling connection
							menu_fd = new_fd;
							printw("Menu process connected: sock [%d]\n", fd[0]);
							refresh();
						}
					}
					else if (pid == monitor_pid) { // the monitor
						monitor_fd = new_fd;
						printw("Monitor connected.\n");
						refresh();
					}
					else { // common process
						fd[program_num] = new_fd;
						prog[program_num].pid = pid;
						prog[program_num].curr_screen = 0; // current screen is 0 by default
						prog[program_num].status = STATUS_INTERRUPTABLE;
						printw("New client %d %s : %d\n", program_num, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
						program_num++;
					}
				}
				else { // receive pid fail
					printw("Client did not send its valid pid\n");
					refresh();
					close(new_fd);
				}
			} else {
				printw("Too Many Connections! Connection refused.\n");
				refresh();
				close(new_fd);
				return TRUE;
			}

			return TRUE;
		}
		return FALSE;
}

int menu_proc() {
		int i;
		if (menu_fd != 0 && FD_ISSET(menu_fd, &readfds)) {
			pid_t pid = receive_pid(menu_fd);
			if (pid == -1) {
				printw("Menu sent illegal PID\n");
				refresh();
				return TRUE;
			}
			for (i=0; i<program_num; i++)
				if (prog[i].pid == pid)
					if (goto_program(i)) {
						printw("Menu: goto program %d\n", i);
						refresh();
						return TRUE;
					}

			int flag = (i != program_num); // whether program is running
			send(menu_fd, &flag, sizeof(int), MSG_DONTWAIT);
			printw("Notify menu that PID [%d] is ", pid);
			if (flag) printw("running");
			else printw("not running");
			printw("\n");
			refresh();
			
			if (pid == -1) { // exit program
				exit_daemon();
			}

			return TRUE;
		}
		return FALSE;
}

int std_input() {
		if (FD_ISSET(0, &readfds)) {
			int ch = getch();
			int sendkey = TRUE;
			if (ch == -1) return FALSE;
			printw("Readin ASCII [%d]\n", ch);
			refresh();
			switch (ch) {
			case KEY_UP:
				move_cursor(-1, 0);
				printw("CURSOR Up\n");
				refresh();
				break;
			case KEY_DOWN:
				move_cursor(1, 0);
				printw("CURSOR Down\n");
				refresh();
				break;
			case KEY_LEFT:
				move_cursor(0, -1);
				printw("CURSOR Left\n");
				refresh();
				break;
			case KEY_RIGHT:
				move_cursor(0, 1);
				printw("CURSOR Right\n");
				refresh();
				break;
			case KEY_HOME:
				cursor_home();
				printw("SCREEN HOME\n");
				refresh();
				break;
			case KEY_END:
				cursor_end();
				printw("SCREEN END\n");
				refresh();
				break;
			case KEY_NPAGE:
				move_cursor(SCREEN_HEIGHT, 0);
				printw("PAGE Down");
				refresh();
				break;
			case KEY_PPAGE:
				move_cursor(-SCREEN_HEIGHT, 0);
				printw("PAGE Up");
				refresh();
				break;
			case 27: // ESC
				if (curr_program != 0) {
					goto_program(0);
					printw("Go to main menu\n");
					refresh();
					sendkey = FALSE;
				} else {
					goto_program(prev_program);
					printw("Return to previos program\n");
					refresh();
					sendkey = FALSE;
				}
				break;
			case 9: // Ctrl + Tab
				switch_program();
				printw("Switching program\n");
				refresh();
				sendkey = FALSE;
				break;
			case 3: // Ctrl + C
				printw("Closing current program [%d] sock [%d]\n", curr_program, fd[curr_program]);
				refresh();
				close_program(curr_program);
				break;
			case 549: // Ctrl + PgUp
				switch_screen(-1);
				printw("Switch to previous screen\n");
				refresh();
				break;
			case 544: // Ctrl + PgDn
				switch_screen(1);
				printw("Switch to next screen\n");
				refresh();
				break;
			case 17: // Ctrl + Q: exit
				exit_daemon();
			}
			
			if (ch >= 265 && ch <= 276) { // is F1~F12 key
				int button = ch - 264;
				press_button(button);
				printw("Button %d pressed\n", button);
				refresh();
			}
			else { // common key
				send_key(ch);
			}

			update_device();
			return TRUE;
		}
		return FALSE;
}

void init_readfd() {
	int i;
	FD_ZERO(&readfds);
	FD_SET(0, &readfds); // add standard input to readfds
	FD_SET(sockfd, &readfds); // add socket listener to readfds
	FD_SET(menu_fd, &readfds);
	for (i=0; i<program_num; i++)
		if (fd[i] != 0)
			FD_SET(fd[i], &readfds);
}

int main() {
	// enter curses mode
	init_curses();
	printw("Curses init\n");
	refresh();
	// init file socket
	init_fd();
	printw("File socket init\n");
	refresh();
	// init monitor
	//init_monitor();
	printw("Monitor init\n");
	refresh();
	// fork menu process
	init_menu();
	printw("Menu init\n");
	printw("Initialization finished.\n");
	refresh();

	// deprecated:
	// 	for each: fcntl(sock_fd, F_SETFL, O_NONBLOCK);
	while (TRUE) {
		int ret;
		struct timeval tmp_tv = tv;
		
		// select a new event
		init_readfd();
		ret = select(max_socket + 1, &readfds, NULL, NULL, &tmp_tv);
		if (ret < 0) {
			printw("Error in SELECT system call. Retry...\n");
			refresh();
			break;
		} else if (ret == 0) {
			printw("Timeout.\n");
			refresh();
			exit_daemon();
		} else {
			printw("SELECT\n");
			refresh();
		}
		
		// handle standard input with curses
		if (std_input()) continue;
		// accept new connection
		if (accept_connection()) {
			show_client();
			continue;
		}
		// handle menu proc handler
		if (menu_proc()) continue;
		// poll sockets
		if (poll_sockets()) continue;
	}

	exit_daemon();
}
