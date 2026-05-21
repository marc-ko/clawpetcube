#include "sdio.h"
#include "string.h"
#include "sys.h"
#include "usart.h"
//

//

/*用于sdio初始化的结构体*/
SDIO_InitTypeDef SDIO_InitStructure;
SDIO_CmdInitTypeDef SDIO_CmdInitStructure;
SDIO_DataInitTypeDef SDIO_DataInitStructure;

SD_Error CmdError(void);
SD_Error CmdResp7Error(void);
SD_Error CmdResp1Error(u8 cmd);
SD_Error CmdResp3Error(void);
SD_Error CmdResp2Error(void);
SD_Error CmdResp6Error(u8 cmd, u16 *prca);
SD_Error SDEnWideBus(u8 enx);
SD_Error IsCardProgramming(u8 *pstatus);
SD_Error FindSCR(u16 rca, u32 *pscr);
u8 convert_from_bytes_to_power_of_two(u16 NumberOfBytes);

static u8 CardType = SDIO_STD_CAPACITY_SD_CARD_V2_0; // SD card type (default is 1.x card)
static u32 CSD_Tab[4], CID_Tab[4], RCA = 0;			 // SD card CSD, CID and relative address (RCA) data
static u8 DeviceMode = SD_DMA_MODE;					 // Work mode, note that the work mode must be set through SD_SetDeviceMode, then it is valid. Here, only define a default mode (SD_DMA_MODE)
static u8 StopCondition = 0;						 // Whether to send stop transfer flag, used for DMA multi-block read/write

volatile SD_Error TransferError = SD_OK; // Data transfer error flag, used for DMA read/write
volatile u8 TransferEnd = 0;			 // Transfer end flag, used for DMA read/write
SD_CardInfo SDCardInfo;					 // SD card information

// SD_ReadDisk/SD_WriteDisk function buffer, when the address of the data buffer of these two functions is not 4-byte aligned,
// use this array to ensure that the data buffer address is 4-byte aligned.
__attribute__((aligned(4))) u8 SDIO_DATA_BUFFER[512];

void SDIO_Register_Deinit(void)
{
	SDIO->POWER = 0x00000000;
	SDIO->CLKCR = 0x00000000;
	SDIO->ARG = 0x00000000;
	SDIO->CMD = 0x00000000;
	SDIO->DTIMER = 0x00000000;
	SDIO->DLEN = 0x00000000;
	SDIO->DCTRL = 0x00000000;
	SDIO->ICR = 0x00C007FF;
	SDIO->MASK = 0x00000000;
}

// Initialize SD card
// Return value: error code (0, no error)
SD_Error SD_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	SD_Error errorstatus = SD_OK;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_DMA2, ENABLE); // Enable GPIOC, GPIOD, and DMA2 clocks

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SDIO, ENABLE); // Enable SDIO clock

	RCC_APB2PeriphResetCmd(RCC_APB2Periph_SDIO, ENABLE); // SDIO reset

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12; // PC8,9,10,11,12 multiplexing function output
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;													 // Multiplexing function
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;												 // 100M
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; // Pull up
	GPIO_Init(GPIOC, &GPIO_InitStructure);		 // PC8,9,10,11,12 multiplexing function output

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_Init(GPIOD, &GPIO_InitStructure); // PD2 multiplexing function output

	// Pin multiplexing mapping settings
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource8, GPIO_AF_SDIO); // PC8,AF12
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource9, GPIO_AF_SDIO);
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource10, GPIO_AF_SDIO);
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource11, GPIO_AF_SDIO);
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource12, GPIO_AF_SDIO);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource2, GPIO_AF_SDIO);

	RCC_APB2PeriphResetCmd(RCC_APB2Periph_SDIO, DISABLE); // SDIO结束复位

	// Set SDIO peripheral registers to default values
	SDIO_Register_Deinit();

	NVIC_InitStructure.NVIC_IRQChannel = SDIO_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; // Preemption priority 3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;		  // Subpriority 3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			  // IRQ channel enable
	NVIC_Init(&NVIC_InitStructure);							  // Initialize VIC registers according to specified parameters

	errorstatus = SD_PowerON(); // SD card power on
	if (errorstatus == SD_OK)
	{
		errorstatus = SD_InitializeCards(); // Initialize SD card
	}
	if (errorstatus == SD_OK)
	{
		errorstatus = SD_GetCardInfo(&SDCardInfo); // Get card information
	}
	if (errorstatus == SD_OK)
	{
		errorstatus = SD_SelectDeselect((u32)(SDCardInfo.RCA << 16)); // Select SD card
	}
	if (errorstatus == SD_OK)
	{
		errorstatus = SD_EnableWideBusOperation(SDIO_BusWide_4b); // 4-bit width, if it is MMC card, 4-bit mode cannot be used
	}
	if ((errorstatus == SD_OK) || (SDIO_MULTIMEDIA_CARD == CardType))
	{
		SDIO_Clock_Set(SDIO_TRANSFER_CLK_DIV); // Set clock frequency, SDIO clock calculation formula: SDIO_CK clock = SDIOCLK/[clkdiv+2]; where, SDIOCLK is fixed at 48Mhz
		// errorstatus=SD_SetDeviceMode(SD_DMA_MODE);	//Set to DMA mode
		errorstatus = SD_SetDeviceMode(SD_POLLING_MODE); // Set to polling mode
	}

	return errorstatus;
}
// SDIO clock initialization
// clkdiv: clock division coefficient
// CK clock = SDIOCLK/[clkdiv+2];(SDIOCLK clock is fixed at 48Mhz)
void SDIO_Clock_Set(u8 clkdiv)
{
	u32 tmpreg = SDIO->CLKCR;
	tmpreg &= 0XFFFFFF00;
	tmpreg |= clkdiv;
	SDIO->CLKCR = tmpreg;
}

