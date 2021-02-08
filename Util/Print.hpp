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

#define _PRINT_WR_SINGLE_F    0x00
#define _PRINT_WR_MULTIPLE_F  0x01
#define _PRINT_WR_MULTINTER_F 0x02

#define _PRINT_WR_FIRSTBYTE_F 0x00
#define _PRINT_WR_MEDBYTE_F   0x04
#define _PRINT_WR_LASTBYTE_F  0x08

#define _PRINT_WR_IFIRSTBYE_F 0x00
#define _PRINT_WR_IMEDBYTE_F  0x10
#define _PRINT_WR_ILAST_F     0x20

#define _PRINT_WR_POSBYTE_M   0x0C
#define _PRINT_WR_IPOSBYTE_M  0x30

#define _PRINT_STATUS_NO_ERROR  0x00
#define _PRINT_STATUS_ERROR     0x01


typedef uint8_t PrintStatus;

class Print{
    public:
        Print(){}

        PrintStatus print(char c);
        PrintStatus print(const char*, int=-1);
        PrintStatus print(int,uint8_t=10);
        PrintStatus print(float);
        PrintStatus print(double);

        PrintStatus println(char);
        PrintStatus println(const char*, int=-1);
        PrintStatus println(int,uint8_t=0);
        PrintStatus println(float);
        PrintStatus println(double);

        PrintStatus printf(const char* format, ...);


    private:
        char buffer[20];
        PrintStatus printDecimal(unsigned int i, uint8_t flags);
        PrintStatus printBinOrHex(unsigned int i, uint8_t iType, uint8_t flags);
        PrintStatus printNumber(unsigned int i, uint8_t iType, bool iSigned, uint8_t flags);

    protected:
        virtual PrintStatus write(const char* byt, int n, uint8_t flags) = 0;
        virtual PrintStatus write(uint8_t c, uint8_t flags=0) = 0;
};



#endif /* UTIL_PRINT_HPP_ */
