#if !defined(OANDA_TRADE_LIST_H)
#define OANDA_TRADE_LIST_H

#include "OandaMgr.h"
#include "OUIDefs.h"
#include "UIContainer.h"
#include "UIScroller.h"

class COTradeList : public CUIContainer
{
public:
    COTradeList();
    virtual ~COTradeList();
    
    VOID Initialize(UINT uSelectedEventID, UINT uAccountIndex);
    virtual VOID Release(VOID) OVERRIDE;
    
    UINT GetTradePairIndex(UINT uTradeID) const;
    const COandaMgr::SOandaTrade* GetTrade(UINT uTradeID) const;
private:
    CUIScroller m_cScroller;
};

#endif // #if !defined(OANDA_TRADE_LIST_H)
