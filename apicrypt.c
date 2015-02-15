#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "apikey.h"

const char carASCII[65]="0123456789ABCDEFGHIJKLMNPQRSTUVXYZabcdefhijklmpqrstuvwxyz$*+-()!";

int getYear(){
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);
	return tm.tm_year;
}

//Supprime les retours à la ligne d'une chaine de caractères.
int stripNewLines(char* str){
	char* readPtr = str;
	char* writePtr = str;
	u_int32_t length = strlen(str);
	while(readPtr<&str[length]){
		if((*readPtr!='\n') && (*readPtr!='\r'))
			*(writePtr++) = *(readPtr++);
		else
			readPtr++;
	}
	*writePtr = '\0';
}


char* longHexa(u_int32_t num){
	char* result = malloc(9);
	sprintf(result,"%08X",num);
	return result;
}


char* binToASCII(u_int8_t* binary,int binLen){
	char strBegin[] = "#BINASCII1#\n";
	char strEnd[] = "\n#ENDBINASCII1#\n";
	int asciiLen = (binLen*4+8)/3+16+strlen(strBegin)+strlen(strEnd)+1;
	asciiLen += ((binLen*4+8)/3+16)/60;
	char* ascii = malloc(asciiLen);

	u_int8_t* readPtr = binary;
	char* writePtr = ascii;
	int cpt=0;
	u_int32_t temp=0;

	u_int32_t checksum = -1;
	u_int32_t i;
	int32_t cptNewLine = 52; // La première ligne n'a que 54 caractères

	for ( i = 0; i < binLen; ++i )
		checksum ^= binary[i];

	strcpy(writePtr,strBegin);
	writePtr+=strlen(strBegin);
	char* hexaLen = longHexa(binLen);
	strcpy(writePtr,hexaLen);
	writePtr+=8;
    free(hexaLen);

	while(readPtr<&binary[binLen]){
		temp = (temp<<8) + *(readPtr++);
		cpt++;
		if(cpt==3){
			for(cpt=0;cpt<4;cpt++){
				*(writePtr++) = carASCII[temp & 0x3F];
				cptNewLine--;
				temp = temp>>6;
			}
			if(cptNewLine<=0){
				*(writePtr++)='\n';
				cptNewLine=60;
			}
			cpt=0;
			temp=0;
		}
	}
	if(cpt!=0){
		temp = temp<<(8*(3-cpt));
		for(cpt=0;cpt<4;cpt++){
				*(writePtr++) = carASCII[temp & 0x3F];
				temp = temp>>6;
		}
	}

	char* hexaChksum = longHexa(checksum);
	strcpy(writePtr,hexaChksum);
	writePtr+=8;
	free(hexaChksum);
	strcpy(writePtr,strEnd);
	writePtr+=strlen(strEnd);
	*(writePtr)=0;
	return ascii;
}

u_int8_t* ASCIIToBin(char* ascii){
	u_int32_t asciiLen = strlen(ascii);
	if(asciiLen%4!=0)
		return 0;
	
	int binLen = (asciiLen/4)*3;
	
	return ascii;
}

// Retourne une clé de taille 'longueur' à partir de offset incluant le header.
u_int8_t* getKey(char* nomFichierCle,u_int32_t longueur,u_int32_t offset){
	//ouvre le fichier de clé
	FILE* fichierCle = fopen(nomFichierCle,"rb");
   
	// Récupère l'entête de la clé
	KeyHeaderStruct keyHeader;
	fread(&keyHeader,1,sizeof(keyHeader),fichierCle);
	convertHeaderEndian(&keyHeader);
	if(offset<224){
		offset = keyHeader.indexCourant;
	}
	keyHeader.nbCrypt++;
	
	// Alloue la mémoire pour la clé
	u_int8_t* cle = malloc(longueur+sizeof(keyHeader));
	// Copie le header au début
	convertHeaderEndian(&keyHeader);
	memcpy(cle,&keyHeader,sizeof(keyHeader));
	
    // Puis copie la clé
	u_int32_t writePos = sizeof(keyHeader);
	u_int32_t toRead = longueur;

	while(toRead>0){
		u_int32_t max = keyHeader.indexMax - offset;
		u_int32_t block = toRead;
		if(block>max)
			block = max;
		fseek(fichierCle,offset+sizeof(keyHeader),SEEK_SET);
		fread(&cle[writePos],1,block,fichierCle);
		toRead-=block;
		writePos+=block;
		//Si on arrive à la fin de la clé, on revient au début
		if(offset + block == keyHeader.indexMax)
			offset = 0;
	}
	fclose(fichierCle);
	return cle;
}


// Cryptage par XOr
int cryptXOR(u_int8_t* src, u_int8_t* dst, u_int32_t taille){
	int i;
	for(i=0;i<taille;i++){
		dst[i]^=src[i];
	}
}


