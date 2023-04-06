#include "stdafx.h"
#include "CMain.h"
#include "EventID.h"
#include "EventManager.h"
#include "AutoLogicData.h"
#include "LogicCheck.h"
#include "PatternData.h"

CLogicCheck::CLogicCheck() :
CGameWindow()
{
    m_bDestroyOnExitModal = TRUE;
    m_bRenderBlackOverlay = TRUE;
}
    
CLogicCheck::~CLogicCheck()
{
}
 
VOID CLogicCheck::InitializeInternals(VOID)
{
    CGameWindow::InitializeInternals();

    SetAnchor(0.5f, 0.5f);
    SetLocalPosition(CMain::GetScreenWidth() * 0.5f, CMain::GetScreenHeight() * 0.5f);
    SetLocalSize(1024.0f, 350.0f);
    const SHLVector2D& sSize = GetLocalSize();
    SetColorBackground(RGBA(0x1F, 0x1F, 0x1F, 0xFF));
    CreateColorButton(sSize.x - 40.0f, 10.0f, 30.0f, 30.0f, EBTN_QUIT, RGBA(0x4F, 0x4F, 0x4F, 0xFF), "X", TRUE);
    
    m_cList.SetLocalPosition(10.0f, 10.0f);
    const CHAR* szListHeaders[10] = { "Category", "Type", "LotSize", "Stop Loss", "TP1", "TP2", "TP3", "TP4", "TP5", "TP6" };
    const FLOAT afListCellWidth[10] = { 200.0f, 60.0f, 60.0f, 100.0f, 80.0f, 80.0f, 80.0f, 80.0f, 80.0f, 80.0f };
    m_cList.Initialize(szListHeaders, afListCellWidth, 10, sSize.y - 20.0f, EBTN_UNUSED);
    AddChild(m_cList);

    const UINT uNum = CAutoLogicDataMgr::GetLogicNum();
    const CHAR* szPtr[10];
    CHAR szLotSize[32];
    for (UINT uIndex = 0; uNum > uIndex; ++uIndex) {
        const SLogic& sLogic = CAutoLogicDataMgr::GetLogic(uIndex);
        szPtr[0] = CPatternDataMgr::GetPattern(sLogic.uCategoryPatternIndex).szName;
        szPtr[1] = sLogic.bLong ? "LONG" : "SHORT";
        szPtr[2] = szLotSize;
        snprintf(szLotSize, 32, "%d", sLogic.uLotSize);
        szPtr[3] = CAutoLogicDataMgr::GetTPEntry(sLogic.uStopLossTPIndex).szName;
        for (UINT uI = 0; TRADE_ORDER_TARGET_NUM > uI; ++uI) {
            if (TRADE_ORDER_UNUSED_TARGET == sLogic.anTargetIndex[uI]) {
                szPtr[4 + uI] = "-";
            }
            else {
                szPtr[4 + uI] = CAutoLogicDataMgr::GetTPEntry(sLogic.anTargetIndex[uI]).szName;
            }
        }
        m_cList.AddRow(szPtr, 10, uIndex);
    }
}

VOID CLogicCheck::Release(VOID)
{
    m_cList.Release();
    m_cList.RemoveFromParent();
    CGameWindow::Release();
}

VOID CLogicCheck::OnReceiveEvent(CEvent& cEvent)
{
    const UINT uEventID = cEvent.GetIntParam(0);
    switch (uEventID) {
        case EBTN_QUIT:
            ExitModal();
            break;
            
        default:
            break;
    }
}
