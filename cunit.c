/*
	gcc -c -Wall -Wshadow -g cunit.c & gcc -o cunit cunit.o

*/
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <err.h>
#include <sys/types.h>
#include <sys/dir.h>
#include <string.h>
#include <unistd.h>
#include <err.h>
#include <fcntl.h>
#include <signal.h>  //en mac no hace falta
#include <sys/wait.h>  //en mac no hace falta


struct Path{ //
	char *paths[32]; //32 paths maximo (y sobra) 
	int numpaths;
};
struct Path path; //almacena las paths y el numero de paths
char *globalfich;
int salida; //salida standar
char fichs[32][32]; //32 ficheros maximo de longitud nombre 32 maximo
// fichs se usa  siempre que tengamos que buscar ficheros


void 
ventorno(char **command, int numcmd)
{
	int i;

	for(i = 0 ; i < numcmd ; i++){

		if (*command[i] == '$'){
			command[i]++;
    		if(getenv(command[i]) == NULL){
    			fprintf(stderr,"no existe variable de entorno '%s'\n",command[i]); 
    			exit(1);//deberia acabar aqi?????
    		}else{
    			command[i] = getenv(command[i]);  //asigna el valor a la variable
    		}
    	}
    }
} //si una variable de entorno no existe el programa acabara con estatus 1

int
tokenize(char *str,char *separator, char *array[])
{
	int i = 0;
	char *token;

	token = strtok_r(str,separator,&str);
	while(token!= '\0'){
		array[i] = token;
		i = i + 1;
		token = strtok_r(str,separator,&str);
	}
	return i;
}//devuelve numero

char* 
troceapath()
{
	char *PATH,*aux;
	
	PATH = getenv ("PATH");
	if (PATH == NULL)
		err(1,"Error getenv");
	aux=strdup(PATH);   /// lo copio por que si no modificaria PATH original
	path.numpaths = tokenize(aux,":\n",path.paths);
	
	return aux;

}  //devuelve la copia para cuando se deje de usarla hacer free de ella

int 
cmd_path(char *cmd,  char *goodcmd)
{
	int i, cmdok = -1;
	char aux[128];

	if (access(cmd,X_OK) == 0){	

		strcpy(goodcmd,cmd);
		cmdok = 1;

	}else{

		for(i = 0 ; i < path.numpaths ; i++){
			sprintf(aux,"%s/%s",path.paths[i],cmd);
			if (access(aux,X_OK) == 0){
					strcpy(goodcmd,aux);
					cmdok = 1;
					break;
			}	
		}
	}	
	return cmdok;
} //1 si encuentra path correcto, -1 si no

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
search_file( char *name)
{
	DIR *d;
	struct dirent *dp;
	char *cadena;
	char *ext = name;
	int nfich=0;

	d = opendir(".");
	if(d == NULL) {
		err(1, "Error al abrir directorio actual");
	}
	while( (dp = readdir(d)) != NULL ) {

		if (isfile(dp->d_name)){

			cadena = strrchr( dp->d_name,'.');  //coge a partir de el ultimo punto
			if((cadena != NULL) && (strcmp(cadena,ext)==0)){

				//fichs[nfich] = dp->d_name; //no es necesrio strcpy, no se modificara la dir
				strcpy(fichs[nfich],dp->d_name);
				nfich++;
			}
		}
	}
	closedir(d);
	return nfich;
}//devuelve numero de ficheros

int
iscd(char *cmdzero)
{
	char *command[32];  //comando + argumentos <<<<<< 32
	char *dir;
	int n;

	n=tokenize(cmdzero," ", command);
	if ( n != 0 && strcmp(command[0],"cd") == 0){
		
		if (n>1){

			dir = command[1];
			
		}else{

			dir=getenv("HOME");
			if (dir == NULL){

				fprintf(stderr, "Error al cambiar de directorio\n");
				return -1;
			}
		}	
		if (chdir(dir) == -1){  

			fprintf(stderr,"cd: %s: No existe el directorio\n",dir);  //no acaba, solo no cambia
			return 0;

		}else{

			fprintf(stderr,"Cambia directorio a: %s\n", dir);
		}
		return 1;

	}else{
		return 0;
	}
}//devuelve 0 si no cambia, 1 si si, -1 si error

