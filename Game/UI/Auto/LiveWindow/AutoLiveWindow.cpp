#include "stdafx.h"
#include "AutoLiveWindow.h"
#include "AutomationMgr.h"
#include "CMain.h"
#include "Event.h"
#include "EventID.h"
#include "EventManager.h"
#include "FractalData.h"

#define WINDOW_WIDTH    (360.0f)
#define WINDOW_HEIGHT   (260.0f)


VOID CAutoLiveWindow::PostEvent(CEvent& cEvent, FLOAT fDelay)
{
    // do not post to parent.
    CEventManager::PostEvent(*this, cEvent, fDelay);
}


CAutoLiveWindow::CAutoLiveWindow(ESOURCE_MARKET eSource, UINT uTradePairNum) :
CGameWindow(),
m_eSource(eSource),
m_uTradePairNum(uTradePairNum),
m_pcStartBtn(NULL),
m_pcStopBtn(NULL),
m_pcFractalBtn(NULL),
m_pcTitle(NULL),
m_pcEntryTime(NULL),
m_pcCloseTime(NULL),
m_cList(EEVENT_SELECTED_LOGIC),
m_uSelectedLogicIndex(0)
{
    m_bDestroyOnExitModal = TRUE;
    SetLocalSize(WINDOW_WIDTH, WINDOW_HEIGHT);
}

CAutoLiveWindow::~CAutoLiveWindow()
{
}

VOID CAutoLiveWindow::Init(VOID)
{
    InitializeInternals();
}

#define LIGHT_GREY      RGBA(0x7F,0x7F,0x7F,0xFF)
#define MEDIUM_GREY     RGBA(0x4F,0x4F,0x4F,0xFF)
VOID CAutoLiveWindow::InitializeInternals(VOID)
{
    CGameWindow::InitializeInternals();
    m_pcTitle = SetDefaultWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Automation");
    if (NULL != m_pcTitle) {
        m_pcTitle->SetLocalPosition(WINDOW_WIDTH * 0.5f, 15.0f);
    }
    m_pcStartBtn = CreateColorButton(5.0f, 35.0f, 40.0f, 20.0f, EBTN_START, MEDIUM_GREY, "Start", TRUE);
    m_pcStopBtn = CreateColorButton(5.0f, 60.0f, 40.0f, 20.0f, EBTN_STOP, MEDIUM_GREY, "Stop", TRUE);
    m_pcFractalBtn = CreateColorButton(WINDOW_WIDTH - 95.0f, 5.0f, 90.0f, 20.0f, EBTN_CHANGE_FRACTAL_SET, MEDIUM_GREY, "F Set 1", TRUE);
    m_pcEntryTime = CreateLabel("", ELABEL_SMALL);
    if (NULL != m_pcEntryTime) {
        m_pcEntryTime->SetLocalPosition(50.0f, 45.0f);
        m_pcEntryTime->SetAnchor(0.0f, 0.5f);
        AddChild(*m_pcEntryTime);
    }
    m_pcCloseTime = CreateLabel("", ELABEL_SMALL);
    if (NULL != m_pcCloseTime) {
        m_pcCloseTime->SetLocalPosition(50.0f, 70.0f);
        m_pcCloseTime->SetAnchor(0.0f, 0.5f);
        AddChild(*m_pcCloseTime);
    }
    
    CreateColorImage(1.0f, 30.0f, WINDOW_WIDTH - 2.0f, 1.0f, LIGHT_GREY, TRUE);

    m_cList.SetLocalPosition(0.0f, 85.0f);
    m_cList.SetLocalSize(WINDOW_WIDTH, WINDOW_HEIGHT - 85.0f);
    AddChild(m_cList);
    m_cList.Initialize();
    
    m_cDetails.Initialize();
    m_cDetails.SetLocalPosition(WINDOW_WIDTH + 5.0f, 0.0f);
    
    UpdateStatus();
    UpdateFractalSetLabel();
    CEventManager::RegisterForBroadcast(*this, EGLOBAL_SYSTEM, EGLOBAL_EVENT_SYSTEM_HEARTBEAT);
}

VOID CAutoLiveWindow::Release(VOID)
{
    CEventManager::UnRegisterForBroadcast(*this, EGLOBAL_SYSTEM, EGLOBAL_EVENT_SYSTEM_HEARTBEAT);
    
    m_cList.Release();
    m_cList.RemoveFromParent();
    
    m_cDetails.Release();
    m_cDetails.RemoveFromParent();
    
    CGameWindow::Release();
}

