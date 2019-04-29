/*
===============================================================================
 Name        : nrf24.h
 Author      : $(author)
 Version     :
 Copyright   : $(copyright)
 Description : main definition
===============================================================================
*/

/*Function definitions*/

void openWritingPipe(const uint8_t *address);

void openReadingPipe(uint8_t child, const uint8_t *address);

void startListening(void);

void stopListening(void);

void read(void* buf, uint8_t len);

bool available(uint8_t* pipe_num);

bool SetupNRF(void);
