/*
 * init.h
 *
 *  Created on: 03 feb. 2015.
 *      Author: grifill
 */

#ifndef INC_INIT_H_
#define INC_INIT_H_

#include "stm32f10x_conf.h"
#include "stm32f10x_it.h"

// DEFINEs
#define LED_BLUE_ON()		  	(((GPIO_TypeDef *) GPIOB_BASE)->BRR  = GPIO_Pin_3)
#define LED_BLUE_OFF()		  	(((GPIO_TypeDef *) GPIOB_BASE)->BSRR  = GPIO_Pin_3)
#define LED_BLUE_TRIGG()		(((GPIO_TypeDef *) GPIOB_BASE)->ODR ^= GPIO_Pin_3)

// LED
void init_cc3d_led();

// TIMERs
void init_cc3d_tim2();

// USARTs
void init_cc3d_flexi_port();





#endif /* INC_INIT_H_ */
