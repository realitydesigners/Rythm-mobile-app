#if !defined(GAME_WIDGET_DEFS_H)
#define GAME_WIDGET_DEFS_H
#include "NibDefs.h"

enum EGameWidgetType {
    EGameWidget_TradeChart = EBaseWidget_Num,
    EGameWidget_CMOChart,
    EGameWidget_DoubleArrow,
    EGameWidget_TimeIndicator,
    EGameWidget_CircleSema,
    EGameWidget_Semaphore,
    EGameWidget_TradePairName,
};

#define CHART_OFFSET_FROM_RIGHT (40.0f)

#define GRID_COLOR              RGBA(0x3F,0x3F,0x3F,0xFF)
#define FINGER_FATNESS          (10.0f)


#endif // #if !defined((GAME_WIDGET_DEFS_H))
