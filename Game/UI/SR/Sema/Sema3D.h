#if !defined(SEMA_3D_H)
#define SEMA_3D_H
#include "MegaZZ.h"
#include "RenkoDef.h"
#include "Semaphore.h"
#include "SRBaseGraph.h"
#include "UIButton.h"
#include "UIImage.h"
#include "UITextLabel.h"

#define REMOVE_COLOR_BTNS

class CMegaZZ;
class CSema3D : public CSRBaseGraph
{
public:
    CSema3D();
    virtual ~CSema3D();
    
    virtual BOOLEAN Initialize(UINT uBoxNum) OVERRIDE;
    virtual VOID PrepareChart(const CMegaZZ& cMegaZZ, UINT uDepth) OVERRIDE;
    virtual VOID ClearChart(VOID) OVERRIDE;
    
    virtual VOID Release(VOID) OVERRIDE;
    
    virtual VOID OnRender(FLOAT fOffsetX, FLOAT fOffsetY) const OVERRIDE;
    virtual VOID OnUpdate(FLOAT fLapsed) OVERRIDE;
    
    virtual BOOLEAN OnTouchBegin(FLOAT fX, FLOAT fY) OVERRIDE;
    virtual BOOLEAN OnTouchMove(FLOAT fX, FLOAT fY) OVERRIDE;
    virtual BOOLEAN OnTouchEnd(FLOAT fX, FLOAT fY) OVERRIDE;

private:
    enum EBTN_ID {
        EBTN_TOGGLE_SIDE_FLIP=0,
        EBTN_SNAP_LATEST,
        EBTN_TOGGLE_MAXIMIZE,
        EBTN_TOGGLE_SEMA,
        EBTN_TOGGLE_FLATTEN,
        EBTN_TOGGLE_ZZ_LINE,
        EBTN_TOGGLE_HISTORY,
        EBTN_TOGGLE_HIDE,
        EBTN_TOGGLE_NEW_DISPLAY,
        EBTN_TOGGLE_RETRACE,
        
        EBTN_DEPTH_09,
        EBTN_DEPTH_08,
        EBTN_DEPTH_07,
        EBTN_DEPTH_06,
        EBTN_DEPTH_05,
        EBTN_DEPTH_04,
        EBTN_DEPTH_03,
        EBTN_DEPTH_02,
        EBTN_DEPTH_01,
        
        EBTN_COLOR_09,
        EBTN_COLOR_08,
        EBTN_COLOR_07,
        EBTN_COLOR_06,
        EBTN_COLOR_05,
        EBTN_COLOR_04,
        EBTN_COLOR_03,
        EBTN_COLOR_02,
        EBTN_COLOR_01,
        
        EBTN_CHANGE_BG_COLOR,
        
        EEVENT_CHANGE_COLOR_09,
        EEVENT_CHANGE_COLOR_08,
        EEVENT_CHANGE_COLOR_07,
        EEVENT_CHANGE_COLOR_06,
        EEVENT_CHANGE_COLOR_05,
        EEVENT_CHANGE_COLOR_04,
        EEVENT_CHANGE_COLOR_03,
        EEVENT_CHANGE_COLOR_02,
        EEVENT_CHANGE_COLOR_01,
        
        EEVENT_CHANGE_BG_COLOR,
    };
    enum {
        POLYGON_NUM = 8000,
        LINE_NUM = 8000,
        SEMAPHORE_NUM = 2500,
        CHART_PAD_PIXEL_NUM = 10,
        CHANNEL_ZZ_DATA_NUM = 10000,
        Z_THICKNESS = 20,
    };
    struct SChannelZZData {
        FLOAT fX;
        FLOAT fPriceY;
        UINT uTimeStamp;
        BYTE byZZLevel;
    };
    struct SChannelData {
        FLOAT   afX[MEGAZZ_MAX_DEPTH];
        FLOAT   afTop[MEGAZZ_MAX_DEPTH];
        FLOAT   afBottom[MEGAZZ_MAX_DEPTH];
        BOOLEAN abUp[MEGAZZ_MAX_DEPTH];
        BOOLEAN abDrawLine[MEGAZZ_MAX_DEPTH];
        BOOLEAN abWasFlipped[MEGAZZ_MAX_DEPTH];
        FLOAT   afPrevX[MEGAZZ_MAX_DEPTH];
        FLOAT   afMidPtX[MEGAZZ_MAX_DEPTH];
        FLOAT   afPrevTop[MEGAZZ_MAX_DEPTH];
        FLOAT   afPrevBottom[MEGAZZ_MAX_DEPTH];
    };

