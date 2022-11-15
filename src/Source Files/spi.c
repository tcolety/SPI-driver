/**
 * @file   spi.c
 * @author Taylor Colety
 * @date   9/12/2021
 * @brief  Functions to uses the SPI communication protocall
 *
 */
//***********************************************************************************
// Include files
//***********************************************************************************
#include "spi.h"

//***********************************************************************************
// private define statements
//***********************************************************************************

//***********************************************************************************
// private variables
//***********************************************************************************
static SPI_STATE_MACHINE usart_state_struct;

//***********************************************************************************
// private function prototypes
//***********************************************************************************

//***********************************************************************************
// private functions
//***********************************************************************************

//***********************************************************************************
// global functions
//***********************************************************************************
/***************************************************************************//**
 * @brief
 *  Function used to open the spi peripheral
 *
 * @details
 *  spi_open() enables the usart3 clock, checks the clock configuration, sets
 *  all of the variables in the intitalization struct, and calls the init
 *  function. Then, it sets the route location and enables the pins and clears
 *  the interrupts.
 *
 * @note
 *  This function is called in the icm20648_open() function
 *
 * @param [in] usart
 *  Defines the USART peripheral that is being used
 *
 * @param [in] spi_settings
 *  struct that contains variables needed for initialization struct
 *
 ******************************************************************************/
void spi_open(USART_TypeDef *usart, SPI_OPEN_STRUCT *spi_settings){
  USART_InitSync_TypeDef usart_init_struct;

  if (usart == USART3){
    CMU_ClockEnable(cmuClock_USART3,true);
  } else {
     EFM_ASSERT(false);
  }

  if((usart->IF & 0x1) == 0){
      usart->IFS = 0x1;
      EFM_ASSERT(usart->IF & 0x1);
      usart->IFC = 0x1;
  } else {
      usart->IFC = 0x1;
      EFM_ASSERT(!(usart->IF & 0x1));
  }

  usart_init_struct.enable = spi_settings->enable;
  usart_init_struct.refFreq = spi_settings->refFreq;
  usart_init_struct.baudrate = spi_settings->baudrate;
  usart_init_struct.databits = spi_settings->databits;
  usart_init_struct.master = spi_settings->master;
  usart_init_struct.msbf = spi_settings->msbf;
  usart_init_struct.clockMode = spi_settings->clockMode;
  usart_init_struct.prsRxEnable = spi_settings->prsRxEnable;
  usart_init_struct.prsRxCh = spi_settings->prsRxCh;
  usart_init_struct.autoTx = spi_settings->autoTx;
  usart_init_struct.autoCsEnable = spi_settings->autoCsEnable;
  usart_init_struct.autoCsHold = spi_settings->autoCsHold;
  usart_init_struct.autoCsSetup = spi_settings->autoCsSetup;

  USART_InitSync(usart, &usart_init_struct);

  usart->ROUTELOC0 = spi_settings->sclk_loc | spi_settings->cs_loc
      | spi_settings->tx_loc | spi_settings->rx_loc;

  usart->ROUTEPEN = (USART_ROUTEPEN_CLKPEN * spi_settings->sclk_pin_en)
      | (USART_ROUTEPEN_CSPEN * spi_settings->cs_pin_en)
      | (USART_ROUTEPEN_TXPEN * spi_settings->tx_pin_en)
      | (USART_ROUTEPEN_RXPEN * spi_settings->rx_pin_en);

  usart_state_struct.busy = false;

  usart->IFC = usart->IF;

  USART_Enable(usart, usartEnable);

  if (usart == USART3){
    NVIC_EnableIRQ(USART3_RX_IRQn);
    NVIC_EnableIRQ(USART3_TX_IRQn);
  } else {
      EFM_ASSERT(false);
  }

}