// Card power on
// Query all SDIO interface card devices, and query their voltage and configure clocks
// Return value: error code (0, no error)
SD_Error SD_PowerON(void)
{
	u8 i = 0;
	SD_Error errorstatus = SD_OK;
	u32 response = 0, count = 0, validvoltage = 0;
	u32 SDType = SD_STD_CAPACITY;

	/*The clock during initialization cannot be greater than 400KHz*/
	SDIO_InitStructure.SDIO_ClockDiv = SDIO_INIT_CLK_DIV; /* HCLK = 72MHz, SDIOCLK = 72MHz, SDIO_CK = HCLK/(178 + 2) = 400 KHz */
	SDIO_InitStructure.SDIO_ClockEdge = SDIO_ClockEdge_Rising;
	SDIO_InitStructure.SDIO_ClockBypass = SDIO_ClockBypass_Disable;					// Do not use bypass mode, directly use HCLK to divide to get SDIO_CK
	SDIO_InitStructure.SDIO_ClockPowerSave = SDIO_ClockPowerSave_Disable;			// Do not turn off clock power when idle
	SDIO_InitStructure.SDIO_BusWide = SDIO_BusWide_1b;								// 1-bit data line
	SDIO_InitStructure.SDIO_HardwareFlowControl = SDIO_HardwareFlowControl_Disable; // Hardware flow control
	SDIO_Init(&SDIO_InitStructure);

	SDIO_SetPowerState(SDIO_PowerState_ON); // Power on, enable card clock
	SDIO->CLKCR |= 1 << 8;					// SDIOCK enable

	for (i = 0; i < 74; i++)
	{
		SDIO_CmdInitStructure.SDIO_Argument = 0x0;					// Send CMD0 to enter IDLE STAGE mode command.
		SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_GO_IDLE_STATE; // cmd0
		SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_No;		// No response
		SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
		SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable; // The CPSM waits for data transfer to end before sending the command.
		SDIO_SendCommand(&SDIO_CmdInitStructure);			// Write command to command register

		errorstatus = CmdError();

		if (errorstatus == SD_OK)
		{
			break;
		}
	}
	if (errorstatus)
		return errorstatus; // Return error status

	SDIO_CmdInitStructure.SDIO_Argument = SD_CHECK_PATTERN;	   // Send CMD8, short response, check SD card interface characteristics
	SDIO_CmdInitStructure.SDIO_CmdIndex = SDIO_SEND_IF_COND;   // cmd8
	SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short; // r7
	SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;			   // Disable wait interrupt
	SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
	SDIO_SendCommand(&SDIO_CmdInitStructure);

	errorstatus = CmdResp7Error(); // Wait for R7 response

	if (errorstatus == SD_OK) // R7 response normal
	{
		CardType = SDIO_STD_CAPACITY_SD_CARD_V2_0; // SD 2.0 card
		SDType = SD_HIGH_CAPACITY;				   // High capacity card
	}

	SDIO_CmdInitStructure.SDIO_Argument = 0x00; // Send CMD55, short response
	SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_APP_CMD;
	SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
	SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
	SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
	SDIO_SendCommand(&SDIO_CmdInitStructure); // Send CMD55, short response

	errorstatus = CmdResp1Error(SD_CMD_APP_CMD); // Wait for R1 response

	if (errorstatus == SD_OK) // SD2.0/SD 1.1,otherwise MMC card
	{
		// SD card, send ACMD41 SD_APP_OP_COND, parameter: 0x80100000
		while ((!validvoltage) && (count < SD_MAX_VOLT_TRIAL))
		{
			SDIO_CmdInitStructure.SDIO_Argument = 0x00;			  // Send CMD55, short response
			SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_APP_CMD; // CMD55
			SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
			SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
			SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
			SDIO_SendCommand(&SDIO_CmdInitStructure); // Send CMD55, short response

			errorstatus = CmdResp1Error(SD_CMD_APP_CMD); // Wait for R1 response

			if (errorstatus != SD_OK)
			{
				return errorstatus; // Response error
			}
			// acmd41, command parameters consist of supported voltage range and HCS bit, HCS bit is used to distinguish between SDSc and sdhc
			SDIO_CmdInitStructure.SDIO_Argument = SD_VOLTAGE_WINDOW_SD | SDType; // Send ACMD41, short response
			SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SD_APP_OP_COND;
			SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short; // r3
			SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
			SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
			SDIO_SendCommand(&SDIO_CmdInitStructure);

			errorstatus = CmdResp3Error(); // Wait for R3 response

			if (errorstatus != SD_OK)
			{
				return errorstatus; // Response error
			}
			response = SDIO->RESP1;
			;												  // Get response
			validvoltage = (((response >> 31) == 1) ? 1 : 0); // Determine if SD card power-on is complete
			count++;
		}
		if (count >= SD_MAX_VOLT_TRIAL)
		{
			errorstatus = SD_INVALID_VOLTRANGE;
			return errorstatus;
		}
		if (response &= SD_HIGH_CAPACITY)
		{
			CardType = SDIO_HIGH_CAPACITY_SD_CARD;
		}
	}
	else // MMC卡
	{
		// MMC card, send CMD1 SDIO_SEND_OP_COND, parameter: 0x80FF8000
		while ((!validvoltage) && (count < SD_MAX_VOLT_TRIAL))
		{
			SDIO_CmdInitStructure.SDIO_Argument = SD_VOLTAGE_WINDOW_MMC; // Send CMD1, short response
			SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SEND_OP_COND;
			SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short; // r3
			SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
			SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
			SDIO_SendCommand(&SDIO_CmdInitStructure);

			errorstatus = CmdResp3Error(); // Wait for R3 response

			if (errorstatus != SD_OK)
			{
				return errorstatus; // Response error
			}
			response = SDIO->RESP1;
			; // Get response
			validvoltage = (((response >> 31) == 1) ? 1 : 0);
			count++;
		}
		if (count >= SD_MAX_VOLT_TRIAL)
		{
			errorstatus = SD_INVALID_VOLTRANGE;
			return errorstatus;
		}
		CardType = SDIO_MULTIMEDIA_CARD;
	}
	return (errorstatus);
}
// SD card Power OFF
// Return value: error code (0, no error)
SD_Error SD_PowerOFF(void)
{

	SDIO_SetPowerState(SDIO_PowerState_OFF); // SDIO电源关闭,时钟停止

	return SD_OK;
}
// Initialize all cards and put them into ready state
// Return value: error code
SD_Error SD_InitializeCards(void)
{
	SD_Error errorstatus = SD_OK;
	u16 rca = 0x01;
	if (SDIO_GetPowerState() == SDIO_PowerState_OFF) // Check power state, ensure it is powered on
	{
		errorstatus = SD_REQUEST_NOT_APPLICABLE;
		return (errorstatus);
	}
	if (SDIO_SECURE_DIGITAL_IO_CARD != CardType) // Not SECURE_DIGITAL_IO_CARD
	{
		SDIO_CmdInitStructure.SDIO_Argument = 0x0; // Send CMD2, get CID, long response
		SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_ALL_SEND_CID;
		SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Long;
		SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
		SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
		SDIO_SendCommand(&SDIO_CmdInitStructure); // Send CMD2, get CID, long response

		errorstatus = CmdResp2Error(); // Wait for R2 response

		if (errorstatus != SD_OK)
		{
			return errorstatus; // Response error
		}
		CID_Tab[0] = SDIO->RESP1;
		CID_Tab[1] = SDIO->RESP2;
		CID_Tab[2] = SDIO->RESP3;
		CID_Tab[3] = SDIO->RESP4;
	}
	if ((SDIO_STD_CAPACITY_SD_CARD_V1_1 == CardType) || (SDIO_STD_CAPACITY_SD_CARD_V2_0 == CardType) || (SDIO_SECURE_DIGITAL_IO_COMBO_CARD == CardType) || (SDIO_HIGH_CAPACITY_SD_CARD == CardType)) // 判断卡类型
	{
		SDIO_CmdInitStructure.SDIO_Argument = 0x00;				   // Send CMD3, short response
		SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SET_REL_ADDR; // cmd3
		SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short; // r6
		SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
		SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
		SDIO_SendCommand(&SDIO_CmdInitStructure); // Send CMD3, short response

		errorstatus = CmdResp6Error(SD_CMD_SET_REL_ADDR, &rca); // Wait for R6 response

		if (errorstatus != SD_OK)
		{
			return errorstatus; // Response error
		}
	}
	if (SDIO_MULTIMEDIA_CARD == CardType)
	{

		SDIO_CmdInitStructure.SDIO_Argument = (u32)(rca << 16);	   // Send CMD3, short response
		SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SET_REL_ADDR; // cmd3
		SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short; // r6
		SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
		SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
		SDIO_SendCommand(&SDIO_CmdInitStructure); // Send CMD3, short response

		errorstatus = CmdResp2Error(); // Wait for R2 response

		if (errorstatus != SD_OK)
		{
			return errorstatus; // Response error
		}
	}
	if (SDIO_SECURE_DIGITAL_IO_CARD != CardType) // Not SECURE_DIGITAL_IO_CARD
	{
		RCA = rca;

		SDIO_CmdInitStructure.SDIO_Argument = (uint32_t)(rca << 16); // Send CMD9+card RCA, get CSD, long response
		SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SEND_CSD;
		SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Long;
		SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
		SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
		SDIO_SendCommand(&SDIO_CmdInitStructure);

		errorstatus = CmdResp2Error(); // Wait for R2 response
		if (errorstatus != SD_OK)
		{
			return errorstatus; // Response error
		}
		CSD_Tab[0] = SDIO->RESP1;
		CSD_Tab[1] = SDIO->RESP2;
		CSD_Tab[2] = SDIO->RESP3;
		CSD_Tab[3] = SDIO->RESP4;
	}
	return SD_OK; // Card initialization successful
}
// Get card information
// cardinfo: card information storage area
// Return value: error status
SD_Error SD_GetCardInfo(SD_CardInfo *cardinfo)
{
	SD_Error errorstatus = SD_OK;
	u8 tmp = 0;
	cardinfo->CardType = (u8)CardType; // Card type
	cardinfo->RCA = (u16)RCA;		   // Card RCA value
	tmp = (u8)((CSD_Tab[0] & 0xFF000000) >> 24);
	cardinfo->SD_csd.CSDStruct = (tmp & 0xC0) >> 6;		   // CSD structure
	cardinfo->SD_csd.SysSpecVersion = (tmp & 0x3C) >> 2;   // 2.0 protocol hasn't defined this part (reserved), it should be defined in subsequent protocols
	cardinfo->SD_csd.Reserved1 = tmp & 0x03;			   // 2 reserved bits
	tmp = (u8)((CSD_Tab[0] & 0x00FF0000) >> 16);		   // 1st byte
	cardinfo->SD_csd.TAAC = tmp;						   // Data read time 1
	tmp = (u8)((CSD_Tab[0] & 0x0000FF00) >> 8);			   // 2nd byte
	cardinfo->SD_csd.NSAC = tmp;						   // Data read time 2
	tmp = (u8)(CSD_Tab[0] & 0x000000FF);				   // 3rd byte
	cardinfo->SD_csd.MaxBusClkFrec = tmp;				   // Transfer speed
	tmp = (u8)((CSD_Tab[1] & 0xFF000000) >> 24);		   // 4th byte
	cardinfo->SD_csd.CardComdClasses = tmp << 4;		   // Card command class high four bits
	tmp = (u8)((CSD_Tab[1] & 0x00FF0000) >> 16);		   // 5th byte
	cardinfo->SD_csd.CardComdClasses |= (tmp & 0xF0) >> 4; // Card command class low four bits
	cardinfo->SD_csd.RdBlockLen = tmp & 0x0F;			   // Maximum read data length
	tmp = (u8)((CSD_Tab[1] & 0x0000FF00) >> 8);			   // 6th byte
	cardinfo->SD_csd.PartBlockRead = (tmp & 0x80) >> 7;	   // Allow partial block read
	cardinfo->SD_csd.WrBlockMisalign = (tmp & 0x40) >> 6;  // Write block misalignment
	cardinfo->SD_csd.RdBlockMisalign = (tmp & 0x20) >> 5;  // Read block misalignment
	cardinfo->SD_csd.DSRImpl = (tmp & 0x10) >> 4;
	cardinfo->SD_csd.Reserved2 = 0;																											// Reserved
	if ((CardType == SDIO_STD_CAPACITY_SD_CARD_V1_1) || (CardType == SDIO_STD_CAPACITY_SD_CARD_V2_0) || (SDIO_MULTIMEDIA_CARD == CardType)) // Standard 1.1/2.0 card/MMC card
	{
		cardinfo->SD_csd.DeviceSize = (tmp & 0x03) << 10; // C_SIZE(12位)
		tmp = (u8)(CSD_Tab[1] & 0x000000FF);			  // 7th byte
		cardinfo->SD_csd.DeviceSize |= (tmp) << 2;
		tmp = (u8)((CSD_Tab[2] & 0xFF000000) >> 24); // 8th byte
		cardinfo->SD_csd.DeviceSize |= (tmp & 0xC0) >> 6;
		cardinfo->SD_csd.MaxRdCurrentVDDMin = (tmp & 0x38) >> 3;
		cardinfo->SD_csd.MaxRdCurrentVDDMax = (tmp & 0x07);
		tmp = (u8)((CSD_Tab[2] & 0x00FF0000) >> 16); // 9th byte
		cardinfo->SD_csd.MaxWrCurrentVDDMin = (tmp & 0xE0) >> 5;
		cardinfo->SD_csd.MaxWrCurrentVDDMax = (tmp & 0x1C) >> 2;
		cardinfo->SD_csd.DeviceSizeMul = (tmp & 0x03) << 1; // C_SIZE_MULT
		tmp = (u8)((CSD_Tab[2] & 0x0000FF00) >> 8);			// 10th byte
		cardinfo->SD_csd.DeviceSizeMul |= (tmp & 0x80) >> 7;
		cardinfo->CardCapacity = (cardinfo->SD_csd.DeviceSize + 1); // Calculate card capacity
		cardinfo->CardCapacity *= (1 << (cardinfo->SD_csd.DeviceSizeMul + 2));
		cardinfo->CardBlockSize = 1 << (cardinfo->SD_csd.RdBlockLen); // Block size
		cardinfo->CardCapacity *= cardinfo->CardBlockSize;
	}
	else if (CardType == SDIO_HIGH_CAPACITY_SD_CARD) // High capacity card
	{
		tmp = (u8)(CSD_Tab[1] & 0x000000FF);			  // 7th byte
		cardinfo->SD_csd.DeviceSize = (tmp & 0x3F) << 16; // C_SIZE
		tmp = (u8)((CSD_Tab[2] & 0xFF000000) >> 24);	  // 8th byte
		cardinfo->SD_csd.DeviceSize |= (tmp << 8);
		tmp = (u8)((CSD_Tab[2] & 0x00FF0000) >> 16); // 9th byte
		cardinfo->SD_csd.DeviceSize |= (tmp);
		tmp = (u8)((CSD_Tab[2] & 0x0000FF00) >> 8);											// 10th byte
		cardinfo->CardCapacity = (long long)(cardinfo->SD_csd.DeviceSize + 1) * 512 * 1024; // Calculate card capacity
		cardinfo->CardBlockSize = 512;														// Block size fixed at 512 bytes
	}
	cardinfo->SD_csd.EraseGrSize = (tmp & 0x40) >> 6;
	cardinfo->SD_csd.EraseGrMul = (tmp & 0x3F) << 1;
	tmp = (u8)(CSD_Tab[2] & 0x000000FF); // 11th byte
	cardinfo->SD_csd.EraseGrMul |= (tmp & 0x80) >> 7;
	cardinfo->SD_csd.WrProtectGrSize = (tmp & 0x7F);
	tmp = (u8)((CSD_Tab[3] & 0xFF000000) >> 24); // 12th byte
	cardinfo->SD_csd.WrProtectGrEnable = (tmp & 0x80) >> 7;
	cardinfo->SD_csd.ManDeflECC = (tmp & 0x60) >> 5;
	cardinfo->SD_csd.WrSpeedFact = (tmp & 0x1C) >> 2;
	cardinfo->SD_csd.MaxWrBlockLen = (tmp & 0x03) << 2;
	tmp = (u8)((CSD_Tab[3] & 0x00FF0000) >> 16); // 13th byte
	cardinfo->SD_csd.MaxWrBlockLen |= (tmp & 0xC0) >> 6;
	cardinfo->SD_csd.WriteBlockPaPartial = (tmp & 0x20) >> 5;
	cardinfo->SD_csd.Reserved3 = 0;
	cardinfo->SD_csd.ContentProtectAppli = (tmp & 0x01);
	tmp = (u8)((CSD_Tab[3] & 0x0000FF00) >> 8); // 14th byte
	cardinfo->SD_csd.FileFormatGrouop = (tmp & 0x80) >> 7;
	cardinfo->SD_csd.CopyFlag = (tmp & 0x40) >> 6;
	cardinfo->SD_csd.PermWrProtect = (tmp & 0x20) >> 5;
	cardinfo->SD_csd.TempWrProtect = (tmp & 0x10) >> 4;
	cardinfo->SD_csd.FileFormat = (tmp & 0x0C) >> 2;
	cardinfo->SD_csd.ECC = (tmp & 0x03);
	tmp = (u8)(CSD_Tab[3] & 0x000000FF); // 15th byte
	cardinfo->SD_csd.CSD_CRC = (tmp & 0xFE) >> 1;
	cardinfo->SD_csd.Reserved4 = 1;
	tmp = (u8)((CID_Tab[0] & 0xFF000000) >> 24); // 0th byte
	cardinfo->SD_cid.ManufacturerID = tmp;
	tmp = (u8)((CID_Tab[0] & 0x00FF0000) >> 16); // 1st byte
	cardinfo->SD_cid.OEM_AppliID = tmp << 8;
	tmp = (u8)((CID_Tab[0] & 0x000000FF00) >> 8); // 2nd byte
	cardinfo->SD_cid.OEM_AppliID |= tmp;
	tmp = (u8)(CID_Tab[0] & 0x000000FF); // 3rd byte
	cardinfo->SD_cid.ProdName1 = tmp << 24;
	tmp = (u8)((CID_Tab[1] & 0xFF000000) >> 24); // 4th byte
	cardinfo->SD_cid.ProdName1 |= tmp << 16;
	tmp = (u8)((CID_Tab[1] & 0x00FF0000) >> 16); // 5th byte
	cardinfo->SD_cid.ProdName1 |= tmp << 8;
	tmp = (u8)((CID_Tab[1] & 0x0000FF00) >> 8); // 6th byte
	cardinfo->SD_cid.ProdName1 |= tmp;
	tmp = (u8)(CID_Tab[1] & 0x000000FF); // 7th byte
	cardinfo->SD_cid.ProdName2 = tmp;
	tmp = (u8)((CID_Tab[2] & 0xFF000000) >> 24); // 8th byte
	cardinfo->SD_cid.ProdRev = tmp;
	tmp = (u8)((CID_Tab[2] & 0x00FF0000) >> 16); // 9th byte
	cardinfo->SD_cid.ProdSN = tmp << 24;
	tmp = (u8)((CID_Tab[2] & 0x0000FF00) >> 8); // 10th byte
	cardinfo->SD_cid.ProdSN |= tmp << 16;
	tmp = (u8)(CID_Tab[2] & 0x000000FF); // 11th byte
	cardinfo->SD_cid.ProdSN |= tmp << 8;
	tmp = (u8)((CID_Tab[3] & 0xFF000000) >> 24); // 12th byte
	cardinfo->SD_cid.ProdSN |= tmp;
	tmp = (u8)((CID_Tab[3] & 0x00FF0000) >> 16); // 13th byte
	cardinfo->SD_cid.Reserved1 |= (tmp & 0xF0) >> 4;
	cardinfo->SD_cid.ManufactDate = (tmp & 0x0F) << 8;
	tmp = (u8)((CID_Tab[3] & 0x0000FF00) >> 8); // 14th byte
	cardinfo->SD_cid.ManufactDate |= tmp;
	tmp = (u8)(CID_Tab[3] & 0x000000FF); // 15th byte
	cardinfo->SD_cid.CID_CRC = (tmp & 0xFE) >> 1;
	cardinfo->SD_cid.Reserved2 = 1;
	return errorstatus;
}
// Set SDIO bus width (MMC cards do not support 4-bit mode)
// wmode: bit width mode. 0, 1-bit data width; 1, 4-bit data width; 2, 8-bit data width
// Return value: SD card error status

