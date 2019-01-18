
#ifdef SERIAL_MONITOR
volatile capture_t Monitor_buffer[MONITOR_BUFFER_SIZE];
VU32 mrx_ptr, mrd_ptr;
#endif // SERIAL_MONITOR

/**
serial monitor

TODO
port monitor to new interface
*/

#ifdef SERIAL_MONITOR
void monitor_log(char Data, capture_direction_t Direction)
{
    /**
    this implements a circular buffer
    we can write new data to buffer as long as rx_ptr does not touch rd_ptr
    this will be when rx >= rd, as long as rx is not on the last element and rd on the first
    or when rx is 2 elements behind rd
    */
    if( ((mrx_ptr >= mrd_ptr) && (mrx_ptr != MONITOR_BUFFER_SIZE -1)) || ((mrx_ptr == MONITOR_BUFFER_SIZE -1) && (mrd_ptr > 0)) || (mrx_ptr < mrd_ptr -1) )
    {
        /**
        save to buffer
        */
        Monitor_buffer[mrx_ptr].data= Data;
        Monitor_buffer[mrx_ptr].direction= Direction;
        Monitor_buffer[mrx_ptr].timestamp= system_time;

        mrx_ptr++;

        if(mrx_ptr == MONITOR_BUFFER_SIZE)
        {
            mrx_ptr =0;
        }
    }
}


/**
notify how many bytes are in RX buffer
*/
U32 monitor_available()
{
    if(mrx_ptr > mrd_ptr)
    {
        return mrx_ptr - mrd_ptr;
    }
    else if (mrx_ptr < mrd_ptr)
    {
        return (MONITOR_BUFFER_SIZE -mrd_ptr + mrx_ptr);
    }
    else
    {
        return 0;
    }
}


/**
print one capture byte from buffer
*/
void monitor_print()
{
    U8 buffered_data;

    if(mrd_ptr != mrx_ptr)
    {
        //direction
        if(Monitor_buffer[mrd_ptr].direction == DIRECTION_IN)
        {
            UART1_Send("\r \n IN: ");
        }
        else
        {
             UART1_Send("\r \n \t OUT: ");
        }

        //time stamp
        UART1_Print_U32(Monitor_buffer[mrd_ptr].timestamp);
        UART1_Send(": ");

        //data
        buffered_data= Monitor_buffer[mrd_ptr].data;

        if((buffered_data > 31) && (buffered_data < 127))
        {
            //printable
            UART1_Tx(buffered_data);
        }
        else
        {
            switch(buffered_data)
            {
            case '\r':
            UART1_Tx('C');
            UART1_Tx('R');
            break;

            case '\n':
            UART1_Tx('N');
            UART1_Tx('L');
            break;

            default:
            UART1_Print_U8Hex_new(buffered_data);
            break;

            }

        }

        /**
        do buffer handling
        */
        mrd_ptr++;

        if(mrd_ptr == MONITOR_BUFFER_SIZE)
        {
            mrd_ptr =0;
        }
    }

}
#endif // SERIAL_MONITOR
