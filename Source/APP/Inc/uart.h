#ifndef __UART_H
#define __UART_H
#ifdef __cplusplus
extern "C"
{
#endif
	#include "stdint.h"
	#include "main.h"
    
    extern UART_HandleTypeDef huart1;
    
	typedef struct{
		uint32_t baudrate;
		uint8_t  data_bits;
		uint8_t  stop_bits;
		uint8_t  parity;
	}uart_cfg_t;

	typedef struct{
		uart_cfg_t cfg;
		uint8_t *rx_buf;
		uint16_t rx_len;
		uint16_t rx_index;
	}uart_obj_t;
	void MX_USART1_UART_Init(void);
#ifdef __cplusplus
}
#endif
#endif