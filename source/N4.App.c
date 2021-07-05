//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Abstracciones
// Nivel4: (N4.App)
// Nivel3: (N3.h) (N3.BH1750, N3.BME280_SPI)
// Nivel2: (N2.h) (N2.xxxxx) (N2.Bme280/N2.Bme280_defs)
// Nivel1: (N1.h) (drivers->fsl_xxx, stdio, string)
//
//
// Version: 1.0				Fecha: 21/04/21
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
#include "N4.App.h"

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Definitions
//-----------------------------------------------------------------------------
#define CMD_UPDATE	1
#define CMD_WRITE	0
//-----------------------------------------------------------------------------
// Prototypes
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Variables
//-----------------------------------------------------------------------------
uint8_t write_update = CMD_WRITE, valid_rxd = 0;
float 		lux;
uint16_t	adc_ch[12], count_1seg = 1000;
uint8_t 	lcd_switch = REFRESH_7SEG, new_refresh = REFRESH_7SEG, up_no_down = 0;
uint8_t		dig[3]="00", digitos_7seg = 0, dig_on = DIGIT_1;
uint8_t		sw_user, sw_isp, sw_s1, sw_s2, count, rx0, rx1;
uint8_t		state = INIT, buffer_txd[BUFFER_UART_TX_SIZE], buffer_rxd0[BUFFER_UART_RX_SIZE], buffer_rxd1[BUFFER_UART_RX_SIZE];
int32_t  	T, P, H;
struct{
	uint8_t leds;
	uint8_t buzzer;
	uint16_t dac;
	uint8_t segments;
} update, device0, device1;
//-----------------------------------------------------------------------------
// Code
//-----------------------------------------------------------------------------

