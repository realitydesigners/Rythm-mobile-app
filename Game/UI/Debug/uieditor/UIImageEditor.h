#if !defined(UI_IMAGE_EDITOR_H)
#define UI_IMAGE_EDITOR_H

#if defined(MAC_PORT)
#include "GameWindow.h"
#include "UIEditorDefs.h"
class CUIEditor;
class CUIImageEditor : public CGameWindow
{
public:
    enum EBTN_ID {
        EBTN_EXIT,
        EBTN_CHANGE_IMAGE,
        EBTN_CHANGE_LEFT,
        EBTN_CHANGE_RIGHT,
        EBTN_CHANGE_TOP,
        EBTN_CHANGE_BOTTOM,
        EBTN_CHANGE_R,
        EBTN_CHANGE_G,
        EBTN_CHANGE_B,
        EBTN_CHANGE_A,
        EBTN_CHANGE_ROTATION,
        EBTN_CHANGE_TYPE,
        EBTN_CHANGE_VERTICAL,
        EBTN_SNAP_SIZE_TO_TEXTURE,
        
        EEVENT_TEXT_INPUT_REPLY,
    };

    CUIImageEditor(CUIEditor& cMaster, SNibEditInfo& sNib);
    virtual ~CUIImageEditor();
    
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
    CUITextLabel*   m_pcImagePath;
    CUITextLabel*   m_pcLeft;
    CUITextLabel*   m_pcRight;
    CUITextLabel*   m_pcTop;
    CUITextLabel*   m_pcBottom;
    CUITextLabel*   m_pcRGBA[4];
    CUITextLabel*   m_pcRotation;
    CUITextLabel*   m_pcType;
    CUITextLabel*   m_pcVertical;
    
    virtual VOID OnReceiveEvent(CEvent& cEvent) OVERRIDE;
    
    VOID RefreshStats(VOID);
    VOID RequireTextInput(UINT uEventID);
    VOID OnInputReply(VOID);
    VOID OnFileOpened(BOOLEAN bSucess);
};

#endif // #if defined(MAC_PORT)
#endif // #if !defined(UI_IMAGE_EDITOR_H)
