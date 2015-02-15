#ifndef __apikey_h__
#define __apikey_h__

#include <sys/types.h>

typedef struct{
	u_int32_t version;      //0
	u_int32_t annee;        //4
	u_int32_t nombre2;      //8
	u_int8_t data2[32];
	u_int32_t utilStrSize;  //44
	u_int8_t utilStr[40];
	u_int32_t indexMax;     //88
	u_int32_t indexCourant; //92
	u_int32_t nbCrypt;      //96
	u_int8_t data3[124];
} KeyHeaderStruct;


void swapLongEndian(u_int32_t *num);
void convertHeaderEndian(KeyHeaderStruct* header);



#endif
