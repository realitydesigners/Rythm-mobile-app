#include "stdafx.h"
#if defined(MAC_PORT)
#include "UIImageEditor.h"
#include "Event.h"
#include "CMain.h"
#include "UIEditor.h"
#include "EventManager.h"
#include "TextInputWindow.h"
#include "MacSystem.h"
#include "Texture.h"
#import <Foundation/Foundation.h>

static FLOAT s_afLastPos[2] = { 10.0f, 10.0f };

CUIImageEditor::CUIImageEditor(CUIEditor& cMaster, SNibEditInfo& sNib) :
CGameWindow(),
m_cMaster(cMaster),
m_sNib(sNib),
m_bUITouch(FALSE),
m_bDragMode(FALSE),
m_uLastEventID(0),
m_pcImagePath(NULL),
m_pcLeft(NULL),
m_pcRight(NULL),
m_pcTop(NULL),
m_pcBottom(NULL),
m_pcRotation(NULL),
m_pcType(NULL),
m_pcVertical(NULL)
{
    memset(m_pcRGBA, 0, sizeof(m_pcRGBA));
    m_bDestroyOnExitModal = TRUE;
}

CUIImageEditor::~CUIImageEditor()
{
}

#define EDITOR_WIDTH    (230.0f)
#define EDITOR_HEIGHT   (500.0f)
#define CREATE_DEBUG_BTN(x, y, id, text) CreateColorButton(x, y, 100.0f, 40.0f, id, RGBA(0x7F, 0x7F, 0x7F, 0x7F), text, TRUE)
VOID CUIImageEditor::InitializeInternals(VOID)
{
    CGameWindow::InitializeInternals();
    CUIWidget* pcBG = CreateColorImage(RGBA(0x3F, 0x3F, 0x3F, 0xFF), EDITOR_WIDTH, EDITOR_HEIGHT);
    if (NULL != pcBG) {
        AddChild(*pcBG);
    }
    SetLocalSize(EDITOR_WIDTH, EDITOR_HEIGHT);
    SetLocalPosition(s_afLastPos[0], s_afLastPos[1]);
    CREATE_DEBUG_BTN(EDITOR_WIDTH - 10.0f - 100.0f, 10.0f, EBTN_EXIT, "Quit");
    CUIButton* pcBtn = CreateColorButton(10.0f, 60.0f, 210.0f, 40.0f, EBTN_CHANGE_IMAGE, RGBA(0x7F, 0x7F, 0x7F, 0x7F), "", TRUE);
    if (NULL != pcBtn) {
        m_pcImagePath = CUITextLabel::CastToMe(pcBtn->GetChild());
    }
    pcBtn = CREATE_DEBUG_BTN(10.0f, 110.0f, EBTN_CHANGE_LEFT, "");
    if (NULL != pcBtn) {
        m_pcLeft = CUITextLabel::CastToMe(pcBtn->GetChild());
    }
    pcBtn = CREATE_DEBUG_BTN(120.0f, 110.0f, EBTN_CHANGE_RIGHT, "");
    if (NULL != pcBtn) {
        m_pcRight = CUITextLabel::CastToMe(pcBtn->GetChild());
    }
    pcBtn = CREATE_DEBUG_BTN(10.0f, 160.0f, EBTN_CHANGE_TOP, "");
    if (NULL != pcBtn) {
        m_pcTop = CUITextLabel::CastToMe(pcBtn->GetChild());
    }
    pcBtn = CREATE_DEBUG_BTN(120.0f, 160.0f, EBTN_CHANGE_BOTTOM, "");
    if (NULL != pcBtn) {
        m_pcBottom = CUITextLabel::CastToMe(pcBtn->GetChild());
    }
    pcBtn = CREATE_DEBUG_BTN(10.0f, 210.0f, EBTN_CHANGE_R, "");
    if (NULL != pcBtn) {
        m_pcRGBA[0] = CUITextLabel::CastToMe(pcBtn->GetChild());
    }
    pcBtn = CREATE_DEBUG_BTN(120.0f, 210.0f, EBTN_CHANGE_G, "");
    if (NULL != pcBtn) {
        m_pcRGBA[1] = CUITextLabel::CastToMe(pcBtn->GetChild());
    }
    pcBtn = CREATE_DEBUG_BTN(10.0f, 260.0f, EBTN_CHANGE_B, "");
    if (NULL != pcBtn) {
        m_pcRGBA[2] = CUITextLabel::CastToMe(pcBtn->GetChild());
    }
    pcBtn = CREATE_DEBUG_BTN(120.0f, 260.0f, EBTN_CHANGE_A, "");
    if (NULL != pcBtn) {
        m_pcRGBA[3] = CUITextLabel::CastToMe(pcBtn->GetChild());
    }
    pcBtn = CREATE_DEBUG_BTN(10.0f, 310.0f, EBTN_CHANGE_ROTATION, "");
    if (NULL != pcBtn) {
        m_pcRotation = CUITextLabel::CastToMe(pcBtn->GetChild());
    }
    pcBtn = CREATE_DEBUG_BTN(120.0f, 310.0f, EBTN_CHANGE_TYPE, "");
    if (NULL != pcBtn) {
        m_pcType = CUITextLabel::CastToMe(pcBtn->GetChild());
    }
    pcBtn = CreateColorButton(10.0f, 360.0f, 210.0f, 40.0f, EBTN_CHANGE_VERTICAL, RGBA(0x7F, 0x7F, 0x7F, 0x7F), "", TRUE);
    if (NULL != pcBtn) {
        m_pcVertical = CUITextLabel::CastToMe(pcBtn->GetChild());
    }
    CreateColorButton(10.0f, 410.0f, 210.0f, 40.0f, EBTN_SNAP_SIZE_TO_TEXTURE, RGBA(0x7F, 0x7F, 0x7F, 0x7F), "Snap To Texture Size", TRUE);
    RefreshStats();
}

