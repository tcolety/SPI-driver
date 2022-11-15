/**
 * @file   Si1133.c
 * @author Taylor Colety
 * @date   10/05/21
 * @brief  Working with the Si1133 Sensor.
 *
 */

//***********************************************************************************
// Include files
//***********************************************************************************
#include "Si1133.h"

//***********************************************************************************
// private define statements
//***********************************************************************************

//***********************************************************************************
// private variables
//***********************************************************************************
uint32_t si1133_read_result;

//***********************************************************************************
// private function prototypes
//***********************************************************************************
void Si1133_configure(void);

//***********************************************************************************
// private functions
//***********************************************************************************
/******************************************************************************
 * @brief
 *  This function configures the si1133 to sense white light
 *
 * @details
 *   Si1133_configure() configures the si1133 to measure and record the value
 *   of what light by writing to the sensor and checking to make sure the sensor
 *   has received the command.
 *
 * @note
 *   This function must be called before si1133 will measure white light
 *
 ******************************************************************************/
void Si1133_configure(void){
  uint32_t command_ctrl1, command_ctrl2;

  Si1133_read(NO_CALLBACK, RESPONSE0_REG, RESPONSE0_BYTES);
  while(get_i2c_busy(I2C1));
  command_ctrl1 = (si1133_read_result)&(BIT_MASK);

  Si1133_write(NO_CALLBACK, INPUT0_REG, INPUT0_BYTES, ADCMUX_WHITE);
  while(get_i2c_busy(I2C1));

  Si1133_write(NO_CALLBACK, COMMAND_REG, COMMAND_BYTES, (PARAM_SET|ADCCONFIG0));
  while(get_i2c_busy(I2C1));

  Si1133_read(NO_CALLBACK, RESPONSE0_REG, RESPONSE0_BYTES);
  while(get_i2c_busy(I2C1));
  command_ctrl2 = (si1133_read_result)&(BIT_MASK);

  if(!(command_ctrl1 == (command_ctrl2-ONE)%DIVISOR)){
      EFM_ASSERT(false);
  }

  Si1133_write(NO_CALLBACK, INPUT0_REG, INPUT0_BYTES, INPUT0_WRITE);
  while(get_i2c_busy(I2C1));
  Si1133_write(NO_CALLBACK, COMMAND_REG, COMMAND_BYTES, (PARAM_SET|CHAN_LIST));
  while(get_i2c_busy(I2C1));
  Si1133_read(NO_CALLBACK, RESPONSE0_REG, RESPONSE0_BYTES);
  while(get_i2c_busy(I2C1));

  command_ctrl2 = (si1133_read_result)&(BIT_MASK);
  if(!(command_ctrl1 == (command_ctrl2-TWO)%DIVISOR)){
      EFM_ASSERT(false);
  }
}

//***********************************************************************************
// global functions
//***********************************************************************************
/***************************************************************************//**
 * @brief
 *  Initializing our Si1133 data
 *
 * @details
 *  Passing in the I2C struct, we can store each required parameter to fill our Si1133
 *  sensor with required data defined in the HAL documentation and Si1133 datasheet.
 *  Then, i2c_open is called to start the i2c peripheral
 *
 * @note
 *  Called in app.c when opening all other peripherals and drivers.
 *
 ******************************************************************************/
void Si1133_i2c_open(void){
   I2C_OPEN_STRUCT i2c_setup_struct;

   timer_delay(HARDWARE_DELAY);

   i2c_setup_struct.enable = true;
   i2c_setup_struct.master = true;
   i2c_setup_struct.refFreq = 0;
   i2c_setup_struct.freq = I2C_FREQ_FAST_MAX;
   i2c_setup_struct.clhr = i2cClockHLRAsymetric;
   i2c_setup_struct.out_pin_route_scl = SCL_ROUTE;
   i2c_setup_struct.out_pin_route_sda = SDA_ROUTE;
   i2c_setup_struct.out_pin_scl_en = true;   // enable out 0 route
   i2c_setup_struct.out_pin_sda_en = true;   // enable out 1 route

   i2c_open(I2C1, &i2c_setup_struct);

   Si1133_configure();
}

 /***************************************************************************//**
  * @brief
  *  Calls the i2c_start function with correct values to start process of reading
  *  with I2C
  *
  * @details
  *  This function begins the i2c read process or reading the Si1133 part ID,
  *  however it can also be used to read from other registers.
  *
  * @note
  *  Called with the comp1 callback function is triggered
  *
  * @param [in] callback
  *  Parameters for each input to I2C_TypeDef Struct to pass into other function.
  *
  *  @param [in] register_address
  *   Parameter defines the register address in si1133 that i2c is reading from
  *
  *  @param [in] bytes
  *   Specifies number of bytes I2C is reading
  ******************************************************************************/
void Si1133_read(uint32_t callback, uint32_t register_address, uint32_t bytes){ //bytes ==1
   i2c_start(I2C1, true, bytes, DEVICE_ADDRESS, register_address, callback, &si1133_read_result, NOP);
}

/******************************************************************************
* @brief
*   Function begins an i2c write to the si1133 register of choice
*
* @details
*   Si1133_write begins a write to si1133. The inputs of the function are a
*   callback, register address, number of bytes, and the bits to be written
*
* @note
*   this function is called in si1133_configure() and si1133_force_cmd()
*
* @param [in] callback
*  Parameters for each input to I2C_TypeDef Struct to pass into other function.
*
* @param [in] register_address
*  Parameter defines the register address in si1133 that i2c is reading from
*
* @param [in] bytes
*  Specifies number of bytes I2C is reading
*
* @param [in] write_data
*  Specifies number of bytes I2C is reading
*
 ******************************************************************************/
void Si1133_write(uint32_t callback, uint32_t register_address, uint32_t bytes, uint32_t write_data){ //bytes == 3
  i2c_start(I2C1, false, bytes, DEVICE_ADDRESS, register_address, callback, NOP, write_data);
 }

/******************************************************************************
* @brief
*      Function to get the part ID
* @details
*      Si1133_get_PartID reads and returns the private variable Si1133_read_result
*      which stores the Si1133 Part ID which has been read with I2C
*
******************************************************************************/
 uint32_t Si1133_get_read_result(void) {
   uint32_t Si1133_result;
   Si1133_result = si1133_read_result;
   return Si1133_result;
 }

/******************************************************************************
* @brief
*   Function calls the force command to the si1133 command register
*
* @details
*   si1133_force_cmd calls si1133_write() to write specific bits to the
*   command register to call the force command
*
* @note
* This function is called in the comp1 interrupt function
*
******************************************************************************/
 void Si1133_force_cmd(void){
   Si1133_write(NO_CALLBACK, COMMAND_REG, COMMAND_BYTES, FORCE);
 }

/******************************************************************************
* @brief
*   Function calls si1133_read() to read the data measure by the sensor
*
* @details
*   Si1133_request() is designed to read the output of the si1133 sensor after
*   it has been configured to measure light. This reads data from the
*   HOSTOUT0 register in the si1133.
*
* @note
*   This function is called in the underflow interrupt callback function.
*
* @param [in] callback
*   Input the callback to be added to the scheduler after the i2c procedure has
*   finished
*
******************************************************************************/
 void Si1133_request(uint32_t callback){
    Si1133_read(callback, HOSTOUT0_REG, HOSTOUT0_BYTES);
  }
