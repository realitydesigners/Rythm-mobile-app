#if !defined(MEGA_ZZ_H)
#define MEGA_ZZ_H
#include "CryptoUtil.h"
#include "MegaZZDef.h"
#include "ZZUtil.h"

class CMegaZZ {
public:
    CMegaZZ();
    ~CMegaZZ();
    
    VOID SetSource(ESOURCE_MARKET eSource, UINT uTradePairIndex);
    BOOLEAN Initlialize(UINT uDepth, UINT uBoxNum, const FLOAT* pfSizes);
    VOID Release(VOID);
    
    VOID SetupFirstPrice(UINT64 ullTimeMilliSec, FLOAT fPrice, BOOLEAN bUp);
    
    VOID Clear(VOID); // clear the chart and start over
    VOID RebuildZZs(VOID) { m_bZZGenerated = FALSE; } // rebuild the zz (due to changed zz setting)

    VOID UpdatePrice(UINT64 ullTimeMilliSec, FLOAT fPrice);
    VOID EndUpdate(VOID); // just to say we finished updating for now

    FLOAT   GetChannelSize(UINT uDepthIndex) const  { ASSERT(MEGAZZ_MAX_DEPTH > uDepthIndex); return m_afSize[uDepthIndex]; }
    UINT    GetChannelBoxNum(VOID) const            { return m_uBoxNum; }
    UINT    GenerateMegaZZ(ZZUtil::SFloatValue** ppsList, UINT uListSize);
    
    const MegaZZ::SChannel& GetOldestChannel(UINT uDepthIndex) const;
    const MegaZZ::SChannel& GetCurrentChannel(UINT uDepthIndex) const   { ASSERT(MEGAZZ_MAX_DEPTH > uDepthIndex); return m_asChannels[uDepthIndex][m_uFrameIndex]; }
    const MegaZZ::SChannel* GetChannels(UINT uDepthIndex) const         { ASSERT(MEGAZZ_MAX_DEPTH > uDepthIndex); return m_asChannels[uDepthIndex]; }
    const MegaZZ::SFrame*   GetFrames(VOID) const                       { return m_asFrames; }
    BYTE GetPrevDisplayValue(UINT uDepthIndex) const                    { ASSERT(MEGAZZ_MAX_DEPTH > uDepthIndex); return m_abyPrevChannelIndex[uDepthIndex]; }
    
    UINT    GetFrameUsedNum(VOID) const             { return m_uUsedFrameNum; }
    UINT    GetFrameIndex(VOID) const               { return m_uFrameIndex; }

    UINT    GetZZFrameNum(VOID) const               { return m_uZZFrameNum; }
    
    FLOAT   GetPreviousPrice(VOID) const            { return m_fPrevPrice; }
    FLOAT   GetCurrentPrice(VOID) const             { return m_fCurrentPrice; }
    UINT    GetLastUpdatedTime(VOID) const          { return m_uLastUpdateTime; }
    UINT64  GetLastUpdatedTimeMillisec(VOID) const  { return m_ullLastUpdateTimeMillisec; }
    UINT    GetTradePairIndex(VOID) const           { return m_uTradePairIndex; }
    ESOURCE_MARKET GetSource(VOID) const            { return m_eSource; }
    const CHAR* GetName(VOID) const;
    BOOLEAN IsPriceMovedUp(VOID) const              { return m_fPrevPrice < m_fCurrentPrice; }
private:
    ESOURCE_MARKET                  m_eSource;
    UINT                            m_uTradePairIndex;
    FLOAT                           m_afSize[MEGAZZ_MAX_DEPTH];
    FLOAT                           m_afDivisor[MEGAZZ_MAX_DEPTH];
    FLOAT                           m_afDisplayDivisor[MEGAZZ_MAX_DEPTH];
    BYTE                            m_abyPrevChannelIndex[MEGAZZ_MAX_DEPTH];
    UINT                            m_uBoxNum;

    MegaZZ::SChannel                m_asChannels[MEGAZZ_MAX_DEPTH][MEGAZZ_MAX_HISTORY]; // all channel history
    MegaZZ::SFrame                  m_asFrames[MEGAZZ_MAX_HISTORY];                     // smallest channel frame history
    ZZUtil::SFloatValue             m_asZZs[MEGAZZ_MAX_HISTORY];                        // mega zz of this channel
    
    UINT                            m_uUsedFrameNum;    // number of frames in use
    UINT                            m_uFrameIndex;      // current frame index (of the latest frame)
    BOOLEAN                         m_bZZGenerated;     // whether ZZ has been generated or not
    UINT                            m_uZZFrameNum;      // number of zz frames generated
    
    FLOAT                           m_fPrevPrice;
    FLOAT                           m_fCurrentPrice;
    UINT                            m_uLastUpdateTime;
    UINT64                          m_ullLastUpdateTimeMillisec;
    INT                             m_nPrevPriceIndex;
    
    VOID    InitAllChannels(UINT uTimeSec);
    VOID    UpdateAllChannels(UINT uFrameIndex, UINT uTimeSec, BOOLEAN bCopyFromPrev);
    
    UINT    GetOldestFrameIndex(VOID) const;
};
#endif // #if !defined(MEGA_ZZ_H)

