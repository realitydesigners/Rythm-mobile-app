#include "stdafx.h"
#if defined(MAC_PORT)
#include "UITextEditor.h"
#include "Event.h"
#include "CMain.h"
#include "UIEditor.h"
#include "EventManager.h"
#include "TextInputWindow.h"
#include "MacSystem.h"
#include "FontManager.h"

static FLOAT s_afLastPos[2] = { 10.0f, 10.0f };

CUITextEditor::CUITextEditor(CUIEditor& cMaster, SNibEditInfo& sNib) :
CGameWindow(),
m_cMaster(cMaster),
m_sNib(sNib),
m_bUITouch(FALSE),
m_bDragMode(FALSE),
m_uLastEventID(0)
{
    m_bDestroyOnExitModal = TRUE;
    memset(m_apcLabels, 0, sizeof(m_apcLabels));
}

CUITextEditor::~CUITextEditor()
{
}

#define EDITOR_WIDTH    (230.0f)
#define EDITOR_HEIGHT   (500.0f)
#define CREATE_DEBUG_BTN(x, y, id, text) CreateColorButton(x, y, 100.0f, 40.0f, id, RGBA(0x7F, 0x7F, 0x7F, 0x7F), text, TRUE)
VOID CUITextEditor::InitializeInternals(VOID)
{
    CGameWindow::InitializeInternals();
    CUIWidget* pcBG = CreateColorImage(RGBA(0x3F, 0x3F, 0x3F, 0xFF), EDITOR_WIDTH, EDITOR_HEIGHT);
    if (NULL != pcBG) {
        AddChild(*pcBG);
    }
    SetLocalSize(EDITOR_WIDTH, EDITOR_HEIGHT);
    SetLocalPosition(s_afLastPos[0], s_afLastPos[1]);
    CREATE_DEBUG_BTN(EDITOR_WIDTH - 10.0f - 100.0f, 10.0f, EBTN_EXIT, "Quit");
    CUIButton* pcBtn = CREATE_DEBUG_BTN(10.0f, 60.0f, EBTN_TOGGLE_FONT_SIZE, "");
    if (NULL != pcBtn) {
        m_apcLabels[EBTN_TOGGLE_FONT_SIZE] = CUITextLabel::CastToMe(pcBtn->GetChild());
    }
    pcBtn = CREATE_DEBUG_BTN(120.0f, 60.0f, EBTN_TOGGLE_ALIGNMENT, "");
    if (NULL != pcBtn) {
        m_apcLabels[EBTN_TOGGLE_ALIGNMENT] = CUITextLabel::CastToMe(pcBtn->GetChild());
    }
    pcBtn = CreateColorButton(10.0f, 110.0f, 210.0f, 40.0f, EBTN_CHANGE_DEFAULT_MSG, RGBA(0x7F, 0x7F, 0x7F, 0x7F), "", TRUE);
    if (NULL != pcBtn) {
        m_apcLabels[EBTN_CHANGE_DEFAULT_MSG] = CUITextLabel::CastToMe(pcBtn->GetChild());
    }
    pcBtn = CREATE_DEBUG_BTN(10.0f, 160.0f, EBTN_CHANGE_R, "");
    if (NULL != pcBtn) {
        m_apcLabels[EBTN_CHANGE_R] = CUITextLabel::CastToMe(pcBtn->GetChild());
    }
    pcBtn = CREATE_DEBUG_BTN(120.0f, 160.0f, EBTN_CHANGE_G, "");
    if (NULL != pcBtn) {
        m_apcLabels[EBTN_CHANGE_G] = CUITextLabel::CastToMe(pcBtn->GetChild());
    }
    pcBtn = CREATE_DEBUG_BTN(10.0f, 210.0f, EBTN_CHANGE_B, "");
    if (NULL != pcBtn) {
        m_apcLabels[EBTN_CHANGE_B] = CUITextLabel::CastToMe(pcBtn->GetChild());
    }
    pcBtn = CREATE_DEBUG_BTN(120.0f, 210.0f, EBTN_CHANGE_A, "");
    if (NULL != pcBtn) {
        m_apcLabels[EBTN_CHANGE_A] = CUITextLabel::CastToMe(pcBtn->GetChild());
    }
    pcBtn = CREATE_DEBUG_BTN(10.0f, 260.0f, EBTN_TOGGLE_EFX, "");
    if (NULL != pcBtn) {
        m_apcLabels[EBTN_TOGGLE_EFX] = CUITextLabel::CastToMe(pcBtn->GetChild());
    }
    pcBtn = CREATE_DEBUG_BTN(10.0f, 310.0f, EBTN_CHANGE_EFX_PARAM1, "");
    if (NULL != pcBtn) {
        m_apcLabels[EBTN_CHANGE_EFX_PARAM1] = CUITextLabel::CastToMe(pcBtn->GetChild());
    }
    pcBtn = CREATE_DEBUG_BTN(120.0f, 310.0f, EBTN_CHANGE_EFX_PARAM2, "");
    if (NULL != pcBtn) {
        m_apcLabels[EBTN_CHANGE_EFX_PARAM2] = CUITextLabel::CastToMe(pcBtn->GetChild());
    }
    pcBtn = CREATE_DEBUG_BTN(10.0f, 360.0f, EBTN_CHANGE_EFX_R, "");
    if (NULL != pcBtn) {
        m_apcLabels[EBTN_CHANGE_EFX_R] = CUITextLabel::CastToMe(pcBtn->GetChild());
    }
    pcBtn = CREATE_DEBUG_BTN(120.0f, 360.0f, EBTN_CHANGE_EFX_G, "");
    if (NULL != pcBtn) {
        m_apcLabels[EBTN_CHANGE_EFX_G] = CUITextLabel::CastToMe(pcBtn->GetChild());
    }
    pcBtn = CREATE_DEBUG_BTN(10.0f, 410.0f, EBTN_CHANGE_EFX_B, "");
    if (NULL != pcBtn) {
        m_apcLabels[EBTN_CHANGE_EFX_B] = CUITextLabel::CastToMe(pcBtn->GetChild());
    }
    pcBtn = CREATE_DEBUG_BTN(120.0f, 410.0f, EBTN_CHANGE_EFX_A, "");
    if (NULL != pcBtn) {
        m_apcLabels[EBTN_CHANGE_EFX_A] = CUITextLabel::CastToMe(pcBtn->GetChild());
    }


    RefreshStats();
}

