#include <stdio.h>
#include "transmission.h"
int main(){
	int i;
	int fd;
	open_port(&fd);
	while(1){
		puts("1.smile");
		puts("2.nong fu shan quan");
		puts("3.braille");
		puts("4.plot");
		puts("5.book");
		puts("6.nike");
		scanf("%d",&i);
		char c = (char)i;
		write(fd,&c,1);
	}
}
