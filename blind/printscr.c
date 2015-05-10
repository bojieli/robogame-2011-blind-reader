#include "bitmap.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
unsigned char zero[BYTES] = {0};
int main(){
	while(1){
		int fd = open("/dev/blindfb",O_WRONLY);
		if(fd==-1){
			perror("error!");
			return 1;
		}
		int choise;
		unsigned char *bmp;
		system("clear");
		puts("1.smile\n2.nong fu shan quan\n3.nike\n4.book\n5.clear");
		scanf("%d",&choise);
		switch(choise){
			case 1:
				bmp = smile;
				break;
			case 2:
				bmp = nfsq;
				break;
			case 3:
				bmp = nike;
				break;
			case 4:
				bmp = book;
				break;
			case 5:
				bmp = zero;
				break;
			default:
				continue;
		}
		write(fd,bmp,BYTES);
	}
}
