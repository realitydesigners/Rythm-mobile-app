#if !defined(TP_CHART_CHECK_H)
#define TP_CHART_CHECK_H

#include "GameWindow.h"
#include "MultiCellTable.h"

class CTPChartCheck : public CGameWindow
{
public:
    enum EBTN_ID {
        EBTN_QUIT=0,
        EBTN_SELECT,
    };
    CTPChartCheck();
    virtual ~CTPChartCheck();
    
    virtual VOID InitializeInternals(VOID) OVERRIDE;
    virtual VOID Release(VOID) OVERRIDE;
private:
    CMultiCellTable         m_cList;
    
    virtual VOID OnReceiveEvent(CEvent& cEvent) OVERRIDE;
    
    VOID OnSelectedIndex(UINT uIndex);
};

#endif // #if !defined(TP_CHART_CHECK_H)