// Set SDIO bus width (MMC cards do not support 4-bit mode)
//    @arg SDIO_BusWide_8b: 8-bit data transfer (Only for MMC)
//    @arg SDIO_BusWide_4b: 4-bit data transfer
//    @arg SDIO_BusWide_1b: 1-bit data transfer (default)
// Return value: SD card error status

SD_Error SD_EnableWideBusOperation(u32 WideMode)
{
	SD_Error errorstatus = SD_OK;
	if (SDIO_MULTIMEDIA_CARD == CardType)
	{
		errorstatus = SD_UNSUPPORTED_FEATURE;
		return (errorstatus);
	}
	else if ((SDIO_STD_CAPACITY_SD_CARD_V1_1 == CardType) || (SDIO_STD_CAPACITY_SD_CARD_V2_0 == CardType) || (SDIO_HIGH_CAPACITY_SD_CARD == CardType))
	{
		if (SDIO_BusWide_8b == WideMode) // 2.0 sd不支持8bits
		{
			errorstatus = SD_UNSUPPORTED_FEATURE;
			return (errorstatus);
		}
		else
		{
			errorstatus = SDEnWideBus(WideMode);
			if (SD_OK == errorstatus)
			{
				SDIO->CLKCR &= ~(3 << 11); // 清除之前的位宽设置
				SDIO->CLKCR |= WideMode;   // 1位/4位总线宽度
				SDIO->CLKCR |= 0 << 14;	   // 不开启硬件流控制
			}
		}
	}
	return errorstatus;
}
// 设置SD卡工作模式
// Mode:
// 返回值:错误状态
SD_Error SD_SetDeviceMode(u32 Mode)
{
	SD_Error errorstatus = SD_OK;
	if ((Mode == SD_DMA_MODE) || (Mode == SD_POLLING_MODE))
	{
		DeviceMode = Mode;
	}
	else
	{
		errorstatus = SD_INVALID_PARAMETER;
	}
	return errorstatus;
}
// 选卡
// 发送CMD7,选择相对地址(rca)为addr的卡,取消其他卡.如果为0,则都不选择.
// addr:卡的RCA地址
SD_Error SD_SelectDeselect(u32 addr)
{

	SDIO_CmdInitStructure.SDIO_Argument = addr; // 发送CMD7,选择卡,短响应
	SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SEL_DESEL_CARD;
	SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
	SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
	SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
	SDIO_SendCommand(&SDIO_CmdInitStructure); // 发送CMD7,选择卡,短响应

	return CmdResp1Error(SD_CMD_SEL_DESEL_CARD);
}
// SD卡读取一个块
// buf:读数据缓存区(必须4字节对齐!!)
// addr:读取地址
// blksize:块大小
SD_Error SD_ReadBlock(u8 *buf, long long addr, u16 blksize)
{
	SD_Error errorstatus = SD_OK;
	u8 power;
	u32 count = 0, *tempbuff = (u32 *)buf; // 转换为u32指针
	u32 timeout = SDIO_DATATIMEOUT;
	if (NULL == buf)
		return SD_INVALID_PARAMETER;
	SDIO->DCTRL = 0x0; // 数据控制寄存器清零(关DMA)

	if (CardType == SDIO_HIGH_CAPACITY_SD_CARD) // 大容量卡
	{
		blksize = 512;
		addr >>= 9;
	}
	SDIO_DataInitStructure.SDIO_DataBlockSize = SDIO_DataBlockSize_1b; // 清除DPSM状态机配置
	SDIO_DataInitStructure.SDIO_DataLength = 0;
	SDIO_DataInitStructure.SDIO_DataTimeOut = SD_DATATIMEOUT;
	SDIO_DataInitStructure.SDIO_DPSM = SDIO_DPSM_Enable;
	SDIO_DataInitStructure.SDIO_TransferDir = SDIO_TransferDir_ToCard;
	SDIO_DataInitStructure.SDIO_TransferMode = SDIO_TransferMode_Block;
	SDIO_DataConfig(&SDIO_DataInitStructure);

	if (SDIO->RESP1 & SD_CARD_LOCKED)
	{
		return SD_LOCK_UNLOCK_FAILED; // 卡锁了
	}
	if ((blksize > 0) && (blksize <= 2048) && ((blksize & (blksize - 1)) == 0))
	{
		power = convert_from_bytes_to_power_of_two(blksize);

		SDIO_CmdInitStructure.SDIO_Argument = blksize;
		SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SET_BLOCKLEN;
		SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
		SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
		SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
		SDIO_SendCommand(&SDIO_CmdInitStructure); // 发送CMD16+设置数据长度为blksize,短响应

		errorstatus = CmdResp1Error(SD_CMD_SET_BLOCKLEN); // 等待R1响应

		if (errorstatus != SD_OK)
		{
			return errorstatus; // 响应错误
		}
	}
	else
	{
		return SD_INVALID_PARAMETER;
	}
	SDIO_DataInitStructure.SDIO_DataBlockSize = power << 4; // 清除DPSM状态机配置
	SDIO_DataInitStructure.SDIO_DataLength = blksize;
	SDIO_DataInitStructure.SDIO_DataTimeOut = SD_DATATIMEOUT;
	SDIO_DataInitStructure.SDIO_DPSM = SDIO_DPSM_Enable;
	SDIO_DataInitStructure.SDIO_TransferDir = SDIO_TransferDir_ToSDIO;
	SDIO_DataInitStructure.SDIO_TransferMode = SDIO_TransferMode_Block;
	SDIO_DataConfig(&SDIO_DataInitStructure);

	SDIO_CmdInitStructure.SDIO_Argument = addr;
	SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_READ_SINGLE_BLOCK;
	SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
	SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
	SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
	SDIO_SendCommand(&SDIO_CmdInitStructure); // 发送CMD17+从addr地址出读取数据,短响应

	errorstatus = CmdResp1Error(SD_CMD_READ_SINGLE_BLOCK); // 等待R1响应
	if (errorstatus != SD_OK)
	{
		return errorstatus; // 响应错误
	}
	if (DeviceMode == SD_POLLING_MODE) // 查询模式,轮询数据
	{
		INTX_DISABLE();																   // 关闭总中断(POLLING模式,严禁中断打断SDIO读写操作!!!)
		while (!(SDIO->STA & ((1 << 5) | (1 << 1) | (1 << 3) | (1 << 10) | (1 << 9)))) // 无上溢/CRC/超时/完成(标志)/起始位错误
		{
			if (SDIO_GetFlagStatus(SDIO_FLAG_RXFIFOHF) != RESET) // 接收区半满,表示至少存了8个字
			{
				for (count = 0; count < 8; count++) // 循环读取数据
				{
					*(tempbuff + count) = SDIO->FIFO;
				}
				tempbuff += 8;
				timeout = 0X7FFFFF; // 读数据溢出时间
			}
			else // 处理超时
			{
				if (timeout == 0)
					return SD_DATA_TIMEOUT;
				timeout--;
			}
		}
		if (SDIO_GetFlagStatus(SDIO_FLAG_DTIMEOUT) != RESET) // 数据超时错误
		{
			SDIO_ClearFlag(SDIO_FLAG_DTIMEOUT); // 清错误标志
			return SD_DATA_TIMEOUT;
		}
		else if (SDIO_GetFlagStatus(SDIO_FLAG_DCRCFAIL) != RESET) // 数据块CRC错误
		{
			SDIO_ClearFlag(SDIO_FLAG_DCRCFAIL); // 清错误标志
			return SD_DATA_CRC_FAIL;
		}
		else if (SDIO_GetFlagStatus(SDIO_FLAG_RXOVERR) != RESET) // 接收fifo上溢错误
		{
			SDIO_ClearFlag(SDIO_FLAG_RXOVERR); // 清错误标志
			return SD_RX_OVERRUN;
		}
		else if (SDIO_GetFlagStatus(SDIO_FLAG_STBITERR) != RESET) // 接收起始位错误
		{
			SDIO_ClearFlag(SDIO_FLAG_STBITERR); // 清错误标志
			return SD_START_BIT_ERR;
		}
		while (SDIO_GetFlagStatus(SDIO_FLAG_RXDAVL) != RESET) // FIFO里面,还存在可用数据
		{
			*tempbuff = SDIO->FIFO; // 循环读取数据
			tempbuff++;
		}
		INTX_ENABLE();					   // 开启总中断
		SDIO_ClearFlag(SDIO_STATIC_FLAGS); // 清除所有标记
	}
	else if (DeviceMode == SD_DMA_MODE)
	{
		TransferError = SD_OK;
		StopCondition = 0;													// 单块读,不需要发送停止传输指令
		TransferEnd = 0;													// 传输结束标置位，在中断服务置1
		SDIO->MASK |= (1 << 1) | (1 << 3) | (1 << 8) | (1 << 5) | (1 << 9); // 配置需要的中断
		SDIO->DCTRL |= 1 << 3;												// SDIO DMA使能
		SD_DMA_Config((u32 *)buf, blksize, DMA_DIR_PeripheralToMemory);
		while (((DMA2->LISR & (1 << 21)) == RESET) && (TransferEnd == 0) && (TransferError == SD_OK) && timeout)
			timeout--; // 等待传输完成
		if (timeout == 0)
		{
			return SD_DATA_TIMEOUT; // 超时
		}
		if (TransferError != SD_OK)
		{
			errorstatus = TransferError;
		}
	}
	return errorstatus;
	//   SD_Error errorstatus = SD_OK;
	//   uint32_t count = 0, *tempbuff = (uint32_t *)buf;

	//   TransferError = SD_OK;
	//   TransferEnd = 0;	 //传输结束标置位，在中断服务置1
	//   StopCondition = 0;  //怎么用的？

	//   SDIO->DCTRL = 0x0;

	//   if (CardType == SDIO_HIGH_CAPACITY_SD_CARD)
	//   {
	//     blksize = 512;
	// 	addr >>= 9;

	//   }
	//   /*******************add，没有这一段容易卡死在DMA检测中*************************************/
	//   /* Set Block Size for Card，cmd16,
	// 	 * 若是sdsc卡，可以用来设置块大小，
	// 	 * 若是sdhc卡，块大小为512字节，不受cmd16影响
	// 	 */
	//   SDIO_CmdInitStructure.SDIO_Argument = (uint32_t) blksize;
	//   SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SET_BLOCKLEN;
	//   SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;   //r1
	//   SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
	//   SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
	//   SDIO_SendCommand(&SDIO_CmdInitStructure);

	//   errorstatus = CmdResp1Error(SD_CMD_SET_BLOCKLEN);

	//   if (SD_OK != errorstatus)
	//   {
	//     return(errorstatus);
	//   }
	//  /*********************************************************************************/
	//   SDIO_DataInitStructure.SDIO_DataTimeOut = SD_DATATIMEOUT;
	//   SDIO_DataInitStructure.SDIO_DataLength = blksize;
	//   SDIO_DataInitStructure.SDIO_DataBlockSize = (uint32_t) 9 << 4;
	//   SDIO_DataInitStructure.SDIO_TransferDir = SDIO_TransferDir_ToSDIO;
	//   SDIO_DataInitStructure.SDIO_TransferMode = SDIO_TransferMode_Block;
	//   SDIO_DataInitStructure.SDIO_DPSM = SDIO_DPSM_Enable;
	//   SDIO_DataConfig(&SDIO_DataInitStructure);

	//   /*!< Send CMD17 READ_SINGLE_BLOCK */
	//   SDIO_CmdInitStructure.SDIO_Argument = (uint32_t)blksize;
	//   SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_READ_SINGLE_BLOCK;
	//   SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
	//   SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
	//   SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
	//   SDIO_SendCommand(&SDIO_CmdInitStructure);

	//   errorstatus = CmdResp1Error(SD_CMD_READ_SINGLE_BLOCK);

	//   if (errorstatus != SD_OK)
	//   {
	//     return(errorstatus);
	//   }
	//     SDIO_ITConfig(SDIO_IT_DATAEND, ENABLE);
	//     SDIO_DMACmd(ENABLE);
	//     SD_DMA_Config((uint32_t *)buf, blksize, DMA_DIR_PeripheralToMemory);
	// 	SDIO_DMACmd(DISABLE);

	//   return(errorstatus);
}

