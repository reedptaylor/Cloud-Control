#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <ctype.h>
#include "portaudio.h"

#define FRAMES_PER_BUFFER (512)
#define PA_SAMPLE_TYPE  paFloat32
typedef float SAMPLE;
#define SAMPLE_SILENCE  (0.0f)
#define NUM_CHANNELS    (2)
#define SAMPLE_RATE  (44100)
#define NUM_SECONDS     (5)

int main(int argc, char *argv[])
{
    // check to make sure that the user entered only the port number to run the server on
    if(argc == 2) {

        PaStreamParameters  outputParameters;
        PaStream*           stream;
        float*      data = 0;

        int serverPort = atoi(argv[1]);

        while(1) {

            // create a socket with UDP
            int sockfd = socket(AF_INET, SOCK_STREAM, 0);

            // initialize structs for the server address and client address
            struct sockaddr_in serverAddress, clientAddress;

            // set the IP address and port for the server address
            serverAddress.sin_family = AF_INET;
            serverAddress.sin_addr.s_addr = INADDR_ANY;
            serverAddress.sin_port = htons(serverPort);

            // bind the socket to the server address/port
            bind(sockfd, (struct sockaddr *)&serverAddress,  sizeof(serverAddress));

            // listen on the server address/port for a connection
            listen(sockfd, 5);

            // when a connection is found, accept it with a new file descriptor
            socklen_t clientAddressSize = sizeof(clientAddress);
            int connectionfd = accept(sockfd, (struct sockaddr *)&clientAddress, &clientAddressSize);
            printf("connectionfd: %d\n", connectionfd);
            printf("Connected by IP: %d, Port: %d\n", clientAddress.sin_addr.s_addr, clientAddress.sin_port);

            Pa_Initialize();
            outputParameters.device = Pa_GetDefaultOutputDevice(); /* default output device */
            if (outputParameters.device == paNoDevice) {
                fprintf(stderr,"Error: No default output device.\n");
            }
            outputParameters.channelCount = 2;                     /* stereo output */
            outputParameters.sampleFormat =  PA_SAMPLE_TYPE;
            outputParameters.suggestedLatency = Pa_GetDeviceInfo( outputParameters.device )->defaultLowOutputLatency;
            outputParameters.hostApiSpecificStreamInfo = NULL;

            printf("\n=== Now playing back. ===\n"); fflush(stdout);
            Pa_OpenStream(
                      &stream,
                      NULL, /* no input */
                      &outputParameters,
                      SAMPLE_RATE,
                      FRAMES_PER_BUFFER,
                      paClipOff,      /* we won't output out of range samples so don't bother clipping them */
                      NULL,
                      NULL );
            Pa_StartStream( stream );
            for (int i = 0; i < SAMPLE_RATE / FRAMES_PER_BUFFER * NUM_SECONDS; i++) {
                printf("%d recieving: %f\n", i, *data);
                recv(connectionfd, data, 4, 0);
                // Pa_WriteStream( stream, &data, FRAMES_PER_BUFFER );

            }
            Pa_StopStream( stream );
            Pa_CloseStream( stream );
            Pa_Terminate();
            close(sockfd);
            close(connectionfd);
        }

    }
    return 0;
}
