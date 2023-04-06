#include "stdafx.h"
#if defined(MAC_PORT)
#include "UIButtonEditor.h"
#include "Event.h"
#include "CMain.h"
#include "UIEditor.h"
#include "EventManager.h"
#include "TextInputWindow.h"
#include "Texture.h"
#include "MacSystem.h"
#include "NibManager.h"
#import <Foundation/Foundation.h>

static FLOAT s_afLastPos[2] = { 10.0f, 10.0f };

CUIButtonEditor::CUIButtonEditor(CUIEditor& cMaster, SNibEditInfo& sNib) :
CGameWindow(),
m_cMaster(cMaster),
m_sNib(sNib),
m_bUITouch(FALSE),
m_bDragMode(FALSE),
m_uLastEventID(0),
m_uLastColor(0),
m_pcOffset(NULL),
m_pcToggleBtnMode(NULL)
{
    memset(m_pcWidgetName, 0, sizeof(m_pcWidgetName));
    memset(m_pcWidgetFixedSize, 0, sizeof(m_pcWidgetFixedSize));
    memset(m_pcWidgetSizeX, 0, sizeof(m_pcWidgetSizeX));
    memset(m_pcWidgetSizeY, 0, sizeof(m_pcWidgetSizeY));
    memset(m_pcWidgetAspectRatio, 0, sizeof(m_pcWidgetAspectRatio));
    memset(m_pcPositionType, 0, sizeof(m_pcPositionType));
    memset(m_pcPosXAlign, 0, sizeof(m_pcPosXAlign));
    memset(m_pcPosYAlign, 0, sizeof(m_pcPosYAlign));
    memset(m_pcWidgetPosX, 0, sizeof(m_pcWidgetPosX));
    memset(m_pcWidgetPosY, 0, sizeof(m_pcWidgetPosY));
    memset(m_pcWidgetAnchorX, 0, sizeof(m_pcWidgetAnchorX));
    memset(m_pcWidgetAnchorY, 0, sizeof(m_pcWidgetAnchorY));
    memset(m_pcImagePath, 0, sizeof(m_pcImagePath));
    memset(m_pcLeft, 0, sizeof(m_pcLeft));
    memset(m_pcRight, 0, sizeof(m_pcRight));
    memset(m_pcTop, 0, sizeof(m_pcTop));
    memset(m_pcBottom, 0, sizeof(m_pcBottom));
    memset(m_pcRGBA, 0, sizeof(m_pcRGBA));
    memset(m_pcRotation, 0, sizeof(m_pcRotation));
    memset(m_pcType, 0, sizeof(m_pcType));
    m_bDestroyOnExitModal = TRUE;
}

CUIButtonEditor::~CUIButtonEditor()
{
}

