/*******************************************************************************
* File Name          : mpu6000_driver.h
* Author             : Filatov G.U.
* Version            : 0.1.1 - SPI only
* Date               : August 2016
* Description        : MPU6000 driver file.h
*******************************************************************************/
#ifndef INC_MEASURE_MPU6000_DRIVER_H_
#define INC_MEASURE_MPU6000_DRIVER_H_

/* Includes ------------------------------------------------------------------*/
#include "mems.h"
#include "stm32f10x_conf.h"
/* Exported types ------------------------------------------------------------*/

//these could change accordingly with the architecture

#ifndef __ARCHDEP__TYPES
#define __ARCHDEP__TYPES
#endif /*__ARCHDEP__TYPES*/

typedef uint8_t MPU6000_IntPinConf_t;
typedef uint8_t MPU6000_IntConf_t;
typedef uint8_t MPU6000_Axis_t;

/* Exported common structure --------------------------------------------------------*/
#ifndef __SHARED__STR
#define __SHARED__STR

typedef enum {
  MEMS_SUCCESS				=		0x01,
  MEMS_ERROR				=		0x00
} status_t;

typedef enum {
  MEMS_ENABLE				=		0x01,
  MEMS_DISABLE				=		0x00
} State_t;
#endif

typedef struct {
  int16_t AXIS_AX;
  int16_t AXIS_AY;
  int16_t AXIS_AZ;
} MPU6000_Acc_AxesRaw_t;

typedef struct {
  int16_t AXIS_GX;
  int16_t AXIS_GY;
  int16_t AXIS_GZ;
} MPU6000_Gyr_AxesRaw_t;

typedef struct {
	int16_t Temp_MPU;
} MPU6000_Temp_t;

typedef struct {
	uint8_t WHO_MPU;
}MPU_Who_Am_I_t;

#define GYRO_SCALE_FACTOR  0.00053292f  // (4/131) * pi/180   (32.75 LSB = 1 DPS)
#define ACCEL_SCALE_FACTOR 0.00119708f // (1/8192) * 9.8065  (8192 LSB = 1 G)


typedef enum {
	Input_disabled           =		0x00,
	TEMP_OUT_L				 =		0x01,
	GYRO_XOUT_L              =		0x02,
	GYRO_YOUT_L				 =		0x03,
	GYRO_ZOUT_L				 =		0x04,
	ACCEL_XOUT_L		     =		0x05,
	ACCEL_YOUT_L			 =		0x06,
	ACCEL_ZOUT_L			 =		0x07
} MPU6000_FSYNC_t;

// Bandwidth (Hz)
typedef enum {
	HZ_260           		 =		0x00, // Delay 0 ms
	HZ_184				 	 =		0x01, // Delay 2 ms
	HZ_94              		 =		0x02, // Delay 3 ms
	HZ_44				 	 =		0x03, // Delay 4.9 ms
	HZ_21				 	 =		0x04, // Delay 8.5 ms
	HZ_10		     		 =		0x05, // Delay 13.8 ms
	HZ_5			 		 =		0x06  // Delay 19.0 ms
} MPU6000_DLFP_t; // Accelerometer Fs = 1kHz

// FS_SEL selects the full scale range of the gyroscope outputs
typedef enum {
	FS_250           		 =		0x00,
	FS_500				 	 =		0x01,
	FS_1000              	 =		0x02,
	FS_2000				 	 =		0x03
} MPU6000_FS_t; // grad per second

// AFS_SEL selects the full scale range of the accelerometer outputs
typedef enum {
	AFS_2           		 =		0x00,
	AFS_4				 	 =		0x01,
	AFS_8              	 	 =		0x02,
	AFS_16				 	 =		0x03
} MPU6000_AFS_t; // gauss

typedef enum {
	Int_Oscill_8Mhz          =		0x00,
	PLL_X_Gyro				 =		0x01,
	PLL_Y_Gyro               =		0x02,
	PLL_Z_Gyro				 =		0x03,
	PLL_External_32Khz		 =		0x04,
	PLL_External_19Mhz		 =		0x05,
	Stop_and_reset			 =		0x07
} MPU6000_CLKSEL_t;

typedef enum {
	Hz_1           			 =		0x00,
	Hz_5				 	 =		0x01,
	Hz_20              	 	 =		0x02,
	Hz_40				 	 =		0x03
} MPU6000_LP_WAKE_t; // gauss

/* Exported macro ------------------------------------------------------------*/

#ifndef __SHARED__MACROS

#define __SHARED__MACROS
#define ValBit(VAR,Place)         (VAR & (1<<Place))
#define BIT(x) ( (x) )

