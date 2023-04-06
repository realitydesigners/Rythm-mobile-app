#if !defined(NOW_LOADING_WINDOW_H)
#define NOW_LOADING_WINDOW_H
#include "UIWindow.h"
#include "UIImage.h"
#include "UI9PartImage.h"
#include "UITextLabel.h"

class CNowLoadingWindow : public CUIWindow
{
private:
	CNowLoadingWindow(UINT uMsgID, const CHAR* szMsg);
	virtual ~CNowLoadingWindow();

public:
    static VOID DisplayWindow(UINT uMsgID = 0xFFFFFFFF, const CHAR* szMsg = NULL);
    static VOID HideWindow(VOID);
    static BOOLEAN IsDisplayed(VOID);
    virtual VOID OnBackBtnPressed(VOID) OVERRIDE;
    
protected:
    virtual VOID InitializeInternals(VOID) OVERRIDE;
    virtual VOID Release(VOID) OVERRIDE;
private:
    CUIImage                m_cLoadingIcon;
    FLOAT                   m_fRotationInterval;
    CUI9PartImage           m_cBG;
    UINT                    m_uMSGID;
    CUITextLabel            m_cText;
    
    // update function
    virtual VOID OnUpdate(FLOAT fLapsed) OVERRIDE;
    virtual VOID OnReceiveEvent(CEvent& cEvent) OVERRIDE;
};


#endif // #if !defined(NOW_LOADING_WINDOW_H)

