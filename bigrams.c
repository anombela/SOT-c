#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <err.h>
#include <string.h>
#include <pthread.h>
#include <sys/mman.h>


typedef struct Bigram Bigram;
struct Bigram {
	unsigned char *bigrams;
	pthread_mutex_t lock;
	char **fichs; //tendra todos los nombres de ficheros
};

int
iferror(int argc, char **argv)
{
	
	if (argc < 2){
		fprintf(stderr, "Introducir nombre de pixmap y algún fichero\n");
		exit(1);
	}else if (strcmp(argv[1],"-p")==0){

		if (argc==2){
			fprintf(stderr, "Introducir nombre de pixmap y algún fichero\n");
			exit(1);
		}else if (argc == 3){
			fprintf(stderr, "Introducir agún fichero\n");
			
		}
		return 1;

	}else if (argc ==2){
		fprintf(stderr, "Introducir agún fichero\n");
		
	}
	return 0;
} // algunos no se acaban por si uso pipes

void 
read_bigrams(Bigram *bgm)
{
	FILE *in;
	char buf[1];
	size_t nr;
	unsigned char primero, segundo;
	int i,v,pasar=0;
	char *fich;

	fich=*bgm->fichs;
	bgm->fichs++;

	in = fopen(fich, "r");
	if(in == NULL){
		err(1, "fopen %s", fich);
	}
	for(i=0;;i++){
		nr = fread(buf, sizeof buf, 1, in);
		if(nr == 0){
			if(ferror(in)) {
				err(1, "read");
			}
			break;
		}
		segundo = *buf;
		if (pasar){  //cuando hay algun  valor desconocido tiene que pasar  cuando es primero como segundo y que no se forme un bigrama mal
			
			pasar =0;
				
		}else if (i!=0 ){

			if ( primero >128 || segundo > 128){ //ocurre cuando encuentra algún caracter raro ((en el quijote pasa))
			
				pasar = 1;

			}else{

				v = (128*primero +segundo);
				if (bgm->bigrams[v]<255)
					bgm->bigrams[v] = bgm->bigrams[v]+1;
			}
		}
		primero = segundo;
	}
	fclose(in);
}

static void*
tmain(void *a)
{
	Bigram *bg2;

	bg2 = a;
	pthread_mutex_lock(&bg2->lock);
	read_bigrams(bg2);
	pthread_mutex_unlock(&bg2->lock);
	
	return NULL;
}

void* 
createpixmap(char *fich)
{
	int fd;
	void *addr;

	fd = creat(fich, 0644);
	if(fd < 0) {
		err(1, "create ");
	}
	if (lseek(fd, 128*128-1, 0)<0) //se mueven  64 * 1024 bytes desde 0  en fd
		err(1,"error lseek");
	if(write(fd, " ", 1) != 1) { //aqui esta escribirnedo algo (un espacio) en la ultima posicion, donde se quedo antes
		err(1, "write");
	}
	close(fd);
	fd = open(fich, O_RDWR); //abre el fichero para leer y escribir
	if(fd < 0) {
		err(1, "open");
	}
	addr =  mmap(NULL, 128*128, PROT_READ|PROT_WRITE, MAP_FILE|MAP_SHARED, fd, 0);
	if(addr == MAP_FAILED) {
		err(1, "mmap");
	}
	close(fd);

	return addr;
}


void
createthreads(int argc, Bigram bg, char **argv)
{

	int i;
	pthread_t thr[argc-2];
	void *sts[argc-2];


	if(pthread_mutex_init(&bg.lock, NULL) != 0) {
		err(1, "mutex");
	}
	bg.fichs = &argv[2]; //paso la direcion de argv 2 para tener todos los ficheros
	for(i = 0; i < argc-2; i++) {
		if(pthread_create(thr+i, NULL, tmain, &bg) != 0) {
			err(1, "thread");
		}
	}

	for(i = 0; i < argc-2; i++) {
		pthread_join(thr[i], sts+i);
		free(sts[i]);
	}
}

void
printpixmap(unsigned char *bigrams){

	int i ,j ;
	int value;

	for ( i = 0; i < 128; i++)
	{
		for (j = 0; j < 128; j++)
		{
			value=bigrams[128*i+j];
			printf("(%d,%d): %d\n",i,j,value );
		}
	}
}


int
main(int argc, char* argv[])
{
	Bigram bg;
	void *addr;
	int print = 0;

	print = iferror(argc,argv);

	if (print){
		argv++;  //avanza uno el argv para luego no ocntar el -p
		argc--;
	}

	addr = createpixmap(argv[1]);
	bg.bigrams = addr;
	memset(bg.bigrams,0,128*128);

	
	createthreads(argc,bg,argv);

	if (print)
		printpixmap(bg.bigrams);

	if(munmap(addr, 128*128) < 0){
		err(1, "munmap");
	}
	
	exit(0);
}
