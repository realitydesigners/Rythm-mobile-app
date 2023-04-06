#if !defined(OBV_CHART_H)
#define OBV_CHART_H

#include "CBaseChart.h"
#include "CryptoUtil.h"
#include "UITextLabel.h"

class COBVChart : public CBaseChart
{
public:
    COBVChart(CTradeChart& cChart);
    virtual ~COBVChart();

    virtual VOID Initialize(VOID) OVERRIDE;
    virtual VOID Release(VOID) OVERRIDE;

    virtual VOID SetupChart(VOID) OVERRIDE;
    
    virtual VOID OnRender(FLOAT fOffsetX, FLOAT fOffsetY) const OVERRIDE;
private:
    CUITextLabel m_cTotalVol;
    UINT    m_uOBVNum;              // number of OBV
    DOUBLE* m_pdOBV;                // have all candle values
    UINT    m_uVisibleStartIndex;   // the actual start index which is visible
    FLOAT*  m_pfValuesY;            // this value has m_uIntervalNum (only visible candles)
    FLOAT*  m_pfDifferenceY;        // this value has m_uIntervalNum (only visible candles)
    virtual VOID GetTitleString(CHAR* szBuffer, UINT uBufferLen) const OVERRIDE;
    virtual VOID GetToolTipString(CHAR* szBuffer, UINT uBufferLen, UINT uCandleIndexSelected) const OVERRIDE;
    virtual FLOAT GetToolTipY(UINT uCandleIndexSelected) const OVERRIDE;
};
#endif // #if !defined(MACD_CHART_H)
