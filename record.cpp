
#include "chatrobot.hpp"

int recordCallback(
    const void *input, void *output,
    unsigned long frameCount,
    const PaStreamCallbackTimeInfo* timeInfo,
    PaStreamCallbackFlags statusFlags,
    void *userData )
{
    if(signal_exit)
        return paComplete;

    if(frameCount != FRAMES_PER_BUFFER)
    {
        fprintf(stderr, "unexcepted frames.\n");
        throw "unexcepted frames.";
    }
    q_record.emplace(input, 3);

    return paContinue;
}
