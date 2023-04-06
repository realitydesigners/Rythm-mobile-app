#include "stdafx.h"
#if defined(MAC_PORT)
#include "UIEditor.h"
#include "CMain.h"
#include "EventManager.h"
#include "MacSystem.h"
#include "ConfirmationWindow.h"
#include "GameRenderer.h"
#include "NibManager.h"

CUIEditor::CUIEditor() :
CGameWindow(),
INITIALIZE_TEXT_LABEL(m_cWorkingDirectory),
m_bEdited(FALSE),
m_pcBaseWidget(NULL),
m_bPickingMode(FALSE),
m_bResizeMode(FALSE),
m_bAnchorMode(FALSE),
m_uLastEventID(0),
m_bInputEnabled(FALSE),
m_bAttachingMode(FALSE)
{
    m_bDestroyOnExitModal = TRUE;
    m_szUIFolder[0] = 0;
}

CUIEditor::~CUIEditor()
{
}

#define CREATE_DEBUG_BTN(x, y, id, text) CreateColorButton(x, y, 100.0f, 40.0f, id, RGBA(0x7F, 0x7F, 0x7F, 0x7F), text, TRUE)
VOID CUIEditor::InitializeInternals(VOID)
{
    CGameWindow::InitializeInternals();
    const FLOAT fScreenW = CMain::GetScreenWidth();
    const FLOAT fScreenH = CMain::GetScreenHeight();
    m_pcWindowTitle = SetDefaultWindow(fScreenW, fScreenH, "UI Editor");
    SetAnchor(0.5f, 0.5f);
    SetLocalPosition(fScreenW * 0.5f, fScreenH * 0.5f);
    CREATE_DEBUG_BTN(10.0f, 10.0f, EBTN_NEW_JSON, "New");
    CREATE_DEBUG_BTN(10.0f, 60.0f, EBTN_LOAD_JSON, "Load");
    CREATE_DEBUG_BTN(10.0f, 110.0f, EBTN_CHANGE_PATH, "Change Path");
    CREATE_DEBUG_BTN(120.0f, 10.0f, EBTN_SAVE_JSON, "Save");
    
    m_pcToggleInput = NULL;
    CUIButton* pcBtn = CREATE_DEBUG_BTN(120.0f, 60.0f, EBTN_TOGGLE_INPUT, "Enable Input");
    if (NULL != pcBtn) {
        m_pcToggleInput = CUITextLabel::CastToMe(pcBtn->GetChild());
    }
    
    m_pcResolution = NULL;
    pcBtn = CreateColorButton(230.0f, 10.0f, 200.0f, 40.0f, EBTN_CHANGE_RESOLUTION, RGBA(0x7F, 0x7F, 0x7F, 0x7F), "", TRUE);
    if (NULL != pcBtn) {
        m_pcResolution = CUITextLabel::CastToMe(pcBtn->GetChild());
    }
    CREATE_DEBUG_BTN(440.0f, 10.0f, EBTN_CREATE_IMAGE, "Create Image");
    CREATE_DEBUG_BTN(550.0f, 10.0f, EBTN_CREATE_TEXT, "Create Text");
    CREATE_DEBUG_BTN(660.0f, 10.0f, EBTN_CREATE_CONTAINER, "Create Container");
    CREATE_DEBUG_BTN(1140.0f, 10.0f, EBTN_SET_CLASSNAME, "Set Classname");
    CREATE_DEBUG_BTN(1250.0f, 10.0f, EBTN_OUTPUT_BINARY, "Create Nib File");
    m_cBorder.SetColor(RGBA(0x7F, 0, 0, 0x2F));
    AddChild(m_cBorder);
    AddChild(m_cBaseContainer);
    
    AdjResolution(0);

    strncpy(m_szUIFolder, CMacSystem::GetApplicationPath(), 1024);
    CNibManager::SetWorkingDirectory(m_szUIFolder);
    CNibManager::SetButtonDefaultEventID(EBTN_NO_OP);
    
    m_cWorkingDirectory.SetFont(EGAMEFONT_SIZE_14);
    m_cWorkingDirectory.SetString(m_szUIFolder);
    m_cWorkingDirectory.SetLocalSize(fScreenW - 100.0f, 20.0f);
    m_cWorkingDirectory.SetAnchor(0.5f, 0.5f);
    m_cWorkingDirectory.SetLocalPosition(fScreenW * 0.5f, fScreenH - 20.0f);
    AddChild(m_cWorkingDirectory);
    
    InitEditor();
}

