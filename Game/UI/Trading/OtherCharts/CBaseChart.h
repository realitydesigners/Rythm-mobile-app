#if !defined(BASE_CHART_H)
#define BASE_CHART_H

#include "UIContainer.h"
#include "UITextLabel.h"
#include "CryptoUtil.h"
#include "ToolTip.h"

class CTradeChart;
#define CHART_PADDING (0.0f)

class CBaseChart : public CUIContainer
{
public:
    CBaseChart(UINT uGlobalEventID, CTradeChart& cChart);
    virtual ~CBaseChart();

    virtual VOID Initialize(VOID)=0;
    virtual VOID Release(VOID) OVERRIDE;

    virtual VOID SetupChart(VOID)=0;

    virtual VOID OnRender(FLOAT fOffsetX, FLOAT fOffsetY) const OVERRIDE;
    
    VOID OnDisplayToolTip(FLOAT& fY, BOOLEAN bShow, UINT uDisplayedCandleIndex);

    VOID UpdateTitleString(VOID);
protected:
    CTradeChart&        m_cMainChart;
    CUITextLabel        m_cLabel;
    FLOAT               m_fStartX;
    FLOAT               m_fCandleWidth;
    UINT                m_uIntervalNum;
    
    VOID RenderTimeLines(VOID) const;
private:
    BOOLEAN             m_bDisplaySelectedValue;
    FLOAT               m_fSelectedX;
    FLOAT               m_fSelectedY;
    CToolTip            m_cToolTip;

    virtual VOID GetTitleString(CHAR* szBuffer, UINT uBufferLen) const=0;
    virtual VOID GetToolTipString(CHAR* szBuffer, UINT uBufferLen, UINT uCandleIndexSelected) const=0;
    virtual FLOAT GetToolTipY(UINT uCandleIndexSelected) const=0;
};
#endif // #if !defined(BASE_CHART_H)
