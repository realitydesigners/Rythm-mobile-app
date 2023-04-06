#include "stdafx.h"
#include "CMain.h"
#include "Event.h"
#include "EventID.h"
#include "EventManager.h"
#include "MACDDetails.h"
#include "PlayerData.h"
#include "SaveData.h"
#include "TextInputWindow.h"
#include "UIButton.h"

#define SELECTED_COLOR      RGBA(0x1F,0x1F,0xFF,0xFF)
#define UNSELECTED_COLOR    RGBA(0x7F, 0x7F, 0x7F, 0xFF)

#define CREATE_BTN(x, y, w, h, id, text) CreateColorButton(x, y, w, h, id, UNSELECTED_COLOR, text, TRUE)

#define WIN_W (815.0f)
#define WIN_H (130.0f)

struct SLabelInfo {
    UINT uColor;
    UINT auBtn[2];
    UINT uToggle;
    UINT uInterval;
};
static const SLabelInfo s_asLabels[3] = {
    { MA_LINE_1_COLOR, { CMACDDetails::EBTN_LINE_1_SMA, CMACDDetails::EBTN_LINE_1_EMA },
        CMACDDetails::EBTN_LINE_1_TOGGLE, CMACDDetails::EBTN_LINE_1_INTERVAL },
    { MA_LINE_1_COLOR, { CMACDDetails::EBTN_LINE_2_SMA, CMACDDetails::EBTN_LINE_2_EMA },
        CMACDDetails::EBTN_LINE_2_TOGGLE, CMACDDetails::EBTN_LINE_2_INTERVAL },
    { MA_LINE_3_COLOR, { CMACDDetails::EBTN_LINE_3_SMA, CMACDDetails::EBTN_LINE_3_EMA },
    CMACDDetails::EBTN_LINE_3_TOGGLE, CMACDDetails::EBTN_LINE_3_INTERVAL },
};


CMACDDetails::CMACDDetails() :
CGameWindow()
{
    m_bDestroyOnExitModal = TRUE;
    memset(m_pcLbls, 0, sizeof(m_pcLbls));
    memset(m_apcBtns, 0, sizeof(m_apcBtns));
    memset(m_apcToggles, 0, sizeof(m_apcToggles));
}

CMACDDetails::~CMACDDetails()
{
}
VOID CMACDDetails::InitializeInternals(VOID)
{
    CGameWindow::InitializeInternals();
    SetLocalSize(WIN_W, WIN_H);
    m_cBG.Set9PartTexture(ETEX::ID::ui_9part_filter_off, 3, 3, 3, 3);
    m_cBG.SetLocalSize(WIN_W, WIN_H);
    AddChild(m_cBG);
    
    CREATE_BTN((WIN_W - 60.0f), WIN_H - 60.0f, 50.0f, 50.0f, EBTN_EXIT, "Quit");
    const SMACDParam& sParam = CPlayerData::GetMACDParam();
    const SMovingAverageData* psMA[3] = {
        &sParam.sMA1,
        &sParam.sMA2,
        &sParam.sSignal,
    };
    CHAR szBuffer[512];
    UINT uIndex = 0;
    for ( ; 3 > uIndex; ++uIndex) {
        const SMovingAverageData& sMA = *(psMA[uIndex]);
        if (2 != uIndex) {
            snprintf(szBuffer, 512, "L%d (%d)", uIndex + 1, sMA.ushInterval);
        }
        else {
            snprintf(szBuffer, 512, "Signal (%d)", sMA.ushInterval);
        }
        const SLabelInfo& sLbl = s_asLabels[uIndex];
        const UINT uYIndex = uIndex % 2;
        FLOAT fY = 35.0f + uYIndex * 60.0f;
        const UINT uXIndex = uIndex / 2;
        FLOAT fX = 10.0f + uXIndex * 410.0f;
        CUITextLabel* pcLbl = CreateLabel(szBuffer, ELABEL_COMMON_BUTTON_LABEL);
        if (NULL != pcLbl) {
            pcLbl->SetColor(sLbl.uColor);
            pcLbl->SetAnchor(0.0f, 0.5f);
            pcLbl->SetLocalPosition(fX, fY);
            AddChild(*pcLbl);
        }
        fY -= 25.0f;
        fX += 25.0f;
        m_pcLbls[uIndex] = pcLbl;
        m_apcBtns[uIndex][0] = CREATE_BTN(fX + 80.0f, fY, 50.0f, 50.0f, sLbl.auBtn[0], "SMA");
        m_apcBtns[uIndex][1] = CREATE_BTN(fX + 140.0f, fY, 50.0f, 50.0f, sLbl.auBtn[1], "EMA");
        m_apcToggles[uIndex] = CREATE_BTN(fX + 200.0f, fY, 50.0f, 50.0f, sLbl.uToggle,  "Toggle");
        CREATE_BTN(fX + 260.0f, fY, 100.0f, 50.0f, sLbl.uInterval, "Change\nInterval");
        
        RefreshBtns(uIndex, (EMA_TYPE)sMA.byType);
        RefreshToggles(uIndex, (EMA_VALUE_SOURCE)sMA.bySource);
    }
}

VOID CMACDDetails::Release(VOID)
{
    m_cBG.RemoveFromParent();
    CGameWindow::Release();
}

