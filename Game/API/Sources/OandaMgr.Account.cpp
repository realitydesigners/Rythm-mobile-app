#include "stdafx.h"
#include "AutoPtr.h"
#include "ConfirmationWindow.h"
#include "EventID.h"
#include "EventManager.h"
#include "GameUtil.h"
#include "json.h"
#include "MessageLog.h"
#include "OandaMgr.h"
#include "PlayerData.h"
#include "SaveData.h"
#include "SimpleHttpClient.h"
#include "System.h"
#include "TextInputWindow.h"


#define OANDA_ACCOUNTS_URL      OANDA_MAIN_URL "v3/accounts"
#define OANDA_ACCOUNT_DETAILS   OANDA_MAIN_URL "v3/accounts/%s"

static CHAR s_szInputBuffer[256];

BOOLEAN COandaMgr::IsSignedIn(VOID) const
{
    return (0 < m_uAccountNum);
}

UINT COandaMgr::GetAccountNum(VOID) const
{
    return m_uAccountNum;
}
COandaMgr::SOandaAccount* COandaMgr::GetAccount(UINT uAccountIndex)
{
    if (NULL == m_psAccounts || m_uAccountNum <= uAccountIndex) {
        return NULL;
    }
    return &m_psAccounts[uAccountIndex];
}

const CHAR* COandaMgr::GetAccountID(UINT uAccountIndex) const
{
    if (NULL == m_psAccounts || m_uAccountNum <= uAccountIndex) {
        ASSERT(FALSE);
        return NULL;
    }
    return m_psAccounts[uAccountIndex].szAccountID;
}

VOID COandaMgr::TrySignIn(VOID)
{
    if (IsSignedIn()) {
        CEventManager::BroadcastEvent(EGLOBAL_OANDA_MGR, CEvent(EGLOBAL_EVT_OANDA_SIGN_IN, TRUE));
        return;
    }
    CHAR szCredentials[128];
    if (!CPlayerData::GetOandaCredential(szCredentials, 128)) {
        ASSERT(FALSE);
        CEventManager::BroadcastEvent(EGLOBAL_OANDA_MGR, CEvent(EGLOBAL_EVT_OANDA_SIGN_IN, FALSE));
        return;
    }
    if (0 == szCredentials[0]) { //empty string!
        CEvent cReply(EGLOBAL_INTERNAL_OANDA_ENTER_CREDENTIAL);
        CTextInputWindow* pcWin = new CTextInputWindow(s_szInputBuffer, 256, *this, cReply);
        if (NULL != pcWin) {
            pcWin->SetTitleAndActionText("Oanda Credentials", "Set");
            pcWin->DoNotAllowQuit();
            pcWin->DoModal();
        }
        else {
            ASSERT(FALSE);
            CEventManager::BroadcastEvent(EGLOBAL_OANDA_MGR, CEvent(EGLOBAL_EVT_OANDA_SIGN_IN, FALSE));
        }
        return;
    }
    CHAR szBuffer[1024];
    snprintf(szBuffer, 1024, "Bearer %s", szCredentials);
    Json::Value cHeaderFields;
    cHeaderFields["Authorization"] = szBuffer;
    const Json::Value* pcParams = new Json::Value();
    CSimpleHttpClient::DoGet(OANDA_ACCOUNTS_URL, *pcParams, EGLOBAL_OANDA_MGR, EGLOBAL_INTERNAL_OANDA_GET_ACCOUNT, &cHeaderFields);
}

VOID COandaMgr::OnCredentialInput(VOID)
{
    if (0 == s_szInputBuffer[0]) {
        TrySignIn();
        return;
    }
    GameUtil::TrimString(s_szInputBuffer, 256);
    CPlayerData::SetOandaCredential(s_szInputBuffer);
    CSaveData::SaveData(FALSE);
    TrySignIn();
}