VOID CUIEditor::Release(VOID)
{
    ReleaseEditor();
    
    m_cWorkingDirectory.ClearString();
    m_cWorkingDirectory.RemoveFromParent();
    m_cBaseContainer.RemoveFromParent();
    m_cBorder.RemoveFromParent();
    CGameWindow::Release();
}

VOID CUIEditor::OnLoadBtnPressed(BOOLEAN bOverride)
{
    if (m_bEdited && !bOverride) {
        CConfirmationWindow* pcWin = new CConfirmationWindow(this);
        pcWin->SetTitle("Load Template");
        pcWin->SetMessage("You have unsaved changes, continue?");
        pcWin->SetOKButton("Continue", EBTN_LOAD_JSON_CONFIRM);
        pcWin->DoModal();
        return;
    }
    m_uLastEventID = EBTN_LOAD_JSON;
    CEventManager::RegisterForBroadcast(*this, EGLOBAL_MAC_SYSTEM_MGR, EGLOBAL_EVENT_MAC_SYSTEM_FILE_OPEN_PATH_CHOSEN, TRUE);
    CMacSystem::TryOpenFile("json");
}
VOID CUIEditor::OnNewUI(BOOLEAN bOverride)
{
    if (m_bEdited && !bOverride) {
        CConfirmationWindow* pcWin = new CConfirmationWindow(this);
        pcWin->SetTitle("New Template");
        pcWin->SetMessage("You have unsaved changes, continue?");
        pcWin->SetOKButton("Continue", EBTN_NEW_JSON_CONFIRM);
        pcWin->DoModal();
        return;
    }
    CHAR szFilePath[1024];
    snprintf(szFilePath, 1024, "%s/%s", m_szUIFolder, NEW_NIB_FILENAME);
    DoLoadNib(szFilePath);
    m_bEdited = TRUE;
}

static VOID RecursiveRenderContainer(CUIContainer* pcContainer, BOOLEAN bSkip)
{
    if (NULL == pcContainer) {
        return;
    }
    if (!bSkip) {
        const SHLVector2D& sWorldPos = pcContainer->GetWorldPosition();
        const SHLVector2D& sWorldSize = pcContainer->GetWorldSize();
        const FLOAT fLeftX = sWorldPos.x - WIDGET_LINE_BORDER;
        const FLOAT fRightX = sWorldPos.x + sWorldSize.x + WIDGET_LINE_BORDER;
        const FLOAT fTopY = sWorldPos.y - WIDGET_LINE_BORDER;
        const FLOAT fBottomY = sWorldPos.y + sWorldSize.y + WIDGET_LINE_BORDER;
        CGameRenderer::DrawLine(fLeftX, fTopY, fRightX, fTopY, RGBA(0, 0xFF, 0, 0xFF));
        CGameRenderer::DrawLine(fLeftX, fTopY, fLeftX, fBottomY, RGBA(0, 0xFF, 0, 0xFF));
        CGameRenderer::DrawLine(fRightX, fBottomY, fRightX, fTopY, RGBA(0, 0xFF, 0, 0xFF));
        CGameRenderer::DrawLine(fLeftX, fBottomY, fRightX, fBottomY, RGBA(0, 0xFF, 0, 0xFF));
    }
    CUIWidget* pcChild = pcContainer->GetChild();
    while (NULL != pcChild) {
        pcContainer = CUIContainer::CastToMe(pcChild);
        if (NULL != pcContainer) {
            RecursiveRenderContainer(pcContainer, FALSE);
        }
        pcChild = pcChild->GetNextSibling();
    }
}

