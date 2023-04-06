#include "stdafx.h"
#if defined(MAC_PORT)
#include "UIEditor.h"
#include "EventManager.h"
#include "TextInputWindow.h"
#include "CMain.h"
#include "MacSystem.h"
#include "NibManager.h"

static CHAR s_szStringBuffer[256];
static UINT s_uTextEventID = 0;

struct SResolutionInfo {
    UINT uW;
    UINT uH;
    const CHAR* szName;
};

static const SResolutionInfo RESOLUTIONS[] = {
    { 960,  640,    "Resolution: 960x640" },
    { 1136, 640,    "Resolution: 1136x640 (iPhone5)" },
    { 1334, 750,    "Resolution: 1334x750 (iPhone8)" },
    { 2208, 1242,   "Resolution: 2208x1242 (iPhone8 Plus)" },
    { 2436, 1125,   "Resolution: 2436x1125 (iPhone X)" },
    { 2048, 1536,   "Resolution: 2048x1536 (iPad)" },
    { 2224, 1668,   "Resolution: 2224x1668 (iPad Pro)" },
    { 2732, 2048,   "Resolution: 2732x2048 (iPad Pro 12\")" },
};
static const UINT RESOLUTION_NUM = sizeof(RESOLUTIONS)/sizeof(SResolutionInfo);

