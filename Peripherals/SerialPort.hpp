/*
 * SerialPort.hpp
 *
 *  Created on: 27 ene. 2021
 *  Author: Manuel Andres Herrera Juárez
 *  Ciudad de México, México
 */

#ifndef PERIPHERALS_SERIALPORT_HPP_
#define PERIPHERALS_SERIALPORT_HPP_

#include <Util/Print.hpp>
#include <stdint.h>
#include <stdarg.h>

#define SERIAL_INTEGER_DECIMAL  0x00
#define SERIAL_INTEGER_BINARY   0x01
#define SERIAL_INTEGER_OCTAL    0x02
#define SERIAL_INTEGER_HEX      0x03
#define SERIAL_INTEGER_SIGNED   0x00
#define SERIAL_INTEGER_UNSIGNED 0x10

#define SERIALPORT_UART0    0
#define SERIALPORT_UART1    1
#define SERIALPORT_UART2    2
#define SERIALPORT_UART3    3
#define SERIALPORT_UART4    4
#define SERIALPORT_UART5    5
#define SERIALPORT_UART6    6
#define SERIALPORT_UART7    7

class SerialPort:public Print{
    public:
        SerialPort();
        SerialPort(uint32_t, uint8_t=0);

        void open();
        void close();

        char read();
        char* readline(char* const, uint8_t = 20, bool = true);

    private:
        uint8_t UART;
        uint32_t* PORT_R;
        uint32_t* UART_R;
        uint32_t* UART_FSTAT_R;
        uint32_t baud;

        char internalBuffer[23];
        inline int assertValidUART();

        PrintStatus write(uint8_t c, uint8_t flags=0) override;
        PrintStatus write(const char* txt, int n, uint8_t flags)override;

};



#endif /* PERIPHERALS_SERIALPORT_HPP_ */
