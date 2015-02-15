#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "apikey.h"

extern char* optarg;
int main(int argc, char **argv){

	char utilisateur[512];
	u_int32_t taille=1000;
	u_int32_t annee=2015;
	int c;
	int error = 0;
	
	strcpy(utilisateur,"User");
	
	while ((c = getopt (argc, argv, "s:u:y:")) != -1)
    switch (c)
    {
	case 's':
		taille = strtol(optarg,NULL,10);
		break;
	case 'y':
		annee = strtol(optarg,NULL,10);
		break;
	case 'u':
		strncpy(utilisateur,optarg,512);
		break;
	default:
		break;
	}
	printf("Utilisateur : %s\n",utilisateur);
	printf("Annee : %d\n",annee);
	printf("Taille : %d\n",taille);

	KeyHeaderStruct header;
	header.version = 1;
	header.annee = annee;
	header.nombre2=4;
	memcpy(&header.data2,"1000",4);
	header.utilStrSize = strlen(utilisateur);
	memcpy(&header.utilStr,utilisateur,header.utilStrSize);
	header.indexMax = taille;
	header.indexCourant = 158;
	header.nbCrypt = 10;

	char nomFichier[512];
	strcpy(nomFichier,utilisateur);
	strcat(nomFichier,".");
	sprintf(&nomFichier[strlen(nomFichier)],"%d",annee-1900);
	FILE *fichier = fopen(nomFichier,"wb");
	
	convertHeaderEndian(&header);
	fwrite(&header,sizeof(header),1,fichier);
	srand(time(NULL));
	u_int32_t i;
	for(i=0;i<taille;i++){
		u_int8_t byte =(u_int8_t)(rand() & 0xFF);
		fwrite(&byte,1,1,fichier);
	}
	fclose(fichier);

	return error;
}
