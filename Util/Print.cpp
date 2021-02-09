/*
 * Print.cpp
 *
 *  Created on: 2 feb. 2021
 *  Author: Manuel Andres Herrera Juárez
 *  Ciudad de México, México
 */

#include <Util/Print.hpp>
#include <Util/Format.h>
#include <stdio.h>

#define _PRINT_NUMBER_FLOAT     'f'
#define _PRINT_NUMBER_BIN_BASE  'b'
#define _PRINT_NUMBER_DEC_BASE  'd'
#define _PRINT_NUMBER_HEX_BASE  'x'

/**
 * Print single character
 *
 * @param c is the character to print
 * @return print attempt result (ERROR or OK)
 */
PrintStatus Print::print(char c){
    return write(c, PRINT_WR_MOD_SINGLE);
}

/**
 * Print n characters of a string
 *
 * @param txt is the string to print
 * @param n indicates the number of characters to print
 *      if n <0 then print all characters of the string
 * @return print attempt result (ERROR or OK)
 */

PrintStatus Print::print(const char* txt, int n){
    return write(txt, n, PRINT_WR_CTL_SNGL_TRXN);
}


/**
 * Print n characters of a string, followed by end of line ("\r\n")
 *
 * @param txt is the string to print
 * @param n indicates the number of characters to print
 *      if n <0 then print all characters of the string
 * @return print attempt result (ERROR or OK)
 */
PrintStatus Print::println(const char* txt, int n){
    if( write(txt, n, PRINT_WR_CTL_INIT_TRXN) != _PRINT_STATUS_OK){
        return _PRINT_STATUS_ERROR;
    }
    return write("\r\n", -1, PRINT_WR_CTL_END_TRXN);
}


/**
 * Print formatted string
 *
 * @param format C string containing the text to be printed,
 * optionally it can contain embedded format specifiers that are
 * replaced by the values specified in the additional arguments
 *  - Format Specifier prototype:
 *      %[flags]specifier
 *  - Available specifiers:
 *      d for signed integer
 *      x for signed hexadecimal integer
 *      b for signed binary integer
 *      u for unsigned integer
 *      ux for unsigned hexadecimal integer (32 bits)
 *      ub for unsigned binary integer (32 bits)
 *      s for string of characters
 *      % A % followed by another % char will write a single %
 * @param ... are the variable arguments to print
 * @return print attempt result (ERROR or OK)
 * */
PrintStatus Print::printf(const char* format, ...){
    va_list args;
    va_start(args, format);

    PrintStatus status;
    uint8_t startTrxn = PRINT_WR_CTL_INIT_TRXN;

    while(*format){ //While current char != '\0' (End of string)
        if(*format == '%' ){ //Possible format specifier
            format++;

            int argFlag = isDigit(*format) ? 0 : -1;
            union{int i; unsigned int ui; float f;} number;
            uint8_t printTrxn = startTrxn | (*(format+1)== 0 ? PRINT_WR_CTL_END_TRXN : PRINT_WR_CTL_CONT_TRXN); //Is the last argument?

            //Argument flag found
            while(isDigit(*format)){
                argFlag = 10*argFlag + getNumber(*(format++));
            }

            switch(*format){
                case '\0':{
                    status = write('%', printTrxn | PRINT_WR_STOP); //Print single %, with stop condition
                    if(argFlag >= 0) status = _PRINT_STATUS_ERROR;
                }break;
                case 'd': case 'x': case 'b':{      //Print signed integer
                    number.i = va_arg(args, int);
                    status = printNumber(number.ui, *format, true, printTrxn);
                }break;
                case 'f':{                          //Print float
                    number.f = va_arg(args, float);
                    status = printNumber(number.ui, *format, true, printTrxn);
                }break;
                case '%':  case 'c': {              //Print single char
                    char car = *format != 'c' ? *format : (char)va_arg(args, int);
                    status = write(car,  printTrxn);
                }break;
                case 's':{                          //Print string
                    status = write((const char*)va_arg(args, const char*), argFlag, printTrxn);
                }break;
                case 'u':{                          //Print unsigned integer
                    number.ui = va_arg(args, unsigned int);
                    if(*(format+1)=='x'){           //Print unsigned hexadecimal
                        status = printNumber(number.ui, _PRINT_NUMBER_HEX_BASE, false, printTrxn);
                        format++;   //'x' read, increment pointer
                    }else if(*(format+1) == 'b'){   //Print unsigned binary
                        status = printNumber(number.ui, _PRINT_NUMBER_BIN_BASE, false, printTrxn);
                        format++;   //'b' read, increment pointer
                    }else{                          //Print unsigned decimal
                        status = printNumber(number.ui, _PRINT_NUMBER_DEC_BASE, false, printTrxn);
                    }
                }break;
                default:{                           //Bad formatted string
                    write('%', printTrxn | PRINT_WR_STOP);
                    status = _PRINT_STATUS_ERROR;
                }break;
            }
        }else{
            uint8_t printTrxn = startTrxn | (*(format+1)== 0 ? PRINT_WR_CTL_END_TRXN : PRINT_WR_CTL_CONT_TRXN);
            status = write(*(format), printTrxn);
        }

        format++;
        startTrxn = PRINT_WR_CTL_CONT_TRXN;
        if(status != _PRINT_STATUS_OK) break;
    }

    va_end(args);
    return status;
}

