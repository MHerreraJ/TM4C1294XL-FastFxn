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


#endif /* PERIPHERALS_BOARD_HPP_ */
