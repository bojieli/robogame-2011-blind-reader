#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include<stdio.h>
int main(){
	int fd,res;
	char c;
	pid_t pid = 9999;
	struct sockaddr_in dest_addr;
	fd = socket(AF_INET,SOCK_STREAM,0);
	if(fd==-1){
		perror("error creating!");
		return 1;
	}
	dest_addr.sin_family = AF_INET;
	dest_addr.sin_port = htons(40000);
	dest_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	bzero(&(dest_addr.sin_zero),8);
	res = connect(fd,(struct sockaddr*)&dest_addr,sizeof(struct sockaddr_in));
	if(res ==-1){
		perror("error connecting!");
		return 1;
	}
	send(fd, &pid, sizeof(pid), 0);
	while (1) {
		send(fd,&c,sizeof(c),0);
		c = getchar();
	}
	close(fd);
}
