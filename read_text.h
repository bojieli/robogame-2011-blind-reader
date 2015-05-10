// read_text.h: speech synthesizer
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

void read_text(char *str) {
	char *pre = "espeak \"", *post = "\"";
	char *cmd = (char *)malloc( (strlen(pre) + strlen(str) + strlen(post)) * sizeof(char));
	*cmd = '\0';
	strcat(cmd, pre);
	strcat(cmd, str);
	strcat(cmd, post);
	system(cmd);
	free(cmd);
}
