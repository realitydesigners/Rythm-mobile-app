#include "stdafx.h"
#include "Event.h"
#include "EventManager.h"
#include "GameRenderer.h"
#include "MegaZZ.h"
#include "OandaMgr.h"
#include "RythmDashboardWidget.h"
#include "TextUtil.h"
#include "TradeLogDefs.h"

#define SELECTED_NAME_FONT      EGAMEFONT_SIZE_16
#define NOT_SELECTED_NAME_FONT  EGAMEFONT_SIZE_14

#define SELECTED_NAME_FONT2     EGAMEFONT_SIZE_12
#define NOT_SELECTED_NAME_FONT2 EGAMEFONT_SIZE_10

#define SELECTED_PRICE_FONT     EGAMEFONT_SIZE_14
#define NOT_SELECTED_PRICE_FONT EGAMEFONT_SIZE_12

#define NUMBERS_FONT            EGAMEFONT_SIZE_16

#define UP_BOX_COLOR            RGBA(140,140,140,0xFF)
#define DOWN_BOX_COLOR          RGBA(70,70,70,0xFF)
#define NORMAL_NUMBER_COLOR     RGBA(0xFF,0xFF,0xFF,0xFF)
#define RETRACE_UP_COLOR        RGBA(110,210,230,0xFF)
#define RETRACE_DOWN_COLOR      RGBA(255,110,110,0xFF)

#define RED_NAME_SELECTED_COLOR         RGBA(0xFF,0x4F,0x4F,0xFF)
#define RED_NAME_NOT_SELECTED_COLOR     RGBA(0xFF,0x0,0x0,0xFF)


VOID CRythmDashboardWidget::PostEvent(CEvent& cEvent, FLOAT fDelay)
{   // Do not post to parent
    CEventManager::PostEvent(*this, cEvent, fDelay);
}

VOID CRythmDashboardWidget::OnReceiveEvent(CEvent& cEvent)
{
    OnTapped();
}

CRythmDashboardWidget::CRythmDashboardWidget(UINT uTradeIndex, UINT uParentHandlerID, UINT uEventID) :
CUIContainer(),
m_uTradeIndex(uTradeIndex),
m_uParentHandlerID(uParentHandlerID),
m_uEventID(uEventID),
m_bSelected(FALSE),
m_bSpreadAbove3(FALSE),
m_fAdditionalSpacing(0.0f),
m_uDisplayNum(9),
m_uBiggestDepthIndex(0),
INITIALIZE_TEXT_LABEL(m_cName1),
INITIALIZE_TEXT_LABEL(m_cName2),
INITIALIZE_TEXT_LABEL(m_cPrice)
{
    memset(m_apcBoxLbls, 0, sizeof(m_apcBoxLbls));
    memset(m_apcRLbls, 0, sizeof(m_apcRLbls));
    memset(m_abVisible, 1, sizeof(m_abVisible));
}
    
CRythmDashboardWidget::~CRythmDashboardWidget()
{
    
}
VOID CRythmDashboardWidget::ToggleVisibility(const BOOLEAN* pbVisible)
{
    memcpy(m_abVisible, pbVisible, sizeof(m_abVisible));
}
    
