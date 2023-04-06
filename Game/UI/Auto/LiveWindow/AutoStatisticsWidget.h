#if !defined(AUTO_STATISTICS_WIDGET_H)
#define AUTO_STATISTICS_WIDGET_H
#include "AutoStatsDef.h"

#include "UIButton.h"
#include "UIImage.h"
#include "UITextLabel.h"

#define STATISTICS_NAME_X           (5.0f)
#define STATISTICS_LONG_X           (STATISTICS_NAME_X + 190.0f)
#define STATISTICS_TARGET_X         (STATISTICS_LONG_X + 30.0f)
#define STATISTICS_STOPLOSS_X       (STATISTICS_TARGET_X + 30.0f)
#define STATISTICS_TAKE_PROFIT_X    (STATISTICS_STOPLOSS_X + 30.0f)
#define STATISTICS_PIP_X            (STATISTICS_TAKE_PROFIT_X + 40.0f)

#define STATISTICS_WIDTH            (STATISTICS_PIP_X + 30.0f)
#define STATISTICS_HEIGHT           (20.0f)

class CAutoStatisticsWidget : public CUIButton
{
public:
    CAutoStatisticsWidget(UINT uIndex, const SFilteredData& sData);
    virtual ~CAutoStatisticsWidget();
    
    VOID SetSelected(BOOLEAN bSelected);
    VOID Initialize(UINT uEventID);
    UINT GetIndex(VOID) const { return m_uIndex; }
    
    virtual VOID Release(VOID) OVERRIDE;
private:
    const UINT              m_uIndex;
    SFilteredData           m_sData;
    CUIImage                m_cBG;
    CUITextLabel            m_cName;
    CUITextLabel            m_cLong;
    CUITextLabel            m_cTargetOpenedNum;
    CUITextLabel            m_cTakeProfitNum;
    CUITextLabel            m_cStopLossNum;
    CUITextLabel            m_cCumulativePip;
    
    virtual BOOLEAN CreateButtonEvent(CEvent& cEvent) OVERRIDE;
};

#endif // #if !AUTO_STATISTICS_WIDGET_H
