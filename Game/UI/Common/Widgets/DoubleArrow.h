#if !defined(DOUBLE_ARROW_H)
#define DOUBLE_ARROW_H
#include "UIWidget.h"

#define DOUBLE_ARROW_DIMENSION (50.0f)
class CDoubleArrow : public CUIWidget
{
public:
    CDoubleArrow();
    virtual ~CDoubleArrow();
    
    VOID Initialize(UINT uBGColor);
    VOID SetRotation(FLOAT fTop, FLOAT fBottom); // rotation,
    VOID SetEnabled(BOOLEAN bEnabled);
    
    virtual VOID OnRender(FLOAT fOffsetX, FLOAT fOffsetY) const OVERRIDE;
private:
    FLOAT   m_afRotation[2];
    UINT    m_auArrowColor[2];
    UINT    m_uBGColor;
    UINT    m_uOriginalColor;
    virtual VOID OnWorldCoordinatesUpdated(VOID) OVERRIDE;
};
#endif // #if !defined(DOUBLE_ARROW_H)
