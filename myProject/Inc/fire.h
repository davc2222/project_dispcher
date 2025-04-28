#ifndef FIRE_H
#define FIRE_H



#define FIRE_CAR_NUM  2
#define FIRE_QUEUE_LENGTH  20
#define FIRE_QUEUE_SIZE       sizeof(call_msg_t)  

#define MIN_FIRE_CALL_HNDL_TIME    5 
#define MAX_FIRE_CALL_HNDL_TIME   10 



// handle cars status
typedef struct
{
   uint8_t fire_1;
   uint8_t fire_2;
   
} busy_fire_cars_t;

// Define a structure to attach to the timer
typedef struct {

   int call_id;
   uint8_t car_num;

} TimerDataFire_t;

 extern  busy_fire_cars_t busy_fire_cars;


void  init_fire_department(void);
uint8_t check_fire_cars_busy(busy_fire_cars_t *cars);
void set_reset_fire_car_busy(busy_fire_cars_t *cars, uint8_t car_num, bool state);
void Task_fire(void *pvParameters);
void init_fire_timers(void);

#endif