// SD卡读取多个块
// buf:读数据缓存区
// addr:读取地址
// blksize:块大小
// nblks:要读取的块数
// 返回值:错误状态
u32 *tempbuff __attribute__((aligned(4)));
SD_Error SD_ReadMultiBlocks(u8 *buf, long long addr, u16 blksize, u32 nblks)
{
	SD_Error errorstatus = SD_OK;
	u8 power;
	u32 count = 0;
	u32 timeout = SDIO_DATATIMEOUT;
	tempbuff = (u32 *)buf; // 转换为u32指针

	SDIO->DCTRL = 0x0;							// 数据控制寄存器清零(关DMA)
	if (CardType == SDIO_HIGH_CAPACITY_SD_CARD) // 大容量卡
	{
		blksize = 512;
		addr >>= 9;
	}

	SDIO_DataInitStructure.SDIO_DataBlockSize = 0;
	; // 清除DPSM状态机配置
	SDIO_DataInitStructure.SDIO_DataLength = 0;
	SDIO_DataInitStructure.SDIO_DataTimeOut = SD_DATATIMEOUT;
	SDIO_DataInitStructure.SDIO_DPSM = SDIO_DPSM_Enable;
	SDIO_DataInitStructure.SDIO_TransferDir = SDIO_TransferDir_ToCard;
	SDIO_DataInitStructure.SDIO_TransferMode = SDIO_TransferMode_Block;
	SDIO_DataConfig(&SDIO_DataInitStructure);

	if (SDIO->RESP1 & SD_CARD_LOCKED)
		return SD_LOCK_UNLOCK_FAILED; // 卡锁了
	if ((blksize > 0) && (blksize <= 2048) && ((blksize & (blksize - 1)) == 0))
	{
		power = convert_from_bytes_to_power_of_two(blksize);

		SDIO_CmdInitStructure.SDIO_Argument = blksize; // 发送CMD16+设置数据长度为blksize,短响应
		SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SET_BLOCKLEN;
		SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
		SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
		SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
		SDIO_SendCommand(&SDIO_CmdInitStructure);

		errorstatus = CmdResp1Error(SD_CMD_SET_BLOCKLEN); // 等待R1响应

		if (errorstatus != SD_OK)
			return errorstatus; // 响应错误
	}
	else
		return SD_INVALID_PARAMETER;

	if (nblks > 1) // 多块读
	{
		if (nblks * blksize > SD_MAX_DATA_LENGTH)
			return SD_INVALID_PARAMETER; // 判断是否超过最大接收长度

		SDIO_DataInitStructure.SDIO_DataBlockSize = power << 4;
		; // nblks*blksize,512块大小,卡到控制器
		SDIO_DataInitStructure.SDIO_DataLength = nblks * blksize;
		SDIO_DataInitStructure.SDIO_DataTimeOut = SD_DATATIMEOUT;
		SDIO_DataInitStructure.SDIO_DPSM = SDIO_DPSM_Enable;
		SDIO_DataInitStructure.SDIO_TransferDir = SDIO_TransferDir_ToSDIO;
		SDIO_DataInitStructure.SDIO_TransferMode = SDIO_TransferMode_Block;
		SDIO_DataConfig(&SDIO_DataInitStructure);

		SDIO_CmdInitStructure.SDIO_Argument = addr; // 发送CMD18+从addr地址出读取数据,短响应
		SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_READ_MULT_BLOCK;
		SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
		SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
		SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
		SDIO_SendCommand(&SDIO_CmdInitStructure);

		errorstatus = CmdResp1Error(SD_CMD_READ_MULT_BLOCK); // 等待R1响应

		if (errorstatus != SD_OK)
			return errorstatus; // 响应错误

		if (DeviceMode == SD_POLLING_MODE)
		{
			INTX_DISABLE();																  // 关闭总中断(POLLING模式,严禁中断打断SDIO读写操作!!!)
			while (!(SDIO->STA & ((1 << 5) | (1 << 1) | (1 << 3) | (1 << 8) | (1 << 9)))) // 无上溢/CRC/超时/完成(标志)/起始位错误
			{
				if (SDIO_GetFlagStatus(SDIO_FLAG_RXFIFOHF) != RESET) // 接收区半满,表示至少存了8个字
				{
					for (count = 0; count < 8; count++) // 循环读取数据
					{
						*(tempbuff + count) = SDIO->FIFO;
					}
					tempbuff += 8;
					timeout = 0X7FFFFF; // 读数据溢出时间
				}
				else // 处理超时
				{
					if (timeout == 0)
						return SD_DATA_TIMEOUT;
					timeout--;
				}
			}
			if (SDIO_GetFlagStatus(SDIO_FLAG_DTIMEOUT) != RESET) // 数据超时错误
			{
				SDIO_ClearFlag(SDIO_FLAG_DTIMEOUT); // 清错误标志
				return SD_DATA_TIMEOUT;
			}
			else if (SDIO_GetFlagStatus(SDIO_FLAG_DCRCFAIL) != RESET) // 数据块CRC错误
			{
				SDIO_ClearFlag(SDIO_FLAG_DCRCFAIL); // 清错误标志
				return SD_DATA_CRC_FAIL;
			}
			else if (SDIO_GetFlagStatus(SDIO_FLAG_RXOVERR) != RESET) // 接收fifo上溢错误
			{
				SDIO_ClearFlag(SDIO_FLAG_RXOVERR); // 清错误标志
				return SD_RX_OVERRUN;
			}
			else if (SDIO_GetFlagStatus(SDIO_FLAG_STBITERR) != RESET) // 接收起始位错误
			{
				SDIO_ClearFlag(SDIO_FLAG_STBITERR); // 清错误标志
				return SD_START_BIT_ERR;
			}

			while (SDIO_GetFlagStatus(SDIO_FLAG_RXDAVL) != RESET) // FIFO里面,还存在可用数据
			{
				*tempbuff = SDIO->FIFO; // 循环读取数据
				tempbuff++;
			}
			if (SDIO_GetFlagStatus(SDIO_FLAG_DATAEND) != RESET) // 接收结束
			{
				if ((SDIO_STD_CAPACITY_SD_CARD_V1_1 == CardType) || (SDIO_STD_CAPACITY_SD_CARD_V2_0 == CardType) || (SDIO_HIGH_CAPACITY_SD_CARD == CardType))
				{
					SDIO_CmdInitStructure.SDIO_Argument = 0; // 发送CMD12+结束传输
					SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_STOP_TRANSMISSION;
					SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
					SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
					SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
					SDIO_SendCommand(&SDIO_CmdInitStructure);

					errorstatus = CmdResp1Error(SD_CMD_STOP_TRANSMISSION); // 等待R1响应

					if (errorstatus != SD_OK)
						return errorstatus;
				}
			}
			INTX_ENABLE();					   // 开启总中断
			SDIO_ClearFlag(SDIO_STATIC_FLAGS); // 清除所有标记
		}
		else if (DeviceMode == SD_DMA_MODE)
		{
			TransferError = SD_OK;
			StopCondition = 1;													// 多块读,需要发送停止传输指令
			TransferEnd = 0;													// 传输结束标置位，在中断服务置1
			SDIO->MASK |= (1 << 1) | (1 << 3) | (1 << 8) | (1 << 5) | (1 << 9); // 配置需要的中断
			SDIO->DCTRL |= 1 << 3;												// SDIO DMA使能
			SD_DMA_Config((u32 *)buf, nblks * blksize, DMA_DIR_PeripheralToMemory);
			while (((DMA2->LISR & (1 << 27)) == RESET) && timeout)
				timeout--; // 等待传输完成
			if (timeout == 0)
				return SD_DATA_TIMEOUT; // 超时
			while ((TransferEnd == 0) && (TransferError == SD_OK))
				;
			if (TransferError != SD_OK)
				errorstatus = TransferError;
		}
	}
	return errorstatus;
}
// SD卡写1个块
// buf:数据缓存区
// addr:写地址
// blksize:块大小
// 返回值:错误状态
SD_Error SD_WriteBlock(u8 *buf, long long addr, u16 blksize)
{
	SD_Error errorstatus = SD_OK;

	u8 power = 0, cardstate = 0;

	u32 timeout = 0, bytestransferred = 0;

	u32 cardstatus = 0, count = 0, restwords = 0;

	u32 tlen = blksize; // 总长度(字节)

	u32 *tempbuff = (u32 *)buf;

	if (buf == NULL)
		return SD_INVALID_PARAMETER; // 参数错误

	SDIO->DCTRL = 0x0; // 数据控制寄存器清零(关DMA)

	SDIO_DataInitStructure.SDIO_DataBlockSize = 0;
	; // 清除DPSM状态机配置
	SDIO_DataInitStructure.SDIO_DataLength = 0;
	SDIO_DataInitStructure.SDIO_DataTimeOut = SD_DATATIMEOUT;
	SDIO_DataInitStructure.SDIO_DPSM = SDIO_DPSM_Enable;
	SDIO_DataInitStructure.SDIO_TransferDir = SDIO_TransferDir_ToCard;
	SDIO_DataInitStructure.SDIO_TransferMode = SDIO_TransferMode_Block;
	SDIO_DataConfig(&SDIO_DataInitStructure);

	if (SDIO->RESP1 & SD_CARD_LOCKED)
		return SD_LOCK_UNLOCK_FAILED;			// 卡锁了
	if (CardType == SDIO_HIGH_CAPACITY_SD_CARD) // 大容量卡
	{
		blksize = 512;
		addr >>= 9;
	}
	if ((blksize > 0) && (blksize <= 2048) && ((blksize & (blksize - 1)) == 0))
	{
		power = convert_from_bytes_to_power_of_two(blksize);

		SDIO_CmdInitStructure.SDIO_Argument = blksize; // 发送CMD16+设置数据长度为blksize,短响应
		SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SET_BLOCKLEN;
		SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
		SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
		SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
		SDIO_SendCommand(&SDIO_CmdInitStructure);

		errorstatus = CmdResp1Error(SD_CMD_SET_BLOCKLEN); // 等待R1响应

		if (errorstatus != SD_OK)
			return errorstatus; // 响应错误
	}
	else
		return SD_INVALID_PARAMETER;

	SDIO_CmdInitStructure.SDIO_Argument = (u32)RCA << 16; // 发送CMD13,查询卡的状态,短响应
	SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SEND_STATUS;
	SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
	SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
	SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
	SDIO_SendCommand(&SDIO_CmdInitStructure);

	errorstatus = CmdResp1Error(SD_CMD_SEND_STATUS); // 等待R1响应

	if (errorstatus != SD_OK)
		return errorstatus;
	cardstatus = SDIO->RESP1;
	timeout = SD_DATATIMEOUT;
	while (((cardstatus & 0x00000100) == 0) && (timeout > 0)) // 检查READY_FOR_DATA位是否置位
	{
		timeout--;

		SDIO_CmdInitStructure.SDIO_Argument = (u32)RCA << 16; // 发送CMD13,查询卡的状态,短响应
		SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SEND_STATUS;
		SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
		SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
		SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
		SDIO_SendCommand(&SDIO_CmdInitStructure);

		errorstatus = CmdResp1Error(SD_CMD_SEND_STATUS); // 等待R1响应

		if (errorstatus != SD_OK)
			return errorstatus;

		cardstatus = SDIO->RESP1;
	}
	if (timeout == 0)
		return SD_ERROR;

	SDIO_CmdInitStructure.SDIO_Argument = addr; // 发送CMD24,写单块指令,短响应
	SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_WRITE_SINGLE_BLOCK;
	SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
	SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
	SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
	SDIO_SendCommand(&SDIO_CmdInitStructure);

	errorstatus = CmdResp1Error(SD_CMD_WRITE_SINGLE_BLOCK); // 等待R1响应

	if (errorstatus != SD_OK)
		return errorstatus;

	StopCondition = 0; // 单块写,不需要发送停止传输指令

	SDIO_DataInitStructure.SDIO_DataBlockSize = power << 4;
	; // blksize, 控制器到卡
	SDIO_DataInitStructure.SDIO_DataLength = blksize;
	SDIO_DataInitStructure.SDIO_DataTimeOut = SD_DATATIMEOUT;
	SDIO_DataInitStructure.SDIO_DPSM = SDIO_DPSM_Enable;
	SDIO_DataInitStructure.SDIO_TransferDir = SDIO_TransferDir_ToCard;
	SDIO_DataInitStructure.SDIO_TransferMode = SDIO_TransferMode_Block;
	SDIO_DataConfig(&SDIO_DataInitStructure);

	timeout = SDIO_DATATIMEOUT;

	if (DeviceMode == SD_POLLING_MODE)
	{
		INTX_DISABLE();																   // 关闭总中断(POLLING模式,严禁中断打断SDIO读写操作!!!)
		while (!(SDIO->STA & ((1 << 10) | (1 << 4) | (1 << 1) | (1 << 3) | (1 << 9)))) // 数据块发送成功/下溢/CRC/超时/起始位错误
		{
			if (SDIO_GetFlagStatus(SDIO_FLAG_TXFIFOHE) != RESET) // 发送区半空,表示至少存了8个字
			{
				if ((tlen - bytestransferred) < SD_HALFFIFOBYTES) // 不够32字节了
				{
					restwords = ((tlen - bytestransferred) % 4 == 0) ? ((tlen - bytestransferred) / 4) : ((tlen - bytestransferred) / 4 + 1);

					for (count = 0; count < restwords; count++, tempbuff++, bytestransferred += 4)
					{
						SDIO->FIFO = *tempbuff;
					}
				}
				else
				{
					for (count = 0; count < 8; count++)
					{
						SDIO->FIFO = *(tempbuff + count);
					}
					tempbuff += 8;
					bytestransferred += 32;
				}
				timeout = 0X3FFFFFFF; // 写数据溢出时间
			}
			else
			{
				if (timeout == 0)
					return SD_DATA_TIMEOUT;
				timeout--;
			}
		}
		if (SDIO_GetFlagStatus(SDIO_FLAG_DTIMEOUT) != RESET) // 数据超时错误
		{
			SDIO_ClearFlag(SDIO_FLAG_DTIMEOUT); // 清错误标志
			return SD_DATA_TIMEOUT;
		}
		else if (SDIO_GetFlagStatus(SDIO_FLAG_DCRCFAIL) != RESET) // 数据块CRC错误
		{
			SDIO_ClearFlag(SDIO_FLAG_DCRCFAIL); // 清错误标志
			return SD_DATA_CRC_FAIL;
		}
		else if (SDIO_GetFlagStatus(SDIO_FLAG_TXUNDERR) != RESET) // 接收fifo下溢错误
		{
			SDIO_ClearFlag(SDIO_FLAG_TXUNDERR); // 清错误标志
			return SD_TX_UNDERRUN;
		}
		else if (SDIO_GetFlagStatus(SDIO_FLAG_STBITERR) != RESET) // 接收起始位错误
		{
			SDIO_ClearFlag(SDIO_FLAG_STBITERR); // 清错误标志
			return SD_START_BIT_ERR;
		}

		INTX_ENABLE();					   // 开启总中断
		SDIO_ClearFlag(SDIO_STATIC_FLAGS); // 清除所有标记
	}
	else if (DeviceMode == SD_DMA_MODE)
	{
		TransferError = SD_OK;
		StopCondition = 0;													// 单块写,不需要发送停止传输指令
		TransferEnd = 0;													// 传输结束标置位，在中断服务置1
		SDIO->MASK |= (1 << 1) | (1 << 3) | (1 << 8) | (1 << 4) | (1 << 9); // 配置产生数据接收完成中断
		SD_DMA_Config((u32 *)buf, blksize, DMA_DIR_MemoryToPeripheral);		// SDIO DMA配置
		SDIO->DCTRL |= 1 << 3;												// SDIO DMA使能.
		while (((DMA2->LISR & (1 << 27)) == RESET) && timeout)
			timeout--; // 等待传输完成
		if (timeout == 0)
		{
			SD_Init();				// 重新初始化SD卡,可以解决写入死机的问题
			return SD_DATA_TIMEOUT; // 超时
		}
		timeout = SDIO_DATATIMEOUT;
		while ((TransferEnd == 0) && (TransferError == SD_OK) && timeout)
			timeout--;
		if (timeout == 0)
			return SD_DATA_TIMEOUT; // 超时
		if (TransferError != SD_OK)
			return TransferError;
	}
	SDIO_ClearFlag(SDIO_STATIC_FLAGS); // 清除所有标记
	errorstatus = IsCardProgramming(&cardstate);
	while ((errorstatus == SD_OK) && ((cardstate == SD_CARD_PROGRAMMING) || (cardstate == SD_CARD_RECEIVING)))
	{
		errorstatus = IsCardProgramming(&cardstate);
	}
	return errorstatus;
}
// SD卡写多个块
// buf:数据缓存区
// addr:写地址
// blksize:块大小
// nblks:要写入的块数
// 返回值:错误状态
SD_Error SD_WriteMultiBlocks(u8 *buf, long long addr, u16 blksize, u32 nblks)
{
	SD_Error errorstatus = SD_OK;
	u8 power = 0, cardstate = 0;
	u32 timeout = 0, bytestransferred = 0;
	u32 count = 0, restwords = 0;
	u32 tlen = nblks * blksize; // 总长度(字节)
	u32 *tempbuff = (u32 *)buf;
	if (buf == NULL)
		return SD_INVALID_PARAMETER; // 参数错误
	SDIO->DCTRL = 0x0;				 // 数据控制寄存器清零(关DMA)

	SDIO_DataInitStructure.SDIO_DataBlockSize = 0;
	; // 清除DPSM状态机配置
	SDIO_DataInitStructure.SDIO_DataLength = 0;
	SDIO_DataInitStructure.SDIO_DataTimeOut = SD_DATATIMEOUT;
	SDIO_DataInitStructure.SDIO_DPSM = SDIO_DPSM_Enable;
	SDIO_DataInitStructure.SDIO_TransferDir = SDIO_TransferDir_ToCard;
	SDIO_DataInitStructure.SDIO_TransferMode = SDIO_TransferMode_Block;
	SDIO_DataConfig(&SDIO_DataInitStructure);

	if (SDIO->RESP1 & SD_CARD_LOCKED)
		return SD_LOCK_UNLOCK_FAILED;			// 卡锁了
	if (CardType == SDIO_HIGH_CAPACITY_SD_CARD) // 大容量卡
	{
		blksize = 512;
		addr >>= 9;
	}
	if ((blksize > 0) && (blksize <= 2048) && ((blksize & (blksize - 1)) == 0))
	{
		power = convert_from_bytes_to_power_of_two(blksize);

		SDIO_CmdInitStructure.SDIO_Argument = blksize; // 发送CMD16+设置数据长度为blksize,短响应
		SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SET_BLOCKLEN;
		SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
		SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
		SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
		SDIO_SendCommand(&SDIO_CmdInitStructure);

		errorstatus = CmdResp1Error(SD_CMD_SET_BLOCKLEN); // 等待R1响应

		if (errorstatus != SD_OK)
			return errorstatus; // 响应错误
	}
	else
		return SD_INVALID_PARAMETER;
	if (nblks > 1)
	{
		if (nblks * blksize > SD_MAX_DATA_LENGTH)
			return SD_INVALID_PARAMETER;
		if ((SDIO_STD_CAPACITY_SD_CARD_V1_1 == CardType) || (SDIO_STD_CAPACITY_SD_CARD_V2_0 == CardType) || (SDIO_HIGH_CAPACITY_SD_CARD == CardType))
		{
			// 提高性能
			SDIO_CmdInitStructure.SDIO_Argument = (u32)RCA << 16; // 发送ACMD55,短响应
			SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_APP_CMD;
			SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
			SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
			SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
			SDIO_SendCommand(&SDIO_CmdInitStructure);

			errorstatus = CmdResp1Error(SD_CMD_APP_CMD); // 等待R1响应

			if (errorstatus != SD_OK)
				return errorstatus;

			SDIO_CmdInitStructure.SDIO_Argument = nblks; // 发送CMD23,设置块数量,短响应
			SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SET_BLOCK_COUNT;
			SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
			SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
			SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
			SDIO_SendCommand(&SDIO_CmdInitStructure);

			errorstatus = CmdResp1Error(SD_CMD_SET_BLOCK_COUNT); // 等待R1响应

			if (errorstatus != SD_OK)
				return errorstatus;
		}

		SDIO_CmdInitStructure.SDIO_Argument = addr; // 发送CMD25,多块写指令,短响应
		SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_WRITE_MULT_BLOCK;
		SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
		SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
		SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
		SDIO_SendCommand(&SDIO_CmdInitStructure);

		errorstatus = CmdResp1Error(SD_CMD_WRITE_MULT_BLOCK); // 等待R1响应

		if (errorstatus != SD_OK)
			return errorstatus;

		SDIO_DataInitStructure.SDIO_DataBlockSize = power << 4;
		; // blksize, 控制器到卡
		SDIO_DataInitStructure.SDIO_DataLength = nblks * blksize;
		SDIO_DataInitStructure.SDIO_DataTimeOut = SD_DATATIMEOUT;
		SDIO_DataInitStructure.SDIO_DPSM = SDIO_DPSM_Enable;
		SDIO_DataInitStructure.SDIO_TransferDir = SDIO_TransferDir_ToCard;
		SDIO_DataInitStructure.SDIO_TransferMode = SDIO_TransferMode_Block;
		SDIO_DataConfig(&SDIO_DataInitStructure);

		if (DeviceMode == SD_POLLING_MODE)
		{
			timeout = SDIO_DATATIMEOUT;
			INTX_DISABLE();																  // 关闭总中断(POLLING模式,严禁中断打断SDIO读写操作!!!)
			while (!(SDIO->STA & ((1 << 4) | (1 << 1) | (1 << 8) | (1 << 3) | (1 << 9)))) // 下溢/CRC/数据结束/超时/起始位错误
			{
				if (SDIO_GetFlagStatus(SDIO_FLAG_TXFIFOHE) != RESET) // 发送区半空,表示至少存了8字(32字节)
				{
					if ((tlen - bytestransferred) < SD_HALFFIFOBYTES) // 不够32字节了
					{
						restwords = ((tlen - bytestransferred) % 4 == 0) ? ((tlen - bytestransferred) / 4) : ((tlen - bytestransferred) / 4 + 1);
						for (count = 0; count < restwords; count++, tempbuff++, bytestransferred += 4)
						{
							SDIO->FIFO = *tempbuff;
						}
					}
					else // 发送区半空,可以发送至少8字(32字节)数据
					{
						for (count = 0; count < SD_HALFFIFO; count++)
						{
							SDIO->FIFO = *(tempbuff + count);
						}
						tempbuff += SD_HALFFIFO;
						bytestransferred += SD_HALFFIFOBYTES;
					}
					timeout = 0X3FFFFFFF; // 写数据溢出时间
				}
				else
				{
					if (timeout == 0)
						return SD_DATA_TIMEOUT;
					timeout--;
				}
			}
			if (SDIO_GetFlagStatus(SDIO_FLAG_DTIMEOUT) != RESET) // 数据超时错误
			{
				SDIO_ClearFlag(SDIO_FLAG_DTIMEOUT); // 清错误标志
				return SD_DATA_TIMEOUT;
			}
			else if (SDIO_GetFlagStatus(SDIO_FLAG_DCRCFAIL) != RESET) // 数据块CRC错误
			{
				SDIO_ClearFlag(SDIO_FLAG_DCRCFAIL); // 清错误标志
				return SD_DATA_CRC_FAIL;
			}
			else if (SDIO_GetFlagStatus(SDIO_FLAG_TXUNDERR) != RESET) // 接收fifo下溢错误
			{
				SDIO_ClearFlag(SDIO_FLAG_TXUNDERR); // 清错误标志
				return SD_TX_UNDERRUN;
			}
			else if (SDIO_GetFlagStatus(SDIO_FLAG_STBITERR) != RESET) // 接收起始位错误
			{
				SDIO_ClearFlag(SDIO_FLAG_STBITERR); // 清错误标志
				return SD_START_BIT_ERR;
			}

			if (SDIO_GetFlagStatus(SDIO_FLAG_DATAEND) != RESET) // 发送结束
			{
				if ((SDIO_STD_CAPACITY_SD_CARD_V1_1 == CardType) || (SDIO_STD_CAPACITY_SD_CARD_V2_0 == CardType) || (SDIO_HIGH_CAPACITY_SD_CARD == CardType))
				{
					SDIO_CmdInitStructure.SDIO_Argument = 0; // 发送CMD12+结束传输
					SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_STOP_TRANSMISSION;
					SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
					SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
					SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
					SDIO_SendCommand(&SDIO_CmdInitStructure);

					errorstatus = CmdResp1Error(SD_CMD_STOP_TRANSMISSION); // 等待R1响应
					if (errorstatus != SD_OK)
						return errorstatus;
				}
			}
			INTX_ENABLE();					   // 开启总中断
			SDIO_ClearFlag(SDIO_STATIC_FLAGS); // 清除所有标记
		}
		else if (DeviceMode == SD_DMA_MODE)
		{
			TransferError = SD_OK;
			StopCondition = 1;														// 多块写,需要发送停止传输指令
			TransferEnd = 0;														// 传输结束标置位，在中断服务置1
			SDIO->MASK |= (1 << 1) | (1 << 3) | (1 << 8) | (1 << 4) | (1 << 9);		// 配置产生数据接收完成中断
			SD_DMA_Config((u32 *)buf, nblks * blksize, DMA_DIR_MemoryToPeripheral); // SDIO DMA配置
			SDIO->DCTRL |= 1 << 3;													// SDIO DMA使能.
			timeout = SDIO_DATATIMEOUT;
			while (((DMA2->LISR & (1 << 27)) == RESET) && timeout)
				timeout--;	  // 等待传输完成
			if (timeout == 0) // 超时
			{
				SD_Init();				// 重新初始化SD卡,可以解决写入死机的问题
				return SD_DATA_TIMEOUT; // 超时
			}
			timeout = SDIO_DATATIMEOUT;
			while ((TransferEnd == 0) && (TransferError == SD_OK) && timeout)
				timeout--;
			if (timeout == 0)
				return SD_DATA_TIMEOUT; // 超时
			if (TransferError != SD_OK)
				return TransferError;
		}
	}
	SDIO_ClearFlag(SDIO_STATIC_FLAGS); // 清除所有标记
	errorstatus = IsCardProgramming(&cardstate);
	while ((errorstatus == SD_OK) && ((cardstate == SD_CARD_PROGRAMMING) || (cardstate == SD_CARD_RECEIVING)))
	{
		errorstatus = IsCardProgramming(&cardstate);
	}
	return errorstatus;
}
// SDIO中断服务函数
void SDIO_IRQHandler(void)
{
	SD_ProcessIRQSrc(); // 处理所有SDIO相关中断
}
// SDIO中断处理函数
// 处理SDIO传输过程中的各种中断事务
// 返回值:错误代码
SD_Error SD_ProcessIRQSrc(void)
{
	if (SDIO_GetFlagStatus(SDIO_FLAG_DATAEND) != RESET) // 接收完成中断
	{
		if (StopCondition == 1)
		{
			SDIO_CmdInitStructure.SDIO_Argument = 0; // 发送CMD12+结束传输
			SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_STOP_TRANSMISSION;
			SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
			SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
			SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
			SDIO_SendCommand(&SDIO_CmdInitStructure);

			TransferError = CmdResp1Error(SD_CMD_STOP_TRANSMISSION);
		}
		else
			TransferError = SD_OK;
		SDIO->ICR |= 1 << 8;																					  // 清除完成中断标记
		SDIO->MASK &= ~((1 << 1) | (1 << 3) | (1 << 8) | (1 << 14) | (1 << 15) | (1 << 4) | (1 << 5) | (1 << 9)); // 关闭相关中断
		TransferEnd = 1;
		return (TransferError);
	}
	if (SDIO_GetFlagStatus(SDIO_FLAG_DCRCFAIL) != RESET) // 数据CRC错误
	{
		SDIO_ClearFlag(SDIO_FLAG_DCRCFAIL);																		  // 清错误标志
		SDIO->MASK &= ~((1 << 1) | (1 << 3) | (1 << 8) | (1 << 14) | (1 << 15) | (1 << 4) | (1 << 5) | (1 << 9)); // 关闭相关中断
		TransferError = SD_DATA_CRC_FAIL;
		return (SD_DATA_CRC_FAIL);
	}
	if (SDIO_GetFlagStatus(SDIO_FLAG_DTIMEOUT) != RESET) // 数据超时错误
	{
		SDIO_ClearFlag(SDIO_FLAG_DTIMEOUT);																		  // 清中断标志
		SDIO->MASK &= ~((1 << 1) | (1 << 3) | (1 << 8) | (1 << 14) | (1 << 15) | (1 << 4) | (1 << 5) | (1 << 9)); // 关闭相关中断
		TransferError = SD_DATA_TIMEOUT;
		return (SD_DATA_TIMEOUT);
	}
	if (SDIO_GetFlagStatus(SDIO_FLAG_RXOVERR) != RESET) // FIFO上溢错误
	{
		SDIO_ClearFlag(SDIO_FLAG_RXOVERR);																		  // 清中断标志
		SDIO->MASK &= ~((1 << 1) | (1 << 3) | (1 << 8) | (1 << 14) | (1 << 15) | (1 << 4) | (1 << 5) | (1 << 9)); // 关闭相关中断
		TransferError = SD_RX_OVERRUN;
		return (SD_RX_OVERRUN);
	}
	if (SDIO_GetFlagStatus(SDIO_FLAG_TXUNDERR) != RESET) // FIFO下溢错误
	{
		SDIO_ClearFlag(SDIO_FLAG_TXUNDERR);																		  // 清中断标志
		SDIO->MASK &= ~((1 << 1) | (1 << 3) | (1 << 8) | (1 << 14) | (1 << 15) | (1 << 4) | (1 << 5) | (1 << 9)); // 关闭相关中断
		TransferError = SD_TX_UNDERRUN;
		return (SD_TX_UNDERRUN);
	}
	if (SDIO_GetFlagStatus(SDIO_FLAG_STBITERR) != RESET) // 起始位错误
	{
		SDIO_ClearFlag(SDIO_FLAG_STBITERR);																		  // 清中断标志
		SDIO->MASK &= ~((1 << 1) | (1 << 3) | (1 << 8) | (1 << 14) | (1 << 15) | (1 << 4) | (1 << 5) | (1 << 9)); // 关闭相关中断
		TransferError = SD_START_BIT_ERR;
		return (SD_START_BIT_ERR);
	}
	return (SD_OK);
}

