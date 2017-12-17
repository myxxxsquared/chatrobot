
#include "chatrobot.hpp"

frame_queue q_record, q_play;
string_queue q_in, q_out;

volatile bool signal_exit = false;

int main()
{
    init_audio();
    init_threads();

    while(1);

    exit_audio();
}
