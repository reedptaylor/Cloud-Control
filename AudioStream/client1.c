#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
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
    // check to make sure the user entered both the IP address and the port number
    if(argc == 3) {
        PaStreamParameters  inputParameters;
        PaStream*           stream;
        float*      data = 0;

        char *serverHostname = argv[1];
        int serverPort = atoi(argv[2]);

        struct hostent *hostEntry;
        hostEntry = gethostbyname(serverHostname);
        char *serverIP = inet_ntoa(*((struct in_addr*) hostEntry->h_addr_list[0]));

        // create a socket with UDP
        int sockfd = socket(AF_INET, SOCK_STREAM, 0);

        // create the struct which holds the server IP address and port number
        struct sockaddr_in serverAddress; // initialize the struct
        serverAddress.sin_family = AF_INET; // set the IP address protocol
        serverAddress.sin_port = htons(serverPort); // set the server port number
        inet_pton(AF_INET, serverIP, &serverAddress.sin_addr); // set the server IP address by converting the string to binary format

        // initiate a TCP connection with the server
        connect(sockfd, (struct sockaddr *)&serverAddress, sizeof(serverAddress));

        Pa_Initialize();

        inputParameters.device = Pa_GetDefaultInputDevice(); /* default input device */
        if (inputParameters.device == paNoDevice) {
            fprintf(stderr,"Error: No default input device.\n");
        }
        inputParameters.channelCount = 2;                    /* stereo input */
        inputParameters.sampleFormat = PA_SAMPLE_TYPE;
        inputParameters.suggestedLatency = Pa_GetDeviceInfo( inputParameters.device )->defaultLowInputLatency;
        inputParameters.hostApiSpecificStreamInfo = NULL;

        Pa_OpenStream(
                  &stream,
                  &inputParameters,
                  NULL,                  /* &outputParameters, */
                  SAMPLE_RATE,
                  FRAMES_PER_BUFFER,
                  paClipOff,      /* we won't output out of range samples so don't bother clipping them */
                  NULL,
                  NULL);

        Pa_StartStream( stream );

        for (int i = 0; i < SAMPLE_RATE / FRAMES_PER_BUFFER * NUM_SECONDS; i++) {
            printf("%d sending: %f\n", i, data);
            // Pa_ReadStream( stream, data, FRAMES_PER_BUFFER );
            send(sockfd, data, 4, 0);

        }

        Pa_StopStream( stream );
        Pa_CloseStream( stream );
        Pa_Terminate();
        close(sockfd);

    }
    return 0;
}
