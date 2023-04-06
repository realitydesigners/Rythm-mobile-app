#include "stdafx.h"
#include "AutoLogicData.h"
#include "DetailedStatisticsList.h"
#include "EventID.h"
#include "EventManager.h"
#include "OandaMgr.h"
#include "PatternData.h"
#include "TradeLog.h"

VOID CDetailedStatisticsList::PostEvent(CEvent& cEvent, FLOAT fDelay)
{
    // do not post to parent.
    CEventManager::PostEvent(*this, cEvent, fDelay);
}

CDetailedStatisticsList::CDetailedStatisticsList() :
CUIContainer(),
INITIALIZE_TEXT_LABEL(m_cExitTxt)
{

}
    
CDetailedStatisticsList::~CDetailedStatisticsList()
{
}
 
VOID CDetailedStatisticsList::Initialize(VOID)
{
    m_cBG.SetColor(RGBA(0x1F, 0x1F, 0x1F, 0xFF));
    m_cBG.SetBorderColor(RGBA(0x7F, 0x7F, 0x7F, 0xFF));
    m_cBG.SetLocalSize(DETAILED_LIST_WIDTH, 300.0f);
    AddChild(m_cBG);
    m_cExitBtnBG.SetColor(RGBA(0x4F, 0x4F, 0x4F, 0xFF));
    m_cExitBtnBG.SetLocalSize(20.0f, 20.0f);
    m_cExitBtn.SetDisplayWidgets(m_cExitBtnBG, m_cExitBtnBG);
    m_cExitBtn.SetID(EBTN_CLOSE);
    m_cExitBtn.SetAnchor(1.0f, 0.0f);
    m_cExitBtn.SetLocalPosition(DETAILED_LIST_WIDTH - 5.0f, 5.0f);
    m_cExitTxt.SetFont(EGAMEFONT_SIZE_12);
    m_cExitTxt.SetAnchor(0.5f, 0.5f);
    m_cExitTxt.SetLocalPosition(10.0f, 10.0f);
    m_cExitTxt.SetString("X");
    m_cExitBtn.AddChild(m_cExitTxt);
    AddChild(m_cExitBtn);
    
    const CHAR* szHeaders[7] = {
        "Pair", "TP Entry", "SL", "Pip", "Highest","Lowest", "Closed"
    };

    const FLOAT afSizes[7] = {
        TRADE_PAIR_NAME_LENGTH, TP_CHART_NAME_LENGTH, SL_NAME_LENGTH, PIP_GAIN_LENGTH, HIGHEST_PIP_GAIN_LENGTH, LOWEST_PIP_GAIN_LENGTH, CLOSED_LENGTH
    };
    m_cTbl.SetLocalPosition(0.0f, 30.0f);
    m_cTbl.Initialize(szHeaders, afSizes, 7, 270.0f, EBTN_UNUSED);
    AddChild(m_cTbl);
    SetLocalSize(DETAILED_LIST_WIDTH, 300.0f);
}

VOID CDetailedStatisticsList::Release(VOID)
{
    m_cBG.RemoveFromParent();
    
    m_cExitTxt.Release();
    m_cExitTxt.RemoveFromParent();
    m_cExitBtn.RemoveFromParent();

    m_cTbl.Release();
    m_cTbl.RemoveFromParent();
    
    CUIContainer::Release();
}
VOID CDetailedStatisticsList::ClearList(VOID)
{
    m_cTbl.ClearEntries();
}
VOID CDetailedStatisticsList::OnReceiveEvent(CEvent& cEvent)
{
    switch (cEvent.GetIntParam(0)) {
        case EBTN_CLOSE:
            RemoveFromParent();
            break;
    }
}


VOID CDetailedStatisticsList::RefreshList(UINT uLogicIndex)
{
    ClearList();
    const CTradeLogs& cOldLogs = CTradeLog::GetLogs();
    for (auto& sLog : cOldLogs) {
        if (uLogicIndex != sLog.uLogicIndex) {
            continue;
        }
        TryAddLog(sLog);
    }
    const CActiveTradeLogMap& cActives = CTradeLog::GetActiveLogs();
    for (auto itr : cActives) {
        const STradeLog* psLog = itr.second;
        if (psLog->uLogicIndex != uLogicIndex) {
            continue;
        }
        TryAddLog(*psLog);
    }
}
VOID CDetailedStatisticsList::TryAddLog(const STradeLog& sLog)
{
    const COandaMgr::SOandaPair* psPair = COandaMgr::GetInstance().GetTradePair(sLog.byTradePairIndex);
    if (NULL == psPair) {
        ASSERT(FALSE);
        return;
    }
    const CHAR* szData[7];
    szData[0] = psPair->szTradePair;
    szData[1] = CAutoLogicDataMgr::GetTPEntry(sLog.uTPIndex).szName;
    szData[2] = CAutoLogicDataMgr::GetTPEntry(sLog.uStopLossTPIndex).szName;
    FLOAT fGain;
    CHAR szBuffer[64];
    CHAR szHighest[32];
    CHAR szLowest[32];
    if (0.0f >= sLog.fCLosePrice) {
        fGain = psPair->dPrice - sLog.fOpenPrice;
        if (0 == sLog.byLong) {
            fGain = -fGain;
        }
    }
    else {
        fGain = sLog.fCLosePrice - sLog.fOpenPrice;
        if (0 == sLog.byLong) {
            fGain = -fGain;
        }
    }
    const FLOAT fPipGain = fGain * psPair->dPipMultiplier;
    if (0.0f <= fPipGain) {
        snprintf(szBuffer, 64, "\\c4%.1f", fGain * psPair->dPipMultiplier);
    }
    else {
        snprintf(szBuffer, 64, "\\c1%.1f", fGain * psPair->dPipMultiplier);
    }
    szData[3] = szBuffer;
    snprintf(szHighest, 32, "%.1f", sLog.fHighestPipReached);
    szData[4] = szHighest;
    snprintf(szLowest, 32, "%.1f", sLog.fLowestPipReached);
    szData[5] = szLowest;
    szData[6] = (0 < sLog.uCloseTimeSec) ? "Yes" : "No";
    m_cTbl.AddRow(szData, 7, 0);
}
