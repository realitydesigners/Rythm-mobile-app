#if !defined(BOLLINGER_BANDS_H)
#define BOLLINGER_BANDS_H

#include "MAUtil.h"
class CTradeChart;

class CBollingerBands
{
public:
    CBollingerBands(const CTradeChart& cChart);
    ~CBollingerBands();

    VOID Release(VOID);

    VOID SetupChart(FLOAT fHighestPriceY, DOUBLE dHighestPrice, DOUBLE dPixelPerPrice);
    
    VOID Render(FLOAT fOffsetX, FLOAT fOffsetY) const;
    
    VOID GetToolTipString(CHAR* szBuffer, UINT uBufferLen, UINT uSelectedIndex);
private:
    const CTradeChart& m_cChart;
    
    SBollingerBands m_sBands;
    FLOAT*          m_pfPolygonValues;
    UINT            m_uCandleNum;
    UINT            m_uMaxNum;
    
    VOID AllocateArrays(UINT uSize);
};
#endif // #if !defined(BOLLINGER_BANDS_H)
