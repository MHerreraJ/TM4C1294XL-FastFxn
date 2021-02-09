/*
 * I2C.cpp
 *
 *  Created on: 28 ene. 2021
 *  Author: Manuel Andres Herrera Juárez
 *  Ciudad de México, México
 */


#include <stdio.h>

#include <../inc/tm4c1294ncpdt.h>
#include <Peripherals/Board.hpp>
#include <Peripherals/I2CMaster.hpp>
#include "../driverlib/sysctl.h"
#include "../driverlib/rom_map.h"

static const uint8_t I2C_SCLIO_B[] = {2, 0, 0, 4, 6, 0, 6, 4, 2, 0}; //SCL Bit, SDA = SCLIO + 1, EXCEPT I2C_2, store SDA2 = SCL2-1
static const uint8_t I2C_PORT_OFF[] = {GPIO_PORTB_OFF, GPIO_PORTG_OFF, GPIO_PORTL_OFF,
                                      GPIO_PORTK_OFF, GPIO_PORTK_OFF, GPIO_PORTB_OFF,
                                      GPIO_PORTA_OFF, GPIO_PORTA_OFF, GPIO_PORTA_OFF,
                                      GPIO_PORTA_OFF}; //GPIO Base offset
/**
 * Default I2CMaster Constructor
 * I2C0 selected, 100kHz
 */
I2CMaster::I2CMaster(){
    I2Cx = 0;
    freq = 100000;
    mode = 0;
    open();
}

/**
 * I2CMaster Constructor
 * @param speed is the bus frequency
 * @param i2cx is the i2c module to use (0 to 9)
 */
I2CMaster::I2CMaster(uint32_t speed, uint8_t i2cx){
    I2Cx = i2cx;
    freq = speed;
    mode = 0;
    open();
}

/**
 * Sets the external device address
 * @param addr is the 7-bit external device address which this instance will communicate
 */
void I2CMaster::setAddress(uint8_t addr){
    slaveAddress = (addr << 1) & 0xFE;
}


/**
 * Open the I2C selected bus
 * Must be called before write or read attempts
 */
void I2CMaster::open(){
    if(I2Cx > 9  || !assertValidI2CSpeed()){
        mode = 1;
        return;
    }

    PORT_R = (uint32_t*)(GPIO_PORT_BASE + (I2C_PORT_OFF[I2Cx] << 12)); //Set pointer to GPIO Port Base Register
    I2C_R = (uint32_t*)(GET_I2C_BASE_R(I2Cx) + (I2Cx << 12)); //Get I2Cx Base Register
    I2C_STATUS_R = I2C_R + (0x004 >> 2);

    SYSCTL_RCGCI2C_R |= (1 << I2Cx); //Enable I2Cx clock
    SYSCTL_RCGCGPIO_R |= (1 << I2C_PORT_OFF[I2Cx]);    //Enable GPIO Port for I2C SDA, SCL Signals

    while((SYSCTL_RCGCI2C_R & (1<<I2Cx)) == 0 || (SYSCTL_RCGCGPIO_R & (1<<I2C_PORT_OFF[I2Cx])) == 0);


    *(PORT_R + (0x400>>2)) &= ~(0x03 <<  I2C_SCLIO_B[I2Cx]);    //Select SDA, SCL as inputs
    *(PORT_R + (0x420>>2)) |= 0x03 <<  I2C_SCLIO_B[I2Cx];       //Select GPIO SDA, SCL pins alternative function
    *(PORT_R + (0x528>>2)) &= ~(0x03 <<  I2C_SCLIO_B[I2Cx]);    //Disable GPIO Tx, Rx analog function

    //Configure SDA as Open-drain
    if(I2Cx != 2){
        *(PORT_R + (0x50C >> 2)) |= 1 << (I2C_SCLIO_B[I2Cx]+1); //SDAIO = SCLIO + 1
    }else{
        *(PORT_R + (0x50C >> 2)) |= 1 << I2C_SCLIO_B[I2Cx]; //Except SDA2
    }

    *(PORT_R + (0x52C >> 2)) |= 0x22 <<  (I2C_SCLIO_B[I2Cx]<<2);   //Port Mux SDA, SCL to I2C (Alternate function 2)
    *(PORT_R + (0x51C >> 2)) |= 0x03 << I2C_SCLIO_B[I2Cx];         //Enable SDA,SCL Pins

    *(I2C_R + (0x020 >> 2)) |= 0x10; //Set I2C as Master or Slave
    *(I2C_R + (0x00C >> 2)) = ((CPU_FREQUENCY/(20*freq) -1) & 0x7F) | (freq <= 1000000 ? 0x00 : 0x80); //Set SCL Speed TPR Val
}

