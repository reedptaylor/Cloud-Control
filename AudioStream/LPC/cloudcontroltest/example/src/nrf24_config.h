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
#define SSP_BITRATE	                5000000

//#define CSN_PIN 18
//#define CSN_PORT 1
//
//#define CE_PIN 21
//#define CE_PORT 2

#define CLK_PIN 0
#define CLK_PORT 1

#define MOSI_PIN 1
#define MOSI_PORT 1

#define MISO_PIN 4
#define MISO_PORT 1

#define CSN_PIN 6
#define CSN_PORT 1

#define CE_PIN 5
#define CE_PORT 1
