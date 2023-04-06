#include "stdafx.h"
#include "LogUI.h"
#include "HLTime.h"

CLogUI::CLogUI() :
CUIContainer(),
INITIALIZE_TEXT_LABEL(m_cDate),
INITIALIZE_TEXT_LABEL(m_cMsg)
{
    
}
    
CLogUI::~CLogUI()
{
}
 
static const CHAR* s_aszMths[12] = {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

#define DATE_WIDTH (120.0f)
BOOLEAN CLogUI::Initialize(const SMessageLog& sLog)
{
    const SHLVector2D& sSize = GetLocalSize();
    const UINT uLocalTimeSec = CHLTime::GetLocalTimeSecs(sLog.uTimeStamp);
    {
        
        UINT uMth, uDay;
        CHLTime::GetMonthAndDay(uLocalTimeSec, uMth, uDay);
        UINT uHr, uMin, uSec;
        CHLTime::GetHHMMSS(uLocalTimeSec, uHr, uMin, uSec);
        CHAR szDateBuffer[256];
        snprintf(szDateBuffer, 256, "%d %s %d:%02d:%02d", uDay, s_aszMths[uMth%12], uHr, uMin, uSec);
        m_cDate.SetFont(EGAMEFONT_SIZE_12);
        m_cDate.SetString(szDateBuffer);
        m_cDate.SetAnchor(0.0f, 0.0f);
        m_cDate.SetLocalPosition(5.0f, 3.0f);
    }
    m_cMsg.SetFont(EGAMEFONT_SIZE_12);
    m_cMsg.SetString(sLog.szMessage);
    m_cMsg.SetAutowrap(sSize.x - DATE_WIDTH);
    m_cMsg.SetLocalPosition(5.0f + DATE_WIDTH, 3.0f);
    m_cMsg.OnUpdate(0.0f);
    const SHLVector2D& sMsgSize = m_cMsg.GetLocalSize();
    const FLOAT fMsgH = sMsgSize.y + 6.0f;
    const FLOAT fH = sSize.y < fMsgH ? fMsgH : sSize.y;
    SetLocalSize(sSize.x, fH);
    m_cBG.SetLocalSize(sSize.x, fH);
    m_cBG.SetColor(RGBA(30, 41, 54, 0xFF));

    AddChild(m_cBG);
    AddChild(m_cDate);
    AddChild(m_cMsg);
    return TRUE;
}

VOID CLogUI::Release(VOID)
{
    m_cBG.RemoveFromParent();

    m_cDate.Release();
    m_cDate.RemoveFromParent();
    
    m_cMsg.Release();
    m_cMsg.RemoveFromParent();
}
