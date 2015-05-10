#include<stdio.h>
#include "./read_text.c"
int main() {
	char str[100];
	while(1) {
		scanf("%s", str);
		read_text(str);
	}
	return 0;
}
