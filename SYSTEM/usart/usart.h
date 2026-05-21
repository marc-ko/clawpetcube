#ifndef __USART_H
#define __USART_H
#include "stdio.h"	
#include "stm32f4xx_conf.h"
#include "sys.h" 

#define USART_REC_LEN  			200  	//Max bytes 200
#define EN_USART1_RX 			1		// Enable USART1 rec
	  	
extern u8  USART_RX_BUF[USART_REC_LEN]; //buff of rx, max USART_REC_LEN. last byte to do next line 
extern u16 USART_RX_STA;         		//status of rx 	
//if u wanna disconnet the rx please comment out the define under there
void uart_init(u32 bound);
#endif


