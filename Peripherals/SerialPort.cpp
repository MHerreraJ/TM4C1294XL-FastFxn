/*
 * SerialPort.cpp
 *
 *  Created on: 27 ene. 2021
 *  Author: Manuel Andres Herrera Juárez
 *  Ciudad de México, México
 */

#include <stdio.h>

#include <../inc/tm4c1294ncpdt.h>
#include <Peripherals/Board.hpp>
#include <Peripherals/SerialPort.hpp>


static const uint32_t GPIO_PORT_BASE = 0x40058000;
static const uint32_t UART_BASE_REG = 0x4000C000;
static const uint8_t UART_PORT_OFF[] = {0, 1, 0, 0, 0, 2, 13, 2}; //GPIO Base offset
static const uint8_t UART_RXIO_B[] = {0, 0, 6, 4, 2, 6, 0, 4}; //RXIO Bit, TX = RXIO + 1

/* Default SerialPort constructor
 * UART0 Selected, at 9600 bauds*/
SerialPort::SerialPort(){
    this->UART = 0;
    this->baud = 9600;
    open();
}

/* SerialPort Constructor
 * \param baudrate is the speed at which UART will work
 * \param uart is the uart module to use (0 to 7)
 * */

SerialPort::SerialPort(uint32_t baudrate, uint8_t uart){
   this->UART = uart;
   this->baud = baudrate;
   open();
}


/* Print formatted integer
 * \param i is the integer to print
 * \param format are the flags for printing
 *    FORMAT OUTPUT (1..0 bits)
 *      - SERIAL_INTEGER_DECIMAL    = 0x0
 *      - SERIAL_INTEGER_BINARY     = 0x1
 *      - SERIAL_INTEGER_OCTAL      = 0x2
 *      - SERIAL_INTEGER_HEX        = 0x3
 *    INT TYPE (2 bit)
 *      - SERIAL_INTEGER_SIGNED     = 0x0
 *      - SERIAL_INTEGER_UNSIGNED   = 0x1
 * */
void SerialPort::print(int i, uint8_t format){
    if(!assertValidUART()) return;
    union{int i; uint32_t ui;} num;
    num.i = i;

    if(format & SERIAL_INTEGER_UNSIGNED){
        switch(format & 0x03){
            case SERIAL_INTEGER_DECIMAL:{
                sprintf(internalBuffer, "%u", num.ui);
            }break;
            case SERIAL_INTEGER_BINARY:{
                if(num.i == 0x00){
                    print("0000");
                    return;
                }
                int lastBit = 32;
                int bitCount;
                while((num.ui & 0x80000000) == 0x00){
                    num.ui <<= 1;
                    lastBit--;
                }

                bitCount = lastBit;
                if(lastBit%4 != 0){
                    bitCount +=  (4-lastBit%4);
                }

                for(;lastBit!=bitCount;bitCount--){
                    print('0');
                }

                while(lastBit>0){
                    print(((num.i & 0x80000000) == 0x00) ? '0':'1');
                    num.ui<<=1;
                    lastBit--;
                }
            }return;
            case SERIAL_INTEGER_OCTAL:{
            }break;
            case SERIAL_INTEGER_HEX:{
                sprintf(internalBuffer, "%X", num.ui);
            }break;
        }
    }else{
        switch(format & 0x03){
            case SERIAL_INTEGER_DECIMAL:{
                sprintf(internalBuffer, "%d", num.i);
            }break;
            case SERIAL_INTEGER_BINARY:{
                if(num.i == 0x00){
                    print("0000");
                    return;
                }
                int lastBit = 32;
                int bitCount;
                if(i<0){
                    print('-');
                    num.i = -i;
                }

                while((num.i & 0x80000000) == 0x00){
                    num.i <<= 1;
                    lastBit--;
                }

                bitCount = lastBit;
                if(lastBit%4 != 0){
                    bitCount +=  (4-lastBit%4);
                }

                for(;lastBit!=bitCount;bitCount--){
                    print('0');
                }

                while(lastBit>0){
                    print(((num.i & 0x80000000) == 0x00) ? '0':'1');
                    num.i<<=1;
                    lastBit--;
                }
            }return;
            case SERIAL_INTEGER_OCTAL:{
            }break;
            case SERIAL_INTEGER_HEX:{
                if(i<0){
                    num.i = -i;
                    sprintf(internalBuffer, "-%X", num.i);
                }else{
                    sprintf(internalBuffer, "%X", num.i);
                }
            }break;
        }
    }
    print(internalBuffer);
}

