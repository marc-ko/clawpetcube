#include "lcd_dma.h"
#include "delay.h"
#include "lcd_init.h"
#include "lvgl.h"

// DMA configuration for each channel
// The transfer mode here is fixed and should be modified according to different situations
// From memory to peripheral mode / 8-bit data width / memory increment mode
// DMA_Streamx: DMA data stream, DMA1_Stream0~7 / DMA2_Stream0~7
// chx: DMA channel selection, @ref DMA_channel DMA_Channel_0~DMA_Channel_7
// par: Peripheral address
// mar: Memory address
// ndtr: Data transfer amount
void LCD_DMA2_Config(u32 par, u32 mar, u16 ndtr)
{
	DMA_InitTypeDef DMA_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE); // Enable DMA2 clock

	DMA_DeInit(DMA2_Stream3);

	while (DMA_GetCmdStatus(DMA2_Stream3) != DISABLE)
	{
	} // Ensure that DMA can be initialized

	/* DMA Stream */
	DMA_InitStructure.DMA_Channel = DMA_Channel_3;							// Channel 3
	DMA_InitStructure.DMA_PeripheralBaseAddr = par;							// DMA peripheral address
	DMA_InitStructure.DMA_Memory0BaseAddr = mar;							// DMA memory address
	DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;					// Memory to peripheral mode
	DMA_InitStructure.DMA_BufferSize = ndtr;								// Data transfer amount
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;		// Peripheral increment mode
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;					// Memory increment mode
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte; // Peripheral data length: 8 bits
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;			// Memory data length: 8 bits
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;							// Normal mode
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;					// Medium priority
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;			// Memory burst single transfer (onebyone)
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single; // Peripheral burst single transfer (onebyone)
	DMA_Init(DMA2_Stream3, &DMA_InitStructure);

	DMA_ITConfig(DMA2_Stream3, DMA_IT_TC, ENABLE); // Enable DMA transfer completion interrupt

	// DMA interrupt configuration
	NVIC_InitStructure.NVIC_IRQChannel = DMA2_Stream3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	DMA_Cmd(DMA2_Stream3, DISABLE); // DISABLE DMA transfer
}

// Start a DMA transfer
// DMA_Streamx: DMA data stream, DMA1_Stream0~7 / DMA2_Stream0~7
// ndtr: Data transfer amount
void LCD_DMA_Enable(DMA_Stream_TypeDef *DMA_Streamx, u32 par, u32 mar, u16 ndtr)
{
	DMA_Cmd(DMA_Streamx, DISABLE); // Disable DMA transfer

	while (DMA_GetCmdStatus(DMA_Streamx) != DISABLE)
	{
	} // Ensure DMA can be configured

	DMA_Streamx->PAR = par;	  // Update peripheral base address
	DMA_Streamx->M0AR = mar;  // Update memory address
	DMA_Streamx->NDTR = ndtr; // Update data transfer amount

	DMA_Cmd(DMA_Streamx, ENABLE); // Enable DMA transfer
}

// DMA refresh interrupt, used to control the LCD screen for the next refresh
extern lv_disp_drv_t disp_drv;
void DMA2_Stream3_IRQHandler(void)
{
	// DMA transfer complete
	if (DMA_GetITStatus(DMA2_Stream3, DMA_IT_TCIF3))
	{
		DMA_Cmd(DMA2_Stream3, DISABLE);					   // Disable DMA transfer
		LCD_CS_Set();									   // Set LCD chip select pin high
		DMA_ClearITPendingBit(DMA2_Stream3, DMA_IT_TCIF3); // Clear DMA transfer complete flag
		lv_disp_flush_ready(&disp_drv);					   // Prepare for the next LCD refresh
	}
}