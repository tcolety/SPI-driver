/**
 * @file cmu.c
 * @author Taylor Colety
 * @date 9/12/2021
 * @brief Function to enable low frequncy clock tree
 *
 */
//***********************************************************************************
// Include files
//***********************************************************************************
#include "cmu.h"

//***********************************************************************************
// defined files
//***********************************************************************************


//***********************************************************************************
// Private variables
//***********************************************************************************


//***********************************************************************************
// Private functions
//***********************************************************************************


//***********************************************************************************
// Global functions
//***********************************************************************************

/***************************************************************************//**
 * @brief
 *          Function to enable/disable and route clocks
 *
 * @details
 *          This function turns off LFRCO and disables the LFXO. It also routes
 *          the low frequency clock to the LF clock tree. Finally, it
 *          enables the low frequency clock tree.
 *
 * @note
 *          The enumeration for each of the clocks/oscillators was found in
 *          the HAL documentation.
 *
 ******************************************************************************/

void cmu_open(void){

    CMU_ClockEnable(cmuClock_HFPER, true);

    // By default, LFRCO is enabled, disable the LFRCO oscillator
    // Disable the LFRCO oscillator
    // What is the enumeration required for LFRCO?
    // It can be found in the online HAL documentation
    CMU_OscillatorEnable(cmuOsc_LFRCO , false, false);

    // Disable the LFXO oscillator
    // What is the enumeration required for LFXO?
    // It can be found in the online HAL documentation
    CMU_OscillatorEnable(cmuOsc_LFXO , true, true);

    // No requirement to enable the ULFRCO oscillator.  It is always enabled in EM0-4H1

    // Route LF clock to the LF clock tree
    // What is the enumeration required to placed the ULFRCO onto the proper clock branch?
    // It can be found in the online HAL documentation
    CMU_ClockSelectSet(cmuClock_LFA , cmuSelect_ULFRCO);    // routing ULFRCO to proper Low Freq clock tree


    CMU_ClockSelectSet(cmuClock_LFB, cmuSelect_LFXO);

    // What is the proper enumeration to enable the clock tree onto the LE clock branches?
    // It can be found in the Assignment 2 documentation
    CMU_ClockEnable(cmuClock_CORELE, true);



}

