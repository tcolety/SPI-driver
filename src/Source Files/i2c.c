/**
 * @file i2c.c
 * @author Taylor Colety
 * @date 9/30/21
 * @brief I2C Driver
 *
 */

//***********************************************************************************
// Include files
//***********************************************************************************
#include "i2c.h"

//***********************************************************************************
// private define statements
//***********************************************************************************

//***********************************************************************************
// private variables
//***********************************************************************************
static I2C_STATE_MACHINE i2c0_state_struct;
static I2C_STATE_MACHINE i2c1_state_struct;

static I2C_STATE_MACHINE *i2c_local_struct;

//***********************************************************************************
// private function prototypes
//***********************************************************************************
void i2c_bus_reset(I2C_TypeDef *i2c);

//***********************************************************************************
// private functions
//***********************************************************************************
/***************************************************************************//**
 * @brief
 *  Clears and resets the I2C bus, passing in values of the I2C struct to reset.
 *
 * @details
 *  Saving the state of the I2C, we can store interrupt flags and clear flags, buffers, and abort commands
 *  to reset the entire I2C peripheral.
 *
 * @note
 *  Using the OR command with both START and STOP in the I2C_CMD allows for reseting the I2C peripheral.
 *
 * @param
 *  I2C_TypeDef Struct with pointer *i2c, storing all variables to enable I2C peripheral.
 *
 ******************************************************************************/
void i2c_bus_reset(I2C_TypeDef *i2c){
  uint32_t save_state;

  i2c->CMD = I2C_CMD_ABORT;
  save_state = i2c->IEN;
  i2c->IEN = false;
  i2c->IFC = i2c->IF;

  i2c->CMD = I2C_CMD_CLEARTX;
  i2c->CMD = I2C_CMD_START | I2C_CMD_STOP;
  while(!(i2c->IF & I2C_IF_MSTOP));

  i2c->IFC = i2c->IF;
  i2c->CMD = I2C_CMD_ABORT;
  i2c->IEN = save_state;
};


//***********************************************************************************
// global functions
//***********************************************************************************
/***************************************************************************//**
 * @brief
 *  Function used to initialize I2C
 *
 * @details
 *  In this function I2C is initialized, the pins are routed, the interrupt (ACK,
 *  RXDATAV, AND MSTOP) are enabled and bus reset is called.
 *
 * @note
 *  Called in Si1133_open
 *
 * @param [in] i2c
 *  Either pointing to I2C0 OR I2C1
 *
 * @param [in] i2c_setup
 *  struct of type I2C_OPEN_STRUCT meant for holding values to initial I2C
 *
 ******************************************************************************/
void i2c_open(I2C_TypeDef *i2c, I2C_OPEN_STRUCT *i2c_setup){
  I2C_Init_TypeDef i2c_values;

  if (i2c == I2C0){
      CMU_ClockEnable(cmuClock_I2C0,true);
      i2c0_state_struct.busy = false;
  } else if(i2c == I2C1){
      CMU_ClockEnable(cmuClock_I2C1,true);
      i2c1_state_struct.busy = false;
  }

  if ((i2c->IF & 0x01) == 0) {
      i2c->IFS = 0x01;
      EFM_ASSERT(i2c->IF & 0x01);
      i2c->IFC = 0x01;
  } else {
      i2c->IFC = 0x01;
      EFM_ASSERT(!(i2c->IF & 0x01));
  }

  i2c_values.enable = i2c_setup->enable;
  i2c_values.master = i2c_setup->master;
  i2c_values.refFreq = i2c_setup->refFreq;
  i2c_values.freq = i2c_setup->freq;
  i2c_values.clhr = i2c_setup->clhr;

  I2C_Init(i2c, &i2c_values);

  i2c->ROUTELOC0 = i2c_setup->out_pin_route_scl | i2c_setup->out_pin_route_sda; //make sure to set the pin route var

  i2c->ROUTEPEN = (I2C_ROUTEPEN_SCLPEN*i2c_setup->out_pin_scl_en)
      | (I2C_ROUTEPEN_SDAPEN*i2c_setup->out_pin_sda_en);

  i2c_bus_reset(i2c);
  i2c->IFC = i2c->IF;
  i2c->IEN |= I2C_IF_ACK;
  i2c->IEN |= I2C_IF_RXDATAV;
  i2c->IEN |= I2C_IF_MSTOP;

  if (i2c == I2C0){
    NVIC_EnableIRQ(I2C0_IRQn);
  } else if(i2c == I2C1){
    NVIC_EnableIRQ(I2C1_IRQn);
  }
}

