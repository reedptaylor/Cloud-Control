/*
===============================================================================
 Name        : nrf24.c
 Author      : $(author)
 Version     :
 Copyright   : $(copyright)
 Description : main definition
===============================================================================
*/

#include "chip.h"
#include "board.h"
#include "nRF24L01.h"
//#include "nrf24.h"
#include "nrf24_config.h"

#define _BV(x) (1<<(x))
#define rf24_min(a,b) (a<b?a:b)
#define pgm_read_byte(p) (*(p))

static SSP_ConfigFormat ssp_format;
static Chip_SSP_DATA_SETUP_T xf_setup;

#define SSP_DATA_BIT_NUM(databits)          (databits + 1)
#define SSP_DATA_BYTES(databits)            (((databits) > SSP_DATA_BITS) ? 2 : 1)

#define BUFFER_SIZE                         (0x1)

//Delays stolen from board.c
/* Very simple (inaccurate) delay function */
static void delay(uint32_t i)
{
	while (i--) {}
}

/* Delay in miliseconds  (cclk = 120MHz) */
static void delayMs(uint32_t ms)
{
	delay(ms * 40000);
}

/* Delay in microseconds  (cclk = 120MHz) */
static void delayUs(uint32_t us)
{
	delay(us * 40);
}

/**
 * Power Amplifier level.
 *
 * For use with setPALevel()
 */
typedef enum { RF24_PA_MIN = 0,RF24_PA_LOW, RF24_PA_HIGH, RF24_PA_MAX, RF24_PA_ERROR } rf24_pa_dbm_e ;

/**
 * Data rate.  How fast data moves through the air.
 *
 * For use with setDataRate()
 */
typedef enum { RF24_1MBPS = 0, RF24_2MBPS, RF24_250KBPS } rf24_datarate_e;

/**
 * CRC Length.  How big (if any) of a CRC is included.
 *
 * For use with setCRCLength()
 */
typedef enum { RF24_CRC_DISABLED = 0, RF24_CRC_8, RF24_CRC_16 } rf24_crclength_e;


/* Tx buffer */
static uint8_t Tx_Buf[1];

/* Rx buffer */
static uint8_t Rx_Buf[1];

//Radio values
uint8_t payload_size = 32;
uint8_t addr_width = 5; /**< The address width to use - 3,4 or 5 bytes. */
uint32_t csDelay = 5;
uint32_t txDelay = 65;
uint8_t pipe0_reading_address[5]; /**< Last address set on pipe 0 for reading. */

void csn (bool state)
{
	Chip_GPIO_SetPinState(LPC_GPIO, CSN_PORT, CSN_PIN, state);
}

void ce (bool state)
{
	Chip_GPIO_SetPinState(LPC_GPIO, CE_PORT, CE_PIN, state);
}

uint8_t transferSSPData (uint8_t data)
{
	xf_setup.rx_cnt = xf_setup.tx_cnt = 0;

	Rx_Buf[0] = 0;
	Tx_Buf[0] = data;

	Chip_SSP_RWFrames_Blocking(LPC_SSP, &xf_setup);
	return Rx_Buf[0];
}

uint8_t write_payload (const void* buf, uint8_t data_len, const uint8_t writeType)
{
	uint8_t status;
	const uint8_t* current = (const uint8_t*)buf;

	csn(false);
	status = transferSSPData (writeType);
	while(data_len--)
		transferSSPData (*current++);
	// If using dynamic payloads (<32 bytes), implement
	csn(true);

	return status;
}

uint8_t read_payload (void* buf, uint8_t data_len)
{
	uint8_t status;
	uint8_t* current = (uint8_t*)buf;

	uint8_t blank_len = 32 - data_len;

	csn(false);
	status = transferSSPData(R_RX_PAYLOAD);
	while(data_len--)
		*current++ = transferSSPData(0xFF);
	while(blank_len--)
		transferSSPData(0xFF);
	// If using dynamic payloads (<32 bytes), implement
	csn(true);

	return status;
}

uint8_t write_register_long(uint8_t reg, const uint8_t* buf, uint8_t len)
{
	uint8_t status;

	csn(false);
	status = transferSSPData( W_REGISTER | ( REGISTER_MASK & reg ) );
	while ( len-- )
		transferSSPData(*buf++);
	csn(true);

	return status;
}

uint8_t write_register(uint8_t reg, uint8_t value)
{
	uint8_t status;

	csn(false);
	status = transferSSPData( W_REGISTER | ( REGISTER_MASK & reg ) );
	transferSSPData(value);
	csn(true);

	return status;
}

