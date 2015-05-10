#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
int main(){
	char ip_addr[16];
	int port;
	int fd,cfd;
	int connected = 0;
	while(1){
		struct sockaddr_in addr;
		puts("input port:");
		scanf("%d",&port);
		if((fd=socket(AF_INET,SOCK_STREAM,0))==-1){
			puts("error creating socket!\ncontinue.");
			continue;
		}
		addr.sin_family = AF_INET;
		addr.sin_port = htons(port);
		addr.sin_addr.s_addr = INADDR_ANY;
		bzero(&addr.sin_zero,8);
		if(bind(fd,(struct sockaddr *)&addr,sizeof(struct sockaddr))==-1){
			puts("error binding!\ncontinue.");
			continue;
		}
		if(listen(fd,1)==-1){
			puts("error listening!\ncontinue");
			continue;
		}
		puts("listening...");
		int sin_size = sizeof(struct sockaddr_in);
		struct sockaddr_in caddr;
		while(1){
			if((cfd=accept(fd,(struct sockaddr *)&caddr,&sin_size))==-1){
				puts("error accepting!\ncontinue.");
				continue;
			}
			puts("connection established");
			help:printf("\"recv <bytes>\" for receive\n""\"send <bytes>\" for send\n""\"quit\" for quit\n");
			while(1){
				printf(">");
				char cmd[5];
				scanf("%s",cmd);
				if(!strcmp(cmd,"recv")){
					//receive
					int bytes;
					scanf("%d",&bytes);
					int received;
					unsigned char buf[bytes];
					if((received=recv(cfd,buf,bytes,0))==-1){
						puts("error receiving!");
					}else{
						printf("%d bytes received:\n",received);
						int i;
						for(i=0;i<received-1;i++)
							printf("%02xH:%03dD\t",(int)buf[i],(int)buf[i]);
						printf("%02xH:%02dD\n",(int)buf[i],(int)buf[i]);
					}
					continue;
				}
				if(!strcmp(cmd,"send")){
					//send
					int bytes;
					scanf("%d",&bytes);
					unsigned char buf[bytes];
					int i;
					puts("input data in hex:");
					for(i=0;i<bytes;i++){
						int hex;
						scanf("%x",&hex);
						buf[i] = (unsigned char)hex;
					}
					if(send(cfd,buf,bytes,0)==-1)
						puts("error sending!");
					continue;
				}
				if(!strcmp(cmd,"quit")){
					close(cfd);
					break;
				}
				goto help;
			}
		}
	}
}