#define EDITOR_WIDTH    (470.0f)
#define EDITOR_HEIGHT   (800.0f)
#define CREATE_DEBUG_BTN(x, y, id, text) CreateColorButton(x, y, 100.0f, 40.0f, id, RGBA(0x7F, 0x7F, 0x7F, 0x7F), text, TRUE)
VOID CUIButtonEditor::InitializeInternals(VOID)
{
    CGameWindow::InitializeInternals();
    CUIWidget* pcBG = CreateColorImage(RGBA(0x3F, 0x3F, 0x3F, 0xFF), EDITOR_WIDTH, EDITOR_HEIGHT);
    if (NULL != pcBG) {
        AddChild(*pcBG);
    }
    SetLocalSize(EDITOR_WIDTH, EDITOR_HEIGHT);
    SetLocalPosition(s_afLastPos[0], s_afLastPos[1]);
    CREATE_DEBUG_BTN(EDITOR_WIDTH - 10.0f - 100.0f, 10.0f, EBTN_EXIT, "Quit");
    
    CUIButton* pcBtn = CREATE_DEBUG_BTN(10.0f, 10.0f, EBTN_TOGGLE_OFFSET, "");
    if (NULL != pcBtn) {
        m_pcOffset = CUITextLabel::CastToMe(pcBtn->GetChild());
    }
    pcBtn = CREATE_DEBUG_BTN(120.0f, 10.0f, EBTN_TOGGLE_MODE_TOGGLE, "");
    if (NULL != pcBtn) {
        m_pcToggleBtnMode = CUITextLabel::CastToMe(pcBtn->GetChild());
    }

    UINT uIndex = 0;
    for ( ; 2 > uIndex; ++uIndex ) {
        const FLOAT fX = 10.0f + uIndex * 240.0f;
        FLOAT fY = 60.0f;
        pcBtn = CreateColorButton(fX, fY, 210.0f, 40.0f, EBTN_CHANGE_NAME1 + uIndex, RGBA(0x7F, 0x7F, 0x7F, 0x7F), "", TRUE);
        if (NULL != pcBtn) {
            m_pcWidgetName[uIndex] = CUITextLabel::CastToMe(pcBtn->GetChild());
        }
        fY += 50.0f;
        pcBtn = CREATE_DEBUG_BTN(fX, fY, EBTN_TOGGLE_FIXED_SIZE1 + uIndex, "");
        if (NULL != pcBtn) {
            m_pcWidgetFixedSize[uIndex] = CUITextLabel::CastToMe(pcBtn->GetChild());
        }
        pcBtn = CREATE_DEBUG_BTN(fX + 110.0f, fY, EBTN_CHANGE_ASPECT1 + uIndex, "");
        if (NULL != pcBtn) {
            m_pcWidgetAspectRatio[uIndex] = CUITextLabel::CastToMe(pcBtn->GetChild());
        }
        fY += 50.0f;
        pcBtn = CREATE_DEBUG_BTN(fX, fY, EBTN_CHANGE_WIDTH1 + uIndex, "");
        if (NULL != pcBtn) {
            m_pcWidgetSizeX[uIndex] = CUITextLabel::CastToMe(pcBtn->GetChild());
        }
        pcBtn = CREATE_DEBUG_BTN(fX + 110.0f, fY, EBTN_CHANGE_HEIGHT1 + uIndex, "");
        if (NULL != pcBtn) {
            m_pcWidgetSizeY[uIndex] = CUITextLabel::CastToMe(pcBtn->GetChild());
        }
        fY += 60.0f;
        pcBtn = CREATE_DEBUG_BTN(fX, fY, EBTN_TOGGLE_POSITION_FIXED_SIZE1 + uIndex, "");
        if (NULL != pcBtn) {
            m_pcPositionType[uIndex] = CUITextLabel::CastToMe(pcBtn->GetChild());
        }
        fY += 50.0f;
        pcBtn = CREATE_DEBUG_BTN(fX, fY, EBTN_TOGGLE_POSITION_X_ALIGNMENT1 + uIndex, "");
        if (NULL != pcBtn) {
            m_pcPosXAlign[uIndex] = CUITextLabel::CastToMe(pcBtn->GetChild());
        }
        pcBtn = CREATE_DEBUG_BTN(fX + 110.0f, fY ,EBTN_TOGGLE_POSITION_Y_ALIGNMENT1 + uIndex, "");
        if (NULL != pcBtn) {
            m_pcPosYAlign[uIndex] = CUITextLabel::CastToMe(pcBtn->GetChild());
        }
        fY += 50.0f;
        pcBtn = CREATE_DEBUG_BTN(fX, fY, EBTN_CHANGE_X1 + uIndex, "");
        if (NULL != pcBtn) {
            m_pcWidgetPosX[uIndex] = CUITextLabel::CastToMe(pcBtn->GetChild());
        }
        pcBtn = CREATE_DEBUG_BTN(fX + 110.0f, fY, EBTN_CHANGE_Y1 + uIndex, "");
        if (NULL != pcBtn) {
            m_pcWidgetPosY[uIndex] = CUITextLabel::CastToMe(pcBtn->GetChild());
        }
        fY += 60.0f;
        pcBtn = CREATE_DEBUG_BTN(fX, fY, EBTN_CHANGE_AX1 + uIndex, "");
        if (NULL != pcBtn) {
            m_pcWidgetAnchorX[uIndex] = CUITextLabel::CastToMe(pcBtn->GetChild());
        }
        pcBtn = CREATE_DEBUG_BTN(fX + 110.0f, fY, EBTN_CHANGE_AY1 + uIndex, "");
        if (NULL != pcBtn) {
            m_pcWidgetAnchorY[uIndex] = CUITextLabel::CastToMe(pcBtn->GetChild());
        }
        fY += 60.0f;
        pcBtn = CreateColorButton(fX, fY, 210.0f, 40.0f, EBTN_CHANGE_IMAGE1 + uIndex, RGBA(0x7F, 0x7F, 0x7F, 0x7F), "", TRUE);
        if (NULL != pcBtn) {
            m_pcImagePath[uIndex] = CUITextLabel::CastToMe(pcBtn->GetChild());
        }
        fY += 50.0f;
        pcBtn = CREATE_DEBUG_BTN(fX, fY, EBTN_CHANGE_LEFT1 + uIndex, "");
        if (NULL != pcBtn) {
            m_pcLeft[uIndex] = CUITextLabel::CastToMe(pcBtn->GetChild());
        }
        pcBtn = CREATE_DEBUG_BTN(fX + 110.0f, fY, EBTN_CHANGE_RIGHT1 + uIndex, "");
        if (NULL != pcBtn) {
            m_pcRight[uIndex] = CUITextLabel::CastToMe(pcBtn->GetChild());
        }
        fY += 50.0f;
        pcBtn = CREATE_DEBUG_BTN(fX, fY, EBTN_CHANGE_TOP1 + uIndex, "");
        if (NULL != pcBtn) {
            m_pcTop[uIndex] = CUITextLabel::CastToMe(pcBtn->GetChild());
        }
        pcBtn = CREATE_DEBUG_BTN(fX + 110.0f, fY, EBTN_CHANGE_BOTTOM1 + uIndex, "");
        if (NULL != pcBtn) {
            m_pcBottom[uIndex] = CUITextLabel::CastToMe(pcBtn->GetChild());
        }
        fY += 50.0f;
        pcBtn = CREATE_DEBUG_BTN(fX, fY, EBTN_CHANGE_R1 + uIndex, "");
        if (NULL != pcBtn) {
            m_pcRGBA[uIndex][0] = CUITextLabel::CastToMe(pcBtn->GetChild());
        }
        pcBtn = CREATE_DEBUG_BTN(fX + 110.0f, fY, EBTN_CHANGE_G1 + uIndex, "");
        if (NULL != pcBtn) {
            m_pcRGBA[uIndex][1] = CUITextLabel::CastToMe(pcBtn->GetChild());
        }
        fY += 50.0f;
        pcBtn = CREATE_DEBUG_BTN(fX, fY, EBTN_CHANGE_B1 + uIndex, "");
        if (NULL != pcBtn) {
            m_pcRGBA[uIndex][2] = CUITextLabel::CastToMe(pcBtn->GetChild());
        }
        pcBtn = CREATE_DEBUG_BTN(fX + 110.0f, fY, EBTN_CHANGE_A1 + uIndex, "");
        if (NULL != pcBtn) {
            m_pcRGBA[uIndex][3] = CUITextLabel::CastToMe(pcBtn->GetChild());
        }
        fY += 50.0f;
        pcBtn = CREATE_DEBUG_BTN(fX, fY, EBTN_CHANGE_ROTATION1 + uIndex, "");
        if (NULL != pcBtn) {
            m_pcRotation[uIndex] = CUITextLabel::CastToMe(pcBtn->GetChild());
        }
        pcBtn = CREATE_DEBUG_BTN(fX + 110.0f, fY, EBTN_CHANGE_TYPE1 + uIndex, "");
        if (NULL != pcBtn) {
            m_pcType[uIndex] = CUITextLabel::CastToMe(pcBtn->GetChild());
        }
        fY += 50.0f;
        CreateColorButton(fX, fY, 210.0f, 40.0f, EBTN_SNAP_SIZE_TO_TEXTURE1 + uIndex, RGBA(0x7F, 0x7F, 0x7F, 0x7F), "Snap To Texture Size", TRUE);
    }
    RefreshStats();
}

