
#include "chatrobot.hpp"

int playCallback(
    const void *input, void *output,
    unsigned long frameCount,
    const PaStreamCallbackTimeInfo* timeInfo,
    PaStreamCallbackFlags statusFlags,
    void *userData )
{
    if(signal_exit)
        return paComplete;
    {
        frame_queue_use use{q_play, true};
        if(use.vaild)
            memcpy(output, (*use).buffer, BUFFER_SIZE);
        else
            memset(output, 0, BUFFER_SIZE);
    }
    return paContinue;
}
