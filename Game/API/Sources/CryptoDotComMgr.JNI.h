#if !defined(CRYPTO_DOT_COM_MGR_JNI_H)
#define CRYPTO_DOT_COM_MGR_JNI_H
#if defined(ANDROID_NDK)

BOOLEAN JNI_Init_CryptoDotComMgr(VOID);
VOID    JNI_Release_CryptoDotComMgr(VOID);

BOOLEAN JNI_CryptoDotComMgr_IsMarketSocketConnected(VOID);
VOID    JNI_CryptoDotComMgr_ConnectMarketSocket(const CHAR* szURL);
BOOLEAN JNI_CryptoDotComMgr_SendMsgToMarketSocket(const CHAR* szMsg);
VOID    JNI_CryptoDotComMgr_CloseMarketSocket(VOID);

#endif //#if defined(ANDROID_NDK)
#endif // #if !defined(CRYPTO_DOT_COM_MGR_JNI_H)