VOID COandaMgr::OnGetAccountReply(const CEvent& cEvent)
{
    const Json::Value* pcJson   = (const Json::Value*)cEvent.GetPtrParam(5);
    const INT nErrCode          = cEvent.GetIntParam(6);
    const CHAR* szResult        = (const CHAR*) cEvent.GetPtrParam(7);
    CAutoPtr<const Json::Value> cJson(pcJson);
    CAutoPtrArray<const CHAR> cResult(szResult);

    if (CSimpleHttpClient::ESuccess != nErrCode) {
        CHAR szBuffer[1024];
        snprintf(szBuffer, 1024, "Oanda Err Code:%d", nErrCode);
        CConfirmationWindow::DisplayErrorMsg(szBuffer, NULL, 0);
        CEventManager::BroadcastEvent(EGLOBAL_OANDA_MGR, CEvent(EGLOBAL_EVT_OANDA_SIGN_IN, FALSE));
        CPlayerData::SetOandaCredential("");
        CSaveData::SaveData(FALSE);
        return;
    }
    Json::Reader cReader;
    Json::Value cData;
    if (!cReader.parse(szResult, cData) || !cData.isObject()) {
        ASSERT(FALSE);
        CEventManager::BroadcastEvent(EGLOBAL_OANDA_MGR, CEvent(EGLOBAL_EVT_OANDA_SIGN_IN, FALSE));
        return;
    }
    const Json::Value& cAccounts = cData["accounts"];
    if (!cAccounts.isArray()) {
        ASSERT(FALSE);
        CEventManager::BroadcastEvent(EGLOBAL_OANDA_MGR, CEvent(EGLOBAL_EVT_OANDA_SIGN_IN, FALSE));
        return;
    }
    ReleaseAccounts();
    const UINT uAccountNum = (UINT)cAccounts.size();
    m_psAccounts = new SOandaAccount[uAccountNum];
    if (NULL == m_psAccounts) {
        ASSERT(FALSE);
        CEventManager::BroadcastEvent(EGLOBAL_OANDA_MGR, CEvent(EGLOBAL_EVT_OANDA_SIGN_IN, FALSE));
        return;
    }
    for (UINT uIndex = 0; uAccountNum > uIndex; ++uIndex) {
        SOandaAccount& sAcct = m_psAccounts[uIndex];
        sAcct.szAccountID = NULL;
        sAcct.bHegdingEnabled = FALSE;
        sAcct.szCurrency = NULL;
        sAcct.dBalance = 0.0;
        sAcct.dMarginAvailable = 0.0;
        sAcct.dMarginUsed = 0.0;
        ASSERT(0 == sAcct.cPendingOrderList.size());
        ASSERT(0 == sAcct.cOpenTradeList.size());
        const Json::Value& cAccountID = cAccounts[uIndex]["id"];
        if (!cAccountID.isString()) {
            ASSERT(FALSE);
            CEventManager::BroadcastEvent(EGLOBAL_OANDA_MGR, CEvent(EGLOBAL_EVT_OANDA_SIGN_IN, FALSE));
            return;
        }
        const CHAR* szAccountID = cAccountID.asCString();
        if (NULL == szAccountID) {
            ASSERT(FALSE);
            CEventManager::BroadcastEvent(EGLOBAL_OANDA_MGR, CEvent(EGLOBAL_EVT_OANDA_SIGN_IN, FALSE));
            return;
        }
        sAcct.szAccountID = CSystem::CloneString(szAccountID);
    }
    m_uAccountNum = uAccountNum;
    CEventManager::BroadcastEvent(EGLOBAL_OANDA_MGR, CEvent(EGLOBAL_EVT_OANDA_SIGN_IN, TRUE));
}

VOID COandaMgr::QueryAccountDetails(UINT uAccountIndex)
{
    const CHAR* szAccountID = GetAccountID(uAccountIndex);
    if (NULL == szAccountID) { // must be already signed in
        ASSERT(FALSE);
        CEventManager::BroadcastEvent(EGLOBAL_OANDA_MGR, CEvent(EGLOBAL_EVT_OANDA_ACCOUNT_DETAILS_UPDATED, uAccountIndex, FALSE));
        return;
    }
    CHAR szCredentials[128];
    if (!CPlayerData::GetOandaCredential(szCredentials, 128)) {
        ASSERT(FALSE);
        CEventManager::BroadcastEvent(EGLOBAL_OANDA_MGR, CEvent(EGLOBAL_EVT_OANDA_ACCOUNT_DETAILS_UPDATED, uAccountIndex, FALSE));
        return;
    }
    CHAR szBuffer[1024];
    snprintf(szBuffer, 1024, "Bearer %s", szCredentials);
    Json::Value cHeaderFields;
    cHeaderFields["Authorization"] = szBuffer;
    cHeaderFields["Accept-Datetime-Format"] = "UNIX"; // unix time
    CHAR szURL[2048];
    snprintf(szURL, 2048, OANDA_ACCOUNT_DETAILS, szAccountID);
    Json::Value* pcParams = new Json::Value();
    (*pcParams)["nounce"] = uAccountIndex;
    CSimpleHttpClient::DoGet(szURL, *pcParams, EGLOBAL_OANDA_MGR, EGLOBAL_INTERNAL_OANDA_GET_ACCOUNT_DETAILS, &cHeaderFields);
}

