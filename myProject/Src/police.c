
#include <FreeRTOS.h>
#include <FreeRTOSConfig.h>
#include <task.h>
#include <timers.h>
#include <time.h>
#include <semphr.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <queue.h>
#include "police.h"
#include "error.h"
#include "disptcher.h"
#include "main_dispacher_project.h"
#include "log.h"

TimerHandle_t xPoliceTimers[POLICE_CAR_NUM];
void vPoliceTimerCallBackFunction(TimerHandle_t xTimer);
QueueHandle_t xQueue_police;
busy_police_cars_t busy_police_cars;
// Declare a global mutex
extern SemaphoreHandle_t xMutex;
extern QueueHandle_t xQueue_log;

void init_police_department(void)
{

    // Create polic task
    if (xTaskCreate(Task_police, "Task police", configMINIMAL_STACK_SIZE, NULL, TASK_PLC_PRI, NULL) != pdPASS)
    {
        my_assert(false, "failed to create Task police");
    }

    // create  police queue

    xQueue_police = xQueueCreate(POLICE_QUEUE_LENGTH, POLICE_QUEUE_SIZE);

    if (xQueue_police == NULL)
    {

        my_assert(false, "failed to create police queue");
    }
    init_police_timers();
}

// Task police
void Task_police(void *pvParameters)
{
    log_msg_call_t log_msg;
    call_msg_t msg_police;
    char call_msg_desc[100] = {0};
    for (;;)

    {
        if (xSemaphoreTake(xMutex, TASKS_SMFR_DELAY) == pdTRUE)
        {
           //  printf("Task police  is using the shared resource\n"); // debug only
            uint8_t available_car = check_police_cars_busy(&busy_police_cars);

            switch (available_car)
            {

            case NO_CAR_AVAILABLE:

                //   printf("all police cars occupied\n");
                break;

            case CAR_1:
            case CAR_2:
            case CAR_3:
                if (xQueueReceive(xQueue_police, &msg_police, TASKS_RCVQUE_DELAY) == pdPASS)
                {
                    const char *car_name = get_police_car_name(available_car);
                    set_reset_police_car_busy(&busy_police_cars, available_car, true);
                    BLUE_TXT_CLR;
                    printf("%s  handle call  %d\n", car_name, msg_police.call_id);
                    snprintf(call_msg_desc, sizeof(call_msg_desc), " >> %s  handle call  %d\n", car_name, msg_police.call_id);
                    snprintf(log_msg.log_call_desc, sizeof(log_msg.log_call_desc), " >> %s  handle call  %d\n", car_name, msg_police.call_id);
                    get_time(log_msg.log_time_stamp);
                    if (xQueueSendToBack(xQueue_log, &log_msg, TASKS_SNDQUE_DELAY) != pdPASS)
                    {
                        my_assert(false, "failed to send call to log queue\n");
                     }
                     int handl_time = getRandomNumber(MIN_POLICE_CALL_HNDL_TIME, MAX_POLICE_CALL_HNDL_TIME) * 1000; // time between 5 - 10sec
                    xTimerChangePeriod(xPoliceTimers[available_car - 1], pdMS_TO_TICKS(handl_time), 0);                  // set new time for call
                    xTimerStart(xPoliceTimers[available_car - 1], 0);
                    RST_TXT_CLR;
                    break;
                }
                else
                {
                    // printf("There are no calls for police\n"); // debug only
                    break;
                }
            }

            // Release the mutex
               xSemaphoreGive(xMutex);
        }
        else
        {
            RST_TXT_CLR;
         //   printf("failed to get mutex for Task_police\n"); // debug only
        }

         
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

// Police

uint8_t check_police_cars_busy(busy_police_cars_t *cars)
{

    if (cars->police_1 == AVAILABLE)
        return 1;

    if (cars->police_2 == AVAILABLE)
        return 2;

    if (cars->police_3 == AVAILABLE)
        return 3;

    return NO_CAR_AVAILABLE;
}

void set_reset_police_car_busy(busy_police_cars_t *cars, uint8_t car_num, bool state)
{
    switch (car_num)
    {
    case 1:
        cars->police_1 = state;
        break;
    case 2:
        cars->police_2 = state;
        break;
    case 3:
        cars->police_3 = state;
        break;
    }
}

const char *get_police_car_name(uint8_t car_num)
{
    switch (car_num)
    {
    case 1:
        return "Police 1";
    case 2:
        return "Police 2";
    case 3:
        return "Police 3";
    }
}

// init police timers

void init_police_timers(void)
{

    for (int i = 0; i < POLICE_CAR_NUM; i++)
    {
        char *timerName = (char *)pvPortMalloc(10); // malloc
        if (timerName == NULL)
        {
            printf("Error: Failed to allocate memory for timer name\n");
            return;
        }
        // Define unique names for each timer
        snprintf(timerName, 10, "police%d", i + 1); // Generate unique name
        xPoliceTimers[i] = xTimerCreate(timerName,
                                        pdMS_TO_TICKS(100),
                                        pdFALSE,
                                        (void *)timerName,             // Store string name as Timer ID
                                        vPoliceTimerCallBackFunction); // Common callback function

        if (xPoliceTimers[i] == NULL)
        {

            printf("Error: Failed to create police  timer %d\n", i);
            vPortFree(timerName); // Free allocated memory if timer creation fails
        }
    }
}

void vPoliceTimerCallBackFunction(TimerHandle_t xTimer)
{
    log_msg_call_t log_msg;
    // get the timer name(ID)
    const char *timerName = (const char *)pvTimerGetTimerID(xTimer);
    //get the car number
    uint8_t carNum = getTimerPoliceCarNum(timerName);
    BLUE_TXT_CLR;
    printf("Police Car %d has finished call handelling\n", carNum);
    snprintf(log_msg.log_call_desc, sizeof(log_msg.log_call_desc), " >> Police Car %d has finished call handelling\n", carNum);
    get_time(log_msg.log_time_stamp);
    RST_TXT_CLR;
    if (xQueueSendToBack(xQueue_log, &log_msg, TASKS_SNDQUE_DELAY) != pdPASS)
    {
        my_assert(false, "failed to send call to log queue\n");
    }
    set_reset_police_car_busy(&busy_police_cars , carNum, false);
    // Stop timer at the end (applies to all cases)
    xTimerStop(xTimer, 0);
}

uint8_t getTimerPoliceCarNum(const char *timerName)
{
    if (strcmp(timerName, "police1") == 0)
        return POLICE_1_TIMER;
    if (strcmp(timerName, "police2") == 0)
        return POLICE_2_TIMER;
    if (strcmp(timerName, "police3") == 0)
        return POLICE_3_TIMER;
}
