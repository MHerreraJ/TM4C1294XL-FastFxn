#include <Peripherals/SerialPort.hpp>
#include "inc/tm4c1294ncpdt.h"
#include <stdint.h>

#define EXT_OSC_FREQ ((uint32_t)25000000)



void UARTEcho(){

}

void init(){
    //Set Alternative Clock Source for GPT, SSI and UART = PIOSC (16 MHz)
    SYSCTL_ALTCLKCFG_R &= ~0x0F;

    SYSCTL_RCGCGPIO_R |= 1 << 12;//Init GPIO_N Clock
    while(SYSCTL_RCGCGPIO_R & (1<<12) == 0); //Wait until clock propagation
    GPIO_PORTN_DIR_R = 0x02; //Set PN1 as output
    GPIO_PORTN_AFSEL_R &= 0xFFFFFFFD; //Set PN1 Controlled by GPIO
    GPIO_PORTN_PC_R &= 0xFFFFFFFE; // Drive values of 2,4,8 mA
    GPIO_PORTN_DEN_R |= 0x02; //Enable PN1
    GPIO_PORTN_DATA_R = 0x00; //Clear Output
}

void configureTimer(){
    SYSCTL_RCGCTIMER_R |= 0x01; //Enable Timer 0S
    while((SYSCTL_RCGCTIMER_R & 0x01) == 0x00);
    TIMER0_CTL_R = 0x00; //Disable Timer A and B
    TIMER0_CFG_R = 0x00; //32 bit timer
    TIMER0_TAMR_R = 0x02;//Periodic Mode
    TIMER0_TAILR_R = 2000000; //Clk = 16Mhz -> 0.5s = 8x10e6 count
    TIMER0_IMR_R = 0x01; //Timer A TimeOut Interrupt Mask
    NVIC_EN0_R |= 1 << 19;
    TIMER0_CTL_R |= 0x01; //Enable Timer0
}
#ifdef __cplusplus
extern "C" {
#endif

void Timer0A_INTERRUPT(){
    if((TIMER0_RIS_R & 0x01) != 0x00){ //Timeout Event Interrupt
        GPIO_PORTN_DATA_R ^= 0x02;
        TIMER0_ICR_R |= 0x01; //Clear RTC Interrupt
    }
}
#ifdef __cplusplus
}
#endif


int main(void){
    //uint32_t i=0;
    init();
    configureTimer();

    SerialPort Serial(9600);
    SerialPort Serial4(9600, 4); //PA2->RX PA3->TX
    char txtBuffer[255];
    int num = 0;

    while(1){
        /*GPIO_PORTN_DATA_R ^= 0x02; //Toggle PN1*/
        //Serial.println("Hola Mundo");

        //Serial.print(Serial.readline(txtBuffer,255)); //UART Echo
        //Serial.printf("%d, %d\r\n", num, -num);
        //Serial.print(num);
        //Serial.print(num, SERIAL_INTEGER_BINARY);
        //Serial.print(num, SERIAL_INTEGER_BINARY);
        //Serial.println("");
        //Serial.printf("%d\t%h\t%b\r\n", num, num, num);
        //Serial.printf("%d\t%h\t%b\r\n", -num, -num, -num);
        //Serial.printf("%u\t%uh\t%ub\r\n\r\n", -num, -num, -num);
        Serial.readline(txtBuffer,255);
        Serial.print(txtBuffer);
        Serial4.print(txtBuffer);
        num++;
    }
}
