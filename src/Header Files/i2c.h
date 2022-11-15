#ifndef HEADER_FILES_I2C_H_
#define HEADER_FILES_I2C_H_

//***********************************************************************************
// include files
//***********************************************************************************
#include "em_i2c.h"
#include "em_cmu.h"
#include "sleep_routines.h"
#include "scheduler.h"

//***********************************************************************************
// defined files
//***********************************************************************************
#define MASK    0xFF

//***********************************************************************************
// global variables
//***********************************************************************************
typedef struct {
  bool                  enable;
  bool                  master;
  uint32_t              refFreq;
  uint32_t              freq;
  I2C_ClockHLR_TypeDef  clhr;
  uint32_t              out_pin_route_scl;   // out 0 route to gpio port/pin
  uint32_t              out_pin_route_sda;   // out 1 route to gpio port/pin
  bool                  out_pin_scl_en;   // enable out 0 route
  bool                  out_pin_sda_en;   // enable out 1 route
} I2C_OPEN_STRUCT;

typedef enum {
  initialize,
  send_RA,
  send_DA,
  read_data,
  write_data,
  send_stop,
} DEFINED_STATES;

typedef struct {
  DEFINED_STATES  currentState;

  I2C_TypeDef     *i2c;          //stores I2C0 OR I2C1 defined struct
  bool            readTrue;      //0=write 1=read
  uint32_t        numOfBytes;
  uint32_t        deviceAddress;
  uint32_t        registerAddress;
  uint32_t        callback;
  uint32_t        *storeData;
  uint32_t        writeData;

  volatile bool   busy;
} I2C_STATE_MACHINE;

//***********************************************************************************
// function prototypes
//***********************************************************************************
void i2c_open(I2C_TypeDef *i2c, I2C_OPEN_STRUCT *i2c_setup);
void i2c_start(I2C_TypeDef *i2c, bool readTrue, uint32_t bytes, uint32_t deviceAddress,
               uint32_t registerAddress, uint32_t callback, uint32_t *storeData,
               uint32_t write_data);
void I2C0_IRQHandler(void);
void I2C1_IRQHandler(void);
void i2c_ack_sm(I2C_STATE_MACHINE *i2c_sm);
void i2c_read_sm(I2C_STATE_MACHINE *i2c_sm);
void i2c_stop_sm(I2C_STATE_MACHINE *i2c_sm);
bool get_i2c_busy(I2C_TypeDef *i2c);

#endif /* HEADER_FILES_I2C_H_ */
