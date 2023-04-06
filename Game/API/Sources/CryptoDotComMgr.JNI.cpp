#include "stdafx.h"
#if defined(ANDROID_NDK)
#include "CryptoDotComMgr.h"
#include "CryptoDotComMgr.JNI.h"
#include "JNIHelper.h"

static jclass       s_pcCryptoComClass                  = NULL;
static jmethodID    s_pcIsMarketSocketConnectedFunc     = NULL;
static jmethodID    s_pcConnectMarketSocketFunc         = NULL;
static jmethodID    s_pcSendMarketMessageFunc           = NULL;
static jmethodID    s_pcCloseMarketSocketFunc           = NULL;

BOOLEAN JNI_Init_CryptoDotComMgr(VOID)
{
    CJavaEnv cEnv;
    JNIEnv* pcEnv = cEnv.GetEnv();
    if (NULL == pcEnv) {
        ASSERT(FALSE);
        return FALSE;
    }
    jclass pcClass = pcEnv->FindClass("com/happylabs/common/websockets/CryptoCom");
    if (NULL == pcClass) {
        ASSERT(FALSE);
        return FALSE;
    }
    // setup global reference to this class variable
    s_pcCryptoComClass = (jclass)pcEnv->NewGlobalRef(pcClass);
    if (NULL == s_pcCryptoComClass) {
        ASSERT(FALSE);
        return FALSE;
    }
    GET_STATIC_METHOD(pcEnv, pcClass, s_pcIsMarketSocketConnectedFunc,  "IsMarketSocketConnected", "()Z");
    GET_STATIC_METHOD(pcEnv, pcClass, s_pcConnectMarketSocketFunc,      "ConnectMarketSocket",      "(Ljava/lang/String;)V");
    GET_STATIC_METHOD(pcEnv, pcClass, s_pcSendMarketMessageFunc,        "ConnectMarketSocket",      "(Ljava/lang/String;)V");
    GET_STATIC_METHOD(pcEnv, pcClass, s_pcCloseMarketSocketFunc,        "CloseMarketSocket",        "()V");
    return TRUE;
}

VOID    JNI_Release_CryptoDotComMgr(VOID)
{
    CJavaEnv cEnv;
    JNIEnv* pcEnv = cEnv.GetEnv();
    if (NULL == pcEnv) {
        ASSERT(FALSE);
        return;
    }
    if (NULL != s_pcCryptoComClass) {
        pcEnv->DeleteGlobalRef(s_pcCryptoComClass);
        s_pcCryptoComClass = NULL;
    }
}

BOOLEAN JNI_CryptoDotComMgr_IsMarketSocketConnected(VOID)
{
    return JNI_CallVoidParamWithBoolResult(s_pcCryptoComClass, s_pcIsMarketSocketConnectedFunc);
}

VOID    JNI_CryptoDotComMgr_ConnectMarketSocket(const CHAR* szURL)
{
    JNI_CallStringParamWithVoidResult(s_pcCryptoComClass, s_pcConnectMarketSocketFunc, szURL);
}

BOOLEAN JNI_CryptoDotComMgr_SendMsgToMarketSocket(const CHAR* szMsg)
{
    return JNI_CallStringParamWithBoolResult(s_pcCryptoComClass, s_pcSendMarketMessageFunc, szMsg);
}

VOID    JNI_CryptoDotComMgr_CloseMarketSocket(VOID)
{
    JNI_CallVariableParamWithVoidResult(s_pcCryptoComClass, s_pcCloseMarketSocketFunc);
}


extern "C" JNIEXPORT void JNICALL Java_com_happylabs_common_websockets_CryptoCom_OnMarketSocketConnectedNative(JNIEnv* pcEnv, jclass pcClass) {
    CCryptoDotComMgr::OnMarketSocketConnected();
}
extern "C" JNIEXPORT void JNICALL Java_com_happylabs_common_websockets_CryptoCom_OnMarketSocketFailedConnectionNative(JNIEnv* pcEnv, jclass pcClass, jstring pcReason) {
    CJniString cReason(pcEnv, pcReason, NULL);
    CCryptoDotComMgr::OnMarketSocketConnectFail(cReason.GetCStr());
}
extern "C" JNIEXPORT void JNICALL Java_com_happylabs_common_websockets_CryptoCom_OnMarketSocketClosedNative(JNIEnv* pcEnv, jclass pcClass, jstring pcReason) {
    CJniString cReason(pcEnv, pcReason, NULL);
    CCryptoDotComMgr::OnMarketSocketClosed(cReason.GetCStr());
}
extern "C" JNIEXPORT void JNICALL Java_com_happylabs_common_websockets_CryptoCom_OnReceiveMarketMessageNative(JNIEnv* pcEnv, jclass pcClass, jstring pcMsg) {
    CJniString cMsg(pcEnv, pcMsg, NULL);
    CCryptoDotComMgr::GetInstance().OnReceiveMarketMessage(cMsg.GetCStr());
}

#endif // #if defined(ANDROID_NDK)