// 检查CMD0的执行状态
// 返回值:sd卡错误码
SD_Error CmdError(void)
{
	SD_Error errorstatus = SD_OK;
	u32 timeout = SDIO_CMD0TIMEOUT;
	while (timeout--)
	{
		if (SDIO_GetFlagStatus(SDIO_FLAG_CMDSENT) != RESET)
			break; // 命令已发送(无需响应)
	}
	if (timeout == 0)
		return SD_CMD_RSP_TIMEOUT;
	SDIO_ClearFlag(SDIO_STATIC_FLAGS); // 清除所有标记
	return errorstatus;
}
// 检查R7响应的错误状态
// 返回值:sd卡错误码
SD_Error CmdResp7Error(void)
{
	SD_Error errorstatus = SD_OK;
	u32 status;
	u32 timeout = SDIO_CMD0TIMEOUT;
	while (timeout--)
	{
		status = SDIO->STA;
		if (status & ((1 << 0) | (1 << 2) | (1 << 6)))
			break; // CRC错误/命令响应超时/已经收到响应(CRC校验成功)
	}
	if ((timeout == 0) || (status & (1 << 2))) // 响应超时
	{
		errorstatus = SD_CMD_RSP_TIMEOUT;	// 当前卡不是2.0兼容卡,或者不支持设定的电压范围
		SDIO_ClearFlag(SDIO_FLAG_CTIMEOUT); // 清除命令响应超时标志
		return errorstatus;
	}
	if (status & 1 << 6) // 成功接收到响应
	{
		errorstatus = SD_OK;
		SDIO_ClearFlag(SDIO_FLAG_CMDREND); // 清除响应标志
	}
	return errorstatus;
}
// 检查R1响应的错误状态
// cmd:当前命令
// 返回值:sd卡错误码
SD_Error CmdResp1Error(u8 cmd)
{
	u32 status;
	while (1)
	{
		status = SDIO->STA;
		if (status & ((1 << 0) | (1 << 2) | (1 << 6)))
			break; // CRC错误/命令响应超时/已经收到响应(CRC校验成功)
	}
	if (SDIO_GetFlagStatus(SDIO_FLAG_CTIMEOUT) != RESET) // 响应超时
	{
		SDIO_ClearFlag(SDIO_FLAG_CTIMEOUT); // 清除命令响应超时标志
		return SD_CMD_RSP_TIMEOUT;
	}
	if (SDIO_GetFlagStatus(SDIO_FLAG_CCRCFAIL) != RESET) // CRC错误
	{
		SDIO_ClearFlag(SDIO_FLAG_CCRCFAIL); // 清除标志
		return SD_CMD_CRC_FAIL;
	}
	if (SDIO->RESPCMD != cmd)
		return SD_ILLEGAL_CMD;						   // 命令不匹配
	SDIO_ClearFlag(SDIO_STATIC_FLAGS);				   // 清除所有标记
	return (SD_Error)(SDIO->RESP1 & SD_OCR_ERRORBITS); // 返回卡响应
}
// 检查R3响应的错误状态
// 返回值:错误状态
SD_Error CmdResp3Error(void)
{
	u32 status;
	while (1)
	{
		status = SDIO->STA;
		if (status & ((1 << 0) | (1 << 2) | (1 << 6)))
			break; // CRC错误/命令响应超时/已经收到响应(CRC校验成功)
	}
	if (SDIO_GetFlagStatus(SDIO_FLAG_CTIMEOUT) != RESET) // 响应超时
	{
		SDIO_ClearFlag(SDIO_FLAG_CTIMEOUT); // 清除命令响应超时标志
		return SD_CMD_RSP_TIMEOUT;
	}
	SDIO_ClearFlag(SDIO_STATIC_FLAGS); // 清除所有标记
	return SD_OK;
}
// 检查R2响应的错误状态
// 返回值:错误状态
SD_Error CmdResp2Error(void)
{
	SD_Error errorstatus = SD_OK;
	u32 status;
	u32 timeout = SDIO_CMD0TIMEOUT;
	while (timeout--)
	{
		status = SDIO->STA;
		if (status & ((1 << 0) | (1 << 2) | (1 << 6)))
			break; // CRC错误/命令响应超时/已经收到响应(CRC校验成功)
	}
	if ((timeout == 0) || (status & (1 << 2))) // 响应超时
	{
		errorstatus = SD_CMD_RSP_TIMEOUT;
		SDIO_ClearFlag(SDIO_FLAG_CTIMEOUT); // 清除命令响应超时标志
		return errorstatus;
	}
	if (SDIO_GetFlagStatus(SDIO_FLAG_CCRCFAIL) != RESET) // CRC错误
	{
		errorstatus = SD_CMD_CRC_FAIL;
		SDIO_ClearFlag(SDIO_FLAG_CCRCFAIL); // 清除响应标志
	}
	SDIO_ClearFlag(SDIO_STATIC_FLAGS); // 清除所有标记
	return errorstatus;
}
// Check R6 response error status
// cmd:Previous sent command
// prca:Card returned RCA address
// Return value: Error status
SD_Error CmdResp6Error(u8 cmd, u16 *prca)
{
	SD_Error errorstatus = SD_OK;
	u32 status;
	u32 rspr1;
	while (1)
	{
		status = SDIO->STA;
		if (status & ((1 << 0) | (1 << 2) | (1 << 6)))
			break; // CRC error/command response timeout/received response (CRC check successful)
	}
	if (SDIO_GetFlagStatus(SDIO_FLAG_CTIMEOUT) != RESET) // Response timeout
	{
		SDIO_ClearFlag(SDIO_FLAG_CTIMEOUT); // Clear command response timeout flag
		return SD_CMD_RSP_TIMEOUT;
	}
	if (SDIO_GetFlagStatus(SDIO_FLAG_CCRCFAIL) != RESET) // CRC error
	{
		SDIO_ClearFlag(SDIO_FLAG_CCRCFAIL); // Clear response flag
		return SD_CMD_CRC_FAIL;
	}
	if (SDIO->RESPCMD != cmd) // Check if the response is for the cmd command
	{
		return SD_ILLEGAL_CMD;
	}
	SDIO_ClearFlag(SDIO_STATIC_FLAGS); // Clear all flags
	rspr1 = SDIO->RESP1;			   // Get response
	if (SD_ALLZERO == (rspr1 & (SD_R6_GENERAL_UNKNOWN_ERROR | SD_R6_ILLEGAL_CMD | SD_R6_COM_CRC_FAILED)))
	{
		*prca = (u16)(rspr1 >> 16); // Right shift 16 bits to get rca
		return errorstatus;
	}
	if (rspr1 & SD_R6_GENERAL_UNKNOWN_ERROR)
		return SD_GENERAL_UNKNOWN_ERROR;
	if (rspr1 & SD_R6_ILLEGAL_CMD)
		return SD_ILLEGAL_CMD;
	if (rspr1 & SD_R6_COM_CRC_FAILED)
		return SD_COM_CRC_FAILED;
	return errorstatus;
}
// Enable wide bus mode
// enx:0, not enable; 1, enable;
// Return value: Error status
SD_Error SDEnWideBus(u8 enx)
{
	SD_Error errorstatus = SD_OK;
	u32 scr[2] = {0, 0};
	u8 arg = 0X00;
	if (enx)
		arg = 0X02;
	else
		arg = 0X00;
	if (SDIO->RESP1 & SD_CARD_LOCKED)
		return SD_LOCK_UNLOCK_FAILED; // SD card is LOCKED
	errorstatus = FindSCR(RCA, scr);  // Get SCR register data
	if (errorstatus != SD_OK)
		return errorstatus;
	if ((scr[1] & SD_WIDE_BUS_SUPPORT) != SD_ALLZERO) // Wide bus support
	{
		SDIO_CmdInitStructure.SDIO_Argument = (uint32_t)RCA << 16; // Send CMD55+RCA, short response
		SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_APP_CMD;
		SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
		SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
		SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
		SDIO_SendCommand(&SDIO_CmdInitStructure);

		errorstatus = CmdResp1Error(SD_CMD_APP_CMD);

		if (errorstatus != SD_OK)
			return errorstatus;

		SDIO_CmdInitStructure.SDIO_Argument = arg; // Send ACMD6, short response, parameter: 10, 4 bits; 00, 1 bit.
		SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_APP_SD_SET_BUSWIDTH;
		SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
		SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
		SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
		SDIO_SendCommand(&SDIO_CmdInitStructure);

		errorstatus = CmdResp1Error(SD_CMD_APP_SD_SET_BUSWIDTH);

		return errorstatus;
	}
	else
		return SD_REQUEST_NOT_APPLICABLE; // Not supported wide bus setting
}
// Check if the card is writing
// pstatus:Current status.
// Return value: Error code
SD_Error IsCardProgramming(u8 *pstatus)
{
	vu32 respR1 = 0, status = 0;

	SDIO_CmdInitStructure.SDIO_Argument = (uint32_t)RCA << 16; // Card relative address parameter
	SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SEND_STATUS;  // Send CMD13
	SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
	SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
	SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
	SDIO_SendCommand(&SDIO_CmdInitStructure);

	status = SDIO->STA;

	while (!(status & ((1 << 0) | (1 << 6) | (1 << 2))))
		status = SDIO->STA;								 // Wait for operation to complete
	if (SDIO_GetFlagStatus(SDIO_FLAG_CCRCFAIL) != RESET) // CRC check failed
	{
		SDIO_ClearFlag(SDIO_FLAG_CCRCFAIL); // Clear error flag
		return SD_CMD_CRC_FAIL;
	}
	if (SDIO_GetFlagStatus(SDIO_FLAG_CTIMEOUT) != RESET) // Command timeout
	{
		SDIO_ClearFlag(SDIO_FLAG_CTIMEOUT); // Clear error flag
		return SD_CMD_RSP_TIMEOUT;
	}
	if (SDIO->RESPCMD != SD_CMD_SEND_STATUS)
		return SD_ILLEGAL_CMD;
	SDIO_ClearFlag(SDIO_STATIC_FLAGS); // Clear all flags
	respR1 = SDIO->RESP1;
	*pstatus = (u8)((respR1 >> 9) & 0x0000000F);
	return SD_OK;
}
// Read current card status
// pcardstatus:Card status
// Return value: Error code
SD_Error SD_SendStatus(uint32_t *pcardstatus)
{
	SD_Error errorstatus = SD_OK;
	if (pcardstatus == NULL)
	{
		errorstatus = SD_INVALID_PARAMETER;
		return errorstatus;
	}

	SDIO_CmdInitStructure.SDIO_Argument = (uint32_t)RCA << 16; // Send CMD13, short response
	SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SEND_STATUS;
	SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
	SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
	SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
	SDIO_SendCommand(&SDIO_CmdInitStructure);

	errorstatus = CmdResp1Error(SD_CMD_SEND_STATUS); // Query response status
	if (errorstatus != SD_OK)
	{
		return errorstatus;
	}
	*pcardstatus = SDIO->RESP1; // Read response value

	return errorstatus;
}
// Return SD card status
// Return value: SD card status
SDCardState SD_GetState(void)
{
	u32 resp1 = 0;
	if (SD_SendStatus(&resp1) != SD_OK)
	{
		return SD_CARD_ERROR;
	}
	else
	{
		return (SDCardState)((resp1 >> 9) & 0x0F);
	}
}
// Find the SCR register value of the SD card
// rca:Card relative address
// pscr:Data buffer (stores SCR content)
// Return value: Error status
SD_Error FindSCR(u16 rca, u32 *pscr)
{
	u32 index = 0;
	SD_Error errorstatus = SD_OK;
	u32 tempscr[2] = {0, 0};

	SDIO_CmdInitStructure.SDIO_Argument = (uint32_t)8;		   // Send CMD16, short response, set Block Size to 8 bytes
	SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SET_BLOCKLEN; // cmd16
	SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short; // r1
	SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
	SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
	SDIO_SendCommand(&SDIO_CmdInitStructure);

	errorstatus = CmdResp1Error(SD_CMD_SET_BLOCKLEN);

	if (errorstatus != SD_OK)
		return errorstatus;

	SDIO_CmdInitStructure.SDIO_Argument = (uint32_t)RCA << 16;
	SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_APP_CMD; // Send CMD55, short response
	SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
	SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
	SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
	SDIO_SendCommand(&SDIO_CmdInitStructure);

	errorstatus = CmdResp1Error(SD_CMD_APP_CMD);
	if (errorstatus != SD_OK)
		return errorstatus;

	SDIO_DataInitStructure.SDIO_DataTimeOut = SD_DATATIMEOUT;
	SDIO_DataInitStructure.SDIO_DataLength = 8;						   // 8 bytes, block is 8 bytes, SD card to SDIO.
	SDIO_DataInitStructure.SDIO_DataBlockSize = SDIO_DataBlockSize_8b; // Block size 8 bytes
	SDIO_DataInitStructure.SDIO_TransferDir = SDIO_TransferDir_ToSDIO;
	SDIO_DataInitStructure.SDIO_TransferMode = SDIO_TransferMode_Block;
	SDIO_DataInitStructure.SDIO_DPSM = SDIO_DPSM_Enable;
	SDIO_DataConfig(&SDIO_DataInitStructure);

	SDIO_CmdInitStructure.SDIO_Argument = 0x0;
	SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SD_APP_SEND_SCR; // Send ACMD51, short response, parameter is 0
	SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;	  // r1
	SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
	SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
	SDIO_SendCommand(&SDIO_CmdInitStructure);

	errorstatus = CmdResp1Error(SD_CMD_SD_APP_SEND_SCR);
	if (errorstatus != SD_OK)
		return errorstatus;
	while (!(SDIO->STA & (SDIO_FLAG_RXOVERR | SDIO_FLAG_DCRCFAIL | SDIO_FLAG_DTIMEOUT | SDIO_FLAG_DBCKEND | SDIO_FLAG_STBITERR)))
	{
		if (SDIO_GetFlagStatus(SDIO_FLAG_RXDAVL) != RESET) // Receive FIFO data available
		{
			*(tempscr + index) = SDIO->FIFO; // Read FIFO content
			index++;
			if (index >= 2)
				break;
		}
	}
	if (SDIO_GetFlagStatus(SDIO_FLAG_DTIMEOUT) != RESET) // Data timeout error
	{
		SDIO_ClearFlag(SDIO_FLAG_DTIMEOUT); // Clear error flag
		return SD_DATA_TIMEOUT;
	}
	else if (SDIO_GetFlagStatus(SDIO_FLAG_DCRCFAIL) != RESET) // Data block CRC error
	{
		SDIO_ClearFlag(SDIO_FLAG_DCRCFAIL); // Clear error flag
		return SD_DATA_CRC_FAIL;
	}
	else if (SDIO_GetFlagStatus(SDIO_FLAG_RXOVERR) != RESET) // Receive FIFO overflow error
	{
		SDIO_ClearFlag(SDIO_FLAG_RXOVERR); // Clear error flag
		return SD_RX_OVERRUN;
	}
	else if (SDIO_GetFlagStatus(SDIO_FLAG_STBITERR) != RESET) // Receive start bit error
	{
		SDIO_ClearFlag(SDIO_FLAG_STBITERR); // Clear error flag
		return SD_START_BIT_ERR;
	}
	SDIO_ClearFlag(SDIO_STATIC_FLAGS); // Clear all flags
	// Reverse the data in 8-bit units.
	*(pscr + 1) = ((tempscr[0] & SD_0TO7BITS) << 24) | ((tempscr[0] & SD_8TO15BITS) << 8) | ((tempscr[0] & SD_16TO23BITS) >> 8) | ((tempscr[0] & SD_24TO31BITS) >> 24);
	*(pscr) = ((tempscr[1] & SD_0TO7BITS) << 24) | ((tempscr[1] & SD_8TO15BITS) << 8) | ((tempscr[1] & SD_16TO23BITS) >> 8) | ((tempscr[1] & SD_24TO31BITS) >> 24);
	return errorstatus;
}
// Get the exponent of 2 for NumberOfBytes.
// NumberOfBytes:Number of bytes.
// Return value: Exponent of 2
u8 convert_from_bytes_to_power_of_two(u16 NumberOfBytes)
{
	u8 count = 0;
	while (NumberOfBytes != 1)
	{
		NumberOfBytes >>= 1;
		count++;
	}
	return count;
}

