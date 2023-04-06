#if !defined(RENKO_DEF_H)
#define RENKO_DEF_H

#include <list>

#define MAX_DEPTH (9)
#define SHIFTED_RANKO_HISTORY_SIZE (40000)
#define SHIFTED_RENKO_INNER_FRAME_NUM (40000)
#define SYNCED_RANKO_HISTORY_SIZE (5000)
#define SYNCED_DEPTH_NUM (9)


#define UNUSED_INDEX (0xFF)

struct STickData {
    DOUBLE dClose;
    DOUBLE dHigh;
    DOUBLE dLow;
    UINT64 ullTime;
};

namespace ZZUtil {
    struct SRythmZZ;
};
namespace Renko {
    enum EType {
        EType_Neutral=0,
        EType_Up,
        EType_Down,
    };
    struct SRenko_Box {
        INT64 nllIndex;
        BYTE  byType;
        BYTE  abyPad[7];
    };
    typedef std::list<SRenko_Box> CRenkoList;
    struct SRenko {
        UINT        uMultiplier;                        // multiplier to bring the value into integer cutoff
        UINT        uDivisor;                           // divisor (size of each renko box)
        UINT        uUnitPerBox;                        // how many units per box
        CRenkoList  cList;
    };
    struct SShiftedRenko {
        DOUBLE  dDivisor;                           // divisor (size of each renko box)
        UINT    uUpdateTimeSec;                     // time this SR was updated (created, or when retrace is higher)
        INT     nLowerIndex;                        // min index that still stays within this renko
        BYTE    byHighRetraceIndex;                 // highest retrace index (lower index + value)
        BYTE    byLowRetraceIndex;                  // lowest retrace index  (lower index + value)
        BYTE    byRankoNum;                         // number of rankos in this SR
        BYTE    byCurrentIndex;                     // index of the latest history
        BYTE    byBarType;                          // (Etype_Neutral, Etype_Up, EType_Down)
        BYTE    byInitialized;                      // initialized?
        BYTE    abyPad[2];
    };
    struct SShiftedRenkoInnerFrame {
        INT  nLowerIndex;   // need to save this
        UINT uIndex;        // current index
        UINT uUpdateTime;   // last updated time
        BYTE byType;        // current channel type
        BYTE byChannelType; // neutral for no movement, (only used by PNF)
        BYTE byBoxType;     // whether this box was up agaist prev frame or not or neutral
        BYTE byPad;
    };

    struct SSyncedRenkoFrame {
        UINT    uUpdateTimeSec;
        INT     nLowerIndex;
        BYTE    byType;
        BYTE    byCurrentIndex;                     // index of the latest history
        BYTE    byHighRetraceIndex;                 // highest retrace index (lower index + value)
        BYTE    byLowRetraceIndex;                  // lowest retrace index  (lower index + value)
        BYTE    abyPad[4];
    };

    VOID GetPriceRange(DOUBLE& dHigh, DOUBLE& dLow, const SShiftedRenko& sSR);
    DOUBLE GetPrice(INT64 nllPriceIndex, const SShiftedRenko& sSR);
    VOID AdjustChannelSize(FLOAT fTopChannelSize, FLOAT* pfSizes, UINT uSizeNum);
    VOID GetChannelDisplayLbl(CHAR* szBuffer, UINT uBufferLen, UINT uDepth);
    UINT GetChannelValue(UINT uDepth);
};

#endif // #if !defined(RENKO_DEF_H)

