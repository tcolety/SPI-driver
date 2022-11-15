//***********************************************************************************
// Include files
//***********************************************************************************
#ifndef APP_HG
#define APP_HG

/* System include statements */


/* Silicon Labs include statements */
#include "em_cmu.h"
#include "em_assert.h"

/* The developer's include statements */
#include "cmu.h"
#include "gpio.h"
#include "letimer.h"
#include "brd_config.h"
#include "scheduler.h"
#include "LEDs_thunderboard.h"
#include "Si1133.h"
#include "icm20648.h"
#include "ble.h"
#include "HW_delay.h"
#include <stdio.h>

//***********************************************************************************
// defined files
//***********************************************************************************
#define   PWM_PER         2   // PWM period in seconds
#define   PWM_ACT_PER     0.002  // PWM active period in seconds

// Application scheduled events
#define NULL_CB               0x0
#define LETIMER0_COMP0_CB     0x00000001 //0b0001
#define LETIMER0_COMP1_CB     0x00000002 //0b0010
#define LETIMER0_UF_CB        0x00000004 //0b0100
#define SI1133_LIGHT_READ_CB  0x00000008 //0b1000
#define BOOT_UP_CB            0x00000010 //0b0001_0000
#define RX_EVENT_CB           0x00000020 //0b0010_0000
#define TX_EVENT_CB           0x00000040 //0b0100_0000
#define BLE_TX_DONE_CB        0x00000080 //0b1000_0000
#define ICM20648_READ_CB      0x00000100

#define EXPECTED_RESULTS       20

#define REGISTER_ADDRESS       0
#define BYTES                  1

#define ACCEL_ZOUT_H_REG       0x31
#define ACCEL_ZOUT_L_REG       0x32
#define ACCEL_ZOUT_BYTES       2

#define UPSIDEDOWN_VALUE       0

#define SYSTEM_BLOCK_EM        EM3

#define DELAY_2                2000
#define ARRAYSIZE              64
//#define BLE_TEST_ENABLED

//***********************************************************************************
// global variables
//***********************************************************************************


//***********************************************************************************
// function prototypes
//***********************************************************************************
void app_peripheral_setup(void);
void app_led_init(void);
void scheduled_letimer0_comp0_cb (void);
void scheduled_letimer0_comp1_cb (void);
void scheduled_letimer0_uf_cb (void);
void schedule_si1133_light_read_cb(void);
void scheduled_boot_up_cb(void);
void scheduled_icm20648_read_cb(void);

#endif
