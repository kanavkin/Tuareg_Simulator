#ifndef COMM_H_INCLUDED
#define COMM_H_INCLUDED


#define COMMAND_DURATION 5


#define COMMAND_CMD_LEN 5
#define COMMAND_NUM_LEN 5
#define COMMAND_LEN (COMMAND_CMD_LEN + COMMAND_NUM_LEN)

typedef enum
{
    CMD_SWEEP_MODE,
    CMD_SWEEP_START,
    CMD_SWEEP_END,
    CMD_SWEEP_INCREMENT,
    CMD_SWEEP_HOLD,
    CMD_CONT_MODE,
    CMD_CONT_RPM,
    CMD_BREAK,
    CMD_IDLE

} commands_t;



typedef struct _tuners_cli_t
{

    U32 cmdPending :1 ;

    commands_t currentCommand;
    U32 command_duration;


} tuners_cli_t ;


extern volatile tuners_cli_t TS_cli;

void comm_periodic();



#endif
