/**
 * @file   icm20648.c
 * @author Taylor Colety
 * @date   11/15/2021
 * @brief  Functions to use the icm20648 acclerometer
 *
 */
//***********************************************************************************
// Include files
//***********************************************************************************
#include "icm20648.h"

//***********************************************************************************
// private define statements
//***********************************************************************************

//***********************************************************************************
// private variables
//***********************************************************************************
uint32_t usart_read_result;

//***********************************************************************************
// private function prototypes
//***********************************************************************************
void icm20648_config(void);

//***********************************************************************************
// private functions
//***********************************************************************************
/***************************************************************************//**
 * @brief
 *  This function is the tst driven development for the SPI peripheral
 *
 * @details
 *  icm20648_config() tests the performance of the spi peripheral by writing to
 *  different registers and reading from the register to check that the write
 *  save correctly
 *
 * @note
 *  The registers are in different banks (0-4), so REG_BANK_SEL must be written
 *  to in order to switch between banks.
 *
 ******************************************************************************/
void icm20648_config(void){
  //for each write to the config reg
  //we will do a read to confirm the write was done correctly

  //wait 22 microseconds
  timer_delay(ICM_WRITE_DELAY);

  icm20648_read(WHO_AM_I_REG, WHO_AM_I_BYTES, NOP);
  timer_delay(ICM_WRITE_DELAY);
  EFM_ASSERT(usart_read_result == ICM_DA);

  timer_delay(ICM_WRITE_DELAY);

  //enable low power mode and disable the temp sensor in PWR MGMT 1
  icm20648_write(PWR_MGMT_1_REG, PWR_MGMT_1_BYTES, PWR_MGMT_1_DATA, NOP);
  timer_delay(ICM_WRITE_DELAY);
  icm20648_read(PWR_MGMT_1_REG, PWR_MGMT_1_BYTES, NOP);
  timer_delay(ICM_WRITE_DELAY);
  EFM_ASSERT(usart_read_result == PWR_MGMT_1_DATA);

  //enable accel and disable gyroscope
  icm20648_write(PWR_MGMT_2_REG, PWR_MGMT_2_BYTES, PWR_MGMT_2_DATA, NOP);
  timer_delay(ICM_WRITE_DELAY);
  icm20648_read(PWR_MGMT_2_REG, PWR_MGMT_2_BYTES, NOP);
  timer_delay(ICM_WRITE_DELAY);
  EFM_ASSERT(usart_read_result == PWR_MGMT_2_DATA);

  //enable accel to operate in duty cycle mode
  icm20648_write(LP_CONFIG_REG, LP_CONFIG_BYTES, LP_CONFIG_DATA, NOP);
  timer_delay(ICM_WRITE_DELAY);
  icm20648_read(LP_CONFIG_REG, LP_CONFIG_BYTES, NOP);
  timer_delay(ICM_WRITE_DELAY);
  EFM_ASSERT(usart_read_result == LP_CONFIG_DATA);

  icm20648_write(REG_BANK_SEL_REG, REG_BANK_SEL_BYTES, REG_BANK_2_DATA, NOP);
  timer_delay(ICM_WRITE_DELAY);

  //set accel thresh to 240 mg
  icm20648_write(ACCEL_WOM_THR_REG, ACCEL_WOM_THR_BYTES, ACCEL_WOM_THR_DATA, NOP);
  timer_delay(ICM_WRITE_DELAY);
  icm20648_read(ACCEL_WOM_THR_REG, ACCEL_WOM_THR_BYTES, NOP);
  timer_delay(ICM_WRITE_DELAY);
  EFM_ASSERT(usart_read_result == ACCEL_WOM_THR_DATA);

  icm20648_write(REG_BANK_SEL_REG, REG_BANK_SEL_BYTES, REG_BANK_0_DATA, NOP);
  timer_delay(ICM_WRITE_DELAY);
}

