#if !defined(MACD_CHART_H)
#define MACD_CHART_H

#include "CBaseChart.h"
#include "CryptoUtil.h"
#include "UITextLabel.h"

class CMACDChart : public CBaseChart
{
public:
    CMACDChart(CTradeChart& cChart);
    virtual ~CMACDChart();

    virtual VOID Initialize(VOID) OVERRIDE;
    virtual VOID Release(VOID) OVERRIDE;

    virtual VOID SetupChart(VOID) OVERRIDE;
    
    virtual VOID OnRender(FLOAT fOffsetX, FLOAT fOffsetY) const OVERRIDE;
private:
    CUITextLabel*       m_pcLabels[2];
    FLOAT*              m_pfPosY[2]; // 0 = MACD Line (MA1 - MA2), 1 = Signal Line (MA of MACD Line)
    DOUBLE*             m_pdValues[2];
    DOUBLE*             m_pdPrices;
    DOUBLE*             m_pdMA1;
    DOUBLE*             m_pdMA2;
    FLOAT*              m_pfHistogramHeight;
    UINT                m_uMaxSize;
    FLOAT               m_fSignalZeroY;

    virtual VOID GetTitleString(CHAR* szBuffer, UINT uBufferLen) const OVERRIDE;
    virtual VOID GetToolTipString(CHAR* szBuffer, UINT uBufferLen, UINT uCandleIndexSelected) const OVERRIDE;
    virtual FLOAT GetToolTipY(UINT uCandleIndexSelected) const OVERRIDE;
    
    VOID ClearDataArrays(VOID);
    BOOLEAN SetupDataArrays(UINT uNum);
};
#endif // #if !defined(MACD_CHART_H)
