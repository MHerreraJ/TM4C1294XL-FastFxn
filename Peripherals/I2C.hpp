/*
 * I2C.hpp
 *
 *  Created on: 28 ene. 2021
 *  Author: Manuel Andres Herrera Juárez
 *  Ciudad de México, México
 */

#ifndef PERIPHERALS_I2C_HPP_
#define PERIPHERALS_I2C_HPP_
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

#define I2C_MASTER  0
#define I2C_SLAVE   1
#define I2C_INVALID 2

#define I2C_WRITE_OK            0
#define I2C_WRITE_ERROR         1
#define I2C_WRITE_ERROR_MODE    2
#define I2C_WRITE_ERROR_NONE    3

#define I2C_READ_OK         0
#define I2C_READ_ERROR      1
#define I2C_READ_ERROR_MODE 2
#define I2C_READ_ERROR_NONE 3
#define I2C_READ_ERROR_WADD 4

class I2C:public Print{
    public:
        I2C(); //Default 400kHz, I2C0
        I2C(uint32_t, uint8_t=0, uint8_t=0); //speed, port, mode

        void open();
        void close();

        uint8_t writeTo(uint8_t, uint8_t);
        uint8_t writeTo(uint8_t, uint8_t, const void*, uint8_t=1);

        uint8_t read(uint8_t, void*, uint8_t=1);
        uint8_t read(uint8_t, uint8_t, void*, uint8_t=1);


    private:
        uint8_t mode;
        uint32_t freq;
        uint8_t I2Cx;

        volatile uint32_t* PORT_R;
        volatile uint32_t* I2C_R;
        volatile uint32_t* I2C_STATUS_R;

        PrintStatus write(const char* byt, int n, uint8_t flags) override;
        PrintStatus write(uint8_t c, uint8_t flags=0) override;


        inline bool assertValidI2CSpeed();

};


#endif /* PERIPHERALS_I2C_HPP_ */
