#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include "help.h"
int Help() {
	char arr;
	FILE *fp = fopen("help", "r");
	if(fp == NULL) {
		perror("fopen failed");
		return errno;
	}
	while(fread(&arr, sizeof(char), 1, fp)) 
		printf("%c", arr);
	return 0;
}