static UINT s_uCurrentResolution = 0;
VOID CUIEditor::AdjResolution(INT nAdj)
{
    s_uCurrentResolution = (s_uCurrentResolution + RESOLUTION_NUM + nAdj) % RESOLUTION_NUM;
    const SResolutionInfo& sResolution = RESOLUTIONS[s_uCurrentResolution];
    if (NULL != m_pcResolution) {
        m_pcResolution->SetString(sResolution.szName);
    }
    SViewPortInfo sInfo;
    CMain::GetViewPortInfo(sInfo, sResolution.uW, sResolution.uH, TRUE);
    TRACE("Game: (%d x %d) VP: (%d x %d) Offset: (%d x %d) Scale: %f\n", sInfo.uGameWidth, sInfo.uGameHeight, sInfo.uViewportWidth, sInfo.uViewportHeight, sInfo.uViewportXOffset, sInfo.uViewportYOffset, sInfo.fScaleFactor);
    m_cBorder.SetLocalPosition(300.0f, 80.0f);
    m_cBorder.SetLocalSize(sInfo.uViewportWidth * sInfo.fScaleFactor, sInfo.uViewportHeight * sInfo.fScaleFactor);
    
    m_cBaseContainer.SetLocalSize(sInfo.uGameWidth, sInfo.uGameHeight);
    m_cBaseContainer.SetLocalPosition(300.0f + sInfo.uViewportXOffset * sInfo.fScaleFactor, 80.0f + sInfo.uViewportYOffset * sInfo.fScaleFactor);
    m_cBaseContainer.UpdateWorldCoordinates(TRUE);
    if (NULL != m_pcBaseWidget) {
        SNibEditInfo* psNib = FindNib(m_pcBaseWidget);
        if (NULL == psNib) {
            return;
        }
        CNibManager::UpdateWidgetFromNibWithParentSize(*psNib, *m_pcBaseWidget, m_cBaseContainer.GetLocalSize(), GetNibMap());
    }
}
VOID CUIEditor::OnTextInput(VOID)
{
    const UINT uPrevEventID = s_uTextEventID;
    s_uTextEventID = 0;
    CUIWidget* pcSelectedWidget = GetSelectedWidget();
    if (NULL == pcSelectedWidget) {
        return;
    }
    SNibEditInfo* psNib = FindNib(pcSelectedWidget);
    if (NULL == psNib) {
        return;
    }
    switch (uPrevEventID) {
        case EBTN_SET_CLASSNAME:
            m_cClassName = s_szStringBuffer;
            if (NULL != m_pcWindowTitle) {
                CHAR szBuffer[1024];
                snprintf(szBuffer, 1024, "UI Editor (%s)", m_cClassName.c_str());
                m_pcWindowTitle->SetString(szBuffer);
            }
            break;
        case EBTN_CHANGE_NAME:
            psNib->cName = s_szStringBuffer;
            break;
        case EBTN_CHANGE_WIDTH:
            psNib->sSize.afValues[0] = atof(s_szStringBuffer);
            break;
        case EBTN_CHANGE_HEIGHT:
            psNib->sSize.afValues[1] = atof(s_szStringBuffer);
            break;
        case EBTN_CHANGE_ASPECT:
            psNib->sSize.fAspectRatio = atof(s_szStringBuffer);
            break;
        case EBTN_CHANGE_X:
            psNib->sPos.afValues[0] = atof(s_szStringBuffer);
            break;
        case EBTN_CHANGE_Y:
            psNib->sPos.afValues[1] = atof(s_szStringBuffer);
            break;
        case EBTN_CHANGE_AX:
            psNib->sAnchor.x = atof(s_szStringBuffer);
            break;
        case EBTN_CHANGE_AY:
            psNib->sAnchor.y = atof(s_szStringBuffer);
            break;

        default:
            TRACE("Invalid Text EventID: %d\n", s_uTextEventID);
            return;
    }
    m_bEdited = TRUE;
    RefreshLabels();
    UpdateWidgetFromNib(*psNib, *pcSelectedWidget);
}
VOID CUIEditor::RequireTextInput(UINT uEventID)
{
    CUIWidget* pcSelectedWidget = GetSelectedWidget();
    if (NULL == pcSelectedWidget) {
        return;
    }
    const SNibEditInfo* psNib = FindNib(pcSelectedWidget);
    if (NULL == psNib) {
        return;
    }
    s_uTextEventID = uEventID;
    
    CEvent cEvent(EEVENT_TEXT_INPUT_REPLY);
    CTextInputWindow* pcWin = new CTextInputWindow(s_szStringBuffer, 255, *this, cEvent);
    if (NULL == pcWin) {
        return;
    }
    switch (uEventID) {
        case EBTN_SET_CLASSNAME:
            strncpy(s_szStringBuffer, m_cClassName.c_str(), 256);
            pcWin->SetTitleAndActionText("Change Name", "Change");
            break;
        case EBTN_CHANGE_NAME:
            strncpy(s_szStringBuffer, psNib->cName.c_str(), 256);
            pcWin->SetTitleAndActionText("Change Name", "Change");
            break;
        case EBTN_CHANGE_WIDTH:
            snprintf(s_szStringBuffer, 256, "%.3f", psNib->sSize.afValues[0]);
            pcWin->SetTitleAndActionText("Change Width", "Change");
            break;
        case EBTN_CHANGE_HEIGHT:
            snprintf(s_szStringBuffer, 256, "%.3f", psNib->sSize.afValues[1]);
            pcWin->SetTitleAndActionText("Change Height", "Change");
            break;
        case EBTN_CHANGE_ASPECT:
            snprintf(s_szStringBuffer, 256, "%.3f", psNib->sSize.fAspectRatio);
            pcWin->SetTitleAndActionText("Change Aspect Ratio", "Change");
            break;
        case EBTN_CHANGE_X:
            snprintf(s_szStringBuffer, 256, "%.3f", psNib->sPos.afValues[0]);
            pcWin->SetTitleAndActionText("Change X", "Change");
            break;
        case EBTN_CHANGE_Y:
            snprintf(s_szStringBuffer, 256, "%.3f", psNib->sPos.afValues[1]);
            pcWin->SetTitleAndActionText("Change Y", "Change");
            break;
        case EBTN_CHANGE_AX:
            snprintf(s_szStringBuffer, 256, "%.3f", psNib->sAnchor.x);
            pcWin->SetTitleAndActionText("Change Anchor X", "Change");
            break;
        case EBTN_CHANGE_AY:
            snprintf(s_szStringBuffer, 256, "%.3f", psNib->sAnchor.y);
            pcWin->SetTitleAndActionText("Change Anchor Y", "Change");
            break;
        default:
            ASSERT(FALSE);
            return;
    }
    pcWin->DoModal();
}

