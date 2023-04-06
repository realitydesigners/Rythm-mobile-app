#include "stdafx.h"
#include "EventManager.h"
#include "PlayerData.h"
#include "RefreshOptions.h"
#include "SaveData.h"

#define BG_COLOR                RGBA(0x4F,  0x4F,   0x4F,   0xFF)
#define BTN_SELECTED_COLOR      RGBA(0x7F,  0x7F,   0x7F,   0xFF)
#define BTN_NOT_SELECTED_COLOR  RGBA(0x2F,  0x2F,   0x2F,   0xFF)

#define BTN_WIDTH           (40.0f)
#define BTN_HEIGHT          (20.0f)
#define BTN_SPACING         (5.0f)
#define TOTAL_BTN_SPACE     (5*BTN_WIDTH + 4*BTN_SPACING)
#define PADDING             (5.0f)
#define TOP_PADDING         (20.0f)
#define TOTAL_HEIGHT        (BTN_HEIGHT + PADDING + TOP_PADDING)
#define TOTAL_WIDTH         (TOTAL_BTN_SPACE + PADDING * 2.0f)

CRefreshOptions::CRefreshOptions() :
CUIContainer(),
INITIALIZE_TEXT_LABEL(m_cTitle)
{
    memset(m_apcLbls, 0, sizeof(m_apcLbls));
}
    
CRefreshOptions::~CRefreshOptions()
{
    
}
VOID CRefreshOptions::PostEvent(CEvent& cEvent, FLOAT fDelay)
{
    CEventManager::PostEvent(*this, cEvent, fDelay);
}

BOOLEAN CRefreshOptions::Initialize(VOID)
{
    SetLocalSize(TOTAL_WIDTH, TOTAL_HEIGHT);
    m_cBG.SetColor(BG_COLOR);
    m_cBG.SetLocalSize(TOTAL_WIDTH, TOTAL_HEIGHT);
    AddChild(m_cBG);
    
    m_cTitle.SetString("Refresh Intervals");
    m_cTitle.SetAnchor(0.0f, 0.0f);
    m_cTitle.SetFont(EGAMEFONT_SIZE_14);
    m_cTitle.SetEffect(CUITextLabel::EFFECT_SHADOW, 1.0f, 1.0f);
    m_cTitle.SetLocalPosition(PADDING, 0.0f);
    AddChild(m_cTitle);
    
    m_cSelectedImg.SetLocalSize(BTN_WIDTH, BTN_HEIGHT);
    m_cSelectedImg.SetColor(BTN_SELECTED_COLOR);
    m_cNotSelectedImg.SetLocalSize(BTN_WIDTH, BTN_HEIGHT);
    m_cNotSelectedImg.SetColor(BTN_NOT_SELECTED_COLOR);
    
    ASSERT(ERythmInterval_Num == 5);
    const CHAR* szString[5] = {
        "2s", "5s", "10s", "15s", "30s"
    };
    UINT uSelectedIndex = CPlayerData::GetRythmFetchInterval();
    for (UINT uIndex = 0; 5 > uIndex; ++uIndex) {
        CUITextLabel* pcLbl = NEW_TEXT_LABEL;
        if (NULL == pcLbl) {
            ASSERT(FALSE);
            return FALSE;
        }
        pcLbl->SetString(szString[uIndex]);
        pcLbl->SetAnchor(0.5f, 0.5f);
        pcLbl->SetFont(EGAMEFONT_SIZE_14);
        pcLbl->SetEffect(CUITextLabel::EFFECT_SHADOW, 1.0f, 1.0f);
        pcLbl->SetLocalPosition(BTN_WIDTH * 0.5f, BTN_HEIGHT * 0.5f);
        CUIWidget& cWidget = (uSelectedIndex == uIndex) ? m_cSelectedImg : m_cNotSelectedImg;
        m_acBtns[uIndex].SetDisplayWidgets(cWidget, cWidget);
        m_acBtns[uIndex].SetID(uIndex);
        m_acBtns[uIndex].SetLocalPosition(PADDING + uIndex * (BTN_WIDTH + BTN_SPACING), TOP_PADDING);
        m_acBtns[uIndex].AddChild(*pcLbl);
        AddChild(m_acBtns[uIndex]);
        m_apcLbls[uIndex] = pcLbl;
    }
    return TRUE;
}
VOID CRefreshOptions::Release(VOID)
{
    m_cBG.RemoveFromParent();
    m_cTitle.Release();
    m_cTitle.RemoveFromParent();
    for (UINT uIndex = 0; 5 > uIndex; ++uIndex) {
        if (NULL != m_apcLbls[uIndex]) {
            m_apcLbls[uIndex]->Release();
            m_apcLbls[uIndex]->RemoveFromParent();
            delete m_apcLbls[uIndex];
            m_apcLbls[uIndex] = NULL;
        }
        m_acBtns[uIndex].RemoveFromParent();
    }
    
    CUIContainer::Release();
}

VOID CRefreshOptions::OnReceiveEvent(CEvent& cEvent)
{
    ASSERT(ERythmInterval_Num == 5);
    const UINT uResultIndex = cEvent.GetIntParam(0);
    if (5 <= uResultIndex) {
        ASSERT(FALSE);
        return;
    }
    CPlayerData::SetRythmFetchInterval((ERythmInterval)uResultIndex);
    CSaveData::SaveData(FALSE);
    for (UINT uIndex = 0; 5 > uIndex; ++uIndex) {
        CUIImage& cImg = (uIndex == uResultIndex) ? m_cSelectedImg : m_cNotSelectedImg;
        m_acBtns[uIndex].SetDisplayWidgets(cImg, cImg);
    }
}