void SerialPort::print(long int i, uint8_t format){
    if(!assertValidUART()) return;
}

void SerialPort::print(float f){
    if(!assertValidUART()) return;
}

void SerialPort::print(double d){
    if(!assertValidUART()) return;
}

void SerialPort::print(char c){
    if(!assertValidUART()) return;
    while(((*UART_FSTAT_R) & 0x20) != 0x00); //Wait until last Tx char send
    *UART_R = c;
}


void SerialPort::print(const char* t){
    if(!assertValidUART()) return;
    while(*t){
        print(*(t++));
    }
}

void SerialPort::println(const char* t){
    if(!assertValidUART()) return;
    while(*t){
        print(*(t++));
    }
    print('\r');
    print('\n');
}

void SerialPort::printf(const char* format, ...){
    if(!assertValidUART()) return;
    va_list args;
    va_start(args, format);

    while(*format){
        while(*format == '%' ){
            switch(*(++format)){
                case '\0':
                    return;
                case 'd':
                    print((int)va_arg(args, int));
                break;
                case 'h':
                    print((int)va_arg(args, int), SERIAL_INTEGER_HEX);
                break;
                case 'b':
                    print((int)va_arg(args, int), SERIAL_INTEGER_BINARY);
                case 'f':
                    print((float)va_arg(args, float));
                break;
                case 'c':
                    print((char)va_arg(args, char));
                    break;
                case 's':
                    print((const char*)va_arg(args, const char*));
                    break;
                case 'l':
                    if(*(format+1)=='f'){//Double
                        print((double)va_arg(args, double));
                        format++;
                    }else{
                        print((long int)va_arg(args, long int));
                    }
                    break;
                case 'u':
                    if(*(format+1)=='h'){
                        print((int)va_arg(args, int), SERIAL_INTEGER_HEX | SERIAL_INTEGER_UNSIGNED);
                        format++;
                    }else if(*(format+1) == 'b'){
                        print((int)va_arg(args, int), SERIAL_INTEGER_BINARY | SERIAL_INTEGER_UNSIGNED);
                        format++;
                    }else{
                        print((int)va_arg(args, int), SERIAL_INTEGER_UNSIGNED);
                    }
                    break;
                case '%':
                    print('%');
                    break;

            }
            format++;
        }
        print(*(format++));
    }
    va_end(args);

}

/* Locks the system until selected UART receives a byte
 * \return Returns the read byte as a char type
 * */
char SerialPort::read(){
    if(!assertValidUART()) return '\0';
    while(((*UART_FSTAT_R) &  0x40) == 0x00); //Wait until Rx char
    return (char)((*UART_R) & 0xFF); //Cast to char
}


/* Locks the system until selected UART receives carry return and newline characters, or until it overflows
 * \param out is the external buffer where the string is stored
 * \param lim is the maximum length of the buffer (out)
 * \param crnl is a boolean value for store the newline characters
 *      - true: Store '\r' and '\n' in the string as long as buffer do not overflows
 *      - false: Ignore '\r' and '\n' in the string
 * \return Returns the input buffer pointer
 * */
