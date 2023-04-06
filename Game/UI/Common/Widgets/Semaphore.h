#if !defined(SEMAPHORE_H)
#define SEMAPHORE_H

#include "UIWidget.h"
#include "UIImage.h"
#include "UITextLabel.h"

class CSemaPhore : public CUIWidget
{
public:
    CSemaPhore();
    virtual ~CSemaPhore();

    VOID Initialize(const CHAR* szString, BOOLEAN bSmallerCircle);
    VOID SetZ(FLOAT fZ) { m_fZ = fZ; }
    FLOAT GetZ(VOID) const { return m_fZ; }
    virtual VOID Release(VOID) OVERRIDE;
    
    virtual VOID OnRender(FLOAT fOffsetX, FLOAT fOffsetY) const OVERRIDE;
    virtual VOID SetColor(UINT uColor) OVERRIDE;
    virtual VOID SetScale(FLOAT fScale) OVERRIDE;
private:
    CUIImage        m_cBG;
    CUITextLabel    m_cTxt;
    FLOAT           m_fZ;
    
    virtual VOID OnWorldCoordinatesUpdated(VOID) OVERRIDE;
};
#endif // #if !defined(SEMAPHORE_H)