int     //si condtype es true son los ficheros .cond
runcmd_tst(char commands[10][100], int numcmds, int condtype)
{	
	int c,numcmd,sts,pid,pid2=0,pid_ultimo,is;
	int fd[numcmds-1][2], cambiadir=0;
	char *command[32];  //comando + argumentos <<<<<< 32
	char goodpath[128],*diractual;
	char aux[100];
	int condok=-1;

	diractual = getenv("PWD");
	if (diractual == NULL){
		fprintf(stderr, "fallo getenv('PWD')");
		return -1;
	}
	strcpy(aux,commands[0]);
	is = iscd(aux);
	if(is == 1){
		commands++;
		numcmds--;
		cambiadir=1;
	}else if(is == -1){
		return -1;
	}
	if (!condtype){
		for (c = 0 ; c < numcmds ; c++){

			pipe(fd[c]);   //crea los pipes necesarios
		}
	}
	for ( c = 0 ; c < numcmds ; c++){

		pid=fork();
		if (!condtype){
			if (c == numcmds-1) pid_ultimo=pid;
		}
		switch(pid){

			case -1:

				warn("error fork");
				return -1;

			case 0:

				if (!condtype){
					if ( c != 0){
						if (dup2(fd[c-1][0],0) < 0){ // entrada estandar el pipe anterior
							err(1,"dup2(fd[c-1][0],0");
						}
					}
					close(fd[c-1][0]);

					if (c != numcmds-1){
						if (dup2(fd[c][1], 1) < 0){// salida estandar rl pipe actual menos el final
							err(1,"dup2(fd[c][1], 1)");
						}
						close(fd[c][1]);
					}else{
						close (fd[c][0]);  //cierra ya el ultimo
					}
				}
				numcmd = tokenize(commands[c]," ", command);
				ventorno(command,numcmd); //comprueba si hay variables de entorno
				command[numcmd] = NULL; //tiene que ser nula la ultima posicion del array
				if (cmd_path(command[0],goodpath) < 0)
					err(1,"No existe el comando %s", command[0]);
				execv(goodpath,command);
				err(1, "execv:-%s-",commands[c]);

			default:
				if (!condtype){
					close(fd[c][1]);
				}else{
					if (wait(&sts) == -1)
						err(1,"error al hacer wait .cond");
					if (WIFEXITED(sts)) {
      					if (WEXITSTATUS(sts)){
      						warn("ha acabado mal un proceso de .cond");  //cuando ha acabado mal un proceso
      					}else{
    						condok = 1; //hasta que haya un acierto 
    						//no acabo porque dejaria zombis, cierro los demas procesos
    					}
    				}
    			}
		}		
	}
	if (!condtype){
		while(pid_ultimo != pid2){ //al ultimo pid  espero
			pid2 = wait(&sts);
			if (pid2 == -1){
				warn("error al hacer wait .tst");
				return -1;
			}
			if (WIFEXITED(sts)) {
      			if (WEXITSTATUS(sts)){
      				warn("ha acabado mal un proceso de .tst");  //cuando ha acabado mal un proceso
      				return -1; //deberia acabar ?
      			}
    		}
		}
		for (c = 0 ; c < numcmds ; c++){
			close(fd[c][0]);
		}
	}
	if (cambiadir){
		if (chdir(diractual) == -1){  
			fprintf(stderr,"Error al reestablecer directorio actual\n");
			return -1;
		}
	}
	if (condtype)
		return condok;
	return 1;
} //1 si bien ,-1 si mal

void
copyfile(int in, int out)
{
	char buffer[8*1024]; //funciona con un buffer muy pequeño
	int nr,nw;

	for(;;){

		memset(buffer,0,sizeof(buffer)); //asi inicializa la variable buffer
		nr = read(in, &buffer, sizeof(buffer));
		if(nr == 0){
			break;
		}
		if(nr < 0){
			err(1,"Error de lectura de fichero");
		}
		nw = write(out, buffer, nr);// escribe en fichero de salida
		if (nw != nr){
			err(1,"Error de escritura en fichero");
		}
	}
}

