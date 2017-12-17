
#include "chatrobot.hpp"

#include "qisr.h"
#include "msp_cmn.h"
#include "msp_errors.h"

#include <sstream>

static void recognize_sentence()
{
    int errcode = MSP_SUCCESS;
    const char* session_begin_params = "sub = iat, domain = iat, language = zh_cn, accent = mandarin, sample_rate = 16000, result_type = plain, result_encoding = utf8";
    const char* session_id = QISRSessionBegin(NULL, session_begin_params, &errcode);
    if(MSP_SUCCESS != errcode)
        throw "QISRSessionBegin failed";

    bool first = true;
    int ep_stat = MSP_EP_LOOKING_FOR_SPEECH;
    int rec_stat = MSP_REC_STATUS_SUCCESS;

    std::stringstream ss;

    while(true)
    {
        int ret;

        {
            frame_queue_use top{q_record};
            ret = QISRAudioWrite(
                session_id,
                (*top).buffer,
                FRAMES_PER_BUFFER*sizeof(SAMPLE),
                first ? MSP_AUDIO_SAMPLE_FIRST : MSP_AUDIO_SAMPLE_CONTINUE,
                &ep_stat,
                &rec_stat);
        }

        if (MSP_SUCCESS != ret)
        {
            printf("\nQISRAudioWrite failed! error code:%d\n", ret);
            throw "QISRAudioWrite failed";
        }
        first = false;

        if (MSP_REC_STATUS_SUCCESS == rec_stat)
        {
            const char *rslt = QISRGetResult(session_id, &rec_stat, 0, &errcode);
            if (MSP_SUCCESS != errcode)
            {
                printf("\nQISRGetResult failed! error code: %d\n", errcode);
                throw "QISRGetResult failed";
            }
            if (NULL != rslt)
            {
                ss<<rslt;
            }
        }

        if (MSP_EP_AFTER_SPEECH == ep_stat)
        {
            errcode = QISRAudioWrite(session_id, NULL, 0, MSP_AUDIO_SAMPLE_LAST, &ep_stat, &rec_stat);
            if (MSP_SUCCESS != errcode)
            {
                printf("\nQISRGetResult failed! error code: %d\n", errcode);
                throw "QISRGetResult failed";
            }
            while (MSP_REC_STATUS_COMPLETE != rec_stat) 
            {
                const char *rslt = QISRGetResult(session_id, &rec_stat, 0, &errcode);
                if (MSP_SUCCESS != errcode)
                {
                    printf("\nQISRGetResult failed, error code: %d\n", errcode);
                    throw "QISRGetResult failed";
                }
                if (NULL != rslt)
                    ss<<rslt;
            }
            break;
        }
    }

    q_in.emplace(ss.str());
    printf("%s\n", ss.str().c_str());

    QISRSessionEnd(session_id, "END");

}

void* do_recognize(void*)
{
    while(!signal_exit)
    {
        recognize_sentence();
    }
    return NULL;
}
