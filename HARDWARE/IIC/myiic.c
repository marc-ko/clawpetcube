#include "myiic.h"
#include "delay.h"

void IIC_Init(void)
{			
  GPIO_InitTypeDef  GPIO_InitStructure;

  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

  GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_6 | GPIO_Pin_7;
  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;			
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;			// Push-pull output
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;	// 100MHz
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;				// Pull-up
  GPIO_Init(GPIOB, &GPIO_InitStructure);							// Initialize
	IIC_SCL=1;
	IIC_SDA=1;
}

//Generate IIC start signal
void IIC_Start(void)
{
	SDA_OUT();    // sda set output
	IIC_SDA=1;	  	  
	IIC_SCL=1;
	delay_us(4);
 	IIC_SDA=0;		// START:when CLK is high,DATA change form high to low 
	delay_us(4);
	IIC_SCL=0;		// // Hold the I2C bus, ready to send or receive data 
}	  
// Generate IIC stop signal
void IIC_Stop(void)
{
    SDA_OUT(); // Set SDA as output
    IIC_SCL = 0;
    IIC_SDA = 0; // STOP: when CLK is high, DATA changes from low to high
     delay_us(4);
    IIC_SCL = 1; 
    IIC_SDA = 1; // Send I2C bus end signal
    delay_us(4);							   	
}
// Wait for the acknowledgment signal to arrive
// Return value: 1, receive acknowledgment failed
//               0, receive acknowledgment succeeded
u8 IIC_Wait_Ack(void)
{
    u8 ucErrTime = 0;
    SDA_IN();      // Set SDA as input  
    IIC_SDA = 1; delay_us(1);	   
    IIC_SCL = 1; delay_us(1);	 
    while (READ_SDA)
    {
        ucErrTime++;
        if (ucErrTime > 250)
        {
            IIC_Stop();
            return 1;
        }
    }
    IIC_SCL = 0; // Clock output 0	   
    return 0;  
} 
// Generate ACK acknowledgment
void IIC_Ack(void)
{
    IIC_SCL = 0;
    SDA_OUT();
    IIC_SDA = 0;
    delay_us(2);
    IIC_SCL = 1;
    delay_us(2);
    IIC_SCL = 0;
}
// Do not generate ACK acknowledgment		    
void IIC_NAck(void)
{
    IIC_SCL = 0;
    SDA_OUT();
    IIC_SDA = 1;
    delay_us(2);
    IIC_SCL = 1;
    delay_us(2);
    IIC_SCL = 0;
}					 				     
// Send one byte via IIC
// Return whether the slave has acknowledged
// 1, acknowledgment received
// 0, no acknowledgment received			  
void IIC_Send_Byte(u8 txd)
{                        
    u8 t;   
    SDA_OUT(); 	    
    IIC_SCL = 0; // Pull down the clock to start data transmission
    for (t = 0; t < 8; t++)
    {              
        IIC_SDA = (txd & 0x80) >> 7;
        txd <<= 1; 	  
        delay_us(2);   // These three delays are necessary for TEA5767
        IIC_SCL = 1;
        delay_us(2); 
        IIC_SCL = 0;	
        delay_us(2);
    }	 
} 	    
// Read one byte, send ACK if ack = 1, send nACK if ack = 0   
u8 IIC_Read_Byte(unsigned char ack)
{
    unsigned char i, receive = 0;
    SDA_IN(); // Set SDA as input
    for (i = 0; i < 8; i++)
    {
        IIC_SCL = 0; 
        delay_us(2);
        IIC_SCL = 1;
        receive <<= 1;
        if (READ_SDA) receive++;   
        delay_us(1); 
    }					 
    if (!ack)
        IIC_NAck(); // Send nACK
    else
        IIC_Ack(); // Send ACK   
    return receive;
}