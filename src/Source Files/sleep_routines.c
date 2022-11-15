/**
 * @file sleep_routines.c
 * @author Taylor Colety
 * @date 9/16/2021
 * @brief function that control sleep modes
 *
 */

//***********************************************************************************
// License
//***********************************************************************************
/***************************************************************************
 * @file sleep.c
 * ****************************************************************************
 * @section License
 * <b>(C) Copyright 2015 Silicon Labs, http://www.silabs.com</b>
 * *****************************************************************************
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 * DISCLAIMER OF WARRANTY/LIMITATION OF REMEDIES: Silicon Labs has no
 * obligation to support this Software. Silicon Labs is providing the
 * Software "AS IS", with no express or implied warranties of any kind,
 * including, but not limited to, any implied warranties of merchantability
 * or fitness for any particular purpose or warranties against infringement
 * of any proprietary rights of a third party.
 *
 * Silicon Labs will not be liable for any consequential, incidental, or
 * special damages, or any other relief, or for any claim by any third party,
 * arising from your use of this Software.
 *
 * ***************************************************************************/

//***********************************************************************************
// Include files
//***********************************************************************************
#include "sleep_routines.h"

//***********************************************************************************
// Private variables
//***********************************************************************************
static int lowest_energy_mode[MAX_ENERGY_MODES];

//***********************************************************************************
// Global functions
//***********************************************************************************

/***************************************************************************//**
 * @brief
 *            sleep_open() set each element in lowest_energy_mode array to 0
 * @details
 *            lowest_energy_mode array stores the number of times each energy
 *            mode has been blocked. This function initializes all values to 0,
 *            which must be done before sleep modes can be entered, blocked,
 *            unblocked, or read.
 * @note
 *            This function takes in no inputs and returns no outputs.
 *
 ******************************************************************************/
void sleep_open(void){
  for(int i=0; i<MAX_ENERGY_MODES; i++){
      lowest_energy_mode[i] = 0;
  }
}

/***************************************************************************//**
 * @brief
 *          Function blocks sleep modes from being entered
 * @details
 *          sleep_block_mode() blocks the input energy mode from being entered
 *          by incrementing the element corresponding to that energy mode in the
 *          lowest_energy_mode array. An assertion test checks to make sure
 *          specific the energy mode has not been blocked more that 5 times.
 * @note
 *          This function takes in the energy mode to be blocked and returns no
 *          outputs.
 * @param[in] EM
 *          Energy mode to be blocked. Numbers 0-4.
 ******************************************************************************/
void sleep_block_mode(uint32_t EM){
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();

  lowest_energy_mode[EM]++;
  EFM_ASSERT(lowest_energy_mode[EM] < 5);

  CORE_EXIT_CRITICAL();
}

/***************************************************************************//**
 * @brief
 *          Function unblocks sleep modes enabling them to be entered
 * @details
 *          sleep_unblock_mode() decrements the element in lowest_energy_mode[]
 *          corresponding to the function's input energy mode. An assertion test
 *          checks to make sure specific the energy mode has not been unblocked
 *          more times than it has been block, checking that the array element
 *          is greater than or equal to 0.
 * @note
 *          This function takes in the energy mode to be blocked and returns no
 *          outputs.
 * @param[in] EM
 *          Energy mode to be unblocked. Numbers 0-4.
 ******************************************************************************/
void sleep_unblock_mode(uint32_t EM){
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();

  lowest_energy_mode[EM]--;
  EFM_ASSERT(lowest_energy_mode[EM] >= 0);

  CORE_EXIT_CRITICAL();
}

/***************************************************************************//**
 * @brief
 *          Functions enters into lowest possible sleep mode.
 * @details
 *          enter_sleep() includes a series of else if statements testing to find
 *          the first blocked energy mode. Once the first blocked energy mode is
 *          found (lowest_energy_mode[EM]>0), then the previous energy mode is
 *          entered. If energy modes 0-3 are not blocked, then EM3 is entered.
 *          Energy mode 4 is not used in this application.
 * @note
 *          This function takes in no inputs and returns no outputs.
 ******************************************************************************/
void enter_sleep(void){
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();

  if(lowest_energy_mode[EM0] > 0){}
  else if(lowest_energy_mode[EM1] > 0){}
  else if(lowest_energy_mode[EM2] > 0){
      EMU_EnterEM1();
  }
  else if(lowest_energy_mode[EM3] > 0){
      EMU_EnterEM2(true);
  }
  else {EMU_EnterEM3(true);}

  CORE_EXIT_CRITICAL();
}

/***************************************************************************//**
 * @brief
 *          Function returns the first energy mode that is blocked
 * @details
 *          current_block_energy_mode() searches through lowest_energy_mode array
 *          to find first element that is nonzero and returns that index. This
 *          gives the first energy mode that is blocked. If no energy mode is
 *          blocked, function returns lowest energy mode possible which is
 *          MAX_ENERGY_MODES-1.
 * @note
 *          This function has no input arguments and returns the current energy
 *          mode.
 ******************************************************************************/
uint32_t current_block_energy_mode(void){
  int i = 0;
    while(i < MAX_ENERGY_MODES){
        if(lowest_energy_mode[i]){
            return i;
        }
        i++;
    }
    return (MAX_ENERGY_MODES - 1);
}
