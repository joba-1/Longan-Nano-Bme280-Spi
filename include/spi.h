#ifndef _SPI_H
#define _SPI_H

#include <stdint.h>
#include "gd32vf103_spi.h"

void spi_Init( uint32_t port, spi_parameter_struct *parameters );
uint8_t spi_Swap( const uint32_t port, const uint8_t byte );
void spi_Send( const uint32_t port, const uint8_t addr, uint16_t num_bytes, const uint8_t *bytes );
void spi_Recv( const uint32_t port, const uint8_t addr, uint16_t num_bytes, uint8_t *bytes );

#endif
