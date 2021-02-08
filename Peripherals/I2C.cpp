/*
 * I2C.cpp
 *
 *  Created on: 28 ene. 2021
 *  Author: Manuel Andres Herrera Juárez
 *  Ciudad de México, México
 */


#include <stdio.h>

#include <../inc/tm4c1294ncpdt.h>
#include <Peripherals/Board.hpp>
#include <Peripherals/I2C.hpp>

#include "../driverlib/sysctl.h"
#include "../driverlib/rom_map.h"

static const uint8_t I2C_SCLIO_B[] = {2, 0, 0, 4, 6, 0, 6, 4, 2, 0}; //SCL Bit, SDA = SCLIO + 1, EXCEPT I2C_2, store SDA2 = SCL2-1
static const uint8_t I2C_PORT_OFF[] = {GPIO_PORTB_OFF, GPIO_PORTG_OFF, GPIO_PORTL_OFF,
                                      GPIO_PORTK_OFF, GPIO_PORTK_OFF, GPIO_PORTB_OFF,
                                      GPIO_PORTA_OFF, GPIO_PORTA_OFF, GPIO_PORTA_OFF,
                                      GPIO_PORTA_OFF}; //GPIO Base offset



I2C::I2C(){
    I2Cx = 0;
    freq = 100000;
    mode = I2C_MASTER;
    open();
}

I2C::I2C(uint32_t speed, uint8_t i2cx, uint8_t i2cMode){
    I2Cx = i2cx;
    freq = speed;
    mode = i2cMode;
    open();
}


void I2C::open(){
    if(I2Cx > 9  || !assertValidI2CSpeed()){
        mode = I2C_INVALID;
        return;
    }

    PORT_R = (uint32_t*)(GPIO_PORT_BASE + (I2C_PORT_OFF[I2Cx] << 12)); //Set pointer to GPIO Port Base Register
    I2C_R = (uint32_t*)(GET_I2C_BASE_R(I2Cx) + (I2Cx << 12)); //Get I2Cx Base Register
    I2C_STATUS_R = I2C_R + (0x004 >> 2);

    SYSCTL_RCGCI2C_R |= (1 << I2Cx); //Enable I2Cx clock
    SYSCTL_RCGCGPIO_R |= (1 << I2C_PORT_OFF[I2Cx]);    //Enable GPIO Port for I2C SDA, SCL Signals

    while((SYSCTL_RCGCI2C_R & (1<<I2Cx)) == 0 || (SYSCTL_RCGCGPIO_R & (1<<I2C_PORT_OFF[I2Cx])) == 0);


    *(PORT_R + (0x400>>2)) &= ~(0x03 <<  I2C_SCLIO_B[I2Cx]);    //Select SDA, SCL as inputs
    *(PORT_R + (0x420>>2)) |= 0x03 <<  I2C_SCLIO_B[I2Cx];       //Select GPIO SDA, SCL pins alternative function
    *(PORT_R + (0x528>>2)) &= ~(0x03 <<  I2C_SCLIO_B[I2Cx]);    //Disable GPIO Tx, Rx analog function

    //Configure SDA as Open-drain
    if(I2Cx != 2){
        *(PORT_R + (0x50C >> 2)) |= 1 << (I2C_SCLIO_B[I2Cx]+1); //SDAIO = SCLIO + 1
    }else{
        *(PORT_R + (0x50C >> 2)) |= 1 << I2C_SCLIO_B[I2Cx]; //Except SDA2
    }

    *(PORT_R + (0x52C >> 2)) |= 0x22 <<  (I2C_SCLIO_B[I2Cx]<<2);   //Port Mux SDA, SCL to I2C (Alternate function 2)
    *(PORT_R + (0x51C >> 2)) |= 0x03 << I2C_SCLIO_B[I2Cx];         //Enable SDA,SCL Pins

    *(I2C_R + (0x020 >> 2)) |= (mode == I2C_MASTER ? 0x10 : 0x20); //Set I2C as Master or Slave
    *(I2C_R + (0x00C >> 2)) = ((CPU_FREQUENCY/(20*freq) -1) & 0x7F) | (freq <= 1000000 ? 0x00 : 0x80); //Set SCL Speed TPR Val
}

