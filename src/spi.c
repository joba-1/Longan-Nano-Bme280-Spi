#include <spi.h>

#include "gd32vf103_rcu.h"
#include "gd32vf103_gpio.h"

void spi_Init( uint32_t port, spi_parameter_struct *parameters ) {
    // Example parameter config
    // spi_init_struct->trans_mode           = SPI_TRANSMODE_FULLDUPLEX;
    // spi_init_struct->device_mode          = SPI_MASTER;
    // spi_init_struct->frame_size           = SPI_FRAMESIZE_8BIT;
    // spi_init_struct->clock_polarity_phase = SPI_CK_PL_HIGH_PH_2EDGE;
    // spi_init_struct->nss                  = SPI_NSS_SOFT;
    // spi_init_struct->prescale             = SPI_PSC_32;
    // spi_init_struct->endian               = SPI_ENDIAN_MSB;

	rcu_periph_clock_enable(RCU_AF); // needed?

    switch( port ) {
        case SPI0:
            rcu_periph_clock_enable(RCU_GPIOA);
            rcu_periph_clock_enable(RCU_SPI0);
            gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_5 | GPIO_PIN_7);
            gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_6);
            break;
        case SPI1:
            rcu_periph_clock_enable(RCU_GPIOB);
            rcu_periph_clock_enable(RCU_SPI1);
            gpio_init(GPIOB, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_13 | GPIO_PIN_15);
            gpio_init(GPIOB, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_14);
            break;
        case SPI2:
            rcu_periph_clock_enable(RCU_GPIOB);
            rcu_periph_clock_enable(RCU_SPI2);
            gpio_init(GPIOB, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_5 | GPIO_PIN_3);
            gpio_init(GPIOB, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_4);
            break;
        default:
            return;
    }

    // Example CS pin configuration. Reset during transfer.
    // gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_12);
    // gpio_bit_set(GPIOB, GPIO_PIN_12);

    spi_init(port, parameters);
    // spi_crc_polynomial_set(port,7); // check this...
    spi_enable(port);
}

uint8_t spi_Swap( const uint32_t port, const uint8_t byte ) {
    while(RESET == spi_i2s_flag_get(port, SPI_FLAG_TBE));
    spi_i2s_data_transmit(port, byte);
    while(RESET == spi_i2s_flag_get(port, SPI_FLAG_RBNE));
    return(spi_i2s_data_receive(port));
}

void spi_Send( const uint32_t port, const uint8_t addr, uint16_t num_bytes, const uint8_t *bytes ) {
    spi_Swap(port, addr);
    while( num_bytes-- ) {
        spi_Swap(port, *(bytes++));
    }
}

void spi_Recv( const uint32_t port, const uint8_t addr, uint16_t num_bytes, uint8_t *bytes ) {
    spi_Swap(port, addr);
    while( num_bytes-- ) {
        *(bytes++) = spi_Swap(port, 0);
    }
}
