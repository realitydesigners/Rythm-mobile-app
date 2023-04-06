#if !defined(TRADE_LOG_H)
#define TRADE_LOG_H
#include "TradeLogDefs.h"

class CMegaZZ;

class CTradeLog
{
public:
    static BOOLEAN Initialize(VOID);
    static VOID Release(VOID);

    static VOID Clear(VOID);
    static VOID AddLog(const STradeLog& sLog);
    static const CTradeLogs& GetLogs(VOID);
    static const CActiveTradeLogMap& GetActiveLogs(VOID);
    
    static STradeLog* GenerateNewLog(const CMegaZZ& cMegaZZ, UINT uLogicIndex, UINT uPreTradePatternIndex, UINT uCatPatternIndex, UINT uSLPatternIndex, UINT uTPIndex, BOOLEAN bLong, UINT uPendingMsgID);
    static VOID OnTradeOpened(UINT uPendingMsgID, UINT uTradeID, FLOAT fOpenPrice);
    static VOID UpdateTrade(BOOLEAN bLong, UINT uTradeID, FLOAT fHighestPipReached, FLOAT fLowestPipReached);
    static VOID OnCloseLog(BOOLEAN bLong, UINT uTradeID, FLOAT fClosePrice, FLOAT fHighestPipReached);
};

#endif // #if !defined(TRADE_LOG_H)
