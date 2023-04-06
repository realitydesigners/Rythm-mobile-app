#include "stdafx.h"
#include "AutoWidget.h"
#include "CMain.h"
#include "EventID.h"
#include "EventManager.h"

CAutoWidget::CAutoWidget() :
CGameWindow(EGLOBAL_AUTO_WIDGET),
m_bRegistered(FALSE),
m_uPatternIndex(0)
{
    m_bDestroyOnExitModal = TRUE;
    m_bRenderBlackOverlay = TRUE;
}
    
CAutoWidget::~CAutoWidget()
{
}
 
VOID CAutoWidget::InitializeInternals(VOID)
{
    SetAnchor(0.5f, 0.5f);
    SetLocalPosition(CMain::GetScreenWidth() * 0.5f, CMain::GetScreenHeight() * 0.5f);
    SetLocalSize(AUTO_WIDGET_WIDTH, AUTO_WIDGET_HEIGHT);
    
    CGameWindow::InitializeInternals();
    m_cLogicList.SetLocalPosition(0.0f, 0.0f);
    m_cLogicList.SetLocalSize(AUTO_LOGIC_LIST_W, AUTO_GRAPH_PANEL_HEIGHT);
    m_cLogicList.Initialize(EEVENT_SELECTED_LOGIC);
    m_cLogicList.RefreshPatternList();
    AddChild(m_cLogicList);

    m_cGraph.SetLocalPosition(AUTO_LOGIC_LIST_W, 0.0f);
    m_cGraph.Initialize();
    AddChild(m_cGraph);
    
    m_cEditor.SetLocalPosition(AUTO_LOGIC_LIST_W + AUTO_GRAPH_PANEL_WIDTH, 0.0f);
    m_cEditor.Initialize();
    AddChild(m_cEditor);
    
    CreateColorButton(AUTO_LOGIC_LIST_W + AUTO_GRAPH_PANEL_WIDTH + AUTO_LOGIC_EDITOR_WIDTH - 105.0f, 5.0f, 100.0f, 30.0f, EBTN_QUIT, RGBA(0x4F, 0x4F, 0x4F, 0xFF), "Close", TRUE);
    OnSelected(0);
#if defined(MAC_PORT)
    CEventManager::RegisterForBroadcast(*this, EGLOBAL_MAC_SYSTEM_MGR, EGLOBAL_EVENT_MAC_SYSTEM_KEY_PRESSED);
#endif //#if defined(MAC_PORT)
}

VOID CAutoWidget::Release(VOID)
{
#if defined(MAC_PORT)
    CEventManager::UnRegisterForBroadcast(*this, EGLOBAL_MAC_SYSTEM_MGR, EGLOBAL_EVENT_MAC_SYSTEM_KEY_PRESSED);
#endif //#if defined(MAC_PORT)
    m_cLogicList.Release();
    m_cLogicList.RemoveFromParent();
    
    m_cGraph.Release();
    m_cGraph.RemoveFromParent();
    
    m_cEditor.Release();
    m_cEditor.RemoveFromParent();
    
    CGameWindow::Release();
}

VOID CAutoWidget::OnReceiveEvent(CEvent& cEvent)
{
    const UINT uEventID = cEvent.GetIntParam(0);
    switch (uEventID) {
        case EBTN_QUIT:
            ExitModal();
            break;
        case EEVENT_SELECTED_LOGIC:
            OnSelected(cEvent.GetIntParam(1));
            break;
#if defined(MAC_PORT)
        case EGLOBAL_EVENT_MAC_SYSTEM_KEY_PRESSED:
            switch (cEvent.GetIntParam(1)) {
                case EKEY_DIRECTION_UP:
                    AdjPattern(-1);
                    break;
                case EKEY_DIRECTION_DOWN:
                    AdjPattern(1);
                    break;
                default:
                    break;
            }
            break;
#endif // #if defined(MAC_PORT)
        default:
            break;
    }
}

VOID CAutoWidget::OnSelected(UINT uPatternIndex)
{
    m_uPatternIndex = uPatternIndex;
    m_cGraph.Refresh(uPatternIndex);
    m_cEditor.Refresh(uPatternIndex);
    m_cLogicList.OnSelectedWidget(uPatternIndex);
}

VOID CAutoWidget::AdjPattern(INT nAdj)
{
    const UINT uNum = CPatternDataMgr::GetPatternNum();
    if (0 < nAdj) {
        if ((uNum -1) > m_uPatternIndex) {
            OnSelected(m_uPatternIndex + 1);
        }
    }
    else {
        if (0 < m_uPatternIndex) {
            OnSelected(m_uPatternIndex - 1);
        }
    }
}
