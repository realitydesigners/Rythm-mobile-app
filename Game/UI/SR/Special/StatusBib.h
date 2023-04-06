#if !defined(STATUS_BIB_H)
#define STATUS_BIB_H
#include "RenkoDef.h"
#include "UIContainer.h"
#include "MegaZZDef.h"

class CMegaZZ;
class CStatusBib : public CUIContainer
{
public:
    CStatusBib();
    virtual ~CStatusBib();
    
    VOID Initialize(VOID);
    VOID PrepareChart(const CMegaZZ& cMegaZZ);
    VOID ClearChart(VOID);
    
    VOID SetDepthIndex(UINT uDepthIndex)    { m_uDepthIndex = uDepthIndex; }
    VOID SetDepthNum(UINT uDepthNum)        { m_uDepthNum = uDepthNum; }
    
    UINT GetDepthIndex(VOID) const      { return m_uDepthIndex; }
    UINT GetDepthNum(VOID) const        { return m_uDepthNum; }
    
    virtual VOID Release(VOID) OVERRIDE;
    
    virtual VOID OnRender(FLOAT fOffsetX, FLOAT fOffsetY) const OVERRIDE;
private:
    enum {
        POLY2D_MAX_NUM = 50,
        LINE2D_MAX_NUM = 100,
    };
    struct SPoly2D {
        FLOAT afPos[8];
        UINT  uColor;
        UINT  uDepthIndex;
    };
    struct SLine2D {
        SHLVector2D sPt1;
        SHLVector2D sPt2;
        SHLVector2D sTangents;
        UINT        uColor;
        UINT        uDepthIndex;
    };

    UINT            m_uDepthIndex;
    UINT            m_uDepthNum;
    SPoly2D         m_asPoly2D[POLY2D_MAX_NUM];
    UINT            m_uPoly2DNum;
    SLine2D         m_asLine2D[LINE2D_MAX_NUM];
    UINT            m_uLine2DNum;
    
    VOID AddPoly2D(FLOAT fTopLeftX, FLOAT fTopLeftY,
                   FLOAT fTopRightX, FLOAT fTopRightY,
                   FLOAT fBottomLeftX, FLOAT fBottomLeftY,
                   FLOAT fBottomRightX, FLOAT fBottomRightY,
                   UINT uColor, UINT uDepthIndex);
    
    
    VOID AddLine2D(FLOAT fPt1X, FLOAT fPt1Y, FLOAT fPt2X, FLOAT fPt2Y, FLOAT fThickness, UINT uColor, UINT uDepthIndex);
    VOID RenderPoly(FLOAT fOffsetX, FLOAT fOffsetY, UINT uDepthIndex) const;
    VOID RenderLines(FLOAT fOffsetX, FLOAT fOffsetY, UINT uDepthIndex) const;
};

#endif // #if !defined(BIB_2_H)
