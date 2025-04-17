#include <FreeRTOS.h>
#include <FreeRTOSConfig.h>
#include <task.h>
#include <semphr.h>
#include <timers.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <queue.h>
#include "police.h"
#include "error.h"
#include "disptcher.h"
#include "fire.h"
#include "main_dispacher_project.h"
#include "log.h"

// fire queue handler
QueueHandle_t xQueue_fire;
TimerHandle_t xFireTimers[FIRE_CAR_NUM];
void vFireTimerCallBackFunction(TimerHandle_t xTimer);
busy_fire_cars_t busy_fire_cars;

// Declare a global mutex
extern SemaphoreHandle_t xMutex;
extern QueueHandle_t xQueue_log;


void init_fire_department(void)
{

    // Create fire task
    if (xTaskCreate(Task_fire, "Task fire", configMINIMAL_STACK_SIZE, NULL, TASK_PLC_PRI, NULL) != pdPASS)
    {
        my_assert(false, "failed to create Task fire");
    }

    // create  fire queue

    xQueue_fire = xQueueCreate(FIRE_QUEUE_LENGTH, FIRE_QUEUE_SIZE);

    if (xQueue_fire == NULL)
    {

        my_assert(false, "failed to create fire queue");
    }
    init_fire_timers();
}

void Task_fire(void *pvParameters)
{
    call_msg_t msg_fire;
    log_msg_call_t log_msg;
    char call_msg_desc[100] = {0};
    for (;;)
    {

        if (xSemaphoreTake(xMutex, TASKS_SMFR_DELAY) == pdTRUE)
        {
            //  printf("Task fire  is using the shared resource\n"); // debug only
            uint8_t available_car = check_fire_cars_busy(&busy_fire_cars);

            switch (available_car)
            {

            case NO_CAR_AVAILABLE:

                //   printf("all fire cars occupied\n");
                break;

            case CAR_1:
            case CAR_2:
                if (xQueueReceive(xQueue_fire, &msg_fire, TASKS_RCVQUE_DELAY) == pdPASS)
                {
                    const char *car_name = get_fire_car_name(available_car);
                    set_reset_fire_car_busy(&busy_fire_cars, available_car, true);
                    GRN_TXT_CLR;
                    printf("%s  handle call- %d\n", car_name, msg_fire.call_id);
                    snprintf(call_msg_desc, sizeof(call_msg_desc), " >> %s  handle call  %d\n", car_name, msg_fire.call_id);
                    snprintf(log_msg.log_call_desc, sizeof(log_msg.log_call_desc), " >> %s  handle call  %d\n", car_name, msg_fire.call_id);
                    get_time(log_msg.log_time_stamp);
                    if (xQueueSendToBack(xQueue_log, &log_msg, TASKS_SNDQUE_DELAY) != pdPASS)
                    {
                        my_assert(false, "failed to send call to log queue\n");
                     }
                    int handl_time = getRandomNumber(MIN_FIRE_CALL_HNDL_TIME,MAX_FIRE_CALL_HNDL_TIME) * 1000; // time between 5 - 10sec
                    xTimerChangePeriod(xFireTimers[available_car - 1], pdMS_TO_TICKS(handl_time), 0);                  // set new time for call
                    xTimerStart(xFireTimers[available_car - 1], 0);
                    RST_TXT_CLR;
                    break;
                }
                else
                {
                    // printf("There are no calls for fire\n"); // debug only
                    break;
                }
            }

            // Release the mutex
            xSemaphoreGive(xMutex);
        }
        else
        {

            RST_TXT_CLR;
           // printf("failed to get mutex for Task_fire\n"); // debug only
        }

        // delay
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

// fire

uint8_t check_fire_cars_busy(busy_fire_cars_t *cars)
{

    if (cars->fire_1 == AVAILABLE)
        return 1;

    if (cars->fire_2 == AVAILABLE)
        return 2;

    return NO_CAR_AVAILABLE;
}

void set_reset_fire_car_busy(busy_fire_cars_t *cars, uint8_t car_num, bool state)
{
    switch (car_num)
    {
    case 1:
        cars->fire_1 = state;
        break;
    case 2:
        cars->fire_2 = state;
        break;
    }
}

const char *get_fire_car_name(uint8_t car_num)
{
    switch (car_num)
    {
    case 1:
        return "fire 1";
    case 2:
        return "fire 2";
    }
}

// init fire timers

void init_fire_timers(void)
{
    for (int i = 0; i <FIRE_CAR_NUM; i++)
    {
        char *timerName = (char *)pvPortMalloc(10); // malloc
        if (timerName == NULL)
        {
            printf("Error: Failed to allocate memory for timer name\n");
            return;
        }
        // Define unique names for each timer
        snprintf(timerName, 10, "fire%d", i + 1); // Generate unique name
        xFireTimers[i] = xTimerCreate(timerName,
                                        pdMS_TO_TICKS(100),
                                        pdFALSE,
                                        (void *)timerName,             // Store string name as Timer ID
                                        vFireTimerCallBackFunction); // Common callback function

        if (xFireTimers[i] == NULL)
        {

            printf("Error: Failed to create fire timer %d\n", i);
            vPortFree(timerName); // Free allocated memory if timer creation fails
        }
    }
}

void vFireTimerCallBackFunction(TimerHandle_t xTimer)
{
    log_msg_call_t log_msg;
    // get the timer name(ID)
    const char *timerName = (const char *)pvTimerGetTimerID(xTimer);
    //get the car number
    uint8_t carNum = getTimerFireCarNum(timerName);
    GRN_TXT_CLR;
    printf("Fire Car %d has finished call handelling\n", carNum);
    RST_TXT_CLR;
    snprintf(log_msg.log_call_desc, sizeof(log_msg.log_call_desc), " >> Fire Car %d has finished call handelling\n", carNum);
    get_time(log_msg.log_time_stamp);
    if (xQueueSendToBack(xQueue_log, &log_msg, TASKS_SNDQUE_DELAY) != pdPASS)
    {
        my_assert(false, "failed to send call to log queue\n");
    }
    set_reset_fire_car_busy(&busy_fire_cars ,  carNum, false);
    // Stop timer at the end (applies to all cases)
    xTimerStop(xTimer, 0);
}

uint8_t getTimerFireCarNum(const char *timerName)
{
    if (strcmp(timerName, "fire1") == 0)
        return FIRE_1_TIMER;

    if (strcmp(timerName, "fire2") == 0)
        return FIRE_2_TIMER;
    
}
 