    struct SPolygon2D {
        SHLVector2D sTopLeft;
        SHLVector2D sTopRight;
        SHLVector2D sBottomLeft;
        SHLVector2D sBottomRight;
        UINT        uColor;
        UINT        uDepth;
    };
    struct SLine {
        SHLVector2D sPt1;
        SHLVector2D sPt2;
        UINT uColor;
    };
    const CMegaZZ*          m_pcMegaZZ;
    BOOLEAN                 m_bInitialized;                 // for storing original pos/size
    BOOLEAN                 m_bMaximized;                   // whether we are in maximized mode
    BOOLEAN                 m_bDisplaySemaphore;            // display sema or not
    BOOLEAN                 m_bAllowFlatline;               // allow D1 line to be drawn
    BOOLEAN                 m_bFlattenMode;                 // whether we are in flatten mode
    BOOLEAN                 m_bDisplayHistory;              // whether we display history or not
    BOOLEAN                 m_bHideUI;                      // hide ui?
    BOOLEAN                 m_bNewMethod;                   // draw with new method?
    BOOLEAN                 m_bDrawRetraceBox;              // draw retracebox?
    BOOLEAN                 m_abDepthEnabled[MEGAZZ_MAX_DEPTH];    // are the depths enabled
    UINT                    m_auSelectableDepthColors[MEGAZZ_MAX_DEPTH];
    UINT                    m_auLineDepthColors[MEGAZZ_MAX_DEPTH];
    SHLVector2D             m_sOriginalPos;
    SHLVector2D             m_sOriginalSize;
    CUIImage                m_cBG;          // used in full screen mode
    CUIImage                m_cBtnBG;
    CUITextLabel            m_cMaximizeTxt;
    CUIButton               m_cMaximizeBtn;
    CUITextLabel*           m_apcDepthtxt[MEGAZZ_MAX_DEPTH];
    CUIButton               m_acDepthBtn[MEGAZZ_MAX_DEPTH];
    CUITextLabel            m_cSemaphoreTxt;
    CUIButton               m_cSemaphoreBtn;
    CUITextLabel            m_cFlatTxt;
    CUIButton               m_cFlatBtn;
    CUITextLabel            m_cZZLineTxt;
    CUIButton               m_cZZLineBtn;
    CUITextLabel            m_cHistoryTxt;
    CUIButton               m_cHistoryBtn;
    CUITextLabel            m_cDisplayToggleTxt;
    CUIButton               m_cDisplayToggleBtn;
    CUITextLabel            m_cRetraceToggleTxt;
    CUIButton               m_cRetraceToggleBtn;
    CUITextLabel            m_cBGColorTxt;
    CUIButton               m_cBGBtn;
    
#if !defined(REMOVE_COLOR_BTNS)
    CUITextLabel*           m_apcColortxt[MEGAZZ_MAX_DEPTH];
    CUIButton               m_acColorBtn[MEGAZZ_MAX_DEPTH];
#endif // #if !defined(REMOVE_COLOR_BTNS)
    
    SHLVector2D             m_sScrollOffset;
    // rendering related variables
    SPolygon2D              m_asPoly2D[POLYGON_NUM];
    UINT                    m_uUsedPoly2D;
    SLine                   m_asLines[LINE_NUM];
    UINT                    m_uUsedLineNum;
    CSemaPhore              m_acSemaphores[SEMAPHORE_NUM];
    UINT                    m_uUsedSemaNum;
    
    CUITextLabel*           m_apcChannelTxt[MEGAZZ_MAX_DEPTH];
    
    BOOLEAN                 m_bGameUITapped;
    FLOAT                   m_fPrevX;
    FLOAT                   m_fPrevY;
    
    // calculation related variables
    FLOAT                   m_fPixelPerPrice;
    FLOAT                   m_fReferenceValue; // (x or y)
    FLOAT                   m_fReferencePrice; // reference price
    FLOAT                   m_afChannelPixelSize[MEGAZZ_MAX_DEPTH];
    SChannelZZData          m_asChannelZZ[CHANNEL_ZZ_DATA_NUM];
    ZZUtil::SFloatValue*    m_ppsZZs[CHANNEL_ZZ_DATA_NUM];
    UINT                    m_uUsedChannelZZ;
    UINT                    m_uBiggestChannelIndex;
    
    // culling box
    FLOAT                   m_fCullTopLeftX;
    FLOAT                   m_fCullTopLeftY;
    FLOAT                   m_fCullBottomRightX;
    FLOAT                   m_fCullBottomRightY;

    VOID UpdateCullingBox(VOID);
    VOID UpdateToggleMaxMinBtn(VOID);
    VOID UpdateDepthLabel(VOID);
    
    virtual VOID OnReceiveEvent(CEvent& cEvent) OVERRIDE;
    
