#ifndef SERIAL_MONITOR_H_INCLUDED
#define SERIAL_MONITOR_H_INCLUDED

/**
use the serial monitor to debug TunerStudio communication
*/
//#define SERIAL_MONITOR


#ifdef SERIAL_MONITOR
typedef enum {

    DIRECTION_IN,
    DIRECTION_OUT

} capture_direction_t ;


typedef struct _capture_t {

    U32 timestamp;
    capture_direction_t direction;
    U8 data;

} capture_t ;

#define MONITOR_BUFFER_SIZE 500

#endif // SERIAL_MONITOR

//serial monitor
#ifdef SERIAL_MONITOR
void monitor_log(char Data, capture_direction_t Direction);
U32 monitor_available();
void monitor_print();
#endif // SERIAL_MONITOR



#endif // SERIAL_MONITOR_H_INCLUDED
