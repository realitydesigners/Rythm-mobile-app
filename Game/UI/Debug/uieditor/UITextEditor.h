#if !defined(UI_TEXT_EDITOR_H)
#define UI_TEXT_EDITOR_H

#if defined(MAC_PORT)
#include "GameWindow.h"
#include "UIEditorDefs.h"
class CUIEditor;
class CUITextEditor : public CGameWindow
{
public:
    enum EBTN_ID {
        EBTN_TOGGLE_FONT_SIZE = 0,
        EBTN_TOGGLE_ALIGNMENT,
        EBTN_CHANGE_DEFAULT_MSG,
        EBTN_CHANGE_R,
        EBTN_CHANGE_G,
        EBTN_CHANGE_B,
        EBTN_CHANGE_A,
        EBTN_TOGGLE_EFX,
        EBTN_CHANGE_EFX_PARAM1,
        EBTN_CHANGE_EFX_PARAM2,
        EBTN_CHANGE_EFX_R,
        EBTN_CHANGE_EFX_G,
        EBTN_CHANGE_EFX_B,
        EBTN_CHANGE_EFX_A,
        EBTN_PARAM_LABEL_NUM,
        EBTN_EXIT,
        EEVENT_TEXT_INPUT_REPLY,
    };

    CUITextEditor(CUIEditor& cMaster, SNibEditInfo& sNib);
    virtual ~CUITextEditor();
    
    virtual BOOLEAN OnTouchBegin(FLOAT fX, FLOAT fY) OVERRIDE;
    virtual BOOLEAN OnTouchMove(FLOAT fX, FLOAT fY) OVERRIDE;
    virtual BOOLEAN OnTouchEnd(FLOAT fX, FLOAT fY) OVERRIDE;

protected:
    virtual VOID InitializeInternals(VOID) OVERRIDE;
    virtual VOID Release(VOID) OVERRIDE;
private:
    CUIEditor&      m_cMaster;
    SNibEditInfo&   m_sNib;
    CUITextLabel*   m_apcLabels[EBTN_PARAM_LABEL_NUM];
    
    BOOLEAN         m_bUITouch;
    BOOLEAN         m_bDragMode;
    FLOAT           m_afPointerOffset[2];
    CHAR            m_szStringBuffer[256];
    UINT            m_uLastEventID;
    
    virtual VOID OnReceiveEvent(CEvent& cEvent) OVERRIDE;
    
    VOID RefreshStats(VOID);
    VOID RequireTextInput(UINT uEventID);
    VOID OnInputReply(VOID);
};

#endif // #if defined(MAC_PORT)
#endif // #if !defined(CUITextEditor)
