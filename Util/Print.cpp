/*
 * Print.cpp
 *
 *  Created on: 2 feb. 2021
 *  Author: Manuel Andres Herrera Juárez
 *  Ciudad de México, México
 */

#include <Util/Print.hpp>
#include <stdio.h>

#define _PRINT_NUMBER_FLOAT  0
#define _PRINT_NUMBER_BIN_BASE  2
#define _PRINT_NUMBER_DEC_BASE  10
#define _PRINT_NUMBER_HEX_BASE  16

/**
 * Print single character
 *
 * @param c is the character to print
 * @return print attempt result (ERROR or OK)
 */
PrintStatus Print::print(char c){
    return write(c, _PRINT_WR_SINGLE_F | _PRINT_WR_FIRSTBYTE_F);
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
    return write(txt, n, _PRINT_WR_MULTIPLE_F | _PRINT_WR_FIRSTBYTE_F | _PRINT_WR_ILAST_F);
}

/**
 * Print signed integer
 * @param i is the number to print
 * @param type (10 default) is the number base, only accept (2, 10 and 16 bases)
 * @return print attempt result (ERROR or OK)
 */
PrintStatus Print::print(int i, uint8_t type){
    union {unsigned int ui; int i;} n;
    n.i = i;
    return printNumber(n.ui, type, true, _PRINT_WR_MULTIPLE_F | _PRINT_WR_FIRSTBYTE_F | _PRINT_WR_ILAST_F);
}

/**
 * Print float type number
 * @param f is the number to print
 * @return print attempt result (ERROR or OK)
 */
PrintStatus Print::print(float f){
    union {unsigned int ui; float bf;} n;
    n.bf = f;
    return printNumber(n.ui, _PRINT_NUMBER_FLOAT, true, _PRINT_WR_MULTIPLE_F | _PRINT_WR_FIRSTBYTE_F | _PRINT_WR_ILAST_F);
}

/**
 * Print double type number
 * @param d is the number to print
 * @return print attempt result (ERROR or OK)
 */
PrintStatus Print::print(double d){
    union {unsigned int ui; float f;} n;
    n.f = (float)d; //Cast to float
    return printNumber(n.ui, _PRINT_NUMBER_FLOAT, true, _PRINT_WR_MULTIPLE_F | _PRINT_WR_FIRSTBYTE_F | _PRINT_WR_ILAST_F);
}


/**
 * Print a single character, followed by end of line ("\r\n")
 *
 * @param c is the character to print
 * @return print attempt result (ERROR or OK)
 */
PrintStatus Print::println(char c){
    if(write(c, _PRINT_WR_MULTIPLE_F | _PRINT_WR_FIRSTBYTE_F) == _PRINT_STATUS_ERROR){
        return _PRINT_STATUS_ERROR;
    }
    return write("\r\n", -1, _PRINT_WR_MULTIPLE_F | _PRINT_WR_MEDBYTE_F | _PRINT_WR_ILAST_F);
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
    if( write(txt, n, _PRINT_WR_MULTIPLE_F | _PRINT_WR_FIRSTBYTE_F | _PRINT_WR_ILAST_F) == _PRINT_STATUS_ERROR){
        return _PRINT_STATUS_ERROR;
    }
    return write("\r\n", -1, _PRINT_WR_MULTIPLE_F | _PRINT_WR_MEDBYTE_F | _PRINT_WR_ILAST_F);
}

/**
 * Print signed integer
 * @param i is the number to print, followed by end of line ("\r\n")
 * @param type (10 default) is the number base, only accept (2, 10 and 16 bases)
 * @return print attempt result (ERROR or OK)
 */
PrintStatus Print::println(int i,uint8_t type){
    union {unsigned int ui; int i;} n;
    n.i = i;
    if(printNumber(n.ui, type, true, _PRINT_WR_MULTIPLE_F | _PRINT_WR_FIRSTBYTE_F | _PRINT_WR_ILAST_F)== _PRINT_STATUS_ERROR){
        return _PRINT_STATUS_ERROR;
    }
    return write("\r\n", -1, _PRINT_WR_MULTIPLE_F | _PRINT_WR_MEDBYTE_F | _PRINT_WR_ILAST_F);
}

/**
 * Print float type number, followed by end of line ("\r\n")
 * @param f is the number to print
 * @return print attempt result (ERROR or OK)
 */
PrintStatus Print::println(float f){
    union {unsigned int ui; float bf;} n;
    n.bf = f;
    if(printNumber(n.ui, _PRINT_NUMBER_FLOAT, true, _PRINT_WR_MULTIPLE_F | _PRINT_WR_FIRSTBYTE_F | _PRINT_WR_ILAST_F) == _PRINT_STATUS_ERROR){
            return _PRINT_STATUS_ERROR;
    }
    return write("\r\n", -1, _PRINT_WR_MULTIPLE_F | _PRINT_WR_MEDBYTE_F | _PRINT_WR_ILAST_F);
}

/**
 * Print double type number, followed by end of line ("\r\n")
 * @param d is the number to print
 * @return print attempt result (ERROR or OK)
 */
