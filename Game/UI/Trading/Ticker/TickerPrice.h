#if !defined(TICKER_PRICE_H)
#define TICKER_PRICE_H

#include "TickDef.h"
#include "UIContainer.h"
#include "UITextLabel.h"

class CTickerPrice : public CUIContainer
{
public:
    CTickerPrice(BOOLEAN bSecondsMode, UINT uEventHandlerID = 0, UINT uPickEventID = 0);
    virtual ~CTickerPrice();

    VOID SetSoloMode(VOID) { m_bSoloMode = TRUE; }
    VOID UpdateMaxDisplayableNum(UINT uNum);
    VOID UpdateTick(const STickListener& sTicker);

    VOID Focus(UINT uOpenSec, UINT uCloseSec);
    VOID ClearFocus(VOID);

    
    FLOAT   GetPriceChangeRatio(VOID) const { return m_fPriceChangeRatio; }
    DOUBLE  GetPixelsPerPrice(VOID) const   { return m_dPixelsPerPrice; }
    DOUBLE  GetHighestPrice(VOID) const     { return m_dHighestPrice; }
    VOID    SetMaster(const CTickerPrice* pcMaster) { m_pcMaster = pcMaster; }
    
    virtual BOOLEAN OnTouchBegin(FLOAT fX, FLOAT fY) OVERRIDE;
    virtual BOOLEAN OnTouchMove(FLOAT fX, FLOAT fY) OVERRIDE;
    virtual BOOLEAN OnTouchEnd(FLOAT fX, FLOAT fY) OVERRIDE;

    virtual VOID OnRender(FLOAT fOffsetX, FLOAT fOffsetY) const OVERRIDE;
    virtual VOID Release(VOID) OVERRIDE;
private:
    const BOOLEAN   m_bSecondsMode;
    const UINT      m_uEventHandlerID;
    const UINT      m_uPickEventID;
    BOOLEAN         m_bSoloMode;
    UINT            m_uMaxDisplayableNum;
    CUITextLabel    m_cLowPrice;
    CUITextLabel    m_cHighPrice;
    UINT            m_uNum;
    FLOAT*          m_pfYValues;
    FLOAT           m_fWidthPerTick;
    FLOAT           m_fPriceChangeRatio;
    DOUBLE          m_dPrevLowest;
    DOUBLE          m_dPrevHighest;
    DOUBLE          m_dPixelsPerPrice;
    DOUBLE          m_dHighestPrice;
    const CTickerPrice*   m_pcMaster;
    FLOAT           m_afLines[2][2][2];
    UINT            m_uFocusStartSec;
    UINT            m_uFocusEndSec;
    UINT            m_uFocusStartIndex;
    UINT            m_uFocusEndIndex;
    
    // touch ui related
    BOOLEAN         m_bPerformPickCheck;
    UINT            m_uStartTimeSec;
    UINT            m_uPerTickTimeSec;

    
    VOID UpdateLines(const STickListener& sTicker);
};
#endif // #if !defined(TICKER_PRICE_H)