void I2C::close(){
}

uint8_t I2C::writeTo(uint8_t address, uint8_t data){
    if(mode != I2C_MASTER) return I2C_WRITE_ERROR_MODE;

    *I2C_R = (address << 1) & 0xFE; //Set Slave Address - Transmit
    *(I2C_R + (0x008 >> 2)) = data; //Set Desired data to MDR
    while(((*I2C_STATUS_R) & 0x40) != 0); //Wait until bus free
    *I2C_STATUS_R = 0x07; //Init single byte transmit
    while(((*I2C_STATUS_R) & 0x01) != 0); //Wait until controller idle

    //Check Error bit
    if(((*I2C_STATUS_R) & 0x02) == 0){
        return I2C_WRITE_OK; //No error
    }
    return I2C_WRITE_ERROR; //Error
}

uint8_t I2C::writeTo(uint8_t address, uint8_t dev_reg, const void* data_r, uint8_t len){
    if(mode != I2C_MASTER) return I2C_WRITE_ERROR_MODE;
    if(len == 0) return I2C_WRITE_ERROR_NONE;

    uint8_t* data = (uint8_t*)data_r;

    *I2C_R = (address << 1) & 0xFE; //Set Slave Address - Transmit
    *(I2C_R + (0x008 >> 2)) = dev_reg; //Set device register to MDR
    while(((*I2C_STATUS_R) & 0x40) != 0); //Wait until bus free
    *I2C_STATUS_R = 0x03; //Init single byte transmit, hold master enable

    while(1){
        while(((*I2C_STATUS_R) & 0x01) != 0); //Wait until controller idle
        //Check Error bit
        if(((*I2C_STATUS_R) & 0x02) != 0){ //Error found
            if(((*I2C_STATUS_R)) & 0x10 == 0){ //I2C Controller won arbitration
                *I2C_STATUS_R = 0x04; //Generate stop condition
            }
            return I2C_WRITE_ERROR;
        }

        *(I2C_R + (0x008 >> 2)) = *data;

        if(--len > 0){
            *I2C_STATUS_R = 0x01;
            data++;
        }else break;
    }

    *I2C_STATUS_R = 0x05; //Transmit last byte and generate stop
    while(((*I2C_STATUS_R) & 0x01) != 0); //Wait until controller idle

    //Check Error bit
    if(((*I2C_STATUS_R) & 0x02) == 0){
        return I2C_WRITE_OK; //No error
    }
    return I2C_WRITE_ERROR; //Error
}

uint8_t I2C::read(uint8_t address, void* out_r, uint8_t len){
    if(mode != I2C_MASTER) return I2C_READ_ERROR_MODE;
    if(len == 0)return I2C_READ_ERROR_NONE;
    uint8_t* out = (uint8_t*)out_r;

    *I2C_R = (address << 1) & 0xFE | 0x01; //Set Slave Address - Receive Mode
    //while(((*I2C_STATUS_R) & 0x40) != 0); //Wait until bus free

    if(len == 1){
        *I2C_STATUS_R = 0x07; //Init single byte receive
        for(int i=0; i<300; i++);

        while(((*I2C_STATUS_R) & 0x01) != 0); //Wait until controller idle
        //Check Error bit
        if(((*I2C_STATUS_R) & 0x02) == 0){ //No error
            *out = (uint8_t)(*(I2C_R + (0x008 >> 2))); //Read Rx data byte
            return I2C_READ_OK;
        }
        return I2C_READ_ERROR; //Error
    }else{
        *I2C_STATUS_R = 0x0B; //!TODO delay(300)

        while(1){
            MAP_SysCtlDelay(100);
            while(((*I2C_STATUS_R) & 0x01) == 0x01); //Wait until controller idle
            //Check Error bit
            if(((*I2C_STATUS_R) & 0x02) != 0){ //Error found
                if(((*I2C_STATUS_R)) & 0x10 == 0){ //I2C Controller won arbitration
                    *I2C_STATUS_R = 0x04; //Generate stop condition
                }
                return I2C_READ_ERROR;
            }
            *(out++) = *(I2C_R + (0x008 >> 2)); //Read Rx data byte
            if(--len > 1){
                *I2C_STATUS_R = 0x09; //Continue Rx with acknowledge
            }else break;
        }
        *I2C_STATUS_R = 0x05; //End last byte transaction
        MAP_SysCtlDelay(100);
        while(((*I2C_STATUS_R) & 0x01) != 0); //Wait until controller idle
        //Check Error bit
        if(((*I2C_STATUS_R) & 0x02) == 0){ //No error
            *out = (uint8_t)(*(I2C_R + (0x008 >> 2))); //Read last Rx data byte
            return I2C_READ_OK;
        }
        return I2C_READ_ERROR; //Error
    }
}

