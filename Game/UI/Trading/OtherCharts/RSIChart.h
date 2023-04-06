#if !defined(RSI_CHART_H)
#define RSI_CHART_H

#include "CBaseChart.h"
#include "CryptoUtil.h"
#include "UITextLabel.h"

class CRSIChart : public CBaseChart
{
public:
    CRSIChart(CTradeChart& cChart);
    virtual ~CRSIChart();

    virtual VOID Initialize(VOID) OVERRIDE;
    virtual VOID Release(VOID) OVERRIDE;

    virtual VOID SetupChart(VOID) OVERRIDE;
    
    virtual VOID OnRender(FLOAT fOffsetX, FLOAT fOffsetY) const OVERRIDE;
private:
    CUITextLabel*       m_pcLabels[3];
    DOUBLE*             m_pdValues;
    FLOAT*              m_pfValuesY;

    virtual VOID GetTitleString(CHAR* szBuffer, UINT uBufferLen) const OVERRIDE;
    virtual VOID GetToolTipString(CHAR* szBuffer, UINT uBufferLen, UINT uCandleIndexSelected) const OVERRIDE;
    virtual FLOAT GetToolTipY(UINT uCandleIndexSelected) const OVERRIDE;
    
    VOID AdjustLabels(VOID);
    
};
#endif // #if !defined(MACD_CHART_H)
