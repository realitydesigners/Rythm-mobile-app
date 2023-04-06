#include "stdafx.h"
#include "CMain.h"
#include "EventID.h"
#include "EventManager.h"
#include "AutoLogicData.h"
#include "PatternData.h"
#include "TPChartCheck.h"

CTPChartCheck::CTPChartCheck() :
CGameWindow()
{
    m_bDestroyOnExitModal = TRUE;
    m_bRenderBlackOverlay = TRUE;
}
    
CTPChartCheck::~CTPChartCheck()
{
}
 
VOID CTPChartCheck::InitializeInternals(VOID)
{
    CGameWindow::InitializeInternals();

    SetAnchor(0.5f, 0.5f);
    SetLocalPosition(CMain::GetScreenWidth() * 0.5f, CMain::GetScreenHeight() * 0.5f);
    SetLocalSize(560.0f, 430.0f);
    const SHLVector2D& sSize = GetLocalSize();
    SetColorBackground(RGBA(0x1F, 0x1F, 0x1F, 0xFF));
    CreateColorButton(sSize.x - 40.0f, 10.0f, 30.0f, 30.0f, EBTN_QUIT, RGBA(0x4F, 0x4F, 0x4F, 0xFF), "X", TRUE);
    
    m_cList.SetLocalPosition(10.0f, 10.0f);
    const CHAR* szListHeaders[6] = { "Name", "Type", "Pip", "Frame", "Percent", "TrailStop" };
    const FLOAT afListCellWidth[6] = { 100.0f, 100.0f, 60.0f, 50.0f, 60.0f, 70.0f };
    m_cList.Initialize(szListHeaders, afListCellWidth, 6, 410.0f, EBTN_SELECT);
    AddChild(m_cList);
    
    const UINT uNum = CAutoLogicDataMgr::GetTPEntryNum();
    const CHAR* szPtr[6];
    CHAR szPip[64];
    CHAR szFrame[32];
    CHAR szPercent[32];
    CHAR szTrailStop[64];
    for (UINT uIndex = 0; uNum > uIndex; ++uIndex) {
        const STPEntry& sEntry = CAutoLogicDataMgr::GetTPEntry(uIndex);
        szPtr[0] = sEntry.szName;
        szPtr[1] = sEntry.eType == ETP_PIP_ONLY ? "Pip" : "Frame";
        szPtr[2] = szPip;
        snprintf(szPip, 32, "%.1f", sEntry.fPip);
        szPtr[3] = szFrame;
        snprintf(szFrame, 32, "%d", sEntry.byFrameIndex);
        szPtr[4] = szPercent;
        snprintf(szPercent, 32, "%.1f%%", sEntry.fPercent * 100.0f);
        szPtr[5] = szTrailStop;
        snprintf(szTrailStop, 32, "%.1f", sEntry.fTrailStop);
        m_cList.AddRow(szPtr, 6, uIndex);
    }
    OnSelectedIndex(0);
}

VOID CTPChartCheck::Release(VOID)
{
    m_cList.Release();
    m_cList.RemoveFromParent();
    
    CGameWindow::Release();
}

VOID CTPChartCheck::OnReceiveEvent(CEvent& cEvent)
{
    const UINT uEventID = cEvent.GetIntParam(0);
    switch (uEventID) {
        case EBTN_QUIT:
            ExitModal();
            break;
        case EBTN_SELECT:
            OnSelectedIndex(cEvent.GetIntParam(1));
            break;
        default:
            break;
    }
}

VOID CTPChartCheck::OnSelectedIndex(UINT uIndex)
{
    const UINT uNum = CAutoLogicDataMgr::GetTPEntryNum();
    if (uNum <= uIndex) {
        ASSERT(FALSE);
        return;
    }
    m_cList.SetSelected(uIndex);
}
