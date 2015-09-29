#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include "Decryption.h"
/* Currently just decrypting the binary code directly without any key */
struct ajinkya {
	long a;
	char b[40];
};
int Decryption() {
	int choice;
	printf("\n/*/*/*/*/*/*/* Decryption */*/*/*/*/*/* \t\n\n");
	printf("1. Decrypt a File\n");
	printf("2. back\n");
	printf("3. Help\n");
	printf("4. Exit\n");
	scanf("%d", &choice);
	FILE* file = fopen("decrypt", "r");
	switch(choice) {
		case 1:
			if(!file) {
				printf("File is not present\n");
			return 1;
			}
	
			struct ajinkya c;
			while( fread(&c, sizeof(c), 1, file) ) {
				printf("%ld%s\n",c.a, c.b);
			}
	
			fclose(file);
			return 0;
			break;
		case 2:
			//retun main();
			break;
		case 3:
			//help();
		case 4:
			return 0;
			break;
		default:
			break;
	}
}
