#include "stdafx.h"
#include "ShiftedRenko.h"

CShiftedRenko::CShiftedRenko() :
m_uDepth(0),
m_dPrevPrice(0.0f),
m_dCurrentPrice(0.0),
m_uLastUpdateTime(0),
m_uBoxNum(0)
{
    memset(m_apsSR, 0, sizeof(m_apsSR));
    memset(m_asSRZZs, 0, sizeof(m_asSRZZs));
    memset(m_abSRZZCalculated, 0, sizeof(m_abSRZZCalculated));
    memset(m_auUsedNum, 0, sizeof(m_auUsedNum));
    memset(m_auCurrentIndex, 0, sizeof(m_auCurrentIndex));

    memset(m_afSize, 0, sizeof(m_afSize));
}
CShiftedRenko::~CShiftedRenko()
{
}

BOOLEAN CShiftedRenko::Initlialize(UINT uDepth, UINT uBoxNum, const FLOAT* pfSizes)
{
    Release(); // safety
    if (MAX_DEPTH < uDepth) {
        ASSERT(FALSE);
        return FALSE;
    }
    m_uDepth = uDepth;
    m_uBoxNum = uBoxNum;
    for (UINT uIndex = 0; m_uDepth > uIndex; ++uIndex) {
        m_afSize[uIndex] = pfSizes[uIndex];
        m_apsSR[uIndex] = new Renko::SShiftedRenko[SHIFTED_RANKO_HISTORY_SIZE];
        if (NULL == m_apsSR[uIndex]) {
            ASSERT(FALSE);
            return FALSE;
        }
        memset(m_apsSR[uIndex], 0, sizeof(Renko::SShiftedRenko) * SHIFTED_RANKO_HISTORY_SIZE);
        Renko::SShiftedRenko& sSR = m_apsSR[uIndex][0];
        if (0xFF < m_uBoxNum) {
            ASSERT(FALSE);
            return FALSE;
        }
        sSR.byRankoNum = m_uBoxNum;
        DOUBLE dSize = ((DOUBLE)pfSizes[uIndex]) / m_uBoxNum;
        if (0.0 == dSize) {
            TRACE("Warn: we have zero size for depth %d!\n", uIndex);
            dSize = 1.0f;
        }
        sSR.dDivisor = 1.0 / dSize;
    }
    return TRUE;
}

VOID CShiftedRenko::Release(VOID)
{
    for (UINT uIndex = 0; MAX_DEPTH > uIndex; ++uIndex ){
        SAFE_DELETE_ARRAY(m_apsSR[uIndex]);
    }
    m_uDepth = 0;
    memset(m_apsSR, 0, sizeof(m_apsSR));
    memset(m_abSRZZCalculated, 0, sizeof(m_abSRZZCalculated));
    memset(m_auUsedNum, 0, sizeof(m_auUsedNum));
    memset(m_auCurrentIndex, 0, sizeof(m_auCurrentIndex));
}

VOID CShiftedRenko::RebuildZZs(VOID)
{
    memset(m_abSRZZCalculated, 0, sizeof(m_abSRZZCalculated));
}
VOID CShiftedRenko::Clear(VOID)
{
    memset(m_abSRZZCalculated, 0, sizeof(m_abSRZZCalculated));
    memset(m_auUsedNum, 0, sizeof(m_auUsedNum));
    memset(m_auCurrentIndex, 0, sizeof(m_auCurrentIndex));
    for (UINT uDepthIndex = 0; m_uDepth > uDepthIndex; ++uDepthIndex) {
        ASSERT(NULL != m_apsSR[uDepthIndex]);
        if (NULL != m_apsSR[uDepthIndex]) {
            for (UINT uIndex = 0; SHIFTED_RANKO_HISTORY_SIZE > uIndex; ++uIndex) {
                Renko::SShiftedRenko& sRenko = m_apsSR[uDepthIndex][uIndex];
                sRenko.byBarType = Renko::EType_Neutral;
                sRenko.byCurrentIndex = UNUSED_INDEX;
                sRenko.byInitialized = FALSE;
            }
        }
    }
}