/**
 * Internal method for printing binary or hexadecimal unsigned integers
 *
 * @param i is the integer value to print
 * @param iType specifies the format
 *      - _PRINT_NUMBER_HEX_BASE hexadecimal format (8 bits packet), Example: i=26 -> 0x1A; i=256 -> 0x0100
 *      - _PRINT_NUMBER_BIN_BASE binary format (4 bits packet), Example: i=9 -> 0b1001; i=28 -> 0b00011100
 * @param flags are the print configurations handled by the write methods
 * overridden in inherited classes
 * @return print attempt result (ERROR or OK)
 */
PrintStatus Print::printBinOrHex(unsigned int i, uint8_t iType, uint8_t flags){
    PrintStatus status = write('0', flags & ~PRINT_WR_STOP); //Prevent stop transaction
    if(status != _PRINT_STATUS_OK) return status;

    int groupSize, binCount, iShift, mask, bitShift, tMask, tShift, rShift;

    if(iType == _PRINT_NUMBER_HEX_BASE){
        groupSize = 2; binCount =  2*sizeof(unsigned int);
        iShift = (sizeof(unsigned int) - 1) << 3; mask = 0xFF << iShift;
        bitShift = 8; tMask = 0xF0 << iShift; tShift = 4; rShift = 4;
    }else{
        groupSize = 4; binCount = 8*sizeof(unsigned int);
        iShift = 4 + ((sizeof(unsigned int) - 1) << 3); mask = 0xF << iShift;
        bitShift = 4; tMask = 0x8 << iShift; tShift = 3; rShift = 1;
    }

    flags &= ~PRINT_WR_START; //Prevent start transaction since handled by first write

    status = write(iType, PRINT_WR_CTL_CONT_TRXN); //Continue transaction
    if(status != _PRINT_STATUS_OK) return status;

    //If i value equals zero, print as many zeros as the format groupsize
    if(i == 0) {
        while(groupSize-- > 1){
            status = write('0', PRINT_WR_CTL_CONT_TRXN); //Continue transaction
            if(status != _PRINT_STATUS_OK) return status;
        }
        return write('0', flags);
    }

    //Find group containing the MSB
    while((i & mask) == 0x00){
        i <<= bitShift;
        binCount -= groupSize;
    }

    for(; binCount > 0; binCount--){
        char toWrite = (char)((i & tMask) >> (iShift+tShift)); // Get number
        toWrite += toWrite <= 9 ? '0' : 'A' - 10; //Get hex char
        i <<= rShift;

        if(binCount - 1 == 0){ //Last char to print
            status = write(toWrite, flags);
        }else{
            status = write(toWrite, PRINT_WR_CTL_CONT_TRXN);
        }
    }
    return status;
}

/**
 * Function for printing unsigned base 10 integer
 * @param i is the number to print
 * @param flags are the print configurations handled by the write methods
 * overridden in inherited classes
 * @return print attempt result (ERROR or OK)
 */
PrintStatus Print::printDecimal(unsigned int i, uint8_t flags){
    sprintf(buffer, "%u", i);
    return write(buffer, -1, flags);
}

/**
 * Function for printing float number
 * @param f is the number to print
 * @param flags are the print configurations handled by the write methods
 * overridden in inherited classes
 * @return print attempt result (ERROR or OK)
 */
PrintStatus Print::printFloat(float f, uint8_t flags){
    sprintf(buffer, "%0.4f", f);
    return write(buffer, -1, flags);
}

/**
 * Internal method that prints a number
 * @param i contains a 32-bit number
 * @param iType specify the number type:
 *      - _PRINT_NUMBER_DEC_BASE decimal integer type
 *      - _PRINT_NUMBER_HEX_BASE hexadecimal integer type
 *      - _PRINT_NUMBER_BIN_BASE binary integer type
 *      - _PRINT_NUMBER_FLOAT float type //TODO
 * @param isSigned indicates if print the number as a signed type
 * @return print attempt result (ERROR or OK)
 */
PrintStatus Print::printNumber(unsigned int i, uint8_t iType, bool iSigned, uint8_t flags){
    union {unsigned int ui; int i; float f;} n; //32-bit number
    PrintStatus status;
    n.ui = i;

    if(iType == _PRINT_NUMBER_FLOAT){
        return printFloat(n.f, flags);
    }

    //If signed integer is less than 0 then print '-' character
    if(iSigned && (n.i < 0)){
        status = write('-', flags & ~PRINT_WR_STOP); //Prevent stop transaction
        flags &= ~PRINT_WR_START; //Prevent for start trxn again

        if(status != _PRINT_STATUS_OK)   return status;
        n.i = -n.i; //Since '-' printed, invert sign for assert unsigned int type
    }

    //Map integer to print method
    if(iType == _PRINT_NUMBER_BIN_BASE){
        return printBinOrHex(n.ui, _PRINT_NUMBER_BIN_BASE, flags);
    }else if(iType == _PRINT_NUMBER_HEX_BASE){
       return printBinOrHex(n.ui, _PRINT_NUMBER_HEX_BASE, flags);
    }else{
        return printDecimal(n.ui, flags);
    }
}

