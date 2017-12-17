
#include "chatrobot.hpp"
#include "qtts.h"
#include "msp_cmn.h"
#include "msp_errors.h"

static char tempbuffer[BUFFER_SIZE];
static int tempbufferlen = 0;

static void generate()
{
    std::string str;
    {
        string_queue_use use{q_out};
        str = *use;
    }

    int ret = -1;
    const char* params = "voice_name = xiaoyan, text_encoding = utf8, sample_rate = 16000, speed = 50, volume = 50, pitch = 50, rdn = 2";
    const char* sessionID = QTTSSessionBegin(params, &ret);
    if (MSP_SUCCESS != ret)
        throw "QTTSSessionBegin() failed";
    ret = QTTSTextPut(sessionID, str.c_str(), str.size(), NULL);
    if (MSP_SUCCESS != ret)
        throw "QTTSTextPut() failed";

    unsigned int audio_len = 0;
    int synth_status = MSP_TTS_FLAG_STILL_HAVE_DATA;
    while(true)
    {
        const char* data = (const char*)QTTSAudioGet(sessionID, &audio_len, &synth_status, &ret);
        if (MSP_SUCCESS != ret)
            break;
        if (NULL != data)
        {
            if(tempbufferlen)
            {
                if(tempbufferlen+audio_len >= BUFFER_SIZE)
                {
                    int left_size = BUFFER_SIZE - tempbufferlen;
                    memcpy(tempbuffer+tempbufferlen, data, left_size);
                    audio_len -= left_size;
                    data += left_size;
                    q_play.emplace(tempbuffer);
                    tempbufferlen = 0;
                }
                else
                {
                    memcpy(tempbuffer+tempbufferlen, data, audio_len);
                    tempbufferlen += audio_len;
                    audio_len = 0;
                }
            }

            while(audio_len > BUFFER_SIZE)
            {
                q_play.emplace(data);
                data += BUFFER_SIZE;
                audio_len -= BUFFER_SIZE;
            }

            if(audio_len > 0)
            {
                memcpy(tempbuffer, data, audio_len);
                tempbufferlen = audio_len;
            }
        }
        if (MSP_TTS_FLAG_DATA_END == synth_status)
            break;
    }
    if (MSP_SUCCESS != ret)
        throw "QTTSAudioGet() failed";

    if(tempbufferlen)
    {
        memset(tempbuffer+tempbufferlen, 0, BUFFER_SIZE-tempbufferlen);
        tempbufferlen = 0;
        q_play.emplace(tempbuffer);
    }
    
    QTTSSessionEnd(sessionID, "END");
}

void* do_generate(void*)
{
    ///////////////////////////////
    // while(true)
    // {
    //     string_queue_use use{q_out};
    //     std::cout<<*use<<std::endl;
    // }
    ///////////////////////////////

    while(!signal_exit)
        generate();

    return NULL;
}