VOID CUIImageEditor::Release(VOID)
{
    CGameWindow::Release();
}

VOID CUIImageEditor::OnInputReply(VOID)
{
    if (NULL == m_sNib.psImg) {
        return;
    }
    const UINT uVal = atoi(m_szStringBuffer);
    const BYTE byR = GET_RED(m_sNib.psImg->uColor);
    const BYTE byG = GET_GREEN(m_sNib.psImg->uColor);
    const BYTE byB = GET_BLUE(m_sNib.psImg->uColor);
    const BYTE byA = GET_ALPHA(m_sNib.psImg->uColor);
    switch (m_uLastEventID) {
        case EBTN_CHANGE_LEFT:
            m_sNib.psImg->abyOffset[0] = uVal;
            break;
        case EBTN_CHANGE_RIGHT:
            m_sNib.psImg->abyOffset[1] = uVal;
            break;
        case EBTN_CHANGE_TOP:
            m_sNib.psImg->abyOffset[2] = uVal;
            break;
        case EBTN_CHANGE_BOTTOM:
            m_sNib.psImg->abyOffset[3] = uVal;
            break;
        case EBTN_CHANGE_R:
            m_sNib.psImg->uColor = RGBA(uVal, byG, byB, byA);
            break;
        case EBTN_CHANGE_G:
            m_sNib.psImg->uColor = RGBA(byR, uVal, byB, byA);
            break;
        case EBTN_CHANGE_B:
            m_sNib.psImg->uColor = RGBA(byR, byG, uVal, byA);
            break;
        case EBTN_CHANGE_A:
            m_sNib.psImg->uColor = RGBA(byR, byG, byB, uVal);
            break;
        case EBTN_CHANGE_ROTATION:
            m_sNib.psImg->fRotation = atof(m_szStringBuffer);
            break;
        default:
            return;

    }
    CEvent cEvent(CUIEditor::EEVENT_REFRESH_SELECTED_WIDGET);
    CEventManager::PostEvent(m_cMaster, cEvent);
    RefreshStats();

}
VOID CUIImageEditor::RequireTextInput(UINT uEventID)
{
    if (NULL == m_sNib.psImg) {
        return;
    }
    m_uLastEventID = uEventID;
    CEvent cEvent(EEVENT_TEXT_INPUT_REPLY);
    CTextInputWindow* pcWin = new CTextInputWindow(m_szStringBuffer, 255, *this, cEvent);
    if (NULL == pcWin) {
        return;
    }
    switch (uEventID) {
        case EBTN_CHANGE_LEFT:
            snprintf(m_szStringBuffer, 256, "%d", m_sNib.psImg->abyOffset[0]);
            pcWin->SetTitleAndActionText("Change Left", "Change");
            break;
        case EBTN_CHANGE_RIGHT:
            snprintf(m_szStringBuffer, 256, "%d", m_sNib.psImg->abyOffset[1]);
            pcWin->SetTitleAndActionText("Change Right", "Change");
            break;
        case EBTN_CHANGE_TOP:
            snprintf(m_szStringBuffer, 256, "%d", m_sNib.psImg->abyOffset[2]);
            pcWin->SetTitleAndActionText("Change Height", "Change");
            break;
        case EBTN_CHANGE_BOTTOM:
            snprintf(m_szStringBuffer, 256, "%d", m_sNib.psImg->abyOffset[3]);
            pcWin->SetTitleAndActionText("Change Bottom", "Change");
            break;
        case EBTN_CHANGE_R:
            snprintf(m_szStringBuffer, 256, "%d", GET_RED(m_sNib.psImg->uColor));
            pcWin->SetTitleAndActionText("Change Red", "Change");
            break;
        case EBTN_CHANGE_G:
            snprintf(m_szStringBuffer, 256, "%d", GET_GREEN(m_sNib.psImg->uColor));
            pcWin->SetTitleAndActionText("Change Green", "Change");
            break;
        case EBTN_CHANGE_B:
            snprintf(m_szStringBuffer, 256, "%d", GET_BLUE(m_sNib.psImg->uColor));
            pcWin->SetTitleAndActionText("Change Blue", "Change");
            break;
        case EBTN_CHANGE_A:
            snprintf(m_szStringBuffer, 256, "%d", GET_ALPHA(m_sNib.psImg->uColor));
            pcWin->SetTitleAndActionText("Change Alpha", "Change");
            break;
        case EBTN_CHANGE_ROTATION:
            snprintf(m_szStringBuffer, 256, "%.2f", m_sNib.psImg->fRotation);
            pcWin->SetTitleAndActionText("Change Rotation", "Change");
            break;
            
        default:
            ASSERT(FALSE);
            return;
    }
    pcWin->DoModal();
}

