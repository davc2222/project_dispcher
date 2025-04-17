#ifndef AMBULANCE_H
#define AMBULANCE_H

#define TASK_AMBLNCE_DELAY   1000



#define AMBULANCE_CAR_NUM  3
#define AMBULANCE_QUEUE_LENGTH  100
#define AMBULANCE_QUEUE_SIZE       sizeof(call_msg_t)  

#define MIN_AMBULANCE_CALL_HNDL_TIME    5 
#define MAX_AMBULANCE_CALL_HNDL_TIME   10 

// used for timers operation
typedef enum {
   AMBULANCE_1_TIMER = 1,
   AMBULANCE_2_TIMER,
   AMBULANCE_3_TIMER,
   
} ambulanceTimerNum_t;

// handle cars status
typedef struct
{
   uint8_t ambulance_1;
   uint8_t ambulance_2;
   uint8_t ambulance_3;
}busy_ambulance_cars_t;
 
// busy struct
extern  busy_ambulance_cars_t busy_ambulance_cars;


// function
void Task_ambulance(void *pvParameters);
void init_ambulance_department(void);
uint8_t check_ambulance_cars_busy(busy_ambulance_cars_t *cars);
const char *get_ambulance_car_name(uint8_t car_num);
void set_reset_ambulance_car_busy(busy_ambulance_cars_t *cars, uint8_t car_num, bool state);
uint8_t getTimerAmbulanceCarNum(const char *timerName);
void  init_ambulace_timers(void);


#endif