VOID CUIEditor::InitEditor(VOID)
{
    CEventManager::RegisterForBroadcast(*this, EGLOBAL_MAC_SYSTEM_MGR, EGLOBAL_EVENT_MAC_SYSTEM_KEY_PRESSED);
    m_pcWidgetName = NULL;
    m_pcWidgetFixedSize = NULL;
    m_pcWidgetSizeX = NULL;
    m_pcWidgetSizeY = NULL;
    m_pcWidgetAspectRatio = NULL;
    m_pcPositionType = NULL;
    m_pcPosXAlign = NULL;
    m_pcPosYAlign = NULL;
    m_pcWidgetPosX = NULL;
    m_pcWidgetPosY = NULL;
    m_pcWidgetAnchorX = NULL;
    m_pcWidgetAnchorY = NULL;
    m_pcWidgetType = NULL;
    CUIButton* pcBtn = CreateColorButton(10.0f, 170.0f, 270.0f, 40.0, EBTN_CHANGE_NAME, RGBA(0x7F, 0x7F, 0x7F, 0xFF), "", TRUE);
    if (NULL != pcBtn) {
        m_pcWidgetName = CUITextLabel::CastToMe(pcBtn->GetChild());
    }
    pcBtn = CreateColorButton(10.0f, 220.0f, 130.0f, 40.0, EBTN_TOGGLE_FIXED_SIZE, RGBA(0x7F, 0x7F, 0x7F, 0xFF), "", TRUE);
    if (NULL != pcBtn) {
        m_pcWidgetFixedSize = CUITextLabel::CastToMe(pcBtn->GetChild());
    }
    pcBtn = CreateColorButton(150.0f, 220.0f, 130.0f, 40.0, EBTN_CHANGE_ASPECT, RGBA(0x7F, 0x7F, 0x7F, 0xFF), "", TRUE);
    if (NULL != pcBtn) {
        m_pcWidgetAspectRatio = CUITextLabel::CastToMe(pcBtn->GetChild());
    }
    pcBtn = CreateColorButton(10.0f, 270.0f, 130.0f, 40.0, EBTN_CHANGE_WIDTH, RGBA(0x7F, 0x7F, 0x7F, 0xFF), "", TRUE);
    if (NULL != pcBtn) {
        m_pcWidgetSizeX = CUITextLabel::CastToMe(pcBtn->GetChild());
    }
    pcBtn = CreateColorButton(150.0f, 270.0f, 130.0f, 40.0, EBTN_CHANGE_HEIGHT, RGBA(0x7F, 0x7F, 0x7F, 0xFF), "", TRUE);
    if (NULL != pcBtn) {
        m_pcWidgetSizeY = CUITextLabel::CastToMe(pcBtn->GetChild());
    }
    
    // position
    pcBtn = CreateColorButton(10.0f, 340.0f, 270.0f, 40.0, EBTN_TOGGLE_POSITION_FIXED_SIZE, RGBA(0x7F, 0x7F, 0x7F, 0xFF), "", TRUE);
    if (NULL != pcBtn) {
        m_pcPositionType = CUITextLabel::CastToMe(pcBtn->GetChild());
    }
    pcBtn = CreateColorButton(10.0f, 390.0f, 130.0f, 40.0, EBTN_TOGGLE_POSITION_X_ALIGNMENT, RGBA(0x7F, 0x7F, 0x7F, 0xFF), "", TRUE);
    if (NULL != pcBtn) {
        m_pcPosXAlign = CUITextLabel::CastToMe(pcBtn->GetChild());
    }
    pcBtn = CreateColorButton(150.0f, 390.0f, 130.0f, 40.0, EBTN_TOGGLE_POSITION_Y_ALIGNMENT, RGBA(0x7F, 0x7F, 0x7F, 0xFF), "", TRUE);
    if (NULL != pcBtn) {
        m_pcPosYAlign = CUITextLabel::CastToMe(pcBtn->GetChild());
    }
    pcBtn = CreateColorButton(10.0f, 440.0f, 130.0f, 40.0, EBTN_CHANGE_X, RGBA(0x7F, 0x7F, 0x7F, 0xFF), "", TRUE);
    if (NULL != pcBtn) {
        m_pcWidgetPosX = CUITextLabel::CastToMe(pcBtn->GetChild());
    }
    pcBtn = CreateColorButton(150.0f, 440.0f, 130.0f, 40.0, EBTN_CHANGE_Y, RGBA(0x7F, 0x7F, 0x7F, 0xFF), "", TRUE);
    if (NULL != pcBtn) {
        m_pcWidgetPosY = CUITextLabel::CastToMe(pcBtn->GetChild());
    }
    pcBtn = CreateColorButton(10.0f, 510.0f, 130.0f, 40.0, EBTN_CHANGE_AX, RGBA(0x7F, 0x7F, 0x7F, 0xFF), "", TRUE);
    if (NULL != pcBtn) {
        m_pcWidgetAnchorX = CUITextLabel::CastToMe(pcBtn->GetChild());
    }
    pcBtn = CreateColorButton(150.0f, 510.0f, 130.0f, 40.0, EBTN_CHANGE_AY, RGBA(0x7F, 0x7F, 0x7F, 0xFF), "", TRUE);
    if (NULL != pcBtn) {
        m_pcWidgetAnchorY = CUITextLabel::CastToMe(pcBtn->GetChild());
    }
    pcBtn = CreateColorButton(10.0f, 580.0f, 270.0f, 40.0, EBTN_TOGGLE_TYPE, RGBA(0x7F, 0x7F, 0x7F, 0xFF), "", TRUE);
    if (NULL != pcBtn) {
        m_pcWidgetType = CUITextLabel::CastToMe(pcBtn->GetChild());
    }
    CreateColorButton(10.0f, 650.0f, 270.0f, 40.0f, EBTN_MOVE_SELECTED_TO_FRONT, RGBA(0x7F, 0x7F, 0x7F, 0xFF), "Move To Front", TRUE);
    pcBtn = CreateColorButton(10.0f, 700.0f, 270.0f, 40.0f, EBTN_ATTACH_DETACH_CONTAINER, RGBA(0x7F, 0x7F, 0x7F, 0xFF), "", TRUE);
    if (NULL != pcBtn) {
        m_pcAttachDetach = CUITextLabel::CastToMe(pcBtn->GetChild());
    }
    CreateColorButton(10.0f, 750.0f, 270.0f, 40.0f, EBTN_DUPLICATE, RGBA(0x7F, 0x7F, 0x7F, 0xFF), "Duplicate", TRUE);
    CreateColorButton(10.0f, 830.0f, 270.0f, 40.0f, EBTN_CONVERT_IMAGE_TO_BUTTON, RGBA(0x7F, 0x7F, 0x7F, 0xFF), "Convert To Button", TRUE);
}

