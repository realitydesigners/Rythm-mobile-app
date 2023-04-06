#if !defined(SHIFTED_RENKO_H)
#define SHIFTED_RENKO_H
#include "RenkoDef.h"
#include "ZZUtil.h"

struct SRythmListener;
class CShiftedRenko {
public:
    CShiftedRenko();
    ~CShiftedRenko();
    
    BOOLEAN Initlialize(UINT uDepth, UINT uBoxNum, const FLOAT* pfSizes);
    VOID Release(VOID);
    
    VOID Clear(VOID); // clear the renko and start over
    VOID Update(DOUBLE dPrice, UINT uTimeSec);
    VOID EndUpdate(VOID); // just to say we finished updating for now
    VOID RebuildZZs(VOID);
    
    const Renko::SShiftedRenko* GetSR(UINT uDepth) const                           { return m_apsSR[uDepth]; }
    const Renko::SShiftedRenko& GetCurrentSR(UINT uDepth) const;
    Renko::EType                GetSRType(UINT uDepth) const;

    const ZZUtil::SRythmZZ*     GetSRZZs(UINT uDepth) const;
    UINT                        GetSRNum(UINT uDepth) const                        { return m_auUsedNum[uDepth]; }
    UINT                        GetSRCurrentIndex(UINT uDepth) const               { return m_auCurrentIndex[uDepth]; }
    FLOAT                       GetChannelSize(UINT uDepthIndex) const             { ASSERT(MAX_DEPTH > uDepthIndex); return m_afSize[uDepthIndex]; }
    UINT                        GetChannelBoxNum(VOID) const                       { return m_uBoxNum; }
    
    DOUBLE GetCurrentPrice(VOID) const { return m_dCurrentPrice; }
    UINT   GetLastUpdatedTime(VOID) const { return m_uLastUpdateTime; }
    
private:
    UINT                            m_uDepth;
    // channel data
    Renko::SShiftedRenko*           m_apsSR[MAX_DEPTH];
    ZZUtil::SRythmZZ                m_asSRZZs[MAX_DEPTH][SHIFTED_RENKO_INNER_FRAME_NUM];
    BOOLEAN                         m_abSRZZCalculated[MAX_DEPTH];
    UINT                            m_auUsedNum[MAX_DEPTH];
    UINT                            m_auCurrentIndex[MAX_DEPTH];
    
    DOUBLE                          m_dPrevPrice;
    DOUBLE                          m_dCurrentPrice;
    UINT                            m_uLastUpdateTime;
    FLOAT                           m_afSize[MAX_DEPTH];
    UINT                            m_uBoxNum;
    
    static VOID UpdateRenkoIndex(UINT uDepthIndex, Renko::SShiftedRenko& sRenko, INT nPriceIndex, UINT uTimeSec, BOOLEAN bAppendMovement);
    static VOID Update(Renko::SShiftedRenko* psRenkos, UINT uRenkoNum, UINT& uCurrentIndex, UINT& uUsedNum, DOUBLE dPrice, UINT uTimeSec, BOOLEAN bAppendMovement, UINT uDepthIndex);
    
    VOID Update(DOUBLE dPrice, UINT uTimeSec, UINT uDepthIndex);
#if defined(DEBUG)
    VOID DebugDump(VOID);
#endif // #if defined(DEBUG)
    
    // Size Optimization code
public:
    static BOOLEAN TryOptimize(FLOAT& fOptimizedSize, UINT uBoxNum, const SRythmListener& sListener);
private:
//    BOOLEAN BrendsMethod(FLOAT fA, FLOAT fB, Renko::SShiftedRenko* psRenkos, UINT uRenkoNum, UINT uBoxSize, const DOUBLE* pdValues, UINT uValueNum);
    static FLOAT Evaluate(Renko::SShiftedRenko* psRenkos, UINT uRenkoNum, FLOAT fSize, UINT uBoxSize, const DOUBLE* pdValues, UINT uValueNum);
    static FLOAT ComputeBalance(const Renko::SShiftedRenko* psRenkos, UINT uCurrentIndex, UINT uUsedNum, UINT uTotalRenkoNum);
};
#endif // #if !defined(SHIFTED_RENKO_H)

