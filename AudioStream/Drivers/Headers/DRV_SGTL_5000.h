/*	
*	DRV_SGTL_5000.h
*	
*	Provides definitions to be used in DRV_SGTL_5000.c 
*/

#ifndef DRV_SGTL_5000_H
#define DRV_SGTL_5000_H

/********** SGTL Registers **********/
#define SGTL_I2C_ADDR				(0xA)
#define SGTL_CHIP_ID				(0x0000)
#define SGTL_CHIP_DIG_PWR			(0x0002)
/*
#define DIG_PWR_DAC_PWRUP_MASK			(0x20)		// 1 = ENABLE; 0 = DISABLE DAC block
#define DIG_PWR_DAC_PWRUP_SHIFT			(0x5)
#define DIG_PWR_I2S_OUT_PWRUP_MASK		(0x2)		// 1 = ENABLE; 0 = DISABLE i2s data output
#define DIG_PWR_I2S_OUT_PWRUP_SHIFT		(0x1)
#define DIG_PWR_I2S_IN_PWRUP_MASK		(0x1)		// 1 = ENABLE; 0 = DISABLE i2s data input
#define DIG_PWR_I2S_IN_PWRUP_SHIFT		(0x0)
*/

#define SGTL_CHIP_CLK_CTRL			(0x0004)
/*
#define CLK_CTRL_RATE_MODE_MASK			(0x30)		// Sets the sample rate mode. MCLK_FREQ is still specified relative to the rate in SYS_FS
#define CLK_CTRL_RATE_MODE_SHIFT		(0x4)
//		0x0 = SYS_FS specifies the rate
//		0x1 = Rate is 1/2 of the SYS_FS rate
//		0x2 = Rate is 1/4 of the SYS_FS rate
//		0x3 = Rate is 1/6 of the SYS_FS rate
#define CLK_CTRL_SYS_FS_MASK			(0xC)		// Sets the internal system sample rate (default=2)
#define CLK_CTRL_SYS_FS_SHIFT			(0x2)
//		0x0 = 32 kHz		1:0
//		0x1 = 44.1 kHz
//		0x2 = 48 kHz
//		0x3 = 96 kHz
*/
#define SGTL_CHIP_I2S_CTRL			(0x0006)
#define I2S_CTRL_SCLKFREQ_MASK			(0x100)		// Sets frequency of I2S_SCLK when in master mode (MS=1). When in slave
								// mode (MS=0), this field must be set appropriately to match SCLK input rate
#define I2S_CTRL_SCLKFREQ_SHIFT			(0x8)
//		0x0 = 64Fs, 0x1 = 32Fs - Not supported for RJ mode (I2S_MODE = 1)
#define I2S_CTRL_MS_MASK			(0x80)		// Configures master or slave of I2S_LRCLK and I2S_SCLK.
#define I2S_CTRL_MS_SHIFT			(0x0)
//		0x0 = Slave: I2S_LRCLK an I2S_SCLK are inputs
//		0x1 = Master: I2S_LRCLK and I2S_SCLK are outputs
//		NOTE: If the PLL is used (CHIP_CLK_CTRL->MCLK_FREQ==0x3), the SGTL5000 must be a master of the I2S port (MS==1)
#define I2S_CTRL_DLEN_MASK			(0x30)		// I2S data length (default = 1)
#define I2S_CTRL_DLEN_SHIFT			(0x4)
//		0x0 = 32 bits (only valid when SCLKFREQ=0), not valid for Right Justified Mode
//		0x1 = 24 bits (only valid when SCLKFREQ=0)
//		0x2 = 20 bits
//		0x3 = 16 bits
#define I2S_CTRL_MODE_MASK			(0x0C)		// Sets the mode for the I2S port
#define I2S_CTRL_MODE_SHIFT			(0x0)
//		0x0 = I2S mode or Left Justified (Use LRALIGN to select) <--- We are using I2S
//		0x1 = Right Justified Mode
//		0x2 = PCM Format A/B
//		0x3 = RESERVED

#define SGTL_CHIP_SSS_CTRL			(0x000a)
#define SSS_CTRL_DAC_SELECT_MASK		(0x30)		// Select data source for DAC (default = 1)
#define SSS_CTRL_DAC_SELECT_SHIFT		(0x4)
//		0x0 = ADC
//		0x1 = I2S_IN
//		0x2 = Reserved
//		0x3 = DAP

