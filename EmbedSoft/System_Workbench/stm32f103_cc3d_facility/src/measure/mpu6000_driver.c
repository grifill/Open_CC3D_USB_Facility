/*******************************************************************************
* File Name          : mpu6000_driver.c
* Author             : Filatov G.U.
* Version            : 0.1.1 - SPI only
* Date               : August 2016
* Description        : MPU6000 driver file.c
*******************************************************************************/
#include "mpu6000_driver.h"

/*******************************************************************************
 * Function Name	: MPU6000_ReadReg
 * Note				: Please select your SPI
 *******************************************************************************/
uint8_t MPU6000_ReadReg(uint8_t Reg, uint8_t* Data) {

	*Data = SPI_Mems_Read_Reg(CS_MPU6000, Reg );
		return 1;
}
/*******************************************************************************
 * Function Name	: MPU6000_WriteReg
 * Note				: Please select your SPI
 *******************************************************************************/
uint8_t MPU6000_WriteReg(uint8_t Reg, uint8_t Data) {
	SPI_Mems_Write_Reg(CS_MPU6000,Reg, Data);
		return 1;
}
/*******************************************************************************
 * Function Name	: MPU6000_Reset
 * Note				: Turn the device into reset and specifies the clock source of the device.
 *******************************************************************************/
status_t MPU6000_Reset(MPU6000_CLKSEL_t cl) {
  uint8_t value;

  if( !MPU6000_ReadReg(MPU6000_PWR_MGMT_1, &value) )
    return MEMS_ERROR;
  value &= 0x80;
  value |= (cl<<MPU6000_CLKSEL);
  if( !MPU6000_WriteReg(MPU6000_PWR_MGMT_1, value) )
    return MEMS_ERROR;

  return MEMS_SUCCESS;
}
/*******************************************************************************
 * Function Name	: MPU6000_I2C_Disable
 * Note				: Disable Primary I2C Interface and FIFO disable
 *******************************************************************************/
status_t MPU6000_I2C_Disable() {
  uint8_t value;

  if( !MPU6000_ReadReg(MPU6000_USER_CTRL, &value) )
    return MEMS_ERROR;
  value &= 0x10;
  if( !MPU6000_WriteReg(MPU6000_USER_CTRL, value) )
    return MEMS_ERROR;

  return MEMS_SUCCESS;
}
/*******************************************************************************
 * Function Name	: MPU6000_Wake_Disable
 * Note				: Disable wake-ups in Accelerometer
 *******************************************************************************/
status_t MPU6000_Wake_Disable() {
  uint8_t value;

  if( !MPU6000_ReadReg(MPU6000_PWR_MGMT_2, &value) )
    return MEMS_ERROR;
  value &= 0x00;
  if( !MPU6000_WriteReg(MPU6000_PWR_MGMT_2, value) )
    return MEMS_ERROR;

  return MEMS_SUCCESS;
}
/*******************************************************************************
 * Function Name	: MPU6000_DefaultDIV
 * Note				: Accel Sample Rate 1000 Hz, Gyro Sample Rate 8000 Hz
 *******************************************************************************/
status_t MPU6000_DefaultDIV() {
	  uint8_t value;

	  if( !MPU6000_ReadReg( MPU6000_SMPLRT_DIV, &value) )
	    return MEMS_ERROR;
	  value &= 0x00;
	  if( !MPU6000_WriteReg( MPU6000_SMPLRT_DIV, value) )
	    return MEMS_ERROR;

	  return MEMS_SUCCESS;
}
/*******************************************************************************
 * Function Name	: MPU6000_DLPF_Config
 * Note				: 3-bit unsigned value. Configures the DLPF setting
 *******************************************************************************/
status_t MPU6000_DLPF_Config(MPU6000_DLFP_t dl) {
	  uint8_t value;

	  if( !MPU6000_ReadReg(MPU6000_CONFIG, &value) )
	    return MEMS_ERROR;
	  value &= 0x00;
	  value |= (dl<<MPU6000_DLPF_CFG);
	  if( !MPU6000_WriteReg(MPU6000_CONFIG, value) )
	    return MEMS_ERROR;

	  return MEMS_SUCCESS;
}
/*******************************************************************************
 * Function Name	: MPU6000_Accel_Config
 * Note				: Accelerometer Configuration
 *******************************************************************************/
status_t MPU6000_Accel_Config(MPU6000_AFS_t adl) {
	  uint8_t value;

	  if( !MPU6000_ReadReg(MPU6000_ACCEL_CONFIG, &value) )
	    return MEMS_ERROR;
	  value &= 0xE0;
	  value |= (adl<<MPU6000_AFS_SEL);
	  if( !MPU6000_WriteReg(MPU6000_ACCEL_CONFIG, value))
	    return MEMS_ERROR;

	  return MEMS_SUCCESS;
}
/*******************************************************************************
 * Function Name	: MPU6000_Gyro_Config
 * Note				: Gyroscope Configuration
 *******************************************************************************/
