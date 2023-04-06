#include "stdafx.h"
#include "Renko.h"

#define MAX_HISTORY_NUM (10000)

CRenko::CRenko() : m_dPipSize(0.0), m_uMaxHistoryNum(MAX_HISTORY_NUM)
{
    
}
CRenko::~CRenko()
{
    
}

BOOLEAN CRenko::Initlialize(DOUBLE dPipSize)
{
    Release(); // safety
    m_dPipSize = dPipSize;
    UINT uMultiplier;
    if (0.000001 > dPipSize) {
        uMultiplier = 100000000;
    }
    else if (0.00001 > dPipSize) {
        uMultiplier = 10000000;
    }
    else if (0.0001 > dPipSize) {
        uMultiplier = 1000000;
    }
    else if (0.001 > dPipSize) {
        uMultiplier = 100000;
    }
    else if (0.01 > dPipSize) { // we don't accept smaller than this amount
        uMultiplier = 10000;
    }
    else if (0.1 > dPipSize) {
        uMultiplier = 1000;
    }
    else if (1.0 > dPipSize) {
        uMultiplier = 100;
    }
    else {
        uMultiplier = 10;
    }
    m_sRenko.uDivisor = (UINT)(uMultiplier * dPipSize);
    m_sRenko.uMultiplier = uMultiplier * 100; // 1 pip is 0.01 so we move it to 1 dot.
    return TRUE;
}

VOID CRenko::Release(VOID)
{
    m_sRenko.cList.clear();
}

VOID CRenko::Update(DOUBLE dPrice)
{
    const INT64 nllPriceIndex = (INT64)((dPrice * m_sRenko.uMultiplier) / m_sRenko.uDivisor);
    if (0 == m_sRenko.cList.size()) {
        const Renko::SRenko_Box sTmp = {
            nllPriceIndex,
            Renko::EType_Neutral,
            { 0 }
        };
        m_sRenko.cList.push_front(sTmp);
        return;
    }
    const Renko::SRenko_Box& sPrev = *(m_sRenko.cList.begin());
    if (nllPriceIndex == sPrev.nllIndex) {
        return;
    }
    const BYTE byType = nllPriceIndex > sPrev.nllIndex ? Renko::EType_Up : Renko::EType_Down;
    const INT nOffset = nllPriceIndex > sPrev.nllIndex ? 1 : -1;
    const Renko::SRenko_Box sTmp = {
        sPrev.nllIndex + nOffset,
        byType,
        { 0 }
    };
    m_sRenko.cList.push_front(sTmp);
    while (m_uMaxHistoryNum < m_sRenko.cList.size()) {
        m_sRenko.cList.pop_back();
    }
    Update(dPrice);
}
