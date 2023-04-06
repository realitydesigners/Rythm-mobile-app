#include "stdafx.h"
#include "AutoLogicData.h"
#include "EventID.h"
#include "EventManager.h"
#include "OandaMgr.h"
#include "PatternData.h"
#include "TradeLogGraph.h"
#include "TradeLogWidget.h"

VOID CTradeLogGraph::PostEvent(CEvent& cEvent, FLOAT fDelay)
{
    CEventManager::PostEvent(*this, cEvent, fDelay);
}

CTradeLogGraph::CTradeLogGraph() :
CUIContainer(),
INITIALIZE_TEXT_LABEL(m_cPriceTxt),
INITIALIZE_TEXT_LABEL(m_cSlippageTxt),
INITIALIZE_TEXT_LABEL(m_cEntryList),
INITIALIZE_TEXT_LABEL(m_cStopLoss),
INITIALIZE_TEXT_LABEL(m_cTPEntry),
INITIALIZE_TEXT_LABEL(m_cOpenTxt),
INITIALIZE_TEXT_LABEL(m_cCloseTxt),
m_cDashboard(0, 0, 0),
m_bOpen(TRUE)
{
    memset(&m_sLog, 0, sizeof(STradeLog));
    memset(m_apcBarLbls, 0, sizeof(m_apcBarLbls));
}
    
CTradeLogGraph::~CTradeLogGraph()
{
}
 

#define GRAPH_DIMENSION (350.0f)
VOID CTradeLogGraph::Initialize(VOID)
{
    const FLOAT fW = 5.0f + GRAPH_DIMENSION + GRAPH_DIMENSION + 5.0f;
    const FLOAT fH = GRAPH_DIMENSION + 80.0f;
    SetLocalSize(fW, fH);
    m_cBG.SetLocalSize(fW, fH);
    m_cBG.SetColor(RGBA(0x1F, 0x1F, 0x1F, 0xFF));
    m_cBG.SetBorderColor(RGBA(0x7F, 0x7F, 0x7F, 0xFF));
    AddChild(m_cBG);

    m_cPriceTxt.SetFont(EGAMEFONT_SIZE_14);
    m_cPriceTxt.SetLocalPosition(10.0f, 0.0f);
    AddChild(m_cPriceTxt);
    
    m_cSlippageTxt.SetFont(EGAMEFONT_SIZE_12);
    m_cSlippageTxt.SetLocalPosition(10.0f, 20.0f);
    AddChild(m_cSlippageTxt);
    
    m_cEntryList.SetFont(EGAMEFONT_SIZE_12);
    m_cEntryList.SetLocalPosition(10.0f, 40.0f);
    AddChild(m_cEntryList);
    
    m_cStopLoss.SetFont(EGAMEFONT_SIZE_12);
    m_cStopLoss.SetAnchor(1.0f, 0.0f);
    m_cStopLoss.SetLocalPosition(fW - 10.0f, 0.0f);
    AddChild(m_cStopLoss);
    
    m_cTPEntry.SetFont(EGAMEFONT_SIZE_12);
    m_cTPEntry.SetAnchor(1.0f, 0.0f);
    m_cTPEntry.SetLocalPosition(fW - 10.0f, 20.0f);
    AddChild(m_cTPEntry);

    CHAR szBuffer[64];
    FLOAT fBarX = 260.0f;
    for (UINT uIndex = 0; 9 > uIndex; ++uIndex) {
        CUITextLabel* pcLbl = NEW_TEXT_LABEL;
        if (NULL == pcLbl) {
            ASSERT(FALSE);
            return;
        }
        pcLbl->SetFont(EGAMEFONT_SIZE_10);
        pcLbl->SetAnchor(0.5f, 0.5f);
        pcLbl->SetLocalPosition(fBarX, 10.0f);
        snprintf(szBuffer, 64, "D%d", 9 - uIndex);
        pcLbl->SetString(szBuffer);
        AddChild(*pcLbl);
        m_apcBarLbls[uIndex] = pcLbl;
        fBarX += 24.0f;
    }
    m_cDashboard.SetLocalPosition(103.0f, 18.0f);
    m_cDashboard.Initialize();
    m_cDashboard.HideNameAndPrice();
    m_cDashboard.SetEnabled(FALSE);
    AddChild(m_cDashboard);
    
    m_cTop.SetLocalPosition(5.0f, 70.0f);
    m_cTop.SetLocalSize(GRAPH_DIMENSION, GRAPH_DIMENSION);
    m_cTop.SetDefaultDepthIndex(0);
    m_cTop.SetDefaultDepthNum(5);
    m_cTop.Initialize();
    AddChild(m_cTop);
    
    m_cBottom.SetLocalPosition(5.0f + GRAPH_DIMENSION, 70.0f);
    m_cBottom.SetLocalSize(GRAPH_DIMENSION, GRAPH_DIMENSION);
    m_cBottom.SetDefaultDepthIndex(4);
    m_cBottom.SetDefaultDepthNum(5);
    m_cBottom.Initialize();
    AddChild(m_cBottom);
    
    m_acBtnIMG[0].SetLocalSize(50.0f, 20.0f);
    m_cOpenTxt.SetFont(EGAMEFONT_SIZE_12);
    m_cOpenTxt.SetLocalPosition(25.0f, 10.0f);
    m_cOpenTxt.SetAnchor(0.5f, 0.5f);
    m_cOpenTxt.SetString("Open");
    m_acBtn[0].SetDisplayWidgets(m_acBtnIMG[0], m_acBtnIMG[0]);
    m_acBtn[0].SetID(EBTN_OPEN);
    m_acBtn[0].SetLocalPosition(fW -  110.0f, 45.0f);
    m_acBtn[0].AddChild(m_cOpenTxt);
    AddChild(m_acBtn[0]);
    
    m_acBtnIMG[1].SetLocalSize(50.0f, 20.0f);
    m_cCloseTxt.SetFont(EGAMEFONT_SIZE_12);
    m_cCloseTxt.SetLocalPosition(25.0f, 10.0f);
    m_cCloseTxt.SetAnchor(0.5f, 0.5f);
    m_cCloseTxt.SetString("Close");
    m_acBtn[1].SetDisplayWidgets(m_acBtnIMG[1], m_acBtnIMG[1]);
    m_acBtn[1].SetID(EBTN_CLOSE);
    m_acBtn[1].SetLocalPosition(fW -  55.0f, 45.0f);
    m_acBtn[1].AddChild(m_cCloseTxt);
    AddChild(m_acBtn[1]);
    
    Clear();
}

