#include "stdafx.h"
#include "CMain.h"
#include "Event.h"
#include "EventID.h"
#include "EventManager.h"
#include "OBVChartDefs.h"
#include "OBVDetails.h"
#include "PlayerData.h"
#include "SaveData.h"
#include "TextInputWindow.h"
#include "UIButton.h"

#define SELECTED_COLOR      RGBA(0x1F,0x1F,0xFF,0xFF)
#define UNSELECTED_COLOR    RGBA(0x7F, 0x7F, 0x7F, 0xFF)

#define CREATE_BTN(x, y, w, h, id, text) CreateColorButton(x, y, w, h, id, UNSELECTED_COLOR, text, TRUE)

#define WIN_W (890.0f)
#define WIN_H (130.0f)

struct SLabelInfo {
    UINT uColor;
    UINT auBtn[3];
    UINT uInterval;
};
static const SLabelInfo s_asLabels[2] = {
    { OBV_MAIN_LINE_MA_COLOR,       { COBVDetails::EBTN_LINE_1_SMA, COBVDetails::EBTN_LINE_1_EMA, COBVDetails::EBTN_LINE_1_NONE }, COBVDetails::EBTN_LINE_1_INTERVAL },
    { OBV_DIFFERENCE_LINE_MA_COLOR, { COBVDetails::EBTN_LINE_2_SMA, COBVDetails::EBTN_LINE_2_EMA, COBVDetails::EBTN_LINE_2_NONE }, COBVDetails::EBTN_LINE_2_INTERVAL },
};


COBVDetails::COBVDetails() :
CGameWindow()
{
    m_bDestroyOnExitModal = TRUE;
    memset(m_pcLbls, 0, sizeof(m_pcLbls));
    memset(m_apcBtns, 0, sizeof(m_apcBtns));
    memset(m_apcToggles, 0, sizeof(m_apcToggles));
}

COBVDetails::~COBVDetails()
{
}
VOID COBVDetails::InitializeInternals(VOID)
{
    CGameWindow::InitializeInternals();
    SetLocalPosition(CMain::GetDisplaySideMargin(), 0.0);
    SetLocalSize(WIN_W, WIN_H);
    m_cBG.Set9PartTexture(ETEX::ID::ui_9part_filter_off, 3.0f, 3.0f, 3.0f, 3.0f);
    m_cBG.SetLocalSize(WIN_W, WIN_H);
    AddChild(m_cBG);
    
    CREATE_BTN((WIN_W - 60.0f), WIN_H - 60.0f, 50.0f, 50.0f, EBTN_EXIT, "Quit");
    const UINT auEfxColor[2] = { RGBA(0, 0, 0, 0xFF), 0xFFFFFFFF };
    UINT uIndex = 0;
    for ( ; 2 > uIndex; ++uIndex) {
        const SLabelInfo& sLbl = s_asLabels[uIndex];
        
        const UINT uYIndex = uIndex % 2;
        const FLOAT fY = 35.0f + uYIndex * 60.0f;
        const FLOAT fX = 10.0f;
        CUITextLabel* pcLbl = CreateLabel("", ELABEL_COMMON_BUTTON_LABEL);
        if (NULL != pcLbl) {
            pcLbl->SetColor(sLbl.uColor);
            pcLbl->SetAnchor(0.0f, 0.5f);
            pcLbl->SetLocalPosition(fX, fY);
            pcLbl->SetEffectColor(auEfxColor[uIndex]);
            pcLbl->SetEffect(CUITextLabel::EFFECT_4SIDE_SHADOW, 1.0f, 1.0f);
            AddChild(*pcLbl);
        }
        m_pcLbls[uIndex] = pcLbl;
        UpdateCurveName(uIndex);
        m_apcBtns[uIndex][0] = CREATE_BTN(fX + 100.0f, fY - 25.0f, 50.0f, 50.0f, sLbl.auBtn[0], "SMA");
        m_apcBtns[uIndex][1] = CREATE_BTN(fX + 160.0f, fY - 25.0f, 50.0f, 50.0f, sLbl.auBtn[1], "EMA");
        m_apcBtns[uIndex][2] = CREATE_BTN(fX + 220.0f, fY - 25.0f, 50.0f, 50.0f, sLbl.auBtn[2], "None");
        CREATE_BTN(fX + 280.0f, fY - 25.0f, 100.0f, 50.0f, sLbl.uInterval, "Change\nInterval");
        RefreshBtns(uIndex);
    }
    
    CREATE_BTN(400.0f, 10.0f, 50.0f, 50.0f, EBTN_TOGGLE_OBV_TYPE, "Type\nChange");
}

