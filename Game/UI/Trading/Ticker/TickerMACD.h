#if !defined(TICKER_MACD_H)
#define TICKER_MACD_H

#include "TickDef.h"
#include "UIContainer.h"
#include "UITextLabel.h"

class CTickerMACD : public CUIContainer
{
public:
    CTickerMACD();
    virtual ~CTickerMACD();

    VOID UpdateMaxDisplayableNum(UINT uNum);
    VOID UpdateTick(const STickListener& sTickListener);
    
    virtual VOID OnRender(FLOAT fOffsetX, FLOAT fOffsetY) const OVERRIDE;
    virtual VOID Release(VOID) OVERRIDE;
private:
    UINT            m_uMaxDisplayableNum;
    UINT            m_uNum;
    FLOAT*          m_apfMACDY[2];
    FLOAT           m_fMACDZeroY;
    FLOAT*          m_pfHistogramHeight;
    FLOAT           m_fHistogramZeroY;
    FLOAT           m_fWidthPerTick;
    
    VOID    ReleaseArrays(VOID);
};
#endif // #if !defined(TICKER_MACD_H)
