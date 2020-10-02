#include <usart.h>
#include <spi.h>
#include <bme280.h>
#include <adc.h>
#include <systick.h>
#include <stdio.h>

// for LED and CS pins
#include <gd32vf103_rcu.h>
#include <gd32vf103_gpio.h>


// According to datasheet temperature can have an offset of upto 45°C. This is for my chip.
#define TEMP_OFFSET (28.7 - 53.64)


// RGB led of Longan Nano board
struct leds { 
    uint32_t port; uint32_t pin; int rcu;
} leds[] = {
    { GPIOC, GPIO_PIN_13, RCU_GPIOC },
    { GPIOA, GPIO_PIN_1, RCU_GPIOA },
    { GPIOA, GPIO_PIN_2, RCU_GPIOA }
};

enum colors { RED, GREEN, BLUE }; // match index of leds[]


struct spi_io { 
    uint32_t spi; uint32_t cs_rcu; uint32_t cs_port; uint32_t cs_pin; 
} bme_io = { 
    SPI0, RCU_GPIOA, GPIOA, GPIO_PIN_4
};

struct bme280_dev bme;


void init_adc() {
    adc_init(TEMP_OFFSET);
}

void init_usart0() {
    usart_init(USART0, 115200);
    printf("BME V3 10/2020\n\rConnect BME280 to SPI0, CS to PA4\n\r");
}


void led_off( enum colors color ) {
    gpio_bit_set(leds[color].port, leds[color].pin);
}


void led_on( enum colors color ) {
    gpio_bit_reset(leds[color].port, leds[color].pin);
}


void init_leds() {
    for( enum colors color = RED; color <= BLUE; color++ ) {
        rcu_periph_clock_enable(leds[color].rcu);
        gpio_init(leds[color].port, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, leds[color].pin);
        led_on(color);
    }
}


void init_spi(uint32_t spi) {
    spi_parameter_struct p = {
        .trans_mode           = SPI_TRANSMODE_FULLDUPLEX,
        .device_mode          = SPI_MASTER,
        .frame_size           = SPI_FRAMESIZE_8BIT,
        .clock_polarity_phase = SPI_CK_PL_HIGH_PH_2EDGE, // MODE3
        .nss                  = SPI_NSS_SOFT,
        .prescale             = SPI_PSC_128, // ABP2=100MHz -> SPI0 ~0.8MHz
        .endian               = SPI_ENDIAN_MSB
    };
    spi_Init(spi, &p);

    uint32_t psc = 1<<((p.prescale >> 3) + 1);
    if( spi == SPI0 ) {
        printf("ABP2 freq: %lu, PSC: %lu, SPI0 freq: %lu\n", rcu_clock_freq_get(CK_APB2), psc, rcu_clock_freq_get(CK_APB2)/psc);
    }
    else {
        printf("ABP1 freq: %lu, PSC: %lu, SPI[12] freq: %lu\n", rcu_clock_freq_get(CK_APB1), psc, rcu_clock_freq_get(CK_APB1)/psc);
    }
}


// interface function required by bosch driver
BME280_INTF_RET_TYPE bme_spi_read( uint8_t addr, uint8_t *data, uint32_t len, void *ptr ) {
    struct spi_io *io = (struct spi_io *)ptr;
    gpio_bit_reset(io->cs_port, io->cs_pin);
    spi_Recv(io->spi, addr, len, data);
    gpio_bit_set(io->cs_port, io->cs_pin);
    return BME280_OK;
}


// interface function required by bosch driver
BME280_INTF_RET_TYPE bme_spi_write( uint8_t addr, const uint8_t *data, uint32_t len, void *ptr ) {
    struct spi_io *io = (struct spi_io *)ptr;
    gpio_bit_reset(io->cs_port, io->cs_pin);
    spi_Send(io->spi, addr, len, data);
    gpio_bit_set(io->cs_port, io->cs_pin);
    return BME280_OK;
}


// interface function required by bosch driver
void bme_delay_us( uint32_t period, void *dummy ) {
    delay_1us(period);
}


int8_t init_bme( struct bme280_dev *dev, struct spi_io *bme_io ) {
    dev->intf_ptr = bme_io;
    dev->intf = BME280_SPI_INTF;
    dev->read = bme_spi_read;
    dev->write = bme_spi_write;
    dev->delay_us = bme_delay_us;

    rcu_periph_clock_enable(bme_io->cs_rcu);
    gpio_init(bme_io->cs_port, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, bme_io->cs_pin);
    gpio_bit_set(bme_io->cs_port, bme_io->cs_pin);

    if( bme280_init(dev) != BME280_OK ) return 1;

	/* Recommended mode of operation: Indoor navigation */
	dev->settings.osr_h = BME280_OVERSAMPLING_1X;
	dev->settings.osr_p = BME280_OVERSAMPLING_16X;
	dev->settings.osr_t = BME280_OVERSAMPLING_2X;
	dev->settings.filter = BME280_FILTER_COEFF_16;
	dev->settings.standby_time = BME280_STANDBY_TIME_62_5_MS;

    uint8_t settings_sel = BME280_OSR_PRESS_SEL
                         | BME280_OSR_TEMP_SEL
                         | BME280_OSR_HUM_SEL
                         | BME280_STANDBY_SEL
                         | BME280_FILTER_SEL;

	return bme280_set_sensor_settings(settings_sel, dev);
    // req_delay = bme280_cal_meas_delay(&dev->settings);
}


void sensor_loop() {
    while(1) {
        // start a bme280 measurement
        bme280_set_sensor_mode(BME280_FORCED_MODE, &bme);
        // start an adc measurement
        adc_start_measurement();

        // ample time for measurements to complete, but we dont want to flood serial anyways...
        delay_1ms(1000);
        
        struct bme280_data bme_data;

        bme280_get_sensor_data(BME280_ALL, &bme_data, &bme);
        printf(" %s: %4d.%02d ", "P [Pa]", (int)bme_data.pressure, (int)(bme_data.pressure*100) %100);
        printf(" %s: %3d.%02d ", "H [%]", (int)bme_data.humidity, (int)(bme_data.humidity*100) % 100);
        printf(" %s: %3d.%02d ", "T [C]", (int)bme_data.temperature, (int)(bme_data.temperature*100) % 100);

        double temperature, vref;
        adc_get_measurement(&temperature, &vref);
        printf(" %s: %3d.%02d ", "Tint [C]", (int)temperature, (int)(temperature*100) % 100);
        printf(" %s: %d.%2d ", "Vref [V]", (int)vref, (int)(vref*100) % 100);

        printf("\n\r");
    }
}


int main(void) {
    init_leds();
    init_usart0();
    init_spi(SPI0);
    init_bme(&bme, &bme_io);
    init_adc();

    for( enum colors color = RED; color <= BLUE; color++ ) {
        led_off(color);
    }

    sensor_loop();
}


int _put_char(int ch) {
    return usart_put_char(USART0, ch);
}