//!TODO
/**
 * Close the I2C selected bus
 * Must be called for free reserved I2Cx GPIOs
 */
void I2CMaster::close(){
    if(mode) return;
}

/**
 * Private: Get I2C transaction result
 *
 * @param stopIfError if true, generate stop condition if error detected
 * @return I2C_WRITE_ERROR if transaction failed, otherwise I2C_WRITE_OK
 */
uint8_t I2CMaster::I2CTransactionResult(bool stopIfError){
    MAP_SysCtlDelay(100); //!TODO change delay
    while(((*I2C_STATUS_R) & 0x01) != 0); //Wait until controller idle
    //Check Error bit
    if(((*I2C_STATUS_R) & 0x02) != 0){ //Error found
        if(stopIfError && (((*I2C_STATUS_R) & 0x10) == 0)){ //I2C Controller won arbitration
            *I2C_STATUS_R = 0x04; //Generate stop condition
        }
        return I2C_WRITE_ERROR;
    }
    return I2C_WRITE_OK;
}


/**
 * Read n bytes from I2C device (address previously saved), and store them in
 * an output buffer.
 *
 * @param out_r points the external buffer that store read values
 * @param len is the quantity of bytes to receive
 * @param endRx if true, adds a '\0' value at the end of the buffer
 * @return I2C transaction result
 */
uint8_t I2CMaster::read(void* out_r, uint8_t len, bool endRx){
    if(mode) return I2C_READ_ERROR;
    if(len == 0)return I2C_READ_ERROR;
    uint8_t* out = (uint8_t*)out_r;

    *I2C_R = slaveAddress | 0x01; //Set Slave Address - Receive Mode

    if(len == 1){
        *I2C_STATUS_R = 0x07; //Init single byte receive
        if(I2CTransactionResult() != I2C_READ_OK) return I2C_READ_ERROR;
        *out = (uint8_t)(*(I2C_R + (0x008 >> 2)));
        return I2C_READ_OK;
    }else{
        *I2C_STATUS_R = 0x0B;

        while(1){
            if(I2CTransactionResult(true) != I2C_READ_OK) return I2C_READ_ERROR;
            *(out++) = *(I2C_R + (0x008 >> 2)); //Read Rx data byte
            if(--len > 1){
                *I2C_STATUS_R = 0x09; //Continue Rx with acknowledge
            }else break;
        }
        *I2C_STATUS_R = 0x05; //End last byte transaction

        if(I2CTransactionResult() != I2C_READ_OK) return I2C_READ_ERROR;
        *out = (uint8_t)(*(I2C_R + (0x008 >> 2))); //Read last Rx data byte
        if(endRx) *(out+1) = '\0';
        return I2C_READ_OK;
    }
}

/**
 * Send command and read n bytes from I2C device (address previously saved), and store them in
 * an output buffer. Useful for reading from external device registers
 *
 * @param dev_reg is the command or memory address to read in the external device
 * @param out_r points the external buffer that store read values
 * @param len is the quantity of bytes to receive
 * @return I2C transaction result
 */