VOID CRythmDashboardWidget::Initialize(VOID)
{    
    const FLOAT fTextY = RYTHM_DASHBOARD_WIDGET_HEIGHT * 0.5f;
    
    m_cInvImg.SetLocalSize(RYTHM_DASHBOARD_WIDGET_WIDTH, RYTHM_DASHBOARD_WIDGET_HEIGHT);
    m_cInvImg.SetVisibility(FALSE);
    m_cBtn.SetDisplayWidgets(m_cInvImg, m_cInvImg);
    AddChild(m_cBtn);
    
    m_cName1.SetFont(NOT_SELECTED_NAME_FONT);
    m_cName1.SetColor(RYTHM_UI_NOT_SELECTED_FONT_COLOR);
    m_cName1.SetAnchor(0.0f, 0.5f);
    m_cName1.SetLocalPosition(RYTHM_DASHBOARD_WIDGET_NAME_X, fTextY);
    m_cName1.SetString("-------");
    m_cBtn.AddChild(m_cName1);
    m_cName2.SetFont(NOT_SELECTED_NAME_FONT2);
    m_cName2.SetColor(RYTHM_UI_NOT_SELECTED_FONT_COLOR);
    m_cName2.SetAnchor(0.0f, 0.5f);
    m_cBtn.AddChild(m_cName2);
    
    m_cPrice.SetFont(NOT_SELECTED_PRICE_FONT);
    m_cPrice.SetAnchor(0.0f, 0.5f);
    m_cPrice.SetColor(RYTHM_UI_NOT_SELECTED_FONT_COLOR);
    m_cPrice.SetLocalPosition(RYTHM_DASHBOARD_WIDGET_PRICE_X, fTextY);
    m_cBtn.AddChild(m_cPrice);
    
    const FLOAT fBtnDimensions = RYTHM_DASHBOARD_WIDGET_HEIGHT - 2.0f;
    FLOAT fX = RYTHM_DASHBOARD_WIDGET_BTN_START_X + 1.0f;
    for (UINT uIndex = 0; m_uDisplayNum > uIndex; ++uIndex) {
        CUIImage& cImg = m_acBoxImg[uIndex];
        cImg.SetLocalSize(fBtnDimensions, fBtnDimensions);
        cImg.SetLocalPosition(fX, 1.0f);
        m_cBtn.AddChild(cImg);
        
        CUITextLabel* pcLbl = NEW_TEXT_LABEL;
        pcLbl->SetFont(EGAMEFONT_SIZE_12);
        pcLbl->SetAnchor(0.0f, 0.0f);
        pcLbl->SetLocalPosition(fX + 2.0f, -1.0f);
        pcLbl->SetColor(NORMAL_NUMBER_COLOR);
        m_cBtn.AddChild(*pcLbl);
        m_apcRLbls[uIndex] = pcLbl;
        
        pcLbl = NEW_TEXT_LABEL;
        pcLbl->SetFont(NUMBERS_FONT);
        pcLbl->SetAnchor(0.5f, 0.5f);
        FLOAT fLblX = fX + fBtnDimensions * 0.5f + 2.0f;
        FLOAT fLblY = fBtnDimensions * 0.5f + 1.0f;
        fLblX += 2.0f;
        fLblY += 2.0f;
        pcLbl->SetLocalPosition(fLblX, fLblY);
        pcLbl->SetColor(NORMAL_NUMBER_COLOR);
        m_cBtn.AddChild(*pcLbl);
        m_apcBoxLbls[uIndex] = pcLbl;
        
        fX += RYTHM_DASHBOARD_WIDGET_HEIGHT + m_fAdditionalSpacing;
    }
    Clear();
}

VOID CRythmDashboardWidget::HideNameAndPrice(VOID)
{
    m_cPrice.SetVisibility(FALSE);
    m_cPrice.RemoveFromParent();
    m_cName1.SetVisibility(FALSE);
    m_cName1.RemoveFromParent();
    m_cName2.SetVisibility(FALSE);
    m_cName2.RemoveFromParent();
}

VOID CRythmDashboardWidget::UpdateTradePairName(const CHAR* szName)
{
    CTextUtil::Tokenize(szName, "_");
    const CHAR* szFirst = CTextUtil::NextToken();
    if (NULL != szFirst) {
        m_cName1.SetString(szFirst);
        const CHAR* szSecond = CTextUtil::NextToken();
        if (NULL != szSecond) {
            m_cName2.SetString(szSecond);
        }
        else {
            m_cName2.ClearString();
        }
    }
    else {
        m_cName1.SetString(szName);
        m_cName2.ClearString();
    }
    RecalculateNamePosition();
}

VOID CRythmDashboardWidget::Release(VOID)
{
    m_cBtn.RemoveFromParent();
    
    m_cName1.Release();
    m_cName1.RemoveFromParent();
    m_cName2.Release();
    m_cName2.RemoveFromParent();
    m_cPrice.Release();
    m_cPrice.RemoveFromParent();
    
    for (UINT uIndex = 0; 9 > uIndex; ++uIndex) {
        SAFE_REMOVE_RELEASE_DELETE(m_apcBoxLbls[uIndex]);
        SAFE_REMOVE_RELEASE_DELETE(m_apcRLbls[uIndex]);
        m_acBoxImg[uIndex].RemoveFromParent();
    }
    CUIContainer::Release();
}

VOID CRythmDashboardWidget::Clear(VOID)
{
    for (UINT uIndex = 0; m_uDisplayNum > uIndex; ++uIndex) {
        m_acBoxImg[uIndex].SetColor(DOWN_BOX_COLOR);
        if (NULL != m_apcBoxLbls[uIndex]) {
            m_apcBoxLbls[uIndex]->SetString("-");
        }
        if (NULL != m_apcRLbls[uIndex]) {
            m_apcRLbls[uIndex]->SetString("-");
        }
    }
    m_cPrice.SetString("------");
    m_bSelected = FALSE;
}

