/*
 * main.h
 *
 *  Created on: 27.06.2015
 *      Author: Filatov
 */

#ifndef MAIN_H_
#define MAIN_H_
//=======INCLUDES=====================
#include "stm32f10x_it.h"
#include "init.h"
#include "mpu6000_driver.h"

status_t MPU6000_INIT(MPU6000_CLKSEL_t arg1, MPU6000_DLFP_t arg2, MPU6000_AFS_t arg3, MPU6000_FS_t arg4);

#endif /* MAIN_H_ */
