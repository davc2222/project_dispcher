/////////////////////
#include <FreeRTOS.h>
#include <FreeRTOSConfig.h>
#include <task.h>
#include <semphr.h>
#include <timers.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
// program
#include "ambulance.h"
#include "disptcher.h"
#include "main_dispacher_project.h"
#include "log.h"
#include "error.h"
#include "police.h"
#include "fire.h"
#include "corona.h"

// Global var
int call_id = 0;
// a global mutex
extern SemaphoreHandle_t xMutex_log;

// Timer handler
TimerHandle_t xDspthCallTimer;
void xDspthCallTimerCBF(TimerHandle_t xTimer);
// queues
QueueHandle_t xQueue_dispcher;
extern QueueHandle_t xQueue_police;
extern QueueHandle_t xQueue_ambulance;
extern QueueHandle_t xQueue_fire;
extern QueueHandle_t xQueue_corona;
// queue log
extern QueueHandle_t xQueue_log;

/**
 * @brief dispcher task . the core of the program
 *
 * This function has the highest priority
 * she read the call queue and assaign calls to the drsired  departements , but before
 *she check if to the desired departement has free car and assaign it directly
 * in case that the desired department has no free car the dispcher will check in all 
 * the depatements to find a free car.if no car free found the call would stay in the queue
 *when assign call to spefic car assigment she set a timer to random handle time.
 * all the events are recorded to log file.
 * 
 * @param[in] void *pvParameters
 * @return void
 */

void Task_dispcher(void *pvParameters)

