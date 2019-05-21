//
//  sgtl5000drvr.c
//  
//
//  Created by Anna Lee on 5/20/19.
//
//
#include <unistd.h>

#include "sgtl5000drvr.h"

void sgtl_Init(void) {
    //TODO
}

void sgtl_WriteReg(uint_16 reg, uint_16 reg_val) {
    uint8_t buffer[4];
    uint32 result;
    buffer[0] = (uint_8)((reg >> 8) & 0xFF);
    buffer[1] =	(uint_8)(reg & 0xFF);
    buffer[2] =	(uint_8)((reg_val >> 8) & 0xFF);
    buffer[3] =	(uint_8)(reg_val & 0xFF);
    result = write(fd, buffer, 4);


}

void InitCodec() {
    //--------------- Power Supply Configuration----------------
    // NOTE: This next 2 Write calls is needed ONLY if VDDD is
    // internally driven by the chip
    // Configure VDDD level to 1.2V (bits 3:0)
    ///////sgtl_WriteReg(CHIP_LINREG_CTRL, 0x0008);
    // Power up internal linear regulator (Set bit 9)
    ///////sgtl_WriteReg(CHIP_ANA_POWER, 0x7260);
    // NOTE: This next Write call is needed ONLY if VDDD is
    // externally driven
    // Turn off startup power supplies to save power (Clear bit 12 and 13)
    sgtl_WriteReg(CHIP_ANA_POWER, 0x4260);
    // NOTE: The next 2 Write calls is needed only if both VDDA and
    // VDDIO power supplies are less than 3.1V.
    // Enable the internal oscillator for the charge pump (Set bit 11)
    ///////sgtl_WriteReg(CHIP_CLK_TOP_CTRL, 0x0800);
    // Enable charge pump (Set bit 11)
    ///////sgtl_WriteReg(CHIP_ANA_POWER, 0x4A60);
    // NOTE: The next 2 modify calls is only needed if both VDDA and
    // VDDIO are greater than 3.1V
    // Configure the charge pump to use the VDDIO rail (set bit 5 and bit 6)
    sgtl_WriteReg(CHIP_LINREG_CTRL, 0x006C);
    
    //------ Reference Voltage and Bias Current Configuration----------
    // NOTE: The value written in the next 2 Write calls is dependent
    // on the VDDA voltage value.
    // Set ground, ADC, DAC reference voltage (bits 8:4). The value should
    // be set to VDDA/2. This example assumes VDDA = 1.8V. VDDA/2 = 0.9V.
    // The bias current should be set to 50% of the nominal value (bits 3:1)
    sgtl_WriteReg(CHIP_REF_CTRL, 0x004E);
    // Set LINEOUT reference voltage to VDDIO/2 (1.65V) (bits 5:0) and bias current
    // (bits 11:8) to the recommended value of 0.36mA for 10kOhm load with 1nF
    // capacitance
    sgtl_WriteReg(CHIP_LINE_OUT_CTRL, 0x0322);
    
    //----------------Other Analog Block Configurations------------------
    // Configure slow ramp up rate to minimize pop (bit 0)
    sgtl_WriteReg(CHIP_REF_CTRL, 0x004F);
    // Enable short detect mode for headphone left/right
    // and center channel and set short detect current trip level
    // to 75mA
    sgtl_WriteReg(CHIP_SHORT_CTRL, 0x1106);
    // Enable Zero-cross detect if needed for HP_OUT (bit 5) and ADC (bit 1)
    sgtl_WriteReg(CHIP_ANA_CTRL, 0x0133);
    
    //----------------Power up Inputs/Outputs/Digital Blocks-------------
    // Power up LINEOUT, HP, ADC, DAC
    sgtl_WriteReg(CHIP_ANA_POWER, 0x6AFF);
    // Power up desired digital blocks
    // I2S_IN (bit 0), I2S_OUT (bit 1), DAP (bit 4), DAC (bit 5),
    // ADC (bit 6) are powered on
    sgtl_WriteReg(CHIP_DIG_POWER, 0x0073);
    
    //--------------------Set LINEOUT Volume Level-----------------------
    // Set the LINEOUT volume level based on voltage reference (VAG)
    // values using this formula
    // Value = (int)(40*log(VAG_VAL/LO_VAGCNTRL) + 15)
    // Assuming VAG_VAL and LO_VAGCNTRL is set to 0.9V and 1.65V respectively, the
    // left LO volume (bits 12:8) and right LO volume (bits 4:0) value should be set
    // to 5
    sgtl_WriteReg(CHIP_LINE_OUT_VOL, 0x0505);
    
    //--------------------System MCLK and Sample Clock-------------------
    // Configure SYS_FS clock to 48kHz
    // Configure MCLK_FREQ to 256*Fs
    sgtl_ModifyReg(CHIP_CLK_CTRL, 0xFFC8, 0x0008); // bits 3:2
    // Configure the I2S clocks in master mode
    // NOTE: I2S LRCLK is same as the system sample clock
    // Data length = 16 bits
    sgtl_ModifyReg(CHIP_I2S_CTRL, 0xFFFF, 0x01B0); // bit 7
    
    //----------------------Input/Output Routing-------------------------
    // I2S_IN -> DAC -> HP_OUT
    // Route I2S_IN to DAC
    sgtl_ModifyReg(CHIP_SSS_CTRL, 0xFFDF, 0x0010);
    // Select DAC as the input to HP_OUT
    sgtl_ModifyReg(CHIP_ANA_CTRL, 0xFFBF, 0x0000);

    
    //---------------- Volume and Mute Control---------------------
    // Configure HP_OUT left and right volume to minimum, unmute
    // HP_OUT and ramp the volume up to desired volume.
    sgtl_WriteReg(CHIP_ANA_HP_CTRL, 0x7F7F);
    // Code assumes that left and right volumes are set to same value
    sgtl_WriteReg(CHIP_ANA_HP_CTRL, 0x0000);
    sgtl_ModifyReg(CHIP_ANA_CTRL, 0xFFEF, 0x0000); // bit 5
    // LINEOUT and DAC volume control
    sgtl_ModifyReg(CHIP_ANA_CTRL, 0xFEFF, 0x0000); // bit 8
    // Configure DAC left and right digital volume. Example shows
    // volume of 0dB
    sgtl_WriteReg(CHIP_DAC_VOL, 0x3C3C);
    sgtl_ModifyReg(CHIP_ADCDAC_CTRL, 0xFFFB, 0x0000); // bit 2
    sgtl_ModifyReg(CHIP_ADCDAC_CTRL, 0xFFF7, 0x0000); // bit 3

}