VOID CUIButtonEditor::Release(VOID)
{
    CGameWindow::Release();
}

VOID CUIButtonEditor::OnInputReply(VOID)
{
    if (NULL == m_sNib.psBtn) {
        return;
    }
    SNibEditImageInfo* apsImg[2] = {
        m_sNib.psBtn->sIdleInfo.psImg,
        m_sNib.psBtn->sPressedInfo.psImg
    };
    SNibEditInfo* apsNib[2] = {
        &(m_sNib.psBtn->sIdleInfo),
        &(m_sNib.psBtn->sPressedInfo)
    };

    const UINT uVal = atoi(m_szStringBuffer);
    const BYTE byR = GET_RED(m_uLastColor);
    const BYTE byG = GET_GREEN(m_uLastColor);
    const BYTE byB = GET_BLUE(m_uLastColor);
    const BYTE byA = GET_ALPHA(m_uLastColor);
    switch (m_uLastEventID) {
        case EBTN_TOGGLE_OFFSET:
            m_sNib.psBtn->fPressedOffset = atof(m_szStringBuffer);
            break;
        case EBTN_CHANGE_NAME1:
        case EBTN_CHANGE_NAME2:
            apsNib[m_uLastEventID-EBTN_CHANGE_NAME1]->cName = m_szStringBuffer;
            break;
        case EBTN_CHANGE_WIDTH1:
        case EBTN_CHANGE_WIDTH2:
            apsNib[m_uLastEventID-EBTN_CHANGE_WIDTH1]->sSize.afValues[0] = atof(m_szStringBuffer);
            break;
        case EBTN_CHANGE_HEIGHT1:
        case EBTN_CHANGE_HEIGHT2:
            apsNib[m_uLastEventID-EBTN_CHANGE_HEIGHT1]->sSize.afValues[1] = atof(m_szStringBuffer);
            break;
        case EBTN_CHANGE_ASPECT1:
        case EBTN_CHANGE_ASPECT2:
            apsNib[m_uLastEventID-EBTN_CHANGE_ASPECT1]->sSize.fAspectRatio = atof(m_szStringBuffer);
            break;
        case EBTN_CHANGE_X1:
        case EBTN_CHANGE_X2:
            apsNib[m_uLastEventID-EBTN_CHANGE_X1]->sPos.afValues[0] = atof(m_szStringBuffer);
            break;
        case EBTN_CHANGE_Y1:
        case EBTN_CHANGE_Y2:
            apsNib[m_uLastEventID-EBTN_CHANGE_Y1]->sPos.afValues[1] = atof(m_szStringBuffer);
            break;
        case EBTN_CHANGE_AX1:
        case EBTN_CHANGE_AX2:
            apsNib[m_uLastEventID-EBTN_CHANGE_AX1]->sAnchor.x = atof(m_szStringBuffer);
            break;
        case EBTN_CHANGE_AY1:
        case EBTN_CHANGE_AY2:
            apsNib[m_uLastEventID-EBTN_CHANGE_AY1]->sAnchor.y = atof(m_szStringBuffer);
            break;
        case EBTN_CHANGE_LEFT1:
        case EBTN_CHANGE_LEFT2:
            apsImg[m_uLastEventID-EBTN_CHANGE_LEFT1]->abyOffset[0] = uVal;
            break;
        case EBTN_CHANGE_RIGHT1:
        case EBTN_CHANGE_RIGHT2:
            apsImg[m_uLastEventID-EBTN_CHANGE_RIGHT1]->abyOffset[1] = uVal;
            break;
        case EBTN_CHANGE_TOP1:
        case EBTN_CHANGE_TOP2:
            apsImg[m_uLastEventID-EBTN_CHANGE_TOP1]->abyOffset[2] = uVal;
            break;
        case EBTN_CHANGE_BOTTOM1:
        case EBTN_CHANGE_BOTTOM2:
            apsImg[m_uLastEventID-EBTN_CHANGE_BOTTOM1]->abyOffset[3] = uVal;
            break;
        case EBTN_CHANGE_R1:
        case EBTN_CHANGE_R2:
            apsImg[m_uLastEventID-EBTN_CHANGE_R1]->uColor = RGBA(uVal, byG, byB, byA);
            break;
        case EBTN_CHANGE_G1:
        case EBTN_CHANGE_G2:
            apsImg[m_uLastEventID-EBTN_CHANGE_G1]->uColor = RGBA(byR, uVal, byB, byA);
            break;
        case EBTN_CHANGE_B1:
        case EBTN_CHANGE_B2:
            apsImg[m_uLastEventID-EBTN_CHANGE_B1]->uColor = RGBA(byR, byG, uVal, byA);
            break;
        case EBTN_CHANGE_A1:
        case EBTN_CHANGE_A2:
            apsImg[m_uLastEventID-EBTN_CHANGE_A1]->uColor = RGBA(byR, byG, byB, uVal);
            break;
        case EBTN_CHANGE_ROTATION1:
        case EBTN_CHANGE_ROTATION2:
            apsImg[m_uLastEventID-EBTN_CHANGE_ROTATION1]->fRotation = atof(m_szStringBuffer);
            break;
        default:
            return;

    }
    CEvent cEvent(CUIEditor::EEVENT_REFRESH_SELECTED_WIDGET);
    CEventManager::PostEvent(m_cMaster, cEvent);
    RefreshStats();
}