VOID CTradeLogGraph::Release(VOID)
{
    m_cBG.RemoveFromParent();
    
    m_cPriceTxt.Release();
    m_cPriceTxt.RemoveFromParent();
    
    m_cSlippageTxt.Release();
    m_cSlippageTxt.RemoveFromParent();
    
    m_cEntryList.Release();
    m_cEntryList.RemoveFromParent();
    
    m_cStopLoss.Release();
    m_cStopLoss.RemoveFromParent();
    
    m_cTPEntry.Release();
    m_cTPEntry.RemoveFromParent();
    
    for (UINT uIndex = 0; 9 > uIndex; ++uIndex) {
        SAFE_REMOVE_RELEASE_DELETE(m_apcBarLbls[uIndex]);
    }
    m_cDashboard.Release();
    m_cDashboard.RemoveFromParent();
    
    m_cTop.Release();
    m_cTop.RemoveFromParent();
    
    m_cBottom.Release();
    m_cBottom.RemoveFromParent();
    
    m_cOpenTxt.Release();
    m_cOpenTxt.RemoveFromParent();
    m_acBtn[0].RemoveFromParent();
    
    m_cCloseTxt.Release();
    m_cCloseTxt.RemoveFromParent();
    m_acBtn[1].RemoveFromParent();
}

VOID CTradeLogGraph::Clear(VOID)
{
    m_cTop.ClearChart();
    m_cBottom.ClearChart();
    m_cDashboard.Clear();
    
    m_cPriceTxt.SetString("Price");
    m_cSlippageTxt.SetString("Slippage");
    m_cEntryList.SetString("Pattern");
    m_cStopLoss.SetString("SL");
    m_cTPEntry.SetString("TP");
    
    m_bOpen = TRUE;
    UpdateBtnDisplay();
}

