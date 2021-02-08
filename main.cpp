#include <Peripherals/SerialPort.hpp>
#include <Peripherals/I2C.hpp>
#include "inc/tm4c1294ncpdt.h"
#include <stdint.h>


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
    TIMER0_TAILR_R = 8000000; //Clk = 16Mhz -> 0.5s = 8x10e6 count
    TIMER0_IMR_R = 0x01; //Timer A TimeOut Interrupt Mask
    NVIC_EN0_R |= 1 << 19;
    TIMER0_CTL_R |= 0x01; //Enable Timer0
}

int request = 0;

#ifdef __cplusplus
extern "C" {
#endif

void Timer0A_INTERRUPT(){
    if((TIMER0_RIS_R & 0x01) != 0x00){ //Timeout Event Interrupt
        GPIO_PORTN_DATA_R ^= 0x02;
        TIMER0_ICR_R |= 0x01; //Clear RTC Interrupt
        request = 1;
    }
}
#ifdef __cplusplus
}
#endif


#define I2C_TEST_ADDRESS    0x08

int main(void){
    init();
    configureTimer();

    SerialPort Serial(115200); //PA0, PA1
    I2C I2C0(100000, I2C_I2C0, I2C_MASTER); //PB2 (SCL), PB3

    char txtBuffer[255];
    uint8_t num = 0;

    char rx;

    while(1){
        //Test Serial (UART0)
        Serial.printf("Hola %s, tienes %d años, en hex = %x\r%c", "Manuel", num, num, '\n');
        request=0;
        while(request == 0);

        num++;
    }
}
