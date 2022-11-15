/**
 * @file leuart.c
 * @author
 * @date
 * @brief Contains all the functions of the LEUART peripheral
 *
 */

//***********************************************************************************
// Include files
//***********************************************************************************

//** Standard Library includes
#include <string.h>

//** Silicon Labs include files
#include "em_gpio.h"
#include "em_cmu.h"

//** Developer/user include files
#include "leuart.h"

//***********************************************************************************
// defined files
//***********************************************************************************


//***********************************************************************************
// private variables
//***********************************************************************************
uint32_t	rx_done_evt;
uint32_t	tx_done_evt;
bool		leuart0_tx_busy;

static LEUART_STATE_MACHINE leuart0_state_struct;

/***************************************************************************//**
 * @brief LEUART driver
 * @details
 *  This module contains all the functions to support the driver's state
 *  machine to transmit a string of data across the LEUART bus.  There are
 *  additional functions to support the Test Driven Development test that
 *  is used to validate the basic set up of the LEUART peripheral.  The
 *  TDD test for this class assumes that the LEUART is connected to the HM-18
 *  BLE module.  These TDD support functions could be used for any TDD test
 *  to validate the correct setup of the LEUART.
 *
 ******************************************************************************/

//***********************************************************************************
// Private prototypes
//***********************************************************************************
void leuart_txbl_sm(LEUART_STATE_MACHINE *leuart_sm);
void leuart_txc_sm(LEUART_STATE_MACHINE *leuart_sm);

//***********************************************************************************
// Private functions
//***********************************************************************************
/***************************************************************************//**
 * @brief
 *  The state machine function triggered by the transmit buffer empty interrupt
 *
 * @details
 *  This function is entered when data can be placed into the transmit buffer in
 *  the send data state. The array of characters to be sent is indexed through
 *  and transmitted. When the index values reaches the length, then the state is
 *  changed to end process and the TXC interrupt is enabled and TXBL is
 *  disabled.
 *
 * @param [in] leuart_sm
 *  struct holding information to be accessed by the state machine
 ******************************************************************************/
void leuart_txbl_sm(LEUART_STATE_MACHINE *leuart_sm){
  switch (leuart_sm->currentState) {

    case send_data:
      if(leuart_sm->i < leuart_sm->length) {
          leuart_sm->leuart->TXDATA = leuart_sm->data[leuart_sm->i];
          leuart_sm->i++;
      } else {
          leuart_sm->leuart->IEN &= ~LEUART_IF_TXBL;
          leuart_sm->leuart->IEN |= LEUART_IF_TXC;
          leuart_sm->currentState = end_process;
      }
      break;

    case end_process:
      EFM_ASSERT(false);
      break;

    default:
      EFM_ASSERT(false);
      break;

  }
}

/***************************************************************************//**
 * @brief
 *  The state machine function triggered by the transmit clear interrupt
 *
 * @details
 *  This function is entered when all the data has been sent and the TXC interrupt
 *  has been triggered. Here energy mode 3 is unblocked, the leuart is busy bit
 *  is set to false, the TXC interrupt is disabled, and an event is added to
 *  the scheduler.
 *
 * @param [in] leuart_sm
 *  struct holding information to be accessed by the state machine
 *
 ******************************************************************************/
void leuart_txc_sm(LEUART_STATE_MACHINE *leuart_sm){
  switch (leuart_sm->currentState) {

    case send_data:
      EFM_ASSERT(false);
      break;

    case end_process:
      sleep_unblock_mode(LEUART_TX_EM);
      leuart_sm->busy = false;
      leuart_sm->leuart->IEN &= ~LEUART_IF_TXC;
      add_scheduled_event(leuart_sm->callback);
      break;
      EFM_ASSERT(false);
      break;

    default:
      EFM_ASSERT(false);
      break;

  }
}

//***********************************************************************************
// Global functions
//***********************************************************************************

/***************************************************************************//**
 * @brief
 *  Performs the necessary actions to setup the LEUART
 *
 * @details
 *  First, this function enables the LEUART0 clock and checks that leuart's
 *  start frame can be set and read. Next, it sets the values for the init typedef
 *  and initializes the leuart with the LEUART_Init function. Then, it sets the pin
 *  location and enable routes, clears the tx and rx buffers, and clears the
 *  interrupts. Lastly, it enables LEUART and enables its interrupts through
 *  NVIC_EnableIRQ().
 *
 * @notes
 *  Must be called before LEUART can work
 *
 * @param [in] leuart
 *  Defines the leuart peripheral to access
 *
 * @param [in] leuart_settings
 *  struct containing values for the init typedef struct
 *
 ******************************************************************************/

