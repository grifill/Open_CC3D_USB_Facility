/*
 * init.c
 *      Author: Filatov Grigorij
 */
#include "init.h"
#include <stdio.h>
#include <stdlib.h>

#include "stm32f10x_it.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*   LED CC3D OPENPILOT  PIN - PB3                                            */
/******************************************************************************/
void init_cc3d_led()
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB |RCC_APB2Periph_AFIO, ENABLE);
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);

	GPIO_InitTypeDef    cc3d_led;
	cc3d_led.GPIO_Pin = GPIO_Pin_3;
	cc3d_led.GPIO_Speed = GPIO_Speed_50MHz;
	cc3d_led.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &cc3d_led);
}

/******************************************************************************/
/*   TIMER2 CC3D OPENPILOT                                                    */
/******************************************************************************/
void init_cc3d_tim2()
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);
       
	TIM_TimeBaseInitTypeDef base_timer_2;
	TIM_TimeBaseStructInit(&base_timer_2);
	base_timer_2.TIM_Prescaler = 3600-1;
	base_timer_2.TIM_Period = 400; // 72000000/(3600*400)=50 (Hz)
	base_timer_2.TIM_CounterMode = TIM_CounterMode_Up  ;
	TIM_TimeBaseInit(TIM2, &base_timer_2);

	// interrupt config
	NVIC_InitTypeDef 	NVIC_InitStructure_TIM2;
	NVIC_InitStructure_TIM2.NVIC_IRQChannel = TIM2_IRQn;
	NVIC_InitStructure_TIM2.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure_TIM2.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure_TIM2.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure_TIM2);
	TIM_ITConfig((TIM_TypeDef *)TIM2_BASE,TIM_IT_Update, ENABLE);
	TIM_ClearITPendingBit((TIM_TypeDef *)TIM2_BASE, TIM_IT_Update);

	TIM_Cmd(TIM2,ENABLE);
}

/******************************************************************************/
/*   USART3 CC3D OPENPILOT (FLEXI PORT)                                       */
/******************************************************************************/
void init_cc3d_flexi_port()
{
	// GPIO
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB |RCC_APB2Periph_AFIO, ENABLE);

	GPIO_InitTypeDef    flexi_init_tx;
	flexi_init_tx.GPIO_Pin = GPIO_Pin_10;
	flexi_init_tx.GPIO_Speed = GPIO_Speed_50MHz;
	flexi_init_tx.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOB, &flexi_init_tx);

	GPIO_InitTypeDef    flexi_init_rx;
	flexi_init_rx.GPIO_Pin = GPIO_Pin_11;
	flexi_init_rx.GPIO_Speed = GPIO_Speed_50MHz;
	flexi_init_rx.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOB, &flexi_init_rx);

	// USART
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);

	USART_InitTypeDef usart_flexi;
	usart_flexi.USART_BaudRate = 115200;
	usart_flexi.USART_WordLength = USART_WordLength_8b;
	usart_flexi.USART_StopBits = USART_StopBits_1;
	usart_flexi.USART_Parity = USART_Parity_No;
	usart_flexi.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	usart_flexi.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
	USART_Init(USART3, &usart_flexi);

	// IT & FLAGs
	NVIC_InitTypeDef 	NVIC_InitStructure_USART3;
	NVIC_InitStructure_USART3.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure_USART3.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure_USART3.NVIC_IRQChannelSubPriority = 2;
	NVIC_InitStructure_USART3.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure_USART3);

	USART_ITConfig(USART3,USART_IT_TC, ENABLE);
	USART_ClearITPendingBit(USART3, USART_IT_TC);

	USART_ClearFlag(USART3, USART_FLAG_TC|USART_FLAG_RXNE);

	USART_Cmd(USART3, ENABLE);

}