VOID CUIImageEditor::OnFileOpened(BOOLEAN bSucess)
{
    if (!bSucess) {
        return;
    }
    const CHAR* szImgPath = CMacSystem::GetUserSelectedPath();
    NSString* cImgPath = [NSString stringWithUTF8String:szImgPath];
    NSArray* cPathComponents = [cImgPath pathComponents];
    NSString* cWorkingPath = [NSString stringWithUTF8String:m_cMaster.GetWorkingPath()];
    NSArray* cWorkingPathComponents = [cWorkingPath pathComponents];
    
    NSInteger componentsInCommon = MIN([cPathComponents count], [cWorkingPathComponents count]);
    for (NSInteger i = 0, n = componentsInCommon; i < n; i++) {
        if (![[cPathComponents objectAtIndex:i] isEqualToString:[cWorkingPathComponents objectAtIndex:i]]) {
            componentsInCommon = i;
            break;
        }
    }
    
    NSUInteger numberOfParentComponents = [cWorkingPathComponents count] - componentsInCommon;
    NSUInteger numberOfPathComponents = [cPathComponents count] - componentsInCommon;
    
    NSMutableArray *relativeComponents = [NSMutableArray arrayWithCapacity:
                                          numberOfParentComponents + numberOfPathComponents];
    for (NSInteger i = 0; i < numberOfParentComponents; i++) {
        [relativeComponents addObject:@".."];
    }
    [relativeComponents addObjectsFromArray: [cPathComponents subarrayWithRange:NSMakeRange(componentsInCommon, numberOfPathComponents)]];
    const CHAR* szFinalString = [[NSString pathWithComponents:relativeComponents] UTF8String];
    
    TRACE("New Img Path: %s\n", szFinalString);
    if (NULL == m_sNib.psImg) {
        return;
    }
    m_sNib.psImg->cImgPath = szFinalString;
    CEvent cEvent(CUIEditor::EEVENT_REFRESH_SELECTED_WIDGET);
    CEventManager::PostEvent(m_cMaster, cEvent);
    RefreshStats();
}