VOID CUIButtonEditor::RequireTextInput(UINT uEventID)
{
    if (NULL == m_sNib.psBtn) {
        return;
    }
    m_uLastEventID = uEventID;
    CEvent cEvent(EEVENT_TEXT_INPUT_REPLY);
    CTextInputWindow* pcWin = new CTextInputWindow(m_szStringBuffer, 255, *this, cEvent);
    if (NULL == pcWin) {
        return;
    }
    const SNibEditImageInfo* apsImg[2] = {
        m_sNib.psBtn->sIdleInfo.psImg,
        m_sNib.psBtn->sPressedInfo.psImg
    };
    const SNibEditInfo* apsNib[2] = {
        &(m_sNib.psBtn->sIdleInfo),
        &(m_sNib.psBtn->sPressedInfo)
    };
    switch (uEventID) {
        case EBTN_TOGGLE_OFFSET:
            snprintf(m_szStringBuffer, 256, "%.1f", m_sNib.psBtn->fPressedOffset);
            pcWin->SetTitleAndActionText("Change Offset", "Change");
            break;
        case EBTN_CHANGE_NAME1:
        case EBTN_CHANGE_NAME2:
            strncpy(m_szStringBuffer, apsNib[uEventID-EBTN_CHANGE_NAME1]->cName.c_str(), 256);
            pcWin->SetTitleAndActionText("Change Name", "Change");
            break;
        case EBTN_CHANGE_WIDTH1:
        case EBTN_CHANGE_WIDTH2:
            snprintf(m_szStringBuffer, 256, "%.3f", apsNib[uEventID-EBTN_CHANGE_WIDTH1]->sSize.afValues[0]);
            pcWin->SetTitleAndActionText("Change Width", "Change");
            break;
        case EBTN_CHANGE_HEIGHT1:
        case EBTN_CHANGE_HEIGHT2:
            snprintf(m_szStringBuffer, 256, "%.3f", apsNib[uEventID-EBTN_CHANGE_HEIGHT1]->sSize.afValues[1]);
            pcWin->SetTitleAndActionText("Change Height", "Change");
            break;
        case EBTN_CHANGE_ASPECT1:
        case EBTN_CHANGE_ASPECT2:
            snprintf(m_szStringBuffer, 256, "%.3f", apsNib[uEventID-EBTN_CHANGE_ASPECT1]->sSize.fAspectRatio);
            pcWin->SetTitleAndActionText("Change Aspect Ratio", "Change");
            break;
        case EBTN_CHANGE_X1:
        case EBTN_CHANGE_X2:
            snprintf(m_szStringBuffer, 256, "%.3f", apsNib[uEventID-EBTN_CHANGE_X1]->sPos.afValues[0]);
            pcWin->SetTitleAndActionText("Change X", "Change");
            break;
        case EBTN_CHANGE_Y1:
        case EBTN_CHANGE_Y2:
            snprintf(m_szStringBuffer, 256, "%.3f", apsNib[uEventID-EBTN_CHANGE_Y1]->sPos.afValues[1]);
            pcWin->SetTitleAndActionText("Change Y", "Change");
            break;
        case EBTN_CHANGE_AX1:
        case EBTN_CHANGE_AX2:
            snprintf(m_szStringBuffer, 256, "%.3f", apsNib[uEventID-EBTN_CHANGE_AX1]->sAnchor.x);
            pcWin->SetTitleAndActionText("Change Anchor X", "Change");
            break;
        case EBTN_CHANGE_AY1:
        case EBTN_CHANGE_AY2:
            snprintf(m_szStringBuffer, 256, "%.3f", apsNib[uEventID-EBTN_CHANGE_AY1]->sAnchor.y);
            pcWin->SetTitleAndActionText("Change Anchor Y", "Change");
            break;
        case EBTN_CHANGE_LEFT1:
        case EBTN_CHANGE_LEFT2:
            snprintf(m_szStringBuffer, 256, "%d", apsImg[uEventID-EBTN_CHANGE_LEFT1]->abyOffset[0]);
            pcWin->SetTitleAndActionText("Change Left", "Change");
            break;
        case EBTN_CHANGE_RIGHT1:
        case EBTN_CHANGE_RIGHT2:
            snprintf(m_szStringBuffer, 256, "%d", apsImg[uEventID-EBTN_CHANGE_RIGHT1]->abyOffset[1]);
            pcWin->SetTitleAndActionText("Change Right", "Change");
            break;
        case EBTN_CHANGE_TOP1:
        case EBTN_CHANGE_TOP2:
            snprintf(m_szStringBuffer, 256, "%d", apsImg[uEventID-EBTN_CHANGE_TOP1]->abyOffset[2]);
            pcWin->SetTitleAndActionText("Change Height", "Change");
            break;
        case EBTN_CHANGE_BOTTOM1:
        case EBTN_CHANGE_BOTTOM2:
            snprintf(m_szStringBuffer, 256, "%d", apsImg[uEventID-EBTN_CHANGE_BOTTOM1]->abyOffset[3]);
            pcWin->SetTitleAndActionText("Change Bottom", "Change");
            break;
        case EBTN_CHANGE_R1:
        case EBTN_CHANGE_R2:
            m_uLastColor = apsImg[uEventID-EBTN_CHANGE_R1]->uColor;
            snprintf(m_szStringBuffer, 256, "%d", GET_RED(m_uLastColor));
            pcWin->SetTitleAndActionText("Change Red", "Change");
            break;
        case EBTN_CHANGE_G1:
        case EBTN_CHANGE_G2:
            m_uLastColor = apsImg[uEventID-EBTN_CHANGE_G1]->uColor;
            snprintf(m_szStringBuffer, 256, "%d", GET_GREEN(m_uLastColor));
            pcWin->SetTitleAndActionText("Change Green", "Change");
            break;
        case EBTN_CHANGE_B1:
        case EBTN_CHANGE_B2:
            m_uLastColor = apsImg[uEventID-EBTN_CHANGE_B1]->uColor;
            snprintf(m_szStringBuffer, 256, "%d", GET_BLUE(m_uLastColor));
            pcWin->SetTitleAndActionText("Change Blue", "Change");
            break;
        case EBTN_CHANGE_A1:
        case EBTN_CHANGE_A2:
            m_uLastColor = apsImg[uEventID-EBTN_CHANGE_A1]->uColor;
            snprintf(m_szStringBuffer, 256, "%d", GET_ALPHA(m_uLastColor));
            pcWin->SetTitleAndActionText("Change Alpha", "Change");
            break;
        case EBTN_CHANGE_ROTATION1:
        case EBTN_CHANGE_ROTATION2:
            snprintf(m_szStringBuffer, 256, "%.2f", apsImg[uEventID-EBTN_CHANGE_ROTATION1]->fRotation);
            pcWin->SetTitleAndActionText("Change Rotation", "Change");
            break;

        default:
            ASSERT(FALSE);
            return;
    }
    pcWin->DoModal();
}

