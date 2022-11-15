/**
 * @file   app.c
 * @author Taylor Colety
 * @date   9/12/2021
 * @brief  Includes callback functions and functions that set up the peripherals
 *
 */

//***********************************************************************************
// Include files
//***********************************************************************************
#include "app.h"


//***********************************************************************************
// defined files
//***********************************************************************************


//***********************************************************************************
// Private variables
//***********************************************************************************
static unsigned int led_color;
static uint32_t x = 3;
static uint32_t y = 0;
static bool facingUpTrue;
static bool firstZRead = true;

//***********************************************************************************
// Private functions
//***********************************************************************************
static void app_letimer_pwm_open(float period, float act_period, uint32_t out0_route, uint32_t out1_route);

//***********************************************************************************
// Global functions
//***********************************************************************************

/***************************************************************************//**
 * @brief
 *    This sets up the peripherals
 *
 * @details
 *    This function enables the LF clock tree, the PWM struct, and initiates the
 *    start of LETIMER0. It also opens the sleep modes and initializes the
 *    LEDs. Also it starts the Si1133 and the i2c protocol.
 * @note
 *    This function passes the period, active period, and PWM routes 0-1 definitions
 *    to the private function app_letimer_pwm_open.
 *
 ******************************************************************************/
void app_peripheral_setup(void){
  cmu_open();
  gpio_open();
  sleep_open();
  scheduler_open();
  app_led_init();
  Si1133_i2c_open();
  icm20648_open();
  sleep_block_mode(SYSTEM_BLOCK_EM); //WHEN SHOULD THIS BE UNBLOCKED??????????????
  ble_open(0,0); //add callback
  app_letimer_pwm_open(PWM_PER, PWM_ACT_PER, PWM_ROUTE_0, PWM_ROUTE_1);
  add_scheduled_event(BOOT_UP_CB);
}

/******************************************************************************
* @brief
*   Function initializes LEDs
*
* @details
*   app_led_init() is called in app_peripheral setup to initialize the LEDs and
*   led_color variable to 0.
*
*
******************************************************************************/
void app_led_init(void){
  rgb_init();
  led_color = 0;
}

/***************************************************************************//**
 * @brief
 *    Function to create APP_LETIMER_PWM_TypeDef struct
 *
 * @details
 *    This function sets the period and active period of the PWM. It also
 *    enables route0 and route1. Additionally, it opens the scheduler.
 *
 * @param[in] period
 *    Passing in the period of the PWM
 *
 * @param[in] act_period
 *    The active period of the PWM
 *
 * @param[in] out0_route
 *    out 0 route to gpio port/pin
 *
 * @param[in] out1_route
 *    out 1 route to gpio port/pin
 *
 ******************************************************************************/
void app_letimer_pwm_open(float period, float act_period, uint32_t out0_route, uint32_t out1_route){
  // Initializing LETIMER0 for PWM operation by creating the
  // letimer_pwm_struct and initializing all of its elements
  // APP_LETIMER_PWM_TypeDef is defined in letimer.h
  APP_LETIMER_PWM_TypeDef   letimer_pwm_struct;

  letimer_pwm_struct.debugRun = false;
  letimer_pwm_struct.enable = false;
  letimer_pwm_struct.out_pin_route0 = out0_route;
  letimer_pwm_struct.out_pin_route1 = out1_route;
  letimer_pwm_struct.out_pin_0_en = false;
  letimer_pwm_struct.out_pin_1_en = false;
  letimer_pwm_struct.period = period;
  letimer_pwm_struct.active_period = act_period;
  letimer_pwm_struct.comp0_irq_enable = false;
  letimer_pwm_struct.comp0_cb = LETIMER0_COMP0_CB;
  letimer_pwm_struct.comp1_irq_enable = true;
  letimer_pwm_struct.comp1_cb = LETIMER0_COMP1_CB;
  letimer_pwm_struct.uf_irq_enable = true;
  letimer_pwm_struct.uf_cb = LETIMER0_UF_CB;

  letimer_pwm_open(LETIMER0, &letimer_pwm_struct);
}

/***************************************************************************//**
 * @brief
 *          Callback function for letimer0 COMP0
 * @details
 *          scheduled_letimer0_comp0_cb() should not be called in this application.
 *          EFM_ASSERT(false) is used to break the project if this function
 *          is called.
 ******************************************************************************/
void scheduled_letimer0_comp0_cb(void){
  EFM_ASSERT(false);
}

/***************************************************************************//**
 * @brief
 *          Callback function for letimer0 COMP1
 * @details
 *          scheduled_letimer0_comp1_cb() turns on the correct led by checking
 *          the value of led_color and enabling the corresponding LED.
 * @note
 *          Turns LED_1 red, green, and then blue in respective order
 ******************************************************************************/
