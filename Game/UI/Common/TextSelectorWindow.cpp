#include "stdafx.h"
#include "CMain.h"
#include "Event.h"
#include "EventHandler.h"
#include "EventID.h"
#include "EventManager.h"
#include "TextSelectorWindow.h"

CTextSelectorWindow::CTextSelectorWindow(CEventHandler& cHandler, UINT uEventID, UINT uParam) :
CGameWindow(),
m_cHandler(cHandler),
m_uEventID(uEventID),
m_uParam(uParam),
m_uCurrentSelectedIndex(0),
m_uSelectionNum(0),
m_ppcTextLabels(NULL),
m_pcBtns(NULL)
{
    m_bDestroyOnExitModal = TRUE;
}

CTextSelectorWindow::~CTextSelectorWindow()
{
}

VOID CTextSelectorWindow::Release(VOID)
{
    m_cBG.RemoveFromParent();
    if (NULL != m_ppcTextLabels) {
        for (UINT uIndex = 0; m_uSelectionNum > uIndex; ++uIndex) {
            SAFE_REMOVE_RELEASE_DELETE(m_ppcTextLabels[uIndex]);
        }
        SAFE_DELETE_ARRAY(m_ppcTextLabels);
    }
    if (NULL != m_pcBtns) {
        for (UINT uIndex = 0; m_uSelectionNum > uIndex; ++uIndex) {
            m_pcBtns[uIndex].RemoveFromParent();
        }
        SAFE_DELETE_ARRAY(m_pcBtns);
    }
    
    m_cScroller.RemoveFromParent();
    CGameWindow::Release();
}

VOID CTextSelectorWindow::OnReceiveEvent(CEvent& cEvent)
{
    const UINT uEventID = cEvent.GetIntParam(0);
    switch (uEventID) {
        default:
            OnBtnPressed(uEventID - EBTN_CHANGE_BTN);
            break;
    }
}

VOID CTextSelectorWindow::OnBtnPressed(UINT uIndex)
{
    if (m_uCurrentSelectedIndex != uIndex) {
        CEventManager::PostEvent(m_cHandler, CEvent(m_uEventID, m_uParam, uIndex));
    }
    ExitModal();
}

BOOLEAN CTextSelectorWindow::Initialize(const CHAR** ppszSelections, UINT uSelectionNum, UINT uCurrentSelectedIndex)
{
    m_ppcTextLabels = new CUITextLabel*[uSelectionNum];
    if (NULL == m_ppcTextLabels) {
        ASSERT(FALSE);
        return FALSE;
    }
    memset(m_ppcTextLabels, 0, sizeof(CUITextLabel*) * uSelectionNum);
    m_pcBtns = new CUIButton[uSelectionNum];
    if (NULL == m_pcBtns) {
        ASSERT(FALSE);
        return FALSE;
    }
    FLOAT fWidth = 0.0f;
    for (UINT uIndex = 0; uSelectionNum > uIndex; ++uIndex) {
        CUITextLabel* pcLbl = NEW_TEXT_LABEL;
        if (NULL == pcLbl) {
            ASSERT(FALSE);
            return FALSE;
        }
        pcLbl->SetFont(EGAMEFONT_SIZE_14);
        pcLbl->SetEffect(CUITextLabel::EFFECT_SHADOW, 1.0f, 1.0f);
        pcLbl->SetString(ppszSelections[uIndex]);
        pcLbl->SetAnchor(0.5f, 0.5f);
        pcLbl->AutoSize();
        pcLbl->OnUpdate(0.0f);
        const SHLVector2D& sSize = pcLbl->GetLocalSize();
        if (fWidth < sSize.x) {
            fWidth = sSize.x;
        }
        m_ppcTextLabels[uIndex] = pcLbl;
    }
    fWidth += 10.0f;
    const FLOAT fHeight = 20.0f;
    m_cSelected.SetLocalSize(fWidth, fHeight);
    m_cSelected.SetColor(RGBA(0x7F, 0x7F, 0x7F, 0xFF));
    
    m_cNotSelected.SetLocalSize(fWidth, fHeight);
    m_cNotSelected.SetColor(RGBA(0x4F, 0x4F, 0x4F, 0xFF));
    
    FLOAT fY = 5.0f;
    FLOAT fSelectedY = 0.0f;
    for (UINT uIndex = 0; uSelectionNum > uIndex; ++uIndex) {
        CUIButton& cBtn = m_pcBtns[uIndex];
        CUIWidget& cWidget = uCurrentSelectedIndex == uIndex ? m_cSelected : m_cNotSelected;
        cBtn.SetDisplayWidgets(cWidget, cWidget);
        cBtn.SetLocalPosition(5.0f, fY);
        if (uCurrentSelectedIndex == uIndex) {
            fSelectedY = fY;
        }
        
        CUITextLabel* pcLbl = m_ppcTextLabels[uIndex];
        pcLbl->SetLocalPosition(fWidth * 0.5f, fHeight * 0.5f);
        cBtn.AddChild(*pcLbl);
        cBtn.SetID(EBTN_CHANGE_BTN + uIndex);
        fY += fHeight + 5.0f;
    }
    m_cBG.SetColor(RGBA(0x1F, 0x1F, 0x1F, 0xFF));
    m_cBG.SetBorderColor(RGBA(0x7F, 0x7F, 0x7F, 0xFF));
    if (450.0f < fY) {
        fY = 450.0f;
    }
    m_cBG.SetLocalSize(fWidth + 10.0f, fY);
    m_uSelectionNum = uSelectionNum;
    m_uCurrentSelectedIndex = uCurrentSelectedIndex;
    m_cScroller.SetLocalSize(fSelectedY, fSelectedY);
    return TRUE;
}

VOID CTextSelectorWindow::InitializeInternals(VOID)
{
    CGameWindow::InitializeInternals();
    const SHLVector2D& sSize = m_cBG.GetLocalSize();
    SetLocalSize(sSize.x, sSize.y);
    AddChild(m_cBG);
    const FLOAT fScrollerOffset = m_cScroller.GetLocalSize().x;
    m_cScroller.SetLocalSize(sSize.x, sSize.y);
    AddChild(m_cScroller);
    
    const FLOAT fW = CMain::GetScreenWidth() - 2.0f * CMain::GetDisplaySideMargin() - 5.0f;
    const FLOAT fH = CMain::GetScreenHeight() - CMain::GetDisplayBottomMargin() - 5.0f;
    SHLVector2D sPos = GetLocalPosition();
    if ((sPos.x + sSize.x) > fW) {
        sPos.x = fW - sSize.x;
    }
    if (sPos.y + sSize.y > fH) {
        sPos.y = fH - sSize.y;
    }
    SetLocalPosition(sPos.x, sPos.y);
    
    for (UINT uIndex = 0; m_uSelectionNum > uIndex; ++uIndex) {
        CUIButton& cBtn = m_pcBtns[uIndex];
        m_cScroller.AddChild(cBtn);
    }
    m_cScroller.TrySetOffSet(fScrollerOffset - 10.0f);
}
