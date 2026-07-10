#include "mpu6050_hal_port.h"
#include "delay.h"

static uint16_t mpu_i2c_addr(u8 addr)
{
    return (uint16_t)(addr << 1);
}

u8 MPU_Write_Len(u8 addr, u8 reg, u8 len, u8 *buf)
{
    return (HAL_I2C_Mem_Write(&hi2c1, mpu_i2c_addr(addr), reg, I2C_MEMADD_SIZE_8BIT, buf, len, 100U) == HAL_OK) ? 0 : 1;
}

u8 MPU_Read_Len(u8 addr, u8 reg, u8 len, u8 *buf)
{
    return (HAL_I2C_Mem_Read(&hi2c1, mpu_i2c_addr(addr), reg, I2C_MEMADD_SIZE_8BIT, buf, len, 100U) == HAL_OK) ? 0 : 1;
}

u8 MPU_Write_Byte(u8 reg, u8 data)
{
    return MPU_Write_Len(MPU_ADDR, reg, 1, &data);
}

u8 MPU_Read_Byte(u8 reg)
{
    u8 data = 0;
    MPU_Read_Len(MPU_ADDR, reg, 1, &data);
    return data;
}

u8 MPU_Init(void)
{
    return (MPU_Read_Byte(MPU_DEVICE_ID_REG) == MPU_ADDR) ? 0 : 1;
}

short MPU_Get_Temperature(void)
{
    u8 buf[2] = {0};
    short raw;
    float temp;

    MPU_Read_Len(MPU_ADDR, MPU_TEMP_OUTH_REG, 2, buf);
    raw = (short)(((u16)buf[0] << 8) | buf[1]);
    temp = 36.53f + ((float)raw / 340.0f);
    return (short)(temp * 100.0f);
}

u8 MPU_Get_Gyroscope(long *gx, long *gy, long *gz)
{
    u8 buf[6];
    u8 res = MPU_Read_Len(MPU_ADDR, MPU_GYRO_XOUTH_REG, 6, buf);
    if (res == 0) {
        *gx = (short)(((u16)buf[0] << 8) | buf[1]);
        *gy = (short)(((u16)buf[2] << 8) | buf[3]);
        *gz = (short)(((u16)buf[4] << 8) | buf[5]);
    }
    return res;
}

u8 MPU_Get_Accelerometer(short *ax, short *ay, short *az)
{
    u8 buf[6];
    u8 res = MPU_Read_Len(MPU_ADDR, MPU_ACCEL_XOUTH_REG, 6, buf);
    if (res == 0) {
        *ax = (short)(((u16)buf[0] << 8) | buf[1]);
        *ay = (short)(((u16)buf[2] << 8) | buf[3]);
        *az = (short)(((u16)buf[4] << 8) | buf[5]);
    }
    return res;
}