VOID CMACDDetails::OnReceiveEvent(CEvent& cEvent)
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

static SMovingAverageData& GetMA(UINT uIndex, SMACDParam& sParam)
{
    switch (uIndex) {
        case 0:
            return sParam.sMA1;
        case 1:
            return sParam.sMA2;
        case 2:
            return sParam.sSignal;
        default:
            ASSERT(FALSE);
            break;
    }
    return sParam.sMA1;
}
static const SMovingAverageData& GetMA(UINT uIndex, const SMACDParam& sParam)
{
    switch (uIndex) {
        case 0:
            return sParam.sMA1;
        case 1:
            return sParam.sMA2;
        case 2:
            return sParam.sSignal;
        default:
            ASSERT(FALSE);
            break;
    }
    return sParam.sMA1;
}
VOID CMACDDetails::ChangeCurveType(UINT uIndex, EMA_TYPE eType)
{
    SMACDParam sParam = CPlayerData::GetMACDParam(); // make copy
    SMovingAverageData& sMA = GetMA(uIndex, sParam);
    sMA.byType = eType;
    CPlayerData::SetMACDParam(sParam);
    CSaveData::SaveData(FALSE);
    RefreshBtns(uIndex, eType);
    CEventManager::PostGlobalEvent(EGLOBAL_TRADE_MAIN_WINDOW, CEvent(EGLOBAL_EVT_TRADE_MAIN_WINDOW_REFRESH_ATTACH_CHART, EGLOBAL_MACD_CHART));
}
VOID CMACDDetails::ToggleSourceType(UINT uIndex)
{
    SMACDParam sParam = CPlayerData::GetMACDParam(); // make copy
    SMovingAverageData& sMA = GetMA(uIndex, sParam);
    sMA.bySource = (sMA.bySource + 1) % EMA_VALUE_SOURCE_NUM;
    CPlayerData::SetMACDParam(sParam);
    CSaveData::SaveData(FALSE);
    RefreshToggles(uIndex, (EMA_VALUE_SOURCE)sMA.bySource);
    CEventManager::PostGlobalEvent(EGLOBAL_TRADE_MAIN_WINDOW, CEvent(EGLOBAL_EVT_TRADE_MAIN_WINDOW_REFRESH_ATTACH_CHART, EGLOBAL_MACD_CHART));
}

VOID CMACDDetails::RefreshBtns(UINT uIndex, EMA_TYPE eType)
{
    UINT uSelectedIndex = 0;
    switch (eType) {
        case EMA_SMA:
            break;
        case EMA_EMA:
            uSelectedIndex = 1;
            break;
        default:
            ASSERT(FALSE);
            break;
    }
    UINT uIndex2 = 0;
    for ( ; 2 > uIndex2; ++uIndex2) {
        if (NULL != m_apcBtns[uIndex]) {
            CUIImage* pcImg = CUIImage::CastToMe(m_apcBtns[uIndex][uIndex2]->GetIdleWidget());
            if (NULL != pcImg) {
                pcImg->SetColor(uIndex2 == uSelectedIndex ? SELECTED_COLOR : UNSELECTED_COLOR);
            }
        }
    }
}
VOID CMACDDetails::RefreshToggles(UINT uIndex, EMA_VALUE_SOURCE eSource)
{
    if (NULL == m_apcToggles[uIndex]) {
        return;
    }
    CUITextLabel* pcLbl = CUITextLabel::CastToMe(m_apcToggles[uIndex]->GetChild());
    if (NULL == pcLbl) {
        return;
    }
    switch (eSource) {
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

VOID CMACDDetails::ChangeInterval(UINT uIndex)
{
    const SMACDParam& sParam = CPlayerData::GetMACDParam(); // make copy
    const SMovingAverageData& sMA = GetMA(uIndex, sParam);
    snprintf(m_szBuffer, 64, "%d", sMA.ushInterval);
    CEvent cEvent(EEVENT_ON_TEXT_INPUT_COMPLETE, uIndex);
    CTextInputWindow* pcWin = new CTextInputWindow(m_szBuffer, 64, *this, cEvent, EInputType_Numeric);
    if (NULL != pcWin) {
        pcWin->DoModal();
    }
}

VOID CMACDDetails::OnTextInputComplete(UINT uIndex)
{
    if (0 == strlen(m_szBuffer)) {
        return;
    }
    const INT nInterval = atoi(m_szBuffer);
    if (0 >= nInterval || 5000 < nInterval) { // safety
        return;
    }
    SMACDParam sParam = CPlayerData::GetMACDParam(); // make copy
    SMovingAverageData& sMA = GetMA(uIndex, sParam);
    sMA.ushInterval = nInterval;
    if (NULL != m_pcLbls[uIndex]) {
        CHAR szBuffer[512];
        snprintf(szBuffer, 512, "Line%d (%d)", uIndex + 1, sMA.ushInterval);
        m_pcLbls[uIndex]->SetString(szBuffer);
    }
    CPlayerData::SetMACDParam(sParam);
    CSaveData::SaveData(FALSE);
    CEventManager::PostGlobalEvent(EGLOBAL_TRADE_MAIN_WINDOW, CEvent(EGLOBAL_EVT_TRADE_MAIN_WINDOW_REFRESH_ATTACH_CHART, EGLOBAL_MACD_CHART));
}
