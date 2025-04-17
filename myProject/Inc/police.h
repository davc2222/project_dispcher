#ifndef POLICE_H
#define POLICE_H


#define  POLICE_CAR_NUM  3
#define  POLICE_QUEUE_LENGTH  20
#define  POLICE_QUEUE_SIZE      sizeof(call_msg_t)   

#define MIN_POLICE_CALL_HNDL_TIME    5 
#define MAX_POLICE_CALL_HNDL_TIME   10 

// used for timers operation
typedef enum {
   POLICE_1_TIMER = 1,
   POLICE_2_TIMER,
   POLICE_3_TIMER,
   
} policeTimerNum_t;

// handle cars status
typedef struct
{
   uint8_t police_1;
   uint8_t police_2;
   uint8_t police_3;

} busy_police_cars_t;

extern busy_police_cars_t busy_police_cars;

void Task_police(void *pvParameters);
void init_police_department(void);
uint8_t check_police_cars_busy(busy_police_cars_t *cars);
const char *get_police_car_name(uint8_t car_num);
void set_reset_police_car_busy(busy_police_cars_t *cars, uint8_t car_num, bool state);
uint8_t getTimerPoliceCarNum(const char *timerName);
void init_police_timers(void);

#endif