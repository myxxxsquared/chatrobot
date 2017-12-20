
#include "chatrobot.hpp"

frame_queue q_record, q_play;
string_queue q_in, q_out;

volatile bool signal_exit = false;

int main()
{
    try
    {
        init_xf();
        init_audio();
        init_threads();
    }
    catch(const char* str)
    {
        printf("%s\n", str);
        return -1;
    }

    while(1);

    exit_audio();
}
