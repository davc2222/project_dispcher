#ifndef LOG_H
#define LOG_H


#define LOG_FILE_NAME "log_file.txt" // Define the log file name

#define LOG_QUEUE_LENGTH  20
#define LOG_QUEUE_SIZE       sizeof(log_msg_call_t)  

void init_log_file(void);

char *get_time(char *time_buffer);

void write_call_details_to_log(const char *file_name, const char *desc);

void write_call_time_to_log(const char *file_name);

void writeToLog(FILE *filename, const char *message);

void Task_log(void *pvParameters);

void init_log_handl(void );



// used to handle time stamp for log file
typedef struct  {

    char  log_time_stamp[25];
    char  log_call_desc[100];

} log_msg_call_t;

#endif