int crypter(char* nomFichSrc, char* nomFichDst, char* cheminCles, char* utilisateur, char* destinataires ){
	
    // Taille du fichier d'entree
	int tailleSrc;
	int error =0;
	struct stat st;
	
	if(!stat(nomFichSrc, &st))
		tailleSrc = st.st_size;
	else{
		printf("Erreur : Taille du fichier source %s\n",nomFichSrc);
		return 1; // Impossible de trouver la taille du fichier d'entrée.		
	}
	
	
	FILE* fichierSrc = fopen(nomFichSrc,"rb");
	if(!fichierSrc){
		printf("Erreur : Lecture du fichier source impossible %s\n",nomFichSrc);
		return -1;
	}
	u_int8_t* srcBuf = malloc(tailleSrc);
	fread(srcBuf,1,tailleSrc,fichierSrc);
	fclose(fichierSrc);
	
	// Ajout des destinataires
	char dstBegin[]="#DESTBEGIN#";
	char dstEnd[]="#DESTEND#";
	u_int8_t* temp = malloc(tailleSrc+strlen(destinataires)
							+strlen(dstBegin)+strlen(dstEnd)+2);
	
	u_int32_t headlen = strlen(destinataires)+strlen(dstBegin)+strlen(dstEnd)+2;
	memcpy(temp,srcBuf,tailleSrc);
	//Ajout des destinataires
	//u_int32_t offset=strlen(dstBegin);
	strcpy(&temp[tailleSrc],dstBegin);
	strcat(&temp[tailleSrc],destinataires);
	strcat(&temp[tailleSrc],",,");
	strcat(&temp[tailleSrc],dstEnd);
	
	
	tailleSrc+=headlen;
	free(srcBuf);
	srcBuf=temp;
	
	

	// Fichier de clé Master
	char nomFichierCles[512];
	strcpy(nomFichierCles,cheminCles);
	strcat(nomFichierCles,"Clefs/Master.");
	char annee1900[4];
	sprintf(annee1900,"%d",getYear());
	strcat(nomFichierCles,annee1900);
	
	printf("Nom fichier clé Master : %s\n",nomFichierCles);
	
	if(access(nomFichierCles,R_OK)){
		printf("Erreur : Impossible de trouver le fichier %s\n",nomFichierCles);
		return -2;  //Le fichier de cles Master n'existe pas                 
	}
	
	u_int8_t* crypt = getKey(nomFichierCles,tailleSrc,0);
	//printf("%d\n",tailleSrc);
	cryptXOR(srcBuf,&crypt[sizeof(KeyHeaderStruct)],tailleSrc);
	
	// Fichier de clé utilisateur
	strcpy(nomFichierCles,cheminCles);
	strcat(nomFichierCles,"Clefs/");
	strcat(nomFichierCles,utilisateur);
	strcat(nomFichierCles,".");
	strcat(nomFichierCles,annee1900);

	if(access(nomFichierCles,R_OK)){ 
		printf("Erreur : Impossible de trouver le fichier %s\n",nomFichierCles);
		return -3;  //Le fichier de cles Utilisateur n'existe pas                 
	}
	
	printf("Nom fichier clé Utilisateur : %s\n",nomFichierCles);
	tailleSrc+=sizeof(KeyHeaderStruct);
	//printf("%d\n",tailleSrc);
	u_int8_t* crypt2 = getKey(nomFichierCles,tailleSrc,0);
	
	printf("taille header : %d\n", sizeof(KeyHeaderStruct));
	cryptXOR(crypt,&crypt2[sizeof(KeyHeaderStruct)],tailleSrc);
	
	int tailleCrypt = tailleSrc+sizeof(KeyHeaderStruct);
	char* ascii = binToASCII(crypt2,tailleCrypt);
	
	FILE* fichierDst = fopen(nomFichDst,"wb");
	fwrite(ascii,1,strlen(ascii),fichierDst);
	fclose(fichierDst);

	free(ascii);
	free(srcBuf);
	free(crypt);
	free(crypt2);
	return error;
}



extern char* optarg;
int main(int argc, char **argv){

	int c;
	char fichierEntree[512];
	char fichierSortie[512];
	char cheminCles[512];
	char utilisateur[512];
	char destinataires[512];
	
	fichierEntree[0] = 0;
	fichierSortie[0] = 0;
	cheminCles[0] = 0;
	utilisateur[0] = 0;
	destinataires[0] = 0;

	while ((c = getopt (argc, argv, "s:u:k:d:o:")) != -1)
    switch (c)
    {
	case 's':
		strncpy(fichierEntree,optarg,512);
		break;
	case 'u':
		strncpy(utilisateur,optarg,512);
		break;
	case 'k':
		strncpy(cheminCles,optarg,512);
		break;
	case 'd':
		strncpy(destinataires,optarg,512);
		break;
	case 'o':
		strncpy(fichierSortie,optarg,512);
		break;
	default:
		break;
	}
	printf("Fichier d'entrée : %s\n",fichierEntree);
	printf("Fichier de sortie : %s\n",fichierSortie);
	printf("Utilisateur : %s\n",utilisateur);
	printf("Destinataires : %s\n",destinataires);
	printf("Chemin clés : %s\n",cheminCles);

	int error = 0;

	if(access(fichierEntree,R_OK)){
		printf("Erreur : Lecture du fichier source impossible %s\n",fichierEntree);
		return -1;                  //Le fichier d'entrée n'existe pas
	}
		
	error=crypter(fichierEntree,fichierSortie,cheminCles,utilisateur,destinataires);

	return error;
}
