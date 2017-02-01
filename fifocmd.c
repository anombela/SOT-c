#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <err.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>


struct Path{ //
	char *paths[32]; //32 paths maximo (y sobra) 
	int numpaths;
};
struct Path path; //almacena las paths y el numero de paths


void
fichsout()
{
	int fdout,fderr;
	fdout = open("fifocmd.out", O_WRONLY|O_CREAT|O_TRUNC, 0660);
	if (fdout < 0) {
		err(1, "fifocmd.out");
	}
	if (dup2(fdout,1)<0) //esta siempre sera la salida del proceso original (padre)
		err(1, "dup2(fdout,1)");
	close(fdout);


	fderr = open("/dev/null", O_WRONLY);
	if (fderr < 0) {
		err(1, "/dev/null");
	}
	if(dup2(fderr,2)<0)
		err(1, "dup2(fderr,2)");
	close(fderr);
}


void
mk_fifo(char *fifo)
{  //mejorar esto

	if(mkfifo(fifo, 0664) < 0){
		if (unlink(fifo) < 0) 
			err(1, "%s",fifo);

		if(mkfifo(fifo, 0664) < 0)
			err(1, "mkfifo");
	}
}

int
tokenize(char *str,char *separator, char *array[]){
	int i = 0;
	char *token;


	token = strtok_r(str,separator,&str);
	while(token!= '\0'){
		array[i] = token;
		i= i + 1;
		token = strtok_r(str,separator,&str);
	}
	return i;
}//devuelve 1

char*
troceapath(){

	char *PATH,*aux;
	
	PATH = getenv ("PATH");
	if (PATH==NULL)
		err(1,"Error getenv");
	aux=strdup(PATH);   /// lo copio por que si no modificaria PATH original
	path.numpaths = tokenize(aux,":\n",path.paths);
	
	return aux;

}  //devuelve el número de paths

int
cmd_path(char *cmd,  char *goodcmd){

	int i, cmdok = -1;
	char aux[128];

	if (access(cmd,X_OK)==0){	

		strcpy(goodcmd,cmd);
		cmdok = 1;

	}else{

		for(i = 0 ; i < path.numpaths ; i++){
			sprintf(aux,"%s/%s",path.paths[i],cmd);
			if (access(aux,X_OK)==0){
					strcpy(goodcmd,aux);
					cmdok = 1;
					break;
			}	
		}
	}	
	return cmdok;
} //1 si encuentra path correcto, 0 si no

int
createprocess(char *cmd, char **argv)
{
	int pid,pid2=0,sts, numcmd,i,fd[2];
	int statusps=1;
	char *command[32];  //comando + argumentos <<<<<< 32
	char goodpath[128];

	pipe(fd);
	for (i = 1; i <= 2; i++){ // dos forks 1- comando del fifo, 2 comando de la linea de comandos
		pid = fork();
		switch(pid){
			case -1:

				err(1,"Error en fork.");

			case 0:

				if (i==1){

					close(fd[0]);
					numcmd = tokenize(cmd," \n", command);
					command[numcmd]=NULL; //tiene que ser nula la ultima posicion del array
					if (cmd_path(command[0],goodpath) < 0)
						err(1,"No existe el comando %s", command[0]);
					if (dup2(fd[1],1)<0)
						warn("Error al hacer dup2--1");
					close(fd[1]);	
					execv(goodpath,command);

				}else if (i==2){

					if (dup2(fd[0],0)<0)
						err(1,"Error al hacer dup2--2");
					close(fd[0]);
					if (cmd_path(argv[0],goodpath) < 0)
						err(1,"No existe el comando %s", argv[0]);
					execv(goodpath,argv);
					
				}
				err(1,"Error en execv.%d", i);
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

}//1 si acaba bien, -1 si algun proceso a acabado mal o a fallado wait*/

int
main(int argc, char* argv[])
{
	char buf[1024];
	FILE *fd;
	char *fifo, *cmd, *n;
	int err = 0;


	fichsout();
	if (argc<3){
		fprintf(stderr, "Error_comandos: [/path/fifo]-[comando]-[parámetros]\n");
		exit(1);
	}
	fifo = argv[1];
	mk_fifo(fifo);
	n = troceapath();

	for(;;){

		fd = fopen(fifo, "r" );
		if (fd ==NULL)
			err = 1;

		cmd = fgets(buf, sizeof(buf), fd);
		if (createprocess(cmd,argv+2) <1)  //dos posiciones mas que el argv[0]
			err = 1;
		fclose(fd);
	}
	free(n);
	if (err)
		exit(1);
	exit(0);
}