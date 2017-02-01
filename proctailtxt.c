#include <stdio.h>
#include <stdlib.h>
#include <err.h>
#include <dirent.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

#include <errno.h>

//cuando algo falla se acaba con exit(1), aunque sea  error en lseek

int
openfichs(char* fich, char*fichout, int *fd){

	
	fd[0] = open(fich, O_RDONLY);
	if(fd[0] < 0){
		warn("Error al abrir el fichero %s", fich);
		return -1;
	}

	fd[1] = open(fichout, O_WRONLY|O_CREAT|O_TRUNC, 0660);
	if (fd[1] < 0) {
		warn("Error al crear el fichero %s", fich);
		return -1;
	}

	return 1;
} //-1 si error, array con fds si bien

int
analycefich(char* fich, int N)
{
	int nr,lk,nw,s;
	int fd[2];
	char buffer[8*1024]; //funciona con un buffer muy pequeño
	char fichout[strlen(fich)];


	strcpy(fichout,fich);  //copia fich a fichout
	strcat(fichout,".out");  //concatena .out a fichout

	s = openfichs(fich, fichout,fd);
	if (s<0)
		return 0;
	
	if (N>=0){
	 	lk = lseek(fd[0],-N,SEEK_END);
	 	if (lk < 0){
	 		warn("Error en funcion lseek en el fichero %s", fich);
	 		//return 0;  //ha habido error en el lseek(((lo esribo igual- fichero completo)))
	 	}
	}
	for(;;){

		memset(buffer,0,sizeof(buffer)); //asi inicializa la variable buffer
		nr = read(fd[0], &buffer, sizeof(buffer));
		if(nr == 0){
			break;
		}
		if(nr < 0){
			warn("Error de lectura de fichero %s", fich);
			return 0;
		}
		nw=write(fd[1], buffer, nr);// escribe en fichero de salida
		if (nw != nr){
			warn("Error de escritura en fichero %s", fichout);
		}
		//printf("%s", buffer);

	}
	close(fd[0]);
	close(fd[1]);
	return 1;

} //return 0 si va  mal, 1 si va bien



int
createprocess(int N, char **fichs, int numfichs){

	int pid, sts,i;

	for (i = 0; i < numfichs; i++){
	
		pid = fork();
		switch(pid){
			case -1:

				exit(1);

			case 0:

				if (analycefich(fichs[i],N))
					exit(0); //ha acabado bien la funcion
				exit(1); //no ha acabado bien la funcion
		}
	}
	while(wait(&sts) != -1){ // -1 devuleve cuando no hay hijos que esperar o error

		if (WIFEXITED(sts)) {
            if (WEXITSTATUS(sts)) return 0;
        }
	}

	printf("\n%s\n",strerror(errno)); //debe escribir no child process
	return 1; 

}//1 si acaba bien, 0 si algun proceso a acabado mal

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


int
search_txt ( char **fichs)
{

	DIR *d;
	struct dirent *dp;
	char *cadena;
	char ext[] =".txt";
	int nfich=0;



	d = opendir(".");
	if(d == NULL) {
		err(1, "Error al abrir directorio actual");
	}
	while( (dp = readdir(d)) != NULL ) {

		if (isfile(dp->d_name)){

			cadena = strrchr( dp->d_name,'.');  //coge a partir de el ultimo punto
			if((cadena != NULL) && (strcmp(cadena,ext)==0)){

				if (nfich>20)
					err(1,"Mas de 20 ficheros acabados en .txt encontrados");

				fichs[nfich] = dp->d_name;
				nfich++;
			}
		}
	}
	closedir(d);
	return nfich;
}//0 si ha habido errores, 1 si no

int
main(int argc, char* argv[])
{
	int  N, numfichs;
	char *fichs[20];

	if(argc==1){
		N=-1; // -1 para  comprobarlo luego y que no se use
	}else{
		N = atoi(argv[1]);  //atoi convierte char* a int, si no meto  un numeor me devuelve 0
		if (N==0){
			err(1, "Debe introducir un numero N mayor que 0");
		}
	}

	numfichs=search_txt( fichs);
	
	if (createprocess(N,fichs, numfichs))
		exit(0);
	
	exit(1);
	
}