VOID CRythmDashboardWidget::Update(const CMegaZZ& cMegaZZ)
{
    if (ESOURCE_MARKET_OANDA == cMegaZZ.GetSource()) {
        const COandaMgr::SOandaPair* psPair = COandaMgr::GetInstance().GetTradePair(cMegaZZ.GetTradePairIndex());
        if (NULL != psPair) {
            m_bSpreadAbove3 = ((psPair->fAskPrice - psPair->fBidPrice) * psPair->dPipMultiplier) >= 3.0f;
            if (m_bSelected) {
                SetSelected();
            }
            else {
                ClearSelection();
            }
        }
    }
    CHAR szBuffer[64];
    for (UINT uIndex = 0; m_uDisplayNum > uIndex; ++uIndex) {
        const UINT uDepthIndex = m_uBiggestDepthIndex + uIndex;
        CUITextLabel* pcLbl = m_apcBoxLbls[uIndex];
        if (NULL == pcLbl) {
            ASSERT(FALSE);
            continue;
        }
        if (!m_abVisible[uIndex]) { // if not visible
            pcLbl->ClearString();
            m_acBoxImg[uIndex].SetColor(RGBA(0x1F, 0x1F, 0x1F, 0xFF));
            m_acBoxImg[uIndex].SetBorderColor(0);
            pcLbl = m_apcRLbls[uIndex];
            if (NULL != pcLbl) {
                pcLbl->ClearString();
            }
            continue;
        }
        if (MEGAZZ_MAX_DEPTH <= uDepthIndex) {
            pcLbl->ClearString();
        }
        else {
            const MegaZZ::SChannel& sChannel = cMegaZZ.GetCurrentChannel(uDepthIndex);
            const UINT uRColor = (MegaZZ::EType_Down == sChannel.byType) ? RETRACE_DOWN_COLOR : RETRACE_UP_COLOR;
            m_acBoxImg[uIndex].SetBorderColor(DOWN_BOX_COLOR);
            m_acBoxImg[uIndex].SetColor(RGBA(0x1F, 0x1F, 0x1F, 0xFF));
            UINT uColor = NORMAL_NUMBER_COLOR;
            if (7 <= sChannel.byDisplayNumber) {
                uColor = uRColor;
                pcLbl->SetEffect(CUITextLabel::EFFECT_SHADOW, 1.0f, 1.0f);
            }
            else {
                pcLbl->SetEffect(CUITextLabel::EFFECT_NONE, 0.0f, 0.0f);
            }
            snprintf(szBuffer, 32, "%d", sChannel.byDisplayNumber);
            pcLbl->SetString(szBuffer);
            pcLbl->SetColor(uColor);
        }
        pcLbl = m_apcRLbls[uIndex];
        if (NULL == pcLbl) {
            continue;
        }
        if (MEGAZZ_MAX_DEPTH <= uDepthIndex) {
            pcLbl->ClearString();
        }
        else {
            const MegaZZ::SChannel& sChannel = cMegaZZ.GetCurrentChannel(uDepthIndex);
            const UINT uRColor = (MegaZZ::EType_Down == sChannel.byType) ? RETRACE_DOWN_COLOR : RETRACE_UP_COLOR;

            UINT uColor = NORMAL_NUMBER_COLOR;
            if (7 <= sChannel.byRetracementNumber) {
                uColor = uRColor;
                pcLbl->SetEffect(CUITextLabel::EFFECT_SHADOW, 1.0f, 1.0f);
            }
            else {
                pcLbl->SetEffect(CUITextLabel::EFFECT_NONE, 0.0f, 0.0f);
            }
            snprintf(szBuffer, 32, "%d", sChannel.byRetracementNumber);
            pcLbl->SetString(szBuffer);
            pcLbl->SetColor(uColor);
        }
    }
}
VOID CRythmDashboardWidget::Update(const STradeLog& sLog, BOOLEAN bOpen)
{
    const MegaZZ::SChannel* psChannels = bOpen ? sLog.asStart : sLog.asEnd;
    CHAR szBuffer[64];
    for (UINT uIndex = 0; m_uDisplayNum > uIndex; ++uIndex) {
        const UINT uDepthIndex = m_uBiggestDepthIndex + uIndex;
        const MegaZZ::SChannel& sChannel = psChannels[uDepthIndex];
        const UINT uBoxColor = (MegaZZ::EType_Down == sChannel.byType) ? DOWN_BOX_COLOR : UP_BOX_COLOR;
        const UINT uRColor = (MegaZZ::EType_Down == sChannel.byType) ? RETRACE_DOWN_COLOR : RETRACE_UP_COLOR;
        m_acBoxImg[uIndex].SetColor(uBoxColor);
        CUITextLabel* pcLbl = m_apcBoxLbls[uIndex];
        if (NULL == pcLbl) {
            ASSERT(FALSE);
            continue;
        }
        UINT uColor = NORMAL_NUMBER_COLOR;
        if (6 <= sChannel.byDisplayNumber) {
            uColor = uRColor;
            pcLbl->SetEffect(CUITextLabel::EFFECT_SHADOW, 1.0f, 1.0f);
        }
        else {
            pcLbl->SetEffect(CUITextLabel::EFFECT_NONE, 0.0f, 0.0f);
        }
        snprintf(szBuffer, 32, "%d", sChannel.byDisplayNumber);
        pcLbl->SetString(szBuffer);
        pcLbl->SetColor(uColor);
        
        pcLbl = m_apcRLbls[uIndex];
        if (NULL == pcLbl) {
            continue;
        }
        uColor = NORMAL_NUMBER_COLOR;
        if (6 <= sChannel.byRetracementNumber) {
            uColor = uRColor;
            pcLbl->SetEffect(CUITextLabel::EFFECT_SHADOW, 1.0f, 1.0f);
        }
        else {
            pcLbl->SetEffect(CUITextLabel::EFFECT_NONE, 0.0f, 0.0f);
        }
        snprintf(szBuffer, 32, "%d", sChannel.byRetracementNumber);
        pcLbl->SetString(szBuffer);
        pcLbl->SetColor(uColor);

    }
}