VOID CUIButtonEditor::OnFileOpened(BOOLEAN bSucess)
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
    if (NULL == m_sNib.psBtn) {
        return;
    }
    SNibEditInfo& sImgNib = (EBTN_CHANGE_IMAGE1 == m_uLastEventID) ? m_sNib.psBtn->sIdleInfo : m_sNib.psBtn->sPressedInfo;
    sImgNib.psImg->cImgPath = szFinalString;
    CEvent cEvent(CUIEditor::EEVENT_REFRESH_SELECTED_WIDGET);
    CEventManager::PostEvent(m_cMaster, cEvent);
    RefreshStats();
}

VOID CUIButtonEditor::OnReceiveEvent(CEvent& cEvent)
{
    const UINT uEventID = cEvent.GetIntParam(0);
    switch (uEventID) {
        case EEVENT_TEXT_INPUT_REPLY:
            OnInputReply();
            break;
        case EBTN_TOGGLE_FIXED_SIZE1:
        case EBTN_TOGGLE_FIXED_SIZE2:
            OnToggleSizeType(EBTN_TOGGLE_FIXED_SIZE1 == uEventID);
            break;
        case EBTN_TOGGLE_POSITION_FIXED_SIZE1:
        case EBTN_TOGGLE_POSITION_FIXED_SIZE2:
            OnTogglePosType(EBTN_TOGGLE_POSITION_FIXED_SIZE1 == uEventID);
            break;
        case EBTN_TOGGLE_POSITION_X_ALIGNMENT1:
        case EBTN_TOGGLE_POSITION_X_ALIGNMENT2:
            OnTogglePositionAlignment(EBTN_TOGGLE_POSITION_X_ALIGNMENT2 == uEventID, TRUE);
            break;
        case EBTN_TOGGLE_POSITION_Y_ALIGNMENT1:
        case EBTN_TOGGLE_POSITION_Y_ALIGNMENT2:
            OnTogglePositionAlignment(EBTN_TOGGLE_POSITION_Y_ALIGNMENT1 == uEventID, FALSE);
            break;
        case EBTN_CHANGE_NAME1:
        case EBTN_CHANGE_NAME2:
        case EBTN_CHANGE_WIDTH1:
        case EBTN_CHANGE_WIDTH2:
        case EBTN_CHANGE_HEIGHT1:
        case EBTN_CHANGE_HEIGHT2:
        case EBTN_CHANGE_ASPECT1:
        case EBTN_CHANGE_ASPECT2:
        case EBTN_CHANGE_X1:
        case EBTN_CHANGE_X2:
        case EBTN_CHANGE_Y1:
        case EBTN_CHANGE_Y2:
        case EBTN_CHANGE_AX1:
        case EBTN_CHANGE_AX2:
        case EBTN_CHANGE_AY1:
        case EBTN_CHANGE_AY2:

        case EBTN_CHANGE_LEFT1:
        case EBTN_CHANGE_LEFT2:
        case EBTN_CHANGE_RIGHT1:
        case EBTN_CHANGE_RIGHT2:
        case EBTN_CHANGE_TOP1:
        case EBTN_CHANGE_TOP2:
        case EBTN_CHANGE_BOTTOM1:
        case EBTN_CHANGE_BOTTOM2:
        case EBTN_CHANGE_R1:
        case EBTN_CHANGE_R2:
        case EBTN_CHANGE_G1:
        case EBTN_CHANGE_G2:
        case EBTN_CHANGE_B1:
        case EBTN_CHANGE_B2:
        case EBTN_CHANGE_A1:
        case EBTN_CHANGE_A2:
        case EBTN_CHANGE_ROTATION1:
        case EBTN_CHANGE_ROTATION2:
        case EBTN_TOGGLE_OFFSET:
            RequireTextInput(uEventID);
            break;
        case EGLOBAL_EVENT_MAC_SYSTEM_FILE_OPEN_PATH_CHOSEN:
            OnFileOpened(cEvent.GetIntParam(1));
            break;
        case EBTN_CHANGE_IMAGE1:
        case EBTN_CHANGE_IMAGE2:
            m_uLastEventID = uEventID;
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
        case EBTN_CHANGE_TYPE1:
        case EBTN_CHANGE_TYPE2:
            {
                SNibEditInfo& sNib = (EBTN_CHANGE_TYPE1 == uEventID) ? m_sNib.psBtn->sIdleInfo : m_sNib.psBtn->sPressedInfo;
                switch (sNib.eType) {
                    case EBaseWidget_Image:         sNib.eType = EBaseWidget_3PartImage;
                        break;
                    case EBaseWidget_3PartImage:    sNib.eType = EBaseWidget_9PartImage;
                        break;
                    case EBaseWidget_9PartImage:    sNib.eType = EBaseWidget_Image;
                        break;
                    default:
                        return;
                }
                CEvent cEvent(CUIEditor::EEVENT_REFRESH_SELECTED_WIDGET);
                CEventManager::PostEvent(m_cMaster, cEvent);
                RefreshStats();
            }
            break;
        case EBTN_SNAP_SIZE_TO_TEXTURE1:
        case EBTN_SNAP_SIZE_TO_TEXTURE2:
            SnapToPictureSize(EBTN_SNAP_SIZE_TO_TEXTURE1 == uEventID);
            break;
        case EBTN_TOGGLE_MODE_TOGGLE:
            {
                m_sNib.psBtn->bToggleBtn = !m_sNib.psBtn->bToggleBtn;
                CEvent cEvent(CUIEditor::EEVENT_REFRESH_SELECTED_WIDGET);
                CEventManager::PostEvent(m_cMaster, cEvent);
                RefreshStats();
            }
            break;
    }
}