VOID CUIEditor::OnRender(FLOAT fOffsetX, FLOAT fOffsetY) const
{
    CGameWindow::OnRender(fOffsetX, fOffsetY);
    for ( auto& pcWidget : m_cSelectedWidget ) {
        const SHLVector2D& sWorldPos = pcWidget->GetWorldPosition();
        const SHLVector2D& sWorldSize = pcWidget->GetWorldSize();
        const FLOAT fLeftX = sWorldPos.x - WIDGET_LINE_BORDER;
        const FLOAT fRightX = sWorldPos.x + sWorldSize.x + WIDGET_LINE_BORDER;
        const FLOAT fTopY = sWorldPos.y - WIDGET_LINE_BORDER;
        const FLOAT fBottomY = sWorldPos.y + sWorldSize.y + WIDGET_LINE_BORDER;
        CGameRenderer::DrawLine(fLeftX, fTopY, fRightX, fTopY, RGBA(0xFF, 0, 0, 0xFF));
        CGameRenderer::DrawLine(fLeftX, fTopY, fLeftX, fBottomY, RGBA(0xFF, 0, 0, 0xFF));
        CGameRenderer::DrawLine(fRightX, fBottomY, fRightX, fTopY, RGBA(0xFF, 0, 0, 0xFF));
        CGameRenderer::DrawLine(fLeftX, fBottomY, fRightX, fBottomY, RGBA(0xFF, 0, 0, 0xFF));
        CGameRenderer::DrawRectNoTex(fRightX - WIDGET_RESIZE_SIZE * 0.5f, fBottomY - WIDGET_RESIZE_SIZE * 0.5f, WIDGET_RESIZE_SIZE, WIDGET_RESIZE_SIZE, RGBA(0, 0, 0xFF, 0xFF));
        // also render anchor point.
        const SHLVector2D& sAnchor = pcWidget->GetAnchor();
        const FLOAT fAnchorX = sWorldPos.x + sWorldSize.x * sAnchor.x;
        const FLOAT fAnchorY = sWorldPos.y + sWorldSize.y * sAnchor.y;
        
        CGameRenderer::DrawLine(fAnchorX - WIDGET_RESIZE_SIZE, fAnchorY - WIDGET_RESIZE_SIZE, fAnchorX + WIDGET_RESIZE_SIZE, fAnchorY + WIDGET_RESIZE_SIZE, RGBA(0xFF, 0, 0, 0xFF));
        CGameRenderer::DrawLine(fAnchorX + WIDGET_RESIZE_SIZE, fAnchorY - WIDGET_RESIZE_SIZE, fAnchorX - WIDGET_RESIZE_SIZE, fAnchorY + WIDGET_RESIZE_SIZE, RGBA(0xFF, 0, 0, 0xFF));
    }
    if (m_bAttachingMode) {
        RecursiveRenderContainer(m_pcBaseWidget, TRUE);
    }
}

BOOLEAN CUIEditor::OnTouchBegin(FLOAT fX, FLOAT fY)
{
    m_bResizeMode = FALSE; // safety
    m_bAnchorMode = FALSE;  // safety
    m_bPickingMode = !CGameWindow::OnTouchBegin(fX, fY);
    if (!m_bPickingMode) {
        return FALSE;
    }
    if (CanDoResize(fX, fY)) {
        m_afLastPickPos[0] = fX;
        m_afLastPickPos[1] = fY;
        m_bResizeMode = TRUE;
        m_bPickingMode = TRUE;
        return TRUE;
    }
    if (CanMoveAnchor(fX, fY)) {
        m_afLastPickPos[0] = fX;
        m_afLastPickPos[1] = fY;
        m_bAnchorMode = TRUE;
        m_bPickingMode = TRUE;
        return TRUE;
    }
    m_afLastPickPos[0] = fX;
    m_afLastPickPos[1] = fY;
    if (m_bAttachingMode) {
        DoAttachPick(TryPick(fX, fY));
    }
    else {
        CUIWidget* pcPickedWidget = TryPick(fX, fY);
        if (CMacSystem::IsShiftKeyPressed()) {
            if (NULL != pcPickedWidget) {
                for (auto& pcWidget : m_cSelectedWidget) {
                    if (pcWidget == pcPickedWidget) { // already picked before
                        return TRUE;
                    }
                }
                m_cSelectedWidget.push_back(pcPickedWidget);
            }
        }
        else {
            m_cSelectedWidget.clear();
            if (NULL != pcPickedWidget) {
                m_cSelectedWidget.push_back(pcPickedWidget);
                RefreshLabels();
            }
        }
    }
    
    return TRUE;
}
BOOLEAN CUIEditor::OnTouchMove(FLOAT fX, FLOAT fY)
{
    if (!m_bPickingMode) {
        return CGameWindow::OnTouchMove(fX, fY);
    }
    const FLOAT fDiffX = fX - m_afLastPickPos[0];
    const FLOAT fDiffY = fY - m_afLastPickPos[1];
    m_afLastPickPos[0] = fX;
    m_afLastPickPos[1] = fY;
    TryMoveSelectedWidget(fDiffX, fDiffY);
    return TRUE;
}
BOOLEAN CUIEditor::OnTouchEnd(FLOAT fX, FLOAT fY)
{
    if (!m_bPickingMode) {
        return CGameWindow::OnTouchEnd(fX, fY);
    }
    
    return TRUE;
}

CUIWidget* CUIEditor::GetSelectedWidget(VOID) const
{
    if (0 == m_cSelectedWidget.size()) {
        return NULL;
    }
    return *(m_cSelectedWidget.begin());
}
    
#endif // #if defined(MAC_PORT)
