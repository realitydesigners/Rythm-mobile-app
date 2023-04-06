#if !defined(BIB_2_H)
#define BIB_2_H
#include "RenkoDef.h"
#include "UIContainer.h"
#include "MegaZZDef.h"

struct STradeLog;
class CMegaZZ;
class CBib2 : public CUIContainer
{
public:
    CBib2();
    virtual ~CBib2();
    
    VOID Initialize(VOID);
    VOID ChangeMode(BOOLEAN bSquareMode, BOOLEAN bFlatMode);
    VOID PrepareChart(const STradeLog& sLog, BOOLEAN bOpen);
    VOID PrepareChart(const CMegaZZ& cMegaZZ);
    VOID ClearChart(VOID);
    
    VOID SetDepthIndex(UINT uDepthIndex)    { m_uDepthIndex = uDepthIndex; }
    VOID SetDepthNum(UINT uDepthNum)        { m_uDepthNum = uDepthNum; }
    VOID SetSquareMode(BOOLEAN bSquareMode) { m_bSquareMode = bSquareMode; }
    VOID SetFlatMode(BOOLEAN bFlatMode)     { m_bFlatMode = bFlatMode; }
    VOID SetHideX(BOOLEAN bHide)            { m_bHideX = bHide; }
    VOID SetSmallestChannelWhite(BOOLEAN bWhite) { m_bSmallestChannelWhite = bWhite; }
    VOID SetHideBars(BOOLEAN bHide)         { m_bHideBars = bHide; }
    
    UINT GetDepthIndex(VOID) const      { return m_uDepthIndex; }
    UINT GetDepthNum(VOID) const        { return m_uDepthNum; }
    BOOLEAN IsSquareMode(VOID) const    { return m_bSquareMode; }
    BOOLEAN IsflatMode(VOID) const      { return m_bFlatMode; }
    BOOLEAN IsHideX(VOID) const         { return m_bHideX; }
    
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
    BOOLEAN         m_bSquareMode; // draw square or diamond?
    BOOLEAN         m_bFlatMode;   // remove the 3d layer?
    BOOLEAN         m_bHideX;      // hide X?
    BOOLEAN         m_bHideBars;
    BOOLEAN         m_bSmallestChannelWhite;
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
    
    VOID Prepare(const MegaZZ::SChannel** ppsChannels, const FLOAT* pfSizes, FLOAT fCurrentPrice);
};

#endif // #if !defined(BIB_2_H)
