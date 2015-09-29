try: Decryption.o help.o project.o Readme.o
	cc *.o -o project
Decryption.o: Decryption.c
	cc Decryption.c -c
help.o: help.c
	cc help.c -c
project.o: project.c Decryption.c help.c Readme.c
	cc Decryption.c -c
	cc help.c -c
	cc Readme.c -c
	cc project.c -c
Readme.o: Readme.c
	cc Readme.c -c