int main(void)
{
    ClockFRO30M();
    GPIO_Init();
    //DebugConsole_Init();
    SysTick_Init(TICK_1ms_30M);
    BME280_Init(BAUD_RATE_SPI);
    BH1750_I2C(BAUD_RATE_I2C);
    UART1_Init(BAUD_RATE_UART, buffer_rxd1, BUFFER_UART_RX_SIZE);	// MOBILE
    UART0_Init(BAUD_RATE_UART, buffer_rxd0, BUFFER_UART_RX_SIZE);	// PC
    ADC0_Init(ADC_R21 | ADC_R22 | ADC_LM35DZ, &adc_ch[0]);
    DAC1_Init();
	DAC1_Write(0);
	update.leds = 0;
	update.buzzer = 0;
	update.segments = 0;
	update.dac = 0;

	state = RECEIVED_CMD;
    while (1)
    {	// led D1 DAC
    	(void)sprintf(dig,"%02d", digitos_7seg);
    	switch(state){
    		case SEND_CMD:
    	        // comando a transmitir write:  {"id":"w","r21":1000,"r22":1000,"humedad":80,"presion":1100,"temperatura":25,"lux":63568,"s1":0,"s2":1,"user":0,"isp":0}
    	        // comando a transmitir update: {"id":"u","leds":0,"buzzer":0,"dac":0,"segments":11}

    			if(write_update == CMD_UPDATE){
        			(void)sprintf(&buffer_txd[0],"{\"id\":\"u\",\"leds\":%d,\"buzzer\":%d,\"dac\":%d,\"segments\":%d}\r\n", update.leds,update.buzzer,update.dac,update.segments);
        			write_update = CMD_WRITE;
    			}
    			else (void)sprintf(&buffer_txd[0],"{\"id\":\"w\",\"r21\":%d,\"r22\":%d,\"humedad\":%d,\"presion\":%d,\"temperatura\":%d,\"lux\":%d,\"s1\":%d,\"s2\":%d,\"user\":%d,\"isp\":%d}\r\n", adc_ch[0] ,adc_ch[8], (int)BME280_Hum, (int)BME280_Press, (int)BME280_Temp, (int16_t) lux, sw_s1, sw_s2, sw_user, sw_isp);
    			uint8_t len = strlen(buffer_txd);
    			UART0_Write(&buffer_txd[0], len);
    			UART1_Write(&buffer_txd[0], len);
    			state = RECEIVED_CMD;
    			break;

    		case RECEIVED_CMD:
    	    	// lectura de Teclas
    	    	sw_user = SW_USER; sw_isp = SW_ISP;
    	    	sw_s1 = SW_S1; sw_s2 = SW_S2;

    	    	// adc R21, R22, LM35 o CNY70
    	    	if(adc_data_valid == TRUE){
    	        	//PRINTF("CH0 : %d CH7 : %d CH8 : %d\r\n",adc_ch[0] ,adc_ch[7] ,adc_ch[8]);
    	        	adc_data_valid = FALSE;
    	            ADC_DoSoftwareTriggerConvSeqA(ADC0);
    	    	}

    	    	// intenidad de luz
    	        lux = BH1750_ReadLight(BH1750_CONT_H_RES_MODE, BH1750_DEFAULT_MTREG);
    	    	//PRINTF("LUX : %d \r\n",(int16_t) lux);

    	    	// temp, press, hum
    	    	BME280_Data_Normal();
    	        //PRINTF("%d deg C, %d hPa, %0d%%\n", (int)BME280_Temp, (int)BME280_Press, (int)BME280_Hum);
    			rx0 = UART0_Read();
    			rx1 = UART1_Read();
    			if(rx0 > 0 || rx1 > 0){
    				// json de recepcion
    				// {"leds":1,"buzzer":1,"dac":1,"segments":88}
    				//(void)sprintf(&buffer_rxd[0],"{\"id\":\"x\",\"leds\":1,\"buzzer\":1,\"dac\":1,\"segments\":88}");
    				int len0 = strlen(&buffer_rxd0[0]) - 1;
    				int len1 = strlen(&buffer_rxd1[0]) - 1;
    				// recepcion de uart0
    				if(len0 > 0){
    					len0 = json_parse(&buffer_rxd0[0]);
    					if(len0 == JSON_SUCCESS){
    						device0.leds = value_keys[0];
    						device0.buzzer = value_keys[1];
    						device0.dac = value_keys[2];
    						device0.segments = value_keys[3];
    						valid_rxd = 1;
    					}
    				}
    				if(len1 > 0){
    					len1 = json_parse(&buffer_rxd1[0]);
    					if(len1 == JSON_SUCCESS){
    						device1.leds = value_keys[0];
    						device1.buzzer = value_keys[1];
    						device1.dac = value_keys[2];
    						device1.segments = value_keys[3];
    						valid_rxd = 2;
    					}
    				}
    				// verifico si es necesario actualizar
    				if(valid_rxd != 0){
    					if(valid_rxd == 1 ){
        					if(update.leds != device0.leds){
        					    write_update = CMD_UPDATE;
        					    update.leds = device0.leds;
        					}
        					if(update.buzzer != device0.buzzer){
        					    write_update = CMD_UPDATE;
        					    update.buzzer = device0.buzzer;
        					}
        					if(update.dac != device0.dac){
        					    write_update = CMD_UPDATE;
        					    update.dac = device0.dac;
        					}
        					if(update.segments != device0.segments){
        					    write_update = CMD_UPDATE;
        					    update.segments = device0.segments;
        					}
    					} else{
        					if(update.leds != device1.leds){
        					    write_update = CMD_UPDATE;
        					    update.leds = device1.leds;
        					}
        					if(update.buzzer != device1.buzzer){
        					    write_update = CMD_UPDATE;
        					    update.buzzer = device1.buzzer;
        					}
        					if(update.dac != device1.dac){
        					    write_update = CMD_UPDATE;
        					    update.dac = device1.dac;
        					}
        					if(update.segments != device1.segments){
        					    write_update = CMD_UPDATE;
        					    update.segments = device1.segments;
        					}
    					}
    					// actualizo estados del MCU
    					if(update.leds == 0){
    					    RED_LED_OFF; GREEN_LED_OFF; BLUE_LED_OFF;
    					}
    					if(update.leds == 1){
    					    RED_LED_ON; GREEN_LED_OFF; BLUE_LED_OFF;
    					}
    					if(update.leds == 2) {
    					    GREEN_LED_ON; RED_LED_OFF; BLUE_LED_OFF;
    					}
    					if(update.leds == 3) {
    					    BLUE_LED_ON; RED_LED_OFF; GREEN_LED_OFF;
    					}
    					// Buzzer
    					if(update.buzzer == 0) BUZZER_OFF;
    					else BUZZER_ON;
    					// DAC
    					if(update.dac > DAC_LIMIT) update.dac = DAC_LIMIT;
    					DAC1_Write(update.dac);
    					// digitos lcd 7 seg
    					digitos_7seg = update.segments;
    					valid_rxd = 0;
    					count = 0;
	   					state = SEND_CMD;
    				}
    				for(int i = 0; i<30; i++){
    					buffer_rxd0[0] = 0;
    					buffer_rxd1[0] = 0;
    				}
    			}
    			if(count < 10){
    				count++;
    				Delay_mseg(50);
    			} else{
    				count = 0;
    				state = SEND_CMD;
    			}
    			break;
    	}
    }
}
//------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------
void Tick_1mseg_interrupt(void){
	// display 7 segmentos
	if(lcd_switch!=0) lcd_switch--;
	else{
		lcd_switch = new_refresh;
		if(dig_on != DIGIT_1){
			dig_on = DIGIT_1;
			Write_7segment(dig[0]-0x30,1,dig_on);
		}
		else{
			dig_on = DIGIT_2;
			Write_7segment(dig[1]-0x30,1,dig_on);
		}
	}
}
//------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------
void ctimer_match0_interrupt(void){
}
//------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------
