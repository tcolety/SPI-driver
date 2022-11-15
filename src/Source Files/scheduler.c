/**
 * @file    scheduler.c
 * @author  Taylor Colety
 * @date    9/16/2021
 * @brief   Functions that control scheduler
 *
 */

//***********************************************************************************
// Include files
//***********************************************************************************
#include "scheduler.h"

//***********************************************************************************
// defined files
//***********************************************************************************


//***********************************************************************************
// Private variables
//***********************************************************************************
static unsigned int event_scheduled;


//***********************************************************************************
// Private functions
//***********************************************************************************

//***********************************************************************************
// Global functions
//***********************************************************************************


/***************************************************************************//**
 * @brief
 *          Function opens the scheduler
 * @details
 *          scheduler_open() initializes static variable event_scheduled to 0.
 * @note
 *          This function must be called before scheduler can be used.
 ******************************************************************************/
void scheduler_open(void){
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();
  event_scheduled = 0;
  CORE_EXIT_CRITICAL();
}

/***************************************************************************//**
 * @brief
 *          Function adds an event to the schedule
 * @details
 *          add_scheduled_event() ORs the input event with the static variable
 *          event_scheduled in order to add the input event to the scheduler.
 * @note
 *          Scheduler must be open before this function is called.
 * @param[in] event
 *          event is unsigned integer representation of the event to be added to
 *          the schedule.
 ******************************************************************************/
void add_scheduled_event(uint32_t event){
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();
  event_scheduled |= event;
  CORE_EXIT_CRITICAL();
}

/***************************************************************************//**
 * @brief
 *          Function removes an event from the schedule
 * @details
 *          removed_scheduled_event() ANDs the inverse to the input event with
 *          event_scheduled in order to remove the input event from the scheduler.
 * @note
 *          Scheduler must be open before this function is called.
 * @param[in] event
 *          event is unsigned integer representation of the event to be removed
 *          from the schedule.
 ******************************************************************************/
void remove_scheduled_event(uint32_t event){
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();
  event_scheduled &= ~event;
  CORE_EXIT_CRITICAL();
}

/***************************************************************************//**
 * @brief
 *          Function returns integer storing scheduled events
 * @details
 *          get_scheduled_events() returns the unsigned int event_scheduled.
 *          Each bit in event_scheduled represents aa different event that can
 *          be added to the scheduler making 32 possible scheduled events.
 * @note
 *          Scheduler must be opened with scheduler_open() before this function
 *          can be called.
 ******************************************************************************/
uint32_t get_scheduled_events(void){
  //CORE_DECLARE_IRQ_STATE;
  //CORE_ENTER_CRITICAL();
  return event_scheduled;
  //CORE_EXIT_CRITICAL();
}
