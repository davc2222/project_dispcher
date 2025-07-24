#ifndef DISPTCHER_H
#define DISPTCHER_H



#define NO_CAR_AVAILABLE    -1

#define CAR_AVA   false
#define CAR_BUSY true

// define departments number
#define CALL_TYPE_NUM  4

#define MIN_DSPCR_CALL_HNDL_TIME    1
#define MAX_DSPCR_CALL_HNDL_TIME   5

// used only one time at program start
#define DSPTC_TIMER__START_PERIOD_MS  1000

#define DSPCR_QUEUE_LENGTH  50
#define DSPCR_QUEUE_SIZE       sizeof(call_msg_t)  

// set prioreties 
#define TASK_DSPTCR_PRI    3
#define TASK_PLC_PRI           2
#define TASK_FRE_PRI           2
#define TASK_AMBLNC_PRI  2
#define TASK_CRNA_PRI       2
#define TASK_LOG_PRI          1


#define CAR_1    1
#define CAR_2    2
#define CAR_3    3
#define CAR_4    4

#define TASK_LOG_SMFR_DELAY  100
#define TASKS_SMFR_DELAY        200
#define TASKS_RCVQUE_DELAY   100
#define TASKS_SNDQUE_DELAY   100


// used for call type number

typedef enum
{
    police,
    ambulance,
    fire,
    corona
} call_types_t;

// struct for handle calls
typedef struct
{
    int call_id;
    int call_type;
    char call_desc[100];

} call_msg_t;

void init_dispacher_center(void);
void Task_dispcher(void *pvParameters);
void handle_call_disptcher(void);
const char *get_call_type_str(call_types_t call_type);

#endif