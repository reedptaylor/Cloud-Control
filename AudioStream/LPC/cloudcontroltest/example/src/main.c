/*
 * @brief I2S example
 * This example show how to use the I2S in 3 modes : Polling, Interrupt and DMA
 *
 * @note
 * Copyright(C) NXP Semiconductors, 2012
 * All rights reserved.
 *
 * @par
 * Software that is described herein is for illustrative purposes only
 * which provides customers with programming information regarding the
 * LPC products.  This software is supplied "AS IS" without any warranties of
 * any kind, and NXP Semiconductors and its licensor disclaim any and
 * all warranties, express or implied, including all implied warranties of
 * merchantability, fitness for a particular purpose and non-infringement of
 * intellectual property rights.  NXP Semiconductors assumes no responsibility
 * or liability for the use of the software, conveys no license or rights under any
 * patent, copyright, mask work right, or any other intellectual property rights in
 * or to any products. NXP Semiconductors reserves the right to make changes
 * in the software without notification. NXP Semiconductors also makes no
 * representation or warranty that such application will be suitable for the
 * specified use without further testing or modification.
 *
 * @par
 * Permission to use, copy, modify, and distribute this software and its
 * documentation is hereby granted, under NXP Semiconductors' and its
 * licensor's relevant copyrights in the software, without fee, provided that it
 * is used in conjunction with NXP Semiconductors microcontrollers.  This
 * copyright, permission, and disclaimer notice must appear in all copies of
 * this code.
 */

#include "board.h"
#include "uda1380.h"

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/

void SystemInit(void)
{
	Board_SetupMuxing();
	Chip_SetupIrcClocking();
}

/**
 * @brief  Main routine for I2S example
 * @return Nothing
 */
int main(void)
{
	I2S_AUDIO_FORMAT_T audio_Confg;
	audio_Confg.SampleRate = 6000; //half of sending speed
	/* Select audio data is 2 channels (1 is mono, 2 is stereo) */
	audio_Confg.ChannelNumber = 1;
	/* Select audio data is 16 bits */
	audio_Confg.WordWidth = 16;

	SystemCoreClockUpdate();
	Board_Init();
	SetupNRF();

#if defined( __GNUC__ )
	__sys_write(0, "", 0);
#endif

	const uint8_t pipes[][6] = {"1Node", "2Node"};
	openWritingPipe(pipes[1]);
	openReadingPipe(1, pipes[0]);

	Board_Audio_Init(LPC_I2S, UDA1380_LINE_IN);
	Chip_I2S_Init(LPC_I2S);
	Chip_I2S_TxConfig(LPC_I2S, &audio_Confg);

	Chip_I2S_TxStop(LPC_I2S);
	Chip_I2S_DisableMute(LPC_I2S);
	Chip_I2S_TxStart(LPC_I2S);

	startListening();
	int loop = 0;

	uint32_t read_payload[8];
	while(1) {
		if (available(NULL))
		{
			read(&read_payload, sizeof(uint32_t)*8);
			loop = 0;
			while (loop < 8) {
				if (Chip_I2S_GetTxLevel(LPC_I2S) < 4) {
					Chip_I2S_Send(LPC_I2S, read_payload[loop]);
					loop++;
				}
			}
		}
	}

	return 0;
}
