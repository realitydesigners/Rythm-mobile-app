#include "stdafx.h"
#include "BibEditor.h"
#include "CMain.h"
#include "Event.h"
#include "EventManager.h"
#include "TextSelectorWindow.h"

#define WINDOW_WIDTH  (AUTO_GRAPH_PANEL_WIDTH + 30.0f + E_CONDITION_CELL_WIDTH)
#define WINDOW_HEIGHT (AUTO_GRAPH_PANEL_HEIGHT + 20.0f)

CBibEditor::CBibEditor() :
CGameWindow(EGLOBAL_BIB_EDITOR),
m_pcRowNumTxt(NULL),
m_uUsedRowNum(CEditConditionsTable::MAX_CONDITION_NUM)
{
    m_bRenderBlackOverlay = TRUE;
    m_bDestroyOnExitModal = TRUE;
}

CBibEditor::~CBibEditor()
{
}

VOID CBibEditor::InitializeInternals(VOID)
{
    CGameWindow::InitializeInternals();
    SetDefaultWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "");
    SetAnchor(0.5f, 0.5f);
    SetLocalPosition(CMain::GetScreenWidth() * 0.5f, CMain::GetScreenHeight() * 0.5f);
    CreateColorButton(WINDOW_WIDTH - 40.0f, 10.0f, 30.0f, 30.0f, EBTN_EXIT, RGBA(0x7F, 0x7F, 0x7F, 0xFF), "X", TRUE);
    CUIButton* pcBtn = CreateColorButton(WINDOW_WIDTH - 80.0f, 10.0f, 30.0f, 30.0f, EBTN_CHANGE_ROW_NUM, RGBA(0x7F, 0x7F, 0x7F, 0xFF), "6", TRUE);
    if (NULL != pcBtn) {
        m_pcRowNumTxt = CUITextLabel::CastToMe(pcBtn->GetChild());
    }
    const FLOAT fTblX = WINDOW_WIDTH - 10.0f - E_CONDITION_CELL_WIDTH;
    m_cTbl.SetLocalPosition(fTblX, 50.0f);
    m_cTbl.Initialize(EEVENT_REFRESH);
    AddChild(m_cTbl);
    
    m_cBox.SetLocalPosition(10.0f, 10.0f);
    m_cBox.Initialize();
    AddChild(m_cBox);

    UpdateLabel();
}

VOID CBibEditor::Release(VOID)
{
    m_cBox.Release();
    m_cBox.RemoveFromParent();
    
    m_cTbl.Release();
    m_cTbl.RemoveFromParent();
    
    CGameWindow::Release();
}

VOID CBibEditor::OnReceiveEvent(CEvent& cEvent)
{
    const UINT uEventID = cEvent.GetIntParam(0);
    switch (uEventID) {
        case EBTN_EXIT:
            DoExit();
            break;
        case EBTN_CHANGE_ROW_NUM:
            OnTapChangeRowNum();
            break;
        case EEVENT_USER_REPLY_ROW_NUM:
            OnUserReplyRowNum(cEvent.GetIntParam(2));
            break;
        case EEVENT_REFRESH:
            RefreshChart();
            break;
        default:
            break;
    }
}



VOID CBibEditor::DoExit(VOID)
{
    ExitModal();
}

VOID CBibEditor::UpdateLabel(VOID)
{
    if (NULL != m_pcRowNumTxt) {
        CHAR szBuffer[32];
        snprintf(szBuffer, 32, "%d", m_uUsedRowNum);
        m_pcRowNumTxt->SetString(szBuffer);
    }
}

VOID CBibEditor::OnTapChangeRowNum(VOID)
{
    CTextSelectorWindow* pcWin = new CTextSelectorWindow(*this, EEVENT_USER_REPLY_ROW_NUM, 0);
    if (NULL != pcWin) {
        const CHAR* szValues[7] = {
            "2", "3", "4", "5", "6", "7", "8"
        };
        pcWin->Initialize(szValues, 7, m_uUsedRowNum - 2);
        if (NULL != m_pcRowNumTxt) {
            const SHLVector2D& sWorldPos = m_pcRowNumTxt->GetWorldPosition();
            pcWin->SetLocalPosition(sWorldPos.x + 15.0f, sWorldPos.y + 15.0f);
        }
        pcWin->DoModal();
    }
}
VOID CBibEditor::OnUserReplyRowNum(UINT uRowIndex)
{
    m_uUsedRowNum = 2 + uRowIndex;
    m_cTbl.SetMaxRow(m_uUsedRowNum);
    RefreshChart();
    UpdateLabel();
}

VOID CBibEditor::RefreshChart(VOID)
{
    m_cBox.Refresh(m_cTbl.GetPatternRows(), m_uUsedRowNum);
}
