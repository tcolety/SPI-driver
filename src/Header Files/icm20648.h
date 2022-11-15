#ifndef HEADER_FILES_ICM20648_H_
#define HEADER_FILES_ICM20648_H_

//***********************************************************************************
// include files
//***********************************************************************************
#include "HW_delay.h"
#include "em_assert.h"
#include "spi.h"
#include "brd_config.h"

//***********************************************************************************
// defined files
//***********************************************************************************
#define WHO_AM_I_REG        0x00    //bank 0
#define WHO_AM_I_BYTES      1
#define PWR_MGMT_1_REG      0x06    //bank 0
#define PWR_MGMT_1_BYTES    1
#define PWR_MGMT_1_DATA     0b00101000
#define PWR_MGMT_2_REG      0x07    //bank 0
#define PWR_MGMT_2_BYTES    1
#define PWR_MGMT_2_DATA     0b000111
#define LP_CONFIG_REG       0x05    //bank 0
#define LP_CONFIG_BYTES     1
#define LP_CONFIG_DATA      0b00100000
#define ACCEL_WOM_THR_REG   0x13    //bank 2
#define ACCEL_WOM_THR_BYTES 1
#define ACCEL_WOM_THR_DATA  60
#define REG_BANK_SEL_REG    0x7F
#define REG_BANK_SEL_BYTES  1
#define REG_BANK_0_DATA     0b000000
#define REG_BANK_2_DATA     0b100000

#define ICM_DA              0xE0
#define ICM_WRITE_DELAY     22
#define NOP                 0

//***********************************************************************************
// global variables
//***********************************************************************************

//***********************************************************************************
// function prototypes
//***********************************************************************************
void icm20648_open(void);
void icm20648_read(uint32_t reg, uint32_t bytes, uint32_t callback);
void icm20648_write(uint32_t reg, uint32_t bytes, uint32_t writeData, uint32_t callback);
uint16_t icm20648_get_read_result(void);

#endif /* HEADER_FILES_ICM20648_H_ */