BOOLEAN CUIButtonEditor::OnTouchBegin(FLOAT fX, FLOAT fY)
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
BOOLEAN CUIButtonEditor::OnTouchMove(FLOAT fX, FLOAT fY)
{
    if (m_bUITouch) {
        return CGameWindow::OnTouchMove(fX, fY);
    }
    if (m_bDragMode) {
        SetLocalPosition(fX + m_afPointerOffset[0], fY + m_afPointerOffset[1]);
    }
    return TRUE;
}

BOOLEAN CUIButtonEditor::OnTouchEnd(FLOAT fX, FLOAT fY)
{
    if (m_bUITouch) {
        m_bUITouch = FALSE;
        return CGameWindow::OnTouchEnd(fX, fY);
    }
    return TRUE;
}

VOID CUIButtonEditor::RefreshStats(VOID)
{
    if (NULL == m_sNib.psBtn) {
        return;
    }
    const SNibEditInfo* apsInfo[2] = {
        &(m_sNib.psBtn->sIdleInfo),
        &(m_sNib.psBtn->sPressedInfo)
    };
    CHAR szBuffer[1024];
    
    if (m_sNib.psBtn->bToggleBtn) {
        m_pcToggleBtnMode->SetString("ToggleButton");
    }
    else {
        m_pcToggleBtnMode->SetString("NormalButton");
    }
    snprintf(szBuffer, 256, "Presed Offset\n%.1f", m_sNib.psBtn->fPressedOffset);
    m_pcOffset->SetString(szBuffer);
    UINT uIndex = 0;
    for ( ; 2 > uIndex; ++uIndex ) {
        const SNibEditInfo& sInfo = *apsInfo[uIndex];
        snprintf(szBuffer, 256, "Name:%s", sInfo.cName.c_str());
        m_pcWidgetName[uIndex]->SetString(szBuffer);
        const SNibSizeInfo& sSize = sInfo.sSize;
        snprintf(szBuffer, 256, "Size:%s", sSize.bFixedSize ? "Fixed" : "Ratio");
        m_pcWidgetFixedSize[uIndex]->SetString(szBuffer);
        snprintf(szBuffer, 256, "W:%.2f", sSize.afValues[0]);
        m_pcWidgetSizeX[uIndex]->SetString(szBuffer);
        snprintf(szBuffer, 256, "H:%.2f", sSize.afValues[1]);
        m_pcWidgetSizeY[uIndex]->SetString(szBuffer);
        snprintf(szBuffer, 256, "Aspect:%.2f", sSize.fAspectRatio);
        m_pcWidgetAspectRatio[uIndex]->SetString(szBuffer);
        
        const SNibPosInfo& sPos = sInfo.sPos;
        snprintf(szBuffer, 256, "Position:%s", sPos.bFixedSize ? "Fixed" : "Ratio");
        m_pcPositionType[uIndex]->SetString(szBuffer);
        m_pcPosXAlign[uIndex]->SetString(CNibManager::GetAlignmentTypeString((EWidgetPositionType)sPos.abyAlign[0]));
        m_pcPosYAlign[uIndex]->SetString(CNibManager::GetAlignmentTypeString((EWidgetPositionType)sPos.abyAlign[1]));
        snprintf(szBuffer, 256, "X:%.2f", sPos.afValues[0]);
        m_pcWidgetPosX[uIndex]->SetString(szBuffer);
        snprintf(szBuffer, 256, "Y:%.2f", sPos.afValues[1]);
        m_pcWidgetPosY[uIndex]->SetString(szBuffer);
        
        snprintf(szBuffer, 256, "AX:%.2f", sInfo.sAnchor.x);
        m_pcWidgetAnchorX[uIndex]->SetString(szBuffer);
        snprintf(szBuffer, 256, "AY:%.2f", sInfo.sAnchor.y);
        m_pcWidgetAnchorY[uIndex]->SetString(szBuffer);

        const SNibEditImageInfo& sImg = *(sInfo.psImg);
        snprintf(szBuffer, 1024, "Path: %s", sImg.cImgPath.c_str());
        m_pcImagePath[uIndex]->SetString(szBuffer);
        snprintf(szBuffer, 1024, "Left: %d", sImg.abyOffset[0]);
        m_pcLeft[uIndex]->SetString(szBuffer);
        snprintf(szBuffer, 1024, "Right: %d", sImg.abyOffset[1]);
        m_pcRight[uIndex]->SetString(szBuffer);
        snprintf(szBuffer, 1024, "Top: %d", sImg.abyOffset[2]);
        m_pcTop[uIndex]->SetString(szBuffer);
        snprintf(szBuffer, 1024, "Bottom: %d", sImg.abyOffset[3]);
        m_pcBottom[uIndex]->SetString(szBuffer);
        
        {
            const BYTE byVal = GET_RED(sImg.uColor);
            snprintf(szBuffer, 1024, "Red: %d", byVal);
            m_pcRGBA[uIndex][0]->SetString(szBuffer);
        }
        {
            const BYTE byVal = GET_GREEN(sImg.uColor);
            snprintf(szBuffer, 1024, "Green: %d", byVal);
            m_pcRGBA[uIndex][1]->SetString(szBuffer);
        }
        {
            const BYTE byVal = GET_BLUE(sImg.uColor);
            snprintf(szBuffer, 1024, "Blue: %d", byVal);
            m_pcRGBA[uIndex][2]->SetString(szBuffer);
        }
        {
            const BYTE byVal = GET_ALPHA(sImg.uColor);
            snprintf(szBuffer, 1024, "Alpha: %d", byVal);
            m_pcRGBA[uIndex][3]->SetString(szBuffer);
        }
        snprintf(szBuffer, 1024, "Rotate: %.1f", sImg.fRotation);
        m_pcRotation[uIndex]->SetString(szBuffer);
        switch (sInfo.eType) {
            case EBaseWidget_Image:
                m_pcType[uIndex]->SetString("Type: Image");
                break;
            case EBaseWidget_3PartImage:
                m_pcType[uIndex]->SetString("Type: 3Part");
                break;
            case EBaseWidget_9PartImage:
                m_pcType[uIndex]->SetString("Type: 9Part");
                break;
            default:
                m_pcType[uIndex]->SetString("Type: unknown");
                break;
        }
    }
}
VOID CUIButtonEditor::SnapToPictureSize(BOOLEAN bIdle)
{
    if (NULL == m_sNib.psBtn) {
        return;
    }
    SNibEditInfo& sImgInfo = bIdle ? m_sNib.psBtn->sIdleInfo : m_sNib.psBtn->sPressedInfo;
    if (NULL == sImgInfo.psImg) {
        return;
    }
    if (NULL == sImgInfo.psImg->pcTexture) {
        return;
    }
    CUIButton* pcSelected = CUIButton::CastToMe(m_cMaster.GetSelectedWidget());
    if (NULL == pcSelected) {
        return;
    }

    const FLOAT fW = sImgInfo.psImg->pcTexture->GetOriginalWidth();
    const FLOAT fH = sImgInfo.psImg->pcTexture->GetOriginalHeight();
    sImgInfo.sSize.afValues[0] = fW;
    sImgInfo.sSize.afValues[1] = fH;
    if (!sImgInfo.sSize.bFixedSize) {
        const SHLVector2D& sParentSize = pcSelected->GetLocalSize();
        sImgInfo.sSize.afValues[0] /= sParentSize.x;
        sImgInfo.sSize.afValues[1] /= sParentSize.y;
    }
    CEvent cEvent(CUIEditor::EEVENT_REFRESH_SELECTED_WIDGET);
    CEventManager::PostEvent(m_cMaster, cEvent);
    RefreshStats();
}
VOID CUIButtonEditor::OnToggleSizeType(BOOLEAN bIdle)
{
    CUIButton* pcSelected = CUIButton::CastToMe(m_cMaster.GetSelectedWidget());
    if (NULL == pcSelected) {
        return;
    }
    if (NULL == m_sNib.psBtn) {
        return;
    }
    CUIWidget* pcImgWidget = bIdle ? pcSelected->GetIdleWidget() : pcSelected->GetPressedWidget();
    if (NULL == pcImgWidget) {
        return;
    }
    SNibEditInfo& sImgInfo = bIdle ? m_sNib.psBtn->sIdleInfo : m_sNib.psBtn->sPressedInfo;
    sImgInfo.sSize.bFixedSize = !sImgInfo.sSize.bFixedSize;
    const SHLVector2D& sParentSize = pcSelected->GetLocalSize();
    const SHLVector2D& sImgSize = pcImgWidget->GetLocalSize();
    sImgInfo.sSize.afValues[0] = sImgSize.x;
    sImgInfo.sSize.afValues[1] = sImgSize.y;
    if (!sImgInfo.sSize.bFixedSize) {
        sImgInfo.sSize.afValues[0] /= sParentSize.x;
        sImgInfo.sSize.afValues[1] /= sParentSize.y;
    }
    CEvent cEvent(CUIEditor::EEVENT_REFRESH_SELECTED_WIDGET);
    CEventManager::PostEvent(m_cMaster, cEvent);
    RefreshStats();
}

