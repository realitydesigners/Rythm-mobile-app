#include "stdafx.h"
#include "CMain.h"
#include "Event.h"
#include "EventID.h"
#include "EventManager.h"
#include "MADetails.h"
#include "PlayerData.h"
#include "SaveData.h"
#include "TextInputWindow.h"
#include "TradeChart.h"
#include "UIButton.h"

#define SELECTED_COLOR      RGBA(0x1F,0x1F,0xFF,0xFF)
#define UNSELECTED_COLOR    RGBA(0x7F, 0x7F, 0x7F, 0xFF)

#define CREATE_BTN(x, y, w, h, id, text) CreateColorButton(x, y, w, h, id, UNSELECTED_COLOR, text, TRUE)

#define WIN_W (890.0f)
#define WIN_H (130.0f)

struct SLabelInfo {
    UINT uColor;
    UINT auBtn[3];
    UINT uToggle;
    UINT uInterval;
};
static const SLabelInfo s_asLabels[3] = {
    { MA_LINE_1_COLOR, { CMADetails::EBTN_LINE_1_SMA, CMADetails::EBTN_LINE_1_EMA, CMADetails::EBTN_LINE_1_NONE },
        CMADetails::EBTN_LINE_1_TOGGLE, CMADetails::EBTN_LINE_1_INTERVAL },
    { MA_LINE_2_COLOR, { CMADetails::EBTN_LINE_2_SMA, CMADetails::EBTN_LINE_2_EMA, CMADetails::EBTN_LINE_2_NONE },
        CMADetails::EBTN_LINE_2_TOGGLE, CMADetails::EBTN_LINE_2_INTERVAL },
    { MA_LINE_3_COLOR, { CMADetails::EBTN_LINE_3_SMA, CMADetails::EBTN_LINE_3_EMA, CMADetails::EBTN_LINE_3_NONE },
        CMADetails::EBTN_LINE_3_TOGGLE, CMADetails::EBTN_LINE_3_INTERVAL },
};


CMADetails::CMADetails(CTradeChart& cChart) :
CGameWindow(),
m_cChart(cChart)
{
    m_bDestroyOnExitModal = TRUE;
    memset(m_pcLbls, 0, sizeof(m_pcLbls));
    memset(m_apcBtns, 0, sizeof(m_apcBtns));
    memset(m_apcToggles, 0, sizeof(m_apcToggles));
}

CMADetails::~CMADetails()
{
}
VOID CMADetails::InitializeInternals(VOID)
{
    CGameWindow::InitializeInternals();
    SetLocalPosition(CMain::GetDisplaySideMargin(), 0.0);
    SetLocalSize(WIN_W, WIN_H);
    m_cBG.Set9PartTexture(ETEX::ID::ui_9part_filter_off, 3.0f, 3.0f, 3.0f, 3.0f);
    m_cBG.SetLocalSize(WIN_W, WIN_H);
    AddChild(m_cBG);
    
    CREATE_BTN((WIN_W - 60.0f), WIN_H - 60.0f, 50.0f, 50.0f, EBTN_EXIT, "Quit");
    
    CHAR szBuffer[512];
    UINT uIndex = 0;
    for ( ; 3 > uIndex; ++uIndex) {
        const SMovingAverage& sMA = m_cChart.GetMovingAverage(uIndex);
        snprintf(szBuffer, 512, "L%d (%d)", uIndex + 1, sMA.uIntervalNum);
        const SLabelInfo& sLbl = s_asLabels[uIndex];
        
        const UINT uYIndex = uIndex % 2;
        const FLOAT fY = 35.0f + uYIndex * 60.0f;
        const UINT uXIndex = uIndex / 2;
        const FLOAT fX = 10.0f + uXIndex * 450.0f;
        CUITextLabel* pcLbl = CreateLabel(szBuffer, ELABEL_COMMON_BUTTON_LABEL);
        if (NULL != pcLbl) {
            pcLbl->SetColor(sLbl.uColor);
            pcLbl->SetAnchor(0.0f, 0.5f);
            pcLbl->SetLocalPosition(fX, fY);
            AddChild(*pcLbl);
        }
        m_pcLbls[uIndex] = pcLbl;
        m_apcBtns[uIndex][0] = CREATE_BTN(fX + 80.0f, fY - 25.0f, 50.0f, 50.0f, sLbl.auBtn[0], "SMA");
        m_apcBtns[uIndex][1] = CREATE_BTN(fX + 140.0f, fY - 25.0f, 50.0f, 50.0f, sLbl.auBtn[1], "EMA");
        m_apcBtns[uIndex][2] = CREATE_BTN(fX + 200.0f, fY - 25.0f, 50.0f, 50.0f, sLbl.auBtn[2], "None");
        m_apcToggles[uIndex] = CREATE_BTN(fX + 260.0f, fY - 25.0f, 50.0f, 50.0f, sLbl.uToggle,  "Toggle");
        CREATE_BTN(fX + 320.0f, fY - 25.0f, 100.0f, 50.0f, sLbl.uInterval, "Change\nInterval");
        
        RefreshBtns(uIndex);
        RefreshToggles(uIndex);
    }
}

VOID CMADetails::Release(VOID)
{
    m_cBG.RemoveFromParent();
    CGameWindow::Release();
}

