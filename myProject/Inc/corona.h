#ifndef CORONA_H
#define CORONA_H

#include <stdint.h>
#include <stdbool.h>

#define CORONA_CAR_NUM  4
#define CORONA_QUEUE_LENGTH  20
#define CORONA_QUEUE_SIZE       sizeof(call_msg_t)  


#define MIN_CORONA_CALL_HNDL_TIME    5 
#define MAX_CORONA_CALL_HNDL_TIME   10 

// used for timers operation
typedef enum {
   CORONA_1_TIMER = 1,
   CORONA_2_TIMER,
   CORONA_3_TIMER,
   CORONA_4_TIMER
}coronaTimerNum_t;

// handle cars status
typedef struct
{
   uint8_t  corona_1;
   uint8_t  corona_2;
   uint8_t  corona_3;
   uint8_t  corona_4;

}busy_corona_cars_t;
 
extern busy_corona_cars_t busy_corona_cars;



void Task_corona(void *pvParameters);
void init_corona_department(void);
uint8_t check_corona_cars_busy(busy_corona_cars_t *cars);
const char *get_corona_car_name(uint8_t);
void set_reset_corona_car_busy(busy_corona_cars_t *cars, uint8_t car_num, bool state);
uint8_t getTimerCoronaCarNum(const char *timerName);
void init_corona_timers(void);


#endif