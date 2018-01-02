
#include "chatrobot.hpp"
#include "qisr.h"
#include "msp_cmn.h"
#include "msp_errors.h"

void init_xf()
{
    int errcode = MSP_SUCCESS;
    const char* login_params = DEFAULT_TOKEN;
    errcode = MSPLogin(NULL, NULL, login_params);
    if (MSP_SUCCESS != errcode)
        throw "MSPLogin() failed.";
}

void init_xf_offline()
// for Command word recognization
{
    int errcode = MSP_SUCCESS;
    const char* login_params = OFFLINE_TOKEN;
    errcode = MSPLogin(NULL, NULL, login_params);
    if (MSP_SUCCESS != errcode)
        throw "MSPLogin() failed.";
}