uint8_t I2C::read(uint8_t address, uint8_t dev_reg, void* out_r, uint8_t len){
    if(mode != I2C_MASTER)  return I2C_READ_ERROR_MODE;
    if(len == 0)return I2C_READ_ERROR_NONE;
    uint8_t* out = (uint8_t*)out_r;

    *I2C_R = (address << 1) & 0xFE; //Set slave address, Tx Mode for subregister
    *(I2C_R + (0x008 >> 2)) = dev_reg; //Set device register to MDR
    while(((*I2C_STATUS_R) & 0x40) != 0); //Wait until bus free
    *I2C_STATUS_R = 0x03; //Init single byte transmit, hold master enable
    while(((*I2C_STATUS_R) & 0x01) != 0); //Wait until controller idle

    //Check Error bit
    if(((*I2C_STATUS_R) & 0x02) != 0){
        return I2C_READ_ERROR_WADD; //Error writing address
    }

    *I2C_R = (address << 1) & 0xFE | 0x01; //Set slave address, Rx Mode
    *I2C_STATUS_R = 0x0B; //Enable I2C, Start condition, with acknowledge

    while(1){
        MAP_SysCtlDelay(100);
        while(((*I2C_STATUS_R) & 0x01) != 0); //Wait until controller idle
        //Check Error bit
        if(((*I2C_STATUS_R) & 0x02) != 0){ //Error found
            if(((*I2C_STATUS_R)) & 0x10 == 0){ //I2C Controller won arbitration
                *I2C_STATUS_R = 0x04; //Generate stop condition
            }
            return I2C_READ_ERROR;
        }
        *(out++) = *(I2C_R + (0x008 >> 2)); //Read Rx data byte
        if(--len > 1){
            *I2C_STATUS_R = 0x09; //Continue Rx with acknowledge
        }else break;
    }

    *I2C_STATUS_R = 0x05; //End I2C Transaction
    MAP_SysCtlDelay(100);

    while(((*I2C_STATUS_R) & 0x01) != 0); //Wait until controller idle
    //Check Error bit
    if(((*I2C_STATUS_R) & 0x02) == 0){ //No error
        *out = (uint8_t)(*(I2C_R + (0x008 >> 2))); //Read last Rx data byte
        return I2C_READ_OK;
    }
    return I2C_READ_ERROR; //Error
}

PrintStatus I2C::write(const char* byt, int n, uint8_t flags){
    /*uint8_t count = 0;
        PrintStatus status;

        while(*txt && (n<0 || count<n)){
            if(count == 0 && !(*(txt+1))){  //Unique byte
                return write(*txt);
            }else if(count == 0){           //First Byte
                status = write(*txt, _PRINT_WR_MULTIPLE_F | _PRINT_WR_FIRSTBYTE_F);
            }else if(!(*(txt+1))){          //Last byte
                status = write(*txt, _PRINT_WR_MULTIPLE_F | _PRINT_WR_LASTBYTE_F);
            }else{                          //Intermediate byte
                status = write(*txt, _PRINT_WR_MULTIPLE_F | _PRINT_WR_MEDBYTE_F);
            }

            if(status &_PRINT_STATUS_ERROR) return status;
            txt++;  count++;
        }
        return _PRINT_STATUS_NO_ERROR;*/
    return 0;
}

PrintStatus I2C::write(uint8_t c, uint8_t flags){
    return 0;
}

inline bool I2C::assertValidI2CSpeed(){
    switch(freq){
        case 100000:    //100KHz
        case 400000:    //400KHz
        case 1000000:   //1MHz
        case 3330000:   //3.33MHz
            return true;
    }
    return false;
}