VOID CUITextEditor::Release(VOID)
{
    CGameWindow::Release();
}

VOID CUITextEditor::OnInputReply(VOID)
{
    if (NULL == m_sNib.psText) {
        return;
    }
    const BYTE byR = GET_RED(m_sNib.psText->uColor);
    const BYTE byG = GET_GREEN(m_sNib.psText->uColor);
    const BYTE byB = GET_BLUE(m_sNib.psText->uColor);
    const BYTE byA = GET_ALPHA(m_sNib.psText->uColor);

    const BYTE byEfxR = GET_RED(m_sNib.psText->uEfxColor);
    const BYTE byEfxG = GET_GREEN(m_sNib.psText->uEfxColor);
    const BYTE byEfxB = GET_BLUE(m_sNib.psText->uEfxColor);
    const BYTE byEfxA = GET_ALPHA(m_sNib.psText->uEfxColor);

    switch (m_uLastEventID) {
        case EBTN_CHANGE_DEFAULT_MSG:
            m_sNib.psText->cDefaultString = m_szStringBuffer;
            break;
        case EBTN_CHANGE_R:
            m_sNib.psText->uColor = RGBA(atoi(m_szStringBuffer), byG, byB, byA);
            break;
        case EBTN_CHANGE_G:
            m_sNib.psText->uColor = RGBA(byR, atoi(m_szStringBuffer), byB, byA);
            break;
        case EBTN_CHANGE_B:
            m_sNib.psText->uColor = RGBA(byR, byG, atoi(m_szStringBuffer), byA);
            break;
        case EBTN_CHANGE_A:
            m_sNib.psText->uColor = RGBA(byR, byG, byB, atoi(m_szStringBuffer));
            break;
        case EBTN_CHANGE_EFX_PARAM1:
            m_sNib.psText->afEfxParams[0] = atof(m_szStringBuffer);
            break;
        case EBTN_CHANGE_EFX_PARAM2:
            m_sNib.psText->afEfxParams[1] = atof(m_szStringBuffer);
            break;
        case EBTN_CHANGE_EFX_R:
            m_sNib.psText->uEfxColor = RGBA(atoi(m_szStringBuffer), byEfxG, byEfxB, byEfxA);
            break;
        case EBTN_CHANGE_EFX_G:
            m_sNib.psText->uEfxColor = RGBA(byEfxR, atoi(m_szStringBuffer), byEfxB, byEfxA);
            break;
        case EBTN_CHANGE_EFX_B:
            m_sNib.psText->uEfxColor = RGBA(byEfxR, byEfxG, atoi(m_szStringBuffer), byEfxA);
            break;
        case EBTN_CHANGE_EFX_A:
            m_sNib.psText->uEfxColor = RGBA(byEfxR, byEfxG, byEfxB, atoi(m_szStringBuffer));
            break;
        default:
            return;

    }
    CEvent cEvent(CUIEditor::EEVENT_REFRESH_SELECTED_WIDGET);
    CEventManager::PostEvent(m_cMaster, cEvent);
    RefreshStats();

}
VOID CUITextEditor::RequireTextInput(UINT uEventID)
{
    if (NULL == m_sNib.psText) {
        return;
    }
    m_uLastEventID = uEventID;
    CEvent cEvent(EEVENT_TEXT_INPUT_REPLY);
    CTextInputWindow* pcWin = new CTextInputWindow(m_szStringBuffer, 255, *this, cEvent);
    if (NULL == pcWin) {
        return;
    }
    switch (uEventID) {
        case EBTN_CHANGE_DEFAULT_MSG:
            snprintf(m_szStringBuffer, 256, "%s", m_sNib.psText->cDefaultString.c_str());
            pcWin->SetTitleAndActionText("Change Text", "Change");
            break;
        case EBTN_CHANGE_R:
            snprintf(m_szStringBuffer, 256, "%d", GET_RED(m_sNib.psText->uColor));
            pcWin->SetTitleAndActionText("Change Red", "Change");
            break;
        case EBTN_CHANGE_G:
            snprintf(m_szStringBuffer, 256, "%d", GET_GREEN(m_sNib.psText->uColor));
            pcWin->SetTitleAndActionText("Change Green", "Change");
            break;
        case EBTN_CHANGE_B:
            snprintf(m_szStringBuffer, 256, "%d", GET_BLUE(m_sNib.psText->uColor));
            pcWin->SetTitleAndActionText("Change Blue", "Change");
            break;
        case EBTN_CHANGE_A:
            snprintf(m_szStringBuffer, 256, "%d", GET_ALPHA(m_sNib.psText->uColor));
            pcWin->SetTitleAndActionText("Change Alpha", "Change");
            break;
        case EBTN_CHANGE_EFX_PARAM1:
            snprintf(m_szStringBuffer, 256, "%.1f", m_sNib.psText->afEfxParams[0]);
            pcWin->SetTitleAndActionText("Change Param", "Change");
            break;
        case EBTN_CHANGE_EFX_PARAM2:
            snprintf(m_szStringBuffer, 256, "%.1f", m_sNib.psText->afEfxParams[1]);
            pcWin->SetTitleAndActionText("Change Param", "Change");
            break;
        case EBTN_CHANGE_EFX_R:
            snprintf(m_szStringBuffer, 256, "%d", GET_RED(m_sNib.psText->uEfxColor));
            pcWin->SetTitleAndActionText("Change Red", "Change");
            break;
        case EBTN_CHANGE_EFX_G:
            snprintf(m_szStringBuffer, 256, "%d", GET_GREEN(m_sNib.psText->uEfxColor));
            pcWin->SetTitleAndActionText("Change Green", "Change");
            break;
        case EBTN_CHANGE_EFX_B:
            snprintf(m_szStringBuffer, 256, "%d", GET_BLUE(m_sNib.psText->uEfxColor));
            pcWin->SetTitleAndActionText("Change Blue", "Change");
            break;
        case EBTN_CHANGE_EFX_A:
            snprintf(m_szStringBuffer, 256, "%d", GET_ALPHA(m_sNib.psText->uEfxColor));
            pcWin->SetTitleAndActionText("Change Alpha", "Change");
            break;

        default:
            ASSERT(FALSE);
            return;
    }
    pcWin->DoModal();
}


