/**
 * @file gpio.c
 * @author Taylor Colety
 * @date 9/12/2021
 * @brief Sets up the GPIO pins and LEDs
 *
 */

//***********************************************************************************
// Include files
//***********************************************************************************
#include "gpio.h"

//***********************************************************************************
// defined files
//***********************************************************************************

//***********************************************************************************
// global variables
//***********************************************************************************


//***********************************************************************************
// function prototypes
//***********************************************************************************


//***********************************************************************************
// functions
//***********************************************************************************

/***************************************************************************//**
 * @brief
 *        This function sets up the GPIO pins.
 *
 * @details
 *        It enables the gpio clock and sets the pins for the LEDs.
 *
 * @note
 *        This function enables both the red and green LEDs
 *
 ******************************************************************************/

void gpio_open(void){

  CMU_ClockEnable(cmuClock_GPIO, true);

	// Configure LED pins
	GPIO_DriveStrengthSet(LED_RED_PORT, LED_RED_DRIVE_STRENGTH);
	GPIO_PinModeSet(LED_RED_PORT, LED_RED_PIN, LED_RED_GPIOMODE, LED_RED_DEFAULT);

	GPIO_DriveStrengthSet(LED_GREEN_PORT, LED_GREEN_DRIVE_STRENGTH);
	GPIO_PinModeSet(LED_GREEN_PORT, LED_GREEN_PIN, LED_GREEN_GPIOMODE, LED_GREEN_DEFAULT);

	//Set RGB LED gpio pin configurations
	 GPIO_PinModeSet(RGB_ENABLE_PORT, RGB_ENABLE_PIN, gpioModePushPull, RGB_DEFAULT_OFF);
	 GPIO_PinModeSet(RGB0_PORT, RGB0_PIN, gpioModePushPull, RGB_DEFAULT_OFF);
	 GPIO_PinModeSet(RGB1_PORT, RGB1_PIN, gpioModePushPull, RGB_DEFAULT_OFF);
	 GPIO_PinModeSet(RGB2_PORT, RGB2_PIN, gpioModePushPull, RGB_DEFAULT_OFF);
	 GPIO_PinModeSet(RGB3_PORT, RGB3_PIN, gpioModePushPull, RGB_DEFAULT_OFF);
	 GPIO_PinModeSet(RGB_RED_PORT, RGB_RED_PIN, gpioModePushPull, COLOR_DEFAULT_OFF);
	 GPIO_PinModeSet(RGB_GREEN_PORT, RGB_GREEN_PIN, gpioModePushPull, COLOR_DEFAULT_OFF);
	 GPIO_PinModeSet(RGB_BLUE_PORT, RGB_BLUE_PIN, gpioModePushPull, COLOR_DEFAULT_OFF);

	 //Configure Ambient light sensor pins
	 GPIO_DriveStrengthSet(SI1133_SENSOR_EN_PORT, gpioDriveStrengthWeakAlternateWeak);
	 GPIO_PinModeSet(SI1133_SENSOR_EN_PORT, SI1133_SENSOR_EN_PIN, gpioModePushPull, true);
	 GPIO_PinModeSet(SI1133_SCL_PORT, SI1133_SCL_PIN, gpioModeWiredAnd, true);
	 GPIO_PinModeSet(SI1133_SDA_PORT, SI1133_SDA_PIN, gpioModeWiredAnd, true);

	 //Configure the pins for the bluetooth device
	 GPIO_DriveStrengthSet(LEUART_TX_PORT,gpioDriveStrengthStrongAlternateWeak);
	 GPIO_PinModeSet(LEUART_TX_PORT, LEUART_TX_PIN, gpioModePushPull, true);
	 GPIO_PinModeSet(LEUART_RX_PORT, LEUART_RX_PIN, gpioModeInput, true);

	 //configure pins for icm20648
	 GPIO_DriveStrengthSet(USART_TX_PORT,gpioDriveStrengthStrongAlternateWeak);
	 GPIO_DriveStrengthSet(USART_ICM_EN_PORT, gpioDriveStrengthWeakAlternateWeak);
	 GPIO_PinModeSet(USART_ICM_EN_PORT, USART_ICM_EN_PIN, gpioModePushPull, true);
	 GPIO_PinModeSet(USART_TX_PORT, USART_TX_PIN, gpioModePushPull, true);
	 GPIO_PinModeSet(USART_RX_PORT, USART_RX_PIN, gpioModeInput, true);
	 GPIO_PinModeSet(USART_CS_PORT, USART_CS_PIN, gpioModePushPull, true);
	 GPIO_PinModeSet(USART_SCLK_PORT, USART_SCLK_PIN, gpioModePushPull, true);
}
