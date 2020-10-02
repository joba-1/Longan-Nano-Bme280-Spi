#include <spi.h>
#include <bme280.h>
#include <systick.h>
#include <gd32vf103_rcu.h>
#include <gd32vf103_adc.h>
#include <stdio.h>


struct leds { 
    uint32_t port; uint32_t pin; int rcu;
} leds[] = {
    { GPIOC, GPIO_PIN_13, RCU_GPIOC },
    { GPIOA, GPIO_PIN_1, RCU_GPIOA },
    { GPIOA, GPIO_PIN_2, RCU_GPIOA }
};

enum colors { RED, GREEN, BLUE }; 

struct spi_cs { 
    uint32_t spi_port; uint32_t cs_port; uint32_t cs_pin; 
} bme_cs = { 
    SPI0, GPIOA, GPIO_PIN_4
};

struct bme280_dev bme;


void init_adc() {
    rcu_periph_clock_enable(RCU_ADC0);
    rcu_adc_clock_config(RCU_CKADC_CKAPB2_DIV8);

    /* reset ADC */
    adc_deinit(ADC0);
    /* ADC mode config */
    adc_mode_config(ADC_MODE_FREE);
    /* ADC scan function enable */
    adc_special_function_config(ADC0, ADC_SCAN_MODE, ENABLE);
    /* ADC data alignment config */
    adc_data_alignment_config(ADC0, ADC_DATAALIGN_RIGHT);
    /* ADC temperature and Vrefint enable */
    adc_tempsensor_vrefint_enable();
    
    /* ADC channel length config */
    adc_channel_length_config(ADC0, ADC_INSERTED_CHANNEL, 2);

    /* ADC temperature sensor channel config */
    adc_inserted_channel_config(ADC0, 0, ADC_CHANNEL_16, ADC_SAMPLETIME_239POINT5);
    /* ADC internal reference voltage channel config */
    adc_inserted_channel_config(ADC0, 1, ADC_CHANNEL_17, ADC_SAMPLETIME_239POINT5);

    /* ADC trigger config */
    adc_external_trigger_source_config(ADC0, ADC_INSERTED_CHANNEL, ADC0_1_EXTTRIG_INSERTED_NONE);
 
    adc_external_trigger_config(ADC0, ADC_INSERTED_CHANNEL, ENABLE);
    
    /* enable ADC interface */
    adc_enable(ADC0);
    delay_1ms(1);
    /* ADC calibration and reset calibration */
    adc_calibration_enable(ADC0);
}


void init_uart0(void) {	
	/* enable GPIO clock */
    rcu_periph_clock_enable(RCU_GPIOA);
    /* enable USART clock */
    rcu_periph_clock_enable(RCU_USART0);

    /* connect port to USARTx_Tx */
    gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_9);
    /* connect port to USARTx_Rx */
    gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_10);

	/* USART configure */
    usart_deinit(USART0);
    usart_baudrate_set(USART0, 115200U);
    usart_word_length_set(USART0, USART_WL_8BIT);
    usart_stop_bit_set(USART0, USART_STB_1BIT);
    usart_parity_config(USART0, USART_PM_NONE);
    usart_hardware_flow_rts_config(USART0, USART_RTS_DISABLE);
    usart_hardware_flow_cts_config(USART0, USART_CTS_DISABLE);
    usart_receive_config(USART0, USART_RECEIVE_ENABLE);
    usart_transmit_config(USART0, USART_TRANSMIT_ENABLE);
    usart_enable(USART0);

    usart_interrupt_enable(USART0, USART_INT_RBNE);

    printf("BME V1 10/2020\n\r");
}


void show_bme() { 
    double temperature = 0;
    double vref = 0;
    while(1) {
        adc_software_trigger_enable(ADC0, ADC_INSERTED_CHANNEL);
        uint64_t adc_start = start_elapse();;
        
        struct bme280_data bme_data;
        char msg[20];
        if( bme280_get_sensor_data(BME280_ALL, &bme_data, &bme) == BME280_OK ) {
            snprintf(msg, sizeof(msg), " %s: %4d.%02d ", "P [Pa]", (int)bme_data.pressure, (int)(bme_data.pressure*100) %100);
            puts(msg);

            snprintf(msg, sizeof(msg), " %s: %3d.%02d ", "H [%]", (int)bme_data.humidity, (int)(bme_data.humidity*100) % 100);
            puts(msg);

            snprintf(msg, sizeof(msg), " %s: %3d.%02d ", "T [C]", (int)bme_data.temperature, (int)(bme_data.temperature*100) % 100);
            puts(msg);
        }

        elapse_1ms(adc_start, 2000);
        temperature = (1.42 - ADC_IDATA0(ADC0)*3.3/4096) * 1000 / 4.3 + 25;
        vref = (ADC_IDATA1(ADC0) * 3.3 / 4096);
      
        snprintf(msg, sizeof(msg), " %s: %3d.%02d ", "Tint [C]", (int)temperature, (int)(temperature*100) % 100);
        puts(msg);

        snprintf(msg, sizeof(msg), " %s: %d.%2d ", "Vref [V]", (int)vref, (int)(vref*100) % 100);
        puts(msg);

        puts("\n\r");
    }
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
        .clock_polarity_phase = SPI_CK_PL_HIGH_PH_2EDGE,
        .nss                  = SPI_NSS_SOFT,
        .prescale             = SPI_PSC_2, // ~675kHz?
        .endian               = SPI_ENDIAN_MSB
    };
    spi_Init(spi, &p);
}


BME280_INTF_RET_TYPE bme_spi_read( uint8_t addr, uint8_t *data, uint32_t len, void *ptr ) {
    struct spi_cs *cs = (struct spi_cs *)ptr;
    gpio_bit_reset(cs->cs_port, cs->cs_pin);
    spi_Recv(cs->spi_port, addr, len, data);
    gpio_bit_set(cs->cs_port, cs->cs_pin);
    return BME280_OK;
}


BME280_INTF_RET_TYPE bme_spi_write( uint8_t addr, const uint8_t *data, uint32_t len, void *ptr ) {
    struct spi_cs *cs = (struct spi_cs *)ptr;
    gpio_bit_reset(cs->cs_port, cs->cs_pin);
    spi_Send(cs->spi_port, addr, len, data);
    gpio_bit_set(cs->cs_port, cs->cs_pin);
    return BME280_OK;
}


void bme_delay_us( uint32_t period, void *dummy ) {
    delay_1us(period);
}


int8_t init_bme( struct bme280_dev *dev ) {
    struct spi_cs *cs = dev->intf_ptr;
    gpio_init(cs->cs_port, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, cs->cs_pin);
    gpio_bit_set(cs->cs_port, cs->cs_pin);

    dev->intf_ptr = &bme_cs;
    dev->intf = BME280_SPI_INTF;
    dev->read = bme_spi_read;
    dev->write = bme_spi_write;
    dev->delay_us = bme_delay_us;

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

	bme280_set_sensor_settings(settings_sel, dev);
    // req_delay = bme280_cal_meas_delay(&dev->settings);
	return bme280_set_sensor_mode(BME280_NORMAL_MODE, dev);
}


int main(void) {
    init_leds();
    init_uart0();
    init_spi(SPI0);
    init_bme(&bme);
    init_adc();

    for( enum colors color = RED; color <= BLUE; color++ ) {
        led_off(color);
    }

    show_bme();
}

int _put_char(int ch) {
    usart_data_transmit(USART0, (uint8_t)ch);
    while( usart_flag_get(USART0, USART_FLAG_TBE) == RESET );
    return ch;
}