Renko::EType CShiftedRenko::GetSRType(UINT uDepth) const
{
    ASSERT(m_uDepth > uDepth);
    const UINT uCurrentIndex = m_auCurrentIndex[uDepth];
    Renko::SShiftedRenko& sRenko = m_apsSR[uDepth][uCurrentIndex];
    return (Renko::EType)sRenko.byBarType;
}
const Renko::SShiftedRenko& CShiftedRenko::GetCurrentSR(UINT uDepth) const
{
    ASSERT(m_uDepth > uDepth);
    const UINT uCurrentIndex = m_auCurrentIndex[uDepth];
    return m_apsSR[uDepth][uCurrentIndex];
}
VOID CShiftedRenko::Update(DOUBLE dPrice, UINT uTimeSec)
{
    m_dPrevPrice = m_dCurrentPrice;
    m_dCurrentPrice = dPrice;
    m_uLastUpdateTime = uTimeSec;
    for (UINT uIndex = 0; m_uDepth > uIndex; ++uIndex) { // for each depth
        Update(dPrice, uTimeSec, uIndex);
    }
}
VOID CShiftedRenko::EndUpdate(VOID)
{
    memset(m_abSRZZCalculated, 0, sizeof(m_abSRZZCalculated));
}

VOID CShiftedRenko::Update(DOUBLE dPrice, UINT uTimeSec, UINT uDepthIndex)
{
    ASSERT(m_uDepth > uDepthIndex);
    Update(m_apsSR[uDepthIndex], SHIFTED_RANKO_HISTORY_SIZE,
           m_auCurrentIndex[uDepthIndex], m_auUsedNum[uDepthIndex],
           dPrice, uTimeSec, TRUE, uDepthIndex);
}

#if defined(DEBUG)
VOID CShiftedRenko::DebugDump(VOID)
{
    for (UINT uIndex = 0; 8 > uIndex; ++uIndex) {
//        TRACE("Depth: %d %d / %d\n", uIndex, m_auInnerFrameCurrentIndex[uIndex], m_auInnerFrameNum[uIndex]);
        TRACE("Depth: %d %d / %d\n", uIndex, m_auCurrentIndex[uIndex], m_auUsedNum[uIndex]);
    }
}
#endif // #if defined(DEBUG)

VOID CShiftedRenko::UpdateRenkoIndex(UINT uDepthIndex, Renko::SShiftedRenko& sRenko, INT nPriceIndex, UINT uTimeSec, BOOLEAN bAppendMovement)
{
    ASSERT(nPriceIndex >= sRenko.nLowerIndex);
    const BYTE byCurrentRetraceIndex = nPriceIndex - sRenko.nLowerIndex;
    ASSERT(byCurrentRetraceIndex < sRenko.byRankoNum);
    if (UNUSED_INDEX == sRenko.byCurrentIndex) { // no entries yet
        ASSERT(UNUSED_INDEX == sRenko.byLowRetraceIndex);
        ASSERT(UNUSED_INDEX == sRenko.byHighRetraceIndex);
        sRenko.byHighRetraceIndex = byCurrentRetraceIndex;
        sRenko.byLowRetraceIndex = byCurrentRetraceIndex;
        sRenko.uUpdateTimeSec = uTimeSec;
    }
    else {
        ASSERT(UNUSED_INDEX != sRenko.byLowRetraceIndex);
        ASSERT(UNUSED_INDEX != sRenko.byHighRetraceIndex);
        if (sRenko.byLowRetraceIndex > byCurrentRetraceIndex) {
            sRenko.byLowRetraceIndex = byCurrentRetraceIndex;
            sRenko.uUpdateTimeSec = uTimeSec;
        }
        else if (sRenko.byHighRetraceIndex < byCurrentRetraceIndex) {
            sRenko.byHighRetraceIndex = byCurrentRetraceIndex;
            sRenko.uUpdateTimeSec = uTimeSec;
        }
    }
    sRenko.byCurrentIndex = byCurrentRetraceIndex;
}

