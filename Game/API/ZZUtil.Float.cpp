#include "stdafx.h"
#include "PlayerData.h"
#include "ZZUtil.h"
#include <math.h>

UINT ZZUtil::GenerateZZ(SFloatValue** ppsList, UINT uMax, SFloatValue* psValues, UINT uNum, FLOAT fDeviation, BYTE byIndicator)
{
    if (5 > uNum) { // skip if less than 5 points..
        return 0;
    }
    UINT uAddedNum = 0;
    ZZUtil::SFloatValue* psPrev = &psValues[0];
    psPrev->byType = Renko::EType_Neutral;
    ppsList[uAddedNum] = psPrev;
    ++uAddedNum;
    ZZUtil::SFloatValue* psCurrent = NULL;
    for (UINT uIndex = 1; uNum > uIndex; ++uIndex) {
        psCurrent = &psValues[uIndex];
        const FLOAT fDiff = psCurrent->fValue - psPrev->fValue;
        switch (psPrev->byType) {
            case Renko::EType_Neutral:
                if (abs(fDiff) >= fDeviation) { // if more than deviation, accept new point
                    const BOOLEAN bUpwards = (0.0f < fDiff);
                    psPrev->byType = bUpwards ? Renko::EType_Down : Renko::EType_Up;
                    psCurrent->byType = bUpwards ? Renko::EType_Up : Renko::EType_Down;
                    if (uMax == uAddedNum) {
                        ASSERT(FALSE);
                        return 0;
                    }
                    ASSERT(uMax > uAddedNum);
                    ppsList[uAddedNum] = psCurrent;
                    ++uAddedNum;
                    psPrev = psCurrent;
                }
                break;
            case Renko::EType_Down: // prev was a down,
                {
                    if (0.0f > fDiff) { // if current is still a down
                        psCurrent->byType = Renko::EType_Down;
                        // replace previous down
                        ASSERT(0 < uAddedNum);
                        ppsList[uAddedNum-1] = psCurrent;
                        psPrev = psCurrent;
                    }
                    else {
                        psCurrent->byType = Renko::EType_Up;
                        if (fDeviation <= fDiff) { // if more than deviation, accept new point
                            if (uMax == uAddedNum) {
                                ASSERT(FALSE);
                                return 0;
                            }
                            ASSERT(uMax > uAddedNum);
                            ppsList[uAddedNum] = psCurrent;
                            ++uAddedNum;
                            psPrev = psCurrent;
                        }
                    }
                }
                break;
            case Renko::EType_Up: // prev was a up
                {
                    if (0.0f < fDiff) { // if current is still a up
                        psCurrent->byType = Renko::EType_Up;
                        // replace previous down
                        ASSERT(uMax > uAddedNum);
                        ASSERT(0 < uAddedNum);
                        ppsList[uAddedNum-1] = psCurrent;
                        psPrev = psCurrent;
                    }
                    else {
                        psCurrent->byType = Renko::EType_Down;
                        if (-fDeviation >= fDiff) { // if more than deviation, accept new point
                            if (uMax == uAddedNum) {
                                ASSERT(FALSE);
                                return 0;
                            }
                            ASSERT(uMax > uAddedNum);
                            ppsList[uAddedNum] = psCurrent;
                            ++uAddedNum;
                            psPrev = psCurrent;
                        }
                    }
                }
                break;
            default:
                ASSERT(FALSE);
                break;
        }
    }
    if (1 == uAddedNum) {
        return 0;
    }
    // iterate thru all the zz and set the indicator value
    for (UINT uIndex = 0; uAddedNum > uIndex; ++uIndex) {
        ppsList[uIndex]->byZZIndicator = byIndicator;
    }
    return uAddedNum;
}
