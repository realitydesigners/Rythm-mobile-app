#if !defined(MEGA_ZZ_DEF_H)
#define MEGA_ZZ_DEF_H
#include <memory.h>
#define MEGAZZ_MAX_DEPTH                    (9)
#define MEGAZZ_MAX_HISTORY                  (40000)
#define MEGAZZ_LOGIC_NAME_LENGTH            (32)
#define MEGAZZ_LOGIC_CATEGORY_DESC_LENGTH   (512)

namespace MegaZZ {
    enum EType {
        EType_Neutral=0,
        EType_Up,
        EType_Down,
    };
    struct SChannel {
        FLOAT fTop;                     // top price of this channel
        FLOAT fBottom;                  // bottom price of this channel
        BYTE byType;                    // channel type
        BYTE byDisplayNumber;           // number to display on dashboard
        BYTE byRetracementNumber;       // biggest number reached
        BYTE byRetracementPercent;      // used by bib
        UINT uRetracementUpdateTime;    // time retracement was performed
    };
    struct SFrame {
        FLOAT fPrice;           // price now
        UINT uUpdateTime;       // last updated time
        BYTE byMovementType;    // whether current price movement was up agaist prev frame or not or neutral
        BYTE byCopy;            // whether this frame is a copy of the previous frame
    };

    DOUBLE GetPrice(INT64 nllPriceIndex, DOUBLE dDivisor);
    VOID AdjustChannelSize(FLOAT fTopChannelSize, FLOAT* pfSizes, UINT uSizeNum);
    VOID GetChannelDisplayLbl(CHAR* szBuffer, UINT uBufferLen, UINT uDepth);
    UINT GetChannelValue(UINT uDepth);
};
#endif // #if !defined(MEGA_ZZ_DEF_H)