void leuart_open(LEUART_TypeDef *leuart, LEUART_OPEN_STRUCT *leuart_settings){
  LEUART_Init_TypeDef leuart_values;

  if (leuart == LEUART0){
    CMU_ClockEnable(cmuClock_LEUART0,true);
  } else {
      EFM_ASSERT(false);
  }

  leuart->STARTFRAME = true;
  while(leuart->SYNCBUSY);
  EFM_ASSERT(leuart->STARTFRAME == true);
  leuart->STARTFRAME = false;
  while(leuart->SYNCBUSY);
  //EFM_ASSERT(leuart->STARTFRAME == true);

  leuart_values.baudrate = leuart_settings->baudrate;
  leuart_values.databits = leuart_settings->databits;
  leuart_values.enable = leuart_settings->enable; //needs to be enumeration for disable
  leuart_values.parity = leuart_settings->parity;
  leuart_values.refFreq = leuart_settings->refFreq;
  leuart_values.stopbits = leuart_settings->stopbits;

  LEUART_Init(leuart, &leuart_values);
  while(leuart->SYNCBUSY);

  leuart->ROUTELOC0 = leuart_settings->tx_loc | leuart_settings->rx_loc;

  leuart->ROUTEPEN = (LEUART_ROUTEPEN_TXPEN*leuart_settings->tx_pin_en)
      | (LEUART_ROUTEPEN_RXPEN*leuart_settings->rx_pin_en);

  leuart->CMD = (LEUART_CMD_CLEARTX)|(LEUART_CMD_CLEARRX);

  leuart->IFC = leuart->IF;

  while(leuart->SYNCBUSY);

  LEUART_Enable(leuart,leuartEnable);

  while(!((leuart->STATUS & LEUART_STATUS_RXENS)&&(leuart->STATUS & LEUART_STATUS_TXENS)))
  EFM_ASSERT((leuart->STATUS & LEUART_STATUS_RXENS)&&(leuart->STATUS & LEUART_STATUS_TXENS));

  NVIC_EnableIRQ(LEUART0_IRQn);
}

/***************************************************************************//**
 * @brief
 *  Responds to interrupts in LEUART0
 *
 * @details
 *  This function is called when the enabled interrupts go off. In this application
 *  the TXBL and TXC interrupts are the only interrupts it checks.
 *
 ******************************************************************************/

void LEUART0_IRQHandler(void){
  uint32_t int_flag;
  int_flag = LEUART0->IF & LEUART0->IEN;
  LEUART0->IFC = int_flag;

  if(int_flag & LEUART_IF_TXBL){
      //EFM_ASSERT(!(LEUART0->IF & LEUART_IF_TXBL));
      leuart_txbl_sm(&leuart0_state_struct);
  }
  if(int_flag & LEUART_IF_TXC){
      EFM_ASSERT(!(LEUART0->IF & LEUART_IF_TXC));
      leuart_txc_sm(&leuart0_state_struct);
  }
}

/***************************************************************************//**
 * @brief
 *  Function used to start the LEUART peripheral
 *
 *  @details
 *    This function initializes variables in the state struct, leuart0_state_struct
 *    with values passed into the function. Next it blocks energy mode 3, sets
 *    the current state equal to send_data, and enables the TXBL interrupt in
 *    order to allow the LEUART to start
 *
 *  @param [in] leuart
 *    Defines the leuart peripheral to access
 *
 *  @param [in] string
 *    A pointer to a the string to be transmitted
 *
 *  @param [in] string_len
 *    The number of chars in the string
 *
 ******************************************************************************/
void leuart_start(LEUART_TypeDef *leuart, char *string, uint32_t string_len){
  while(leuart0_state_struct.busy);

  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();

  leuart0_state_struct.leuart = leuart;
  strcpy(leuart0_state_struct.data, string);
  leuart0_state_struct.length = string_len;
  leuart0_state_struct.busy = true;
  leuart0_state_struct.i = 0;

  sleep_block_mode(LEUART_TX_EM);
  leuart0_state_struct.currentState = send_data;
  leuart->IEN |= LEUART_IF_TXBL;

  CORE_EXIT_CRITICAL();
}