PrintStatus Print::println(double d){
    union {unsigned int ui; float f;} n;
    n.f = (float)d; //Cast to float
    if(printNumber(n.ui, _PRINT_NUMBER_FLOAT, true, _PRINT_WR_MULTIPLE_F | _PRINT_WR_FIRSTBYTE_F | _PRINT_WR_ILAST_F) == _PRINT_STATUS_ERROR){
        return _PRINT_STATUS_ERROR;
    }
    return write("\r\n", -1, _PRINT_WR_MULTIPLE_F | _PRINT_WR_MEDBYTE_F | _PRINT_WR_ILAST_F);
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
    int p_count = 0; //Count of characters/values printed
    PrintStatus status;

    while(*format){ //While current char != '\0' (End of string)
        if(*format == '%' ){ //Possible format specifier
            int numberPrintType = 0;
            union{int i; unsigned int ui;} number;
            bool isSigned = true;

            uint8_t lastArg = _PRINT_WR_MULTIPLE_F | (*(++format+1) == 0 ? _PRINT_WR_ILAST_F  : 0); //Is the last argument?

            switch(*(format)){
                case '\0': break; //Do nothing
                case 'd': number.i = va_arg(args, int); numberPrintType = _PRINT_NUMBER_DEC_BASE; break;
                case 'x': number.i = va_arg(args, int); numberPrintType = _PRINT_NUMBER_HEX_BASE; break;
                case 'b': number.i = va_arg(args, int); numberPrintType = _PRINT_NUMBER_BIN_BASE; break;
                //case 'f': print((float)va_arg(args, float)); break;
                case '%':  case 'c': {
                    char car = *format != 'c' ? *format : (char)va_arg(args, int);
                    status = write(car,  (p_count == 0? _PRINT_WR_FIRSTBYTE_F:_PRINT_WR_MEDBYTE_F)  | lastArg);
                }break;
                case 's':{
                    status = write((const char*)va_arg(args, const char*), -1, (p_count == 0? _PRINT_WR_FIRSTBYTE_F:_PRINT_WR_MEDBYTE_F)  | lastArg);
                }break;
                case 'u':{
                    isSigned = false;
                    number.ui = va_arg(args, unsigned int);
                    if(*(format+1)=='x'){
                        numberPrintType = _PRINT_NUMBER_HEX_BASE; format++; //Readed 'x', increment format pointer
                    }else if(*(format+1) == 'b'){
                        numberPrintType = _PRINT_NUMBER_BIN_BASE; format++; //Readed 'b', increment format pointer
                    }else{
                        numberPrintType = _PRINT_NUMBER_DEC_BASE; //Default base 10 integer
                    }
                }break;
            }
            if(numberPrintType != 0){
                lastArg = *(format+1) == 0 ? _PRINT_WR_ILAST_F  : 0; //Is the last character
                if(p_count == 0){
                    status = printNumber(number.ui, numberPrintType, isSigned,
                            _PRINT_WR_MULTIPLE_F | _PRINT_WR_FIRSTBYTE_F | lastArg);
                }else{
                    status = printNumber(number.ui, numberPrintType, isSigned,
                            _PRINT_WR_MULTIPLE_F | _PRINT_WR_MEDBYTE_F | lastArg);
                }
            }
        }else{
            uint8_t flg = (*(format+1) == 0? _PRINT_WR_ILAST_F  : 0) | ((p_count == 0? _PRINT_WR_FIRSTBYTE_F:_PRINT_WR_MEDBYTE_F));
            status = write(*(format), flg);
        }

        if(status == _PRINT_STATUS_ERROR) break;
        format++; p_count++;
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
    PrintStatus status = write('0', (flags & ~ _PRINT_WR_POSBYTE_M) | _PRINT_WR_FIRSTBYTE_F);
    if(status == _PRINT_STATUS_ERROR) return status;

    char sep;
    int groupSize, binCount, iShift, mask, bitShift, tMask, tShift, rShift;

    if(iType == _PRINT_NUMBER_HEX_BASE){
        sep = 'x'; groupSize = 2; binCount =  2*sizeof(unsigned int);
        iShift = (sizeof(unsigned int) - 1) << 3; mask = 0xFF << iShift;
        bitShift = 8; tMask = 0xF0 << iShift; tShift = 4; rShift = 4;
    }else{
        sep = 'b'; groupSize = 4; binCount = 8*sizeof(unsigned int);
        iShift = 4 + ((sizeof(unsigned int) - 1) << 3); mask = 0xF << iShift;
        bitShift = 4; tMask = 0x8 << iShift; tShift = 3; rShift = 1;
    }

    status = write(sep, _PRINT_WR_MULTIPLE_F | _PRINT_WR_MEDBYTE_F);
    if(status == _PRINT_STATUS_ERROR) return status;

    //If i value equals zero, print as many zeros as the format groupsize
    if(i == 0) {
        while(--groupSize > 1){
            status = write('0', _PRINT_WR_MULTIPLE_F | _PRINT_WR_MEDBYTE_F);
            if(status == _PRINT_STATUS_ERROR) return status;
        }
        return write('0', (flags & ~_PRINT_WR_POSBYTE_M) | _PRINT_WR_LASTBYTE_F);
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
            status = write(toWrite, (flags & ~ _PRINT_WR_POSBYTE_M) | _PRINT_WR_LASTBYTE_F);
        }else{
            status = write(toWrite, _PRINT_WR_MULTIPLE_F | _PRINT_WR_MEDBYTE_F);
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
        //TODO
        return 0;
    }

    //If signed integer is less than 0 then print '-' character
    if(iSigned && (n.i < 0)){
        if((flags & _PRINT_WR_POSBYTE_M) == _PRINT_WR_FIRSTBYTE_F){
            flags |= _PRINT_WR_MEDBYTE_F;
            status = write('-', _PRINT_WR_FIRSTBYTE_F | _PRINT_WR_MULTIPLE_F);
        }else{
            status = write('-', _PRINT_WR_MEDBYTE_F | _PRINT_WR_MULTIPLE_F);
        }
        if(status == _PRINT_STATUS_ERROR)   return status;
        n.i = -n.i; //Since '-' printed, convert to positive number for assert unsigned int type
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

