#include "stdafx.h"
#if defined(MAC_PORT)
#include "UIEditor.h"
#include "EventManager.h"
#include "MacSystem.h"
#include "UIImageEditor.h"
#include "UITextEditor.h"
#include "UIButtonEditor.h"

VOID CUIEditor::OnReceiveEvent(CEvent& cEvent)
{
    const UINT uEventID = cEvent.GetIntParam(0);
    switch (uEventID) {
        case EBTN_OUTPUT_BINARY:
            TrySaveBinary();
            break;
        case EBTN_DUPLICATE:
            PerformDuplicate();
            break;
        case EBTN_ATTACH_DETACH_CONTAINER:
            ToggleAttachDetach();
            break;
        case EBTN_MOVE_SELECTED_TO_FRONT:
            MoveSelectedToFront();
            break;
        case EBTN_CREATE_TEXT:
            CreateNewText();
            break;
        case EBTN_CREATE_IMAGE:
            CreateNewImage();
            break;
        case EBTN_CREATE_CONTAINER:
            CreateNewContainer();
            break;
        case EBTN_TOGGLE_INPUT:
            ToggleInput(!m_bInputEnabled);
            break;
        case EBTN_CONVERT_IMAGE_TO_BUTTON:
            ConvertImageToButton();
            break;
        case EEVENT_SNAP_IMAGE_TO_TEXTURE_SIZE:
            SnapImageToTextureSize();
            break;
        case EEVENT_REFRESH_SELECTED_WIDGET:
            RefreshSelectedWidget();
            break;
        case EBTN_TOGGLE_TYPE:
            OnToggleType();
            break;
        case EGLOBAL_EVENT_MAC_SYSTEM_KEY_PRESSED:
            OnKeyPressed(cEvent.GetIntParam(1));
            break;
        case EBTN_CHANGE_RESOLUTION:
            AdjResolution(1);
            break;
        case EBTN_TOGGLE_FIXED_SIZE:
            OnToggleSizeType();
            break;
        case EBTN_TOGGLE_POSITION_FIXED_SIZE:
            OnTogglePosType();
            break;
        case EBTN_TOGGLE_POSITION_X_ALIGNMENT:
            OnTogglePositionAlignment(TRUE);
            break;
        case EBTN_TOGGLE_POSITION_Y_ALIGNMENT:
            OnTogglePositionAlignment(FALSE);
            break;
        case EBTN_SAVE_JSON:
            if (m_bEdited) {
                m_uLastEventID = EBTN_SAVE_JSON;
                CEventManager::RegisterForBroadcast(*this, EGLOBAL_MAC_SYSTEM_MGR, EGLOBAL_EVENT_MAC_SYSTEM_FILE_SAVE_PATH_CHOSEN, TRUE);
                CMacSystem::TryGetSaveFilePath("json");
            }
            break;
        case EGLOBAL_EVENT_MAC_SYSTEM_FILE_SAVE_PATH_CHOSEN:
            OnOutputPathChosen(cEvent.GetIntParam(1));
            break;
        case EGLOBAL_EVENT_MAC_SYSTEM_FILE_OPEN_PATH_CHOSEN:
            OnReceiveMacPath(cEvent.GetIntParam(1));
            break;
        case EBTN_CHANGE_PATH:
            m_uLastEventID = uEventID;
            CEventManager::RegisterForBroadcast(*this, EGLOBAL_MAC_SYSTEM_MGR, EGLOBAL_EVENT_MAC_SYSTEM_FILE_OPEN_PATH_CHOSEN, TRUE);
            CMacSystem::TryOpenPath();
            break;
        case EBTN_NEW_JSON:
            OnNewUI(FALSE);
            break;
        case EBTN_NEW_JSON_CONFIRM:
            OnNewUI(TRUE);
            break;
        case EBTN_LOAD_JSON:
            OnLoadBtnPressed(FALSE);
            break;
        case EBTN_LOAD_JSON_CONFIRM:
            OnLoadBtnPressed(TRUE);
            break;
        case EBTN_CHANGE_NAME:
        case EBTN_CHANGE_WIDTH:
        case EBTN_CHANGE_HEIGHT:
        case EBTN_CHANGE_ASPECT:
        case EBTN_CHANGE_X:
        case EBTN_CHANGE_Y:
        case EBTN_CHANGE_AX:
        case EBTN_CHANGE_AY:
        case EBTN_SET_CLASSNAME:
            RequireTextInput(uEventID);
            break;
        case EEVENT_TEXT_INPUT_REPLY:
            OnTextInput();
            break;
        default:
            break;
    }
}

VOID CUIEditor::OnToggleType(VOID)
{
    CUIWidget* pcSelected = GetSelectedWidget();
    if (NULL == pcSelected) {
        return;
    }
    SNibEditInfo* psInfo = FindNib(pcSelected);
    if (NULL == psInfo) {
        return;
    }
    switch (psInfo->eType) {
        case EBaseWidget_Image:
        case EBaseWidget_3PartImage:
        case EBaseWidget_9PartImage:
            {
                CUIWindow* pcWin = new CUIImageEditor(*this, *psInfo);
                if (NULL != pcWin) {
                    pcWin->DoModal();
                }
            }
            break;
        case EBaseWidget_TextLabel:
            {
                CUIWindow* pcWin = new CUITextEditor(*this, *psInfo);
                if (NULL != pcWin) {
                    pcWin->DoModal();
                }
            }
            break;
        case EBaseWidget_Button:
            {
                CUIWindow* pcWin = new CUIButtonEditor(*this, *psInfo);
                if (NULL != pcWin) {
                    pcWin->DoModal();
                }
            }
            break;
        default:
            break;
    }
}

#endif // #if defined(MAC_PORT)