{
    // call msg struct
    call_msg_t call_msg;
    // Pointer to store the chosen queue
    QueueHandle_t selected_queue = NULL;
    // to store call type
       uint8_t selected_call_type_num = 0;
    // cal type(ambulancs, fire.....)
    int call_type = 0;
    // available car number
    uint8_t available_car = 0;
    char selected_call_type_str[15] = {0};
      log_msg_call_t log_msg;

    for (;;)
    {

           
            // check if there is call waitting
            if (xQueuePeek(xQueue_dispcher, &call_msg, 100) == pdPASS)
            {
                call_type = call_msg.call_type;
             
                switch (call_type)
                {
                case police:

                    available_car = check_police_cars_busy(&busy_police_cars);
                    if (available_car != NO_CAR_AVAILABLE)
                    {

                        selected_queue = xQueue_police;
                        strcpy(selected_call_type_str, "Police");
                        break;
                    }
                    else if (check_ambulance_cars_busy(&busy_ambulance_cars) != NO_CAR_AVAILABLE)
                    {
                        available_car = check_ambulance_cars_busy(&busy_ambulance_cars);
                        selected_queue = xQueue_ambulance;
                        strcpy(selected_call_type_str, "Ambulance");
                        break;
                    }
                    else if (check_corona_cars_busy(&busy_corona_cars) != NO_CAR_AVAILABLE)
                    {
                        available_car = check_corona_cars_busy(&busy_corona_cars);
                        selected_queue = xQueue_corona;
                        strcpy(selected_call_type_str, "Corona");
                        break;
                    }

                    else if (check_fire_cars_busy(&busy_fire_cars) != NO_CAR_AVAILABLE)
                    {
                        available_car = check_fire_cars_busy(&busy_fire_cars);
                        selected_queue = xQueue_fire;
                        strcpy(selected_call_type_str, "Fire");
                        break;
                    }

                case ambulance:
            
                   available_car = check_ambulance_cars_busy(&busy_ambulance_cars);
                   if (available_car != NO_CAR_AVAILABLE)
                    {
                        selected_queue = xQueue_ambulance;
                        strcpy(selected_call_type_str, "Ambulance");
                        break;
                    }
                    else if (check_corona_cars_busy(&busy_corona_cars) != NO_CAR_AVAILABLE)
                    {
                        available_car = check_corona_cars_busy(&busy_corona_cars);
                        selected_queue = xQueue_corona;
                        strcpy(selected_call_type_str, "Corona");
                        break;
                    }

                    else if (check_police_cars_busy(&busy_police_cars) != NO_CAR_AVAILABLE)
                    {
                        available_car = check_police_cars_busy(&busy_police_cars);
                        selected_queue = xQueue_police;
                        strcpy(selected_call_type_str, "Police");
                        break;
                    }
                    else if (check_fire_cars_busy(&busy_fire_cars) != NO_CAR_AVAILABLE)
                    {
                        available_car = check_fire_cars_busy(&busy_fire_cars);
                        selected_queue = xQueue_fire;
                        strcpy(selected_call_type_str, "Fire");
                        break;
                    }

                case fire:

                    if (check_fire_cars_busy(&busy_fire_cars) != NO_CAR_AVAILABLE)
                    {
                        available_car = check_fire_cars_busy(&busy_fire_cars);
                        selected_queue = xQueue_fire;
                        strcpy(selected_call_type_str, "Fire");
                        break;
                    }
                    else if (check_ambulance_cars_busy(&busy_ambulance_cars) != NO_CAR_AVAILABLE)
                    {
                        available_car = check_ambulance_cars_busy(&busy_ambulance_cars);
                        selected_queue = xQueue_ambulance;
                        strcpy(selected_call_type_str, "Ambulance");
                        break;
                    }
                    else if (check_corona_cars_busy(&busy_corona_cars) != NO_CAR_AVAILABLE)
                    {
                        available_car = check_corona_cars_busy(&busy_corona_cars);
                        selected_queue = xQueue_corona;
                        strcpy(selected_call_type_str, "Corona");
                        break;
                    }

                    else if (check_police_cars_busy(&busy_police_cars) != NO_CAR_AVAILABLE)
                    {
                        available_car = check_police_cars_busy(&busy_police_cars);
                        selected_queue = xQueue_police;
                        strcpy(selected_call_type_str, "Police");
                        break;
                    }

                case corona:
                    
                    if (check_corona_cars_busy(&busy_corona_cars) != NO_CAR_AVAILABLE)
                    {
                      
                        available_car = check_corona_cars_busy(&busy_corona_cars);
                        selected_queue =  xQueue_corona;
                        strcpy(selected_call_type_str, "Corona");
                        break;
                    }

                    else if (check_ambulance_cars_busy(&busy_ambulance_cars) != NO_CAR_AVAILABLE)
                    {
                        available_car = check_ambulance_cars_busy(&busy_ambulance_cars);
                        selected_queue = xQueue_ambulance;
                        strcpy(selected_call_type_str, "Ambulance");
                        break;
                    }
                    else if (check_police_cars_busy(&busy_police_cars) != NO_CAR_AVAILABLE)
                    {
                        available_car = check_police_cars_busy(&busy_police_cars);
                        selected_queue = xQueue_police;
                        strcpy(selected_call_type_str, "Police");
                        break;
                    }

                    else if (check_fire_cars_busy(&busy_fire_cars) != NO_CAR_AVAILABLE)
                    {
                        available_car = check_fire_cars_busy(&busy_fire_cars);
                        selected_queue = xQueue_fire;
                        strcpy(selected_call_type_str, "Fire");
                        break;
                    }
                  
              
                }
            }
            else
            {

               //  printf("There is no messagess to dispatcher\n");debug only
            }
            
            if (selected_queue != NULL)
            {
               
                *call_msg.call_desc='\0';
                call_id = call_msg.call_id;
                snprintf(call_msg.call_desc, sizeof(call_msg.call_desc), " >> Assign call number %d for %s \n", call_msg.call_id, selected_call_type_str);
                RED_TXT_CLR;
                printf("Dispacher recievd call number %d\n", call_msg.call_id);
                RST_TXT_CLR;
                 if (xQueueSendToBack(selected_queue, &call_msg, TASKS_SNDQUE_DELAY) != pdPASS)
                {
                    my_assert(false, " disptcer : failed to send call to log  queue\n");
                }
               
                available_car = NO_CAR_AVAILABLE;
                // clear the current queue
                selected_queue = NULL;
                // print dispacher messages
                RED_TXT_CLR;
                memset(& log_msg, 0, sizeof( log_msg));
                printf("%s", call_msg.call_desc);
                get_time(log_msg.log_time_stamp);
  
                   if (xSemaphoreTake(xMutex_log, TASKS_SMFR_DELAY) == pdTRUE)
                    { 
                    if (xQueueSendToBack(xQueue_log, &log_msg, TASKS_SNDQUE_DELAY) != pdPASS)
                    {
                        my_assert(false, "failed to send call to log queue\n");

                    }
                     // Release the mutex
                   xSemaphoreGive(xMutex_log);
                    }
 

                RST_TXT_CLR;
             //   Remove the call from dispatcher queue , only if successfully assigned
         
                if (xQueueReceive(xQueue_dispcher, &call_msg, 200) != pdPASS)
                {
                    printf("Failed to remove last call from dispatcher queue\n");
                }
            
            }
            else
            {
                  //printf(" queue is null\n");//debug only
                  
            }
      
    
       
         //  delay
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

/**
 * @brief init the disptcher department.
 *
 * This function create  task , queue  
 * @param[in] void
 * @return void
 */

void init_dispacher_center(void)
{

    // Create  task
    if (xTaskCreate(Task_dispcher, "Task dispatcer", configMINIMAL_STACK_SIZE, NULL, TASK_PLC_PRI, NULL) != pdPASS)
    {
        my_assert(false, "failed to create disptcer queue");
    }

    // create dispcher queue

    xQueue_dispcher = xQueueCreate(DSPCR_QUEUE_LENGTH, DSPCR_QUEUE_SIZE);

    if (xQueue_dispcher == NULL)
    {
        my_assert(false, "failed to create dispatcer queue");
    }

    // create dispacher timer
    xDspthCallTimer = xTimerCreate("dispach_timer", pdMS_TO_TICKS(DSPTC_TIMER__START_PERIOD_MS), pdTRUE, (void *)0, xDspthCallTimerCBF);

    // create dispacher timer
    xTimerStart(xDspthCallTimer, 0);
}


/**
 * @brief generate calls to disptcher
 *
 * generate random call type
 * increment the call id var 
 * send the new call to dispcher
 * @param[in] void
 * @return void
 */

void handle_call_disptcher(void)
{
  
        // get random call type 
        int call_type_num = getRandomNumber(0, (int)(CALL_TYPE_NUM - 1));
        call_msg_t call_msg;
        call_msg.call_id = ++call_id;
        call_msg.call_type = call_type_num;
        snprintf(call_msg.call_desc, sizeof(call_msg.call_desc), " >> Send  call number %d to disptcher queue\n", call_id);
        RED_TXT_CLR;
       printf("call number %d been recived\n", call_msg.call_id);
       printf("call type %s been recived\n", get_call_type_str(call_type_num));
        RST_TXT_CLR;
        if (xQueueSendToBack(xQueue_dispcher, &call_msg, TASKS_SNDQUE_DELAY) != pdPASS)
        {
            my_assert(false, "failed to send call to dispther queue\n");
        }
 
}

/**
 * @brief dispcher timer call back fubctiom
 * This function genrate a random time 
 * and call the handle call dispcher function
 * @param[in] xTimer object
 * @return void
 */

void xDspthCallTimerCBF(TimerHandle_t xTimer)
{
    int period = getRandomNumber(MIN_DSPCR_CALL_HNDL_TIME, MAX_DSPCR_CALL_HNDL_TIME) * 1000;

    // change randomly timer period
    xTimerChangePeriod(xTimer, pdMS_TO_TICKS(period), 100);

    // call dispather to generate new call
    handle_call_disptcher();
}

/**
 * @brief return call type string by num.
 *
 * 
 * @param[in] call type num
 * @return string pointer
 */

const char *get_call_type_str(call_types_t callType)
{
    switch (callType)
    {
    case police:
        return "Police";
    case ambulance:
        return "Ambulance";
    case fire:
        return "Fire";
    case corona:
        return "Corona";
    default:
        return "Unknown";
    }
}
