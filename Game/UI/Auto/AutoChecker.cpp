#include "stdafx.h"
#include "AutoChecker.h"
#include "CMain.h"
#include "EventID.h"
#include "EventManager.h"


CAutoChecker::CAutoChecker() :
CGameWindow()
{
    m_bDestroyOnExitModal = TRUE;
    m_bRenderBlackOverlay = TRUE;
}
    
CAutoChecker::~CAutoChecker()
{
}
 
VOID CAutoChecker::InitializeInternals(VOID)
{
    SetAnchor(0.5f, 0.5f);
    SetLocalPosition(CMain::GetScreenWidth() * 0.5f, CMain::GetScreenHeight() * 0.5f);
    SetLocalSize(300.0f, 170.0f);
    SetColorBackground(RGBA(0x1F, 0x1F, 0x1F, 0xFF));
    CreateColorButton(260.0f, 10.0f, 30.0f, 30.0f, EBTN_QUIT, RGBA(0x4F, 0x4F, 0x4F, 0xFF), "X", TRUE);
    
    CreateColorButton(50.0f, 10.0f, 200.0f, 30.0f, EBTN_PATTERN_CHECK, RGBA(0x4F, 0x4F, 0x4F, 0xFF), "Pattern Check", TRUE);
    CreateColorButton(50.0f, 50.0f, 200.0f, 30.0f, EBTN_TP_CHECK, RGBA(0x4F, 0x4F, 0x4F, 0xFF), "TPChart", TRUE);
    CreateColorButton(50.0f, 90.0f, 200.0f, 30.0f, EBTN_LOGIC_CHECK, RGBA(0x4F, 0x4F, 0x4F, 0xFF), "Main Logic", TRUE);
    CreateColorButton(50.0f, 130.0f, 200.0f, 30.0f, EBTN_BIB_EDITOR, RGBA(0x4F, 0x4F, 0x4F, 0xFF), "Bib Edit", TRUE);
    
    CGameWindow::InitializeInternals();
}

VOID CAutoChecker::Release(VOID)
{
    CGameWindow::Release();
}

#include "AutoWidget.h"
#include "TPChartCheck.h"
#include "LogicCheck.h"
#include "BibEditor.h"
VOID CAutoChecker::OnReceiveEvent(CEvent& cEvent)
{
    const UINT uEventID = cEvent.GetIntParam(0);
    switch (uEventID) {
        case EBTN_QUIT:
            ExitModal();
            break;
        case EBTN_PATTERN_CHECK:
            {
                CAutoWidget* pcWidget = new CAutoWidget();
                if (NULL != pcWidget) {
                    pcWidget->DoModal();
                }
            }
            break;
       
        case EBTN_TP_CHECK:
            {
                CTPChartCheck* pcWin = new CTPChartCheck();
                if (NULL != pcWin) {
                    pcWin->DoModal();
                }
            }
            break;
        case EBTN_LOGIC_CHECK:
            {
                CLogicCheck* pcWin = new CLogicCheck();
                if (NULL != pcWin) {
                    pcWin->DoModal();
                }
            }
            break;
        case EBTN_BIB_EDITOR:
            {
                CBibEditor* pcEditor = new CBibEditor();
                if (NULL != pcEditor) {
                    pcEditor->DoModal();
                }
            }
            break;
        default:
            break;
    }
}
