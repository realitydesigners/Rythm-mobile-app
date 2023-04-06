#if !defined(AUTOMATION_MGR_DEFS_H)
#define AUTOMATION_MGR_DEFS_H
#include "TradeLogDefs.h"
#include "FractalDataDefs.h"
#include "AutoLogicDefs.h"
#include <list>
#include <map>

class CMegaZZ;
// map of fractal index
typedef std::map<UINT,CMegaZZ*>  CFractalMegaZZMap;
typedef std::pair<UINT,CMegaZZ*> CFractalMegaZZPair;

// map of trade pairs
typedef std::map<UINT, CFractalMegaZZMap*>  CTradePairFractalMap;
typedef std::pair<UINT, CFractalMegaZZMap*> CTradePairFractalPair;

struct SAutoRunningLogic {
    const SFractal&             sFractal;
    const SLogic&               sLogic;
    const UINT                  uLogicIndex;
    BOOLEAN                     bStopped;
    UINT                        uStopLossCount;      // number of times we "stop loss"ed
    SAutoRunningLogic(const SFractal& _sFrac, const SLogic& _sLogic, UINT _uLogicIndex) :
    sFractal(_sFrac), sLogic(_sLogic), uLogicIndex(_uLogicIndex), bStopped(FALSE), uStopLossCount(0) { }
};
typedef std::list<SAutoRunningLogic*> CAutoRunningLogicList;

// an entry (can be pending entry)
struct SAutomationEntry {
    UINT uEntrySequence;                        // a running ID of sequences (for multiple targets to reference each other in the same entry)
    BYTE byTradePairIndex;                      // trade pair index
    BYTE byLong;                                // long or short
    
    BYTE byPipLevelCrossed;                     // whether we crossed the required pip level
    FLOAT fHighestPipGain;                      // highest pip gained. (for computing trailing stop for ETP_PIP)
    FLOAT fLowestPipLoss;

    UINT  uLogicIndex;                          // logic index
    UINT  uStopLossTPIndex;                // stop loss pattern
    FLOAT fStopLossSafetyPip;                   // biggest stop loss pip to check
    UINT  uTPIndex;                             // TPChart index
    FLOAT fTP_PipCheck;                         // current target pips to hit
    FLOAT fTP_PipTrailCheck;                    // current atrget pips to trail
    
    FLOAT fSafetyTP_PipCheck;                   // safety pip check // sell if fall below here.
    
    UINT uLotSize;                              // lot size in this trade
    UINT uPendingMsgID;                         // the logic that triggered this entry
    
    UINT uTradeID;                              // trade ID;
    FLOAT fTransactPrice;                       // transacted price
    FLOAT fSlippage;                            // slippage
};
typedef std::list<SAutomationEntry*>      CTradeEntryList;
typedef std::map<UINT, CTradeEntryList*>  CTradePairEntryMap;
typedef std::pair<UINT, CTradeEntryList*> CTradePairEntryPair;


typedef std::map<UINT, SAutomationEntry*>  CPendingEntryMap;
typedef std::pair<UINT, SAutomationEntry*> CPendingEntryPair;

struct SAutoPendingCloseTrade {
    UINT        uPendingMsgID;
    UINT        uTradePairIndex;
    FLOAT       fHighestPipGain;
    BOOLEAN     bLong;
};
typedef std::map<UINT, SAutoPendingCloseTrade*>  CClosingTradeMap;
typedef std::pair<UINT, SAutoPendingCloseTrade*> CClosingTradePair;

struct SBibLog {
    UINT uTime;
    FLOAT fPrice;
    BYTE abyWAS[8];
    BYTE abyIS[8];
    FLOAT afTop[8];
    FLOAT afBottom[8];
};
typedef std::list<SBibLog*> CBibLogList;
typedef std::map<UINT, CBibLogList*>  CBibLogMap;
typedef std::pair<UINT, CBibLogList*> CBibLogPair;

#endif // #if !defined(AUTOMATION_MGR_DEFS_H)
