#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <err.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>


struct infoltr{
	char letra;
	int apariciones;
	int nprimero;
	int nultimo;
};
typedef struct infoltr infoltr;

int total_letras=0;
int primero = 1;
int iguales =1 ; //mayusculas y minusculas iguales al inicio 
int lengtharray = 26;   //cuando  igualles == true se dobla
int digit = 0;

void
createabc(infoltr *letras){

	char i;
	int j;
	infoltr infoletra;

	if (iguales){
		i='a';
	}else{
		i='A';
	}
	j=0;
	for (; i <= 'z'; i++)
		//si no inicializo a cero dara error
	{
		if (!isalpha(i)) 
					continue;

		infoletra.letra = i;
		infoletra.apariciones=0;
		infoletra.nprimero=0;
		infoletra.nultimo=0;
		letras[j]= infoletra;
		j++;
	}	
}

int
analyze (char *buffer, int nr ,infoltr *letras )
{
	int i,j,posicion;
	char buff;

	for (j=0 ; j<nr ; j++){
		if (isalpha(*buffer)){ //solo se ejecutara si es alfanumerico
			
			if (iguales) {
				buff = tolower(*buffer);
			}else{
				buff = *buffer;
			}
			for (i=0;i<lengtharray;i++){

				if (letras[i].letra == buff){
					letras[i].apariciones++;
					total_letras++;
					if (primero && !digit){
					 	letras[i].nprimero++;
					 	
					}
					
					primero=0;

					posicion = i;
					break;
				}
			}
			digit = 0;
			
		}else if (isdigit(*buffer)){
			digit = 1;

		}else{

			if (primero==0 && digit==0){ //primer no alfanumerico despues de alfanumerico, el anterior alfanumerico se considera ultimo
				letras[i].nultimo++;
			}
			primero=1;
			digit=0;		
		}
		buffer++;
	}
	return posicion; //devuelve la posicion de la ultima letra leida (para cuando acaba en fichero en alfanumerico)
}

int
readin (int fd, infoltr *letras)	
{
	int nr,pos;
	char buffer[1024*8];

	for(;;){
		
		memset(buffer,0,sizeof(buffer)); //asi inicializa la variable buffer
		nr = read(fd, &buffer, sizeof(buffer));
		if(nr == 0){
			break;
		}
		if(nr < 0){
			return -1; //-1 cuando es error de lectura
		}


		pos=analyze(buffer,nr,letras);	
	}
	if (primero==0 && !digit)
				letras[pos].nultimo++;

	primero = 1; //se resetea para otros ficheros
	if (fd !=0) close(fd);
	return 1;
}

void 
readdata(int argc, char **argv, infoltr *letras){

	int fd,f,i=1;

	if (argc !=1 && strcmp(argv[1],"-i")==0){
		iguales=0;
		i++;
		lengtharray= 2*lengtharray;
	} 		
	createabc(letras);	

	if (argc==1 || (strcmp(argv[1],"-i")==0 && argc==2)){

		printf("Pulsar ctrl+d para dejar de leer de la entrada estandar\n");
		f = readin(0,letras);
		if (f<0){
			fprintf(stderr, "Error de lectura de entrada estandar\n");
		}

	}else{

		for (; i < argc; i++)
		{
			fd = open(argv[i], O_RDONLY);
				if(fd < 0){
					warn("%s", argv[i]);; // si devuelve 0 es que el fichero no existe
					continue;  // si un fichero no existe el programa no acaba, funcionara con los demas
				}
			f = readin(fd,letras);
			if (f<0){
				fprintf(stderr, "Error al leer del fichero: %s\n", argv[i]);
				continue;  //si lectura de fichero falla, el programa sigue leyendo otros ficheros
			}
		}	
	}
}

void
printletras (infoltr *letras)
{
	int i;
	float porcentaje;
	for(i=0;i<lengtharray;i++){
		if (letras[i].apariciones!=0){
			porcentaje=(letras[i].apariciones/ (float) total_letras)*100;
			printf("%c %.2f%% ", letras[i].letra,porcentaje);
			printf("%d %d\n", letras[i].nprimero, letras[i].nultimo);
		}
	}
}

int 
main(int argc, char *argv[])
{
	infoltr letras[52];  //dos veces el abecedario
			
	readdata(argc,argv,letras);

	printletras(letras);

	exit(0);
}

//son 26 X 2 palabras (la ñ no la considera)
//isalpha devuelve 0 si no lo es, isdigit igual
//tolower convierte mayusculas en minusculas
//%.2f para escribir un float de solo 2 decimales

