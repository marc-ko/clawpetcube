/**
 * @file lv_fs_fatfs.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "../../../lvgl.h"

#if LV_USE_FS_FATFS
#include "../../../../../../../FatFs/ff.h"
#include "../../../../../../../../HARDWARE/SDIO/sdio.h"
/* Headers include */
#include <stdio.h>

/*********************
 *      DEFINES
 *********************/

#if LV_FS_FATFS_LETTER == '\0'
#error "LV_FS_FATFS_LETTER must be an upper case ASCII letter"
#endif

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void fs_init(void);

static void *fs_open(lv_fs_drv_t *drv, const char *path, lv_fs_mode_t mode);
static lv_fs_res_t fs_close(lv_fs_drv_t *drv, void *file_p);
static lv_fs_res_t fs_read(lv_fs_drv_t *drv, void *file_p, void *buf, uint32_t btr, uint32_t *br);
static lv_fs_res_t fs_write(lv_fs_drv_t *drv, void *file_p, const void *buf, uint32_t btw, uint32_t *bw);
static lv_fs_res_t fs_seek(lv_fs_drv_t *drv, void *file_p, uint32_t pos, lv_fs_whence_t whence);
static lv_fs_res_t fs_tell(lv_fs_drv_t *drv, void *file_p, uint32_t *pos_p);
static void *fs_dir_open(lv_fs_drv_t *drv, const char *path);
static lv_fs_res_t fs_dir_read(lv_fs_drv_t *drv, void *dir_p, char *fn);
static lv_fs_res_t fs_dir_close(lv_fs_drv_t *drv, void *dir_p);

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void lv_fs_fatfs_init(void)
{
    /*----------------------------------------------------
     * Initialize your storage device and File System
     * -------------------------------------------------*/
    fs_init();

    /*---------------------------------------------------
     * Register the file system interface in LVGL
     *--------------------------------------------------*/

    /*Add a simple drive to open images*/
    static lv_fs_drv_t fs_drv; /*A driver descriptor*/
    lv_fs_drv_init(&fs_drv);

    /*Set up fields...*/
    fs_drv.letter = LV_FS_FATFS_LETTER;
    fs_drv.cache_size = LV_FS_FATFS_CACHE_SIZE;

    fs_drv.open_cb = fs_open;
    fs_drv.close_cb = fs_close;
    fs_drv.read_cb = fs_read;
    fs_drv.write_cb = fs_write;
    fs_drv.seek_cb = fs_seek;
    fs_drv.tell_cb = fs_tell;

    fs_drv.dir_close_cb = fs_dir_close;
    fs_drv.dir_open_cb = fs_dir_open;
    fs_drv.dir_read_cb = fs_dir_read;

    lv_fs_drv_register(&fs_drv);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/**
 * @brief       Initialize the storage device and File System
 * @param       N/A
 * @retval      N/A
 */
FATFS FatFs;

// static FIL fnew;
// static FRESULT res_sd;
// static UINT fnum;
// static BYTE ReadBuffer[256]= {0};
// static BYTE WriteBuffer[] =  "Hello World!";

static void fs_init(void)
{
    /*Initialize the SD card and FatFS itself.
     *Better to do it in your code to keep this library untouched for easy updating*/
    // uint8_t res;
    // static FATFS fs;  // Declare the fs variable

    // /* Initialize SD card and FatFS itself.
    //  * Better to do it in your code to keep this library untouched for easy updating*/
    // while (SD_Init())               /* Initialize SD card */
    // {
    //    // lcd_show_string(10, 10, 200, 24, 24, "SD Card Error!", RED);
    //     printf("SD Card Error, Please Check!\r\n");

    //     //LED0_TOGGLE();
    //     //HAL_Delay(200);
    // }

    // //LED0(0);

    // //exfuns_init();
    // /* Mount the SD card */

    // res = f_mount(&fs, "0:", 1);  /*  SD  */

    // if (0 != res)
    // {

    //     //lcd_show_string(10, 40, 200, 24, 24, "SD Card Mount Fail!", RED);
    //     printf("SD Card Mount Fail, Please Check!\r\n");
    //     //LED0_TOGGLE();
    //     //HAL_Delay(200);
    // }
    // printf("SD Card Mount Success!\r\n");
}

/**
 * @brief Open a file
 * @param drv: the driver for finding
 * @param path Uses a one char as the drive letter (e.g. "S:/folder/file.txt")
 * @param mode: FS_MODE_RD, write: FS_MODE_WR, both: FS_MODE_RD | FS_MODE_WR
 * @retval non NULL: Success, NULL pointer if error
 */
static void *fs_open(lv_fs_drv_t *drv, const char *path, lv_fs_mode_t mode)
{
    LV_UNUSED(drv);
    uint8_t flags = 0;

    if (mode == LV_FS_MODE_WR)
        flags = FA_WRITE | FA_OPEN_ALWAYS;
    else if (mode == LV_FS_MODE_RD)
        flags = FA_READ;
    else if (mode == (LV_FS_MODE_WR | LV_FS_MODE_RD))
        flags = FA_READ | FA_WRITE | FA_OPEN_ALWAYS;

    FIL *f = lv_mem_alloc(sizeof(FIL));
    if (f == NULL)
        return NULL;

    FRESULT res = f_open(f, path, flags);
    if (res == FR_OK)
    {
        return f;
    }
    else
    {
        lv_mem_free(f);
        return NULL;
    }
}

/**
 * @brief  Close a file
 * @param  drv: the driver for finding
 * @param  file_p: pointer to the file (from lv_ufs_open)
 * @retval LV_FS_RES_OK: Success, lv_fs_res_t enum if error
 */
static lv_fs_res_t fs_close(lv_fs_drv_t *drv, void *file_p)
{
    LV_UNUSED(drv);
    f_close(file_p);
    lv_mem_free(file_p);
    return LV_FS_RES_OK;
}

/**
 * @brief Read from a file
 * @param drv: the driver for finding
 * @param file_p: pointer to the file (from lv_ufs_open)
 * @param buf: pointer to the buffer to read into
 * @param btr: number of bytes to read
 * @param br: actual number of bytes read (bytes read)
 * @retval LV_FS_RES_OK: Success, lv_fs_res_t enum if error
 */
static lv_fs_res_t fs_read(lv_fs_drv_t *drv, void *file_p, void *buf, uint32_t btr, uint32_t *br)
{
    LV_UNUSED(drv);
    FRESULT res = f_read(file_p, buf, btr, (UINT *)br);
    if (res == FR_OK)
        return LV_FS_RES_OK;
    else
        return LV_FS_RES_UNKNOWN;
}

/**
 * @brief  Write to a file
 * @param  drv: the driver for finding
 * @param  file_p: pointer to the file (from lv_ufs_open)
 * @param  buf: pointer to the buffer to write from
 * @param  btw: number of bytes to write
 * @param  br: actual number of bytes written (bytes written)
 * @retval LV_FS_RES_OK: Success, lv_fs_res_t enum if error
 */
static lv_fs_res_t fs_write(lv_fs_drv_t *drv, void *file_p, const void *buf, uint32_t btw, uint32_t *bw)
{
    LV_UNUSED(drv);
    FRESULT res = f_write(file_p, buf, btw, (UINT *)bw);
    if (res == FR_OK)
        return LV_FS_RES_OK;
    else
        return LV_FS_RES_UNKNOWN;
}

/**
 * @brief Seek to a position
 * @param  drv: the driver for finding
 * @param  file_p: pointer to the file (from lv_ufs_open)
 * @param  pos: the position to write to
 * @retval LV_FS_RES_OK: Success, lv_fs_res_t enum if error
 */
static lv_fs_res_t fs_seek(lv_fs_drv_t *drv, void *file_p, uint32_t pos, lv_fs_whence_t whence)
{
    LV_UNUSED(drv);
    switch (whence)
    {
    case LV_FS_SEEK_SET:
        f_lseek(file_p, pos);
        break;
    case LV_FS_SEEK_CUR:
        f_lseek(file_p, f_tell((FIL *)file_p) + pos);
        break;
    case LV_FS_SEEK_END:
        f_lseek(file_p, f_size((FIL *)file_p) + pos);
        break;
    default:
        break;
    }
    return LV_FS_RES_OK;
}

/**
 * @brief Get the current position
 * @param  drv: the driver for finding
 * @param  file_p: pointer to the file (from lv_ufs_open)
 * @param  pos_p: pointer to store the position
 * @retval LV_FS_RES_OK: Success, lv_fs_res_t enum if error
 */
static lv_fs_res_t fs_tell(lv_fs_drv_t *drv, void *file_p, uint32_t *pos_p)
{
    LV_UNUSED(drv);
    *pos_p = f_tell((FIL *)file_p);
    return LV_FS_RES_OK;
}

/**
 s* @brief Open a directory
 * @param  drv: the driver for finding
 * @param  rddir_p: pointer to 'lv_fs_dir_t'
 * @param  path: directory path
 * @retval pointer to the opened directory
 */
static void *fs_dir_open(lv_fs_drv_t *drv, const char *path)
{
    LV_UNUSED(drv);
    DIR *d = lv_mem_alloc(sizeof(DIR));
    if (d == NULL)
        return NULL;

    FRESULT res = f_opendir(d, path);
    if (res != FR_OK)
    {
        lv_mem_free(d);
        d = NULL;
    }
    return d;
}

/**
 * @brief Read the first file in a directory
 * @param  drv: the driver for finding
 * @param  rddir_p: pointer to the opened 'lv_fs_dir_t'
 * @param  fn: pointer to the buffer to store the file name
 * @retval LV_FS_RES_OK: Success, lv_fs_res_t enum if error
 */
static lv_fs_res_t fs_dir_read(lv_fs_drv_t *drv, void *dir_p, char *fn)
{
    LV_UNUSED(drv);
    FRESULT res;
    FILINFO fno;
    fn[0] = '\0';

    do
    {
        res = f_readdir(dir_p, &fno);
        if (res != FR_OK)
            return LV_FS_RES_UNKNOWN;

        if (fno.fattrib & AM_DIR)
        {
            fn[0] = '/';
            strcpy(&fn[1], fno.fname);
        }
        else
            strcpy(fn, fno.fname);

    } while (strcmp(fn, "/.") == 0 || strcmp(fn, "/..") == 0);

    return LV_FS_RES_OK;
}

/**
 * @brief Close a directory
 * @param  drv: the driver for finding
 * @param  rddir_p: pointer to the opened 'lv_fs_dir_t'
 * @retval LV_FS_RES_OK: Success, lv_fs_res_t enum if error
 */
static lv_fs_res_t fs_dir_close(lv_fs_drv_t *drv, void *dir_p)
{
    LV_UNUSED(drv);
    f_closedir(dir_p);
    lv_mem_free(dir_p);
    return LV_FS_RES_OK;
}

#else /*LV_USE_FS_FATFS == 0*/

#if defined(LV_FS_FATFS_LETTER) && LV_FS_FATFS_LETTER != '\0'
#warning "LV_USE_FS_FATFS is not enabled but LV_FS_FATFS_LETTER is set"
#endif

#endif /*LV_USE_FS_POSIX*/
