/*
 * SerialPort.cpp
 *
 *  Created on: 27 ene. 2021
 *  Author: Manuel Andres Herrera Juárez
 *  Ciudad de México, México
 */

#include <stdio.h>
#include <stdarg.h>
#include <../inc/tm4c1294ncpdt.h>
#include <Peripherals/Board.hpp>
#include <Peripherals/SerialPort.hpp>


static const uint8_t UART_PORT_OFF[] = {0, 1, 0, 0, 0, 2, 13, 2}; //GPIO Base offset
static const uint8_t UART_RXIO_B[] = {0, 0, 6, 4, 2, 6, 0, 4}; //RXIO Bit, TX = RXIO + 1

/** Default SerialPort constructor
 *
 * UART0 Selected, at 9600 bauds
 */
SerialPort::SerialPort(){
    this->UART = 0;
    this->baud = 9600;
    open();
}

/** SerialPort Constructor
 *
 * @param baudrate is the speed at which UART will work
 * @param uart is the uart module to use (0 to 7)
 */

SerialPort::SerialPort(uint32_t baudrate, uint8_t uart){
   this->UART = uart;
   this->baud = baudrate;
   open();
}


/** Locks the system until selected UART receives a byte
 *
 * @return Returns the read byte as a char type
 */
char SerialPort::read(){
    if(!assertValidUART()) return '\0';
    while(((*UART_FSTAT_R) &  0x40) == 0x00); //Wait until Rx char
    return (char)((*UART_R) & 0xFF); //Cast to char
}


/** Locks the system until selected UART receives carry return and newline
 * characters, or until it overflows
 *
 * @param out is the external buffer where the string is stored
 * @param lim is the maximum length of the buffer (out)
 * @param crnl is a boolean value for store the newline characters
 *      - true: Store '\r' and '\n' in the string as long as buffer do not overflows
 *      - false: Ignore '\r' and '\n' in the string
 * @return Returns the input buffer pointer
 */
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

/**
 * Open the Selected TM4C1294 UART if valid
 * This function enable and configure the IO ports and maps
 * the corresponding GPIOs to the selected UART module.
 * UART is powered on, with 8 bits, 1 stop bit and no parity line control,
 * working at the baudrate specified in the constructor
 *
 */
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


/**
 * TM4C1294 has 8 different UART Modules UART0 - UART7
 * Check that valid UART was selected.
 * @return false if UART > 8
 *         true if UART <=7
 */
inline int SerialPort::assertValidUART(){
    return UART <= 7;
}

/**
 * Overrides Print class write method
 * @param c is the byte to send
 * @param flags not needed
 * @return ERROR if not valid UART, else NO ERROR
 */
PrintStatus SerialPort::write(uint8_t c, uint8_t flags){
    if(!assertValidUART()) return _PRINT_STATUS_ERROR;
    while(((*UART_FSTAT_R) & 0x20) != 0x00); //Wait until last Tx char send
    *UART_R = c;
    return _PRINT_STATUS_NO_ERROR;
}

/**
 * Overrides Print class write method
 * @param txt is the character string to send
 * @param n is the length of the string
 *      if n < 0, print all the string
 * @param flags not needed
 * @return ERROR if not valid UART, else NO ERROR
 */
PrintStatus SerialPort::write(const char* txt, int n, uint8_t flags){
    if(!assertValidUART()) return _PRINT_STATUS_ERROR;

    uint8_t count = 0;
    while(*txt && (n<0 || count<n)){
        if(count == 0 && !(*(txt+1))){  //Unique byte
            return write(*txt);
        }else{                          //Intermediate byte
           write(*txt);
        }
        txt++;  count++;
    }
    return _PRINT_STATUS_NO_ERROR;
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



