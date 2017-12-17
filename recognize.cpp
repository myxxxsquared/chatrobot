
#include "chatrobot.hpp"

static void recognize_sentence()
{
}

void* do_recognize(void*)
{
    while(!signal_exit)
    {
        recognize_sentence();
    }
}
