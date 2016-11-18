/**
 ******************************************************************************
 * @file    Demo/src/stm32f10x_it.c
 * @author  MCD Application Team
 * @version V1.0.0
 * @date    09/13/2010
 * @brief   Main Interrupt Service Routines.
 *          This file provides template for all exceptions handler and peripherals
 *          interrupt service routine.
 ******************************************************************************
 * @copy
 *
 * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
 * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
 * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
 * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
 * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
 * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
 *
 * <h2><center>&copy; COPYRIGHT 2010 STMicroelectronics</center></h2>
 */

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x_it.h"
#include "stm32f10x_conf.h"
#include "math.h"
#include <string.h>
#include "init.h"
#include "mpu6000_driver.h"
// usb ---------------------------
#include "usb_lib.h"
#include "usb_desc.h"
#include "hw_config.h"
#include "usb_pwr.h"
#include "usb_istr.h"

/** @addtogroup Demo
 * @{
 */
#define M_PI  3.14159265358979323846
//#define FLOAT_CONVERTION_USE


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/******************************************************************************/
/*            Cortex-M3 Processor Exceptions Handlers                         */
/******************************************************************************/
/**
 * @brief  This function handles NMI exception.
 * @param  None
 * @retval None
 */
void NMI_Handler(void)
{
}

/**
 * @brief  This function handles Hard Fault exception.
 * @param  None
 * @retval None
 */
void HardFault_Handler(void)
{
	/* Go to infinite loop when Hard Fault exception occurs */
	while (1)
	{
	}
}

/**
 * @brief  This function handles Memory Manage exception.
 * @param  None
 * @retval None
 */
void MemManage_Handler(void)
{
	/* Go to infinite loop when Memory Manage exception occurs */
	while (1)
	{
	}
}

/**
 * @brief  This function handles Bus Fault exception.
 * @param  None
 * @retval None
 */
void BusFault_Handler(void)
{
	/* Go to infinite loop when Bus Fault exception occurs */
	while (1)
	{
	}
}

/**
 * @brief  This function handles Usage Fault exception.
 * @param  None
 * @retval None
 */
void UsageFault_Handler(void)
{
	/* Go to infinite loop when Usage Fault exception occurs */
	while (1)
	{
	}
}

/**
 * @brief  This function handles SVCall exception.
 * @param  None
 * @retval None
 */
void SVC_Handler(void)
{
}

/**
 * @brief  This function handles Debug Monitor exception.
 * @param  None
 * @retval None
 */
void DebugMon_Handler(void)
{
}

/**
 * @brief  This function handles PendSV_Handler exception.
 * @param  None
 * @retval None
 */
void PendSV_Handler(void)
{
}

/**
 * @brief  This function handles SysTick Handler.
 * @param  None
 * @retval None
 */
void SysTick_Handler(void)
{
}

/******************************************************************************/
/*                 STM32F10x Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f10x_xx.s).                                            */
/******************************************************************************/
extern MPU6000_Acc_AxesRaw_t data_mpu_acc;
extern MPU6000_Gyr_AxesRaw_t data_mpu_gyro;
extern MPU6000_Temp_t data_mpu_temp;
uint8_t buf_tx[18];
uint8_t delay_irq=0;
void TIM2_IRQHandler ()
{
	switch (TIM_GetITStatus(TIM2, TIM_IT_Update))
	{
	case 0:
		break;
	default:
		LED_BLUE_TRIGG();
		//--------------------------------------------
		buf_tx[0]=0xEF;
		buf_tx[1]=0xBE;
		buf_tx[2]=0xAD;
		buf_tx[3]=0xDE;
		MPU6000_GetAccAxesRaw(&data_mpu_acc);
		memmove(buf_tx+4,&data_mpu_acc.AXIS_AX,2);
		memmove(buf_tx+6,&data_mpu_acc.AXIS_AY,2);
		memmove(buf_tx+8,&data_mpu_acc.AXIS_AZ,2);
		MPU6000_GetGyroAxesRaw(&data_mpu_gyro);
		memmove(buf_tx+10,&data_mpu_gyro.AXIS_GX,2);
		memmove(buf_tx+12,&data_mpu_gyro.AXIS_GY,2);
		memmove(buf_tx+14,&data_mpu_gyro.AXIS_GZ,2);
		switch (delay_irq/20)
		{
		case 0:
			delay_irq++;
			break;
		default:
			MPU6000_GetTempRaw(&data_mpu_temp);
#if defined (FLOAT_CONVERTION_USE)
			int8_t temp_buf[2];
			float temp_conv;
			temp_conv=((float)(data_mpu_temp.Temp_MPU)/340.0)+36.0;
			temp_buf[1]=(int)temp_conv;
			temp_buf[0]=(int)((temp_conv-temp_buf[0])*1000);
			memmove(buf_tx+16,&temp_buf,2);
#else
			int16_t temp;
			temp=(data_mpu_temp.Temp_MPU/340)+36;
			memmove(buf_tx+16,&temp,2);
#endif
			delay_irq=0;
			break;
		}
		//------ÂÛÂÎÄ-------------------------------------
		USB_Send_Data(buf_tx,18);
		break;
	}
	TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
}

void USB_LP_CAN1_RX0_IRQHandler(void)
{
	USB_Istr();
}
/**
 * @brief  This function handles PPP interrupt request.
 * @param  None
 * @retval None
 */
/*void PPP_IRQHandler(void)
{
}*/


/**
 * @}
 */

/******************* (C) COPYRIGHT 2010 STMicroelectronics *****END OF FILE****/