// Configure SDIO DMA
// mbuf:Memory address
// bufsize:Transfer data amount
// dir:方向;DMA_DIR_MemoryToPeripheral  存储器-->SDIO(写数据);DMA_DIR_PeripheralToMemory SDIO-->存储器(读数据);
void SD_DMA_Config(u32 *mbuf, u32 bufsize, u32 dir)
{

	DMA_InitTypeDef DMA_InitStructure;

	while (DMA_GetCmdStatus(DMA2_Stream6) != DISABLE)
	{
	} // Wait for DMA to be configurable

	DMA_DeInit(DMA2_Stream6); // Clear all interrupt flags on stream6

	DMA_InitStructure.DMA_Channel = DMA_Channel_4;							// Channel selection
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&SDIO->FIFO;			// DMA peripheral address
	DMA_InitStructure.DMA_Memory0BaseAddr = (u32)mbuf;						// DMA memory0 address
	DMA_InitStructure.DMA_DIR = dir;										// Memory to peripheral mode
	DMA_InitStructure.DMA_BufferSize = 0;									// Data transfer amount
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;		// Peripheral non-increment mode
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;					// Memory increment mode
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word; // Peripheral data length: 32 bits
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Word;			// Memory data length: 32 bits
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;							// Use normal mode
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;					// Highest priority
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Enable;					// FIFO enabled
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;			// Full FIFO
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_INC4;				// Memory burst 4 transfers
	DMA_Init(DMA2_Stream6, &DMA_InitStructure);								// Initialize DMA Stream

	DMA_FlowControllerConfig(DMA2_Stream6, DMA_FlowCtrl_Peripheral); // Peripheral flow control

	DMA_Cmd(DMA2_Stream6, ENABLE); // Enable DMA transfer
}

