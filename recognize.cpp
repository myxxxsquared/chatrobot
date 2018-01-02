
#include "chatrobot.hpp"

#include "qisr.h"
#include "msp_cmn.h"
#include "msp_errors.h"

#include <sstream>

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

#define SAMPLE_RATE_16K     (16000)
#define SAMPLE_RATE_8K      (8000)
#define MAX_GRAMMARID_LEN   (32)
#define MAX_PARAMS_LEN      (1024)


typedef struct _UserData {
    int     build_fini;  //标识语法构建是否完成
    int     update_fini; //标识更新词典是否完成
    int     errcode; //记录语法构建或更新词典回调错误码
    char    grammar_id[MAX_GRAMMARID_LEN]; //保存语法构建返回的语法ID
}UserData;


const char * ASR_RES_PATH        = "fo|res/asr/common.jet";  //离线语法识别资源路径
#ifdef _WIN64
const char * GRM_BUILD_PATH      = "res/asr/GrmBuilld_x64";  //构建离线语法识别网络生成数据保存路径
#else
const char * GRM_BUILD_PATH      = "res/asr/GrmBuilld";  //构建离线语法识别网络生成数据保存路径
#endif
const char * GRM_FILE            = "robot.bnf"; //构建离线识别语法网络所用的语法文件
const char * LEX_NAME            = "command"; //更新离线识别语法的contact槽

static FILE* file_recognize[2];
static char recognize_buffer[BUFFER_SIZE];

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
            {
                int dummy = 0;
                myfwrite(&dummy, sizeof(int), 1, file_recognize[1]);
                fflush(file_recognize[1]);
                myfread(recognize_buffer, BUFFER_SIZE, 1, file_recognize[0]);
            }
            ret = QISRAudioWrite(
                session_id,
                recognize_buffer,
                FRAMES_PER_BUFFER*sizeof(SAMPLE),
                first ? MSP_AUDIO_SAMPLE_FIRST : MSP_AUDIO_SAMPLE_CONTINUE,
                &ep_stat,
                &rec_stat);
        }

        if (MSP_SUCCESS != ret)
        {
            fprintf(stderr, "\nQISRAudioWrite failed! error code:%d\n", ret);
            throw "QISRAudioWrite failed";
        }
        first = false;

        if (MSP_REC_STATUS_SUCCESS == rec_stat)
        {
            const char *rslt = QISRGetResult(session_id, &rec_stat, 0, &errcode);
            if (MSP_SUCCESS != errcode)
            {
                fprintf(stderr, "\nQISRGetResult failed! error code: %d\n", errcode);
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
                fprintf(stderr, "\nQISRGetResult failed! error code: %d\n", errcode);
                throw "QISRGetResult failed";
            }
            while (MSP_REC_STATUS_COMPLETE != rec_stat) 
            {
                const char *rslt = QISRGetResult(session_id, &rec_stat, 0, &errcode);
                if (MSP_SUCCESS != errcode)
                {
                    fprintf(stderr, "\nQISRGetResult failed, error code: %d\n", errcode);
                    throw "QISRGetResult failed";
                }
                if (NULL != rslt)
                    ss<<rslt;
            }
            break;
        }
    }

    std::string str = ss.str();
    int len = str.size() + 1;
    myfwrite(&len, sizeof(int), 1, file_recognize[1]);
    fflush(file_recognize[1]);
    myfwrite(str.c_str(), len, 1, file_recognize[1]);
    fflush(file_recognize[1]);
    fprintf(stderr, "IN : %s\n", ss.str().c_str());

    QISRSessionEnd(session_id, "END");

}

int build_grm_cb(int ecode, const char *info, void *udata)
{
    UserData *grm_data = (UserData *)udata;

    if (NULL != grm_data) {
        grm_data->build_fini = 1;
        grm_data->errcode = ecode;
    }

    if (MSP_SUCCESS == ecode && NULL != info) {
        printf("构建语法成功！ 语法ID:%s\n", info);
        if (NULL != grm_data)
            snprintf(grm_data->grammar_id, MAX_GRAMMARID_LEN - 1, info);
    }
    else
        printf("构建语法失败！%d\n", ecode);

    return 0;
}