VOID CAutoLiveWindow::OnRegainFocus(VOID)
{
    UpdateStatus();
}
VOID CAutoLiveWindow::OnReceiveEvent(CEvent& cEvent)
{
    const UINT uEventID = cEvent.GetIntParam(0);
    switch (uEventID) {
        case EBTN_START:
            OnTapStartBtn();
            break;
        case EBTN_STOP:
            OnTapStopBtn();
            break;
        case EGLOBAL_EVENT_SYSTEM_HEARTBEAT:
            UpdateStatus();
            break;
        case EEVENT_SELECTED_LOGIC:
            OnSelectedLogic(cEvent.GetIntParam(1));
            break;
        case EBTN_REATTACH:
            {
                CUIContainer* pcParent = GetParent();
                if (NULL != pcParent) {
                    pcParent->ReAttachChild(*this);
                }
            }
            break;
        case EBTN_CHANGE_FRACTAL_SET:
            OnTapFractalBtn();
            break;
        case EEVENT_SELECTED_FRACTAL_INDEX:
            OnSelectedFractalIndex(cEvent.GetIntParam(2));
            break;
        default:
            break;
    }
}

VOID CAutoLiveWindow::UpdateStatus(VOID)
{
    CAutomationMgr& cMgr = CAutomationMgr::GetInstance();
    CHAR szBuffer[1024];
    if (NULL != m_pcTitle) {
        snprintf(szBuffer, 1024, "Auto %s", cMgr.HasStarted() ? "STARTED" : "STOPPED");
        m_pcTitle->SetString(szBuffer);
        m_pcTitle->SetColor(cMgr.HasStarted() ? RGBA(80, 255, 80, 255) : RGBA(255, 80, 80, 255));
    }
    if (cMgr.IsMarketClosedForWeekend()) {
        if (NULL != m_pcEntryTime) {
            m_pcEntryTime->SetString("Weekend Closure");
        }
        if (NULL != m_pcCloseTime) {
            m_pcCloseTime->ClearString();
        }
        return;
    }
    if (NULL != m_pcEntryTime) {
        UINT uTime;
        const CHAR* szString;
        if (cMgr.CanEnterNewTrades()) {
            szString = "Entry Stops in";
            uTime = cMgr.GetSecondsToStopEntry();
        }
        else {
            szString = "Entry Starts in";
            uTime = cMgr.GetSecondsToOpen();
        }
        UINT uHr = 0;
        UINT uMin = 0;
        UINT uSec = 0;
        uHr = uTime / (60*60);
        uMin = (uTime / 60) % 60;
        uSec = (uTime % 60);
        snprintf(szBuffer, 1024, "%s %02d:%02d:%02d", szString, uHr, uMin, uSec);
        m_pcEntryTime->SetString(szBuffer);
    }
    if (NULL != m_pcCloseTime) {
        const UINT uTime = cMgr.GetSecondsToCloseAllTrade();
        if (0 == uTime) {
            snprintf(szBuffer, 1024, "Closing in --:--:--");
        }
        else {
            const UINT uHr = uTime / (60*60);
            const UINT uMin = (uTime / 60) % 60;
            const UINT uSec = (uTime % 60);
            snprintf(szBuffer, 1024, "Closing in %02d:%02d:%02d", uHr, uMin, uSec);
        }
        m_pcCloseTime->SetString(szBuffer);
    }
    if (NULL != m_cDetails.GetParent()) {
        m_cDetails.RefreshList(m_uSelectedLogicIndex);        
    }
}

VOID CAutoLiveWindow::OnTapStartBtn(VOID)
{
    CAutomationMgr& cMgr = CAutomationMgr::GetInstance();
    if (cMgr.IsMarketClosedForWeekend()) {
        return;
    }
    if (cMgr.HasStarted()) {
        return;
    }
    cMgr.Start();
    UpdateStatus();
}
VOID CAutoLiveWindow::OnTapStopBtn(VOID)
{
    CAutomationMgr& cMgr = CAutomationMgr::GetInstance();
    if (!cMgr.HasStarted()) {
        return;
    }
    cMgr.Stop();
    UpdateStatus();
}

VOID CAutoLiveWindow::OnSelectedLogic(UINT uLogicIndex)
{
    m_uSelectedLogicIndex = uLogicIndex;
    m_cDetails.RefreshList(uLogicIndex);
    if (NULL == m_cDetails.GetParent()) {
        AddChild(m_cDetails);
    }
}