VOID CUIEditor::RefreshLabels(VOID)
{
    SNibEditInfo* psNib = NULL;
    CUIWidget* pcSelectedWidget = GetSelectedWidget();
    if (NULL != pcSelectedWidget) {
        psNib = CUIEditor::FindNib(pcSelectedWidget);
    }
    CHAR szBuffer[256];
    if (NULL == psNib) {
        m_pcWidgetName->ClearString();
        m_pcWidgetFixedSize->ClearString();
        m_pcWidgetSizeX->ClearString();
        m_pcWidgetSizeY->ClearString();
        m_pcWidgetAspectRatio->ClearString();
        m_pcPositionType->ClearString();
        m_pcPosXAlign->ClearString();
        m_pcPosYAlign->ClearString();
        m_pcWidgetPosX->ClearString();
        m_pcWidgetPosY->ClearString();
        m_pcWidgetAnchorX->ClearString();
        m_pcWidgetAnchorY->ClearString();
        m_pcWidgetType->ClearString();
        m_pcAttachDetach->ClearString();
    }
    else {
        snprintf(szBuffer, 256, "Name:%s", psNib->cName.c_str());
        m_pcWidgetName->SetString(szBuffer);
        
        snprintf(szBuffer, 256, "Size:%s", psNib->sSize.bFixedSize ? "Fixed" : "Ratio");
        m_pcWidgetFixedSize->SetString(szBuffer);
        snprintf(szBuffer, 256, "W:%.2f", psNib->sSize.afValues[0]);
        m_pcWidgetSizeX->SetString(szBuffer);
        snprintf(szBuffer, 256, "H:%.2f", psNib->sSize.afValues[1]);
        m_pcWidgetSizeY->SetString(szBuffer);
        snprintf(szBuffer, 256, "Aspect:%.2f", psNib->sSize.fAspectRatio);
        m_pcWidgetAspectRatio->SetString(szBuffer);
        
        
        snprintf(szBuffer, 256, "Position:%s", psNib->sPos.bFixedSize ? "Fixed" : "Ratio");
        m_pcPositionType->SetString(szBuffer);
        m_pcPosXAlign->SetString(CNibManager::GetAlignmentTypeString((EWidgetPositionType)psNib->sPos.abyAlign[0]));
        m_pcPosYAlign->SetString(CNibManager::GetAlignmentTypeString((EWidgetPositionType)psNib->sPos.abyAlign[1]));
        snprintf(szBuffer, 256, "X:%.2f", psNib->sPos.afValues[0]);
        m_pcWidgetPosX->SetString(szBuffer);
        snprintf(szBuffer, 256, "Y:%.2f", psNib->sPos.afValues[1]);
        m_pcWidgetPosY->SetString(szBuffer);
        
        
        snprintf(szBuffer, 256, "AX:%.2f", psNib->sAnchor.x);
        m_pcWidgetAnchorX->SetString(szBuffer);
        snprintf(szBuffer, 256, "AY:%.2f", psNib->sAnchor.y);
        m_pcWidgetAnchorY->SetString(szBuffer);
        
        m_pcWidgetType->SetString(CNibManager::GetWidgetTypeString(psNib->eType));
        
        if (pcSelectedWidget == m_pcBaseWidget) {
            m_pcAttachDetach->ClearString();
        }
        else {
            CUIContainer* pcParent = pcSelectedWidget->GetParent();
            if (m_pcBaseWidget == pcParent) {
                m_pcAttachDetach->SetString("Attach To Container");
            }
            else {
                m_pcAttachDetach->SetString("Detach From Container");
            }
        }
    }
}
VOID CUIEditor::ReleaseEditor(VOID)
{
    CEventManager::UnRegisterForBroadcast(*this, EGLOBAL_MAC_SYSTEM_MGR, EGLOBAL_EVENT_MAC_SYSTEM_KEY_PRESSED);
}

