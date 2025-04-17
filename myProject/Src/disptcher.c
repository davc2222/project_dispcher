
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
extern SemaphoreHandle_t xMutex;

// Timer handlers
// void xDspthCallTimerCBF(TimerHandle_t xTimer);
TimerHandle_t xDspthCallTimer;
void xDspthCallTimerCBF(TimerHandle_t xTimer);
// queues
QueueHandle_t xQueue_dispcher;
extern QueueHandle_t xQueue_police;
extern QueueHandle_t xQueue_ambulance;
extern QueueHandle_t xQueue_fire;
extern QueueHandle_t xQueue_corona;

// Task dispther
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
    char selected_call_type_str[15] = "";

    for (;;)
    {

        if (xSemaphoreTake(xMutex, TASKS_SMFR_DELAY) == pdTRUE)

        {
           printf(" dis running\n");     // debug only
            // check if there is call waitting
            if (xQueuePeek(xQueue_dispcher, &call_msg, 100) == pdPASS)
            {
                call_type = call_msg.call_type;
                // call_type = 3;
                // selected_queue = xQueue_police;

                printf("dspch call id %d+ call type %d\n " , call_msg.call_id  , call_msg.call_type);
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

                    //  printf("call type from task msg call type   %d\n", call_msg.call_type); // debug only
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
                        printf("corronaaaaaaaaaa\n");
                        available_car = check_corona_cars_busy(&busy_corona_cars);
                        selected_queue =  xQueue_corona;

                        printf("corona car = %d\n",available_car);
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
                    //  break;
                default:

                    printf("from case default %d \n", call_msg.call_type);
                }
            }
            else
            {

                 printf("There is no messagess to dispatcher\n");
            }

            if (selected_queue != NULL)
            {

                call_id = call_msg.call_id;
                snprintf(call_msg.call_desc, sizeof(call_msg.call_desc), " >> Assign call number %d for %s \n", call_msg.call_id, selected_call_type_str);
                RED_TXT_CLR;
                printf("Dispacher recievd call number %d\n", call_msg.call_id);
                RST_TXT_CLR;

                if (xQueueSendToBack(selected_queue, &call_msg, TASKS_SNDQUE_DELAY) != pdPASS)
                {
                    my_assert(false, "failed to send call to log  queue\n");
                }

                available_car = NO_CAR_AVAILABLE;
                // clear the current queue
                selected_queue = NULL;
                // print dispacher messages
                RED_TXT_CLR;
                printf("%s", call_msg.call_desc);
                write_call_time_to_log(LOG_FILE_NAME);
                write_call_details_to_log(LOG_FILE_NAME, call_msg.call_desc);
                RST_TXT_CLR;
                // Remove the call from dispatcher queue only if successfully assigned
                if (xQueueReceive(xQueue_dispcher, &call_msg, 5) != pdPASS)
                {
                    printf("Failed to remove last call from dispatcher queue\n");
                }
            }
            else
            {
                  printf(" queue is null\n");
                  printf(" call type\n",  call_type);
                  printf("ava car \n", available_car );
            }
        }
        else
        {

            // printf("failed to get mutex for Disptcher\n"); // debug only
        }

        // Release the mutex
        xSemaphoreGive(xMutex);
        //  printf("dispacher relese mutex\n");
        //  delay
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

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

void handle_call_disptcher(void)
{
    if (xSemaphoreTake(xMutex, 100) == pdTRUE)
    {

        int call_type_num = getRandomNumber(0, (int)(CALL_TYPE_NUM - 1));

        call_msg_t call_msg;
        //  printf("call type from gen  in function %d\n", call_type_num); // debug only
        call_msg.call_id = ++call_id;
        call_msg.call_type = call_type_num;
        snprintf(call_msg.call_desc, sizeof(call_msg.call_desc), " >> Send  call number %d to disptcher queue\n", call_id);
        RED_TXT_CLR;
      //  printf("call number %d been recived\n", call_msg.call_id);
     //   printf("call type %s been recived\n", get_call_type_str(call_type_num));
        RST_TXT_CLR;
        if (xQueueSendToBack(xQueue_dispcher, &call_msg, TASKS_SNDQUE_DELAY) != pdPASS)
        {
            my_assert(false, "failed to send call to dispther queue\n");
        }
    }
    // Release the mutex
    xSemaphoreGive(xMutex);
}

void xDspthCallTimerCBF(TimerHandle_t xTimer)
{
    int period = getRandomNumber(MIN_DSPCR_CALL_HNDL_TIME, MAX_DSPCR_CALL_HNDL_TIME) * 1000;

    // change randomly timer period

    xTimerChangePeriod(xTimer, pdMS_TO_TICKS(period), 100);

    // call dispather to generate new call
    handle_call_disptcher();
}

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