PrintStatus I2CMaster::readFrom(uint8_t dev_reg, void* out_r, uint8_t len){
    if(mode)  return I2C_READ_ERROR;
    if(len == 0)return I2C_READ_ERROR;
    uint8_t* out = (uint8_t*)out_r;

    *I2C_R = slaveAddress; //Set slave address, Tx Mode for subregister
    *(I2C_R + (0x008 >> 2)) = dev_reg; //Set device register to MDR
    while(((*I2C_STATUS_R) & 0x40) != 0); //Wait until bus free
    *I2C_STATUS_R = 0x03; //Init single byte transmit, hold master enable
    if(I2CTransactionResult() != I2C_READ_OK) return I2C_READ_ERROR;

    *I2C_R = slaveAddress | 0x01; //Set slave address, Rx Mode
    *I2C_STATUS_R = 0x0B; //Enable I2C, Start condition, with acknowledge

    while(1){
        if(I2CTransactionResult(true) != I2C_READ_OK) return I2C_READ_ERROR;

        *(out++) = *(I2C_R + (0x008 >> 2)); //Read Rx data byte
        if(--len > 1){
            *I2C_STATUS_R = 0x09; //Continue Rx with acknowledge
        }else break;
    }
    *I2C_STATUS_R = 0x05; //End I2C Transaction

    if(I2CTransactionResult() != I2C_READ_OK) return I2C_READ_ERROR;
    *out = (uint8_t)(*(I2C_R + (0x008 >> 2))); //Read last Rx data byte
    return I2C_READ_OK;
}

/**
 * Overrides Print class write method
 *
 * @param data is the byte to sent
 * @param flags specifies the transaction state (START, CONTINUE, STOP)
 * @return I2C Transaction result
 */
PrintStatus I2CMaster::write(uint8_t data, uint8_t flags){
    if(mode) return I2C_WRITE_ERROR;

    //Single byte transaction
    if((flags & PRINT_WR_MODE) == PRINT_WR_MOD_SINGLE){
        *I2C_R = slaveAddress;          //Set Slave Address - Transmit
        *(I2C_R + (0x008 >> 2)) = data; //Set Desired data to MDR
        *I2C_STATUS_R = 0x07;           //Init single byte transmit
        return I2CTransactionResult();
    }

    //Multiple bytes transaction mode
    if(flags & PRINT_WR_START){     //First byte in transaction?
        *I2C_R = slaveAddress;      //Then Set Slave Address - Transmit Mode
    }
    *(I2C_R + (0x008 >> 2)) = data; //Set Desired data to MDR
    *I2C_STATUS_R = flags & PRINT_WR_CTL; //Set transaction

    return I2CTransactionResult((flags & PRINT_WR_STOP) != PRINT_WR_STOP);
}

/**
 * Overrides Print class write method
 *
 * @param txt is the byte array pointer to send
 * @param n is the number of the bytes to write
 *       if n < 0, write until a 0 is found in the array
 * @param flags specifies the transaction state (START, CONTINUE, STOP)
 * @return I2C Transaction result
 */
PrintStatus I2CMaster::write(const char* txt, int n, uint8_t flags){
    if(mode) return _PRINT_STATUS_ERROR;
    if((*txt == 0 && n < 0) || n == 0)   return write(0, flags);        //Length 0 string
    if((*(txt+1) == 0 && n < 0) || n == 1) return write(*(txt), flags); //Length 1 string

    uint8_t count = 0;
    PrintStatus status = write(*(txt++), flags & ~PRINT_WR_STOP); //Prevent stop, since length >= 2

    flags &= ~PRINT_WR_START; //Start condition handled, remove it

    while((n < 0 && *txt) || count < n){
        if(status &_PRINT_STATUS_ERROR) return status;
        if(((n < 0) && (*(txt+1) == 0)) || (count == n-1)){ //Is Last byte?
            status = write(*txt, flags);
        }else{  //Prevent stop transaction, since isn't last byte
            status = write(*txt, flags & ~PRINT_WR_STOP);
        }
        count++; txt++;
    }

    return status;
}

/**
 * Asserts is a valid I2CSpeed
 * (Tiva TM4C1294 doesn't limit for specific bus frequency values, but this ones are the most common)
 * @return true if valid frequency
 */
inline bool I2CMaster::assertValidI2CSpeed(){
    switch(freq){
        case 100000:    //100KHz
        case 400000:    //400KHz
        case 1000000:   //1MHz
        case 3330000:   //3.33MHz
            return true;
    }
    return false;
}