status_t MPU6000_Gyro_Config(MPU6000_FS_t adl) {
	  uint8_t value;

	  if( !MPU6000_ReadReg(MPU6000_GYRO_CONFIG, &value) )
	    return MEMS_ERROR;
	  value &= 0xE0;
	  value |= (adl<<MPU6000_FS_SEL);
	  if( !MPU6000_WriteReg(MPU6000_GYRO_CONFIG, value) )
	    return MEMS_ERROR;

	  return MEMS_SUCCESS;
}
/*******************************************************************************
* Function Name  : MPU6000_WHO_AM_I_Get
* Note           : Read Data from MPU6000
*******************************************************************************/
status_t MPU6000_WHO_AM_I_Get(MPU_Who_Am_I_t* buff) {
  uint8_t value;

  if( !MPU6000_ReadReg(MPU6000_WHO_AM_I, (uint8_t *)(&value)))
    return MEMS_ERROR;

  //value &=0x7E;
  buff->WHO_MPU = value;

  return MEMS_SUCCESS;
}
/*******************************************************************************
* Function Name  : MPU6000_GetAccAxesRaw
* Note           : Read Data from accelerometer
*******************************************************************************/
status_t MPU6000_GetAccAxesRaw(MPU6000_Acc_AxesRaw_t* buff) {
  int16_t value;
  uint8_t *valueL = (uint8_t *)(&value);
  uint8_t *valueH = ((uint8_t *)(&value)+1);

  if( !MPU6000_ReadReg(MPU6000_ACCEL_XOUT_L, valueL) )
    return MEMS_ERROR;

  if( !MPU6000_ReadReg(MPU6000_ACCEL_XOUT_H, valueH) )
    return MEMS_ERROR;

  buff->AXIS_AX = value; //AXIS X buff

  if( !MPU6000_ReadReg(MPU6000_ACCEL_YOUT_L, valueL) )
    return MEMS_ERROR;

  if( !MPU6000_ReadReg(MPU6000_ACCEL_YOUT_H, valueH) )
    return MEMS_ERROR;

  buff->AXIS_AY = value; //AXIS Y buff

  if( !MPU6000_ReadReg(MPU6000_ACCEL_ZOUT_L, valueL) )
    return MEMS_ERROR;

  if( !MPU6000_ReadReg(MPU6000_ACCEL_ZOUT_H, valueH) )
    return MEMS_ERROR;

  buff->AXIS_AZ = value; //AXIS Z buff

  return MEMS_SUCCESS;
}
/*******************************************************************************
* Function Name  : MPU6000_GetGyroAxesRaw
* Note           : Read Data from gyroscope
*******************************************************************************/
status_t MPU6000_GetGyroAxesRaw(MPU6000_Gyr_AxesRaw_t* buff) {
  int16_t value;
  uint8_t *valueL = (uint8_t *)(&value);
  uint8_t *valueH = ((uint8_t *)(&value)+1);

  if( !MPU6000_ReadReg(MPU6000_GYRO_XOUT_L, valueL) )
    return MEMS_ERROR;

  if( !MPU6000_ReadReg(MPU6000_GYRO_XOUT_H, valueH) )
    return MEMS_ERROR;

  buff->AXIS_GX = value; //AXIS X buff

  if( !MPU6000_ReadReg(MPU6000_GYRO_YOUT_L, valueL) )
    return MEMS_ERROR;

  if( !MPU6000_ReadReg(MPU6000_GYRO_YOUT_H, valueH) )
    return MEMS_ERROR;

  buff->AXIS_GY = value; //AXIS Y buff

  if( !MPU6000_ReadReg(MPU6000_GYRO_ZOUT_L, valueL) )
    return MEMS_ERROR;

  if( !MPU6000_ReadReg(MPU6000_GYRO_ZOUT_H, valueH) )
    return MEMS_ERROR;

  buff->AXIS_GZ = value; //AXIS Z buff

  return MEMS_SUCCESS;
}
/*******************************************************************************
* Function Name  : MPU6000_GetTempRaw
* Note           : Read Data from temp register
*******************************************************************************/
status_t MPU6000_GetTempRaw(MPU6000_Temp_t* buff) {
  int16_t value;
  uint8_t *valueL = (uint8_t *)(&value);
  uint8_t *valueH = ((uint8_t *)(&value)+1);

  if( !MPU6000_ReadReg(MPU6000_TEMP_OUT_L, valueL) )
    return MEMS_ERROR;

  if( !MPU6000_ReadReg(MPU6000_TEMP_OUT_H, valueH) )
    return MEMS_ERROR;

  buff->Temp_MPU = value; //AXIS X buff

  return MEMS_SUCCESS;
}