VOID CUITextEditor::OnReceiveEvent(CEvent& cEvent)
{
    const UINT uEventID = cEvent.GetIntParam(0);
    switch (uEventID) {
        case EBTN_TOGGLE_FONT_SIZE:
            {
                if (NULL == m_sNib.psText) {
                    return;
                }
                m_sNib.psText->eFontSize = (EGAMEFONT)((m_sNib.psText->eFontSize + 1) % EGAMEFONT_NUM);
                CEvent cEvent(CUIEditor::EEVENT_REFRESH_SELECTED_WIDGET);
                CEventManager::PostEvent(m_cMaster, cEvent);
                RefreshStats();
            }
            break;
        case EBTN_TOGGLE_ALIGNMENT:
            {
                if (NULL == m_sNib.psText) {
                    return;
                }
                if (CUITextLabel::EALIGNMENT_LEFT == m_sNib.psText->uTextAlignment) {
                    m_sNib.psText->uTextAlignment = CUITextLabel::EALIGNMENT_CENTER;
                }
                else if (CUITextLabel::EALIGNMENT_CENTER == m_sNib.psText->uTextAlignment) {
                    m_sNib.psText->uTextAlignment = CUITextLabel::EALIGNMENT_RIGHT;
                }
                else {
                    m_sNib.psText->uTextAlignment = CUITextLabel::EALIGNMENT_LEFT;
                }
                CEvent cEvent(CUIEditor::EEVENT_REFRESH_SELECTED_WIDGET);
                CEventManager::PostEvent(m_cMaster, cEvent);
                RefreshStats();
            }
            break;
        case EBTN_TOGGLE_EFX:
            {
                if (NULL == m_sNib.psText) {
                    return;
                }
                m_sNib.psText->uEfxType = (m_sNib.psText->uEfxType + 1) % CUITextLabel::EFFECT_TYPE_NUM;
                CEvent cEvent(CUIEditor::EEVENT_REFRESH_SELECTED_WIDGET);
                CEventManager::PostEvent(m_cMaster, cEvent);
                RefreshStats();
            }
            break;
            
        case EBTN_CHANGE_DEFAULT_MSG:
        case EBTN_CHANGE_R:
        case EBTN_CHANGE_G:
        case EBTN_CHANGE_B:
        case EBTN_CHANGE_A:
        case EBTN_CHANGE_EFX_PARAM1:
        case EBTN_CHANGE_EFX_PARAM2:
        case EBTN_CHANGE_EFX_R:
        case EBTN_CHANGE_EFX_G:
        case EBTN_CHANGE_EFX_B:
        case EBTN_CHANGE_EFX_A:
            RequireTextInput(uEventID);
            break;
        case EEVENT_TEXT_INPUT_REPLY:
            OnInputReply();
            break;
            
        case EBTN_EXIT:
            {
                const SHLVector2D& sWorldPos = GetWorldPosition();
                s_afLastPos[0] = sWorldPos.x;
                s_afLastPos[1] = sWorldPos.y;
                ExitModal();
            }
            break;
    }
}

