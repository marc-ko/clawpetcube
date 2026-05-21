/*-----------------------------------------------------------------------*/
/* Low level disk I/O module SKELETON for FatFs     (C)ChaN, 2019        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "ff.h"			/* Obtains integer types */
#include "diskio.h"		/* Declarations of disk functions */
#include <stdint.h>        /* Definitions of uint8_t and other types */
#include "../../HARDWARE/SDIO/sdio.h"    /* SDIO_SD driver header file */


/* Definitions of physical drive number for each drive */
#define DEV_SD_CARD        0    //SD Card
#define DEV_SPI_FLASH            1    //SPI FLASH
#define DEV_USB            2    //USB

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
){
      
        
        return RES_OK;

}




/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
	DSTATUS stat;
	int result;

  
            
            if(SD_Init() == SD_OK)    
            {
                stat = RES_OK;
            }else{
                stat = STA_NOINIT;
            }
           
            
            return stat;



}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	LBA_t sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
)
{
	DRESULT res;
	int result;

	 result = SD_ReadDisk(buff, sector, count);
            if(result != 0)
            {
                res = RES_PARERR;
            }else{
                res = RES_OK;
            }
            return res;


                          
	

	return RES_PARERR;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	LBA_t sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */
)
{
	DRESULT res;
	int result;

	 result = SD_WriteDisk((uint8_t *)buff, sector, count);
            if(result != 0)
            {
                res = RES_PARERR;
            }else{
                res = RES_OK;
            }
            return res;

}

#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	DRESULT res;
	int result;

	switch (cmd) 
        {
            
        case GET_SECTOR_COUNT:    /* 扇区数量 */
            *(DWORD * )buff = SDCardInfo.CardCapacity/SDCardInfo.CardBlockSize;        
            break;
                    
        case GET_SECTOR_SIZE :    /* 扇区大小  */
            *(WORD * )buff = SDCardInfo.CardBlockSize;
            break;
        
        case GET_BLOCK_SIZE :    /* 同时擦除扇区个数(单位为扇区) */
            *(DWORD * )buff = 1;
            break;        
        }
        res = RES_OK;
        return res;

}

