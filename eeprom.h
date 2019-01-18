#ifndef EEPROM_H_INCLUDED
#define EEPROM_H_INCLUDED

void init_eeprom(void);
void eeprom_i2c_deinit(void);
U32 eeprom_read_byte(U32 Address, U8 * Data_read);
U32 eeprom_read_bytes(U32 Address, U32 * Data, U32 Length);
U32 eeprom_write_byte(U32 Address, U32 data);
U32 eeprom_write_bytes(U32 Address, U32 Data, U32 Length);
U32 eeprom_update(U32 Address, U32 Data);
U32 eeprom_update_bytes(U32 Address, U32 Data, U32 Length);
U32 eeprom_wait(void);



// E0 = E1 = E2 = 0
#define sEE_HW_ADDRESS      0xA0
#define I2C_OWN_ADDRESS     0xA0

#define I2C_SPEED               300000




/* Maximum Timeout values for flags and events waiting loops. These timeouts are
   not based on accurate values, they just guarantee that the application will
   not remain stuck if the I2C communication is corrupted.
   You may modify these timeout values depending on CPU frequency and application
   conditions (interrupts routines ...). */
#define sEE_FLAG_TIMEOUT         ((uint32_t)0x1000)
#define sEE_LONG_TIMEOUT         ((uint32_t)(10 * sEE_FLAG_TIMEOUT))


/* Maximum number of trials for sEE_WaitEepromStandbyState() function */
#define sEE_MAX_TRIALS_NUMBER     150






/**
i2c communication
error codes
*/
#define EE_BUS_BUSY 1
#define EE_MASTER_MODE 5
#define EE_MASTER_WRITE 6
#define EE_TRANSMIT_FAIL 8
#define EE_ADDR_FAIL 9
#define EE_RX_FAIL 10
#define EE_STOP_FAIL 11
#define EE_SLAVE_ACK 12
#define EE_MAX_TRIALS 13
#define EE_VERIFICATION 14



#endif // EEPROM_H_INCLUDED