uint8_t read_register_long (uint8_t reg, uint8_t* buf, uint8_t len)
{
  uint8_t status;

  csn(false);
  status = transferSSPData( R_REGISTER | ( REGISTER_MASK & reg ) );
  while ( len-- )
    *buf++ = transferSSPData(0xff);
  csn(true);

  return status;
}

uint8_t read_register (uint8_t reg)
{
  uint8_t result;

  csn(false);
  transferSSPData( R_REGISTER | ( REGISTER_MASK & reg ) );
  result = transferSSPData(0xff);
  csn(true);

  return result;
}

uint8_t spiTrans(uint8_t cmd)
{
	uint8_t status;

	csn(false);
	status = transferSSPData(cmd);
	csn(true);

	return status;

}

uint8_t get_status()
{
	return spiTrans(NOP);
}

bool setRetries (uint8_t delay, uint8_t count)
{
	bool result = false;
	uint8_t setup = ((delay & 0xf) << ARD) | ((count & 0xf) << ARC);
	write_register(SETUP_RETR, setup);
	//Verify
	if (read_register(SETUP_RETR) == setup)
		result = true;

	return result;
}

bool setDataRate(rf24_datarate_e speed)
{
	bool result = false;
	uint8_t setup = read_register(RF_SETUP);

	setup &= ~(_BV(RF_DR_LOW) | _BV(RF_DR_HIGH));

	if (speed == RF24_250KBPS )
		setup |= _BV(RF_DR_LOW);
	else if (speed == RF24_2MBPS)
		setup |= _BV(RF_DR_HIGH);

	write_register(RF_SETUP, setup);
	//Verify
	if (read_register(RF_SETUP) == setup)
		result = true;

	return result;
}

void toggle_features(void)
{
	csn(false);
	transferSSPData (ACTIVATE);
	transferSSPData (0x73);
	csn(true);
}

void setChannel(uint8_t channel)
{
	const uint8_t max_channel = 125;
	write_register(RF_CH, rf24_min(channel, max_channel));
}

uint8_t flush_rx(void)
{
	return spiTrans(FLUSH_RX);
}

uint8_t flush_tx(void)
{
	return spiTrans(FLUSH_TX);
}

void powerUp(void)
{
	uint8_t cfg = read_register(CONFIG);

	//if not powered up then power up and wait for the radio to initialize
	if (!(cfg & _BV(PWR_UP)))
	{
		write_register (CONFIG, cfg | _BV(PWR_UP));

		// For nRF24L01+ to go from power down mode to TX or RX mode it must first pass through stand-by mode.
		// There must be a delay of Tpd2stby (see Table 16.) after the nRF24L01+ leaves power down mode before
		// the CEis set high. - Tpd2stby can be up to 5ms per the 1.0 datasheet
		delayMs(5);
	}
}

bool SetupNRF(void)
{
	uint8_t setup = 0;

	//GPIO pins configured to simulate CSN and CE
	Chip_IOCON_PinMux(LPC_IOCON, CSN_PORT, CSN_PIN, IOCON_FUNC0, IOCON_MODE_PULLUP);
	Chip_GPIO_SetPinDIROutput(LPC_GPIO, CSN_PORT, CSN_PIN);
	Chip_IOCON_PinMux(LPC_IOCON, CE_PORT, CE_PIN, IOCON_FUNC0, IOCON_MODE_PULLUP);
	Chip_GPIO_SetPinDIROutput(LPC_GPIO, CE_PORT, CE_PIN);

	ce(false);
	csn(true);

	//Setup SSP interface
	Board_SSP_Init(LPC_SSP);
	Chip_SSP_Init(LPC_SSP);

	ssp_format.frameFormat = SSP_FRAMEFORMAT_SPI;
	ssp_format.bits = SSP_DATA_BITS;
	ssp_format.clockMode = SSP_CLOCK_MODE1;

	Chip_SSP_SetFormat(LPC_SSP, ssp_format.bits, ssp_format.frameFormat, ssp_format.clockMode);
	Chip_SSP_Enable(LPC_SSP);
	Chip_SSP_SetMaster(LPC_SSP, 1);
	Chip_SSP_SetBitRate(LPC_SSP, 6000000);

	xf_setup.length = BUFFER_SIZE;
	xf_setup.tx_data = Tx_Buf;
	xf_setup.rx_data = Rx_Buf;

	// Must allow the radio time to settle else configuration bits will not necessarily stick.
	// This is actually only required following power up but some settling time also appears to
	// be required after resets too. For full coverage, we'll always assume the worst.
	// Enabling 16b CRC is by far the most obvious case if the wrong timing is used - or skipped.
	// Technically we require 4.5ms + 14us as a worst case. We'll just call it 5ms for good measure.
	// WARNING: Delay is based on P-variant whereby non-P *may* require different timing.
	delayMs(5);

	// Reset NRF_CONFIG and enable 16-bit CRC.
	write_register (CONFIG, 0x0C);

	if (!setRetries(5, 15))
		return false;

	if (!setDataRate(RF24_1MBPS))
		return false;

	setup = read_register(RF_SETUP);

	toggle_features();
	write_register(FEATURE, 0);
	write_register(DYNPD, 0);
	//dynamic_payloads_enabled = false;

	// Reset current Status
	// Notice reset and flush is the last thing we do
	write_register(STATUS, _BV(RX_DR) | _BV(TX_DS) | _BV(MAX_RT));

	// Set up default configuration.  Callers can always change it later.
	// This channel should be universally safe and not bleed over into adjacent
	// spectrum.
	setChannel(76);

	// Flush buffers
	flush_rx();
	flush_tx();

	//power up by default when begin() is called
	powerUp();

	write_register(CONFIG, ( read_register(CONFIG) ) & ~_BV(PRIM_RX) );

	return (setup != 0 && setup != 0xFF);
}

