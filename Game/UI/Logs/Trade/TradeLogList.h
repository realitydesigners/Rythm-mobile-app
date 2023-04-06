#if !defined(TRADE_LOG_LIST_H)
#define TRADE_LOG_LIST_H

#include "UIContainer.h"
#include "UIScroller.h"

class CTradeLogWidget;
class CTradeLogList : public CUIContainer
{
public:
    CTradeLogList();
    virtual ~CTradeLogList();
    
    VOID Initialize(UINT uEventID);
    virtual VOID Release(VOID) OVERRIDE;

    BOOLEAN SelectWidget(const CTradeLogWidget& cWidget);
private:
    CUIScroller m_cScroller;
};

#endif // #if !defined(TRADE_LOG_LIST_H)
