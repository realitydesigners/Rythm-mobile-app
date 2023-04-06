#if !defined(NEW_BIB_BOX_H)
#define NEW_BIB_BOX_H
#include "Bib2.h"
#include "DepthDisplay.h"
#include "UIContainer.h"
#include "UITextLabel.h"
#include "RythmDashboardWidget.h"
#include "TradeLogDefs.h"

class CMegaZZ;

#define NEW_BIB_BOX_HEIGHT_OFFSET (60.0f)
#define HIGHER_DEPTH_THICKNESS (30.0f)

class CNewBibBox : public CUIContainer
{
public:
    CNewBibBox();
    virtual ~CNewBibBox();
    
    BOOLEAN Initialize(VOID);

    VOID PrepareChart(const STradeLog& sLog, BOOLEAN bOpen);
    VOID PrepareChart(const CMegaZZ& cMegaZZ);
    VOID ClearChart(VOID);
    
    virtual VOID Release(VOID) OVERRIDE;
    virtual VOID PostEvent(CEvent& cEvent, FLOAT fDelay) OVERRIDE;
    virtual VOID OnRender(FLOAT fOffsetX, FLOAT fOffsetY) const OVERRIDE;
private:
    enum EBTN_ID {
        EBTN_OUTER_DEPTH_INDEX=0,
        EBTN_OUTER_DEPTH_NUM,
        EBTN_INNER_DEPTH_INDEX,
        EBTN_INNER_DEPTH_NUM,
        EEVENT_OUTER_DEPTH_INDEX_REPLY,
        EEVENT_OUTER_DEPTH_NUM_REPLY,
        
        EEVENT_INNER_DEPTH_INDEX_REPLY,
        EEVENT_INNER_DEPTH_NUM_REPLY,
    };
    enum {
        EBOX_NUM = 100,
        ELINE_NUM = 100,
    };
    struct SBox2D {
        SHLVector2D sPos;
        SHLVector2D sSize;
        UINT uColor;
    };
    struct SLine2D {
        SHLVector2D sPt1;
        SHLVector2D sPt2;
        UINT uColor;
    };
    SBox2D                  m_asBox[EBOX_NUM];
    UINT                    m_uUsedBoxNum;
    SLine2D                 m_asLine[ELINE_NUM];
    UINT                    m_uUsedLineNum;
    
    const CMegaZZ*          m_pcMegaZZ;
    STradeLog               m_sTmp;
    const STradeLog*        m_psTradelog;
    BOOLEAN                 m_bTradeLogOpen;
    CBib2                   m_cBibWidget;
    CDepthDisplay           m_cDepthDisplay;
    CRythmDashboardWidget   m_cDepthValues;
    CUITextLabel*           m_apcDepthhLbls[MEGAZZ_MAX_DEPTH];
    CUIImage                m_cBtnImg;
    CUITextLabel            m_cOuterDepthIndex;
    CUITextLabel            m_cOuterDepthNum;
    CUITextLabel            m_cInnerDepthIndex;
    CUITextLabel            m_cInnerDepthNum;
    CUIButton               m_cOuterDepthIndexBtn;
    CUIButton               m_cOuterDepthNumBtn;
    CUIButton               m_cInnerDepthIndexBtn;
    CUIButton               m_cInnerDepthNumBtn;
    UINT                    m_uOuterDepthIndex;
    UINT                    m_uOuterDepthNum;
    
    VOID AddBox(FLOAT fX, FLOAT fY, FLOAT fW, FLOAT fH, UINT uColor);
    VOID AddLine(FLOAT fX1, FLOAT fY1, FLOAT fX2, FLOAT fY2, UINT uColor);
    
    VOID UpdateOuterLabels(VOID);
    VOID UpdateInnerLabels(VOID);
    
    virtual VOID OnReceiveEvent(CEvent& cEvent) OVERRIDE;
    
    VOID OnTapOuterDepthIndex(VOID);
    VOID OnTapOuterDepthNum(VOID);
    
    VOID OnReplyOuterDepthIndex(UINT uDepthIndex);
    VOID OnReplyOuterDepthNum(UINT uDepthNum);
    
    VOID UpdateInnerBtnPositions(VOID);
    
    VOID OnTapInnerDepthIndex(VOID);
    VOID OnTapInnerDepthNum(VOID);
    
    VOID OnReplyInnerDepthIndex(UINT uDepthIndex);
    VOID OnReplyInnerDepthNum(UINT uSelectedIndex);
};



#endif // #if !defined(NEW_BIB_BOX_H)
