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
#define SAVESIZE 16000

static uint8_t send_flag;
uint32_t polling_data = 0;
uint32_t saveddata[SAVESIZE];
int i = 0;
int j = 0;

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/



/*****************************************************************************
 * Private functions
 ****************************************************************************/

/* Polling routine for I2S example */
static void App_Polling_Test(void)
{
	DEBUGOUT("I2S Polling mode\n\r");
	while (1) {
		if (Chip_I2S_GetRxLevel(LPC_I2S) > 0) {
			polling_data = Chip_I2S_Receive(LPC_I2S);
			if (i < SAVESIZE) {
				saveddata[i] = polling_data;
				i++;
			}
			else {
				send_flag = 1;
			}
//			polling_data+=20;
//			if (polling_data == 0xFFFF) {
//				polling_data = 0;
//			}
//			send_flag = 1;
		}
		if ((Chip_I2S_GetTxLevel(LPC_I2S) < 4) && (send_flag == 1)) {
			if (j < SAVESIZE) {
				Chip_I2S_Send(LPC_I2S, saveddata[j]);
				j++;
			}
			else {
				j=0;
				Chip_I2S_Send(LPC_I2S, saveddata[j]);
			}
//			Chip_I2S_Send(LPC_I2S, polling_data);
//			printf("polling data: %d\n\r", polling_data);
			send_flag = 0;
		}
	}
}


/*****************************************************************************
 * Public functions
 ****************************************************************************/


/**
 * @brief  Main routine for I2S example
 * @return Nothing
 */
int main(void)
{
	I2S_AUDIO_FORMAT_T audio_Confg;
	audio_Confg.SampleRate = 6000;
	/* Select audio data is 2 channels (1 is mono, 2 is stereo) */
	audio_Confg.ChannelNumber = 1;
	/* Select audio data is 16 bits */
	audio_Confg.WordWidth = 16;

	SystemCoreClockUpdate();
	Board_Init();

#if defined( __GNUC__ )
	__sys_write(0, "", 0);
#endif


	Board_Audio_Init(LPC_I2S, UDA1380_LINE_IN);
	Chip_I2S_Init(LPC_I2S);
	Chip_I2S_RxConfig(LPC_I2S, &audio_Confg);
	Chip_I2S_TxConfig(LPC_I2S, &audio_Confg);

	Chip_I2S_TxStop(LPC_I2S);
	Chip_I2S_DisableMute(LPC_I2S);
	Chip_I2S_TxStart(LPC_I2S);
	send_flag = 0;
	while (1) {
		App_Polling_Test();
	}
	return 0;
}
