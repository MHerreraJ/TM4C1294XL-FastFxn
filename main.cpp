#include "inc/tm4c1294ncpdt.h"
#include <Peripherals/I2CMaster.hpp>
#include <Peripherals/SerialPort.hpp>

#include <Peripherals/Board.hpp>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Init Tiva EK-TM4C1294XL Launchpad for this example
 * Clock source = Precision Internal Oscillator (PIOSC = 16MHz)
 *
 * Enable GPIO PORTN clock, and configure user led PN1 as output
 * Configure Timer0 for blocking main process
 */
void init(){
    SYSCTL_ALTCLKCFG_R &= ~0x0F;            //Set Clock for GPT, SSI and UART = PIOSC (16 MHz)
    SYSCTL_RCGCGPIO_R |= 1 << 12;           //Init GPIO_N Clock
    while(SYSCTL_RCGCGPIO_R & (1<<12) == 0);//Wait until clock propagation
    GPIO_PORTN_DIR_R = 0x02;                //Set PN1 as output
    GPIO_PORTN_AFSEL_R &= 0xFFFFFFFD;       //Set PN1 Controlled by GPIO
    GPIO_PORTN_PC_R &= 0xFFFFFFFE;          // Drive values of 2,4,8 mA
    GPIO_PORTN_DEN_R |= 0x02;               //Enable PN1
    GPIO_PORTN_DATA_R = 0x00;               //Clear Output

    SYSCTL_RCGCTIMER_R |= 0x01;                 //Enable Timer 0S
    while((SYSCTL_RCGCTIMER_R & 0x01) == 0x00); //Wait for timer0 clock enable
    TIMER0_CTL_R = 0x00;                        //Disable Timer A and B
    TIMER0_CFG_R = 0x00;                        //Configure as 32 bit timer
    TIMER0_TAMR_R = 0x02;                       //Periodic Mode
    TIMER0_TAILR_R = (uint32_t)(((float)CPU_PIOSC_FREQUENCY)*0.5f); //0.5 second count
    TIMER0_IMR_R = 0x01;                        //Timer A TimeOut Interrupt Mask
    NVIC_EN0_R |= 1 << 19;                      //Enable NVIC Timer0 Interrupt
    TIMER0_CTL_R |= 0x01;                       //Enable Timer0
}



/**
 * Timer interrupt for free main process, using the request variable
 */
volatile int request = 0;
void Timer0A_INTERRUPT(){
    if((TIMER0_RIS_R & 0x01) != 0x00){  //Timeout Event Interrupt
        GPIO_PORTN_DATA_R ^= 0x02;      //Show interrupt by toggling User LED PN1
        TIMER0_ICR_R |= 0x01;           //Clear Interrupt
        request = 1;                    //Change request value
    }
}

#ifdef __cplusplus
}
#endif




/**
 * Main process executing an example with I2CMaster and SerialPort classes
 * for the Tiva TM4C1294NCPDT
 * @return none
 */

#define I2C_TEST_ADDRESS    0x08

int main(void){
    init();

    SerialPort Serial(115200);        //UART0 115200 bauds, GPIO's: PA0(RX), PA1(TX)
    I2CMaster I2C0(100000, I2C_I2C0); //I2C0 100000 Hz, GPIO's: PB2 (SCL), PB3(SDA)

    //Different buffers for demonstrate not garbage collected
    //Can be implemented with a single buffer
    char UARTBuffer[30]; //R/W buffer for UART0
    char I2CBuffer[10]; // R/W buffer for I2C0
    I2C0.setAddress(I2C_TEST_ADDRESS);

    //Test Serial and I2C0
    while(1){
        Serial.print("Enter a string and pulse intro: ");
        Serial.readline(UARTBuffer, 30, false);
        Serial.println("\r\nAttempt to write first 5 characters to 0x2A mem. loc. at extern I2C device");

        if(I2C0.printf("%c%5s", 0x2A, UARTBuffer) == I2C_READ_OK){ //Write 6 bytes to I2C Line Control
            if(I2C0.read(I2CBuffer, 6, true) == I2C_READ_OK){ //Read device response (6 bytes)
                Serial.printf("I2C: %d chars received\r\n\trxMsg: %6s\r\n\n", 6, I2CBuffer); //Show msg
            }else{
                Serial.println("I2C: Error Reading");
            }
        }else{
            Serial.println("I2C: Error Writing");
        }

        request = 0;
        while(request == 0); //Delay 0.5s
    }
}
