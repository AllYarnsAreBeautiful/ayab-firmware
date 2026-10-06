#ifndef CRC8_H
#define CRC8_H

#include <stddef.h>
#include <stdint.h>

/**
 * Calculate CRC-8 checksum for the given data buffer
 * 
 * @param buffer Pointer to the data buffer
 * @param length Length of the data buffer in bytes
 * @return The calculated CRC-8 checksum
 * 
 * Note: This implementation uses Maxim/Dallas 1-wire "Dow CRC"
         based on x^8 + x^5 + x^4 + 1 polynomial 
 */
uint8_t crc8(const uint8_t *buffer, size_t length);

#endif
