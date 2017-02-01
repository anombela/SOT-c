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


int
openfich(char* fich)
{
	int fd;

	fd = open(fich, O_RDONLY);
	if(fd < 0){
		warn("Error al abrir el fichero %s", fich);
		return -1;
	}
	return fd;
} //-1 si error, fd si bien

void
fork1(char *fich, int *fd)
{
	int fi;

	close(fd[0]);
	fi = openfich(fich);
	if (fi <0){
		warn("Error al abrir el fichero %s", fich);
	}else{
		if (dup2(fi,0)<0)
			warn("Error al hacer dup2--0");
		close(fi);
	}
	if (dup2(fd[1],1)<0)
		warn("Error al hacer dup2--1");
	close(fd[1]);	
}

int
createprocess( char *fich, char **argv)
{
	int pid,sts,i;
	int pid2=0,statusps=1;//comprueba si ha habido procesos mal acabados
	int fd[2];
	char path[1024] = "/usr/bin/"; //luego concatenará (por eso el espacio de mas)

	char* cmd[] = {"grep", *++argv,NULL}; //muevo a segundo valor el argv
	argv++;  			//avanzo posicion para tener argv desde el comando

	pipe(fd);
	for (i = 1; i <= 2; i++){ // se harán dos forks 1- comando, 2- grep
		pid = fork();
		switch(pid){
			case -1:
		
				err(1,"Error en fork.%d--%s",i,fich);

			case 0:

				if (i==1){ //el trabajo del primer hijor

					fork1(fich, fd);
					execv(strcat(path,*argv),argv) ;

				}else if (i==2){ //el trabajo del segundo hijo

					if (dup2(fd[0],0)<0)
					err(1,"Error al hacer dup2--0");
					close(fd[0]);
					execv("/bin/grep",cmd) ;
				}
				err(1,"Error en execv.%i--%s", i,fich);
		}
 		if (i==1)	close(fd[1]);//cierra despues del primer fork
 		if (i==2)	close(fd[0]); //cierra despues del segundo fork

	} 
	while(pid2 != pid){ //al segundo pid  espero

		pid2 = wait(&sts);
		if (WIFEXITED(sts)) {
      		if (WEXITSTATUS(sts)) statusps=-1;  //cuando ha acabado mal un proceso
    	}
	}
	return statusps; 

}//1 si acaba bien, -1 si algun proceso a acabado mal o a fallado wait

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
search_txt (char **argv)
{
	DIR *d;
	struct dirent *dp;
	int error = 0;

	d = opendir(".");
	if(d == NULL) {
		err(1, "Error al abrir directorio actual");
	}
	while( (dp = readdir(d)) != NULL ) {

		if (isfile(dp->d_name)){

			if (createprocess(dp->d_name, argv)<0)
				error=1; //ha habido error
		}
	}
	closedir(d);
	return error;
}//0 si ha habido errores, numero de ficheros leidos correctamente si no

int
main(int argc, char* argv[])
{

	if (argc<3){
		fprintf(stderr, "Error_comandos: [expresión regular]-[comando]-[parámetros]\n");
		exit(1);
	}

	if (search_txt(argv)) 
		exit(1);   // tambiaen acaba con error si el !!!grep no ha encontrado nada!!!
	exit(0);
	
}
