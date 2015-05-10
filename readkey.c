#include<stdio.h>
int read_key() {
	int key;
	while (bioskey(1) == 0);
	key = bioskey(0);
	if ((key & 0xff) != 0)
		key = key & 0xff;
	printf("%d\n", key);
}
int main() {
	while (1) {
		read_key();
	}
}