VOID CUIEditor::OnToggleSizeType(VOID)
{
    CUIWidget* pcSelectedWidget = GetSelectedWidget();
    if (NULL == pcSelectedWidget) {
        return;
    }
    SNibEditInfo* psNib = FindNib(pcSelectedWidget);
    if (NULL == psNib) {
        return;
    }
    CUIWidget* pcParent = pcSelectedWidget->GetParent();
    if (NULL == pcParent) {
        return;
    }
    const SHLVector2D& sParentSize = pcParent->GetLocalSize();
    SNibSizeInfo& sSizeInfo = psNib->sSize;
    sSizeInfo.bFixedSize = !sSizeInfo.bFixedSize;
    if (sSizeInfo.bFixedSize) { // from ratio become fixed fize
        sSizeInfo.afValues[0] = sSizeInfo.afValues[0] * sParentSize.x;
        sSizeInfo.afValues[1] = sSizeInfo.afValues[1] * sParentSize.y;
    }
    else { // from fixed size become ratio
        sSizeInfo.afValues[0] = sSizeInfo.afValues[0] / sParentSize.x;
        sSizeInfo.afValues[1] = sSizeInfo.afValues[1] / sParentSize.y;
    }
    m_bEdited = TRUE;
    RefreshLabels();
    CNibManager::UpdateWidgetFromNibWithParentSize(*psNib, *pcSelectedWidget, sParentSize, GetNibMap());
}
VOID CUIEditor::OnTogglePosType(VOID)
{
    CUIWidget* pcSelectedWidget = GetSelectedWidget();
    if (NULL == pcSelectedWidget) {
        return;
    }
    SNibEditInfo* psNib = FindNib(pcSelectedWidget);
    if (NULL == psNib) {
        return;
    }
    CUIWidget* pcParent = pcSelectedWidget->GetParent();
    if (NULL == pcParent) {
        return;
    }
    const SHLVector2D& sParentSize = pcParent->GetLocalSize();
    SNibPosInfo& sPosInfo = psNib->sPos;
    sPosInfo.bFixedSize = !sPosInfo.bFixedSize;
    if (sPosInfo.bFixedSize) { // from ratio become fixed fize
        sPosInfo.afValues[0] = sPosInfo.afValues[0] * sParentSize.x;
        sPosInfo.afValues[1] = sPosInfo.afValues[1] * sParentSize.y;
    }
    else { // from fixed size become ratio
        sPosInfo.afValues[0] = sPosInfo.afValues[0] / sParentSize.x;
        sPosInfo.afValues[1] = sPosInfo.afValues[1] / sParentSize.y;
    }
    m_bEdited = TRUE;
    RefreshLabels();
    CNibManager::UpdateWidgetFromNibWithParentSize(*psNib, *pcSelectedWidget, sParentSize, GetNibMap());
}

