#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>

#define TTY_NODE "/dev/ttyUSB0"
void open_port(int *fd){
	*fd = open(TTY_NODE,O_RDWR|O_NOCTTY|O_NDELAY);
	if(*fd!=-1){
		fcntl(*fd,F_SETFL,0);
		struct termios options;
		tcgetattr(*fd,&options);
		//set baud
		cfsetispeed(&options,B9600);
		cfsetospeed(&options,B9600);
		//set local
		options.c_cflag |= (CLOCAL|CREAD);
		//set check to 8N1
		options.c_cflag &= ~PARENB;
		options.c_cflag &= ~CSTOPB;
		options.c_cflag &= ~CSIZE;
		options.c_cflag |= CS8;
		//cancel soft stream control
		options.c_lflag &= ~(IXON|IXOFF|IXANY);
		tcsetattr(*fd,TCSANOW,&options);
	}
}
void close_port(int fd){
	close(fd);
}
