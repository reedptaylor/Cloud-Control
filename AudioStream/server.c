#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <ctype.h>
#include "portaudio.h"

/* #define SAMPLE_RATE  (17932) // Test failure to open with this value. */
#define SAMPLE_RATE  (44100)
#define FRAMES_PER_BUFFER (512)
#define NUM_SECONDS     (10)
#define NUM_CHANNELS    (2)
/* #define DITHER_FLAG     (paDitherOff) */
#define DITHER_FLAG     (0) /**/
/** Set to 1 if you want to capture the recording to a file. */
#define WRITE_TO_FILE   (0)

/* Select sample format. */
#if 1
#define PA_SAMPLE_TYPE  paFloat32
typedef float SAMPLE;
#define SAMPLE_SILENCE  (0.0f)
#define PRINTF_S_FORMAT "%.8f"
#elif 1
#define PA_SAMPLE_TYPE  paInt16
typedef short SAMPLE;
#define SAMPLE_SILENCE  (0)
#define PRINTF_S_FORMAT "%d"
#elif 0
#define PA_SAMPLE_TYPE  paInt8
typedef char SAMPLE;
#define SAMPLE_SILENCE  (0)
#define PRINTF_S_FORMAT "%d"
#else
#define PA_SAMPLE_TYPE  paUInt8
typedef unsigned char SAMPLE;
#define SAMPLE_SILENCE  (128)
#define PRINTF_S_FORMAT "%d"
#endif

typedef struct
{
    int          frameIndex;  /* Index into sample array. */
    int          maxFrameIndex;
    SAMPLE      *recordedSamples;
}
paTestData;

/* This routine will be called by the PortAudio engine when audio is needed.
** It may be called at interrupt level on some machines so don't do anything
** that could mess up the system like calling malloc() or free().
*/
static int playCallback( const void *inputBuffer, void *outputBuffer,
                         unsigned long framesPerBuffer,
                         const PaStreamCallbackTimeInfo* timeInfo,
                         PaStreamCallbackFlags statusFlags,
                         void *userData )
{
    paTestData *data = (paTestData*)userData;
    SAMPLE *rptr = &data->recordedSamples[data->frameIndex * NUM_CHANNELS];
    SAMPLE *wptr = (SAMPLE*)outputBuffer;
    unsigned int i;
    int finished;
    unsigned int framesLeft = data->maxFrameIndex - data->frameIndex;

    (void) inputBuffer; /* Prevent unused variable warnings. */
    (void) timeInfo;
    (void) statusFlags;
    (void) userData;

    if( framesLeft < framesPerBuffer )
    {
        /* final buffer... */
        for( i=0; i<framesLeft; i++ )
        {
            *wptr++ = *rptr++;  /* left */
            if( NUM_CHANNELS == 2 ) *wptr++ = *rptr++;  /* right */
        }
        for( ; i<framesPerBuffer; i++ )
        {
            *wptr++ = 0;  /* left */
            if( NUM_CHANNELS == 2 ) *wptr++ = 0;  /* right */
        }
        data->frameIndex += framesLeft;
        finished = paComplete;
    }
    else
    {
        for( i=0; i<framesPerBuffer; i++ )
        {
            *wptr++ = *rptr++;  /* left */
            if( NUM_CHANNELS == 2 ) *wptr++ = *rptr++;  /* right */
        }
        data->frameIndex += framesPerBuffer;
        finished = paContinue;
    }
    return finished;
}

int main(int argc, char *argv[])
{
    // check to make sure that the user entered only the port number to run the server on
    if(argc == 2) {

        PaStreamParameters  outputParameters;
        PaStream*           stream;
        PaError             err = paNoError;
        paTestData          data;
        int                 totalFrames;
        int                 numSamples;
        int                 numBytes;

        // extract the port number from the command line args and convert to an integer
        int serverPort = atoi(argv[1]);

        // server loops forever so that multiple client sessions can be handled
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

            totalFrames = NUM_SECONDS * SAMPLE_RATE;
            numSamples = totalFrames * NUM_CHANNELS;
            numBytes = numSamples * sizeof(SAMPLE);
            data.recordedSamples = (SAMPLE *) malloc( numBytes ); /* From now on, recordedSamples is initialised. */
            int rcvLength;
            rcvLength = recv(connectionfd, &data.maxFrameIndex, 4, 0);
            if(rcvLength == -1) {
                fprintf(stderr, "Error: rcvLength = -1.\n");
            }
            for(int i = 0; i < numSamples; i++) {
                rcvLength = recv(connectionfd, &data.recordedSamples[i], 4, 0);
                if(rcvLength == -1) {
                    fprintf(stderr, "Error: rcvLength = -1.\n");
                }
            }

            /* Playback recorded data.  -------------------------------------------- */
            err = Pa_Initialize();

            data.frameIndex = 0;

            outputParameters.device = Pa_GetDefaultOutputDevice(); /* default output device */
            if (outputParameters.device == paNoDevice) {
                fprintf(stderr,"Error: No default output device.\n");
                goto done;
            }
            outputParameters.channelCount = 2;                     /* stereo output */
            outputParameters.sampleFormat =  PA_SAMPLE_TYPE;
            outputParameters.suggestedLatency = Pa_GetDeviceInfo( outputParameters.device )->defaultLowOutputLatency;
            outputParameters.hostApiSpecificStreamInfo = NULL;

            printf("\n=== Now playing back. ===\n"); fflush(stdout);
            err = Pa_OpenStream(
                      &stream,
                      NULL, /* no input */
                      &outputParameters,
                      SAMPLE_RATE,
                      FRAMES_PER_BUFFER,
                      paClipOff,      /* we won't output out of range samples so don't bother clipping them */
                      playCallback,
                      &data );
            if( err != paNoError ) goto done;

            if( stream )
            {
                err = Pa_StartStream( stream );
                if( err != paNoError ) goto done;

                printf("Waiting for playback to finish.\n"); fflush(stdout);

                while( ( err = Pa_IsStreamActive( stream ) ) == 1 ) Pa_Sleep(100);
                if( err < 0 ) goto done;

                err = Pa_CloseStream( stream );
                if( err != paNoError ) goto done;

                printf("Done.\n"); fflush(stdout);
            }

            done:
                Pa_Terminate();
                if( data.recordedSamples )       /* Sure it is NULL or valid. */
                    free( data.recordedSamples );
                if( err != paNoError )
                {
                    fprintf( stderr, "An error occured while using the portaudio stream\n" );
                    fprintf( stderr, "Error number: %d\n", err );
                    fprintf( stderr, "Error message: %s\n", Pa_GetErrorText( err ) );
                    err = 1;          /* Always return 0 or 1, but no other return codes. */
                }
                return err;

                    // close the socket now that we are done communicating
                    close(sockfd);
                    close(connectionfd);

                }
    }
    else {
        printf("Please enter only the port number when running the program.\n");
    }

    return(0);
}