VOID CUIEditor::OnTogglePositionAlignment(BOOLEAN bX)
{
    CUIWidget* pcSelectedWidget = GetSelectedWidget();
    if (NULL == pcSelectedWidget) {
        return;
    }
    SNibEditInfo* psNib = FindNib(pcSelectedWidget);
    if (NULL == psNib) {
        return;
    }
    CUIWidget* pcParent = pcSelectedWidget->GetParent();
    if (NULL == pcParent) {
        return;
    }
    const SHLVector2D& sParentSize = pcParent->GetLocalSize();
    SNibPosInfo& sPosInfo = psNib->sPos;
    if (bX) {
        if (EWidgetPosition_Right == sPosInfo.abyAlign[0]) {
            sPosInfo.abyAlign[0] = EWidgetPosition_Left;
        }
        else {
            sPosInfo.abyAlign[0] = EWidgetPosition_Right;
        }
        if (sPosInfo.bFixedSize) {
            sPosInfo.afValues[0] = sParentSize.x - sPosInfo.afValues[0];
        }
        else {
            sPosInfo.afValues[0] = 1.0f - sPosInfo.afValues[0];
        }
    }
    else {
        if (EWidgetPosition_Bottom == sPosInfo.abyAlign[1]) {
            sPosInfo.abyAlign[1] = EWidgetPosition_Top;
        }
        else {
            sPosInfo.abyAlign[1] = EWidgetPosition_Bottom;
        }
        if (sPosInfo.bFixedSize) {
            sPosInfo.afValues[1] = sParentSize.y - sPosInfo.afValues[1];
        }
        else {
            sPosInfo.afValues[1] = 1.0f - sPosInfo.afValues[1];
        }
    }
    m_bEdited = TRUE;
    RefreshLabels();
    CNibManager::UpdateWidgetFromNibWithParentSize(*psNib, *pcSelectedWidget, sParentSize, GetNibMap());
}

