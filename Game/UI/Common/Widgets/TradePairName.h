#if !defined(TRADE_PAIR_NAME_H)
#define TRADE_PAIR_NAME_H

#include "CryptoUtil.h"
#include "UIWidget.h"
#include "UITextLabel.h"

class CTradePairName : public CUIWidget
{
public:
    CTradePairName();
    virtual ~CTradePairName();

    VOID SetName(ESOURCE_MARKET eSource, UINT uTradePairIndex);
    
    VOID SetSelected(BOOLEAN bSelected);
    
    virtual VOID Release(VOID) OVERRIDE;
    virtual VOID OnRender(FLOAT fOffsetX, FLOAT fOffsetY) const OVERRIDE;
private:
    CUITextLabel    m_cName1;
    CUITextLabel    m_cName2;
    
    VOID RecalculateNamePosition(VOID);
    virtual VOID OnWorldCoordinatesUpdated(VOID) OVERRIDE;
};
#endif // #if !defined(TRADE_PAIR_NAME_H)