VOID CMADetails::OnReceiveEvent(CEvent& cEvent)
{
    const UINT uEventID = cEvent.GetIntParam(0);
    switch (uEventID) {
        case EBTN_EXIT:
            ExitModal();
            return;
        case EBTN_LINE_1_SMA:
            ChangeCurveType(0, EMA_SMA);
            break;
        case EBTN_LINE_2_SMA:
            ChangeCurveType(1, EMA_SMA);
            break;
        case EBTN_LINE_3_SMA:
            ChangeCurveType(2, EMA_SMA);
            break;
        case EBTN_LINE_1_EMA:
            ChangeCurveType(0, EMA_EMA);
            break;
        case EBTN_LINE_2_EMA:
            ChangeCurveType(1, EMA_EMA);
            break;
        case EBTN_LINE_3_EMA:
            ChangeCurveType(2, EMA_EMA);
            break;
        case EBTN_LINE_1_NONE:
            ChangeCurveType(0, EMA_NONE);
            break;
        case EBTN_LINE_2_NONE:
            ChangeCurveType(1, EMA_NONE);
            break;
        case EBTN_LINE_3_NONE:
            ChangeCurveType(2, EMA_NONE);
            break;
        case EBTN_LINE_1_TOGGLE:
            ToggleSourceType(0);
            break;
        case EBTN_LINE_2_TOGGLE:
            ToggleSourceType(1);
            break;
        case EBTN_LINE_3_TOGGLE:
            ToggleSourceType(2);
            break;
        case EBTN_LINE_1_INTERVAL:
            ChangeInterval(0);
            break;
        case EBTN_LINE_2_INTERVAL:
            ChangeInterval(1);
            break;
        case EBTN_LINE_3_INTERVAL:
            ChangeInterval(2);
            break;
        case EEVENT_ON_TEXT_INPUT_COMPLETE:
            OnTextInputComplete(cEvent.GetIntParam(1));
            break;
        default:
            break;
    }
}

VOID CMADetails::ChangeCurveType(UINT uIndex, EMA_TYPE eType)
{
    SMovingAverage& sMA = m_cChart.GetMovingAverage(uIndex);
    sMA.eType = eType;
    RefreshBtns(uIndex);
    m_cChart.GenerateMA(uIndex);
    CPlayerData::SetMovingAverageData(uIndex, sMA.eSource, sMA.eType, sMA.uIntervalNum);
    CSaveData::SaveData(FALSE);
}
VOID CMADetails::ToggleSourceType(UINT uIndex)
{
    SMovingAverage& sMA = m_cChart.GetMovingAverage(uIndex);
    sMA.eSource = (EMA_VALUE_SOURCE)((sMA.eSource + 1) % EMA_VALUE_SOURCE_NUM);
    RefreshToggles(uIndex);
    m_cChart.GenerateMA(uIndex);
    CPlayerData::SetMovingAverageData(uIndex, sMA.eSource, sMA.eType, sMA.uIntervalNum);
    CSaveData::SaveData(FALSE);
}

VOID CMADetails::RefreshBtns(UINT uIndex)
{
    const SMovingAverage& sMA = m_cChart.GetMovingAverage(uIndex);
    UINT uSelectedIndex = 0;
    switch (sMA.eType) {
        case EMA_SMA:
            break;
        case EMA_EMA:
            uSelectedIndex = 1;
            break;
        case EMA_NONE:
            uSelectedIndex = 2;
            break;
        default:
            ASSERT(FALSE);
            break;
    }
    UINT uIndex2 = 0;
    for ( ; 3 > uIndex2; ++uIndex2) {
        if (NULL != m_apcBtns[uIndex]) {
            CUIImage* pcImg = CUIImage::CastToMe(m_apcBtns[uIndex][uIndex2]->GetIdleWidget());
            if (NULL != pcImg) {
                pcImg->SetColor(uIndex2 == uSelectedIndex ? SELECTED_COLOR : UNSELECTED_COLOR);
            }
        }
    }
}
VOID CMADetails::RefreshToggles(UINT uIndex)
{
    if (NULL == m_apcToggles[uIndex]) {
        return;
    }
    const SMovingAverage& sMA = m_cChart.GetMovingAverage(uIndex);
    CUITextLabel* pcLbl = CUITextLabel::CastToMe(m_apcToggles[uIndex]->GetChild());
    if (NULL == pcLbl) {
        return;
    }
    switch (sMA.eSource) {
        case EMA_VALUE_OPEN:
            pcLbl->SetString("Open");
            break;
        case EMA_VALUE_CLOSE:
            pcLbl->SetString("Close");
            break;
        case EMA_VALUE_AVG:
            pcLbl->SetString("Avg");
            break;
        default:
            ASSERT(FALSE);
            break;
    }
}

VOID CMADetails::ChangeInterval(UINT uIndex)
{
    const SMovingAverage& sMA = m_cChart.GetMovingAverage(uIndex);
    snprintf(m_szBuffer, 64, "%d", sMA.uIntervalNum);
    CEvent cEvent(EEVENT_ON_TEXT_INPUT_COMPLETE, uIndex);
    CTextInputWindow* pcWin = new CTextInputWindow(m_szBuffer, 64, *this, cEvent, EInputType_Numeric);
    if (NULL != pcWin) {
        pcWin->DoModal();
    }
}

VOID CMADetails::OnTextInputComplete(UINT uIndex)
{
    if (0 == strlen(m_szBuffer)) {
        return;
    }
    const INT nInterval = atoi(m_szBuffer);
    if (0 >= nInterval || 5000 < nInterval) { // safety
        return;
    }
    SMovingAverage& sMA = m_cChart.GetMovingAverage(uIndex);
    sMA.uIntervalNum = nInterval;
    if (NULL != m_pcLbls[uIndex]) {
        CHAR szBuffer[512];
        snprintf(szBuffer, 512, "Line%d (%d)", uIndex + 1, sMA.uIntervalNum);
        m_pcLbls[uIndex]->SetString(szBuffer);
    }
    m_cChart.GenerateMA(uIndex);
    CPlayerData::SetMovingAverageData(uIndex, sMA.eSource, sMA.eType, sMA.uIntervalNum);
    CSaveData::SaveData(FALSE);
}
