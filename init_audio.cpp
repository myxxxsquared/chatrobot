
#include "chatrobot.hpp"

PaStream *inputStream, *outputStream;

void init_audio()
{
    PaError err = paNoError;
    err = Pa_Initialize();
    if(paNoError != err)
        throw "Pa_Initialize() Failed.";

    PaStreamParameters input, output;

#ifndef USE_STDIN
    input.channelCount = CHANNEL_COUNT;
    input.sampleFormat = SAMPLE_FORMAT;
    input.device = Pa_GetDefaultInputDevice();
    if(paNoDevice == input.device)
        throw "Pa_GetDefaultInputDevice() Failed.";
    input.suggestedLatency = Pa_GetDeviceInfo(input.device)->defaultLowInputLatency;
    input.hostApiSpecificStreamInfo = NULL;

    err = Pa_OpenStream(
        &inputStream,
        &input,
        NULL,
        SAMPLE_RATE*RECORD_MULTIPLY,
        FRAMES_PER_BUFFER,
        paNoFlag,
        &recordCallback,
        (void *)NULL);
    if(paNoError != err)
        throw "Pa_OpenStream() Failed.";

    err = Pa_StartStream(inputStream);
    if(paNoError != err)
        throw "Pa_StartStream() Failed.";
#endif

#ifndef USE_STDOUT
    output.device = Pa_GetDefaultOutputDevice();
    if(paNoDevice == output.device)
        throw "Pa_GetDefaultOutputDevice() Failed.";
    output.channelCount = CHANNEL_COUNT;
    output.sampleFormat = SAMPLE_FORMAT;
    output.suggestedLatency = Pa_GetDeviceInfo(output.device)->defaultLowOutputLatency;
    output.hostApiSpecificStreamInfo = NULL;

    err = Pa_OpenStream(
        &outputStream,
        NULL,
        &output,
        SAMPLE_RATE,
        FRAMES_PER_BUFFER,
        paNoFlag,
        &playCallback,
        (void *)NULL);
    if(paNoError != err)
        throw "Pa_OpenStream() Failed.";
    err = Pa_StartStream(outputStream);
    if(paNoError != err)
        throw "Pa_StartStream() Failed.";
#endif

    fprintf(stderr, "%lf, %lf\n", input.suggestedLatency, output.suggestedLatency);
}

void exit_audio()
{
    Pa_StopStream(inputStream);
    Pa_CloseStream(inputStream);
    Pa_StopStream(outputStream);
    Pa_CloseStream(outputStream);
    Pa_Terminate();
}
