#include "stdafx.h"
#include "EventID.h"
#include "EventManager.h"
#include "AlertList.h"
#include "AutomationMgr.h"
#include "PatternData.h"
#include "AutoLogicData.h"

VOID CAlertList::PostEvent(CEvent& cEvent, FLOAT fDelay)
{
    CEventManager::PostEvent(*this, cEvent, fDelay);
}

CAlertList::CAlertList() :
CUIContainer(),
INITIALIZE_TEXT_LABEL(m_cEnable),
INITIALIZE_TEXT_LABEL(m_cMute),
m_bEnabled(FALSE),
m_bMuted(FALSE)
{
    
}
    
CAlertList::~CAlertList()
{
}
 
VOID CAlertList::Initialize(VOID)
{
    const SHLVector2D& sSize = GetLocalSize();
    m_cBG.SetLocalSize(sSize.x, sSize.y);
    m_cBG.SetColor(RGBA(0x1F, 0x1F, 0x1F, 0xFF));
    m_cBG.SetBorderColor(RGBA(0x7F, 0x7F, 0x7F, 0xFF));
    AddChild(m_cBG);

    m_cBtnBG.SetLocalSize(100.0f, 20.0f);
    m_cBtnBG.SetColor(RGBA(0x1F, 0x1F, 0x1F, 0xFF));
    m_cBtnBG.SetBorderColor(RGBA(0x7F, 0x7F, 0x7F, 0xFF));
    
    m_cEnable.SetFont(EGAMEFONT_SIZE_12);
    m_cEnable.SetAnchor(0.5f, 0.5f);
    m_cEnable.SetLocalPosition(50.0f, 10.0f);
    m_cEnableBtn.SetID(EBTN_TOGGLE_ENABLE);
    m_cEnableBtn.SetAnchor(1.0f, 0.0f);
    m_cEnableBtn.SetLocalPosition(sSize.x - 5.0f - 100.0f - 5.0f, 5.0f);
    m_cEnableBtn.SetDisplayWidgets(m_cBtnBG, m_cBtnBG);
    m_cEnableBtn.AddChild(m_cEnable);
    AddChild(m_cEnableBtn);
    
    m_cMute.SetFont(EGAMEFONT_SIZE_12);
    m_cMute.SetAnchor(0.5f, 0.5f);
    m_cMute.SetLocalPosition(50.0f, 10.0f);
    m_cMute.SetString("Mute");
    m_cMuteBtn.SetID(EBTN_MUTE);
    m_cMuteBtn.SetAnchor(1.0f, 0.0f);
    m_cMuteBtn.SetLocalPosition(sSize.x - 5.0f, 5.0f);
    m_cMuteBtn.SetDisplayWidgets(m_cBtnBG, m_cBtnBG);
    m_cMuteBtn.AddChild(m_cMute);
    AddChild(m_cMuteBtn);
    
    const CHAR* aszHeaders[2] = {
        "Pair", "Pattern"
    };
    const FLOAT afSizes[2] = {
        80.0f, sSize.x - 80.0f
    };
    m_cTbl.SetLocalPosition(0.0f, 30.0f);
    m_cTbl.Initialize(aszHeaders, afSizes, 2, sSize.y - 30.0f, EBTN_DO_NOTHING);
    AddChild(m_cTbl);
    
    UpdateEnableLabel();
    CEventManager::RegisterForBroadcast(*this, EGLOBAL_AUTOMATION_MGR, EGLOBAL_EVT_AUTOMATION_MEGAZZ_UPDATED);
}

VOID CAlertList::Release(VOID)
{
    CEventManager::UnRegisterForBroadcast(*this, EGLOBAL_AUTOMATION_MGR, EGLOBAL_EVT_AUTOMATION_MEGAZZ_UPDATED);
    m_cEnable.Release();
    m_cEnable.RemoveFromParent();
    m_cEnableBtn.RemoveFromParent();
    
    m_cMute.Release();
    m_cMute.RemoveFromParent();
    m_cMuteBtn.RemoveFromParent();
    
    m_cTbl.Release();
    m_cTbl.RemoveFromParent();
    
    m_cBG.RemoveFromParent();
    m_cMap.clear();
}

VOID CAlertList::OnReceiveEvent(CEvent& cEvent)
{
    switch (cEvent.GetIntParam(0)) {
        case EGLOBAL_EVT_AUTOMATION_MEGAZZ_UPDATED:
            OnUpdateMegaZZ(cEvent.GetIntParam(1));
            break;
    }
}

VOID CAlertList::UpdateEnableLabel(VOID)
{
    m_cEnable.SetString(m_bEnabled ? "Status: On" : "Status : Off");
}

VOID CAlertList::OnUpdateMegaZZ(UINT uTradePairIndex)
{
    const CMegaZZ* pcZZ = CAutomationMgr::GetInstance().FindMegaZZ(uTradePairIndex, CAutomationMgr::GetInstance().GetFractalIndex());
    if (NULL == pcZZ) {
//        ASSERT(FALSE);
        return;
    }
    const UINT uPatternNum = CAutoLogicDataMgr::GetAlertNum();
    const UINT* puPatternIndex = CAutoLogicDataMgr::GetAlertPatterns();
    for (UINT uIndex = 0; uPatternNum > uIndex; ++uIndex) {
        const UINT uPattern = puPatternIndex[uIndex];
        if (CPatternDataMgr::CanPassPattern(*pcZZ, uPattern, FALSE)) {
            SetPair(uTradePairIndex, uPattern);
            return;
        }
    }
    SetPair(uTradePairIndex, -1);
}

VOID CAlertList::SetPair(UINT uTradePairIndex, INT nPattern)
{
    CTradePairPatternMap::iterator itFound = m_cMap.find(uTradePairIndex);
    if (m_cMap.end() == itFound) {
        if (0 > nPattern) {
            return;
        }
        m_cMap.insert(CTradePairPatternPair(uTradePairIndex, nPattern));
    }
    else {
        const INT nOldPattern = itFound->second;
        if (nOldPattern == nPattern) {
            return;
        }
        if (0 > nPattern) {
            m_cMap.erase(itFound);
        }
        else {
            m_cMap.insert(CTradePairPatternPair(uTradePairIndex, nPattern));
        }
    }
    UpdateList();
}

#include "OandaMgr.h"
VOID CAlertList::UpdateList(VOID)
{
    m_cTbl.ClearEntries();
    const CHAR* aszData[2];
    for (auto it : m_cMap) {
        const COandaMgr::SOandaPair* psPair = COandaMgr::GetInstance().GetTradePair(it.first);
        if (NULL == psPair) {
            ASSERT(FALSE);
            continue;
        }
        aszData[0] = psPair->szTradePair;
        aszData[1] = CPatternDataMgr::GetPattern(it.second).szName;
        m_cTbl.AddRow(aszData, 2, 0);
    }
}
