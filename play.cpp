
#include "chatrobot.hpp"

int playCallback(
    const void *input, void *output,
    unsigned long frameCount,
    const PaStreamCallbackTimeInfo* timeInfo,
    PaStreamCallbackFlags statusFlags,
    void *userData )
{
    memset(output, 0, sizeof(SAMPLE)*FRAMES_PER_BUFFER);
    return paContinue;
}