int build_grammar(UserData *udata)
{
    FILE *grm_file                           = NULL;
    char *grm_content                        = NULL;
    unsigned int grm_cnt_len                 = 0;
    char grm_build_params[MAX_PARAMS_LEN]    = {NULL};
    int ret                                  = 0;

    grm_file = fopen(GRM_FILE, "rb");
    if(NULL == grm_file) {
        printf("打开\"%s\"文件失败！[%s]\n", GRM_FILE, strerror(errno));
        return -1;
    }

    fseek(grm_file, 0, SEEK_END);
    grm_cnt_len = ftell(grm_file);
    fseek(grm_file, 0, SEEK_SET);

    grm_content = (char *)malloc(grm_cnt_len + 1);
    if (NULL == grm_content)
    {
        printf("内存分配失败!\n");
        fclose(grm_file);
        grm_file = NULL;
        return -1;
    }
    fread((void*)grm_content, 1, grm_cnt_len, grm_file);
    grm_content[grm_cnt_len] = '\0';
    fclose(grm_file);
    grm_file = NULL;

    snprintf(grm_build_params, MAX_PARAMS_LEN - 1,
              "engine_type = local, \
		asr_res_path = %s, sample_rate = %d, \
		grm_build_path = %s, ",
              ASR_RES_PATH,
              SAMPLE_RATE_16K,
              GRM_BUILD_PATH
    );
    ret = QISRBuildGrammar("bnf", grm_content, grm_cnt_len, grm_build_params, build_grm_cb, udata);

    free(grm_content);
    grm_content = NULL;

    return ret;
}

static void recognize_sentence_offline(UserData *udata)
{
    char asr_params[MAX_PARAMS_LEN]    = {NULL};
    const char *session_id             = NULL;
    const char *asr_audiof             = NULL;
//    int last_audio                     = 0;
//    int aud_stat                       = MSP_AUDIO_SAMPLE_CONTINUE;
    int ep_stat                        = MSP_EP_LOOKING_FOR_SPEECH;
    int rec_stat                       = MSP_REC_STATUS_INCOMPLETE;
//    int rss_status                     = MSP_REC_STATUS_INCOMPLETE;
    int errcode                        = -1;

    //离线语法识别参数设置
    snprintf(asr_params, MAX_PARAMS_LEN - 1,
              "engine_type = local, \
        asr_res_path = %s, sample_rate = %d, \
		grm_build_path = %s, local_grammar = %s, \
		result_type = xml, result_encoding = GB2312, ",
              ASR_RES_PATH,
              SAMPLE_RATE_16K,
              GRM_BUILD_PATH,
              udata->grammar_id
    );
    session_id = QISRSessionBegin(NULL, asr_params, &errcode);
    if (NULL == session_id)
        throw "Session begin fail, ASR.";  // goto run_error;
    printf("开始识别...\n");

    if(MSP_SUCCESS != errcode)
        throw "QISRSessionBegin failed";

    bool first = true;

    std::stringstream ss;

    while(true)
    {
        int ret;

        {
            {
                int dummy = 0;
                myfwrite(&dummy, sizeof(int), 1, file_recognize[1]);
                fflush(file_recognize[1]);
                myfread(recognize_buffer, BUFFER_SIZE, 1, file_recognize[0]);
            }
            ret = QISRAudioWrite(
                    session_id,
                    recognize_buffer,
                    FRAMES_PER_BUFFER*sizeof(SAMPLE),
                    first ? MSP_AUDIO_SAMPLE_FIRST : MSP_AUDIO_SAMPLE_CONTINUE,
                    &ep_stat,
                    &rec_stat);
        }

        if (MSP_SUCCESS != ret)
        {
            fprintf(stderr, "\nQISRAudioWrite failed! error code:%d\n", ret);
            throw "QISRAudioWrite failed";
        }
        first = false;

        if (MSP_REC_STATUS_SUCCESS == rec_stat)
        {
            const char *rslt = QISRGetResult(session_id, &rec_stat, 0, &errcode);
            if (MSP_SUCCESS != errcode)
            {
                fprintf(stderr, "\nQISRGetResult failed! error code: %d\n", errcode);
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
                fprintf(stderr, "\nQISRGetResult failed! error code: %d\n", errcode);
                throw "QISRGetResult failed";
            }
            while (MSP_REC_STATUS_COMPLETE != rec_stat)
            {
                const char *rslt = QISRGetResult(session_id, &rec_stat, 0, &errcode);
                if (MSP_SUCCESS != errcode)
                {
                    fprintf(stderr, "\nQISRGetResult failed, error code: %d\n", errcode);
                    throw "QISRGetResult failed";
                }
                if (NULL != rslt)
                    ss<<rslt;
            }
            break;
        }
    }

    std::string str = ss.str();
    int len = str.size() + 1;
    myfwrite(&len, sizeof(int), 1, file_recognize[1]);
    fflush(file_recognize[1]);
    myfwrite(str.c_str(), len, 1, file_recognize[1]);
    fflush(file_recognize[1]);
    fprintf(stderr, "IN : %s\n", ss.str().c_str());

    QISRSessionEnd(session_id, "END");

}

