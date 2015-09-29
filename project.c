/* This is File Encryption and Decryption program using AES alogorithm the alogorithm is on papaer and not yet build properly thats why  uploading a patial program just of menu driven programs and little idea of project the cocept is in readme file how actually it works*/
# include "help.h"
# include "Decryption.h"
# include "Readme.h"
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
int main() {
	int choice;
	char ch[20];
	printf("\n/*/*/*/*/*/*/* Welcome to World of Encryption */*/*/*/*/*/* \t\n\n");
	printf("1. Encrypt a File\n");
	printf("2. Decrypt a File\n");
	printf("3. Readme (Recomanded for new uers)\n");
	printf("4. Exit\n");
	scanf("%d", &choice);
	switch(choice) {
		case 1:
			//Encryption();
			break;
		case 2:
			Decryption();
			break;
		case 3:
			Readme();
		case 4:
			return 0;
			break;
		default:
			break;
	}
return 0;
}

