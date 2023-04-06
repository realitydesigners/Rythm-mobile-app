#include "stdafx.h"
#include "AppResourceManager.h"
#include "AutoPtr.h"
#include "EventID.h"
#include "EventManager.h"
#include "HLTime.h"
#include "MessageLog.h"
#include "System.h"
#include <list>

#define MAX_LOG_NUM         (500)
#define LOG_SAVE_INTERVAL   (10.0f)
#define SAFE_FILE_NAME      "MsgLog.dat"
#define MSG_LOG_VERSION     (0x00000001)

struct SMessageLogHeader {
    UINT uVersion;
    UINT uLogNum;
};
// timestsamp x msgnum
// offsets to char sequences x msg num
// chara sequences.

static CMessageLogList s_cList;
static BOOLEAN         s_bDirty = FALSE;
static FLOAT           s_fTimeToSave = LOG_SAVE_INTERVAL;

static VOID DoSave(VOID)
{
    // determine file size
    const UINT uLogNum = (UINT)s_cList.size();
    UINT uSize = sizeof(SMessageLogHeader) +    // header
                (sizeof(UINT) * uLogNum * 2);   // time and message buffer offsets
    UINT uLogOffset = uSize;
    for (const auto& sLog : s_cList) {
        uSize += ((UINT)strlen(sLog.szMessage) + 1);
    }
    BYTE* pbyBuffer = new BYTE[uSize];
    if (NULL == pbyBuffer) {
        ASSERT(FALSE);
        return;
    }
    CAutoPtrArray<BYTE> cGuard(pbyBuffer);
    SMessageLogHeader* psHeader = (SMessageLogHeader*)pbyBuffer;
    psHeader->uVersion = MSG_LOG_VERSION;
    psHeader->uLogNum = uLogNum;
    UINT* puTimeStamps = (UINT*)(psHeader + 1);
    UINT* puLogOffsets = (UINT*)(puTimeStamps + uLogNum);
    CHAR* szLogBuffer = (CHAR*)(puLogOffsets + uLogNum);
    
    UINT uLogIndex = 0;
    for (const auto& sLog : s_cList) {
        ASSERT(uLogNum > uLogIndex);
        puTimeStamps[uLogIndex] = sLog.uTimeStamp;
        puLogOffsets[uLogIndex] = uLogOffset;
        ASSERT(((UINT64)szLogBuffer - (UINT64)pbyBuffer) == uLogOffset);
        const UINT uLen = (UINT)strlen(sLog.szMessage) + 1;
        if (uSize < (uLogOffset + uLen)) {
            ASSERT(FALSE);
            return;
        }
        memcpy(szLogBuffer, sLog.szMessage, uLen);
        szLogBuffer += uLen;
        uLogOffset += uLen;
        ++uLogIndex;
    }
    ASSERT(uLogOffset == uSize);
    CAppResourceManager::WriteToLocalStorage(SAFE_FILE_NAME, pbyBuffer, uSize);
}

BOOLEAN CMessageLog::Initialize(VOID)
{
    Clear();
    UINT uDataSize = 0;
    BYTE* pbyBuffer = CAppResourceManager::ReadFromLocalStorage(SAFE_FILE_NAME, uDataSize);
    if (NULL == pbyBuffer) {
        AddLog("Log Not Found, clearing Log");
        s_bDirty = TRUE;
        return TRUE;
    }
    CAutoPtrArray<BYTE> cGuardBuffer(pbyBuffer);
    const SMessageLogHeader* psHeader = (const SMessageLogHeader*)pbyBuffer;
    if (MSG_LOG_VERSION != psHeader->uVersion) {
        AddLog("Log Version Changed, clearing Log");
        s_bDirty = TRUE;
        return TRUE;
    }
    const UINT uLogNum = psHeader->uLogNum;
    const UINT* puTimeStamps = (const UINT*)(psHeader+1);
    const UINT* pszMessageOffsets = (const UINT*)(puTimeStamps + uLogNum);
    for (UINT uIndex = 0; uLogNum > uIndex; ++uIndex) {
        const UINT uTimeStamp = puTimeStamps[uIndex];
        const CHAR* szMessage = (const CHAR*)(pbyBuffer + pszMessageOffsets[uIndex]);
        const CHAR* szCloned = CSystem::CloneString(szMessage);
        const SMessageLog sTmp = {
            uTimeStamp,
            szCloned
        };
        s_cList.push_back(sTmp);
    }
    s_bDirty = FALSE;
    s_fTimeToSave = LOG_SAVE_INTERVAL;
    return TRUE;
}

VOID CMessageLog::Release(VOID)
{
    Clear();
}

VOID CMessageLog::Update(FLOAT fLapsed)
{
    if (s_bDirty) {
        s_fTimeToSave -= fLapsed;
        if (0.0f >= s_fTimeToSave) {
            s_fTimeToSave = LOG_SAVE_INTERVAL;
            s_bDirty = FALSE;
            // do file saving
            DoSave();
        }
    }
}
VOID CMessageLog::Clear(VOID)
{
    for (auto & sMsg : s_cList) {
        SAFE_DELETE_ARRAY(sMsg.szMessage);
    }
    s_cList.clear();
    s_bDirty = TRUE;
    CEventManager::BroadcastEvent(EGLOBAL_MESSAGE_LOG, CEvent(EGLOBAL_EVT_MSG_LOG_REFRESHED));
}
VOID CMessageLog::AddLog(const CHAR* szMessage)
{
    ASSERT(NULL != szMessage);
    const CHAR* szCloneMsg = CSystem::CloneString(szMessage);
    if (NULL == szCloneMsg) {
        ASSERT(FALSE);
        return;
    }
    const UINT uNowSec = CHLTime::GetTimeSecs();
    const SMessageLog sTmp = {
        uNowSec,
        szCloneMsg
    };
    s_cList.push_front(sTmp);
    while (MAX_LOG_NUM < s_cList.size()) {
        const SMessageLog& sLog = *(s_cList.rbegin());
        ASSERT(NULL != sLog.szMessage);
        delete [] sLog.szMessage;
        s_cList.pop_back();
    }
    s_bDirty = TRUE;
    CEventManager::BroadcastEvent(EGLOBAL_MESSAGE_LOG, CEvent(EGLOBAL_EVT_MSG_LOG_REFRESHED));
}

const CMessageLogList& CMessageLog::GetLogs(VOID)
{
    return s_cList;
}