static void recognize_parent()
{
    while(true)
    {
        int len;
        myfread(&len, sizeof(int), 1, file_recognize[0]);
        if(len)
        {
            char* text;
            text = (char*)malloc(len);
            myfread(text, len, 1, file_recognize[0]);
            q_in.emplace(std::string(text));
            free(text);
        }
        else
        {
            frame_queue_use top{q_record};
            void* buffer = (*top).buffer;
            myfwrite(buffer, BUFFER_SIZE, 1, file_recognize[1]);
            fflush(file_recognize[1]);
        }
    }
}

static void recognize_child()
{
    int errcode = MSP_SUCCESS;
    const char* login_params = DEFAULT_TOKEN;
    errcode = MSPLogin(NULL, NULL, login_params);
    if (MSP_SUCCESS != errcode)
        throw "MSPLogin() failed.";
    while(true)
        recognize_sentence();
}

static void recognize_child_offline()
// for offline version
{
    int ret = 0;
    UserData asr_data;
    int errcode = MSP_SUCCESS;
    const char* login_params = OFFLINE_TOKEN;
    errcode = MSPLogin(NULL, NULL, login_params);
    if (MSP_SUCCESS != errcode)
        throw "MSPLogin() failed.";

    memset(&asr_data, 0, sizeof(UserData));
    printf("构建离线识别语法网络...\n");
    ret = build_grammar(&asr_data);  //第一次使用某语法进行识别，需要先构建语法网络，获取语法ID，之后使用此语法进行识别，无需再次构建
    if (MSP_SUCCESS != ret)
        throw "Grammar build failed.";

    while (1 != asr_data.build_fini)
        sleep(300);
    if (MSP_SUCCESS != asr_data.errcode)
        throw "Offline recognition ERROR, UNK";
    printf("离线识别语法网络构建完成，开始识别...\n");
    while(true)
        recognize_sentence_offline(&asr_data);
}

void* do_recognize(void*)
{
#ifdef USE_STDIN
    std::string line;
    while(true)
    {
        std::getline(std::cin, line);
        q_in.emplace(line);
    }
#endif

    pid_t pid;
    int pipe_recognize[2][2];
    if(pipe(pipe_recognize[0]) || pipe(pipe_recognize[1]))
        throw "pipe error";

    pid = fork();
    if(pid == 0)
    {
        file_recognize[0] = fdopen(pipe_recognize[0][0], "r");
        file_recognize[1] = fdopen(pipe_recognize[1][1], "w");
        recognize_child();
    }
    else
    {
        file_recognize[0] = fdopen(pipe_recognize[1][0], "r");
        file_recognize[1] = fdopen(pipe_recognize[0][1], "w");
        recognize_parent();
    }

    return NULL;
}

void* do_recognize_offline(void*)
{
#ifdef USE_STDIN
    std::string line;
    while(true)
    {
        std::getline(std::cin, line);
        q_in.emplace(line);
    }
#endif

    pid_t pid;
    int pipe_recognize[2][2];
    if(pipe(pipe_recognize[0]) || pipe(pipe_recognize[1]))
        throw "pipe error";

    pid = fork();
    if(pid == 0)
    {
        file_recognize[0] = fdopen(pipe_recognize[0][0], "r");
        file_recognize[1] = fdopen(pipe_recognize[1][1], "w");
        recognize_child_offline();
    }
    else
    {
        file_recognize[0] = fdopen(pipe_recognize[1][0], "r");
        file_recognize[1] = fdopen(pipe_recognize[0][1], "w");
        recognize_parent();
    }

    return NULL;
}