
#include "chatrobot.hpp"

PaStream *inputStream, *outputStream;

void init_audio()
{
    PaError err = paNoError;
    err = Pa_Initialize();
    if(paNoError != err)
        throw "Pa_Initialize() Failed.";

    PaStreamParameters input, output;

    input.device = Pa_GetDefaultInputDevice();
    if(paNoDevice == input.device)
        throw "Pa_GetDefaultInputDevice() Failed.";
    input.channelCount = CHANNEL_COUNT;
    input.sampleFormat = SAMPLE_FORMAT;
    input.suggestedLatency = Pa_GetDeviceInfo(input.device)->defaultLowInputLatency;
    input.hostApiSpecificStreamInfo = NULL;

    output.device = Pa_GetDefaultOutputDevice();
    if(paNoDevice == output.device)
        throw "Pa_GetDefaultOutputDevice() Failed.";
    output.channelCount = CHANNEL_COUNT;
    output.sampleFormat = SAMPLE_FORMAT;
    output.suggestedLatency = Pa_GetDeviceInfo(output.device)->defaultLowInputLatency;
    output.hostApiSpecificStreamInfo = NULL;

    err = Pa_OpenStream(
        &inputStream,
        &input,
        NULL,
        SAMPLE_RATE,
        FRAMES_PER_BUFFER,
        paClipOff,
        &recordCallback,
        (void *)NULL);
    if(paNoError != err)
        throw "Pa_OpenStream() Failed.";
    err = Pa_StartStream(inputStream);
    if(paNoError != err)
        throw "Pa_StartStream() Failed.";

    err = Pa_OpenStream(
        &outputStream,
        NULL,
        &output,
        SAMPLE_RATE,
        FRAMES_PER_BUFFER,
        paClipOff,
        &playCallback,
        (void *)NULL);
    if(paNoError != err)
        throw "Pa_OpenStream() Failed.";
    err = Pa_StartStream(outputStream);
    if(paNoError != err)
        throw "Pa_StartStream() Failed.";
}

void exit_audio()
{
    Pa_StopStream(inputStream);
    Pa_CloseStream(inputStream);
    Pa_StopStream(outputStream);
    Pa_CloseStream(outputStream);
    Pa_Terminate();
}