// Read SD card
// buf:Read data buffer
// sector:Sector address
// cnt:Number of sectors
// Return value: Error status; 0, normal; other, error code;
u8 SD_ReadDisk(u8 *buf, u32 sector, u8 cnt)
{
	u8 sta = SD_OK;

	long long lsector = sector;
	u8 n;
	if (CardType != SDIO_STD_CAPACITY_SD_CARD_V1_1)
		lsector <<= 9;

	if ((u32)buf % 4 != 0)
	{
		for (n = 0; n < cnt; n++)
		{

			sta = SD_ReadBlock(SDIO_DATA_BUFFER, lsector + 512 * n, 512); // 单个sector的读操作
			memcpy(buf, SDIO_DATA_BUFFER, 512);
			buf += 512;
		}
	}
	else
	{
		if (cnt == 1)
			sta = SD_ReadBlock(buf, lsector, 512); // 单个sector的读操作
		else
			sta = SD_ReadMultiBlocks(buf, lsector, 512, cnt); // 多个sector
	}

	return sta;
}
// Write SD card
// buf:Write data buffer
// sector:Sector address
// cnt:Number of sectors
// Return value: Error status; 0, normal; other, error code;
u8 SD_WriteDisk(u8 *buf, u32 sector, u8 cnt)
{
	u8 sta = SD_OK;
	u8 n;
	long long lsector = sector;
	if (CardType != SDIO_STD_CAPACITY_SD_CARD_V1_1)
		lsector <<= 9;
	if ((u32)buf % 4 != 0)
	{
		for (n = 0; n < cnt; n++)
		{
			memcpy(SDIO_DATA_BUFFER, buf, 512);
			sta = SD_WriteBlock(SDIO_DATA_BUFFER, lsector + 512 * n, 512); // 单个sector的写操作
			buf += 512;
		}
	}
	else
	{
		if (cnt == 1)
			sta = SD_WriteBlock(buf, lsector, 512); // 单个sector的写操作
		else
			sta = SD_WriteMultiBlocks(buf, lsector, 512, cnt); // 多个sector
	}
	return sta;
}
