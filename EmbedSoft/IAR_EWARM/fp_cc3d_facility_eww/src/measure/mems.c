/*
 * mems.c for STM32F103 chip choise
 *
 *  Created on: 04 aug. 2016.
 *      Author: grifill
 */

#include "mems.h"


__IO uint32_t  MPU6000_Timeout = MPU6000_LONG_TIMEOUT;

/*******************************************************************************
* Function Name  : SPI_3W_Mems_Init
* Description    : Initializes the peripherals used by the 3 wires SPI Mems driver.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SPI_Mems_Init(void)
{
  SPI_InitTypeDef  SPI_InitStructure;
  GPIO_InitTypeDef GPIO_InitStructure;

  /* Enable SPI and GPIO clocks */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA |RCC_APB2Periph_AFIO, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);

  /* Configure SPI pins: SCK, MISO and MOSI */
  GPIO_InitStructure.GPIO_Pin = SPI1_CLK_PIN | SPI1_MOSI_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(SPI1_PORT, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = SPI1_MISO_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(SPI1_PORT, &GPIO_InitStructure);

  /* Configure I/O for MEMS Chip select */
  GPIO_InitStructure.GPIO_Pin = NSS_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(NSS_PORT, &GPIO_InitStructure);

  //GPIO_PinRemapConfig(GPIO_Remap_SPI1, ENABLE);

  /* Deselect Mems Sensor: Chip Select high */
  Pin_Set(NSS_PORT,NSS_PIN);

  /* SPI configuration */
  SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
  SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_128;
  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
  SPI_InitStructure.SPI_CRCPolynomial = 7;
  SPI_Init(SPI1, &SPI_InitStructure);

  /* Enable the SPI  */
  SPI_Cmd(SPI1, ENABLE);
}

/*******************************************************************************
* Function Name  : SPI_Mems_DeInit
* Description    : DeInitializes the peripherals used by the SPI Mems driver.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SPI_Mems_DeInit(void) {
  GPIO_InitTypeDef GPIO_InitStructure;

  GPIO_InitStructure.GPIO_Pin =SPI1_CLK_PIN | SPI1_MOSI_PIN|SPI1_MISO_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(SPI1_PORT, &GPIO_InitStructure);

  /* Configure I/O for MEMS Chip select */
    GPIO_InitStructure.GPIO_Pin = NSS_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(NSS_PORT, &GPIO_InitStructure);

}


/*******************************************************************************
* Function Name  : SPI_Mems_Read_Reg
* Description    : Reads Mems Register
* Input          : Register
* Output         : None
* Return         : Register Content
*******************************************************************************/
uint8_t SPI_Mems_Read_Reg(uint8_t MemsType, uint8_t reg) {
  uint8_t Temp;

  reg |= 0x80; //reading procedure has to set the most significant bit
  // Select Mems Sensor: Chip Select low
  switch(MemsType)
      {
  	case CS_MPU6000: Pin_Rst(NSS_PORT,NSS_PIN);break;
  	default: break;
      }

  // Send Register Address
  SPI_Mems_SendByte(reg);
  /* Read a byte from the MEMS Sensor */
  Temp = SPI_Mems_SendByte(DUMMY_BYTE);

  /* Deselect Mems Sensor: Chip Select high */
  switch(MemsType)
     {
       case CS_MPU6000: Pin_Set(NSS_PORT,NSS_PIN);break;
       default: break;
     }

  return Temp;
}


/*******************************************************************************
* Function Name  : SPI_Mems_Write_Reg
* Description    : Write Data in Mems Register
* Input          : Register, data
* Output         : None
* Return         : None
*******************************************************************************/
void SPI_Mems_Write_Reg(uint8_t MemsType,uint8_t regAddr, uint8_t data) {

  // Select Mems Sensor: Chip Select low
  switch(MemsType)
    {
	case CS_MPU6000: Pin_Rst(NSS_PORT,NSS_PIN);break;
	default: break;
    }

  // Send Register Address
  SPI_Mems_SendByte(regAddr);
  /* Write a byte into the MEMS Sensor Register*/
  SPI_Mems_SendByte(data);

  /* Deselect Mems Sensor: Chip Select high */
  switch(MemsType)
   {
  case CS_MPU6000: Pin_Set(NSS_PORT,NSS_PIN);break;
     default: break;
   }
}


/*******************************************************************************
* Function Name  : SPI_Mems_SendByte
* Description    : Sends a byte through the SPI interface and return the byte
*                  received from the SPI bus.
* Input          : byte : byte to send.
* Output         : None
* Return         : The value of the received byte.
*******************************************************************************/
uint8_t SPI_Mems_SendByte(uint8_t byte) {

  /* Loop while DR register in not emplty */
  while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);

  /* Send byte through the SPI1 peripheral */
  SPI_I2S_SendData(SPI1, byte);

  /* Wait to receive a byte */
  while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);

  /* Return the byte read from the SPI bus */
  return SPI_I2S_ReceiveData(SPI1);
}