#define SGTL_CHIP_ADCDAC_CTRL			(0x000e)
#define SGTL_CHIP_DAC_VOL			(0x0010)
#define SGTL_CHIP_PAD_STRENGTH			(0x0014)
#define SGTL_CHIP_ANA_ADC_CTRL			(0x0020)
#define SGTL_CHIP_ANA_HP_CTRL			(0x0022)
#define SGTL_CHIP_ANA_CTRL			(0x0024)
//		MAybe need to add some sub defs here for LINEIN / DAC input to HEADPHONE OUT? 
#define SGTL_CHIP_LINREG_CTRL			(0x0026)
#define SGTL_CHIP_REF_CTRL			(0x0028)
#define SGTL_CHIP_MIC_CTRL			(0x002a)
#define SGTL_CHIP_LINE_OUT_CTRL			(0x002c)	//WE ARE USING LINEOUT, NOT ANA
#define SGTL_CHIP_LINE_OUT_VOL			(0x002e)
#define SGTL_CHIP_ANA_POWER			(0x0030)
#define SGTL_CHIP_PLL_CTRL			(0x0032)
#define SGTL_CHIP_CLK_TOP_CTRL			(0x0034)
#define SGTL_CHIP_ANA_STATUS			(0x0036)
#define SGTL_CHIP_SHORT_CTRL			(0x003c)
#define SGTL_CHIP_ANA_TEST2			(0x003a)
// Probably don't need any of the DAP stuff
#define SGTL_DAP_BASS_ENHANCE			(0x0104)
#define SGTL_DAP_BASS_ENHANCE_CTRL		(0x0106)
#define SGTL_DAP_AUDIO_EQ			(0x0108)
#define SGTL_DAP_AUDIO_EQ_BASS_BAND0		(0x0116)
#define SGTL_DAP_AUDIO_EQ_BAND1			(0x0118)
#define SGTL_DAP_AUDIO_EQ_BAND2			(0x011A)
#define SGTL_DAP_AUDIO_EQ_BAND3			(0x011C)
#define SGTL_DAP_AUDIO_EQ_TREBLE_BAND4		(0x011E)
#define SGTL_DAP_AVC_CTRL			(0x0124)
#define SGTL_DAP_AVC_THRESHOLD			(0x0126)
#define SGTL_DAP_AVC_ATTACK			(0x0128)
#define SGTL_DAP_AVC_DECAY			(0x012A)
#define SGTL_DAP_MAIN_CHAN			(0x0120)
#define SGTL_DAP_CONTROL			(0x0100)

/********** SGTL5000 Register Default Values *********/
#define SGTL_REG_EVALCLK_DVAL		(0xF << 8 | 0x3 << 4 | 1 << 1)
#define SGTL_REG_I2S_DVAL		0x0000
#define SGTL_REG_PWRCTRL_DVAL		(1 << 15 | 1 << 13 | 1 << 10 | 1 << 8 | 1 << 6 | 1 << 4 | 0x0F) // fix
#define SGTL_REG_HEADAMP_DVAL		(1 << 9 | 2)
#define SGTL_REG_MSTRVOL_DVAL		0x0000
#define SGTL_REG_ANAMIX_DVAL		0x0000
#define SGTL_REG_MIXVOL_DVAL		0x0000
#define SGTL_REG_MODEBBT_DVAL		0x0000
#define SGTL_REG_MSTRMUTE_DVAL		(2 << 8 | 2)
#define SGTL_REG_MIXSDO_DVAL		0x0000
#define SGTL_REG_DECVOL_DEFAULT_VALUE     0xE4E4	/* Decrease Volume -28dB */
#define SGTL_REG_PGA_DEFAULT_VALUE        0x0000
#define SGTL_REG_ADC_DEFAULT_VALUE        0x0001	/* Apply 0bB VGA Gain, enable DC Filter */
#define SGTL_REG_AVC_DEFAULT_VALUE        0x0000	// this is AGC in UDA1380, auto gain control