/***************************************************************************//**
 * @brief
 *  Checks which I2C peripheral is being used in order to instantiate the correct structure.
 *
 * @details
 *  Passing in the specific I2C peripheral I2C0 vs. I2C1, the function checks for the correct
 *  peripheral to configure for use with the Mighy Gecko.
 *
 * @note
 *  Using if statements and comparing the value of the *i2c the function checks the correct value
 *  to configure.
 *
 * @param [in] i2c
 *    I2C_STATE_MACHINE *i2c_sm which holds the state of the I2C peripheral and configuring data.
 *
 * @param [in] readTrue
 *    bool readTrue indicates whether i2c will be reading or writing 1=read, 0=write
 *
 * @param [in] bytes
 *    uint32_t bytes holds the number of bytes that is being red or written
 *
 * @param [in] deviceAddress
 *    uint32_t deviceAddress is the address of what i2c is interfacing with
 *
 * @param [in] registerAddress
 *    uint32_t registerAddress indicated the register that is going to be read
 *    or written to in the si1133
 *
 * @param [in] callback
 *    uint32_t callback represent the callback that gets add scheduler after
 *    the i2c procedure is complete
 *
 * @param [in] storeData
 *    uint32_t *storeData points to the location that the data red in should be
 *    stored
 *
 * @param [in] write_data
 *    uint32_t write_data stores the bits to be written to the si1133, null if
 *    a read is taking place
 *
 ******************************************************************************/
void i2c_start(I2C_TypeDef *i2c, bool readTrue, uint32_t bytes, uint32_t deviceAddress,
               uint32_t registerAddress, uint32_t callback, uint32_t *storeData,
               uint32_t write_data){

  if (i2c == I2C0){
      i2c_local_struct = &i2c0_state_struct;
  }else if(i2c == I2C1){
      i2c_local_struct = &i2c1_state_struct;
  }

  while(i2c_local_struct->busy);
  EFM_ASSERT((i2c->STATE & _I2C_STATE_STATE_MASK) == I2C_STATE_STATE_IDLE);

  i2c_local_struct->i2c = i2c;
  i2c_local_struct->readTrue = readTrue;
  i2c_local_struct->numOfBytes = bytes;
  i2c_local_struct->deviceAddress = deviceAddress;
  i2c_local_struct->registerAddress = registerAddress;
  i2c_local_struct->callback = callback;
  i2c_local_struct->storeData = storeData;
  i2c_local_struct->writeData = write_data;

  sleep_block_mode(EM2);
  i2c_local_struct->busy = true;
  i2c_local_struct->currentState = initialize; // WHAT DO I INITLAIZE THIS TO

  i2c->CMD = I2C_CMD_START;
  i2c->TXDATA = (deviceAddress<<1 | 0);

}

/***************************************************************************//**
 * @brief
 *  IRQHandler for I2C0 peripheral, called when interrupts are flagged in I2C0 peripheral.
 *
 * @details
 *  Function calls, managing ACK interrupts from Si1133 sensor to monitor states
 *  to enter for next process.
 *
 * @note
 *  IRQHandler specific to the I2C0 peripheral, seperate from I2C1.
 *
 ******************************************************************************/
