#if !defined(UI_BUTTON_EDITOR_H)
#define UI_BUTTON_EDITOR_H

#if defined(MAC_PORT)
#include "GameWindow.h"
#include "UIEditorDefs.h"

class CUIEditor;
class CUIButtonEditor : public CGameWindow
{
public:
    enum EBTN_ID {
        EBTN_EXIT,
        EBTN_TOGGLE_OFFSET,
        EBTN_TOGGLE_MODE_TOGGLE,
        
        EBTN_CHANGE_NAME1,
        EBTN_CHANGE_NAME2,
        EBTN_TOGGLE_FIXED_SIZE1,
        EBTN_TOGGLE_FIXED_SIZE2,
        EBTN_CHANGE_WIDTH1,
        EBTN_CHANGE_WIDTH2,
        EBTN_CHANGE_HEIGHT1,
        EBTN_CHANGE_HEIGHT2,
        EBTN_CHANGE_ASPECT1,
        EBTN_CHANGE_ASPECT2,
        EBTN_TOGGLE_POSITION_FIXED_SIZE1,
        EBTN_TOGGLE_POSITION_FIXED_SIZE2,
        EBTN_TOGGLE_POSITION_X_ALIGNMENT1,
        EBTN_TOGGLE_POSITION_X_ALIGNMENT2,
        EBTN_TOGGLE_POSITION_Y_ALIGNMENT1,
        EBTN_TOGGLE_POSITION_Y_ALIGNMENT2,
        EBTN_CHANGE_X1,
        EBTN_CHANGE_X2,
        EBTN_CHANGE_Y1,
        EBTN_CHANGE_Y2,
        EBTN_CHANGE_AX1,
        EBTN_CHANGE_AX2,
        EBTN_CHANGE_AY1,
        EBTN_CHANGE_AY2,

        EBTN_CHANGE_IMAGE1,
        EBTN_CHANGE_IMAGE2,
        EBTN_CHANGE_LEFT1,
        EBTN_CHANGE_LEFT2,
        EBTN_CHANGE_RIGHT1,
        EBTN_CHANGE_RIGHT2,
        EBTN_CHANGE_TOP1,
        EBTN_CHANGE_TOP2,
        EBTN_CHANGE_BOTTOM1,
        EBTN_CHANGE_BOTTOM2,
        EBTN_CHANGE_R1,
        EBTN_CHANGE_R2,
        EBTN_CHANGE_G1,
        EBTN_CHANGE_G2,
        EBTN_CHANGE_B1,
        EBTN_CHANGE_B2,
        EBTN_CHANGE_A1,
        EBTN_CHANGE_A2,
        EBTN_CHANGE_ROTATION1,
        EBTN_CHANGE_ROTATION2,
        EBTN_CHANGE_TYPE1,
        EBTN_CHANGE_TYPE2,
        EBTN_SNAP_SIZE_TO_TEXTURE1,
        EBTN_SNAP_SIZE_TO_TEXTURE2,

        EEVENT_TEXT_INPUT_REPLY,
    };

    CUIButtonEditor(CUIEditor& cMaster, SNibEditInfo& sNib);
    virtual ~CUIButtonEditor();
    
    virtual BOOLEAN OnTouchBegin(FLOAT fX, FLOAT fY) OVERRIDE;
    virtual BOOLEAN OnTouchMove(FLOAT fX, FLOAT fY) OVERRIDE;
    virtual BOOLEAN OnTouchEnd(FLOAT fX, FLOAT fY) OVERRIDE;

protected:
    virtual VOID InitializeInternals(VOID) OVERRIDE;
    virtual VOID Release(VOID) OVERRIDE;
private:
    CUIEditor&      m_cMaster;
    SNibEditInfo&   m_sNib;
    BOOLEAN         m_bUITouch;
    BOOLEAN         m_bDragMode;
    FLOAT           m_afPointerOffset[2];
    CHAR            m_szStringBuffer[256];
    UINT            m_uLastEventID;
    UINT            m_uLastColor;
    
    CUITextLabel*   m_pcOffset;
    CUITextLabel*   m_pcToggleBtnMode;
    CUITextLabel*   m_pcWidgetName[2];
    CUITextLabel*   m_pcWidgetFixedSize[2];
    CUITextLabel*   m_pcWidgetSizeX[2];
    CUITextLabel*   m_pcWidgetSizeY[2];
    CUITextLabel*   m_pcWidgetAspectRatio[2];
    CUITextLabel*   m_pcPositionType[2];
    CUITextLabel*   m_pcPosXAlign[2];
    CUITextLabel*   m_pcPosYAlign[2];
    CUITextLabel*   m_pcWidgetPosX[2];
    CUITextLabel*   m_pcWidgetPosY[2];
    CUITextLabel*   m_pcWidgetAnchorX[2];
    CUITextLabel*   m_pcWidgetAnchorY[2];
    CUITextLabel*   m_pcImagePath[2];
    CUITextLabel*   m_pcLeft[2];
    CUITextLabel*   m_pcRight[2];
    CUITextLabel*   m_pcTop[2];
    CUITextLabel*   m_pcBottom[2];
    CUITextLabel*   m_pcRGBA[2][4];
    CUITextLabel*   m_pcRotation[2];
    CUITextLabel*   m_pcType[2];
    
    virtual VOID OnReceiveEvent(CEvent& cEvent) OVERRIDE;
    
    VOID RefreshStats(VOID);
    VOID RequireTextInput(UINT uEventID);
    VOID OnInputReply(VOID);
    VOID OnFileOpened(BOOLEAN bSucess);
    
    VOID OnToggleSizeType(BOOLEAN bIdle);
    VOID OnTogglePosType(BOOLEAN bIdle);
    VOID OnTogglePositionAlignment(BOOLEAN bIdle, BOOLEAN bX);
    VOID SnapToPictureSize(BOOLEAN bIdle);
};

#endif // #if defined(MAC_PORT)
#endif // #if !defined(UI_BUTTON_EDITOR_H)