VOID CRythmDashboardWidget::DisplayMessage(const CHAR* szMsg)
{
    m_cPrice.SetString(szMsg);
}

VOID CRythmDashboardWidget::SetSelected(VOID)
{
    if (!m_bSelected) {
        m_bSelected = TRUE;
        m_cName1.SetFont(SELECTED_NAME_FONT);
        m_cName2.SetFont(SELECTED_NAME_FONT2);
        m_cPrice.SetFont(SELECTED_PRICE_FONT);
        RecalculateNamePosition();
    }
    const UINT uColor = m_bSpreadAbove3 ? RED_NAME_SELECTED_COLOR : RYTHM_UI_SELECTED_FONT_COLOR;
    m_cName1.SetColor(uColor);
    m_cName2.SetColor(uColor);
    m_cPrice.SetColor(uColor);
}

VOID CRythmDashboardWidget::ClearSelection(VOID)
{
    if (m_bSelected) {
        m_bSelected = FALSE;
        m_cName1.SetFont(NOT_SELECTED_NAME_FONT);
        m_cName2.SetFont(NOT_SELECTED_NAME_FONT2);
        m_cPrice.SetFont(NOT_SELECTED_PRICE_FONT);
        RecalculateNamePosition();
    }
    const UINT uColor = m_bSpreadAbove3 ? RED_NAME_NOT_SELECTED_COLOR : RYTHM_UI_NOT_SELECTED_FONT_COLOR;
    m_cName1.SetColor(uColor);
    m_cName2.SetColor(uColor);
    m_cPrice.SetColor(uColor);
}

VOID CRythmDashboardWidget::RecalculateNamePosition(VOID)
{
    m_cName1.OnUpdate(0.0f);
    const SHLVector2D& sSize = m_cName1.GetLocalSize();
    m_cName2.SetLocalPosition(RYTHM_DASHBOARD_WIDGET_NAME_X + sSize.x, RYTHM_DASHBOARD_WIDGET_HEIGHT * 0.5f);
}

VOID CRythmDashboardWidget::OnTapped(VOID)
{
    SetSelected();
    CEvent cEvent(m_uEventID, m_uTradeIndex);
    if (0 != m_uParentHandlerID) {
        CEventManager::PostGlobalEvent(m_uParentHandlerID, cEvent);
    }
    else {
        CUIContainer* pcParent = GetParent();
        if (NULL != pcParent) {
            pcParent->PostEvent(cEvent, 0.0f);
        }
    }
}