void I2C0_IRQHandler(void){
  uint32_t int_flag;
  int_flag = I2C0->IF & I2C0->IEN;
  I2C0->IFC = int_flag;

  if(int_flag & I2C_IF_ACK){
      EFM_ASSERT(!(I2C0->IF & I2C_IF_ACK));
      i2c_ack_sm(&i2c0_state_struct);
  }
  if(int_flag & I2C_IF_RXDATAV){
      EFM_ASSERT(!(I2C0->IF & I2C_IF_RXDATAV));
      i2c_read_sm(&i2c0_state_struct);
  }
  if(int_flag & I2C_IF_MSTOP){
      EFM_ASSERT(!(I2C0->IF & I2C_IF_MSTOP));
      i2c_stop_sm(&i2c0_state_struct);
  }
}


/***************************************************************************//**
 * @brief
 *  IRQHandler for I2C1 peripheral, called when interrupts are flagged in I2C1 peripheral.
 *
 * @details
 *  Function calls, managing ACK interrupts from Si1133 sensor to monitor states
 *  to enter for next process.
 *
 * @note
 *  IRQHandler specific to the I2C1 peripheral, seperate from I2C0.
 *
 ******************************************************************************/
void I2C1_IRQHandler(void){
  uint32_t int_flag;
  int_flag = I2C1->IF & I2C1->IEN;
  I2C1->IFC = int_flag;

  if(int_flag & I2C_IF_ACK){
      EFM_ASSERT(!(I2C1->IF & I2C_IF_ACK));
      i2c_ack_sm(&i2c1_state_struct);
  }
  if(int_flag & I2C_IF_RXDATAV){
      //EFM_ASSERT(!(I2C1->IF & I2C_IF_RXDATAV));
      i2c_read_sm(&i2c1_state_struct);
  }
  if(int_flag & I2C_IF_MSTOP){
      EFM_ASSERT(!(I2C1->IF & I2C_IF_MSTOP));
      i2c_stop_sm(&i2c1_state_struct);
  }
}

/***************************************************************************//**
 * @brief
 *  Function accessing steps when transitioning states from an ACK from the Si1133 sensor
 *
 * @details
 *  Specific to ACK command recieved from Si1133 sensor.
 *
 * @note
 *  This function is specific to a particular state and interrupt command, other commands and interrupts have seperate functions.
 *
 * @param [in] i2c_sm
 *  I2C_STATE_MACHINE *i2c_sm which holds the state of the I2C peripheral and configuring data.
 *
 ******************************************************************************/
void i2c_ack_sm(I2C_STATE_MACHINE *i2c_sm){
  switch (i2c_sm->currentState) {

    case initialize:
      //send RA
      i2c_sm->i2c->TXDATA = i2c_sm->registerAddress;
      i2c_sm->currentState = send_RA;
      break;

    case send_RA:
      if(i2c_sm->readTrue){
          i2c_sm->i2c->CMD = I2C_CMD_START;
          i2c_sm->i2c->TXDATA = (i2c_sm->deviceAddress<<1 | 1);
          i2c_sm->currentState = send_DA;
      } else {
          i2c_sm->numOfBytes--;
          i2c_sm->i2c->TXDATA = (i2c_sm->writeData>>(8*i2c_sm->numOfBytes)) & MASK; //writing data to Si1133
          i2c_sm->currentState = write_data;
      }
      break;

    case send_DA:
      if(i2c_sm->readTrue){
          i2c_sm->currentState = read_data;
      } else {
          EFM_ASSERT(false);
      }
      break;

    case read_data:
      EFM_ASSERT(false);
      break;

    case write_data:
      if(i2c_sm->numOfBytes > 0){
          i2c_sm->numOfBytes--;
          i2c_sm->i2c->TXDATA = (i2c_sm->writeData>>(8*i2c_sm->numOfBytes)) & MASK; //writing data to Si1133
      } else {

          i2c_sm->i2c->CMD = I2C_CMD_STOP;
          i2c_sm->currentState = send_stop;
      }

      break;

    case send_stop:
      EFM_ASSERT(false);
      break;

    default:
      EFM_ASSERT(false);
  }
}

