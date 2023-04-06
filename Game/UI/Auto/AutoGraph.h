#if !defined(AUTO_GRAPH_H)
#define AUTO_GRAPH_H

#include "DepthDisplay.h"
#include "MegaZZDef.h"
#include "PatternData.h"
#include "TradeLogDefs.h"
#include "UIButton.h"
#include "UIContainer.h"
#include "UIImage.h"
#include "UITextLabel.h"

#define AUTO_GRAPH_WIDTH  (250.0f)
#define AUTO_GRAPH_HEIGHT (280.0f)

class CAutoGraph : public CUIContainer
{
public:
    CAutoGraph(UINT uStartDepthIndex, UINT uDepthNum);
    virtual ~CAutoGraph();
    
    VOID Initialize(BOOLEAN bOverrideSize = FALSE);
    VOID Refresh(UINT uPatternIndex);
    VOID Refresh(const SPatternRow* psRows, UINT uRowNum);
    VOID Clear(VOID);
    VOID ToggleGrid(VOID);
    VOID ToggleTransparency(VOID);
    
    virtual VOID Release(VOID) OVERRIDE;
    
    virtual VOID OnRender(FLOAT fOffsetX, FLOAT fOffsetY) const OVERRIDE;
    
    virtual VOID PostEvent(CEvent& cEvent, FLOAT fDelay) OVERRIDE;
private:
    enum EBTN_ID {
        EBTN_CHANGE_DEPTH_INDEX=0,
        EBTN_CHANGE_DEPTH_NUM,
        EEVENT_USER_REPLY_DEPTH_INDEX,
        EEVENT_USER_REPLY_DEPTH_NUM,
    };
    enum  {
        EBOX_NUM = 300,
        ELINE_NUM = 600,
    };
    struct SBox {
        FLOAT fX;
        FLOAT fY;
        FLOAT fW;
        FLOAT fH;
        UINT uColor;
        UINT uDepthIndex;
    };
    struct SLine {
        FLOAT fX1;
        FLOAT fY1;
        FLOAT fX2;
        FLOAT fY2;
        UINT uColor;
        UINT uDepthIndex;
    };
    
    UINT    m_uStartDepthIndex;
    UINT    m_uDepthNum;
    UINT    m_uBoxNum;
    UINT    m_uLineNum;
    SBox    m_asBox[EBOX_NUM];
    SLine   m_asLine[ELINE_NUM];
    
    BOOLEAN m_bDrawGrid;
    BOOLEAN m_bTransparent;
    BOOLEAN m_abEnabled[MEGAZZ_MAX_DEPTH];
    
    CUIImage        m_cBtnBG;
    CUITextLabel    m_cDepthIndexTxt;
    CUITextLabel    m_cDepthNumTxt;
    CUIButton       m_cDepthBtn;
    CUIButton       m_cDepthNumBtn;
    CDepthDisplay   m_cDisplay;
    
    const SPatternRow*              m_psPatternRows;
    UINT                            m_uPatternRowNum;
    FLOAT                           m_afSizes[MEGAZZ_MAX_DEPTH];
    
    virtual VOID OnReceiveEvent(CEvent& cEvent) OVERRIDE;
    
    VOID AddBox(FLOAT fX, FLOAT fY, FLOAT fW, FLOAT fH, UINT uColor, UINT uDepthIndex);
    VOID AddLine(FLOAT fX1, FLOAT fY1, FLOAT fX2, FLOAT fY2, UINT uColor, UINT uDepthIndex);
    
    VOID RenderBoxes(FLOAT fOffsetX, FLOAT fOffsetY, UINT uDepthIndex) const;
    VOID RenderLines(FLOAT fOffsetX, FLOAT fOffsetY, UINT uDepthIndex) const;
    
    VOID PrepareDepth(UINT uDepthIndex, FLOAT fStartX, FLOAT fStartY, FLOAT fEndX, FLOAT fEndY, UINT uColor, UINT uGridColor);
    
    BOOLEAN GetChannelInfo(const SPatternRow* psRow, UINT uRowNum, UINT uDepthIndex, BYTE& byChannelType, BYTE& byCurrentNumber1, BYTE& byCurrentNumber2, BYTE& byR1, BYTE& byR2, EPATTERN_MATCH& eMatch, UINT& uMatchDepthIndex);
    
    VOID OnTapDepthIndexBtn(VOID);
    VOID OnTapDepthNumBtn(VOID);
    
    VOID OnUserReplyDepthIndex(UINT uSelectedIndex);
    VOID OnUserReplyNum(UINT uSelectedIndex);
    
    VOID UpdateDepthLabels(VOID);
    VOID UpdateDepthDisplay(VOID);
    
    VOID Refresh(const SPattern* psPattern);
};

#endif // #if !defined(AUTO_GRAPH_H)
