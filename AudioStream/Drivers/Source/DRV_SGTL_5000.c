#ifndef DRV_SGTL_5000_C
#define DRV_SGTL_5000_C

#ifdef  __cplusplus
extern "C"
{
#endif

#include "chip.h"

/* DEFAULT SGTL5000 VALUES */
/* System Register Data Set */
static const uint8_t SGTL_sys_regs_dat[] = {
	SGTL_EVALM_CLK,
	SGTL_U8(SGTL_REG_EVALCLK_DVAL),
	SGTL_U8(SGTL_REG_I2S_DVAL),
	SGTL_U8(SGTL_REG_PWRCTRL_DVAL),
	SGTL_U8(SGTL_REG_ANAMIX_DVAL),
	SGTL_U8(SGTL_REG_HEADAMP_DVAL)
};
	
static const uint8_t SGTL_interfil_regs_dat[] = {
	SGTL_CHIP_LINE_OUT_CTRL,
	SGTL_U8(SGTL_REG_MSTRVOL_DVAL),
	SGTL_U8(SGTL_REG_MIXVOL_DVAL),
	SGTL_U8(SGTL_REG_MODEBBT_DVAL),
	SGTL_U8(SGTL_REG_MSTRMUTE_DVAL),
	SGTL_U8(SGTL_REG_MIXSDO_DVAL)
};
	
static const uint8_t SGTL_decimator_regs_dat[] = {
	SGTL_CHIP_ADCDAC_CTRL,
	SGTL_U8(SGTL_REG_DECVOL_DVAL),
	SGTL_U8(SGTL_REG_ANA_ADC_DVAL),
	SGTL_U8(SGTL_REG_ADC_DVAL),
	SGTL_U8(SGTL_REG_AVC_DVAL)
};

/* PRIVATE FUNCTIONS */
static int SGTL_5000_SetDefaultValues(const uint8_t *values, int sz) {
	int return_value;
	uint8_t verification_buffer[10];
	if (return_value) {
		return_value = SGTL_REG_VerifyMult(values[0], &values[1], verification_buffer, sz-1);
	}
	return return_value;
}

/* PUBLIC FUNCTIONS */
void SGTL_REG_Write(uint8_t reg, uint16_t val) {
	uint8_t dat[3];
	dat[0] = reg;
	dat[1] = val >> 8;
	dat[2] = val & 0xFF;
	Chip_I2C_MasterSend(SGTL_I2C_BUS, SGTL_I2C_ADDR, dat, sizeof(dat));	//TODO
}

void SGTL_REG_Read(uint8_t reg) {
	uint8_t rx_data[2];
	if (Chip_I2C_MasterCmdRead(SGTL_I2C_BUS, SGTL_I2C_ADDR, reg, rx_data, 2) == 2) {	//TODO
		return (rx_data[0] << 8) | rx_data[1];
	}
	return 0;
}

int SGTL_REG_WriteVerify(uint8_t reg, uint16_t val) {
	uint16_t ret;
	SGTL_REG_Write(reg, val);
	ret = SGTL_REG_Read(reg);
	return ret == val;
}

int SGTL_REG_WriteMult(const uint8_t *buff, int len) {
	return Chip_I2C_MasterSend(SGTL_I2C_BUS, SGTL_I2C_ADDR, buff, len) == len;	//TODO
}
	
int SGTL_REG_VerifyMult(uint8_t reg, const uint8_t *value, uint8_t *buff, int len) {
	int i;
	if (Chip_I2C_MasterCmdRead(SGTL_I2C_BUS, SGTL_I2C_ADDR, reg, buff, len) != len) {	//TODO
		return 0;	// partial read
	}
	for (i = 0; i < len; i++) {
		if (value[i] != buff[i]) {
			break;
		}
	}
	return i == len;
}

int SGTL5000_I2C_Init(int input) {
	I2C_EVENTHANDLER_T old = Chip_I2C_GetMasterEventHandler(SGTL_I2C_BUS);
	int ret;
	Board_I2C_Init(SGTL_I2C_BUS);
	Chip_I2C_Init(SGTL_I2C_BUS);
	Chip_I2C_SetClockRate(SGTL_I2C_BUS, 100000);
	Chip_I2C_SetMasterEventHandler(SGTL_I2C_BUS, Chip_I2C_EventHandlerPolling);
	
	ret = SGTL_5000_SetDefaultValues(SGTL_sys_regs_dat, sizeof(SGTL_sys_regs_dat));
	if (ret) {
		ret = SGTL_5000_SetDefaultValues(SGTL_interfil_regs_dat, sizeof(SGTL_interfil_regs_dat));
	}
	if (ret) {
		ret = SGTL_5000_SetDefaultValues(SGTL_decimator_regs_dat, sizeof(SGTL_decimator_regs_dat));
	}
	
	Chip_I2C_SetMasterEventHandler(SGTL_I2C_BUS, old);
	return ret;
}
	
#ifdef  __cplusplus
}
#endif

#endif /* DRV_SGTL_C */ 
