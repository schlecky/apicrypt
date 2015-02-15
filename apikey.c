#include "apikey.h"


void swapLongEndian(u_int32_t *num){
	u_int8_t *buff = (u_int8_t *)num;
	u_int8_t temp = buff[0];
	buff[0] = buff[3];
	buff[3] = temp;
	temp = buff[1];
	buff[1] = buff[2];
	buff[2] = temp;
}



void convertHeaderEndian(KeyHeaderStruct* header){
	swapLongEndian((u_int32_t*)&header->version);
	swapLongEndian((u_int32_t*)&header->annee);
	swapLongEndian((u_int32_t*)&header->nombre2);
	swapLongEndian((u_int32_t*)&header->utilStrSize);
	swapLongEndian((u_int32_t*)&header->indexMax);
	swapLongEndian((u_int32_t*)&header->indexCourant);
	swapLongEndian((u_int32_t*)&header->nbCrypt);
}
