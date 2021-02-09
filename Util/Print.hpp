/*
 * Print.hpp
 *
 *  Created on: 2 feb. 2021
 *  Author: Manuel Andres Herrera Juárez
 *  Ciudad de México, México
 */

#ifndef UTIL_PRINT_HPP_
#define UTIL_PRINT_HPP_

#include <stdint.h>
#include <stdarg.h>


// PRINT FLAG DESCRIPTION (8 bits)
// Bit 3..0: WRITE CONTROL (Only for multiple bytes mode, otherwise ignored)
//      Bit 0: ENABLE    -Write if enabled
//      Bit 1: START     -Generate start condition (Use if 1st byte)
//      Bit 2: STOP      -Generate stop condition (Use if last byte)
// Bit 4: WRITE MODE
//          - 0 SINGLE      -1 byte per transaction
//          - 1 MULTIPLE    - Multiple bytes per transaction

#define PRINT_WR_MODE   0x10
#define PRINT_WR_MOD_SINGLE     0x00
#define PRINT_WR_MOD_MULTIPLE   0x10

#define PRINT_WR_CTL    0x07
#define PRINT_WR_EN     0x01
#define PRINT_WR_START  0x02
#define PRINT_WR_STOP   0x04

#define PRINT_WR_CTL_SNGL_TRXN  (PRINT_WR_MOD_MULTIPLE | PRINT_WR_CTL)
#define PRINT_WR_CTL_INIT_TRXN  (PRINT_WR_MOD_MULTIPLE | PRINT_WR_EN | PRINT_WR_START)
#define PRINT_WR_CTL_END_TRXN   (PRINT_WR_MOD_MULTIPLE | PRINT_WR_EN | PRINT_WR_STOP)
#define PRINT_WR_CTL_CONT_TRXN  (PRINT_WR_MOD_MULTIPLE | PRINT_WR_EN)


#define _PRINT_STATUS_OK        0x00
#define _PRINT_STATUS_ERROR     0x01

typedef uint8_t PrintStatus;

class Print{
    public:
        Print(){}

        PrintStatus print(char c);
        PrintStatus print(const char*, int=-1);
        PrintStatus println(const char*, int=-1);
        PrintStatus printf(const char* format, ...);

    private:
        char buffer[15];
        PrintStatus printFloat(float, uint8_t);
        PrintStatus printDecimal(unsigned int, uint8_t);
        PrintStatus printBinOrHex(unsigned int, uint8_t, uint8_t);
        PrintStatus printNumber(unsigned int, uint8_t, bool, uint8_t);

    protected:
        virtual PrintStatus write(const char* byt, int n, uint8_t flags) = 0;
        virtual PrintStatus write(uint8_t c, uint8_t flags) = 0;
};



#endif /* UTIL_PRINT_HPP_ */
