#if !defined(TICKER_OBV_H)
#define TICKER_OBV_H

#include "TickDef.h"
#include "UIContainer.h"
#include "UITextLabel.h"

class CTickerOBV : public CUIContainer
{
public:
    CTickerOBV(BOOLEAN bSecondsMode);
    virtual ~CTickerOBV();

    VOID UpdateTick(const STickListener& sTicker);
    VOID UpdateMaxDisplayableNum(UINT uNum);
    
    FLOAT  GetLastTickOBVY(VOID) const              { return m_fOBVLastTickY; }
    DOUBLE GetLastTickOBV(VOID) const               { return m_dOBVLastTick; }
    DOUBLE GetPixelsPerOBV(VOID) const              { return m_dPixelsPerOBV; }
    FLOAT  GetOBVDiffZeroY(VOID) const              { return m_fOBVDiffZeroY; }
    VOID   SetMaster(const CTickerOBV* pcMaster)    { m_pcMaster = pcMaster; }

    virtual VOID OnRender(FLOAT fOffsetX, FLOAT fOffsetY) const OVERRIDE;
    virtual VOID Release(VOID) OVERRIDE;
private:
    const BOOLEAN       m_bSecondsMode;
    UINT                m_uMaxDisplayableNum;
    const CTickerOBV*   m_pcMaster;
    CUITextLabel        m_cNowDiff;
    CUITextLabel        m_cLow;
    CUITextLabel        m_cHigh;
    UINT                m_uNum;
    FLOAT               m_fFirstOBVY;
    FLOAT*              m_pfOBVY;
    FLOAT*              m_pfOBVDiffY;
    FLOAT               m_fOBVLastTickY;    // Y value of the tick before the first seconds tick
    DOUBLE              m_dOBVLastTick;
    DOUBLE              m_dPixelsPerOBV;
    FLOAT               m_fOBVDiffZeroY;
    FLOAT               m_fWidthPerTick;
    DOUBLE              m_dPrevLowest;
    DOUBLE              m_dPrevHighest;
    
    FLOAT               m_afLines[2][2][2];
    
    VOID UpdateLines(const STickListener& sTicker);
    
};
#endif // #if !defined(TICKER_OBV_H)