VOID COandaMgr::OnGetAccountDetailsReply(const CEvent& cEvent)
{
    const Json::Value* pcJson   = (const Json::Value*)cEvent.GetPtrParam(5);
    const UINT uAccountIndex    = (*pcJson)["nounce"].asUInt();
    const INT nErrCode          = cEvent.GetIntParam(6);
    const CHAR* szResult        = (const CHAR*) cEvent.GetPtrParam(7);
    CAutoPtr<const Json::Value> cJson(pcJson);
    CAutoPtrArray<const CHAR> cResult(szResult);
    if (CSimpleHttpClient::ESuccess != nErrCode) {
        CMessageLog::AddLog("Fetch Account Details Error");
        if (NULL != szResult) {
            CMessageLog::AddLog(szResult);
        }
        CEventManager::BroadcastEvent(EGLOBAL_OANDA_MGR, CEvent(EGLOBAL_EVT_OANDA_ACCOUNT_DETAILS_UPDATED, uAccountIndex, FALSE));
        return;
    }
    Json::Reader cReader;
    Json::Value cData;
    if (!cReader.parse(szResult, cData) || !cData.isObject()) {
        ASSERT(FALSE);
        CEventManager::BroadcastEvent(EGLOBAL_OANDA_MGR, CEvent(EGLOBAL_EVT_OANDA_ACCOUNT_DETAILS_UPDATED, uAccountIndex, FALSE));
        return;
    }
    SOandaAccount* psAcct = GetAccount(uAccountIndex);
    if (NULL == psAcct) {
        ASSERT(FALSE);
        CEventManager::BroadcastEvent(EGLOBAL_OANDA_MGR, CEvent(EGLOBAL_EVT_OANDA_ACCOUNT_DETAILS_UPDATED, uAccountIndex, FALSE));
        return;
    }
    const Json::Value& cAccount = cData["account"];
    const Json::Value& cHedgingEnabled = cAccount["hedgingEnabled"];
    if (!cHedgingEnabled.isBool()) {
        ASSERT(FALSE);
        CEventManager::BroadcastEvent(EGLOBAL_OANDA_MGR, CEvent(EGLOBAL_EVT_OANDA_ACCOUNT_DETAILS_UPDATED, uAccountIndex, FALSE));
        return;
    }
    psAcct->bHegdingEnabled = cHedgingEnabled.asBool();
    const Json::Value& cCurrency = cAccount["currency"];
    if (!cCurrency.isString()) {
        ASSERT(FALSE);
        CEventManager::BroadcastEvent(EGLOBAL_OANDA_MGR, CEvent(EGLOBAL_EVT_OANDA_ACCOUNT_DETAILS_UPDATED, uAccountIndex, FALSE));
        return;
    }
    SAFE_DELETE_ARRAY(psAcct->szCurrency); // safety
    psAcct->szCurrency = CSystem::CloneString(cCurrency.asCString());
    
    const Json::Value& cBalance = cAccount["balance"];
    if (!cBalance.isString()) {
        ASSERT(FALSE);
        CEventManager::BroadcastEvent(EGLOBAL_OANDA_MGR, CEvent(EGLOBAL_EVT_OANDA_ACCOUNT_DETAILS_UPDATED, uAccountIndex, FALSE));
        return;
    }
    psAcct->dBalance = std::stod(cBalance.asCString());
    
    const Json::Value& cMarginUsed = cAccount["marginUsed"];
    if (!cMarginUsed.isString()) {
        ASSERT(FALSE);
        CEventManager::BroadcastEvent(EGLOBAL_OANDA_MGR, CEvent(EGLOBAL_EVT_OANDA_ACCOUNT_DETAILS_UPDATED, uAccountIndex, FALSE));
        return;
    }
    psAcct->dMarginUsed = std::stod(cMarginUsed.asCString());
    
    const Json::Value& cMarginAvailable = cAccount["marginAvailable"];
    if (!cMarginAvailable.isString()) {
        ASSERT(FALSE);
        CEventManager::BroadcastEvent(EGLOBAL_OANDA_MGR, CEvent(EGLOBAL_EVT_OANDA_ACCOUNT_DETAILS_UPDATED, uAccountIndex, FALSE));
        return;
    }
    psAcct->dMarginAvailable = std::stod(cMarginAvailable.asCString());
    CEventManager::BroadcastEvent(EGLOBAL_OANDA_MGR, CEvent(EGLOBAL_EVT_OANDA_ACCOUNT_DETAILS_UPDATED, uAccountIndex, TRUE));
}
