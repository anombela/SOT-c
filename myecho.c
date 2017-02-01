#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

void 
arguments(int argc, char **argv){

	char *home, *dir, *user;
	int pid,i=1;

	if (strcmp(argv[1],"-n")==0)	i++;
	
	for (; i < argc; i++){
	
		if(strcmp(argv[i],"USUARIO")==0){

			user=getenv("USER");
			if (user == NULL){
				fprintf(stderr, "'getenv('USER')'returns null");
				exit(1);
			}
			printf("%s",user);

		}else if(strcmp(argv[i],"CASA")==0){

			home = getenv("HOME");
			if (home == NULL){
				fprintf(stderr, "'getenv('HOME')'returns null");
				exit(1);
			}
			printf("%s",home);

		}else if(strcmp(argv[i],"DIRECTORIO")==0){

			dir = getenv("PWD");
			if (dir == NULL){
				fprintf(stderr, "'getenv('PWD')'returns null");
				exit(1);
			}
			printf("%s",dir);

		}else if(strcmp(argv[i],"*")==0){

			pid = getpid();
			printf("%d",pid);

		}else{

			printf("%s",argv[i]);

		}

		if (i!=argc-1)	printf(" ");

	}
}

int
main(int argc, char* argv[])
{
	if (argc==1){
		printf("\n");

	}else{
		arguments(argc,argv);
		if (strcmp(argv[1],"-n")!=0)	printf("\n");
	}
	exit(0);
}