static const uint8_t child_pipe[] =
{
  RX_ADDR_P0, RX_ADDR_P1, RX_ADDR_P2, RX_ADDR_P3, RX_ADDR_P4, RX_ADDR_P5
};

static const uint8_t child_payload_size[] =
{
  RX_PW_P0, RX_PW_P1, RX_PW_P2, RX_PW_P3, RX_PW_P4, RX_PW_P5
};

static const uint8_t child_pipe_enable[] =
{
  ERX_P0, ERX_P1, ERX_P2, ERX_P3, ERX_P4, ERX_P5
};

void openWritingPipe(const uint8_t *address)
{
	write_register_long(RX_ADDR_P0, address, addr_width);
	write_register_long(TX_ADDR, address, addr_width);
	write_register(RX_PW_P0, payload_size);
}

void openReadingPipe(uint8_t child, const uint8_t *address)
{
	//if (child == 0)
		// memcpy(pipe0_reading_address, address, addr_width);
	if (child <= 6)
	{
		//For pipes 2-5, only write the LSB
		if (child < 2)
			write_register_long(pgm_read_byte(&child_pipe[child]), address, addr_width);
		else
			write_register_long(pgm_read_byte(&child_pipe[child]), address, 1);
		write_register(pgm_read_byte(&child_payload_size[child]),payload_size);

		write_register(EN_RXADDR, read_register(EN_RXADDR) | _BV(pgm_read_byte(&child_pipe_enable[child])));
	}
}

void closeReadingPipe(uint8_t pipe)
{
	write_register(EN_RXADDR,read_register(EN_RXADDR) & ~_BV(pgm_read_byte(&child_pipe_enable[pipe])));
}

void startListening(void)
{
	write_register(CONFIG, read_register(CONFIG) | _BV(PRIM_RX));
	write_register(STATUS, _BV(RX_DR) | _BV(TX_DS) | _BV(MAX_RT));
	ce(true);

	if(pipe0_reading_address[0] > 0)
		write_register_long(RX_ADDR_P0, pipe0_reading_address, addr_width);
	else
		closeReadingPipe(0);

	if (read_register(FEATURE) & _BV(EN_ACK_PAY))
		flush_tx();
}

void stopListening(void)
{
	ce(false);

	delayUs(txDelay);

	if (read_register(FEATURE) & _BV(EN_ACK_PAY))
	{
		delayUs(txDelay);
		flush_tx();
	}

	write_register(CONFIG, read_register(CONFIG) & ~_BV(PRIM_RX));
	write_register(EN_RXADDR, read_register(EN_RXADDR) | _BV(pgm_read_byte(&child_pipe_enable[0])));
}

void read(void* buf, uint8_t len)
{
	//Fetch the payload
	read_payload(buf, len);

	//Clear the two possible interrupt flags with one command
	write_register(STATUS, _BV(RX_DR) | _BV(MAX_RT) | _BV(TX_DS) );
}

bool available(uint8_t* pipe_num)
{

	if (!( read_register(FIFO_STATUS) & _BV(RX_EMPTY) ))
	{
		// uint8_t readreg = read_register(FIFO_STATUS);
		// printf("Read register: %x\n", readreg);
		if (pipe_num)
		{
			uint8_t status = get_status();
			*pipe_num = (status >> RX_P_NO) & 0x07;
		}
		return 1;
	}
	return 0;
}