    VOID OnTapSnapLatest(VOID);
    VOID OnTapToggleSemaBtn(VOID);
    VOID OnTapToggleMaximizeBtn(VOID);
    VOID OnTapFlattenBtn(VOID);
    VOID OnTapDepthBtn(UINT uIndex);
    VOID OnTapToggleZZLineBtn(VOID);
    VOID OnTapToggleHistoryBtn(VOID);
    VOID OnTapColorBtn(UINT uIndex);
    VOID OnTapToggleHideBtn(VOID);
    VOID OnTapToggleNewDisplayBtn(VOID);
    VOID OnTapRetraceBtn(VOID);
    VOID OnTapChangeBGColor(VOID);
    VOID UpdateDepthBtnPos(VOID);
    VOID UpdateColorBtnColors(VOID);
    VOID OnChangedColor(UINT uIndex, UINT uColor);
    VOID OnChangedBGColor(UINT uColor);
    
    VOID ReGenerateZZ(VOID);
    VOID GenerateZZ(const CMegaZZ& cSR); // generate the ZZs
    VOID PrepareZZ(const CMegaZZ& cSR);  // prepare the chart
    // generate all the things needed for rendering
    VOID GenerateRenderables(VOID);
    
    VOID InitializeChannelData(SChannelData& sChannel);
    UINT ProcessChannelData(SChannelData& sChannel);
    
    VOID ProcessHistory(FLOAT fZZLineStart, UINT uDrawBoxIndex, UINT uLargestChannelIndex, UINT uLastDataAccessIndex, SChannelData& sChannelData);
    VOID ProcessBeforeEndChannelData(FLOAT fZZLineStart, const SChannelData& sChannel);
    VOID ProcessProtudingZZLine(FLOAT fZZLineStart, UINT uLastDataAccessIndex);
    VOID ProcessEndChannelData(FLOAT fZZLineStart, const SChannelData& sChannel);
    
    VOID ProcessNewEndChannelData(FLOAT fZZLineStart, FLOAT fDistAdjusted, UINT uLastDataAccessIndex, const SChannelData& sChannel);
    VOID UpdateChannelLabels(const SChannelData& sChannel);
    
    VOID AddPolygonAsBox(FLOAT fTopLeftX, FLOAT fTopLeftY, FLOAT fBottomRightX, FLOAT fBottomRightY, FLOAT fZ, UINT uColor, UINT uDepth);
    VOID AddPolygon(FLOAT fTopLeftX, FLOAT fTopLeftY, FLOAT fTopRightX, FLOAT fTopRightY, FLOAT fBottomLeftX, FLOAT fBottomLeftY, FLOAT fBottomRightX, FLOAT fBottomRightY, FLOAT fZ, UINT uColor, UINT uDepth);
    VOID AddLine(FLOAT fX1, FLOAT fY1, FLOAT fZ1, FLOAT fX2, FLOAT fY2, FLOAT fZ2, UINT uColor);
    VOID AddSemaphore(FLOAT fX, FLOAT fY, FLOAT fZ, UINT uLevel, BOOLEAN bUp, UINT uColor);
    
    VOID RenderPolygons(FLOAT fOffsetX, FLOAT fOffsetY) const;
    VOID RenderLines(FLOAT fOffsetX, FLOAT fOffsetY) const;
    VOID RenderSemaphores(FLOAT fOffsetX, FLOAT fOffsetY) const;
    VOID RenderChannelLabels(FLOAT fOffsetX, FLOAT fOffsetY) const;
    
    VOID Transform(SHLVector2D& sResult, FLOAT fX, FLOAT fY, FLOAT fZ) const;
    VOID UnTransform(SHLVector2D& sResult, FLOAT fUIX, FLOAT fUIY) const;
    VOID AddPolygon2D(const SHLVector2D& sTopLeft, const SHLVector2D& sTopRight, const SHLVector2D& sBottomLeft, const SHLVector2D& sBottomRight, UINT uColor, UINT uDepth);
    
    // experimental code
    VOID RenderBoxText(FLOAT fOffsetX, FLOAT fOffsetY) const;
    
    VOID InitExperiment(VOID);
    VOID ReleaseExperiment(VOID);
    VOID ClearExperiment(VOID);
    
    VOID StartExperiment(const SChannelData& sChannel, FLOAT fNowX);
    VOID ProcessExperiment(const SChannelData& sChannel, FLOAT fNowPriceX, FLOAT fNowPriceY);
    VOID EndExperiment(FLOAT fEndX, FLOAT fNowPriceY, const SChannelData& sChannel);
};

#endif // #if !defined(SEMA_3D_H)
