#if !defined(TRADE_LOG_BUTTON_H)
#define TRADE_LOG_BUTTON_H

#include "UIContainer.h"
#include "UIImage.h"
#include "UITextLabel.h"
#include "TradeLogGraph.h"
#include "TradeLogList.h"
class CTradeLogWidget;

class CTradeLogButton : public CUIContainer
{
public:
    CTradeLogButton();
    virtual ~CTradeLogButton();
    
    BOOLEAN Initialize(VOID);
    virtual VOID Release(VOID) OVERRIDE;
    
    virtual VOID PostEvent(CEvent& cEvent, FLOAT fDelay) OVERRIDE;
    
    virtual BOOLEAN OnTouchBegin(FLOAT fX, FLOAT fY) OVERRIDE;
    virtual BOOLEAN OnTouchMove(FLOAT fX, FLOAT fY) OVERRIDE;
    virtual BOOLEAN OnTouchEnd(FLOAT fX, FLOAT fY) OVERRIDE;
private:
    enum EBTN_ID {
        EBTN_REATTACH=0,
        EBTN_TAP_WIDGET,
        EBTN_CLEAR,
#if defined(MAC_PORT)
        EBTN_EXPORT,
#endif // #if defined(MAC_PORT)
    };
    BOOLEAN         m_bRegistered;
    CUIImage        m_cHeaderBG;
    CUITextLabel    m_cLogTitle;
    
    CUIImage        m_cLogBG;
    CTradeLogList   m_cList;
    CTradeLogGraph  m_cGraph;
    BOOLEAN         m_bUITouched;
    BOOLEAN         m_bScrollMode; // whether we are scrolling
    FLOAT           m_afPrevTouch[2];
    CUIButton       m_cClearBTn;
    CUIImage        m_cClearBtnBG;
    CUITextLabel    m_cClearTxt;
#if defined(MAC_PORT)
    CUITextLabel    m_cExportTxt;
    CUIButton       m_cExportBtn;
#endif // #if defined(MAC_PORT)

    VOID UpdateTitle(VOID);
    VOID ToggleLog(VOID);
    
    virtual VOID OnReceiveEvent(CEvent& cEvent) OVERRIDE;
    
    VOID ReattachToParent(VOID);
    VOID OnTapWidget(CTradeLogWidget* pcWidget);
#if defined(MAC_PORT)
    VOID OnTapExportBtn(VOID);
#endif // #if defined(MAC_PORT)
};

#endif // #if !defined(TRADE_LOG_BUTTON_H)
