
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
// program
#include "police.h"
#include "error.h"
#include "disptcher.h"
#include "main_dispacher_project.h"
#include "log.h"

 // array to store timers
TimerHandle_t xPoliceTimers[POLICE_CAR_NUM];
void vPoliceTimerCallBackFunction(TimerHandle_t xTimer);
QueueHandle_t xQueue_police;
busy_police_cars_t busy_police_cars;
// struct to hold the data structures for each timer
TimerDataPolice_t policeTimerData[POLICE_CAR_NUM];
//  global mutex
extern SemaphoreHandle_t xMutex;
// log queue
extern QueueHandle_t xQueue_log;

/**
 * @brief init the police  department.
 *
 * This function create  task , queue  and  timers
 * @param[in] void
 * @return void
 */

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

/**
 * @brief corona task . handle all police cars
 *
 * This function check if there is  call for police
 * then check if there is free car , if yes it assign car
 * start timer for the call interval
 * and remove the call from the queue
 * if no car ava the call stay in the queue waiting for next try
 * from the task
 * @param[in] void
 * @return void
 */
void Task_police(void *pvParameters)
{
    log_msg_call_t log_msg;
    call_msg_t msg_police;
    char car_name[15] = {0};
    for (;;)

    {
        if (xSemaphoreTake(xMutex, TASKS_SMFR_DELAY) == pdTRUE)
        {
           
            uint8_t available_car = check_police_cars_busy(&busy_police_cars);

            switch (available_car)
            {

            case NO_CAR_AVAILABLE:

                //   printf("all police cars occupied\n"); debug only
                break;

            case CAR_1:
            case CAR_2:
            case CAR_3:
                if (xQueueReceive(xQueue_police, &msg_police, TASKS_RCVQUE_DELAY) == pdPASS)
                {             

                    snprintf(car_name, sizeof(car_name), "Police %d", available_car);
                    set_reset_police_car_busy(&busy_police_cars, available_car, CAR_BUSY);
                    BLUE_TXT_CLR;
                    printf("%s  handle call  %d\n", car_name, msg_police.call_id);
                    snprintf(log_msg.log_call_desc, sizeof(log_msg.log_call_desc), " >> %s  handle call  %d\n", car_name, msg_police.call_id);
                    get_time(log_msg.log_time_stamp);
                    if (xQueueSendToBack(xQueue_log, &log_msg, TASKS_SNDQUE_DELAY) != pdPASS)
                    {
                        my_assert(false, "failed to send call to log queue\n");
                     }
                    policeTimerData[available_car - 1].call_id = msg_police.call_id;
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
           
         //   printf("failed to get mutex for Task_police\n"); // debug only
        }

         
        vTaskDelay(pdMS_TO_TICKS(TASK_POLICE_DELAY));
    }
}

/**
 * @brief set or reset car status
 *
 * @param[in] pointer for the busy  struct, car number , state
 * @return  void
 */

uint8_t check_police_cars_busy(busy_police_cars_t *cars)
{

    if (cars->police_1 == CAR_AVA)
        return 1;

    if (cars->police_2 == CAR_AVA)
        return 2;

    if (cars->police_3 ==CAR_AVA)
        return 3;
    /* no free cars*/
    return NO_CAR_AVAILABLE;
}

/**
 * @brief set or reset car status
 *
 * @param[in] pointer for the busy  struct, car number , state
 * @return  void
 */

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



// init police timers
/**
 * @brief init the cars timer
 *
 * This function create timer to each car 
 * and associated with that car number
 * the timer data struct hold the call id
 * and hold the car num.
 
 * @param[in] void
 * @return void
 */
void init_police_timers(void)
{

    //  array to store the timer names
     char timerName[POLICE_CAR_NUM][11]; // Array to hold timer names 

    for (int i = 0; i < POLICE_CAR_NUM; i++)
    {
        // Generate a unique timer name for each timer
        snprintf(timerName[i], sizeof(timerName[i]), "police%d", i + 1);

        policeTimerData[i].call_id = 0; // set call id to 0
       policeTimerData[i].car_num = i + 1;  
          // Create the timer with the generated name
        xPoliceTimers[i] = xTimerCreate(timerName[i],
                                        pdMS_TO_TICKS(100),            // Timer period in milliseconds
                                        pdFALSE,                       // Auto-reload set to false
                                        (void *)&policeTimerData[i],   // Store the timer data
                                        vPoliceTimerCallBackFunction); // Callback function when timer expires

        if ( xPoliceTimers[i] == NULL)
        {
            printf("Error: Failed to create corona timer %d\n", i);
        }
    }
}


/**
 * @brief police timers call back function
 *
 * this function handle all police timers call back
 * it retrive the data from timer structure
 *and print it to terminal indiacting that car  has
 * finished handle call
 * and althugh send the details to log queue
 * stop the timer
 
 * @param[in] timer object
 * @return void
 */

void vPoliceTimerCallBackFunction(TimerHandle_t xTimer)
{
    log_msg_call_t log_msg;
    // Retrieve the TimerDataCorona structure from the Timer 
    TimerDataPolice_t *data = (TimerDataPolice_t*)pvTimerGetTimerID(xTimer);
    uint8_t carNum = 0;
    int call_id = 0;
    if (data != NULL)
    {
        
        call_id = data->call_id;
        carNum = data->car_num;
    }
    else
    {
       //  printf("Error: Timer data is invalid\n");debug only
    }
 
    BLUE_TXT_CLR;
    printf("Police Car %d has finished handelling call number  %d \n", carNum, call_id);
    snprintf(log_msg.log_call_desc, sizeof(log_msg.log_call_desc), " >> police Car %d has finished handelling call number  %d \n", carNum, call_id);
    get_time(log_msg.log_time_stamp);
    if (xQueueSendToBack(xQueue_log, &log_msg, TASKS_SNDQUE_DELAY) != pdPASS)
    {
        my_assert(false, "failed to send call to log queue\n");
    }
    set_reset_police_car_busy(&busy_police_cars, carNum, false);
    RST_TXT_CLR;
    
    xTimerStop(xTimer, 0);
}

