/*
 * mems.h
 *
 *  Created on: 04 aug. 2016.
 *      Author: grifill
 */

#ifndef INC_MEASURE_MEMS_H_
#define INC_MEASURE_MEMS_H_

#include "stm32f10x_conf.h"


#define	SPI1_CLK_PIN	GPIO_Pin_5	// SPI MPU6000
#define	SPI1_MISO_PIN	GPIO_Pin_6
#define	SPI1_MOSI_PIN	GPIO_Pin_7
#define	SPI1_PORT	GPIOA

#define	NSS_PIN	GPIO_Pin_4	// CS MPU6000
#define	NSS_PORT	GPIOA

#define	INT_PIN		GPIO_Pin_3	// INT MPU6000
#define	INT_PORT	GPIOA

//flags timeout
#define MPU6000_FLAG_TIMEOUT             ((uint32_t)0x1000)
#define MPU6000_LONG_TIMEOUT             ((uint32_t)(10 * MPU6000_FLAG_TIMEOUT))


#define CS_MPU6000	0

#define DUMMY_BYTE    0x00




void SPI_Mems_Init(void);
void SPI_Mems_DeInit(void);
uint8_t SPI_Mems_Read_Reg(uint8_t MemsType, uint8_t reg);
void SPI_Mems_Write_Reg(uint8_t MemsType,uint8_t regAddr, uint8_t data);
uint8_t SPI_Mems_SendByte(uint8_t byte);


#endif /* INC_MEASURE_MEMS_H_ */
