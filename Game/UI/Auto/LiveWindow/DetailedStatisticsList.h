#if !defined(DETAILED_STATISTICS_LIST_H)
#define DETAILED_STATISTICS_LIST_H

#include "UIButton.h"
#include "UIImage.h"
#include "UITextLabel.h"
#include "MultiCellTable.h"

#define TRADE_PAIR_NAME_LENGTH      (100.0f)
//#define PRE_TRADE_NAME_LENGTH       (120.0f)
#define TP_CHART_NAME_LENGTH        (80.0f)
#define SL_NAME_LENGTH              (80.0f)
#define PIP_GAIN_LENGTH             (40.0f)
#define HIGHEST_PIP_GAIN_LENGTH     (60.0f)
#define LOWEST_PIP_GAIN_LENGTH      HIGHEST_PIP_GAIN_LENGTH
#define CLOSED_LENGTH               (60.0f)

#define DETAILED_LIST_WIDTH (TRADE_PAIR_NAME_LENGTH + TP_CHART_NAME_LENGTH + SL_NAME_LENGTH + PIP_GAIN_LENGTH + HIGHEST_PIP_GAIN_LENGTH + LOWEST_PIP_GAIN_LENGTH + CLOSED_LENGTH)
struct STradeLog;
class CDetailedStatisticsList : public CUIContainer
{
public:
    CDetailedStatisticsList();
    virtual ~CDetailedStatisticsList();
    
    VOID Initialize(VOID);
    VOID RefreshList(UINT uLogicIndex);
    virtual VOID Release(VOID) OVERRIDE;
    
    virtual VOID PostEvent(CEvent& cEvent, FLOAT fDelay) OVERRIDE;
private:
    enum EBTN_ID {
        EBTN_UNUSED = 0,
        EBTN_CLOSE,
    };
    CUIImage                m_cBG;
    CUIImage                m_cExitBtnBG;
    CUITextLabel            m_cExitTxt;
    CUIButton               m_cExitBtn;
    CMultiCellTable         m_cTbl;

    VOID ClearList(VOID);
    VOID TryAddLog(const STradeLog& sLog);
    virtual VOID OnReceiveEvent(CEvent& cEvent) OVERRIDE;
};

#endif // #if !defined(DETAILED_STATISTICS_LIST_H)
