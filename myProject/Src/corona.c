
#include <FreeRTOS.h>
#include <FreeRTOSConfig.h>
#include <task.h>
#include <semphr.h>
#include <timers.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <queue.h>
#include "error.h"
#include "disptcher.h"
#include "main_dispacher_project.h"
#include "corona.h"
#include "log.h"

/*declare timers for corona */ 
TimerHandle_t xCoronaTimers[CORONA_CAR_NUM];
/*  declare call back function to corona timere*/
void vCoronaTimerCallBackFunction(TimerHandle_t xTimer);
// corona queue handler
QueueHandle_t xQueue_corona;
/* struct contain cars status*/
busy_corona_cars_t busy_corona_cars;
/*  global mutex */
extern SemaphoreHandle_t xMutex; 
/* log queue*/
extern QueueHandle_t xQueue_log;

/**
 * @brief init the corona department.
 *
 * This function create  task , queue  and init timers
 * @param[in] void
 * @return void
 */
void init_corona_department(void)
{

    // Create ambulancd task
    if (xTaskCreate(Task_corona, "Task Corona", configMINIMAL_STACK_SIZE, NULL, TASK_CRNA_PRI, NULL) != pdPASS)
    {
        my_assert(false, "failed to create Task corona");
    }

    // create  police queue
    xQueue_corona = xQueueCreate(CORONA_QUEUE_LENGTH, CORONA_QUEUE_SIZE);

    if (xQueue_corona == NULL)
    {

        my_assert(false, "failed to create corona queue");
    }

      init_corona_timers();
}
/**
 * @brief corona task . handle all corona cars
 *
 * This function check if there is  corona car ava then check for
 * calls and if there is assign it to the free car and set it busy
 * init timer for  random handle time
 * @param[in] void
 * @return void
 */
void Task_corona(void *pvParameters)
{
    log_msg_call_t log_msg; // struct fo log file
    call_msg_t msg_corona; // struct to read and write queue
 
    for (;;)
    {
        if (xSemaphoreTake(xMutex, TASKS_SMFR_DELAY) == pdTRUE)
        {
            uint8_t available_car = check_corona_cars_busy(&busy_corona_cars);

            switch (available_car)
            {
               /*  no car ava , get out  and give mutex */
            case NO_CAR_AVAILABLE:
                break;

            case CAR_1:
            case CAR_2:
            case CAR_3:
            case CAR_4:
                if (xQueueReceive(xQueue_corona, &msg_corona, TASKS_RCVQUE_DELAY) == pdPASS)
                {
                    const char *car_name = get_corona_car_name(available_car);
                    set_reset_corona_car_busy(&busy_corona_cars, available_car, true);
                    PRPL_TXT_CLR;
                    printf("%s  handle call- %d\n", car_name, msg_corona.call_id);
                    snprintf(log_msg.log_call_desc, sizeof(log_msg.log_call_desc), " >> %s  handle call  %d\n", car_name, msg_corona.call_id);
                    get_time(log_msg.log_time_stamp);
                    if (xQueueSendToBack(xQueue_log, &log_msg, TASKS_SNDQUE_DELAY) != pdPASS)
                    {
                        my_assert(false, "failed to send call to log queue\n");
                    }
                    
                    int handl_time = getRandomNumber(MIN_CORONA_CALL_HNDL_TIME, MAX_CORONA_CALL_HNDL_TIME) * 1000; // time between 5 - 10sec
                    printf("corona ava car = %d\n" ,available_car);
                    xTimerChangePeriod(xCoronaTimers[available_car - 1], pdMS_TO_TICKS(handl_time), 0);            // set new time for call
                    xTimerStart(xCoronaTimers[available_car - 1], 0);
                    RST_TXT_CLR;
                    break;
                }
                else
                {
                     printf("There are no calls for corona\n"); // debug only
                    break;
                }
            }

            // Release the mutex
            xSemaphoreGive(xMutex);
        }
        else
        {

          
         printf("failed to get mutex for Task_corona\n"); // debug only
        }

        // delay
        vTaskDelay(pdMS_TO_TICKS(1000));
   
    }
}

/** 
* @brief check if there is corona free car 
* 
* @param[in] pointer for the struct
* @return  number that represent the free car
* if non of the cars are free return "NO_CAR_AVAILABLE"
*/
uint8_t check_corona_cars_busy(busy_corona_cars_t *cars)
{
    if (cars->corona_1 == AVAILABLE)  return 1; 
    if (cars->corona_2 == AVAILABLE)  return 2;
    if (cars->corona_3 == AVAILABLE)  return 3;   
    if (cars->corona_4 == AVAILABLE)  return 4;
    /* no free cars*/ 
    return NO_CAR_AVAILABLE;
}

void set_reset_corona_car_busy(busy_corona_cars_t *cars, uint8_t car_num, bool state)
{
    switch (car_num)
    {
    case 1: cars->corona_1 = state; break;    
    case 2: cars->corona_2 = state; break;
    case 3:  cars->corona_3 = state;break;
    case 4:  cars->corona_4 = state; break;
       
    }
}

const char *get_corona_car_name(uint8_t car_num)
{
    switch (car_num)
    {
    case 1:
        return "Corona 1";
    case 2:
        return "Corona 2";
    case 3:
        return "Corona  3";
    case 4:
        return "Corona  4";
    }
}

void init_corona_timers(void)
{

    for (int i = 0; i < CORONA_CAR_NUM; i++)
    {
        char *timerName = (char *)pvPortMalloc(11); // malloc
        if (timerName == NULL)
        {
            printf("Error: Failed to allocate memory for timer name\n");
            return;
        }
        // Define unique names for each timer
        snprintf(timerName, 11, "corona%d", i + 1); // Generate unique name
        xCoronaTimers[i] = xTimerCreate(timerName,
                                        pdMS_TO_TICKS(100),
                                        pdFALSE,
                                        (void *)timerName,             // Store string name as Timer ID
                                        vCoronaTimerCallBackFunction); // Common callback function

        if (xCoronaTimers[i] == NULL)
        {

            printf("Error: Failed to create corona timer %d\n", i);
            vPortFree(timerName); // Free allocated memory if timer creation fails
        }
    }
}

void vCoronaTimerCallBackFunction(TimerHandle_t xTimer)
{
    log_msg_call_t log_msg;
    // get the timer name(ID)
    const char *timerName = (const char *)pvTimerGetTimerID(xTimer);
    // get the car number
    uint8_t carNum = getTimerCoronaCarNum(timerName);
    PRPL_TXT_CLR;
    printf("Corona Car %d has finished call handelling\n", carNum);
    snprintf(log_msg.log_call_desc, sizeof(log_msg.log_call_desc), " >> Corona Car %d has finished call handelling\n", carNum);
    get_time(log_msg.log_time_stamp);
    // if (xQueueSendToBack(xQueue_log, &log_msg, TASKS_SNDQUE_DELAY) != pdPASS)
    // {
    //     my_assert(false, "failed to send call to log queue\n");
    // }
    set_reset_corona_car_busy(&busy_corona_cars, carNum, false);
  RST_TXT_CLR;
    xTimerStop(xTimer, 0);
}

uint8_t getTimerCoronaCarNum(const char *timerName)
{

    if (strcmp(timerName, "corona1") == 0)
        return CORONA_1_TIMER;
    if (strcmp(timerName, "corona2") == 0)
        return CORONA_2_TIMER;
    if (strcmp(timerName, "corona3") == 0)
        return CORONA_3_TIMER;
    if (strcmp(timerName, "corona4") == 0)
        return CORONA_4_TIMER;
}