int
runtest(char *fileout)
{ 
	char fileok[strlen(fileout)-1]; //.out -1 = .ok
	char *aux, fileaux[32];
	int nrok,nrout,ok,out,testok=7;
	char bufferok[8*1024],bufferout[8*1024]; //deben ser iguales

	out = open(fileout, O_RDONLY);
	if (out < 0)
		err(1,"error al abrir fichero");

	strcpy(fileaux, fileout);
	aux = strrchr(fileaux,'.');
	*aux = '\0';
	aux = fileaux;

	if (aux == NULL){
		err(1,"No esxiste token .tst");
	}
	if (snprintf(fileok,(strlen(aux)+5),"%s.ok",aux) < 0){ //concatena con .out
		err(1, "Error snprintfp");
	}
	ok = open(fileok, O_RDONLY);
	if (ok < 0){
		ok = creat(fileok, 0660);
		if (ok < 0) 
			err(1, "%s\n",fileok);
		copyfile(out, ok);   //copia lo del .out al .ok
		close(ok);
		close(out);
		return -1;
	}

	for(;;){

		memset(bufferout,0,sizeof(bufferout)); //asi inicializa la variable buffer
		memset(bufferok,0,sizeof(bufferok)); //asi inicializa la variable buffer
		
		nrout = read(out, &bufferout, sizeof(bufferout));
		if(nrout < 0)
			err(1,"Error de lectura de fichero .out\n");

		nrok = read(ok, &bufferok, sizeof(bufferok));
		if(nrok < 0)
			err(1,"Error de lectura de fichero .ok\n");

		if (memcmp(bufferout,bufferok,sizeof(bufferout))==0){ 
			testok=1;
		}else{
			testok=0;
			break;
		}

		if(nrok == 0 && nrout == 0){  //si los dos son igules a 0 es que han acabado iguales
			testok = 1;
			break;
		}
		if(nrok == 0 || nrout == 0){  // si soo uno de los dos es igual a 0 es que son distintos
			break;
			testok = 0;
		}
	}
	close(ok);
	close(out);
	return testok;
}

void 
fileout(char *fichout)
{
	int out;
	
	out = open(fichout, O_WRONLY|O_CREAT|O_TRUNC, 0660);
	if (out < 0)
		err(1, "open %s",fichout);
	if (dup2(out,1) < 0){
		err(1,"error dup2(out,1)");
	}
	if (dup2(out,2) < 0){
		err(1,"error dup2(out,2)");
	}
	close(out);  //2 salidas al ficheo
}// acaba si hay errores

int
readfich(char *fich)
{
	char buf[1024];
	FILE *fd;
	char commands[10][100];  //10 comandos como maximo, 100  caracteres cada uno maximo
	int numcmds=0,c;
	char fichout[strlen(fich)]; //.out == .tst
	char *aux, fich2[32];
	int testok,condtype=0,istest=1;
	char *cadena;

	fd = fopen(fich, "r" );
	if (fd == NULL)
		err(1, "Error al abrir fichero: %s",fich);

	strcpy(fich2,fich);
	aux = strrchr(fich2,'.');
	*aux = '\0';
	aux = fich2;

	if (aux == NULL){
		err(1,"No esxiste token .tst");
	}
	if (snprintf(fichout,(strlen(aux)+5),"%s.out",aux) < 0){ //concatena con .out
		err(1,"error snprintf");
	}
	fileout(fichout);

	while(fgets(buf, sizeof(buf), fd) != NULL){
		if (*buf == '\n') 
			continue;

		c = strlen(buf);
		if (buf[c-1] == '\n')
			buf[c-1] = '\0';   //quito el fin de linea  que  introduce fgets

		strcpy(commands[numcmds],buf);
		numcmds++;
		if(numcmds == 10) break;  //que no supere el tamaño maximo del array
	}

	fclose(fd);
	//dependiendo de la extension hara una cosa u otra
	cadena = strrchr( fich,'.');  //coge a partir de el ultimo punto
	if(cadena != NULL){

		if (strcmp(cadena,".cond")==0){

			condtype = 1;
		}
		istest = runcmd_tst(commands,numcmds,condtype);
		if (istest < 0)
			return 0; //retornara test fallido
	}

	testok=runtest(fichout);
	return testok;
} //devuelve el estado del test