VOID CShiftedRenko::Update(Renko::SShiftedRenko* psRenkos, UINT uRenkoNum,
                           UINT& uCurrentIndex, UINT& uUsedNum,
                           DOUBLE dPrice, UINT uTimeSec, BOOLEAN bAppendMovement, UINT uDepthIndex)
{
    ASSERT(NULL != psRenkos);
    ASSERT(SHIFTED_RANKO_HISTORY_SIZE > uCurrentIndex);
    Renko::SShiftedRenko& sRenko = psRenkos[uCurrentIndex];
    INT nPriceIndex = 0;
    {
        const INT64 nllPriceIndex = (INT64)(dPrice * sRenko.dDivisor);
        if (0x7FFFFFFF <= nllPriceIndex || 0 > nllPriceIndex) {
            TRACE("Too small!\n");
            //ASSERT(FALSE);
            return;
        }
        nPriceIndex = (INT)nllPriceIndex;
    }
    if (0 == sRenko.byInitialized) { // if not yet initialized
        // update the upper/lower bounds
        sRenko.nLowerIndex = nPriceIndex - (sRenko.byRankoNum >> 1);
        sRenko.uUpdateTimeSec = uTimeSec;
        sRenko.byHighRetraceIndex = sRenko.byLowRetraceIndex = UNUSED_INDEX;
        sRenko.byInitialized = 1;
        sRenko.byCurrentIndex = UNUSED_INDEX;
        ++uUsedNum;
        ASSERT(1 == uUsedNum);
        return;
    }
    if (Renko::EType_Neutral == sRenko.byBarType) { // if at 1st renko, check if price moved. if moved, shift the first renko
        const INT nPrevPriceIndex = sRenko.nLowerIndex + sRenko.byCurrentIndex;
        if (nPrevPriceIndex != nPriceIndex) {
            const BOOLEAN bUp = nPriceIndex > nPrevPriceIndex;
            if (bUp) {
                const INT nDiff = (sRenko.byRankoNum - 1) - sRenko.byCurrentIndex;
                sRenko.byCurrentIndex = (sRenko.byRankoNum - 1);
                sRenko.nLowerIndex -= nDiff;
                sRenko.byBarType = Renko::EType_Up;
            }
            else {
                sRenko.nLowerIndex += sRenko.byCurrentIndex;
                sRenko.byCurrentIndex = 0;
                sRenko.byBarType = Renko::EType_Down;
            }
        }
    }
    Renko::SShiftedRenko* psRenko = &sRenko;
    BOOLEAN bHigher = (psRenko->nLowerIndex + psRenko->byRankoNum) <= nPriceIndex;
    BOOLEAN bLower = psRenko->nLowerIndex > nPriceIndex;
    UINT uNewIndex = uCurrentIndex;
    while (bHigher || bLower) { // while out of boundary
        ++uNewIndex;
        if (SHIFTED_RANKO_HISTORY_SIZE <= uNewIndex) {
            uNewIndex -= SHIFTED_RANKO_HISTORY_SIZE;
        }
        ASSERT(uNewIndex < SHIFTED_RANKO_HISTORY_SIZE);
        if (SHIFTED_RANKO_HISTORY_SIZE > uUsedNum) {
            ++uUsedNum;
        }
        uCurrentIndex = uNewIndex;
        Renko::SShiftedRenko& sNewRenko = psRenkos[uNewIndex];
        sNewRenko.dDivisor = psRenko->dDivisor;
        if (bHigher) {
            sNewRenko.nLowerIndex = psRenko->nLowerIndex + 1;
            sNewRenko.byLowRetraceIndex = sNewRenko.byHighRetraceIndex = UNUSED_INDEX;
            sNewRenko.byBarType = Renko::EType_Up;
        }
        else {
            sNewRenko.nLowerIndex = psRenko->nLowerIndex - 1;
            sNewRenko.byLowRetraceIndex = sNewRenko.byHighRetraceIndex = UNUSED_INDEX;
            sNewRenko.byBarType = Renko::EType_Down;
        }
        sNewRenko.uUpdateTimeSec = uTimeSec;
        sNewRenko.byRankoNum = psRenko->byRankoNum;
        sNewRenko.byCurrentIndex = UNUSED_INDEX;
        sNewRenko.byInitialized = 1;
        psRenko = &sNewRenko;
        bHigher = (psRenko->nLowerIndex + psRenko->byRankoNum) <= nPriceIndex;
        bLower = psRenko->nLowerIndex > nPriceIndex;
    }
    if (!bHigher && !bLower) { // if within boundary
        UpdateRenkoIndex(uDepthIndex, *psRenko, nPriceIndex, uTimeSec, bAppendMovement);
    }
}

const ZZUtil::SRythmZZ* CShiftedRenko::GetSRZZs(UINT uDepth) const
{
    ASSERT(MAX_DEPTH > uDepth);
    if (!m_abSRZZCalculated[uDepth]) {
        const Renko::SShiftedRenko* psFrames = m_apsSR[uDepth];
        ZZUtil::SRythmZZ* psZZs = const_cast<ZZUtil::SRythmZZ*>(m_asSRZZs[uDepth]);
        ZZUtil::GenerateZZ(psZZs,
                           psFrames,
                           m_auUsedNum[uDepth],
                           SHIFTED_RENKO_INNER_FRAME_NUM,
                           m_auCurrentIndex[uDepth],
                           psFrames->byRankoNum);
        BOOLEAN* pbCalculated = const_cast<BOOLEAN*>(m_abSRZZCalculated);
        pbCalculated[uDepth] = TRUE;
    }
    return m_asSRZZs[uDepth];
}
