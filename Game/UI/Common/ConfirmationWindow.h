#if !defined(CONFIRMATION_WINDOW_H)
#define CONFIRMATION_WINDOW_H

#include "GameWindow.h"
#include "UI9PartImage.h"
#include "UITextLabel.h"

class CConfirmationWindow : public CGameWindow
{
public:
    enum EBTN_ID {
        EBTN_CANCEL = 0,
        EBTN_OK,
        EBTN_DEFAULT_CANCEL,
    };
    static VOID DisplayErrorMsg(const CHAR* szMessage, CEventHandler* pcHandler, UINT uCloseEventID);
    
    CConfirmationWindow(CEventHandler* pcHandler);
    virtual ~CConfirmationWindow();
    
    VOID SetTitle(const CHAR* szTitle);
    VOID SetMessage(const CHAR* szMessage);
    VOID SetOKButton(const CHAR* szOK, INT nEventID = -1);
    VOID SetCancelButton(const CHAR* szCancel, INT nEventID = -1);
    VOID HideCancelButton(VOID) { m_bHideCancel = TRUE; }
    VOID ShowDefaultCancelButton(VOID) { m_bDisplayUpperRightCancel = TRUE; }

protected:
    virtual VOID InitializeInternals(VOID) OVERRIDE;
    virtual VOID Release(VOID) OVERRIDE;
private:
    CUI9PartImage   m_cBG;
    CUITextLabel    m_cTitle;
    CUITextLabel    m_cMessage;
    CUITextLabel    m_cOK;
    CUITextLabel    m_cCancel;
    BOOLEAN         m_bHideCancel;
    BOOLEAN         m_bDisplayUpperRightCancel;
    INT             m_nOKID;
    INT             m_nCancelID;
    CEventHandler*  m_pcHandler;
    
    virtual VOID OnReceiveEvent(CEvent& cEvent) OVERRIDE;
};
#endif // #if !defined(CONFIRMATION_WINDOW_H)
