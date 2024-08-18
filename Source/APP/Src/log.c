/** 
 * @brief:  This is a log module
 * @author: hyj
 * @date:   24/07
 */



/* incalude header files */
#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "log.h"
#include "uart.h"
/* typedef */


/* globle variables */


/* function definition */

/**
 * @brief: 	      Output formatted data to a specific port
 * @param[in]:    fmt  data format
 * @return:       void
 */

void log_printf(const char *fmt, ...)
{
	char buf[PRINT_BUF_SIZE] = {0};
	/* format data */ 
	va_list args;
	va_start(args, fmt);
	vsprintf(buf, fmt, args);
	va_end(args);
	/* output data */
	LOG_OUTPUT((uint8_t*)buf, strlen(buf));
}


#define DEFINE_FUN_FLOAT_2_INT(type,name)   \
type name(float data, uint32_t mulriple)    \
{                                           \
    float d = data;                        \
    d *= mulriple;                              \
    return d;                               \
}                                           
DEFINE_FUN_FLOAT_2_INT(uint32_t,FloatToU32)   
DEFINE_FUN_FLOAT_2_INT(int32_t,FloatToS32)   