void scheduled_letimer0_comp1_cb(void){
  Si1133_force_cmd();
}

/***************************************************************************//**
 * @brief
 *          Callback function for  letimer0 underflow flag, UF
 * @details
 *          scheduled_letimer0_uf_cb() turns off the correct led by checking
 *          the value of led_color and disabling the corresponding LED. Then it
 *          adds 1 to the led_color variable or reset the variable to 0 if at
 *          its maximum.
 * @note
 *          Function is called when underflow flag of letimer0 is set.
 ******************************************************************************/
void scheduled_letimer0_uf_cb(void){
  Si1133_request(SI1133_LIGHT_READ_CB);
  icm20648_read(ACCEL_ZOUT_H_REG, ACCEL_ZOUT_BYTES, ICM20648_READ_CB);

  float z;

  x = x + 3;
  y = y + 1;
  z = x/(y*1.0);
  char zstring[ARRAYSIZE];
  ble_write("z = ");
  //gcvt(z, 2, zstring);
  //strncat(zstring, &ch, 2);
  sprintf(zstring, "%3.1f\n", z);
  ble_write(zstring);
}

/*******************************************************************************
 * @brief
 *  Reads the value of the Si1133 Callback function for determining the value of
 *  the Callback functions result.
 *
 * @details
 *  Based on the value recieved back, the function compares with the defined
 *  expected result from earlier and manipulates the output of the LED based
 *  on this comparison.
 *
 * @note
 *  Only toggles the RED and GREEN LEDs on the Mighty Gecko and indicates a correct
 *  vs. an incorrect value.
 *
 ******************************************************************************/
 void schedule_si1133_light_read_cb(void){
   uint32_t read_result;
   read_result = Si1133_get_read_result();
   if(read_result < EXPECTED_RESULTS){
       leds_enabled(RGB_LED_1, COLOR_BLUE, true);
       ble_write("It's dark = ");
   } else {
       leds_enabled(RGB_LED_1, COLOR_BLUE, false);
       ble_write("It's light outside = ");
   }
   char readString[ARRAYSIZE];
   sprintf(readString, "%lu\n", read_result);
   ble_write(readString);
   ble_write("\n");

 }

 /*****************************************************************************
  * @brief
  *   Callback for the boot up event
  *
  * @details
  *   This function has the option of running ble_test to see if the bluetooth
  *   connects works. Then it writes Hello World to the modules that it is
  *   connected to. Finally, it opens the LETIMER peripheral.
  *
  * @note
  *   There is a two second delay to wait for ble_test to finish.
  *
  ******************************************************************************/
 void scheduled_boot_up_cb(void){

#ifdef BLE_TEST_ENABLED
   EFM_ASSERT(ble_test("TaylorBLE"));
   timer_delay(DELAY_2);
#endif

   ble_write("\nHello World\n");

   letimer_start(LETIMER0, true);
 }

 /***************************************************************************//**
  * @brief
  *   Callback function for after the the z direction of the accelerometer has
  *   been read
  *
  * @details
  *   This function requests the read result from icm20648.c and converts this
  *   value from a 16 bit unsigned integer to a short to an integer. This value
  *   is then used to determine whether the board is upside down or right side
  *   up. If upside down, the led 2 is changed to green and the phrase "upside
  *   down" is sent the bluetooth device.
  *
  ******************************************************************************/
 void scheduled_icm20648_read_cb(void){
    uint16_t zDirection_unsigned = icm20648_get_read_result();
    short zDirection_short = (short)zDirection_unsigned;
    int zDirection = (int)zDirection_short;

    if(firstZRead){
        if(zDirection < UPSIDEDOWN_VALUE){
            facingUpTrue = false;
            leds_enabled(RGB_LED_2, COLOR_GREEN, true);
            ble_write("Upside down\n");
         } else if(zDirection >= UPSIDEDOWN_VALUE) {
            facingUpTrue = true;
         }
        firstZRead = false;
    }

    if((zDirection < UPSIDEDOWN_VALUE) && (facingUpTrue)){
        facingUpTrue = false;
        leds_enabled(RGB_LED_2, COLOR_GREEN, true);
        ble_write("Upside down\n");
    } else if((zDirection >= UPSIDEDOWN_VALUE) && !(facingUpTrue)) {
        facingUpTrue = true;
        leds_enabled(RGB_LED_2, COLOR_GREEN, false);
        ble_write("Facing up\n");
    }
    ble_write("\n");
  }
