/*
 * I2C.hpp
 *
 *  Created on: 28 ene. 2021
 *  Author: Manuel Andres Herrera Juárez
 *  Ciudad de México, México
 */

#ifndef PERIPHERALS_I2CMASTER_HPP_
#define PERIPHERALS_I2CMASTER_HPP_
#include <stdint.h>
#include <Util/Print.hpp>

#define I2C_I2C0    0
#define I2C_I2C1    1
#define I2C_I2C2    2
#define I2C_I2C3    3
#define I2C_I2C4    4
#define I2C_I2C5    5
#define I2C_I2C6    6
#define I2C_I2C7    7
#define I2C_I2C8    8
#define I2C_I2C9    9


#define I2C_WRITE_OK    _PRINT_STATUS_OK
#define I2C_WRITE_ERROR _PRINT_STATUS_ERROR

#define I2C_READ_OK    _PRINT_STATUS_OK
#define I2C_READ_ERROR _PRINT_STATUS_ERROR


// I2C FLAG WR DESCRIPTION (8 bits) Same as defined in PRINT, for external class use
#define I2C_WR_MODE           PRINT_WR_MODE
#define I2C_WR_MOD_SINGLE     PRINT_WR_MOD_SINGLE
#define I2C_WR_MOD_MULTIPLE   PRINT_WR_MOD_MULTIPLE

#define I2C_WR_CTL    PRINT_WR_CTL
#define I2C_WR_EN     PRINT_WR_EN
#define I2C_WR_START  PRINT_WR_START
#define I2C_WR_STOP   PRINT_WR_STOP

#define I2C_WR_CTL_SNGL_TRXN  PRINT_WR_CTL_SNGL_TRXN
#define I2C_WR_CTL_INIT_TRXN  PRINT_WR_CTL_INIT_TRXN
#define I2C_WR_CTL_END_TRXN   PRINT_WR_CTL_END_TRXN
#define I2C_WR_CTL_CONT_TRXN  PRINT_WR_CTL_CONT_TRXN



class I2CMaster:public Print{
    public:
        I2CMaster(); //Default 400kHz, I2C0
        I2CMaster(uint32_t, uint8_t=0); //speed, port, mode

        void open();
        void close();
        void setAddress(uint8_t);


        PrintStatus read(void*, uint8_t=1, bool=false);
        PrintStatus readFrom(uint8_t, void*, uint8_t=1);


        PrintStatus write(const char*, int, uint8_t) override;
        PrintStatus write(uint8_t c, uint8_t flags=0) override;


    private:
        uint8_t mode;
        uint32_t freq;
        uint8_t I2Cx;
        uint8_t slaveAddress;

        volatile uint32_t* PORT_R;
        volatile uint32_t* I2C_R;
        volatile uint32_t* I2C_STATUS_R;

        uint8_t I2CTransactionResult(bool=false);
        inline bool assertValidI2CSpeed();

};


#endif /* PERIPHERALS_I2CMASTER_HPP_ */
