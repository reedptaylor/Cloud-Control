#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "portaudio.h"

/* #define SAMPLE_RATE  (17932) // Test failure to open with this value. */
#define SAMPLE_RATE  (44100)
#define FRAMES_PER_BUFFER (512)
#define NUM_SECONDS     (5)
#define NUM_CHANNELS    (2)
/* #define DITHER_FLAG     (paDitherOff) */
#define DITHER_FLAG     (0) /**/
/** Set to 1 if you want to capture the recording to a file. */
#define WRITE_TO_FILE   (1)

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
static int recordCallback( const void *inputBuffer, void *outputBuffer,
                           unsigned long framesPerBuffer,
                           const PaStreamCallbackTimeInfo* timeInfo,
                           PaStreamCallbackFlags statusFlags,
                           void *userData )
{
    paTestData *data = (paTestData*)userData;
    const SAMPLE *rptr = (const SAMPLE*)inputBuffer;
    SAMPLE *wptr = &data->recordedSamples[data->frameIndex * NUM_CHANNELS];
    long framesToCalc;
    long i;
    int finished;
    unsigned long framesLeft = data->maxFrameIndex - data->frameIndex;

    (void) outputBuffer; /* Prevent unused variable warnings. */
    (void) timeInfo;
    (void) statusFlags;
    (void) userData;

    if( framesLeft < framesPerBuffer )
    {
        framesToCalc = framesLeft;
        finished = paComplete;
    }
    else
    {
        framesToCalc = framesPerBuffer;
        finished = paContinue;
    }

    if( inputBuffer == NULL )
    {
        for( i=0; i<framesToCalc; i++ )
        {
            *wptr++ = SAMPLE_SILENCE;  /* left */
            if( NUM_CHANNELS == 2 ) *wptr++ = SAMPLE_SILENCE;  /* right */
        }
    }
    else
    {
        for( i=0; i<framesToCalc; i++ )
        {
            *wptr++ = *rptr++;  /* left */
            if( NUM_CHANNELS == 2 ) *wptr++ = *rptr++;  /* right */
        }
    }
    data->frameIndex += framesToCalc;
    return finished;
}

int main(int argc, char *argv[])
{
    // check to make sure the user entered both the IP address and the port number
    if(argc == 3) {

        PaStreamParameters  inputParameters;
        PaStream*           stream;
        PaError             err = paNoError;
        paTestData          data;
        int                 i;
        int                 totalFrames;
        int                 numSamples;
        int                 numBytes;
        SAMPLE              max, val;
        double              average;

        // extract the IP address and port number from the command line args
        char *serverHostname = argv[1];
        int serverPort = atoi(argv[2]);

        // convert the hostname to an IP address for the socket struct
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

        printf("patest_record.c\n"); fflush(stdout);

        data.maxFrameIndex = totalFrames = NUM_SECONDS * SAMPLE_RATE; /* Record for a few seconds. */
        data.frameIndex = 0;
        numSamples = totalFrames * NUM_CHANNELS;
        numBytes = numSamples * sizeof(SAMPLE);
        data.recordedSamples = (SAMPLE *) malloc( numBytes ); /* From now on, recordedSamples is initialised. */
        if( data.recordedSamples == NULL )
        {
            printf("Could not allocate record array.\n");
            goto done;
        }
        for( i=0; i<numSamples; i++ ) data.recordedSamples[i] = 0;

        err = Pa_Initialize();
        if( err != paNoError ) goto done;

        inputParameters.device = Pa_GetDefaultInputDevice(); /* default input device */
        if (inputParameters.device == paNoDevice) {
            fprintf(stderr,"Error: No default input device.\n");
            goto done;
        }
        inputParameters.channelCount = 2;                    /* stereo input */
        inputParameters.sampleFormat = PA_SAMPLE_TYPE;
        inputParameters.suggestedLatency = Pa_GetDeviceInfo( inputParameters.device )->defaultLowInputLatency;
        inputParameters.hostApiSpecificStreamInfo = NULL;

        /* Record some audio. -------------------------------------------- */
        err = Pa_OpenStream(
                  &stream,
                  &inputParameters,
                  NULL,                  /* &outputParameters, */
                  SAMPLE_RATE,
                  FRAMES_PER_BUFFER,
                  paClipOff,      /* we won't output out of range samples so don't bother clipping them */
                  recordCallback,
                  &data );
        if( err != paNoError ) goto done;

        err = Pa_StartStream( stream );
        if( err != paNoError ) goto done;
        printf("\n=== Now recording!! Please speak into the microphone. ===\n"); fflush(stdout);

        while( ( err = Pa_IsStreamActive( stream ) ) == 1 )
        {
            Pa_Sleep(1000);
            printf("index = %d\n", data.frameIndex ); fflush(stdout);
        }
        if( err < 0 ) goto done;

        err = Pa_CloseStream( stream );
        if( err != paNoError ) goto done;

        /* Measure maximum peak amplitude. */
        max = 0;
        average = 0.0;
        for( i=0; i<numSamples; i++ )
        {
            val = data.recordedSamples[i];
            if( val < 0 ) val = -val; /* ABS */
            if( val > max )
            {
                max = val;
            }
            average += val;
        }

        average = average / (double)numSamples;

        printf("sample max amplitude = "PRINTF_S_FORMAT"\n", max );
        printf("sample average = %lf\n", average );
        printf("length: %lu\n", NUM_CHANNELS * sizeof(SAMPLE));
        printf("total frames: %d\n", totalFrames);
        printf("Sizeof(data): %lu\n", sizeof(data));

        /* Write recorded data to a file. */
        #if WRITE_TO_FILE
            {
                FILE  *fid;
                fid = fopen("recorded.raw", "wb");
                if( fid == NULL )
                {
                    printf("Could not open file.");
                }
                else
                {
                    fwrite( data.recordedSamples, NUM_CHANNELS * sizeof(SAMPLE), totalFrames, fid );
                    fclose( fid );
                    printf("Wrote data to 'recorded.raw'\n");
                }
            }
        #endif

        printf("frameIndex: %d, maxFrameIndex: %d\n", data.frameIndex, data.maxFrameIndex);
        printf("size of recordedSamples: %lu\n", sizeof(data.recordedSamples));

        send(sockfd, (struct paTestData *)&data, sizeof(data), 0);

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

        // close the socket now that we are finished
        close(sockfd);
    }
    else {
        printf("Please enter only the IP address and port number when running the program.\n");
    }

    return(0);
}
