#include "stdafx.h"
#include "CryptoDotComMgr.h"
#if !defined(ANDROID_NDK) && !defined(WINDOWS_DEV)
#include "SRWebSocket.h"

@interface MarketDelegate : NSObject <SRWebSocketDelegate>
@end

@implementation MarketDelegate
- (void)webSocketDidOpen:(SRWebSocket *)webSocket;
{
    CCryptoDotComMgr::OnMarketSocketConnected();
}

- (void)webSocket:(SRWebSocket *)webSocket didFailWithError:(NSError *)error;
{
    CCryptoDotComMgr::OnMarketSocketConnectFail([[error description] UTF8String]);
}

- (void)webSocket:(SRWebSocket *)webSocket didReceiveMessageWithString:(nonnull NSString *)string
{
    CCryptoDotComMgr::GetInstance().OnReceiveMarketMessage([string UTF8String]);
}

- (void)webSocket:(SRWebSocket *)webSocket didCloseWithCode:(NSInteger)code reason:(NSString *)reason wasClean:(BOOL)wasClean;
{
    CCryptoDotComMgr::OnMarketSocketClosed([reason UTF8String]);
}

- (void)webSocket:(SRWebSocket *)webSocket didReceivePong:(NSData *)pongPayload;
{
    TRACE("Websocket received pong\n");
}
@end
static SRWebSocket*     s_pcMarketSocket = nil;
static MarketDelegate*  s_pcMarketDelegate = nil;
#else // #if !defined(ANDROID_NDK) && !defined(WINDOWS_DEV)
#include "CryptoDotComMgr.JNI.h"
#endif // #if !defined(ANDROID_NDK) && !defined(WINDOWS_DEV)

BOOLEAN CCryptoDotComMgr::IsMarketSocketConnectedInternal(VOID)
{
#if defined(WINDOWS_DEV)
    return FALSE;
#else // #if defined(WINDOWS_DEV)
#if !defined(ANDROID_NDK)
    if (nil == s_pcMarketSocket) {
        return FALSE;
    }
    return (SR_OPEN == s_pcMarketSocket.readyState);
#else // #if !defined(ANDROID_NDK)
    return JNI_CryptoDotComMgr_IsMarketSocketConnected();
#endif // #if !defined(ANDROID_NDK)
#endif // #if defined(WINDOWS_DEV)

}

VOID CCryptoDotComMgr::ConnectMarketSocketInternal(const CHAR* szURL)
{
#if defined(WINDOWS_DEV)

#else // #if defined(WINDOWS_DEV)
    CloseMarketSocketInternal(); // safety
#if !defined(ANDROID_NDK)
    s_pcMarketSocket = [[SRWebSocket alloc] initWithURL:[NSURL URLWithString:[NSString stringWithUTF8String:szURL]]];
    s_pcMarketDelegate = [[MarketDelegate alloc] init];
    s_pcMarketSocket.delegate = s_pcMarketDelegate;
    [s_pcMarketSocket open];
#else // #if !defined(ANDROID_NDK)
    JNI_CryptoDotComMgr_ConnectMarketSocket(szURL);
#endif // #if !defined(ANDROID_NDK)
#endif // #if defined(WINDOWS_DEV)
}

BOOLEAN CCryptoDotComMgr::SendMessageToMarketInternal(const CHAR* szMessage)
{
#if defined(WINDOWS_DEV)
    return FALSE;
#else // #if defined(WINDOWS_DEV)
#if !defined(ANDROID_NDK)
    if (NULL == s_pcMarketSocket) {
        ASSERT(FALSE);
        return FALSE;
    }
    if (SR_OPEN != s_pcMarketSocket.readyState) {
        TRACE("Market Socket is not ready yet!\n");
        return FALSE;
    }
    NSString* cNSString = [NSString stringWithUTF8String:szMessage];
    NSError* pcErr = nil;
    [s_pcMarketSocket sendString:cNSString error:&pcErr];
    if (nil != pcErr) {
        TRACE("Err: %s\n", [[pcErr description]UTF8String]);
        ASSERT(FALSE);
        return FALSE;
    }
    return TRUE;
#else // #if !defined(ANDROID_NDK)
    return JNI_CryptoDotComMgr_SendMsgToMarketSocket(szMessage);
#endif // #if !defined(ANDROID_NDK)
#endif // #if defined(WINDOWS_DEV)
}

VOID CCryptoDotComMgr::CloseMarketSocketInternal(VOID)
{
#if defined(WINDOWS_DEV)

#else // #if defined(WINDOWS_DEV)
#if !defined(ANDROID_NDK)
    if (nil != s_pcMarketSocket) {
        [s_pcMarketSocket close];
        s_pcMarketSocket = nil;
        s_pcMarketDelegate = nil;
    }
#else // #if !defined(ANDROID_NDK)
    JNI_CryptoDotComMgr_CloseMarketSocket();
#endif // #if !defined(ANDROID_NDK)
#endif // #if defined(WINDOWS_DEV)
}
