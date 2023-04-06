#include "stdafx.h"
#include "AutomationMgr.h"
#include "HLTime.h"

#define SECONDS_A_DAY       (86400)

// GMT 22:00 (entry start)
#define ENTRY_START_TIME_HH (23)
#define ENTRY_START_TIME_MM (0)

// GMT 17:30 (entry stop)
#define ENTRY_STOP_TIME_HH  (19)
#define ENTRY_STOP_TIME_MM  (30)

// 20:00 GMT // close all trades
#define TRADE_CLOSE_TIME_HH (20)
#define TRADE_CLOSE_TIME_MM (0)

#define WEEKEND_OPEN_TIME_HH (22)
#define WEEKEND_OPEN_TIME_MM (0)

#define ENTRY_START_TIME_SEC    (ENTRY_START_TIME_HH * 60 * 60 +    ENTRY_START_TIME_MM * 60)
#define ENTRY_STOP_TIME_SEC     (ENTRY_STOP_TIME_HH * 60 * 60 +     ENTRY_STOP_TIME_MM * 60)
#define TRADE_CLOSE_TIME_SEC    (TRADE_CLOSE_TIME_HH * 60 * 60 +    TRADE_CLOSE_TIME_MM * 60)
#define WEEKEND_OPEN_TIME_SEC   (WEEKEND_OPEN_TIME_HH * 60 * 60 +   WEEKEND_OPEN_TIME_MM * 60)


//#define IGNORE_TIME
// forex closes on friday 10pm gmt to sunday 10pm gmt
VOID CAutomationMgr::UpdateTime(VOID)
{
    const UINT uTimeSecs = CHLTime::GetTimeSecs();
    const UINT uDayOfWeek = CHLTime::GetDayOfWeek(uTimeSecs);
    m_bMarketClosed = FALSE;
    const UINT uTimeSec = uTimeSecs % 86400; // get today's time in GMT seconds
    if (5 <= uDayOfWeek) { // friday or more
        if (5 == uDayOfWeek && uTimeSec >= TRADE_CLOSE_TIME_SEC) { // if friday and closed
            m_bMarketClosed = TRUE;
        }
        else if (5 < uDayOfWeek) {
            m_bMarketClosed = TRUE;
        }
    }
    else if (0 == uDayOfWeek) { // sunday
        m_bMarketClosed = (uTimeSec <= WEEKEND_OPEN_TIME_SEC);
    }
    
#if defined(IGNORE_TIME)
    m_bCanEnterNewTrades = TRUE;
    m_uSecondsToOpenTime = 0;
    m_uSecondsToStopNewEntryTime = ENTRY_STOP_TIME_SEC;
    m_uSecondsToCloseAllTrades = TRADE_CLOSE_TIME_SEC;
#else // #if defined(IGNORE_TIME)
    m_bCanEnterNewTrades = (ENTRY_START_TIME_SEC <= uTimeSec) || (ENTRY_STOP_TIME_SEC >= uTimeSec);
    if (m_bCanEnterNewTrades) {
        m_uSecondsToOpenTime = 0;
        if (ENTRY_START_TIME_SEC <= uTimeSec) {
            m_uSecondsToStopNewEntryTime = ENTRY_STOP_TIME_SEC + 2 * 60 * 60 - (uTimeSec - ENTRY_START_TIME_SEC);
        }
        else {
            m_uSecondsToStopNewEntryTime = ENTRY_STOP_TIME_SEC - uTimeSec;
        }
    }
    else {
        m_uSecondsToStopNewEntryTime = 0;
        if (ENTRY_START_TIME_SEC > uTimeSec) {
            m_uSecondsToOpenTime = ENTRY_START_TIME_SEC - uTimeSec;
        }
        else {
            ASSERT(SECONDS_A_DAY > uTimeSec);
            m_uSecondsToOpenTime = (SECONDS_A_DAY - uTimeSec) + ENTRY_START_TIME_SEC;
        }
    }
    const UINT uCloseAllTradeSeconds = m_uSecondsToCloseAllTrades;
    if (0 < m_uSecondsToStopNewEntryTime) {
        m_uSecondsToCloseAllTrades = m_uSecondsToStopNewEntryTime + 30 * 60;
    }
    else {
        if (TRADE_CLOSE_TIME_SEC > uTimeSec) {
            m_uSecondsToCloseAllTrades = TRADE_CLOSE_TIME_SEC - uTimeSec;
        }
        else {
            m_uSecondsToCloseAllTrades = 0;
        }
    }
    if (0 < uCloseAllTradeSeconds && 0 == m_uSecondsToCloseAllTrades) {
        CloseAllOpenEntries();
    }
#endif // #if defined(IGNORE_TIME)
}
