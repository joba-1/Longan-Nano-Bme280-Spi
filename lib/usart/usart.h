#ifndef USART_H
#define USART_H

#include <stdint.h>

void usart_init( uint32_t usart, uint32_t baud );
int usart_put_char( uint32_t usart, int ch );

#endif
