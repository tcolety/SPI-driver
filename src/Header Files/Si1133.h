/*
 * Si1133.h
 *
 *  Created on: Oct 5, 2021
 *      Author: tcole
 */

#ifndef HEADER_FILES_SI1133_H_
#define HEADER_FILES_SI1133_H_
//***********************************************************************************
// include files
//***********************************************************************************
#include "i2c.h"
#include "em_assert.h"
#include "HW_delay.h"
#include "brd_config.h"
#include <stddef.h>

//***********************************************************************************
// defined files
//***********************************************************************************
#define NO_CALLBACK       0x00
#define NOP               0
#define DEVICE_ADDRESS    0x55
#define HARDWARE_DELAY    30
#define BIT_MASK          0xF

#define RESPONSE0_REG     0x11
#define RESPONSE0_BYTES   1
#define INPUT0_REG        0xA
#define INPUT0_BYTES      1
#define COMMAND_REG       0xB
#define COMMAND_BYTES     1
#define HOSTOUT0_REG      0x13
#define HOSTOUT0_BYTES    2

#define ADCMUX_WHITE      0b01011
#define PARAM_SET         0b10000000
#define ADCCONFIG0        0x2
#define INPUT0_WRITE      0b1
#define CHAN_LIST         0x1
#define FORCE             0x11

#define ONE               1
#define TWO               2

#define   DIVISOR         16

//***********************************************************************************
// global variables
//***********************************************************************************

//***********************************************************************************
// function prototypes
//***********************************************************************************
void Si1133_i2c_open(void);
void Si1133_read(uint32_t callback, uint32_t register_addresss, uint32_t bytes);
void Si1133_write(uint32_t callback, uint32_t register_address, uint32_t bytes, uint32_t write_data);
uint32_t Si1133_get_read_result(void);
void Si1133_force_cmd(void);
void Si1133_request(uint32_t callback);

#endif /* HEADER_FILES_SI1133_H_ */
