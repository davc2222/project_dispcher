#ifndef POLICE_H
#define POLICE_H

#define TASK_POLICE_DELAY   1000

#define  POLICE_CAR_NUM  3
#define  POLICE_QUEUE_LENGTH  50
#define  POLICE_QUEUE_SIZE      sizeof(call_msg_t)   

#define MIN_POLICE_CALL_HNDL_TIME    5 
#define MAX_POLICE_CALL_HNDL_TIME   10 



// handle cars status
typedef struct
{
   uint8_t police_1;
   uint8_t police_2;
   uint8_t police_3;

} busy_police_cars_t;

// Define a structure to attach to the timer
typedef struct {

   int call_id;
   uint8_t car_num;
  
} TimerDataPolice_t;

extern busy_police_cars_t busy_police_cars;

void Task_police(void *pvParameters);
void init_police_department(void);
uint8_t check_police_cars_busy(busy_police_cars_t *cars);
void set_reset_police_car_busy(busy_police_cars_t *cars, uint8_t car_num, bool state);
void init_police_timers(void);

#endif