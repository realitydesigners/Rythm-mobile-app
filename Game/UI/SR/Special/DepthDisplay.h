#if !defined(DEPTH_DISPLAY_H)
#define DEPTH_DISPLAY_H

#include "MegaZZDef.h"
#include "UIContainer.h"
#include "UITextLabel.h"
class CMegaZZ;
class CDepthDisplay : public CUIContainer
{
public:
    CDepthDisplay();
    virtual ~CDepthDisplay();
    
    VOID UpdateLabels(const CMegaZZ& cZZ);
    VOID SetEnabled(UINT uDepthIndex, BOOLEAN bEnabled);
    VOID Initialize(const FLOAT* pfSizes, UINT uStartDepthIndex, UINT uDepthNum, BOOLEAN bShowPercent = FALSE);
    virtual VOID Release(VOID) OVERRIDE;
private:
    UINT m_uNum;
    UINT m_uDepthIndex;
    BOOLEAN       m_abEnabled[MEGAZZ_MAX_DEPTH];
    CUITextLabel* m_apcDepth[MEGAZZ_MAX_DEPTH];
    CUITextLabel* m_apcUnit[MEGAZZ_MAX_DEPTH];
};

#endif // #if !defined(DEPTH_DISPLAY_H)
