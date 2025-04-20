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

// array to store timers
TimerHandle_t xAmbulanceTimers[AMBULANCE_CAR_NUM];
// struct to hold the data structures for each timer
  TimerDataAmbulance_t ambulanceTimerData[AMBULANCE_CAR_NUM];
// call back function fot the timers
void vAmbulanceTimerCallBackFunction(TimerHandle_t xTimer);
// ambulance queue handler
QueueHandle_t xQueue_ambulance;
// cars status
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
     // init timers
    init_ambulace_timers();
}

/**
 * @brief ambulance task . handle all ambulance cars
 *
 * This function check if there is  call to ambulance
 * then check if there is ava car , if yes it assign car
 * start timer for the call interval
 * and remove the call from the queue
 * if no car ava the call stay in the queue waiting for next try
 * from the task
 * @param[in] void
 * @return void
 */
void Task_ambulance(void *pvParameters)
{
    log_msg_call_t log_msg;
    call_msg_t msg_ambulance;
    static char call_msg_desc[100] = {0};
    static char car_name[15];  
    for (;;)
    {

        if (xSemaphoreTake(xMutex, TASKS_SMFR_DELAY) == pdTRUE)
        {
             // check for ava free car
            uint8_t available_car = check_ambulance_cars_busy(&busy_ambulance_cars);

            switch (available_car)
            {
               // there is no car free  , get out
            case NO_CAR_AVAILABLE:
                //   printf("all ambulance cars occupied\n");//debug
                break;

            case CAR_1:
            case CAR_2:
            case CAR_3:
                if (xQueueReceive(xQueue_ambulance, &msg_ambulance, TASKS_RCVQUE_DELAY) == pdPASS)
                {
                    
                    snprintf(car_name, sizeof(car_name), "Ambulance %d", available_car);
                    set_reset_ambulance_car_busy(&busy_ambulance_cars, available_car, CAR_BUSY);
                    YLW_TXT_CLR;
                    printf("%s  handle call- %d\n", car_name, msg_ambulance.call_id);
                    snprintf(call_msg_desc, sizeof(call_msg_desc), " >> %s  handle call  %d\n", car_name, msg_ambulance.call_id);
                    snprintf(log_msg.log_call_desc, sizeof(log_msg.log_call_desc), " >> %s  handle call  %d\n", car_name, msg_ambulance.call_id);
                    get_time(log_msg.log_time_stamp);
                    if (xQueueSendToBack(xQueue_log, &log_msg, TASKS_SNDQUE_DELAY) != pdPASS)
                    {
                        my_assert(false, "failed to send call to log queue\n");
                    }
                    ambulanceTimerData[available_car - 1].call_id = msg_ambulance.call_id;
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

    if (cars->ambulance_1 == AVAILABLE)
        return 1;
    if (cars->ambulance_2 == AVAILABLE)
        return 2;
    if (cars->ambulance_3 == AVAILABLE)
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

void set_reset_ambulance_car_busy(busy_ambulance_cars_t *cars, uint8_t car_num, bool state)
{
    switch (car_num)
    {
    case 1:
        cars->ambulance_1 = state;
        break;
    case 2:
        cars->ambulance_2 = state;
        break;
    case 3:
        cars->ambulance_3 = state;
        break;
    }
}


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
void init_ambulace_timers(void)
{

    // Static array to store the timer names
    static char timerName[AMBULANCE_CAR_NUM][11]; // Array to hold timer names 

    for (int i = 0; i < AMBULANCE_CAR_NUM; i++)
    {
        // Generate a unique timer name for each timer
        snprintf(timerName[i], sizeof(timerName[i]), "ambulance%d", i + 1);
      
        ambulanceTimerData[i].call_id = 0; // set call id to 
       ambulanceTimerData[i].car_num = i + 1;  
        // Create the timer with the generated name
        xAmbulanceTimers[i] = xTimerCreate(timerName[i],
                                           pdMS_TO_TICKS(100),               // Timer period in milliseconds
                                           pdFALSE,                          // Auto-reload set to false
                                           (void *)&ambulanceTimerData[i],   // Store the timer name as Timer ID
                                           vAmbulanceTimerCallBackFunction); // Callback function when timer expires

        if (xAmbulanceTimers[i] == NULL)
        {
            printf("Error: Failed to create corona timer %d\n", i);
        }
    }
}
/**
 * @brief ambulance timers call back function
 *
 * this function handle all amnulance timers call back
 * it retrive the data from timer structure
 *and print it to terminal indiacting that car  has
 * finished andle call
 * and althugh send the details to log queue
 * stop the timer
 
 * @param[in] timer object
 * @return void
 */
void vAmbulanceTimerCallBackFunction(TimerHandle_t xTimer)
{
    log_msg_call_t log_msg;
    // Retrieve the TimerDataCorona structure from the Timer
    TimerDataAmbulance_t *data = (TimerDataAmbulance_t *)pvTimerGetTimerID(xTimer);
    uint8_t carNum = 0;
    int call_id = 0;
    if (data != NULL)
    { 
        call_id = data->call_id;
        carNum = data->car_num;
    }
    else
    {
        printf("Error: Timer data is invalid\n");
    }
   
    YLW_TXT_CLR;
    printf("Ambulance Car %d has finished handelling call number  %d \n", carNum, call_id);
    snprintf(log_msg.log_call_desc, sizeof(log_msg.log_call_desc), " >> Ambulance Car %d has finished handelling call number  %d \n", carNum, call_id);
    get_time(log_msg.log_time_stamp);
    if (xQueueSendToBack(xQueue_log, &log_msg, TASKS_SNDQUE_DELAY) != pdPASS)
    {
        my_assert(false, "failed to send call to log queue\n");
    }
    set_reset_ambulance_car_busy(&busy_ambulance_cars, carNum, CAR_AVA);
    // Stop timer at the end (applies to all cases)
    xTimerStop(xTimer, 0);
    RST_TXT_CLR;
} 

