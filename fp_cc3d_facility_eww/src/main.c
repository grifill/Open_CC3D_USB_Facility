/*
 * main.c
 *
 *  Created on: 04.08.2016
 *      Author: Filatov Grigorij
 */
#include "init.h"
#include "main.h"
#include "stm32f10x_conf.h"
#include "mems.h"
#include "mpu6000_driver.h"

// usb ---------------------------
#include "usb_lib.h"
#include "usb_desc.h"
#include "hw_config.h"
#include "usb_pwr.h"

#include "usb_istr.h"


/* Extern  variables ---------------------------------------------------------*/
uint8_t response;
MPU6000_Acc_AxesRaw_t data_mpu_acc;
MPU6000_Gyr_AxesRaw_t data_mpu_gyro;
MPU6000_Temp_t data_mpu_temp;
MPU_Who_Am_I_t data_mpu_who;
/* Private functions ---------------------------------------------------------*/
/*******************************************************************************
 * Function Name	: MPU6000_INIT
 * Note				:
 *******************************************************************************/
status_t MPU6000_INIT(MPU6000_CLKSEL_t arg1, MPU6000_DLFP_t arg2, MPU6000_AFS_t arg3, MPU6000_FS_t arg4)
{
	status_t MPU6000_response= MEMS_SUCCESS;
	response= MPU6000_Reset(arg1);
	response= MPU6000_I2C_Disable();
	response= MPU6000_Wake_Disable();
	response= MPU6000_DefaultDIV();
	response= MPU6000_DLPF_Config(arg2);
	response= MPU6000_Accel_Config(arg3);
	response= MPU6000_Gyro_Config(arg4);

	return MPU6000_response;
}


int main()
{
	SystemInit();
	// usb
	Set_System();
	Set_USBClock();
	USB_Init();
	USB_Interrupts_Config();
	for(int i=0; i<9900000; i++);

	//per
	init_cc3d_led();

	SPI_Mems_Init();
        
        // MPU6000 Settings
	MPU6000_INIT(PLL_Z_Gyro,HZ_260,AFS_16,FS_1000);
        for(int i=0; i<9000000; i++);// delay init usb layer

	init_cc3d_tim2();
	while (1){
	}
}





