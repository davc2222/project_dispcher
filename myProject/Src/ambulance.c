#include <FreeRTOS.h>
#include <FreeRTOSConfig.h>
#include <task.h>
#include <semphr.h>
#include <timers.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <queue.h>
// program
#include "ambulance.h"
#include "error.h"
#include "disptcher.h"
#include "main_dispacher_project.h"
#include "log.h"

// create timers for cars
TimerHandle_t xAmbulanceTimers[AMBULANCE_CAR_NUM];
// call back function fot the timers
void vAmbulanceTimerCallBackFunction(TimerHandle_t xTimer);
// ambulance queue handler
QueueHandle_t xQueue_ambulance;
busy_ambulance_cars_t busy_ambulance_cars;
// global mutex
extern SemaphoreHandle_t xMutex;
// queue
extern QueueHandle_t xQueue_log;

/**
 * @brief init the ambulance department.
 *
 * This function create  task , queue  and init timers
 * @param[in] void
 * @return void
 */

void init_ambulance_department(void)
{
    // Create ambulancd task
    if (xTaskCreate(Task_ambulance, "Task Ambulance", configMINIMAL_STACK_SIZE, NULL, TASK_PLC_PRI, NULL) != pdPASS)
    {
        my_assert(false, "failed to create Task ambulance");
    }

    // create  police queue
    xQueue_ambulance = xQueueCreate(AMBULANCE_QUEUE_LENGTH, AMBULANCE_QUEUE_SIZE);

    if (xQueue_ambulance == NULL)
    {

        my_assert(false, "failed to create ambulance queue");
    }

   init_ambulace_timers();
}

/**
 * @brief ambulance task . handle all ambulance cars
 *
 * This function check if there is  ambulance car ava then check for
 * calls and if there is assign it to the free car and set it busy
 * init timer for  random handle time
 * @param[in] void
 * @return void
 */
void Task_ambulance(void *pvParameters)
{
    log_msg_call_t log_msg;
    call_msg_t msg_ambulance;
    static char call_msg_desc[100] = {0};
    for (;;)
    {

        if (xSemaphoreTake(xMutex, TASKS_SMFR_DELAY) == pdTRUE)
        {
         //   printf("Task ambulance  is using the shared resource\n"); // debug only
            uint8_t available_car = check_ambulance_cars_busy(&busy_ambulance_cars);

            switch (available_car)
            {

            case NO_CAR_AVAILABLE:

                //   printf("all ambulance cars occupied\n");
                break;

            case CAR_1:
            case CAR_2:
            case CAR_3:
                if (xQueueReceive(xQueue_ambulance, &msg_ambulance, TASKS_RCVQUE_DELAY) == pdPASS)
                {
                    const char *car_name = get_ambulance_car_name(available_car);
                    set_reset_ambulance_car_busy(&busy_ambulance_cars, available_car, true);
                    YLW_TXT_CLR;
                    printf("%s  handle call- %d\n", car_name, msg_ambulance.call_id);
                    snprintf(call_msg_desc, sizeof(call_msg_desc), " >> %s  handle call  %d\n", car_name, msg_ambulance.call_id);
                    snprintf(log_msg.log_call_desc, sizeof(log_msg.log_call_desc), " >> %s  handle call  %d\n", car_name, msg_ambulance.call_id);
                    get_time(log_msg.log_time_stamp);
                    if (xQueueSendToBack(xQueue_log, &log_msg, TASKS_SNDQUE_DELAY) != pdPASS)
                    {
                        my_assert(false, "failed to send call to log queue\n");
                    }
                    int handl_time = getRandomNumber(MIN_AMBULANCE_CALL_HNDL_TIME, MAX_AMBULANCE_CALL_HNDL_TIME) * 1000; // time between 5 - 10sec
                    xTimerChangePeriod(xAmbulanceTimers[available_car - 1], pdMS_TO_TICKS(handl_time), 0);               // set new time for call
                    xTimerStart(xAmbulanceTimers[available_car - 1], 0);
                    RST_TXT_CLR;
                    break;
                }
                else
                {
                    // printf("There are no calls for ambulance\n"); // debug only
                    break;
                }
            }

            // Release the mutex
            xSemaphoreGive(xMutex);
        }
        else
        {

            RST_TXT_CLR;
            // printf("failed to get mutex for Task_ambulance\n"); // debug only
        }

        // delay
        vTaskDelay(pdMS_TO_TICKS(TASK_AMBLNCE_DELAY));
    }
}