/***************************************************************************//**
 * @brief
 *  Function accessing steps when transitioning states from an read state within the Thunderboard
 *
 * @details
 *  Specific to read state within the Mighty Gecko to receive data from the Si1133 TXDATA buffer.
 *
 * @note
 *  This function is specific to a particular state and interrupt command, other commands and interrupts have seperate functions.
 *
 * @param [in] i2c_sm
 *  I2C_STATE_MACHINE *i2c_sm which holds the state of the I2C peripheral and configuring data.
 *
 ******************************************************************************/
void i2c_read_sm(I2C_STATE_MACHINE *i2c_sm){
  switch (i2c_sm->currentState) {
    case initialize:
      EFM_ASSERT(false);
      break;
    case send_DA:
      EFM_ASSERT(false);
      break;
    case send_RA:
      EFM_ASSERT(false);
      break;

    case read_data:
      if(i2c_sm->numOfBytes > 0){
          *(i2c_sm->storeData) = ((i2c_sm->i2c->RXDATA)<<(8*(i2c_sm->numOfBytes-1))); //either shift data over or make it an array
//          i2c_sm->storeData++;
          i2c_sm->numOfBytes--;
          if(i2c_sm->numOfBytes == 0){
              i2c_sm->i2c->CMD = I2C_CMD_NACK;
              i2c_sm->i2c->CMD = I2C_CMD_STOP;
              i2c_sm->currentState = send_stop;
          } else {
              i2c_sm->i2c->CMD = I2C_CMD_ACK;
          }
      } else {
          EFM_ASSERT(false);
      }

      break;

    case write_data:
      EFM_ASSERT(false);
      break;

    case send_stop:
      EFM_ASSERT(false);
      break;

    default:
      EFM_ASSERT(false);
  }
}

/*******************************************************************************
 * @brief
 *  Function accessing steps when transitioning states from a STOP command from the Thunderboard to end communication.
 * @details
 *  i2c_stop_sm() is called when the MSTOP interrupt is flagged after the master
 *  has sent a stop bit
 *
 * @note
 *  Only the send_stop state has actions in this interrupt
 *
 * @param [in] i2c_sm
 *  I2C_STATE_MACHINE *i2c_sm which holds the state of the I2C peripheral and configuring data.
 *
 ******************************************************************************/
void i2c_stop_sm(I2C_STATE_MACHINE *i2c_sm){
  switch (i2c_sm->currentState) {
    case initialize:
      EFM_ASSERT(false);
      break;

    case send_DA:
      EFM_ASSERT(false);
      break;

    case send_RA:
      EFM_ASSERT(false);
      break;

    case read_data:
      EFM_ASSERT(false);
      break;

    case write_data:
      EFM_ASSERT(false);
      break;

    case send_stop:
      add_scheduled_event(i2c_sm->callback);
      sleep_unblock_mode(EM2);
      i2c_sm->busy = false;
      break;

    default:
      EFM_ASSERT(false);
  }
}

/******************************************************************************
 * @brief
 *    Function returns the busy bit from the i2c state struct
 *
 * @details
 *    Get_i2c_busy() checks whether I2C0 OR I2C1 is being used, then returns the
 *    corresponding busy bool variable in the i2c state struct
 *
 * @note
 *    Used in Si1133_config() function to check to see whether the i2c is being
 *    used or not
 *
 * @param [in] i2c_sm
 *    I2C_STATE_MACHINE *i2c_sm which holds the state of the I2C peripheral and configuring data.
 *
 ******************************************************************************/
bool get_i2c_busy(I2C_TypeDef *i2c){
  if(i2c == I2C0){
      return i2c0_state_struct.busy;
  } else {
      return i2c1_state_struct.busy;
  }
}
