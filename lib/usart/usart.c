#include <usart.h>
#include <gd32vf103_usart.h>
#include <gd32vf103_rcu.h>
#include <gd32vf103_gpio.h>

void usart_init( uint32_t usart, uint32_t baud )
{
    switch( usart ) {
        case USART0:
            rcu_periph_clock_enable(RCU_GPIOA);
            rcu_periph_clock_enable(RCU_USART0);
            gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_9);
            gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_10);
           break;
        case USART1:
            rcu_periph_clock_enable(RCU_GPIOA);
            rcu_periph_clock_enable(RCU_USART1);
            gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_2);
            gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_3);
            break;
        case USART2:
            rcu_periph_clock_enable(RCU_GPIOB);
            rcu_periph_clock_enable(RCU_USART2);
            gpio_init(GPIOB, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_10);
            gpio_init(GPIOB, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_11);
            break;
        default:
            return;
    }

	/* USART configure */
    usart_deinit(usart);
    usart_baudrate_set(usart, baud);
    usart_word_length_set(usart, USART_WL_8BIT);
    usart_stop_bit_set(usart, USART_STB_1BIT);
    usart_parity_config(usart, USART_PM_NONE);
    usart_hardware_flow_rts_config(usart, USART_RTS_DISABLE);
    usart_hardware_flow_cts_config(usart, USART_CTS_DISABLE);
    usart_receive_config(usart, USART_RECEIVE_ENABLE);
    usart_transmit_config(usart, USART_TRANSMIT_ENABLE);
    usart_enable(usart);

    usart_interrupt_enable(usart, USART_INT_RBNE);
}


int usart_put_char( uint32_t usart, int ch ) {
    usart_data_transmit(usart, (uint8_t)ch);
    while( usart_flag_get(usart, USART_FLAG_TBE) == RESET );
    return ch;
}