#endif /*__SHARED__MACROS*/

/* Exported constants --------------------------------------------------------*/

#ifndef __SHARED__CONSTANTS
#define __SHARED__CONSTANTS

#define MEMS_SET                            0x01
#define MEMS_RESET                          0x00

#endif /*__SHARED__CONSTANTS*/

//=================================================================================
#define MPU6000_WHO_AM_I					0x75  // Device identification register

// --- Self Test Registers -------------------------------------------------------------------
#define MPU6000_SELF_TEST_X					0x0D  // Test X-axis
#define MPU6000_XA_TEST						BIT(5)
#define MPU6000_XG_TEST						BIT(0)

#define MPU6000_SELF_TEST_Y					0x0E  // Test Y-axis
#define MPU6000_YA_TEST						BIT(5)
#define MPU6000_YG_TEST						BIT(0)

#define MPU6000_SELF_TEST_Z					0x0F  // Test Y-axis
#define MPU6000_ZA_TEST						BIT(5)
#define MPU6000_ZG_TEST						BIT(0)

#define MPU6000_SELF_TEST_A					0x10  // Accelerometer test
#define MPU6000_XA_TEST_2						BIT(4)
#define MPU6000_YA_TEST_2						BIT(2)
#define MPU6000_ZA_TEST_2						BIT(0)

// --- Sample Rate Divider -------------------------------------------------------------------
#define MPU6000_SMPLRT_DIV					0x19  // The divider from the gyroscope output

// --- Configuration registers ---------------------------------------------------------------
#define MPU6000_CONFIG						0x1A   // This register configures the external Frame
#define MPU6000_EXT_SYNC_SET				BIT(3) // Synchronization (FSYNC)
#define MPU6000_DLPF_CFG				    BIT(0) // pin sampling and the Digital Low Pass Filter
 	 	 	 	 	 	 	 	 	 	 	 	   // (DLPF) setting for both
 	 	 	 	 	 	 	 	 	 	 	 	   // the gyroscopes and accelerometers.

#define MPU6000_GYRO_CONFIG					0x1B   // This register is used to trigger gyroscope
#define MPU6000_XG_ST						BIT(7) // self-test and configure
#define MPU6000_YG_ST						BIT(6) // the gyroscopes’ full scale range.
#define MPU6000_ZG_ST						BIT(5) // When set to 1, performs self test.
#define MPU6000_FS_SEL						BIT(3)

#define MPU6000_ACCEL_CONFIG				0x1C   // This register is used to trigger accelerometer
#define MPU6000_XA_ST						BIT(7) // self test and configure
#define MPU6000_YA_ST						BIT(6) // the accelerometer full scale range.
#define MPU6000_ZA_ST						BIT(5) // When set to 1, performs self test.
#define MPU6000_AFS_SEL						BIT(3)

#define MPU6000_MOT_THR						0x1F  // This register configures the detection threshold
												  // for Motion interrupt generation.

#define MPU6000_FIFO_EN					    0x23  // FIFO Enable
#define MPU6000_TEMP_FIFO_EN				BIT(7)
#define MPU6000_XG_FIFO_EN					BIT(6)
#define MPU6000_YG_FIFO_EN					BIT(5)
#define MPU6000_ZG_FIFO_EN					BIT(4)
#define MPU6000_ACCEL_FIFO_EN				BIT(3)
#define MPU6000_SLV2_FIFO_EN				BIT(2)
#define MPU6000_SLV1_FIFO_EN				BIT(1)
#define MPU6000_SLV0_FIFO_EN				BIT(0)

// --- Interupt registers --------------------------------------------------------------------------
#define MPU6000_INT_PIN_CFG 				0x37
#define MPU6000_INT_LEVEL				    BIT(7)
#define MPU6000_INT_OPEN					BIT(6)
#define MPU6000_LATCH_INT_EN				BIT(5)
#define MPU6000_INT_RD_CLEAR				BIT(4)
#define MPU6000_FSYNC_INT_LEVEL				BIT(3)
#define MPU6000_FSYNC_INT_EN				BIT(2)
#define MPU6000_I2C_BYPASS_EN				BIT(1)

#define MPU6000_INT_ENABLE 					0x38
#define MPU6000_MOT_EN					    BIT(6)
#define MPU6000_FIFO_OFLOW_EN				BIT(4)
#define MPU6000_I2C_MST_INT_EN				BIT(3)
#define MPU6000_DATA_RDY_EN					BIT(0)