VOID CUIEditor::SelectNextWidget(VOID)
{
    CUIWidget* pcSelectedWidget = GetSelectedWidget();
    if (NULL == pcSelectedWidget) {
        ASSERT(FALSE);
        return;
    }
    CUIContainer* pcContainer = CUIContainer::CastToMe(pcSelectedWidget);
    if (NULL != pcContainer) {
        CUIWidget* pcChild = pcContainer->GetChild();
        if (NULL != pcChild) {
            m_cSelectedWidget.clear();
            m_cSelectedWidget.push_back(pcChild);
            RefreshLabels();
            return;
        }
    }
    CUIWidget* pcSibling = pcSelectedWidget->GetNextSibling();
    if (NULL != pcSibling) {
        m_cSelectedWidget.clear();
        m_cSelectedWidget.push_back(pcSibling);
        RefreshLabels();
        return;
    }
    CUIWidget* pcParent = pcSelectedWidget->GetParent();
    if (NULL == pcParent || &m_cBaseContainer == pcParent) {
        m_cSelectedWidget.clear();
        m_cSelectedWidget.push_back(m_pcBaseWidget);
        RefreshLabels();
        return;
    }
    m_cSelectedWidget.clear();
    pcSibling = pcParent->GetNextSibling();
    if (NULL == pcSibling) {
        m_cSelectedWidget.push_back(m_pcBaseWidget);
    }
    else {
        m_cSelectedWidget.push_back(pcSibling);
    }
    RefreshLabels();
}
VOID CUIEditor::SelectPrevWidget(VOID)
{
    CUIWidget* pcSelectedWidget = GetSelectedWidget();
    if (NULL == pcSelectedWidget) {
        ASSERT(FALSE);
        return;
    }
    CUIWidget* pcSibling = pcSelectedWidget->GetPrevSibling();
    if (NULL != pcSibling) {
        m_cSelectedWidget.clear();
        m_cSelectedWidget.push_back(pcSibling);
        RefreshLabels();
        return;
    }
    CUIWidget* pcParent = pcSelectedWidget->GetParent();
    if (NULL == pcParent || pcParent == &m_cBaseContainer) {
        m_cSelectedWidget.clear();
        m_cSelectedWidget.push_back(m_pcBaseWidget);
        RefreshLabels();
        return;
    }
    m_cSelectedWidget.clear();
    m_cSelectedWidget.push_back(pcParent);
    RefreshLabels();
}
VOID CUIEditor::OnKeyPressed(UINT uKeyCode)
{
    if (EKEY_TAB == uKeyCode) {
        CUIWidget* pcSelectedWidget = GetSelectedWidget();
        if (NULL == m_pcBaseWidget) {
            return;
        }
        if (NULL == pcSelectedWidget) {
            m_cSelectedWidget.clear();
            m_cSelectedWidget.push_back(m_pcBaseWidget);
            RefreshLabels();
            return;
        }
        if (CMacSystem::IsShiftKeyPressed()) {
            SelectPrevWidget();
        }
        else {
            SelectNextWidget();
        }
        return;
    }
    if (EKEY_DELETE == uKeyCode) { // deletion pressed
        CUIWidget* pcSelectedWidget = GetSelectedWidget();
        if (m_pcBaseWidget == pcSelectedWidget) { // cannot delete base widget.
            return;
        }
        RecursiveReleaseWidget(*pcSelectedWidget);
        m_cSelectedWidget.clear();
        RefreshLabels();
        m_bEdited = TRUE;
        return;
    }
    FLOAT fDX = 0.0f;
    FLOAT fDY = 0.0f;
    switch (uKeyCode) {
        case EKEY_DIRECTION_UP:
            fDY = -1.0;
            break;
        case EKEY_DIRECTION_DOWN:
            fDY = 1.0;
            break;
        case EKEY_DIRECTION_LEFT:
            fDX = -1.0f;
            break;
        case EKEY_DIRECTION_RIGHT:
            fDX = 1.0f;
            break;
            
        default:
            break;
    }
    if (0.0f == fDX && 0.0f == fDY) {
        return;
    }
    for ( auto& pcWidget : m_cSelectedWidget ) {
        SNibEditInfo* psNib = FindNib(pcWidget);
        if (NULL == psNib) {
            continue;
        }
        FLOAT fMoveX = fDX;
        if (EWidgetPosition_Right == psNib->sPos.abyAlign[0]) {
            fMoveX = -fMoveX;
        }
        FLOAT fMoveY = fDY;
        if (EWidgetPosition_Bottom == psNib->sPos.abyAlign[1]) {
            fMoveY = -fMoveY;
        }
        if (!psNib->sPos.bFixedSize) {
            CUIContainer* pcParent = pcWidget->GetParent();
            if (NULL == pcParent) {
                return;
            }
            const SHLVector2D& sParentSize = pcParent->GetLocalSize();
            fMoveX /= sParentSize.x;
            fMoveY /= sParentSize.y;
        }
        psNib->sPos.afValues[0] += fMoveX;
        psNib->sPos.afValues[1] += fMoveY;
        UpdateWidgetFromNib(*psNib, *pcWidget);
        RefreshLabels();
        m_bEdited = TRUE;
    }
}

static VOID RecursiveToggleInput(CUIWidget* pcWidget, BOOLEAN bEnabled)
{
    if (NULL == pcWidget) {
        return;
    }
    pcWidget->SetEnabled(bEnabled);
    CUIContainer* pcContainer = CUIContainer::CastToMe(pcWidget);
    if (NULL != pcContainer) {
        RecursiveToggleInput(pcContainer->GetChild(), bEnabled);
    }
    RecursiveToggleInput(pcWidget->GetNextSibling(), bEnabled);
}
VOID CUIEditor::ToggleInput(BOOLEAN bEnabled)
{
    m_bInputEnabled = bEnabled;
    if (NULL != m_pcToggleInput) {
        m_pcToggleInput->SetString(m_bInputEnabled ? "Disable Input" : "Enable Input");
    }
    RecursiveToggleInput(m_cBaseContainer.GetChild(), m_bInputEnabled);
}