/***************************************************************************//**
 * @brief
 *  This function opens the spi state machine to begin a read or write of data
 *
 * @details
 *  spi_start() checks to make sure spi is not busy and sets all of the variables in
 *  the state machine struct according to the function's input variables. Then
 *  it blocks the correct sleep mode (EM2), sets the busy bit to true, sets
 *  chip select to low, and defines the current state of the state machine to
 *  be sendRA. Finally, the interrupts TXBL and RXDATAV are enabled and the
 *  receive buffer is cleared.
 *
 * @note
 *  This function must be called after spi_open
 *
 * @param [in] usart
 *  Defines the USART peripheral that is being used
 *
 * @param [in] readTrue
 *  Indicates whether the state machine should be reading or writing. If the
 *  variable is true, the a read must be done.
 *
 * @param [in] bytes
 *  Defines the number of bytes being read or written
 *
 * @param [in] registerAddress
 *  Indicates the register address to be read or written to
 *
 * @param [in] callback
 *  Specifies the callback that is added to the scheduler after the state machine
 *  is complete
 *
 * @param [in] storeData
 *  Pointer that points to location to store the data read in
 *
 * @param [in] writeData
 *  Data to be written with spi
 *
 ******************************************************************************/
void spi_start(USART_TypeDef *usart, bool readTrue, uint32_t bytes,
               uint32_t registerAddress, uint32_t callback, uint32_t *storeData,
               uint32_t write_data){

  while(usart_state_struct.busy);

//  CORE_DECLARE_IRQ_STATE;
//  CORE_ENTER_CRITICAL();

  usart_state_struct.usart = usart;
  usart_state_struct.readTrue = readTrue;
  usart_state_struct.registerAddress = registerAddress;
  usart_state_struct.callback = callback;
  usart_state_struct.storeData = storeData;
  usart_state_struct.writeData = write_data;
  usart_state_struct.bitBucketTrue = true;
  usart_state_struct.readCounter = bytes;
  usart_state_struct.writeCounter = bytes;
  *(usart_state_struct.storeData) = 0;

  sleep_block_mode(SPI_SLEEP_BLOCK);
  usart_state_struct.busy = true;
  GPIO_PinOutClear(USART_CS_PORT, USART_CS_PIN);
  usart_state_struct.currentState = sendRA;

  //usart->CMD = USART_CMD_CLEARRX;
  usart->IEN |= USART_IEN_TXBL | USART_IEN_RXDATAV;

//  CORE_EXIT_CRITICAL();

}

/***************************************************************************//**
 * @brief
 *  Interrupt handler for the RX USART interrupts
 *
 * @details
 *  This function checks if the RXDATAV interrupts has been triggered and enters
 *  the usart_rxdatav_sm function if so.
 *
 * @note
 *  the RXDATAV interrupt is enabled in spi_start()
 *
 ******************************************************************************/
void USART3_RX_IRQHandler(void){
  uint32_t int_flag;
  int_flag = USART3->IF & USART3->IEN;
  //USART3->IFC = int_flag;

  if(int_flag & USART_IF_RXDATAV){
      usart_rxdatav_sm(&usart_state_struct);
  }
}

/***************************************************************************//**
 * @brief
 *  the RXDATAV state machine function
 *
 * @details
 *  RXDATAV is triggered went data is in the recieve buffer. This function is
 *  called after the interrupt is triggered and is used to read data out of the
 *  RX buffer. If it is performing a read, the the data is stored but if if is
 *  a write, the data is written to a local variable that is discarded at the end
 *  of the function. After all the data has been read, the RXDATAV interrupt is
 *  disabled and the state machine is ended. After all the data
 *  has been written, the RXDATAV interrupt is disabled.
 *
 * @note
 *  The RXDATAV interrupt can only be disabled in this function and not the
 *  TXBL or TXC functions.
 *
 * @param [in] spi_sm
 *  SPI_STATE_MACHINE *spi_sm which holds variables needed to use the state
 *  machine
 ******************************************************************************/
void usart_rxdatav_sm(SPI_STATE_MACHINE *spi_sm){
  uint32_t bitBucket;
  bitBucket = spi_sm->usart->RXDATA;

  switch (spi_sm->currentState) {
    case sendRA:
      EFM_ASSERT(false);
      break;

    case read:
      if(spi_sm->bitBucketTrue) {
          spi_sm->bitBucketTrue = false;
      } else {
        spi_sm->readCounter--;
        *(spi_sm->storeData) = ((bitBucket)<<(EIGHT*spi_sm->readCounter))|*(spi_sm->storeData);
        if(spi_sm->readCounter == 0){
            //spi_sm->usart->IEN &= ~USART_IEN_RXDATAV;
            spi_sm->usart->IFC = USART_IEN_TXC;
            sleep_unblock_mode(SPI_SLEEP_BLOCK);
            spi_sm->busy = false;
            GPIO_PinOutSet(USART_CS_PORT, USART_CS_PIN);
            add_scheduled_event(spi_sm->callback);
        }
      }
      break;

    case write:
      //disable rxdatav interrupt when data has all been sent
//      if(spi_sm->writeCounter == 0){
//         spi_sm->usart->IEN &= ~USART_IEN_RXDATAV;
//      }
      break;

    default:
      EFM_ASSERT(false);
      break;
  }
  if(bitBucket){}
}

