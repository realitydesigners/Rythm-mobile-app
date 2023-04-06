#if !defined(SR_INNER_ZZ_H)
#define SR_INNER_ZZ_H
#include "RenkoDef.h"
#include "SRBaseGraph.h"
#include "UIButton.h"
#include "UIImage.h"
#include "UITextLabel.h"
#include "ZZUtil.h"

class CMegaZZ;
#define SR_INNER_ZZ_PADDING (4.0f)
#define SR_INNER_ZZ_HEIGHT_OFFSET (60.0f)
class CSRInnerZZ : public CSRBaseGraph
{
public:
    CSRInnerZZ();
    virtual ~CSRInnerZZ();
    
    virtual BOOLEAN Initialize(UINT uBoxNum) OVERRIDE;
    
    virtual VOID PrepareChart(const CMegaZZ& cMegaZZ, UINT uDepth) OVERRIDE;
    virtual VOID ClearChart(VOID) OVERRIDE;
    
    virtual VOID Release(VOID) OVERRIDE;
    
    virtual BOOLEAN OnTouchBegin(FLOAT fX, FLOAT fY) OVERRIDE;
    virtual BOOLEAN OnTouchMove(FLOAT fX, FLOAT fY) OVERRIDE;
    virtual BOOLEAN OnTouchEnd(FLOAT fX, FLOAT fY) OVERRIDE;
    
    virtual VOID OnRender(FLOAT fOffsetX, FLOAT fOffsetY) const OVERRIDE;
private:
    enum EBTN_ID {
        EBTN_SNAP_LATEST=0,
        EBTN_CHANGE_LINE_NUM,
        EBTN_X_TOGGLE,
        EEVENT_ON_TEXT_REPLY,
    };
    struct SR_ZZ {
        FLOAT fX;
        FLOAT fY;
        FLOAT fSize;
        UINT  uZZLevel;
        UINT  uColor;
    };

    struct SR_Area {
        FLOAT fX1;
        FLOAT fY1;
        FLOAT fX2;
        FLOAT fY2;
        FLOAT fY3;
        UINT uColor;
    };
    struct SR_Line {
        FLOAT fX1;
        FLOAT fY1;
        FLOAT fX2;
        FLOAT fY2;
        FLOAT fTangentX;
        FLOAT fTangentY;
        UINT uColor;
    };
    CUITextLabel            m_cLineIndicator;
    CUIImage                m_cBtnBG;
    CUIButton               m_cLineBtn;
    const CMegaZZ*          m_pcMegaZZ;
    UINT                    m_uDepth;
    SR_ZZ*                  m_psZZs;
    UINT                    m_uMaxZZ;
    UINT                    m_uUsedZZ;
    SR_Area*                m_psAreas;
    UINT                    m_uMaxArea;
    UINT                    m_uUsedArea;
    SR_Line*                m_psLines;
    UINT                    m_uMaxLine;
    UINT                    m_uUsedLine;
    
    FLOAT                   m_fPrevTouchX;
    BOOLEAN                 m_bTouched;
    FLOAT                   m_fXOffset;
    
    UINT                    m_uHorizontalLineNum;       // number of horizontal lines to draw
    FLOAT                   m_fHorizontalLineOffset;    // offset Y
    
    BOOLEAN                 m_bHideUI;
    
    // working data for calculations
    ZZUtil::SFloatValue**   m_ppsZZFloatValueList;
    ZZUtil::SFloatValue*    m_psZZFloatValues;
    UINT                    m_uZZValueNum;
    
    CHAR                    m_szBuffer[64];

    VOID                    AddZZ(FLOAT fX, FLOAT fY, FLOAT fSize, UINT uZZLevel, UINT uColor);
    VOID                    AddArea(FLOAT fX1, FLOAT fY1, FLOAT fX2, FLOAT fY2, FLOAT fY3, UINT uColor);
    VOID                    AddLine(FLOAT fX1, FLOAT fY1, FLOAT fX2, FLOAT fY2, UINT uColor, BOOLEAN bTimeLine);

    VOID                    UpdateIndicator(FLOAT fChannelSize);
    VOID                    UpdateToggleXLabel(VOID);
    
    BOOLEAN                 ResizeWorkBuffer(UINT uNum);
    BOOLEAN                 GenerateZZ(UINT uNum, const CMegaZZ& cMegaZZ, UINT uDepth);
    
    virtual VOID            OnReceiveEvent(CEvent& cEvent) OVERRIDE;
    
    VOID OnPressChangeLineBtn(VOID);
    VOID OnUserInputReply(VOID);
    
    VOID SetHorizontalLineNum(UINT uNum);
};

#endif // #if !defined(SR_INNER_ZZ_H)
