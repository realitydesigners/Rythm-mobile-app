#if !defined(CMO_CHART_H)
#define CMO_CHART_H

#include "CBaseChart.h"
#include "CryptoUtil.h"
#include "UITextLabel.h"

class CCMOChart : public CBaseChart
{
public:
    CCMOChart(CTradeChart& cChart);
    virtual ~CCMOChart();

    virtual VOID Initialize(VOID) OVERRIDE;
    virtual VOID Release(VOID) OVERRIDE;

    virtual VOID SetupChart(VOID) OVERRIDE;
    
    virtual VOID OnRender(FLOAT fOffsetX, FLOAT fOffsetY) const OVERRIDE;
private:
    CUITextLabel*       m_pcLabels[3];
    FLOAT*              m_pfPosY;
    DOUBLE*             m_pdValues;

    virtual VOID GetTitleString(CHAR* szBuffer, UINT uBufferLen) const OVERRIDE;
    virtual VOID GetToolTipString(CHAR* szBuffer, UINT uBufferLen, UINT uCandleIndexSelected) const OVERRIDE;
    virtual FLOAT GetToolTipY(UINT uCandleIndexSelected) const OVERRIDE;
    
    VOID AdjustLabels(VOID);
};
#endif // #if !defined(CMO_CHART_H)