//***********************************************************************************
// global functions
//***********************************************************************************/*
/***************************************************************************//**
 * @brief
 *  This function begins the setup of the accelerometer icm20648
 *
 * @details
 *  icm20648_open() defines all of the variables in the spi_setup_struct, calls
 *  spi_open with the setup struct as an input argument, and calls incm20648_config()
 *  to test the functionality of the spi peripheral. The struct spi_setup_struct
 *  has varibales that are used in the intialization struct, along with the pin
 *  locations and pin enables.
 *
 * @note
 *  This function is called in app_peripheral_setup in app.c
 *
 ******************************************************************************/
void icm20648_open(void){
  SPI_OPEN_STRUCT spi_setup_struct;

  spi_setup_struct.enable = ICM_ENABLE;
  spi_setup_struct.refFreq = ICM_REFFREQ;
  spi_setup_struct.baudrate = ICM_BAUDRATE;
  spi_setup_struct.databits = ICM_DATABITS;
  spi_setup_struct.master = ICM_MASTER;
  spi_setup_struct.msbf = ICM_MSBF;
  spi_setup_struct.clockMode = ICM_CLOCKMODE;
  spi_setup_struct.prsRxEnable = ICM_PRSRXENABLE;
  spi_setup_struct.prsRxCh = ICM_PRSRXCH;
  spi_setup_struct.autoTx = ICM_AUTOTX;
  spi_setup_struct.autoCsEnable = ICM_AUTOCSENABLE;
  spi_setup_struct.autoCsHold = ICM_AUTOCSHOLD;
  spi_setup_struct.autoCsSetup = ICM_AUTOCSSETUP;

  spi_setup_struct.tx_loc = USART_TX_ROUTE;
  spi_setup_struct.rx_loc = USART_RX_ROUTE;
  spi_setup_struct.cs_loc = USART_CS_ROUTE;
  spi_setup_struct.sclk_loc = USART_SCLK_ROUTE;

  spi_setup_struct.tx_pin_en = true;
  spi_setup_struct.rx_pin_en = true;
  spi_setup_struct.cs_pin_en = false;
  spi_setup_struct.sclk_pin_en = true;

  spi_open(ICM_USART, &spi_setup_struct);

  icm20648_config();
}


/***************************************************************************//**
 * @brief
 *  Starts a read with spi peripheral
 *
 * @details
 *  This function calls spi_start with the correct input argument in order to
 *  begin a read with the spi peripheral
 *
 * @note
 *  SPI peripheal must be set up with spi_open() before a read can be done
 *
 * @param [in] reg
 *  Indicates the register address to be read or written to
 *
 * @param [in] bytes
 *  Defines the number of bytes being read or written
 *
 * @param [in] callback
 *  Specifies the callback that is added to the scheduler after the state machine
 *  is complete
 *
 ******************************************************************************/
void icm20648_read(uint32_t reg, uint32_t bytes, uint32_t callback){
  spi_start(ICM_USART, true, bytes, reg, callback, &usart_read_result, NOP);
}

/***************************************************************************//**
 * @brief
 *  Starts a write with spi peripheral
 *
 * @details
 *  This function calls spi_start with the correct input argument in order to
 *  begin a write with the spi peripheral
 *
 * @note
 *  SPI peripheral must be set up with spi_open() before a write can be done
 *
 * @param [in] reg
 *  Indicates the register address to be read or written to
 *
 * @param [in] bytes
 *  Defines the number of bytes being read or written
 *
 * @param [in] writeData
 *  Data to be written with spi
 *
 * @param [in] callback
 *  Specifies the callback that is added to the scheduler after the state machine
 *  is complete
 *
 ******************************************************************************/
void icm20648_write(uint32_t reg, uint32_t bytes, uint32_t writeData, uint32_t callback){
  spi_start(ICM_USART, false, bytes, reg, callback, NOP, writeData);
}

/***************************************************************************//**
 * @brief
 *  Returns the value in usart_read_result which stores the bits read with spi
 *
 * @details
 *  This function is necessary since usart_read_result is a private variable
 *  and can only be access by functions in icm20648.c
 *
 * @note
 *  Called in app.c
 *
 ******************************************************************************/
uint16_t icm20648_get_read_result(void){
  return usart_read_result;
}
