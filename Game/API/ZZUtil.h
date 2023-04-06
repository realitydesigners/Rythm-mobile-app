#if !defined(ZZ_UTIL_H)
#define ZZ_UTIL_H
#include "RenkoDef.h"

namespace ZZUtil {

    struct SInfo {
        UINT uColor;
        FLOAT fDeviationPercent; // deviation percentage (1 = 1%)
        FLOAT fThickness;
    };
    VOID LoadZZInfo(VOID);
    UINT GetZZLevelNum(VOID);
    const SInfo& GetInfo(UINT uLevel);

    struct SFloatValue {
        FLOAT fValue;
        USHORT ushActualIndex; // index into the correct frames
        USHORT ushIndex;
        BYTE byType;
        BYTE byFrameType;       // whether its up or down frame (used by inner zz)
        BYTE byZZIndicator;     // whether this is a ZZ, and what level is it
    };
    UINT GenerateZZ(SFloatValue** ppsList, UINT uMax, SFloatValue* psValues, UINT uNum, FLOAT fDeviation, BYTE byIndicator);

    struct SRythmZZ {
        UINT uActualIndex;  // index into the frame
        INT nPriceIndex;    // price index
        BYTE byZZType;      // Up or Down ZZ
        BYTE byIndicator;   // indicator (0xFF means not indicator)
        BYTE abyPad[2];
    };
    BOOLEAN GenerateZZ(SRythmZZ* psZZs, const Renko::SShiftedRenkoInnerFrame* psFrames, UINT uUsed, UINT uMax, UINT uCurrentIndex, UINT uRenkoPerFrame);
    BOOLEAN GenerateCustomZZ(SRythmZZ* psZZs, const Renko::SShiftedRenkoInnerFrame* psFrames, UINT uUsed, UINT uMax, UINT uCurrentIndex, UINT uRenkoPerFrame, UINT uZZLevelNum, const FLOAT* pfDeviations);

    BOOLEAN GenerateZZ(SRythmZZ* psZZs, const Renko::SShiftedRenko* psFrames, UINT uUsed, UINT uMax, UINT uCurrentIndex, UINT uRenkoPerFrame);
    BOOLEAN GenerateZZ(SRythmZZ* psZZs, const Renko::SSyncedRenkoFrame* psFrames, UINT uUsed, UINT uMax, UINT uCurrentIndex, UINT uRenkoPerFrame);
};
#endif // #if !defined(ATR_UTIL_H)
