#include "stdafx.h"
#include "AutoLogicData.h"
#include "AutoStatisticsWidget.h"
#include "Event.h"
#include "PatternData.h"


CAutoStatisticsWidget::CAutoStatisticsWidget(UINT uIndex, const SFilteredData& sData) :
CUIButton(),
m_uIndex(uIndex),
m_sData(sData),
INITIALIZE_TEXT_LABEL(m_cName),
INITIALIZE_TEXT_LABEL(m_cLong),
INITIALIZE_TEXT_LABEL(m_cTargetOpenedNum),
INITIALIZE_TEXT_LABEL(m_cTakeProfitNum),
INITIALIZE_TEXT_LABEL(m_cStopLossNum),
INITIALIZE_TEXT_LABEL(m_cCumulativePip)
{
    
}
    
CAutoStatisticsWidget::~CAutoStatisticsWidget()
{
    
}
 
VOID CAutoStatisticsWidget::Initialize(UINT uEventID)
{
    SetLocalSize(STATISTICS_WIDTH, STATISTICS_HEIGHT);
    m_cBG.SetLocalSize(STATISTICS_WIDTH, STATISTICS_HEIGHT);
    m_cBG.SetColor(RGBA(0x1F, 0x1F, 0x1F, 0xFF));
    SetDisplayWidgets(m_cBG, m_cBG);
    SetID(uEventID);
    m_cName.SetFont(EGAMEFONT_SIZE_12);
    m_cName.SetAnchor(0.0f, 0.5f);
    m_cName.SetLocalPosition(STATISTICS_NAME_X, STATISTICS_HEIGHT * 0.5f);
    AddChild(m_cName);
    
    m_cLong.SetFont(EGAMEFONT_SIZE_12);
    m_cLong.SetAnchor(0.5f, 0.5f);
    m_cLong.SetLocalPosition(STATISTICS_LONG_X, STATISTICS_HEIGHT * 0.5f);
    AddChild(m_cLong);
    
    m_cTargetOpenedNum.SetFont(EGAMEFONT_SIZE_12);
    m_cTargetOpenedNum.SetAnchor(0.5f, 0.5f);
    m_cTargetOpenedNum.SetLocalPosition(STATISTICS_TARGET_X, STATISTICS_HEIGHT * 0.5f);
    AddChild(m_cTargetOpenedNum);

    m_cTakeProfitNum.SetFont(EGAMEFONT_SIZE_12);
    m_cTakeProfitNum.SetAnchor(0.5f, 0.5f);
    m_cTakeProfitNum.SetLocalPosition(STATISTICS_TAKE_PROFIT_X, STATISTICS_HEIGHT * 0.5f);
    AddChild(m_cTakeProfitNum);
    
    m_cStopLossNum.SetFont(EGAMEFONT_SIZE_12);
    m_cStopLossNum.SetAnchor(0.5f, 0.5f);
    m_cStopLossNum.SetLocalPosition(STATISTICS_STOPLOSS_X, STATISTICS_HEIGHT * 0.5f);
    AddChild(m_cStopLossNum);
    
    m_cCumulativePip.SetFont(EGAMEFONT_SIZE_12);
    m_cCumulativePip.SetAnchor(0.5f, 0.5f);
    m_cCumulativePip.SetLocalPosition(STATISTICS_PIP_X, STATISTICS_HEIGHT * 0.5f);
    AddChild(m_cCumulativePip);
    
    m_cName.SetString(m_sData.szName);
    m_cLong.SetString(m_sData.bLong ? "B" : "S");
    CHAR szBuffer[128];
    snprintf(szBuffer, 128, "%d", m_sData.uTargetNum);
    m_cTargetOpenedNum.SetString(szBuffer);
    snprintf(szBuffer, 128, "%d", m_sData.uTPNum);
    m_cTakeProfitNum.SetString(szBuffer);
    snprintf(szBuffer, 128, "%d", m_sData.uSLNum);
    m_cStopLossNum.SetString(szBuffer);
    snprintf(szBuffer, 128, "%.1f", m_sData.fPipGain);
    m_cCumulativePip.SetString(szBuffer);
    if (0.0f < m_sData.fPipGain) {
        m_cCumulativePip.SetColor(RGBA(74, 255, 74, 255));
    }
    else if (0.0f > m_sData.fPipGain) {
        m_cCumulativePip.SetColor(RGBA(255, 74, 74, 255));
    }
    else {
        m_cCumulativePip.SetColor(0xFFFFFFFF);
    }
}


VOID CAutoStatisticsWidget::SetSelected(BOOLEAN bSelected)
{
    m_cBG.SetColor(bSelected ? RGBA(0x4F, 0x4F, 0x4F, 0xFF) : RGBA(0x1F, 0x1F, 0x1F, 0xFF));
}

VOID CAutoStatisticsWidget::Release(VOID)
{
    m_cName.Release();
    m_cName.RemoveFromParent();
    
    m_cLong.Release();
    m_cLong.RemoveFromParent();
    
    m_cTargetOpenedNum.Release();
    m_cTargetOpenedNum.RemoveFromParent();
    
    m_cTakeProfitNum.Release();
    m_cTakeProfitNum.RemoveFromParent();
    
    m_cStopLossNum.Release();
    m_cStopLossNum.RemoveFromParent();
    
    m_cCumulativePip.Release();
    m_cCumulativePip.RemoveFromParent();
    
    CUIButton::Release();
}

BOOLEAN CAutoStatisticsWidget::CreateButtonEvent(CEvent& cEvent)
{
    cEvent.SetIntParam(0, GetID());
    cEvent.SetIntParam(1, m_uIndex);
    return TRUE;
}

