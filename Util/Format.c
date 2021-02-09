/*
 * Format.c
 *
 *  Created on: 5 feb. 2021
 *  Author: Manuel Andres Herrera Juárez
 *  Ciudad de México, México
 */


#include "Format.h"

/**
 * Check if is a decimal digit character
 *
 * @param c is the character to check
 * @return true if c is between '0'-'9'
 */
uint8_t isDigit(char c){
    return c >= '0' && c <= '9';
}


/**
 * Get integer decimal value from its character representation
 *
 * @param c is the character representing a decimal value
 * @return the corresponding integer
 */
uint8_t getNumber(char c){
    return c - '0';
}
