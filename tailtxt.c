#include <stdio.h>
#include <stdlib.h>
#include <err.h>
#include <dirent.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

//cuando algo falla se acaba con exit(1), aunque sea  error en lseek

void
analycefich(char* fich, int N)
{
	int fd,nr,lk;
	char buffer[6];

	fd = open(fich, O_RDONLY);
	if(fd < 0){
		warn("Error al abrir el fichero %s", fich);
		return;	
	}

	if (N>=0){
	 	lk = lseek(fd,-N,SEEK_END);
	 	if (lk < 0){
	 		warn("Error en funcion lseek en el fichero %s", fich);
	 		return;  //ha habido error en el lseek
	 	}
	}
	for(;;){
		
		memset(buffer,0,sizeof(buffer)); //asi inicializa la variable buffer
		nr = read(fd, &buffer, sizeof(buffer));
		if(nr == 0){
			break;
		}
		if(nr < 0){
			warn("Error de lectura de fichero %s", fich);
			return; 
		}
		printf("%s", buffer);
	}
	close(fd);
	return;

}

int
isfile(char* fich)
{
	struct stat stbuf;

	if (stat(fich, &stbuf) < 0)	{
		warn("Error al usar stat en el fichero %s",fich);
		return 0;
	}
    if ((stbuf.st_mode & S_IFMT) == S_IFREG)	return 1;

    return 0;

} //1 cuando es fichero, 0 cuando no lo es o ha habido error


void 
search_txt (int N)
{
	
	DIR *d;
	struct dirent *dp;
	char *cadena;
	char ext[] =".txt";

	
	d = opendir(".");
	if(d == NULL) {
		err(1, "Error al abrir directorio actual");
	}
	while( (dp = readdir(d)) != NULL ) {

		if (isfile(dp->d_name)){

			cadena = strrchr( dp->d_name,'.');  //coge a partir de el ultimo punto
			if((cadena != NULL) && (strcmp(cadena,ext)==0)){
			
				analycefich(dp->d_name,N);
			}
		}
	}
}

int
main(int argc, char* argv[])
{
	int  N;
	if(argc==1){
		N=-1; // -1 para  comprobarlo luego y que no se use
	}else{
		N = atoi(argv[1]);  //atoi convierte char* a int, si no meto  un numeor me devuelve 0
		if (N==0){
			err(1, "Debe introducir un numero N mayor que 0");
		}
	}
	
	search_txt(N);

	printf("\n");

	exit(0);
}

//lseek 2
//strstr 3 strrchr
//stat 2 mascaras y no se que
//readdir 2