VOID CUIEditor::MoveSelectedToFront(VOID)
{
    CUIWidget* pcSelectedWidget = GetSelectedWidget();
    if (NULL == pcSelectedWidget) {
        return;
    }
    CUIContainer* pcParent = pcSelectedWidget->GetParent();
    if (NULL == pcParent) {
        return;
    }
    pcSelectedWidget->RemoveFromParent();
    pcParent->AddChild(*pcSelectedWidget);
}


VOID CUIEditor::PerformAttach(CUIWidget& cWidget, SNibEditInfo& sNib, CUIContainer& cParent)
{
    const SHLVector2D& sWorldPos = cWidget.GetWorldPosition();
    const SHLVector2D& sWorldSize = cWidget.GetWorldSize();
    
    const SHLVector2D& sNewParentPos = cParent.GetWorldPosition();
    const SHLVector2D& sNewParentSize = cParent.GetWorldSize();
    // reset alignment to left and top
    sNib.sPos.abyAlign[0] = EWidgetPosition_Left;
    sNib.sPos.abyAlign[1] = EWidgetPosition_Top;
    
    sNib.sPos.afValues[0] = sWorldPos.x - sNewParentPos.x + sNib.sAnchor.x * sWorldSize.x;
    sNib.sPos.afValues[1] = sWorldPos.y - sNewParentPos.y + sNib.sAnchor.y * sWorldSize.y;
    if (!sNib.sPos.bFixedSize) {
        sNib.sPos.afValues[0] /= sNewParentSize.x;
        sNib.sPos.afValues[1] /= sNewParentSize.y;
    }
    
    if (!sNib.sSize.bFixedSize) {
        sNib.sSize.afValues[0] = sWorldSize.x / sNewParentSize.x;
        sNib.sSize.afValues[1] = sWorldSize.y / sNewParentSize.y;
    }
    cWidget.RemoveFromParent();
    cParent.AddChild(cWidget);
    UpdateWidgetFromNib(sNib, cWidget);
}

VOID CUIEditor::ToggleAttachDetach(VOID)
{
    m_bAttachingMode = FALSE;
    CUIWidget* pcSelectedWidget = GetSelectedWidget();
    if (NULL == pcSelectedWidget) {
        return;
    }
    if (m_pcBaseWidget == pcSelectedWidget) {
        return;
    }
    CUIContainer* pcNowParent = pcSelectedWidget->GetParent();
    if (m_pcBaseWidget != pcNowParent) { // do detach
        SNibEditInfo* psNib = FindNib(pcSelectedWidget);
        if (NULL == psNib) {
            return;
        }
        PerformAttach(*pcSelectedWidget, *psNib, *m_pcBaseWidget);
        m_bEdited = TRUE;
        RefreshLabels();
        return;
    }
    m_bAttachingMode = TRUE;
    m_cWorkingDirectory.SetString("Pick a Container to attach to");
}
VOID CUIEditor::DoAttachPick(CUIWidget* pcPicked)
{
    m_bAttachingMode = FALSE; // safety
    CUIWidget* pcSelectedWidget = GetSelectedWidget();
    if (NULL == pcPicked || NULL == pcSelectedWidget) {
        m_cWorkingDirectory.SetString("Cancelled");
        return;
    }
    SNibEditInfo* psNib = FindNib(pcSelectedWidget);
    if (NULL == psNib) {
        m_cWorkingDirectory.SetString("Cancelled");
        return;
    }
    
    CUIContainer* pcNewParent = CUIContainer::CastToMe(pcPicked);
    if (NULL == pcNewParent) {
        m_cWorkingDirectory.SetString("Cancelled");
        return;
    }
    PerformAttach(*pcSelectedWidget, *psNib, *pcNewParent);
    m_bEdited = TRUE;
    RefreshLabels();
    m_cWorkingDirectory.SetString("Done");
}


#endif // #if defined(MAC_PORT)
