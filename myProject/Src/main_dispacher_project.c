#include <FreeRTOS.h>
#include <FreeRTOSConfig.h>
#include <task.h>
#include <semphr.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
// program
#include "disptcher.h"
#include "log.h"
#include "error.h"
#include "police.h"
#include "fire.h"
#include "corona.h"
#include "ambulance.h"
#include "main_dispacher_project.h"
#include "log.h"

// Declare a global mutex
SemaphoreHandle_t xMutex;

int main_dispacher_project(void)
{

  init_program();

  // Start FreeRTOS Scheduler
  vTaskStartScheduler();

  // If all went well, we should never reach here

  for (;;)
    ;
}

/**
 * @brief init the program.
 *
 * This function create mutex ,and init the disptcer
 * and all depatments + log init _ans start random
 * @param[in] void
 * @return void
 */

void init_program(void)
{

  // Create the mutex
  xMutex = xSemaphoreCreateMutex();

  if (xMutex == NULL)
  {
    my_assert(false, "failed to create mutex");
  }
  //   dispatcer
  init_dispacher_center();

  // police
  init_police_department();

  // ambulance
  init_ambulance_department();

  // fire
  init_fire_department();

  // corona
  init_corona_department();

  // init random function
  srand(time(NULL));

  // init new log  file
   init_log_handler();
}

/**
 * @brief return random number
 * between min and max

 * @param[in] void
 * @return int between min and max
 */

int getRandomNumber(int min, int max)
{
  return (rand() % (max - min + 1)) + min;
}
