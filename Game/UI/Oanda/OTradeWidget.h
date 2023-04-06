#if !defined(OANDA_TRADE_WIDGET_H)
#define OANDA_TRADE_WIDGET_H

#include "OUIDefs.h"
#include "OandaMgr.h"
#include "UIButton.h"
#include "UIImage.h"
#include "UITextLabel.h"

class COTradeWidget : public CUIButton
{
public:
    COTradeWidget(const COandaMgr::SOandaTrade& sTrade);
    virtual ~COTradeWidget();
    
    BOOLEAN Initialize(UINT uSelectedEventID);
    virtual VOID Release(VOID) OVERRIDE;
    
    virtual VOID PostEvent(CEvent& cEvent, FLOAT fDelay) OVERRIDE;
    
    const COandaMgr::SOandaTrade& GetTrade(VOID) const { return m_sTrade; }
private:
    const COandaMgr::SOandaTrade m_sTrade;
    CUIImage     m_cBG;
    CUITextLabel m_cTradePairAndUnits;
    CUITextLabel m_cPositionAndPrice;
    CUIImage     m_cArrow;
    CUITextLabel m_cCurrentPrice;
    CUITextLabel m_cPLAndPip;
    
    DOUBLE       m_dPipMultiplier;
    
    virtual VOID OnReceiveEvent(CEvent& cEvent) OVERRIDE;
    virtual BOOLEAN CreateButtonEvent(CEvent& cEvent) OVERRIDE;
    VOID OnPriceUpdate(UINT uTradePairIndex);
    VOID UpdatePip(DOUBLE dDiff);
};

#endif // #if !defined(OANDA_TRADE_WIDGET_H)
