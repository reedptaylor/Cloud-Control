/*
===============================================================================
 Name        : nrf24_config.h
 Author      : $(author)
 Version     :
 Copyright   : $(copyright)
 Description : main definition
===============================================================================
*/

/*Settings*/

#define LPC_SSP           			LPC_SSP2
#define SSP_DATA_BITS               (SSP_BITS_8)
#define SSP_IRQ                     SSP2_IRQn
#define SSPIRQHANDLER               SSP2_IRQHandler

#define CSN_PIN 18
#define CSN_PORT 1

#define CE_PIN 21
#define CE_PORT 2
