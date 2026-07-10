#ifndef MPU6050_HAL_PORT_H
#define MPU6050_HAL_PORT_H

#include "sys.h"

#define MPU_ADDR 0x68
#define MPU_DEVICE_ID_REG 0x75
#define MPU_TEMP_OUTH_REG 0x41
#define MPU_GYRO_XOUTH_REG 0x43
#define MPU_ACCEL_XOUTH_REG 0x3B

u8 MPU_Init(void);
u8 MPU_Write_Len(u8 addr, u8 reg, u8 len, u8 *buf);
u8 MPU_Read_Len(u8 addr, u8 reg, u8 len, u8 *buf);
u8 MPU_Write_Byte(u8 reg, u8 data);
u8 MPU_Read_Byte(u8 reg);
short MPU_Get_Temperature(void);
u8 MPU_Get_Gyroscope(long *gx, long *gy, long *gz);
u8 MPU_Get_Accelerometer(short *ax, short *ay, short *az);

#endif