BOOLEAN CAutoLiveWindow::OnTouchBegin(FLOAT fX, FLOAT fY)
{
    m_bUITouched = CUIContainer::OnTouchBegin(fX, fY);
    if (m_bUITouched) {
        return TRUE;
    }
    const SHLVector2D sPos = { fX, fY };
    const BOOLEAN bHit = HitTest(sPos);
    if (bHit) {
        m_bScrollMode = FALSE;
        m_afPrevTouch[0] = fX;
        m_afPrevTouch[1] = fY;
        ReattachToParent();
        return TRUE;
    }
    return FALSE;
}
BOOLEAN CAutoLiveWindow::OnTouchMove(FLOAT fX, FLOAT fY)
{
    if (m_bUITouched) {
        return CUIContainer::OnTouchMove(fX, fY);
    }
    const FLOAT fDiffX = fX - m_afPrevTouch[0];
    const FLOAT fDiffY = fY - m_afPrevTouch[1];
    if (!m_bScrollMode) {
        if ((fDiffX * fDiffX + fDiffY * fDiffY) > 25.0f) {
            m_bScrollMode = TRUE;
            m_afPrevTouch[0] = fX;
            m_afPrevTouch[1] = fY;
        }
        return TRUE;
    }
    m_afPrevTouch[0] = fX;
    m_afPrevTouch[1] = fY;
    SHLVector2D sLocalPos = GetLocalPosition();
    sLocalPos.x += fDiffX;
    sLocalPos.y += fDiffY;
    SetLocalPosition(sLocalPos.x, sLocalPos.y);
    return TRUE;
}

BOOLEAN CAutoLiveWindow::OnTouchEnd(FLOAT fX, FLOAT fY)
{
    if (m_bUITouched) {
        return CUIContainer::OnTouchEnd(fX, fY);
    }
    m_bScrollMode = FALSE;
    return TRUE;
}

VOID CAutoLiveWindow::ReattachToParent(VOID)
{
    CEvent cEvent(EBTN_REATTACH);
    CEventManager::PostEvent(*this, cEvent);
}


VOID CAutoLiveWindow::UpdateFractalSetLabel(VOID)
{
    if (NULL == m_pcFractalBtn) {
        return;
    }
    CUITextLabel* pcTxt = CUITextLabel::CastToMe(m_pcFractalBtn->GetChild());
    if (NULL == pcTxt) {
        return;
    }
    CHAR szBuffer[512];
    snprintf(szBuffer, 512, "F %s", CFractalDataMgr::GetFractal(CAutomationMgr::GetInstance().GetFractalIndex()).szName);
    pcTxt->SetString(szBuffer);
}


#include "TextSelectorWindow.h"
#include "CMain.h"
VOID CAutoLiveWindow::OnTapFractalBtn(VOID)
{
    const UINT uNum = CFractalDataMgr::GetNum();
    CHAR** aszPtr = new CHAR*[uNum];
    if (NULL == aszPtr) {
        ASSERT(FALSE);
        return;
    }
    for (UINT uIndex = 0; uNum > uIndex; ++uIndex) {
        aszPtr[uIndex] = new CHAR[256];
        if (NULL == aszPtr[uIndex]) {
            ASSERT(FALSE);
            return;
        }
        const SFractal& sFractal = CFractalDataMgr::GetFractal(uIndex);
        snprintf(aszPtr[uIndex], 256, "%s\t%3.2f\t%3.2f\t%2.2f\t%2.2f\t%2.2f\t%2.2f\t%2.2f\t%2.2f\t%2.2f\t%d", sFractal.szName,
                 sFractal.afPipSize[0],
                 sFractal.afPipSize[1],
                 sFractal.afPipSize[2],
                 sFractal.afPipSize[3],
                 sFractal.afPipSize[4],
                 sFractal.afPipSize[5],
                 sFractal.afPipSize[6],
                 sFractal.afPipSize[7],
                 sFractal.afPipSize[8],
                 sFractal.uBoxNum);
    }
    CTextSelectorWindow* pcWin = new CTextSelectorWindow(*this, EEVENT_SELECTED_FRACTAL_INDEX, 0);
    if (NULL != pcWin) {
        pcWin->Initialize((const CHAR**)aszPtr, uNum, CAutomationMgr::GetInstance().GetFractalIndex());
        pcWin->SetLocalPosition(CMain::GetScreenWidth() * 0.5f, CMain::GetScreenHeight() * 0.5f);
        pcWin->SetAnchor(0.5f, 0.5f);
        pcWin->DoModal();
    }
    for (UINT uIndex = 0; uNum > uIndex; ++uIndex) {
        delete [] aszPtr[uIndex];
    }
    delete [] aszPtr;
}

VOID CAutoLiveWindow::OnSelectedFractalIndex(UINT uIndex)
{
    const UINT uNum = CFractalDataMgr::GetNum();
    if (uNum <= uIndex) {
        ASSERT(FALSE);
        return;
    }
    CAutomationMgr::GetInstance().ChangeFractalIndex(uIndex);
    UpdateFractalSetLabel();
}
