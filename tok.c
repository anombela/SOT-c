#include <stdlib.h>
#include <stdio.h>

int mytokenize(char *str, char **args, int maxargs)
{
	char *aux;
	aux= str;
	int longi=0;
	while (*str!='\0'){
			
		if (*str=='\t' || *str=='\r' || *str==' ' || *str=='\n'){
			
			*str='\0';
		}
		
		longi++;
		str++;	
	}
	
	str=aux;
	int N_args = 0;
	int n_caracters=0;
	int caract=1;
	while (n_caracters < longi){  /*longi cuenta e \0 */

	 	if(*str!='\0' && caract && N_args<maxargs){
			args[N_args] = str;
			//printf("%s\n", args[N_args]);
			caract=0;
			N_args++;

		}else if( *str=='\0'){

			caract=1;
		}
		n_caracters++;
		str++;	
	}
	
	return N_args;
}

int main()
{
	int i, Max_Arguments = 40;
	char cadena[] = "hola hola 	\r\n	dsfsfs  	 dfswefw dfg dr g er er ye fsf	";
	char *array[Max_Arguments];
	
	int s = mytokenize(cadena,array,Max_Arguments);

	for (i = 0; i < s; i++)
	{
		printf("%s\n", array[i]);
	}
	printf("Número de subcadenas: %d\n\n", s);

	exit(EXIT_SUCCESS); 
} 




