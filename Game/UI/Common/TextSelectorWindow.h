#if !defined(TEXT_SELECTOR_WINDOW_H)
#define TEXT_SELECTOR_WINDOW_H

#include "GameWindow.h"
#include "UIImage.h"
#include "UIScroller.h"

class CEventHandler;
class CTextSelectorWindow : public CGameWindow
{
public:
    enum EBTN_ID {
        EBTN_CHANGE_BTN=0,
    };
    
    CTextSelectorWindow(CEventHandler& cHandler, UINT uEventID, UINT uParam);
    virtual ~CTextSelectorWindow();
    
    BOOLEAN Initialize(const CHAR** ppszSelections, UINT uSelectionNum, UINT uCurrentSelectedIndex);
    virtual VOID Release(VOID) OVERRIDE;
protected:
    virtual VOID InitializeInternals(VOID) OVERRIDE;
private:
    virtual VOID OnReceiveEvent(CEvent& cEvent) OVERRIDE;
    
    VOID OnBtnPressed(UINT uIndex);
    
    CEventHandler&          m_cHandler;
    UINT                    m_uEventID;
    UINT                    m_uParam;
    UINT                    m_uCurrentSelectedIndex;
    UINT                    m_uSelectionNum;
    CUIScroller             m_cScroller;
    CUITextLabel**          m_ppcTextLabels;
    CUIButton*              m_pcBtns;
    CUIImage                m_cBG;
    CUIImage                m_cSelected;
    CUIImage                m_cNotSelected;
};
#endif // #if !defined(TEXT_SELECTOR_WINDOW_H)