BOOLEAN CUITextEditor::OnTouchBegin(FLOAT fX, FLOAT fY)
{
    m_bUITouch = CGameWindow::OnTouchBegin(fX, fY);
    if (m_bUITouch) {
        return TRUE;
    }
    const SHLVector2D sPos = { fX, fY };
    m_bDragMode = HitTest(sPos);
    if (m_bDragMode) {
        const SHLVector2D& sWorldPos = GetWorldPosition();
        m_afPointerOffset[0] = sWorldPos.x - fX;
        m_afPointerOffset[1] = sWorldPos.y - fY;
    }
    return TRUE;
}
BOOLEAN CUITextEditor::OnTouchMove(FLOAT fX, FLOAT fY)
{
    if (m_bUITouch) {
        return CGameWindow::OnTouchMove(fX, fY);
    }
    if (m_bDragMode) {
        SetLocalPosition(fX + m_afPointerOffset[0], fY + m_afPointerOffset[1]);
    }
    return TRUE;
}

BOOLEAN CUITextEditor::OnTouchEnd(FLOAT fX, FLOAT fY)
{
    if (m_bUITouch) {
        m_bUITouch = FALSE;
        return CGameWindow::OnTouchEnd(fX, fY);
    }
    return TRUE;
}

VOID CUITextEditor::RefreshStats(VOID)
{
    if (NULL == m_sNib.psText) {
        return;
    }
    CHAR szBuffer[1024];
    if (NULL != m_apcLabels[EBTN_TOGGLE_FONT_SIZE]) {
        snprintf(szBuffer, 1024, "Font: %.1f", CFontManager::GetFontSize(m_sNib.psText->eFontSize));
        m_apcLabels[EBTN_TOGGLE_FONT_SIZE]->SetString(szBuffer);
    }
    if (NULL != m_apcLabels[EBTN_TOGGLE_ALIGNMENT]) {
        switch (m_sNib.psText->uTextAlignment) {
            case CUITextLabel::EALIGNMENT_LEFT:
                m_apcLabels[EBTN_TOGGLE_ALIGNMENT]->SetString(("Align Left"));
                break;
            case CUITextLabel::EALIGNMENT_RIGHT:
                m_apcLabels[EBTN_TOGGLE_ALIGNMENT]->SetString(("Align Right"));
                break;
            case CUITextLabel::EALIGNMENT_CENTER:
                m_apcLabels[EBTN_TOGGLE_ALIGNMENT]->SetString(("Align Center"));
                break;

            default:
                break;
        }
    }
    if (NULL != m_apcLabels[EBTN_CHANGE_DEFAULT_MSG]) {
        snprintf(szBuffer, 1024, "text: %s\n", m_sNib.psText->cDefaultString.c_str());
        m_apcLabels[EBTN_CHANGE_DEFAULT_MSG]->SetString(szBuffer);
    }
    if (NULL != m_apcLabels[EBTN_CHANGE_R]) {
        snprintf(szBuffer, 1024, "Red: %d\n", GET_RED(m_sNib.psText->uColor));
        m_apcLabels[EBTN_CHANGE_R]->SetString(szBuffer);
    }
    if (NULL != m_apcLabels[EBTN_CHANGE_EFX_R]) {
        snprintf(szBuffer, 1024, "Red: %d\n", GET_RED(m_sNib.psText->uEfxColor));
        m_apcLabels[EBTN_CHANGE_EFX_R]->SetString(szBuffer);
    }
    if (NULL != m_apcLabels[EBTN_CHANGE_G]) {
        snprintf(szBuffer, 1024, "Green: %d\n", GET_GREEN(m_sNib.psText->uColor));
        m_apcLabels[EBTN_CHANGE_G]->SetString(szBuffer);
    }
    if (NULL != m_apcLabels[EBTN_CHANGE_EFX_G]) {
        snprintf(szBuffer, 1024, "Green: %d\n", GET_GREEN(m_sNib.psText->uEfxColor));
        m_apcLabels[EBTN_CHANGE_EFX_G]->SetString(szBuffer);
    }
    if (NULL != m_apcLabels[EBTN_CHANGE_B]) {
        snprintf(szBuffer, 1024, "Blue: %d\n", GET_BLUE(m_sNib.psText->uColor));
        m_apcLabels[EBTN_CHANGE_B]->SetString(szBuffer);
    }
    if (NULL != m_apcLabels[EBTN_CHANGE_EFX_B]) {
        snprintf(szBuffer, 1024, "Blue: %d\n", GET_BLUE(m_sNib.psText->uEfxColor));
        m_apcLabels[EBTN_CHANGE_EFX_B]->SetString(szBuffer);
    }
    if (NULL != m_apcLabels[EBTN_CHANGE_A]) {
        snprintf(szBuffer, 1024, "Alpha: %d\n", GET_ALPHA(m_sNib.psText->uColor));
        m_apcLabels[EBTN_CHANGE_A]->SetString(szBuffer);
    }
    if (NULL != m_apcLabels[EBTN_CHANGE_EFX_A]) {
        snprintf(szBuffer, 1024, "Alpha: %d\n", GET_ALPHA(m_sNib.psText->uEfxColor));
        m_apcLabels[EBTN_CHANGE_EFX_A]->SetString(szBuffer);
    }
    const CHAR* aszEfxType[CUITextLabel::EFFECT_TYPE_NUM] = {
        "no efx",   //        EFFECT_NONE=0,
        "shadow",   //        EFFECT_SHADOW,
        "2 diag",    //        EFFECT_2DIAG_SHADOW,
        "4 side",   //        EFFECT_4SIDE_SHADOW,
        "4 diag",   //        EFFECT_4SIDE_DIAG,
        "8 side",   //        EFFECT_8SIDE,
        "8 side n s",//        EFFECT_8SIDE_N_SHADOW,
    };
    if (NULL != m_apcLabels[EBTN_TOGGLE_EFX]) {
        m_apcLabels[EBTN_TOGGLE_EFX]->SetString(aszEfxType[m_sNib.psText->uEfxType % CUITextLabel::EFFECT_TYPE_NUM]);
    }
    if (NULL != m_apcLabels[EBTN_CHANGE_EFX_PARAM1]) {
        snprintf(szBuffer, 1024, "Param1: %.1f\n", m_sNib.psText->afEfxParams[0]);
        m_apcLabels[EBTN_CHANGE_EFX_PARAM1]->SetString(szBuffer);
    }
    if (NULL != m_apcLabels[EBTN_CHANGE_EFX_PARAM2]) {
        snprintf(szBuffer, 1024, "Param2: %.1f\n", m_sNib.psText->afEfxParams[1]);
        m_apcLabels[EBTN_CHANGE_EFX_PARAM2]->SetString(szBuffer);
    }
}
#endif // #if defined(MAC_PORT)

