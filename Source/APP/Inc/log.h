#ifndef LOG_H
#define LOG_H
/* incalude header files */


/* macro definition */

#define PRINT_BUF_SIZE 32 /* default 32 bytes */

#define LOG_OUTPUT(pbuf, size)    HAL_UART_Transmit(&huart1, pbuf, size, 5)

/* typedef */

/* function declaration */

/*
 *@brief 	Output formatted data to a specific port
 *@param    fmt  data format
 *@return   void
 */
void log_printf(const char *fmt, ...);

#endif