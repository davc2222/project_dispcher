#include <FreeRTOS.h>
#include <FreeRTOSConfig.h>
#include <task.h>
#include <time.h>
#include <queue.h>
#include <semphr.h>
#include <stdio.h>
#include <stdbool.h>
#include "log.h"
#include "error.h"
#include "main_dispacher_project.h"
#include  "disptcher.h"


#define TIME_BUFFER_SIZE 25
char time_buffer[TIME_BUFFER_SIZE];
char *time_buffer_p = &time_buffer[0];
char formatted_message[100];
const char *log_filename = LOG_FILE_NAME;
QueueHandle_t xQueue_log;
extern SemaphoreHandle_t xMutex;

/**
 * @brief init the log 
 *
 * This function create  task , queue  and log file
 * @param[in] void
 * @return void
 */

void init_log_handler(void)
{

    if (xTaskCreate(Task_log, "Task log", configMINIMAL_STACK_SIZE, NULL, TASK_LOG_PRI, NULL) != pdPASS)
    {
        my_assert(false, "failed to create Task log");
    }

    // create  log queue

    xQueue_log = xQueueCreate(LOG_QUEUE_LENGTH, LOG_QUEUE_SIZE);

    if (xQueue_log == NULL)
    {

        my_assert(false, "failed to create log queue");
    }

    init_log_file();
}


/**
 * @brief the task read messages from queue 
 * and write it to log file with tims stamp
 *
 * 
 * @param[in] void *pvParameters
 * @return void
 */

void Task_log(void *pvParameters)
{

    log_msg_call_t log_msg;

    for (;;)
    {
        if (xSemaphoreTake(xMutex, TASK_LOG_SMFR_DELAY) == pdTRUE)
        {
            if (xQueueReceive(xQueue_log, &log_msg, TASKS_RCVQUE_DELAY) == pdPASS)
            {

              write_call_time_to_log(LOG_FILE_NAME);
              write_call_details_to_log(LOG_FILE_NAME, log_msg.log_call_desc);
             
            }

        }
        else
        {

           // printf("failed to get mutex for log task\n"); // debug only
        }
     
        // Release the mutex
        xSemaphoreGive(xMutex);
        vTaskDelay(pdMS_TO_TICKS(1000));

    }
}


/**
 * @brief open a log file
 * clear the file 
 * and then open it in appened mode
 * write the title , date created and description
 * 
 * @param[in] void *pvParameters
 * @return void
 */
void init_log_file(void)
{

    // store time in buffer and get pointer
    time_buffer_p = get_time(time_buffer);

    // create file if not exsist in write mode
    FILE *log_file = fopen(log_filename, "w"); // create and open file
    if (log_file == NULL)
    {

        my_assert(false, "failed opening log.txt");
    }

    // clear file if not empty
    writeToLog(log_file, " ");
    // close file
    if (fclose(log_file) != 0)
    {

        my_assert(false, "failed closing log.txt");
    }

    // open the file in appened mode
    log_file = fopen(log_filename, "a");
    if (log_file == NULL)
    {

        my_assert(false, "failed opening log.txt");
    }

    // write the header file >> date and time
    snprintf(formatted_message, sizeof(formatted_message), "Log file for 911 calls.created in :============ : %s============\n\n\n", time_buffer_p);
    writeToLog(log_file, formatted_message);
    fflush(log_file); //
}

/// @ write string to file
/// @param[in] filename pointer , string pointer
/// @return void

void writeToLog(FILE *filename, const char *message)
{
    fprintf(filename, "%s", message); // Write the message to the file
}

/**
 * @brief write time stamp to log file
 * 
 * @param[in] pointer to file name
 * @return void
 */

void write_call_time_to_log(const char *file_name)
{

    FILE *file = fopen(file_name, "a"); // Open the file in write mode

    // store time in buffer and get pointer
    time_buffer_p = get_time(time_buffer);
    writeToLog(file, time_buffer_p);
    fflush(file);
}

/**
 * @brief write call details to log file

 * @param[in] pointer to log file, pointer to string
 * @return void
 */

void write_call_details_to_log(const char *file_name, const char *desc)
{

    FILE *file = fopen(file_name, "a"); // Open the file in write mode
   
    writeToLog(file, desc);

    fflush(file);
}

/**
 * @brief get time and store it 
 * into buffer
 * 
 * @param[in] pointer to buffer
 * @return pointer to buffer
 */

char *get_time(char *time_buffer)
{
    time_t current_time;
    struct tm *local_time;

    // Get current time
    current_time = time(NULL);
    if (current_time == (time_t)(-1))
    {
        my_assert(false, "failed to get time");
    }

    // Convert time_t to struct tm (local time)
    local_time = localtime(&current_time);

    if (local_time == NULL)
    {
        my_assert(false, "Failed to convert time");
        return NULL;
    }
    // Format the time as a string
    strftime(time_buffer, TIME_BUFFER_SIZE, "%Y-%m-%d %H:%M:%S", local_time);

    

    return &time_buffer[0];
}
