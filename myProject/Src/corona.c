
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
#include "error.h"
#include "disptcher.h"
#include "main_dispacher_project.h"
#include "corona.h"
#include "log.h"

  // array to store timers
TimerHandle_t xCoronaTimers[CORONA_CAR_NUM];
// struct to hold the data structures for each timer
TimerDataCorona_t coronaTimerData[CORONA_CAR_NUM];
// declare call back function to corona timere 
void vCoronaTimerCallBackFunction(TimerHandle_t xTimer);
// corona queue handler
QueueHandle_t xQueue_corona;
// struct contain cars status
busy_corona_cars_t busy_corona_cars;
// global mutex 
extern SemaphoreHandle_t xMutex;
//  log queue 
extern QueueHandle_t xQueue_log;

/**
 * @brief init the corona department.
 *
 * This function create  task , queue  and  timers
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
     
    // create timers
    init_corona_timers();
}
/**
 * @brief corona task . handle all ambulance cars
 *
 * This function check if there is  call to corona
 * then check if there is free car , if yes it assign car
 * start timer for the call interval
 * and remove the call from the queue
 * if no car ava the call stay in the queue waiting for next try
 * from the task
 * @param[in] void
 * @return void
 */
void Task_corona(void *pvParameters)
{
    log_msg_call_t log_msg; // struct fo log file
    call_msg_t msg_corona;  // struct to read and write queue

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
                     
                    char car_name[15];
                    snprintf(car_name, sizeof(car_name), "Corona %d", available_car);
                    set_reset_corona_car_busy(&busy_corona_cars, available_car, CAR_BUSY);
                    PRPL_TXT_CLR;
                    printf("%s  handle call- %d\n", car_name, msg_corona.call_id);
                    snprintf(log_msg.log_call_desc, sizeof(log_msg.log_call_desc), " >> %s  handle call  %d\n", car_name, msg_corona.call_id);
                    get_time(log_msg.log_time_stamp);
                    if (xQueueSendToBack(xQueue_log, &log_msg, TASKS_SNDQUE_DELAY) != pdPASS)
                    {
                        my_assert(false, "failed to send call to log queue\n");
                    }
                    coronaTimerData[available_car - 1].call_id = msg_corona.call_id;
                    int handl_time = getRandomNumber(MIN_CORONA_CALL_HNDL_TIME, MAX_CORONA_CALL_HNDL_TIME) * 1000; // time between 5 - 10sec
                    printf("corona ava car = %d\n", available_car);
                    xTimerChangePeriod(xCoronaTimers[available_car - 1], pdMS_TO_TICKS(handl_time), 0); // set new time for call
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
    if (cars->corona_1 == AVAILABLE)
        return 1;
    if (cars->corona_2 == AVAILABLE)
        return 2;
    if (cars->corona_3 == AVAILABLE)
        return 3;
    if (cars->corona_4 == AVAILABLE)
        return 4;
    /* no free cars*/
    return NO_CAR_AVAILABLE;
}

/**
 * @brief set or reset car status
 *
 * @param[in] pointer for the busy  struct, car number , state
 * @return  void
 */

void set_reset_corona_car_busy(busy_corona_cars_t *cars, uint8_t car_num, bool state)
{
    switch (car_num)
    {
    case 1:
        cars->corona_1 = state;
        break;
    case 2:
        cars->corona_2 = state;
        break;
    case 3:
        cars->corona_3 = state;
        break;
    case 4:
        cars->corona_4 = state;
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

void init_corona_timers(void)
{
    // Static array to store the timer names
    static char timerName[CORONA_CAR_NUM][11]; // Array to hold timer names

    for (int i = 0; i < CORONA_CAR_NUM; i++)
    {
        // Generate a unique timer name for each timer
        snprintf(timerName[i], sizeof(timerName[i]), "corona%d", i + 1);

        coronaTimerData[i].timerId = timerName[i]; // 
        coronaTimerData[i].call_id = 0; // set call id to 0
        coronaTimerData[i].car_num = i + 1;  
          // Create the timer with the generated name
        xCoronaTimers[i] = xTimerCreate(timerName[i],
                                        pdMS_TO_TICKS(100),            // Timer period in milliseconds
                                        pdFALSE,                       // Auto-reload set to false
                                        (void *)&coronaTimerData[i],   // Store the timer data
                                        vCoronaTimerCallBackFunction); // Callback function when timer expires

        if (xCoronaTimers[i] == NULL)
        {
            printf("Error: Failed to create corona timer %d\n", i);
        }
    }
}

/**
 * @brief corona timers call back function
 *
 * this function handle all corona timers call back
 * it retrive the data from timer structure
 *and print it to terminal indiacting that car  has
 * finished andle call
 * and althugh send the details to log queue
 * stop the timer
 
 * @param[in] timer object
 * @return void
 */
void vCoronaTimerCallBackFunction(TimerHandle_t xTimer)
{
    log_msg_call_t log_msg;
    // Retrieve the TimerDataCorona structure from the Timer 
    TimerDataCorona_t *data = (TimerDataCorona_t *)pvTimerGetTimerID(xTimer);
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

    PRPL_TXT_CLR;
    printf("Corona Car %d has finished handelling call number  %d \n", carNum, call_id);
    snprintf(log_msg.log_call_desc, sizeof(log_msg.log_call_desc), " >> Corona Car %d has finished handelling call number  %d \n", carNum, call_id);
    get_time(log_msg.log_time_stamp);
    if (xQueueSendToBack(xQueue_log, &log_msg, TASKS_SNDQUE_DELAY) != pdPASS)
    {
        my_assert(false, "failed to send call to log queue\n");
    }
    set_reset_corona_car_busy(&busy_corona_cars, carNum, CAR_AVA);
    RST_TXT_CLR;
    xTimerStop(xTimer, 0);
}