/********** Evaluation mode and clock setting register bits **********/
#define EVCLK_EV2             (1 << 15)
#define EVCLK_EV1             (1 << 14)
#define EVCLK_EV0             (1 << 13)
#define EVCLK_EN_ADC          (1 << 11)
#define EVCLK_EN_DEC          (1 << 10)
#define EVCLK_EN_DAC          (1 << 9)
#define EVCLK_EN_INT          (1 << 8)
#define EVCLK_ADC_CLK         (1 << 5)
#define EVCLK_DAC_CLK         (1 << 4)
#define EVCLK_SYS_DIV1        (1 << 3)
#define EVCLK_SYS_DIV0        (1 << 2)
#define EVCLK_PLL1            (1 << 1)
#define EVCLK_PLL0            (1 << 0)

/********** SGTL5000 Audio Input selection *********/
#define SGTL_LINE_IN 	0		// LINE_IN_L in left stream, LINE_IN_R in right stream
#define SGTL_MIC_IN_L	(1 << 2)	// MIC audio in Left stream, LINE_IN_R in right stream
#define SGTL_MIC_IN_LR	(3 << 2)	// MIC audio in Left & Right stream

/********** Some functions **********/
#define SGTL_U8(val)	(((val) >> 8) & 0xFF), ((val) & 0xFF)

// Write a 16-bit value to codec's register
void SGTL_REG_write(uint8_t reg, uint16_t val);

// Read a 16-bit value from codec's register (reg)
uint16_t SGTL_REG_read(uint8_t reg);

// Write a value to chip register (reg), reads back and verifies value (val)
int SGTL_REG_WriteVerify(uint8_t reg, uint16_t val);

/* Write multiple values to codec's registers
* @param buff : Pointer to buffer, buff[0] must be address of reg to which
* 		the first data (ie. buff[1], buff[2]) be written, the next
*		bytes buff[3],buff[4] be written to reg buff[0]+1 and so on
* @param len  : Number of bytes in buff
* @return 	1 on success, 0 on failure
*/
int SGTL_REG_WriteMulti(const uint8_t *buff, int len);

/* Verify values in multiple codec registers
* @param reg   : Starting register from which data be read
* @param value : Pointer to memory which contains values to be compared
* @param buff  : Pointer to memory to which data be read
* @param len   : Length of bytes in value @a buff
* @return 	1 on success & data is valid, 0 on failure
*/
int SGTL_REG_VerifyMult(uint8_t reg, const uint8_t *value, uint8_t *buff, int len);

/* Initialize SGTL5000 to its default state
* @param input : Audio input source (must be #SGTL_LINE_IN/#SGTL_MIC_IN_L/#SGTL_MIC_IN_LR)
* @return	1 on success, 0 on failure
*/
int SGTL5000_Init(int input);	//TODO: EDIT

/********** I2C Event *********/
#define SGTL_CHIP_I2C_EVENT_TRANSFER_DONE       (1UL << 0)  ///< Master/Slave Transmit/Receive finished
#define SGTL_CHIP_I2C_EVENT_TRANSFER_INCOMPLETE (1UL << 1)  ///< Master/Slave Transmit/Receive incomplete transfer
#define SGTL_CHIP_I2C_EVENT_SLAVE_TRANSMIT      (1UL << 2)  ///< Addressed as Slave Transmitter but transmit operation is not set.
#define SGTL_CHIP_I2C_EVENT_SLAVE_RECEIVE       (1UL << 3)  ///< Addressed as Slave Receiver but receive operation is not set.
#define SGTL_CHIP_I2C_EVENT_ADDRESS_NACK        (1UL << 4)  ///< Address not acknowledged from Slave
#define SGTL_CHIP_I2C_EVENT_GENERAL_CALL        (1UL << 5)  ///< Slave addressed with general call address
#define SGTL_CHIP_I2C_EVENT_ARBITRATION_LOST    (1UL << 6)  ///< Master lost arbitration
#define SGTL_CHIP_I2C_EVENT_BUS_ERROR           (1UL << 7)  ///< Bus error detected (START/STOP at illegal position)
#define SGTL_CHIP_I2C_EVENT_BUS_CLEAR           (1UL << 8)  ///< Bus clear finished

/********** Brief Access structure of the I2C Driver **********/
//TODO:EDIT
                                                               ///< Pointer to \ref : Get I2C status.
} const SGTL_CHIP_DRIVER_I2C;
