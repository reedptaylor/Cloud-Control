/*
===============================================================================
 Name        : nrf24.c
 Author      : $(author)
 Version     :
 Copyright   : $(copyright)
 Description : main definition
===============================================================================
*/

#if defined (__USE_LPCOPEN)
#if defined(NO_BOARD_LIB)
#include "chip.h"
#else
#include "board.h"
#endif
#endif

#include "nrf24.h"

int main(void) {

#if defined (__USE_LPCOPEN)
#if !defined(NO_BOARD_LIB)
    SystemCoreClockUpdate();
    Board_Init();

    Board_LED_Set(0, false);
	if (SetupNRF())
		Board_LED_Toggle(0); // using LED to confirm working!

    const uint8_t pipes[][6] = {"1Node", "2Node"};
    openWritingPipe(pipes[1]);
    openReadingPipe(1, pipes[0]);

	startListening();

	printf("Hello World!\n");


#endif
#endif

	unsigned long read_payload;
    while(1) {
    	if (available(NULL))
    	{
    		//stopListening();
    		read(&read_payload, sizeof(unsigned long));
    		//printf("Read payload: %u\n", read_payload);
    		//startListening();
    	}
    	//delayMs(250);
    }
    return 0 ;
}