VOID COBVDetails::Release(VOID)
{
    m_cBG.RemoveFromParent();
    CGameWindow::Release();
}

VOID COBVDetails::OnReceiveEvent(CEvent& cEvent)
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
        case EBTN_LINE_1_EMA:
            ChangeCurveType(0, EMA_EMA);
            break;
        case EBTN_LINE_2_EMA:
            ChangeCurveType(1, EMA_EMA);
            break;
        case EBTN_LINE_1_NONE:
            ChangeCurveType(0, EMA_NONE);
            break;
        case EBTN_LINE_2_NONE:
            ChangeCurveType(1, EMA_NONE);
            break;
        case EBTN_LINE_1_INTERVAL:
            ChangeInterval(0);
            break;
        case EBTN_LINE_2_INTERVAL:
            ChangeInterval(1);
            break;
        case EEVENT_ON_TEXT_INPUT_COMPLETE:
            OnTextInputComplete(cEvent.GetIntParam(1));
            break;
        case EBTN_TOGGLE_OBV_TYPE:
            OnOBVTypeChange();
            break;
        default:
            break;
    }
}

VOID COBVDetails::ChangeCurveType(UINT uIndex, EMA_TYPE eType)
{
    const SMovingAverageData& sMA = CPlayerData::GetOBV_MA(uIndex);
    CPlayerData::SetOBV_MA(uIndex, (EMA_VALUE_SOURCE)sMA.bySource, eType, sMA.ushInterval);
    CSaveData::SaveData(FALSE);
    CEventManager::PostGlobalEvent(EGLOBAL_TRADE_MAIN_WINDOW, CEvent(EGLOBAL_EVT_TRADE_MAIN_WINDOW_REFRESH_ATTACH_CHART, EGLOBAL_OBV_CHART));
    RefreshBtns(uIndex);
}
VOID COBVDetails::RefreshBtns(UINT uIndex)
{
    const SMovingAverageData& sMA = CPlayerData::GetOBV_MA(uIndex);
    UINT uSelectedIndex = 0;
    switch (sMA.byType) {
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

VOID COBVDetails::ChangeInterval(UINT uIndex)
{
    const SMovingAverageData& sMA = CPlayerData::GetOBV_MA(uIndex);
    snprintf(m_szBuffer, 64, "%d", sMA.ushInterval);
    CEvent cEvent(EEVENT_ON_TEXT_INPUT_COMPLETE, uIndex);
    CTextInputWindow* pcWin = new CTextInputWindow(m_szBuffer, 64, *this, cEvent, EInputType_Numeric);
    if (NULL != pcWin) {
        pcWin->DoModal();
    }
}

VOID COBVDetails::OnTextInputComplete(UINT uIndex)
{
    if (0 == strlen(m_szBuffer)) {
        return;
    }
    const INT nInterval = atoi(m_szBuffer);
    if (0 >= nInterval || 5000 < nInterval) { // safety
        return;
    }
    const SMovingAverageData& sMA = CPlayerData::GetOBV_MA(uIndex);
    CPlayerData::SetOBV_MA(uIndex, (EMA_VALUE_SOURCE)sMA.bySource, (EMA_TYPE)sMA.byType, nInterval);
    CSaveData::SaveData(FALSE);
    CEventManager::PostGlobalEvent(EGLOBAL_TRADE_MAIN_WINDOW, CEvent(EGLOBAL_EVT_TRADE_MAIN_WINDOW_REFRESH_ATTACH_CHART, EGLOBAL_OBV_CHART));
    UpdateCurveName(uIndex);
}

VOID COBVDetails::UpdateCurveName(UINT uIndex)
{
    CUITextLabel* pcLbl = m_pcLbls[uIndex%2];
    if (NULL != pcLbl) {
        const SMovingAverageData& sMA = CPlayerData::GetOBV_MA(uIndex);
        CHAR szBuffer[512];
        if (0 == uIndex) {
            snprintf(szBuffer, 512, "Main(%d)", sMA.ushInterval);
        }
        else {
            snprintf(szBuffer, 512, "Diff(%d)", sMA.ushInterval);
        }
        pcLbl->SetString(szBuffer);
    }
}

VOID COBVDetails::OnOBVTypeChange(VOID)
{
    CPlayerData::SetOBVUsingPriceMultiplier(!CPlayerData::IsOBVUsingPriceMultiplier());
    CSaveData::SaveData(FALSE);
    CEventManager::PostGlobalEvent(EGLOBAL_TRADE_MAIN_WINDOW, CEvent(EGLOBAL_EVT_TRADE_MAIN_WINDOW_REFRESH_ATTACH_CHART, EGLOBAL_OBV_CHART));
}
