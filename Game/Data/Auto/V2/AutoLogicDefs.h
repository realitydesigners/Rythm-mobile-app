#if !defined(AUTO_LOGIC_DEFS_H)
#define AUTO_LOGIC_DEFS_H

#define TP_NAME_LEN                 (32)
#define TP_PARAM_TO_PIP(x)          ((x) * 0.01f)
#define TP_PIP_TO_PARAM(x)          (UINT)((x) * 100)
enum ETP_TYPE {
    ETP_PIP_ONLY=0,
    ETP_FRAME_LVL,
};
struct STPEntry {
    CHAR        szName[TP_NAME_LEN];
    ETP_TYPE    eType;
    FLOAT       fPip;
    FLOAT       fTrailStop;
    BYTE        byFrameIndex;
    FLOAT       fPercent;       // 1 = 100% of frame size
};

#define TRADE_ORDER_NAME_LEN        (32)
#define TRADE_ORDER_TARGET_NUM      (6)
#define TRADE_ORDER_UNUSED_TARGET   (-1)
struct SLogic {
    CHAR    szName[TRADE_ORDER_NAME_LEN];       // name
    UINT    uCategoryPatternIndex;              // lookup pattern table
    BOOLEAN bLong;                              // long or short
    UINT    uLotSize;                           // lot size
    UINT    uStopLossTPIndex;              // index for stop loss
    INT     anTargetIndex[TRADE_ORDER_TARGET_NUM];    // lookup to target table
};

#endif // #if !defined(AUTO_LOGIC_DEFS_H)
