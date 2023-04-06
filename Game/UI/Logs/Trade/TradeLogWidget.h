#if !defined(TRADE_LOG_WIDGET_HEADER)
#define TRADE_LOG_WIDGET_HEADER

#include "UIButton.h"
#include "UIImage.h"
#include "UITextLabel.h"
#include "TradeLogDefs.h"
#include "TradePairName.h"

#define TRADE_LOG_WIDGET_W (298.0f)
#define TRADE_LOG_WIDGET_H (40.0f)

class CTradeLogWidget : public CUIButton
{
public:
    static VOID GetTimeString(UINT uGmtTimeSec, CHAR* szBuffer, UINT uBufferLen);
    CTradeLogWidget(const STradeLog& sLog);
    virtual ~CTradeLogWidget();
    
    BOOLEAN Initialize(UINT uEventID);
    virtual VOID Release(VOID) OVERRIDE;
    
    virtual VOID OnRender(FLOAT fOffsetX, FLOAT fOffsetY) const OVERRIDE;

    
    VOID SetSelected(BOOLEAN bSelected);
    
    const STradeLog& GetLog(VOID) const { return m_sLog; }
private:
    const STradeLog     m_sLog; // make a copy in case deleted
    CUIImage            m_cBG;
    CTradePairName      m_cTradeName;
    CUITextLabel        m_cDate;
    CUITextLabel        m_cID;
    CUITextLabel        m_cPip;
    virtual BOOLEAN CreateButtonEvent(CEvent& cEvent) OVERRIDE;
};

#endif // #if !defined(TRADE_LOG_WIDGET_HEADER)