/***************************************************************************//**
 * @brief
 *  Returns 1 if leuart is busy and 0 is leuart is avaliable
 *
 * @details
 *  leuart_tx_busy checks if LEUART0 is being used and reutrns the busy bit
 *  from the leuart0_state_struct. If LEUART0 is not the typedef passed in as
 *  the input, then it gets stuck in a while(1) loop in EFM_ASSERT(false).
 *
 * @param[in] *leuart
 *  Defines the leuart peripheral to access.
 ******************************************************************************/
bool leuart_tx_busy(LEUART_TypeDef *leuart){
  if(leuart == LEUART0) {
      return leuart0_state_struct.busy;
  } else {
      EFM_ASSERT(false);
  }
  EFM_ASSERT(false);
  return false;
}

/***************************************************************************//**
 * @brief
 *   LEUART STATUS function returns the STATUS of the peripheral for the
 *   TDD test
 *
 * @details
 * 	 This function enables the LEUART STATUS register to be provided to
 * 	 a function outside this .c module.
 *
 * @param[in] *leuart
 *   Defines the LEUART peripheral to access.
 *
 * @return
 * 	 Returns the STATUS register value as an uint32_t value
 *
 ******************************************************************************/

uint32_t leuart_status(LEUART_TypeDef *leuart){
	uint32_t	status_reg;
	status_reg = leuart->STATUS;
	return status_reg;
}

/***************************************************************************//**
 * @brief
 *   LEUART CMD Write sends a command to the CMD register
 *
 * @details
 * 	 This function is used by the TDD test function to program the LEUART
 * 	 for the TDD tests.
 *
 * @note
 *   Before exiting this function to update  the CMD register, it must
 *   perform a SYNCBUSY while loop to ensure that the CMD has by synchronized
 *   to the lower frequency LEUART domain.
 *
 * @param[in] *leuart
 *   Defines the LEUART peripheral to access.
 *
 * @param[in] cmd_update
 * 	 The value to write into the CMD register
 *
 ******************************************************************************/

void leuart_cmd_write(LEUART_TypeDef *leuart, uint32_t cmd_update){

	leuart->CMD = cmd_update;
	while(leuart->SYNCBUSY);
}

/***************************************************************************//**
 * @brief
 *   LEUART IF Reset resets all interrupt flag bits that can be cleared
 *   through the Interrupt Flag Clear register
 *
 * @details
 * 	 This function is used by the TDD test program to clear interrupts before
 * 	 the TDD tests and to reset the LEUART interrupts before the TDD
 * 	 exits
 *
 * @param[in] *leuart
 *   Defines the LEUART peripheral to access.
 *
 ******************************************************************************/

void leuart_if_reset(LEUART_TypeDef *leuart){
	leuart->IFC = 0xffffffff;
}

/***************************************************************************//**
 * @brief
 *   LEUART App Transmit Byte transmits a byte for the LEUART TDD test
 *
 * @details
 * 	 The BLE module will respond to AT commands if the BLE module is not
 * 	 connected to the phone app.  To validate the minimal functionality
 * 	 of the LEUART peripheral, write and reads to the LEUART will be
 * 	 performed by polling and not interrupts.
 *
 * @note
 *   In polling a transmit byte, a while statement checking for the TXBL
 *   bit in the Interrupt Flag register is required before writing the
 *   TXDATA register.
 *
 * @param[in] *leuart
 *   Defines the LEUART peripheral to access.
 *
 * @param[in] data_out
 *   Byte to be transmitted by the LEUART peripheral
 *
 ******************************************************************************/

void leuart_app_transmit_byte(LEUART_TypeDef *leuart, uint8_t data_out){
	while (!(leuart->IF & LEUART_IF_TXBL));
	leuart->TXDATA = data_out;
}


/***************************************************************************//**
 * @brief
 *   LEUART App Receive Byte polls a receive byte for the LEUART TDD test
 *
 * @details
 * 	 The BLE module will respond to AT commands if the BLE module is not
 * 	 connected to the phone app.  To validate the minimal functionality
 * 	 of the LEUART peripheral, write and reads to the LEUART will be
 * 	 performed by polling and not interrupts.
 *
 * @note
 *   In polling a receive byte, a while statement checking for the RXDATAV
 *   bit in the Interrupt Flag register is required before reading the
 *   RXDATA register.
 *
 * @param[in] leuart
 *   Defines the LEUART peripheral to access.
 *
 * @return
 * 	 Returns the byte read from the LEUART peripheral
 *
 ******************************************************************************/

uint8_t leuart_app_receive_byte(LEUART_TypeDef *leuart){
	uint8_t leuart_data;
	while (!(leuart->IF & LEUART_IF_RXDATAV));
	leuart_data = leuart->RXDATA;
	return leuart_data;
}