VOID CUIButtonEditor::OnTogglePosType(BOOLEAN bIdle)
{
    CUIButton* pcSelected = CUIButton::CastToMe(m_cMaster.GetSelectedWidget());
    if (NULL == pcSelected) {
        return;
    }
    if (NULL == m_sNib.psBtn) {
        return;
    }
    CUIWidget* pcImgWidget = bIdle ? pcSelected->GetIdleWidget() : pcSelected->GetPressedWidget();
    if (NULL == pcImgWidget) {
        return;
    }
    SNibEditInfo& sImgInfo = bIdle ? m_sNib.psBtn->sIdleInfo : m_sNib.psBtn->sPressedInfo;
    
    sImgInfo.sPos.bFixedSize = !sImgInfo.sPos.bFixedSize;
    const SHLVector2D& sParentSize = pcSelected->GetLocalSize();
    const SHLVector2D& sImgPos = pcImgWidget->GetLocalPosition();
    sImgInfo.sPos.afValues[0] = sImgPos.x;
    sImgInfo.sPos.afValues[1] = sImgPos.y;
    if (!sImgInfo.sPos.bFixedSize) {
        sImgInfo.sPos.afValues[0] /= sParentSize.x;
        sImgInfo.sPos.afValues[1] /= sParentSize.y;
    }
    CEvent cEvent(CUIEditor::EEVENT_REFRESH_SELECTED_WIDGET);
    CEventManager::PostEvent(m_cMaster, cEvent);
    RefreshStats();
}
VOID CUIButtonEditor::OnTogglePositionAlignment(BOOLEAN bIdle, BOOLEAN bX)
{
    CUIButton* pcSelected = CUIButton::CastToMe(m_cMaster.GetSelectedWidget());
    if (NULL == pcSelected) {
        return;
    }
    if (NULL == m_sNib.psBtn) {
        return;
    }
    CUIWidget* pcImgWidget = bIdle ? pcSelected->GetIdleWidget() : pcSelected->GetPressedWidget();
    if (NULL == pcImgWidget) {
        return;
    }
    const SHLVector2D& sParentSize = pcSelected->GetLocalSize();
    SNibEditInfo& sImgInfo = bIdle ? m_sNib.psBtn->sIdleInfo : m_sNib.psBtn->sPressedInfo;
    SNibPosInfo& sPosInfo = sImgInfo.sPos;
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
    CEvent cEvent(CUIEditor::EEVENT_REFRESH_SELECTED_WIDGET);
    CEventManager::PostEvent(m_cMaster, cEvent);
    RefreshStats();
}

#endif // #if defined(MAC_PORT)

