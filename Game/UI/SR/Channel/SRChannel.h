#if !defined(SR_GRAPH_NEW_H)
#define SR_GRAPH_NEW_H
#include "RenkoDef.h"
#include "SRBaseGraph.h"

class CShiftedRenko;
class CSRChannel : public CSRBaseGraph
{
public:
    CSRChannel();
    virtual ~CSRChannel();
    
    virtual BOOLEAN Initialize(UINT uBoxNum) OVERRIDE;
    
//    virtual VOID PrepareChart(const CShiftedRenko& cSR, UINT uDepth) OVERRIDE;
    virtual VOID ClearChart(VOID) OVERRIDE;
    
    virtual VOID Release(VOID) OVERRIDE;
    
    virtual BOOLEAN OnTouchBegin(FLOAT fX, FLOAT fY) OVERRIDE;
    virtual BOOLEAN OnTouchMove(FLOAT fX, FLOAT fY) OVERRIDE;
    virtual BOOLEAN OnTouchEnd(FLOAT fX, FLOAT fY) OVERRIDE;
    
    virtual VOID OnRender(FLOAT fOffsetX, FLOAT fOffsetY) const OVERRIDE;
private:
    struct SR_Square {
        FLOAT fX;
        FLOAT fY;
        FLOAT fW;
        FLOAT fH;
        UINT uColor;
    };
    struct SR_Area {
        FLOAT fX1;
        FLOAT fY1;
        FLOAT fX2;
        FLOAT fY2;
        FLOAT fY3;
        FLOAT fY4;
        UINT uColor;
    };
    const CShiftedRenko*    m_pcSR;
    UINT                    m_uDepth;
    SR_Square*              m_psSquares;
    UINT                    m_uMax;
    UINT                    m_uUsed;
    SR_Area*                m_psAreas;
    UINT                    m_uAreaMax;
    UINT                    m_uAreaUsed;
    
    UINT                    m_uBlockNum;
    UINT                    m_uMaxdisplayNum;
    UINT                    m_uHistoryNum;      // total number of horizontal boxes
    UINT                    m_uXOffset;         // current offset to start rendering
    UINT                    m_uMaxXOffset;      // max offset allowed;
    FLOAT                   m_fBoxPixel;        // size of a box to render
    INT                     m_nMaxYOffset;
    INT                     m_nMinYOffset;
    INT                     m_nYOffset;

    FLOAT                   m_fPrevTouchX;
    FLOAT                   m_fPrevTouchY;
    BOOLEAN                 m_bTouched;
    BOOLEAN                 m_bScrollMode;

    virtual VOID            OnReceiveEvent(CEvent& cEvent) OVERRIDE;

    VOID                    AddBox(FLOAT fX, FLOAT fY, FLOAT fW, FLOAT fH, UINT uColor);
    VOID                    AddArea(FLOAT fX1, FLOAT fY1, FLOAT fX2, FLOAT fY2, FLOAT fY3, FLOAT fY4, UINT uColor);
    VOID                    UpdateIndicator(VOID);
};

#endif // #if !defined(SR_GRAPH_NEW_H)
