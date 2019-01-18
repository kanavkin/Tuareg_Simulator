#include "stm32f10x.h"
#include "stm32_libs/boctok/stm32_gpio.h"

#include "types.h"
#include "eeprom.h"


void eeprom_i2c_deinit(void)
{
    I2C_Cmd(I2C1, DISABLE);
    I2C_DeInit(I2C1);

    //clock
    RCC->APB1ENR &= ~RCC_APB1ENR_I2C1EN;

    //SCL and SDA
    GPIO_configure(GPIOB, 6, GPIO_IN_FLOAT);
    GPIO_configure(GPIOB, 7, GPIO_IN_FLOAT);
}



/**
using I2C_1 for eeprom
*/
void init_eeprom(void)
{
    I2C_InitTypeDef  I2C_InitStructure;

    //clock
    RCC->APB2ENR |= RCC_APB2Periph_GPIOB;
    RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;

    //SCL and SDA
    GPIO_configure(GPIOB, 6, GPIO_AF_OD_50MHZ);
    GPIO_configure(GPIOB, 7, GPIO_AF_OD_50MHZ);


    //I2C configuration
    I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
    I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
    I2C_InitStructure.I2C_OwnAddress1 = I2C_OWN_ADDRESS;
    I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
    I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_InitStructure.I2C_ClockSpeed = I2C_SPEED;

    I2C_Cmd(I2C1, ENABLE);
    I2C_Init(I2C1, &I2C_InitStructure);

}



U32 eeprom_write_byte(U32 Address, U32 data)
{
    U32 sEETimeout;

    // wait while the bus is busy
    sEETimeout = sEE_LONG_TIMEOUT;
    while(I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY))
    {
        if((sEETimeout--) == 0) return EE_BUS_BUSY;
    }

    /**
    wait for the previous write cycle to complete
    */
    sEETimeout= eeprom_wait();

    if( sEETimeout != 0)
    {
        return sEETimeout;
    }

    I2C_GenerateSTART(I2C1, ENABLE);

    //wait until Master mode selected
    sEETimeout = sEE_FLAG_TIMEOUT;
    while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT))
    {
        if((sEETimeout--) == 0) return EE_MASTER_MODE;
    }

    //Send EEPROM address for write mode
    I2C_Send7bitAddress(I2C1, (U8) sEE_HW_ADDRESS, I2C_Direction_Transmitter);


    //wait until Master Write mode selected
    sEETimeout = sEE_FLAG_TIMEOUT;
    while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
    {
        if((sEETimeout--) == 0) return EE_MASTER_WRITE;
    }


    //Send the EEPROM's internal address: MSB
    I2C_SendData(I2C1, (U8)((Address & 0xFF00) >> 8));

    //wait until byte was transmitted
    sEETimeout = sEE_FLAG_TIMEOUT;
    while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
    {
        if((sEETimeout--) == 0) return EE_TRANSMIT_FAIL;
    }

    //Send the EEPROM's internal address: LSB
    I2C_SendData(I2C1, (U8)(Address & 0x00FF));

    //wait until byte was transmitted
    sEETimeout = sEE_FLAG_TIMEOUT;
    while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
    {
        if((sEETimeout--) == 0) return EE_TRANSMIT_FAIL;
    }


    //Send data to write
    I2C_SendData(I2C1, data);

    //wait until byte was transmitted
    sEETimeout = sEE_FLAG_TIMEOUT;
    while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
    {
        if((sEETimeout--) == 0) return EE_TRANSMIT_FAIL;
    }

    I2C_GenerateSTOP(I2C1, ENABLE);

    /**
    Perform a read on SR1 and SR2 register
    to clear pending flags
    */
    (void)I2C1->SR1;
    (void)I2C1->SR2;

    return 0;
}