/***************************************************************************//**
 * @brief
 *  Interrupt handler for the TX USART interrupts
 *
 * @details
 *  This function checks if the TXBL and TXC interrupts has been triggered, then
 *  enters the usart_txbl_sm or usart_txc_sm function.
 *
 * @note
 *  the txbl interrupt is enabled in spi_start() and txc interrupt is enabled
 *  after all data has been written
 *
 ******************************************************************************/
void USART3_TX_IRQHandler(void){
  uint32_t int_flag;
  int_flag = USART3->IF & USART3->IEN;
  USART3->IFC = int_flag;

  if(int_flag & USART_IF_TXBL){
    //EFM_ASSERT(!(USART0->IF & USART_IF_TXBL));
    usart_txbl_sm(&usart_state_struct);
  }

  if(int_flag & USART_IF_TXC){
    EFM_ASSERT(!(USART3->IF & USART_IF_TXC));
    usart_txc_sm(&usart_state_struct);
  }
}

/***************************************************************************//**
 * @brief
 *  the function triggered by the TXBL interrupt
 *
 * @details
 *  This function is triggered by the TXBL interrupt and is used to transmit data
 *  with the SPI peripheral.
 *
 * @note
 *  The TXBL interrupt can only be disabled in this function and not the rxdatav
 *  state machine function
 *
 * @param [in] spi_sm
 *  SPI_STATE_MACHINE *spi_sm which holds variables needed to use the state
 *  machine
 *
 ******************************************************************************/
void usart_txbl_sm(SPI_STATE_MACHINE *spi_sm){
  switch (spi_sm->currentState) {
    case sendRA:
      if(spi_sm->readTrue) {
          spi_sm->usart->TXDATA = (spi_sm->registerAddress) | (READBIT << SHIFTBY7);
          spi_sm->currentState = read;
      } else {
          spi_sm->usart->TXDATA = (spi_sm->registerAddress) | (WRITEBIT << SHIFTBY7);
          spi_sm->currentState = write;
      }
      break;

    case read:
      spi_sm->usart->TXDATA = GARBAGEDATA;
      //disable txbl once all data has been read
      spi_sm->writeCounter--;
      if((spi_sm->writeCounter) == 0) {
          spi_sm->usart->IEN &= ~USART_IF_TXBL;
      }
      break;

    case write:
      spi_sm->writeCounter--;
      spi_sm->usart->TXDATA = (spi_sm->writeData>>(EIGHT*spi_sm->writeCounter)) & MASK;
      //disable txbl and enable txc when everything has been written
      if(spi_sm->writeCounter == 0) {
         spi_sm->usart->IEN &= ~USART_IEN_TXBL;
         spi_sm->usart->IEN |= USART_IEN_TXC;
      }
      break;

    default:
      EFM_ASSERT(false);
      break;
  }
}

/***************************************************************************//**
 * @brief
 *  the function triggered by the TXC interrupt
 *
 * @details
 *  This function is triggered by the TXC interrupt and is used to end the state
 *  machine when it is in a write state. In order to end the state machine the
 *  sleep mode is unblock, the busy bit is set to false, chip select is set to
 *  high, and the callback is added to the scheduler.
 *
 * @param [in] spi_sm
 *  SPI_STATE_MACHINE *spi_sm which holds variables needed to use the state
 *  machine
 *
 ******************************************************************************/
void usart_txc_sm(SPI_STATE_MACHINE *spi_sm){
  switch (spi_sm->currentState) {
    case sendRA:
      EFM_ASSERT(false);
      break;

    case read:
      EFM_ASSERT(false);
      break;

    case write:
      sleep_unblock_mode(SPI_SLEEP_BLOCK);
      spi_sm->busy = false;
      GPIO_PinOutSet(USART_CS_PORT, USART_CS_PIN);
      add_scheduled_event(spi_sm->callback);
      spi_sm->usart->IEN &= ~USART_IF_TXC;
      break;

    default:
      EFM_ASSERT(false);
      break;

  }
}