char* SerialPort::readline(char* const out, uint8_t lim, bool crnl){
    if(!assertValidUART()) return out;
    //Start a finite state machine (\variable fsm) for detecting valid carry return '\r' and
    //new line '\n' characters, \variable count for detect overflow
    uint8_t fsm = 0, count=0;
    char* rx_p = out;   //Cast to mutable pointer
    do{
        *rx_p = read();
        count++;
        if(fsm == 0 && *rx_p == '\r'){
            fsm = 1;
        }else if(fsm == 1 && *rx_p == '\n'){
            fsm = 2;
        }
        rx_p++; //Iterate over out buffer
    }while(fsm != 2 && count != lim); //Exit when endline detected or overflow


    //String end
    if(fsm != 2){ //No newline detected
        *(rx_p-1) = '\0';
    }else if(crnl){ //Keep CR and NL
        *(rx_p) = '\0';
    }else{ //Erase CR and NL
        *(rx_p-2) = '\0';
    }
    return out;
}

/*
 * */
void SerialPort::open(){
    if(!assertValidUART())  return;

    SYSCTL_RCGCUART_R |= (1 << UART); // Enable UART Clock
    PORT_R = (uint32_t*)(GPIO_PORT_BASE + (UART_PORT_OFF[UART] << 12)); //Set pointer to GPIO Port Base Register
    UART_R = (uint32_t*)(UART_BASE_REG + (UART << 12)); //Set pointer to base UART Register
    UART_FSTAT_R = UART_R + (0x18 >> 2);                //Set pointer to Fifo Status UART Base Register
    SYSCTL_RCGCGPIO_R |= (1 << UART_PORT_OFF[UART]);    //Enable GPIO Port for UART Tx, Rx Signals

    *(PORT_R + (0x420>>2)) |= 0x03 <<  UART_RXIO_B[UART];        //Select GPIO Tx, Rx alternative function
    *(PORT_R + (0x528>>2)) &= ~ (0x03 <<  UART_RXIO_B[UART]);    //Disable GPIO Tx, Rx analog function
    *(PORT_R + (0x52C>>2)) |= 0x11 <<  (UART_RXIO_B[UART]<<2);   //Port Mux Tx, Rx to UART
    *(PORT_R + (0x51C>>2)) |= 0x03 << UART_RXIO_B[UART];         //Enable Tx,Rx Pins

    /* Calculate Baud-Rate Divisor, integer and fractional parts, according datasheet*/
    float baudRateDivisor = (float)(CPU_PIOSC_FREQUENCY) / (16.0 * baud);
    uint32_t iBRD = (uint32_t)baudRateDivisor;
    uint32_t fBRD = (uint32_t)((baudRateDivisor-iBRD)*64 + 0.5);

    *(UART_R + (0x030>>2)) = 0x300;  //Disable UART and set default UART Control configuration.
    *(UART_R + (0x024>>2)) = iBRD;   //Set Integer baud-rate divisor
    *(UART_R + (0x028>>2)) = fBRD;   //Set Fractional baud-rate divisor
    *(UART_R + (0x02C>>2)) = 0x60;   //Line Control 8 bits, 1 byte FIFO, 1 stop bit, no parity
    *(UART_R + (0xFC8>>2)) = 0x05;   //Select alternative clock as source (PIOSC)
    *(UART_R + (0x030>>2)) |= 0x01;  //Enable UART
}

void SerialPort::close(){
    if(!assertValidUART())  return;
    SYSCTL_RCGCUART_R &= ~(1<<UART); // Disable UART Clock
}



int SerialPort::assertValidUART(){
    return UART <= 7;
}


#ifdef __cplusplus
extern "C"{
#endif
void SerialPort_UART0_Interrupt(){}
void SerialPort_UART1_Interrupt(){}
void SerialPort_UART2_Interrupt(){}
void SerialPort_UART3_Interrupt(){}
void SerialPort_UART4_Interrupt(){}
void SerialPort_UART5_Interrupt(){}
void SerialPort_UART6_Interrupt(){}
void SerialPort_UART7_Interrupt(){}
#ifdef __cplusplus
}
#endif



