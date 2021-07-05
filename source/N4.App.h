#include <N3.h>

enum{
	INIT, READ_DATA, SEND_CMD, RECEIVED_CMD
};
enum { ADC0_CH0, ADC0_CH1, ADC0_CH2, ADC0_CH3, ADC0_CH4, ADC0_CH5, ADC0_CH6, ADC0_CH7, ADC0_CH8, ADC0_CH9, ADC0_CH10, ADC0_CH11};

#define BUFFER_UART_RX_SIZE 70
#define BUFFER_UART_TX_SIZE 128
#define DAC_LIMIT		 	1023
#define TIME_UPDATE_LIMIT 	200

#define BAUD_RATE_I2C		200000
#define BAUD_RATE_UART		38400
#define BAUD_RATE_SPI		500000
#define TICK_1ms_18M		18000
#define TICK_1ms_24M		24000
#define TICK_1ms_30M		30000
#define REFRESH_7SEG		10
#define DIGIT_1				1
#define DIGIT_2				2

#define ADC_R21				(1 << ADC0_CH0)
#define ADC_R22				(1 << ADC0_CH8)
#define ADC_LM35DZ			(1 << ADC0_CH7)


uint16_t ascii_to_int(uint8_t *pf, uint8_t c);






