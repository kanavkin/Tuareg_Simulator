#ifndef SCHEDULER_H_INCLUDED
#define SCHEDULER_H_INCLUDED


#define SCHEDULER_PS 720
#define SCHEDULER_PERIOD_US 10
#define SCHEDULER_MAX_PERIOD_US (U32) 0xFFFF * SCHEDULER_PERIOD_US -1



typedef enum {

    IGN_CH1,
    IGN_CH2,

    FUEL_CH1,
    FUEL_CH2

} scheduler_channel_t;




typedef struct _scheduler_t {

    output_pin_t ign_ch1_action;
    output_pin_t ign_ch2_action;
    output_pin_t fuel_ch1_action;
    output_pin_t fuel_ch2_action;

} scheduler_t;

void init_scheduler();
void scheduler_set_channel(scheduler_channel_t target_ch, output_pin_t action, U32 delay_us);
void scheduler_reset_channel(scheduler_channel_t target_ch);

#endif // SCHEDULER_H_INCLUDED