U32 eeprom_read_byte(U32 Address, U8 * Data_read)
{
    U32 sEETimeout;

    // wait while the bus is busy
    sEETimeout = sEE_LONG_TIMEOUT;
    while(I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY))
    {
        if((sEETimeout--) == 0) return EE_BUS_BUSY;
    }

    /**
    wait for the previous write cycle to complete
    */
    sEETimeout= eeprom_wait();

    if( sEETimeout != 0)
    {
        return sEETimeout;
    }

    I2C_GenerateSTART(I2C1, ENABLE);

    //wait until Master mode selected
    sEETimeout = sEE_FLAG_TIMEOUT;
    while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT))
    {
        if((sEETimeout--) == 0) return EE_MASTER_MODE;
    }


    /**
    Dummy write
    */

    //Send EEPROM address for write mode
    I2C_Send7bitAddress(I2C1, (U8) sEE_HW_ADDRESS, I2C_Direction_Transmitter);


    //wait until Master Write mode selected
    sEETimeout = sEE_FLAG_TIMEOUT;
    while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
    {
        if((sEETimeout--) == 0) return EE_MASTER_WRITE;
    }


    //Send the EEPROM's internal address: MSB
    I2C_SendData(I2C1, (U8)((Address & 0xFF00) >> 8));

    //wait until byte was transmitted
    sEETimeout = sEE_FLAG_TIMEOUT;
    while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
    {
        if((sEETimeout--) == 0) return EE_TRANSMIT_FAIL;
    }

    //Send the EEPROM's internal address: LSB
    I2C_SendData(I2C1, (U8)(Address & 0x00FF));

    //wait until byte was transmitted
    sEETimeout = sEE_FLAG_TIMEOUT;
    while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
    {
        if((sEETimeout--) == 0) return EE_TRANSMIT_FAIL;
    }

    /**
    Read data
    */

    I2C_GenerateSTART(I2C1, ENABLE);

    //wait until Master mode selected
    sEETimeout = sEE_FLAG_TIMEOUT;
    while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT))
    {
        if((sEETimeout--) == 0) return EE_MASTER_MODE;
    }


    //send EEPROM address for read
    I2C_Send7bitAddress(I2C1, (U8) (sEE_HW_ADDRESS | 0x01), I2C_Direction_Receiver);


    //wait until ADDR flag is set (ADDR is still not cleared)
    sEETimeout = sEE_FLAG_TIMEOUT;
    while(I2C_GetFlagStatus(I2C1, I2C_FLAG_ADDR) == RESET)
    {
      if((sEETimeout--) == 0) return EE_ADDR_FAIL;
    }

    // Disable Acknowledgement
    I2C_AcknowledgeConfig(I2C1, DISABLE);


    __disable_irq();

    //Clear ADDR register by reading SR2 register
    (void)I2C1->SR2;

    I2C_GenerateSTOP(I2C1, ENABLE);

    __enable_irq();


    //wait for the byte to be received
    sEETimeout = sEE_FLAG_TIMEOUT;
    while(I2C_GetFlagStatus(I2C1, I2C_FLAG_RXNE) == RESET)
    {
      if((sEETimeout--) == 0) return EE_RX_FAIL;
    }

    //read the byte received
    *Data_read= I2C_ReceiveData(I2C1);

    //wait to make sure that STOP control bit has been cleared
    sEETimeout = sEE_FLAG_TIMEOUT;
    while(I2C1->CR1 & I2C_CR1_STOP)
    {
      if((sEETimeout--) == 0) return EE_STOP_FAIL;
    }

    //Re-Enable Acknowledgement to be ready for another reception
    I2C_AcknowledgeConfig(I2C1, ENABLE);

    return 0;
}



/**
  * Wait for EEPROM Standby state.
  */
