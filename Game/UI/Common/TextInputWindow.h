#if !defined(TEXT_INPUT_WINDOW_H)
#define TEXT_INPUT_WINDOW_H

#include "GameWindow.h"
#include "Event.h"
#include "TextInputManager.h"
#include "UITextLabel.h"
class CTextInputWindow : public CGameWindow
{
public:
    enum EBTN_ID {
        EBTN_EXIT = 0,
        EBTN_DONE,
    };

    CTextInputWindow(CHAR* szStringBuffer, UINT uMaxLen, CEventHandler& cHandler, const CEvent& cEvent, EInputType eType = EInputType_Generic);
    virtual ~CTextInputWindow();
    
    virtual VOID OnBackBtnPressed(VOID) OVERRIDE;
    VOID SetTitleAndActionText(const CHAR* szTitle, const CHAR* szAction);
    VOID DoNotAllowQuit(VOID) { m_bAllowQuit = FALSE; }
protected:
    virtual VOID InitializeInternals(VOID) OVERRIDE;
    virtual VOID Release(VOID) OVERRIDE;
    
    virtual VOID PostDoModal(VOID) OVERRIDE;
private:
    CUITextLabel        m_cTitleText;
    CUITextLabel        m_cActionText;
    CHAR*               m_szStringBuffer;
    const UINT          m_uMaxLen;
    CEventHandler&      m_cHandler;
    CEvent              m_cEvent;
    const EInputType    m_eInputType;
    BOOLEAN             m_bAllowQuit;
    CUIWidget*          m_pcTextBGWidget;
    
    virtual VOID OnReceiveEvent(CEvent& cEvent) OVERRIDE;
    
    VOID OnInputCompleted(VOID);
};
#endif // #if !defined(QUIT_GAME_WINDOW_H)
