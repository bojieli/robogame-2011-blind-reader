#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <syslog.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include "transmission.h"

#define ROWS 15
#define COLS 20
#define BYTES 38
#define DEV "/dev/blindfb"
#define LOG "blindsyncd"

void init_daemon(){
	int pid;
	if((pid=fork())) exit(0);
	else if(pid<0){
		perror("Can't create proccess");
		syslog(LOG_ERR,"Can't create proccess");
		exit(1);
	}
	setsid();
	if((pid=fork())) exit(0);
	else if(pid<0){
		perror("Can't create proccess");
		syslog(LOG_ERR,"Can't create proccess");
		exit(1);
	}
	chdir("/");
	umask(0);
	return;
}
		

//sta,[index],[index],[index],[data],[data],[data],end
int main() {
	//init system log
	openlog(LOG,LOG_PID,LOG_DAEMON);
	//create a daemon
	signal(SIGCHLD,SIG_IGN);
	init_daemon();
	//do sync
	int fd;
	open_port(&fd);
	if(fd==-1)
		syslog(LOG_ERR,"Can't open port");
	unsigned char data[BYTES*12] = {0};
	unsigned char buf[BYTES] = {0};
	while(1){
		sleep(1);
		//read file
		FILE *fp;
		fp = fopen(DEV,"rb");
		if (fp == NULL) {
			syslog(LOG_ERR,"Error opening file!");
			return 1;
		}
		int result = fread(buf,1,BYTES,fp);
		fclose(fp);
		if (result != BYTES) {
			syslog(LOG_ERR,"Error reading file!");
			return 1;
		}
		//send data
		int i;
		unsigned char *bufptr;
		for(i=0,bufptr=data;i<BYTES;i++){
			*bufptr++ = 's';
			*bufptr++ = 't';
			*bufptr++ = 'a';
			*bufptr++ = (unsigned char)i;
			*bufptr++ = (unsigned char)i;
			*bufptr++ = (unsigned char)i;
			*bufptr++ = buf[i];
			*bufptr++ = buf[i];
			*bufptr++ = buf[i];
			*bufptr++ = 'e';
			*bufptr++ = 'n';
			*bufptr++ = 'd';
		}
		write(fd,data,sizeof(data));
	}
	close_port(fd);
}