U32 eeprom_wait(void)
{
    U32 sEETimeout;
    VU16 tmpSR1 = 0;
    VU32 sEETrials = 0;

    //while the bus is busy
    sEETimeout = sEE_LONG_TIMEOUT;
    while(I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY))
    {
        if((sEETimeout--) == 0) return EE_BUS_BUSY;
    }

    /**
    Keep looping till the slave acknowledge his address or maximum number
    of trials is reached
    */
    while (1)
    {
        I2C_GenerateSTART(I2C1, ENABLE);

        //wait until Master mode selected
        sEETimeout = sEE_FLAG_TIMEOUT;
        while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT))
        {
            if((sEETimeout--) == 0) return EE_MASTER_MODE;
        }

        //send EEPROM address for write */
        I2C_Send7bitAddress(I2C1, (U8) sEE_HW_ADDRESS, I2C_Direction_Transmitter);

        sEETimeout = sEE_LONG_TIMEOUT;

        do
        {
            tmpSR1 = I2C1->SR1;

            if((sEETimeout--) == 0) return EE_SLAVE_ACK;
        }
        while((tmpSR1 & (I2C_SR1_ADDR | I2C_SR1_AF)) == 0);

        //check if the ADDR flag has been set
        if (tmpSR1 & I2C_SR1_ADDR)
        {
            (void)I2C1->SR2;
            I2C_GenerateSTOP(I2C1, ENABLE);

            return 0;
        }
        else
        {
            I2C_ClearFlag(I2C1, I2C_FLAG_AF);
        }


        if (sEETrials++ == sEE_MAX_TRIALS_NUMBER)
        {
            return EE_MAX_TRIALS;
        }
    }
}



/***************************************************************************************************************************
* eeprom interface
***************************************************************************************************************************/

U32 eeprom_read_bytes(U32 Address, U32 * Data, U32 Length)
{
    /**
    Eeprom data order: Little Endian
    MSB LSB
    */

    U32 eeprom_data =0, eeprom_code =0, i;
    U8 data;

    if( (Length < 1) || (Length > 4) )
    {
        return 1;
    }

    for(i=0; i< Length; i++)
    {
        eeprom_code += eeprom_read_byte(Address + i, &data);

        if(eeprom_code == 0)
        {
            eeprom_data |= (data << 8*(Length - (i+1)) );
        }
    }

    /**
    do not touch target data pointer if
    at least one eeprom read failed
    */
    if(eeprom_code)
    {
        return eeprom_code;
    }

    *Data= eeprom_data;

    return 0;
}


U32 eeprom_write_bytes(U32 Address, U32 Data, U32 Length)
{
    U32 eeprom_code =0, i;

    if( (Length < 1) || (Length > 4) )
    {
        return 1;
    }

    for(i=0; i< Length; i++)
    {
        //data order: Little Endian MSB LSB
        eeprom_code += eeprom_write_byte(Address + i, (U8) (Data >> 8*(Length - (i+1))) );
    }

    return eeprom_code;

}



/**
* Write to the EEPROM only where needed to save write cycles
*/
U32 eeprom_update(U32 Address, U32 Data)
{
    U32 eeprom_code;
    U8 eeprom_data;

    //read current data
    eeprom_code= eeprom_read_byte(Address, &eeprom_data);

    //read return code
    if(eeprom_code != 0)
    {
        return eeprom_code;
    }

    if(eeprom_data == Data)
    {
        //no need to write anything
        return 0;
    }

    /**
    else
    continue with writing
    */

    //write to eeprom
    eeprom_code= eeprom_write_byte(Address, Data);

    //write return code
    if(eeprom_code != 0)
    {
        return eeprom_code;
    }

    //read back written data
    eeprom_code= eeprom_read_byte(Address, &eeprom_data);

    //read return code
    if(eeprom_code != 0)
    {
        return eeprom_code;
    }

    //verify written data
    if(eeprom_data == Data)
    {
        //success
        return 0;
    }
    else
    {
        return EE_VERIFICATION;
    }

}


U32 eeprom_update_bytes(U32 Address, U32 Data, U32 Length)
{
    U32 eeprom_code =0, i;

    if( (Length < 1) || (Length > 4) )
    {
        return 1;
    }

    for(i=0; i< Length; i++)
    {
        //data order: Little Endian MSB LSB
        eeprom_code += eeprom_update(Address + i, (U8) (Data >> 8*(Length - (i+1))) );
    }

    return eeprom_code;
}








