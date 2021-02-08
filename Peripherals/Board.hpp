/*
 * Board.hpp
 *
 *  Created on: 27 ene. 2021
 *      Author: manue
 */

#ifndef PERIPHERALS_BOARD_HPP_
#define PERIPHERALS_BOARD_HPP_

#include <stdint.h>

#define USE_PLL         (0)

#define CPU_PIOSC_FREQUENCY (16000000U)

#if USE_PLL == 0
#define CPU_FREQUENCY   CPU_PIOSC_FREQUENCY
#else
#define CPU_FREQUENCY   (120000000U)
#endif


#define TIVA_HWREG(x)   (*((volatile uint32_t*)(x)))

#define GPIO_PORTA_OFF  0
#define GPIO_PORTB_OFF  1
#define GPIO_PORTC_OFF  2
#define GPIO_PORTD_OFF  3
#define GPIO_PORTE_OFF  4
#define GPIO_PORTF_OFF  5
#define GPIO_PORTG_OFF  6
#define GPIO_PORTH_OFF  7
#define GPIO_PORTJ_OFF  8
#define GPIO_PORTK_OFF  9
#define GPIO_PORTL_OFF  10
#define GPIO_PORTM_OFF  11
#define GPIO_PORTN_OFF  12
#define GPIO_PORTP_OFF  13
#define GPIO_PORTQ_OFF  14



extern const uint32_t GPIO_PORT_BASE;
extern const uint32_t UART_BASE_REG;
extern const uint32_t I2C_BASE_REG_0;
extern const uint32_t I2C_BASE_REG_1;
extern const uint32_t I2C_BASE_REG_2;


#define GET_I2C_BASE_R(x)   (((x) <= 3) ? I2C_BASE_REG_0 : (((x) <= 7) ? I2C_BASE_REG_1 : I2C_BASE_REG_2))

#endif /* PERIPHERALS_BOARD_HPP_ */