VOID CUIImageEditor::OnReceiveEvent(CEvent& cEvent)
{
    const UINT uEventID = cEvent.GetIntParam(0);
    switch (uEventID) {
        case EEVENT_TEXT_INPUT_REPLY:
            OnInputReply();
            break;
        case EBTN_CHANGE_VERTICAL:
            if (NULL != m_sNib.psImg) {
                m_sNib.psImg->bVertical = !m_sNib.psImg->bVertical;
                CEvent cEvent(CUIEditor::EEVENT_REFRESH_SELECTED_WIDGET);
                CEventManager::PostEvent(m_cMaster, cEvent);
                RefreshStats();
            }
            break;
        case EBTN_CHANGE_LEFT:
        case EBTN_CHANGE_RIGHT:
        case EBTN_CHANGE_TOP:
        case EBTN_CHANGE_BOTTOM:
        case EBTN_CHANGE_R:
        case EBTN_CHANGE_G:
        case EBTN_CHANGE_B:
        case EBTN_CHANGE_A:
        case EBTN_CHANGE_ROTATION:
            RequireTextInput(uEventID);
            break;
        case EGLOBAL_EVENT_MAC_SYSTEM_FILE_OPEN_PATH_CHOSEN:
            OnFileOpened(cEvent.GetIntParam(1));
            break;
        case EBTN_CHANGE_IMAGE:
            CEventManager::RegisterForBroadcast(*this, EGLOBAL_MAC_SYSTEM_MGR, EGLOBAL_EVENT_MAC_SYSTEM_FILE_OPEN_PATH_CHOSEN, TRUE);
            CMacSystem::TryOpenFile("png");
            break;
            
        case EBTN_EXIT:
            {
                const SHLVector2D& sWorldPos = GetWorldPosition();
                s_afLastPos[0] = sWorldPos.x;
                s_afLastPos[1] = sWorldPos.y;
                ExitModal();
            }
            break;
        case EBTN_CHANGE_TYPE:
            {
                switch (m_sNib.eType) {
                    case EBaseWidget_Image:         m_sNib.eType = EBaseWidget_3PartImage;
                        break;
                    case EBaseWidget_3PartImage:    m_sNib.eType = EBaseWidget_9PartImage;
                        break;
                    case EBaseWidget_9PartImage:    m_sNib.eType = EBaseWidget_Image;
                        break;
                    default:
                        return;
                }
                CEvent cEvent(CUIEditor::EEVENT_REFRESH_SELECTED_WIDGET);
                CEventManager::PostEvent(m_cMaster, cEvent);
                RefreshStats();
            }
            break;
        case EBTN_SNAP_SIZE_TO_TEXTURE:
            {
                CEvent cEvent(CUIEditor::EEVENT_SNAP_IMAGE_TO_TEXTURE_SIZE);
                CEventManager::PostEvent(m_cMaster, cEvent);
            }
            break;
    }
}