static void
tout(int no)
{
	if (dup2(salida,1) < 0){
		fprintf(stderr,"error dup2(salida,1)\n");
		exit(1);
	}
	printf( "\t%s: time out.\n",globalfich);
	exit(1);
}

int
rm_ok_out()
{
	int numfichs,i,n;

	char *ext[2] = {".out",".ok"};
	printf("\t--FICHEROS ELIMINADOS--\n");
	for (n = 0; n < 2; n++)
	{
		memset(fichs,0,sizeof(fichs));  //resetea por si habia algo
		numfichs=search_file(ext[n]);

		for (i = 0; i < numfichs; i++)
		{
			printf("\t\t%s\n", fichs[i]);
			if(unlink(fichs[i])<0){
				warn("Error a hacer unlink: %s",fichs[i]);
				return -1;
			}
		}
		numfichs=0;
	}
	return 0;
}//-1 si error  , 0 si bien

int
run(int numfichs, int time)
{
	int sts,i;
	int err = 0;
	int testok;
	char *test;

	for(i=0;i<numfichs;i++){

		globalfich = fichs[i]; //para el timeout
		switch(fork()){

			case -1:

				warn("Error en fork.");
				err=1;

			case 0:

				if (time){
					if (signal(SIGALRM, tout) == SIG_ERR)
						fprintf(stderr, "error alarm\n");
					alarm(time);
				}
				testok=readfich(fichs[i]);
				if (dup2(salida,1) < 0){
					fprintf(stderr,"dup2(salida,1)\n");
					exit(1);
				}
				switch(testok){
					case 0:
						test = "fallido";
						break;
					case 1:
						test = "correcto";
						break;
					case -1:
						test = "creado .ok";
						break;
					default:
						test = "ha ocurrido un error";
				}
				printf("\ttest %s: %s\n",fichs[i],test);
				exit(0);
		}
	}
	for(i=0;i<numfichs;i++){
		wait(&sts);
		if (WIFEXITED(sts)) {
      		if (WEXITSTATUS(sts)){
      			warn("ha acabado mal un proceso ");
      			err = 1;  //cuando ha acabado mal un proceso
      		}
    	}
	}
	return err;
}//devuelve si hay error

int 
test(int time)
{
	int numfichs;
	int err = 0, i,fd;
	char *ext[2] = {".tst",".cond"};

	fd = open("/dev/null", O_RDONLY);
	if (fd < 0) {
		fprintf(stderr, "/dev/null\n");
		return 1;
	}
	if (dup2(fd,0) < 0){
		fprintf(stderr,"dup2(fd,0)\n");
		return 1;
	}
	close(fd);
	for (i = 0; i < 2; i++){  //para hacer las dos pruebas

		printf("---RESULTADOS TESTS-->'%s'---\n",ext[i]);
		memset(fichs,0,sizeof(fichs));
		numfichs=search_file(ext[i]);
		if (numfichs == 0){

			fprintf(stderr, "-No encontrados test: --%s\n", ext[i]);

		}else{
			err = run(numfichs,time);
		}
		printf("\n");
	}
	return err;
}//devuelve  si hay error

int
main(int argc, char* argv[])
{
	char *n;  //mas de 20????
	int time=0, errn;

	salida = dup(1);
	if(salida < 0)
		err(1,"Error al hacer dup(1)\n");
	if (argc > 1){
		if (strcmp(argv[1],"-t") == 0){
			if (argc == 2){
				fprintf(stderr,"Introducir un tiempo\n"); 
				exit(1);
			}
			time = atoi(argv[2]);  //si falla devuelve 0 siempre(no afecta)
			
		}else if (strcmp(argv[1],"-c") == 0){
			errn = rm_ok_out();
			if (errn)
				exit(0);
			exit(0);
		}
	}

	n = troceapath(); //libero mas tarde
	errn = test(time);  //opcional 3
	free(n);

	if (errn)
		exit(1);
	exit(0);
}