VOID CTradeLogGraph::Update(const STradeLog& sLog)
{
    if (m_sLog.uTradeID != sLog.uTradeID) {
        m_bOpen = TRUE;
        UpdateBtnDisplay();
    }
    m_sLog = sLog;
    COandaMgr& cMgr = COandaMgr::GetInstance();
    const COandaMgr::SOandaPair* psPair = cMgr.GetTradePair(sLog.byTradePairIndex);
    if (NULL == psPair) {
        ASSERT(FALSE);
        return;
    }
    CHAR szBuffer[1024];
    CHAR szOpenPrice[64];
    CHAR szClosePrice[64];
    cMgr.GetPriceDisplayString(szOpenPrice, 64, sLog.byTradePairIndex, sLog.fOpenPrice);
    if (0 < sLog.uCloseTimeSec) {
        cMgr.GetPriceDisplayString(szClosePrice, 64, sLog.byTradePairIndex, sLog.fCLosePrice);
    }
    else {
        cMgr.GetPriceDisplayString(szClosePrice, 64, sLog.byTradePairIndex, psPair->dPrice);
    }
    snprintf(szBuffer, 1024, "Price %s => %s", szOpenPrice, szClosePrice);
    m_cPriceTxt.SetString(szBuffer);
    m_cSlippageTxt.ClearString();
    snprintf(szBuffer, 1024, "Slippage %.2f pip", sLog.fSlippage * psPair->dPipMultiplier);
    m_cSlippageTxt.SetString(szBuffer);
    
    const CHAR* szLogicName = CPatternDataMgr::GetPattern(sLog.uCategoryPatternIndex).szName;
    CHAR szLogicFullName[2048];
    snprintf(szLogicFullName, 2048, "Cat: %s", szLogicName);
    m_cEntryList.SetString(szLogicFullName);

    snprintf(szBuffer, 1024, "SL %s", CAutoLogicDataMgr::GetTPEntry(sLog.uStopLossTPIndex).szName);
    m_cStopLoss.SetString(szBuffer);

    // do tp string
    const STPEntry& sTP = CAutoLogicDataMgr::GetTPEntry(sLog.uTPIndex);
    switch (sTP.eType) {
        case ETP_PIP_ONLY:
            snprintf(szBuffer, 1024, "TP Pip %.1f/%.1f", sTP.fPip, sTP.fTrailStop);
            break;
        case ETP_FRAME_LVL:
            snprintf(szBuffer, 1024, "TP F%d %.1f%%/%.1f", sTP.byFrameIndex, sTP.fPercent * 100.0f, sTP.fTrailStop);
            break;
        default:
            ASSERT(FALSE);
            snprintf(szBuffer, 1024, "TP Error");
            break;
    }
    m_cTPEntry.SetString(szBuffer);
    
    m_cTop.PrepareChart(sLog, m_bOpen);
    m_cBottom.PrepareChart(sLog, m_bOpen);
    m_cDashboard.Update(sLog, m_bOpen);
}

VOID CTradeLogGraph::OnReceiveEvent(CEvent& cEvent)
{
    const UINT uEventID = cEvent.GetIntParam(0);
    switch (uEventID) {
        case EBTN_OPEN:
            ToggleDisplay(TRUE);
            break;
        case EBTN_CLOSE:
            ToggleDisplay(FALSE);
            break;
    }
}

VOID CTradeLogGraph::ToggleDisplay(BOOLEAN bOpen)
{
    if (!bOpen && 0 == m_sLog.uCloseTimeSec) {
        return;
    }
    m_bOpen = bOpen;
    UpdateBtnDisplay();
    if (0 != m_sLog.uOpenTimeSec) {
        Update(m_sLog);
    }
}

VOID CTradeLogGraph::UpdateBtnDisplay(VOID)
{
    m_acBtnIMG[0].SetColor(m_bOpen ? RGBA(0x7F, 0x7F, 0x7F, 0xFF) : RGBA(0x4F, 0x4F, 0x4F, 0xFF) );
    m_acBtnIMG[1].SetColor(!m_bOpen ? RGBA(0x7F, 0x7F, 0x7F, 0xFF) : RGBA(0x4F, 0x4F, 0x4F, 0xFF) );
}