BOOLEAN CUIImageEditor::OnTouchBegin(FLOAT fX, FLOAT fY)
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
BOOLEAN CUIImageEditor::OnTouchMove(FLOAT fX, FLOAT fY)
{
    if (m_bUITouch) {
        return CGameWindow::OnTouchMove(fX, fY);
    }
    if (m_bDragMode) {
        SetLocalPosition(fX + m_afPointerOffset[0], fY + m_afPointerOffset[1]);
    }
    return TRUE;
}

BOOLEAN CUIImageEditor::OnTouchEnd(FLOAT fX, FLOAT fY)
{
    if (m_bUITouch) {
        m_bUITouch = FALSE;
        return CGameWindow::OnTouchEnd(fX, fY);
    }
    return TRUE;
}

VOID CUIImageEditor::RefreshStats(VOID)
{
    if (NULL == m_sNib.psImg) {
        return;
    }
    CHAR szBuffer[1024];
    if (NULL != m_pcImagePath) {
        snprintf(szBuffer, 1024, "Path: %s", m_sNib.psImg->cImgPath.c_str());
        m_pcImagePath->SetString(szBuffer);
    }
    if (NULL != m_pcLeft) {
        snprintf(szBuffer, 1024, "Left: %d", m_sNib.psImg->abyOffset[0]);
        m_pcLeft->SetString(szBuffer);
    }
    if (NULL != m_pcRight) {
        snprintf(szBuffer, 1024, "Right: %d", m_sNib.psImg->abyOffset[1]);
        m_pcRight->SetString(szBuffer);
    }
    if (NULL != m_pcTop) {
        snprintf(szBuffer, 1024, "Top: %d", m_sNib.psImg->abyOffset[2]);
        m_pcTop->SetString(szBuffer);
    }
    if (NULL != m_pcBottom) {
        snprintf(szBuffer, 1024, "Bottom: %d", m_sNib.psImg->abyOffset[3]);
        m_pcBottom->SetString(szBuffer);
    }
    if (NULL != m_pcRGBA[0]) {
        const BYTE byVal = GET_RED(m_sNib.psImg->uColor);
        snprintf(szBuffer, 1024, "Red: %d", byVal);
        m_pcRGBA[0]->SetString(szBuffer);
    }
    if (NULL != m_pcRGBA[1]) {
        const BYTE byVal = GET_GREEN(m_sNib.psImg->uColor);
        snprintf(szBuffer, 1024, "Green: %d", byVal);
        m_pcRGBA[1]->SetString(szBuffer);
    }
    if (NULL != m_pcRGBA[2]) {
        const BYTE byVal = GET_BLUE(m_sNib.psImg->uColor);
        snprintf(szBuffer, 1024, "Blue: %d", byVal);
        m_pcRGBA[2]->SetString(szBuffer);
    }
    if (NULL != m_pcRGBA[3]) {
        const BYTE byVal = GET_ALPHA(m_sNib.psImg->uColor);
        snprintf(szBuffer, 1024, "Alpha: %d", byVal);
        m_pcRGBA[3]->SetString(szBuffer);
    }
    if (NULL != m_pcRotation) {
        snprintf(szBuffer, 1024, "Rotate: %.1f", m_sNib.psImg->fRotation);
        m_pcRotation->SetString(szBuffer);
    }
    if (NULL != m_pcVertical) {
        m_pcVertical->SetString(m_sNib.psImg->bVertical ? "Vertical" : "Horizontal");
    }
    if (NULL != m_pcType) {
        switch (m_sNib.eType) {
            case EBaseWidget_Image:
                m_pcType->SetString("Type: Image");
                break;
            case EBaseWidget_3PartImage:
                m_pcType->SetString("Type: 3Part");
                break;
            case EBaseWidget_9PartImage:
                m_pcType->SetString("Type: 9Part");
                break;
            default:
                m_pcType->SetString("Type: unknown");
                break;
        }
    }
}
#endif // #if defined(MAC_PORT)

