#ifndef __MYIIC_H
#define __MYIIC_H
#include "sys.h" 
  		   
// IO direction settings
#define SDA_IN()  {GPIOB->MODER &= ~(3 << (7 * 2)); GPIOB->MODER |= 0 << (7 * 2);}  // PB7 input mode
#define SDA_OUT() {GPIOB->MODER &= ~(3 << (7 * 2)); GPIOB->MODER |= 1 << (7 * 2);} // PB7 output mode
// IO operation functions	 
#define IIC_SCL    PBout(6) // SCL
#define IIC_SDA    PBout(7) // SDA	 
#define READ_SDA   PBin(7)  // Input SDA 

// All IIC operation functions
void IIC_Init(void);                // Initialize IIC IO ports				 
void IIC_Start(void);               // Send IIC start signal
void IIC_Stop(void);                // Send IIC stop signal
void IIC_Send_Byte(u8 txd);         // IIC send one byte
u8 IIC_Read_Byte(unsigned char ack);// IIC read one byte
u8 IIC_Wait_Ack(void);              // IIC wait for ACK signal
void IIC_Ack(void);                 // IIC send ACK signal
void IIC_NAck(void);                // IIC do not send ACK signal


void IIC_Write_One_Byte(u8 daddr,u8 addr,u8 data);
u8 IIC_Read_One_Byte(u8 daddr,u8 addr);	  
#endif
