#define MPU6000_INT_STATUS 					0x3A
#define MPU6000_MOT_EN_State				BIT(6)
#define MPU6000_FIFO_OFLOW_EN_State			BIT(4)
#define MPU6000_I2C_MST_INT_EN_State		BIT(3)
#define MPU6000_DATA_RDY_EN_State			BIT(0)
// --- Output registers -----------------------------------------------------------------------------
// Acc
#define MPU6000_ACCEL_XOUT_H 				0x3B
#define MPU6000_ACCEL_XOUT_L 				0x3C
#define MPU6000_ACCEL_YOUT_H 				0x3D
#define MPU6000_ACCEL_YOUT_L 				0x3E
#define MPU6000_ACCEL_ZOUT_H 				0x3F
#define MPU6000_ACCEL_ZOUT_L 				0x40
// Temperature
#define MPU6000_TEMP_OUT_H 					0x41
#define MPU6000_TEMP_OUT_L 					0x42
// Gyro
#define MPU6000_GYRO_XOUT_H 				0x43
#define MPU6000_GYRO_XOUT_L 				0x44
#define MPU6000_GYRO_YOUT_H 				0x45
#define MPU6000_GYRO_YOUT_L 				0x46
#define MPU6000_GYRO_ZOUT_H 				0x47
#define MPU6000_GYRO_ZOUT_L 				0x48

// --- Other registers -------------------------------------------------------------------------------
#define MPU6000_SIGNAL_PATH_RESET 			0x68 // Reset Gyro, Accel and Temp
#define MPU6000_GYRO_RESET					BIT(2)
#define MPU6000_ACCEL_RESET					BIT(1)
#define MPU6000_TEMP_RESET					BIT(0)

#define MPU6000_MOT_DETECT_CTRL 			0x69 // Used to add delay to the accelerometer power on time
#define MPU6000_ACCEL_ON_DELAY				BIT(4)

#define MPU6000_USER_CTRL 				    0x6A   // This register allows the user to enable and disable
#define MPU6000_FIFO_EN_Bit						BIT(6) // the FIFO buffer, I2C Master Mode, and primary I2C interface.
#define MPU6000_I2C_MST_EN					BIT(5)
#define MPU6000_I2C_IF_DIS					BIT(4)
#define MPU6000_FIFO_RESET					BIT(2)
#define MPU6000_I2C_MST_RESET				BIT(1)
#define MPU6000_SIG_COND_RESET				BIT(0)

#define MPU6000_PWR_MGMT_1					0x6B // Power manage1
#define MPU6000_DEVICE_RESET				BIT(7)
#define MPU6000_SLEEP						BIT(6)
#define MPU6000_CYCLE					    BIT(5)
#define MPU6000_TEMP_DIS				    BIT(3)
#define MPU6000_CLKSEL						BIT(0)

#define MPU6000_PWR_MGMT_2					0x6C // Power manage2
#define MPU6000_LP_WAKE_CTRL				BIT(6)
#define MPU6000_STBY_XA						BIT(5)
#define MPU6000_STBY_YA						BIT(4)
#define MPU6000_STBY_ZA						BIT(3)
#define MPU6000_STBY_XG						BIT(2)
#define MPU6000_STBY_YG						BIT(1)
#define MPU6000_STBY_ZG						BIT(0)

#define MPU6000_FIFO_COUNTH					0x72
#define MPU6000_FIFO_COUNTL					0x73
#define MPU6000_FIFO_R_W					0x74




//Sensor Configuration Functions
uint8_t MPU6000_ReadReg(uint8_t Reg, uint8_t* Data);
uint8_t MPU6000_WriteReg(uint8_t Reg, uint8_t Data);
status_t MPU6000_Reset(MPU6000_CLKSEL_t cl);
status_t MPU6000_I2C_Disable();
status_t MPU6000_Wake_Disable();
status_t MPU6000_DefaultDIV();
status_t MPU6000_DLPF_Config(MPU6000_DLFP_t dl);
status_t MPU6000_Accel_Config(MPU6000_AFS_t adl);
status_t MPU6000_Gyro_Config(MPU6000_FS_t adl);

//output
status_t MPU6000_WHO_AM_I_Get(MPU_Who_Am_I_t* buff);
status_t MPU6000_GetAccAxesRaw(MPU6000_Acc_AxesRaw_t* buff);
status_t MPU6000_GetGyroAxesRaw(MPU6000_Gyr_AxesRaw_t* buff);
status_t MPU6000_GetTempRaw(MPU6000_Temp_t* buff);







#endif /* INC_MEASURE_MPU6000_DRIVER_H_ */