/**
 * @brief check if there is ambulance free car
 *
 * @param[in] pointer for the struct
 * @return  number that represent the free car
 * if non of the cars are free return "NO_CAR_AVAILABLE"
 */

uint8_t check_ambulance_cars_busy(busy_ambulance_cars_t *cars)
{

    if (cars->ambulance_1 == AVAILABLE) return 1;  
    if (cars->ambulance_2 == AVAILABLE) return 2;  
    if (cars->ambulance_3 == AVAILABLE) return 3;
       
     /* no free cars*/ 
       return NO_CAR_AVAILABLE;
}


/**
 * @brief set or reset car status
 *
 * @param[in] pointer for the busy  struct, car number , state
 * @return  void
  */
void set_reset_ambulance_car_busy(busy_ambulance_cars_t *cars, uint8_t car_num, bool state)
{
    switch (car_num)
    {
    case 1:
        cars->ambulance_1 = state; break; 
    case 2:
        cars->ambulance_2 = state; break;
    case 3:
        cars->ambulance_3 = state; break;    
    }
}

/**
 * @brief return car name string
 *
 * @param[in] car number
 * @return  pointer to car name string
  */
// const char *get_ambulance_cars_name(uint8_t car_num)
// {
//     static const char *ambulance_names[] = {
//         "Ambulance 1",
//         "Ambulance 2",
//         "Ambulance 3"
//     };

//     if (car_num < AMBULANCE_CAR_NUM)
//         return ambulance_names[car_num - 1];
//     else
//         return "Unknown Ambulance";
// }


const char *get_ambulance_car_name(uint8_t car_num)
{
    switch (car_num)
    {
    case 1:
        return "Ambulance 1";
    case 2:
        return "Ambulance  2";
    case 3:
        return "Ambulance 3";
   
    }
}
// init ambulace timers

void init_ambulace_timers(void)
{

     // Static array to store the timer names 
     static char timerName[AMBULANCE_CAR_NUM][11];  // Array to hold timer names ("corona1", "corona2", ..., up to "corona10")

     for (int i = 0; i < AMBULANCE_CAR_NUM; i++)
     {
         // Generate a unique timer name for each timer
         snprintf(timerName[i], sizeof(timerName[i]), "ambulance%d", i + 1);
 
         // Create the timer with the generated name
         xAmbulanceTimers[i] = xTimerCreate(timerName[i],
                                         pdMS_TO_TICKS(100),  // Timer period in milliseconds
                                         pdFALSE,             // Auto-reload set to false
                                         (void *)timerName[i],  // Store the timer name as Timer ID
                                         vAmbulanceTimerCallBackFunction); // Callback function when timer expires
 
         if ( xAmbulanceTimers[i] == NULL)
         {
             printf("Error: Failed to create corona timer %d\n", i);
         }
     }
}

void vAmbulanceTimerCallBackFunction(TimerHandle_t xTimer)
{
    log_msg_call_t log_msg;
    // get the timer name(ID)
    const char *timerName = (const char *)pvTimerGetTimerID(xTimer);
    // get the car number
    uint8_t carNum = getTimerAmbulanceCarNum(timerName);
    YLW_TXT_CLR;
    printf("Ambulance Car %d has finished call handelling\n", carNum);
    snprintf(log_msg.log_call_desc, sizeof(log_msg.log_call_desc), " >> Ambulance Car %d has finished call handelling\n", carNum);
    get_time(log_msg.log_time_stamp);
    if (xQueueSendToBack(xQueue_log, &log_msg, TASKS_SNDQUE_DELAY) != pdPASS)
    {
        my_assert(false, "failed to send call to log queue\n");
    }
    set_reset_ambulance_car_busy(&busy_ambulance_cars, carNum, false);
    // Stop timer at the end (applies to all cases)
    xTimerStop(xTimer, 0);
    RST_TXT_CLR;
}

uint8_t getTimerAmbulanceCarNum(const char *timerName)
{

    if (strcmp(timerName, "ambulance1") == 0)
        return AMBULANCE_1_TIMER;
    if (strcmp(timerName, "ambulance2") == 0)
        return AMBULANCE_2_TIMER;
    if (strcmp(timerName, "ambulance3") == 0)
        return AMBULANCE_3_TIMER;
}