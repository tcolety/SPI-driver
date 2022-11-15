#ifndef HEADER_FILES_SPI_H_
#define HEADER_FILES_SPI_H_

//***********************************************************************************
// include files
//***********************************************************************************
#include "em_usart.h"
#include "em_assert.h"
#include "sleep_routines.h"
#include "scheduler.h"
#include "em_cmu.h"
#include "brd_config.h"

//***********************************************************************************
// defined files
//***********************************************************************************
#define SPI_SLEEP_BLOCK EM2
#define SHIFTBY7        7
#define READBIT         1
#define WRITEBIT        0
#define GARBAGEDATA     0xFF
#define MASK            0xFF
#define EIGHT           8

//***********************************************************************************
// global variables
//***********************************************************************************
typedef struct {
  USART_Enable_TypeDef    enable;
  uint32_t                refFreq;
  uint32_t                baudrate;
  USART_Databits_TypeDef  databits;
  bool                    master;
  bool                    msbf;
  USART_ClockMode_TypeDef clockMode;
  bool                    prsRxEnable;
  USART_PRS_Channel_t     prsRxCh;
  bool                    autoTx;
  bool                    autoCsEnable;
  uint8_t                 autoCsHold;
  uint8_t                 autoCsSetup;

  uint32_t                tx_loc;
  uint32_t                rx_loc;
  uint32_t                cs_loc;
  uint32_t                sclk_loc;

  bool                    tx_pin_en;
  bool                    rx_pin_en;
  bool                    cs_pin_en;
  bool                    sclk_pin_en;

} SPI_OPEN_STRUCT;

typedef enum {
  sendRA,
  read,
  write,
} DEFINED_SPI_STATES;

typedef struct {
  DEFINED_SPI_STATES  currentState;

  USART_TypeDef     *usart;          //stores I2C0 OR I2C1 defined struct
  bool            readTrue;      //0=write 1=read
  uint32_t        readCounter;
  uint32_t        writeCounter;
  //uint32_t        deviceAddress;
  uint32_t        registerAddress;
  uint32_t        callback;
  uint32_t        *storeData;
  uint32_t        writeData;
  bool            bitBucketTrue;

  volatile bool   busy;
} SPI_STATE_MACHINE;

//***********************************************************************************
// function prototypes
//***********************************************************************************
void spi_open(USART_TypeDef *spi, SPI_OPEN_STRUCT *spi_settings);
void spi_start(USART_TypeDef *spi, bool readTrue, uint32_t bytes,
               uint32_t registerAddress, uint32_t callback, uint32_t *storeData,
               uint32_t write_data);
void USART0_RX_IRQHandler(void);
void usart_rxdatav_sm(SPI_STATE_MACHINE *spi_sm);
void USART0_TX_IRQHandler(void);
void usart_txbl_sm(SPI_STATE_MACHINE *spi_sm);
void usart_txc_sm(SPI_STATE_MACHINE *spi_sm);

#endif /* HEADER_FILES_SPI_H_ */
