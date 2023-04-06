#include "stdafx.h"
#include "EditableCondition.h"
#include "Event.h"
#include "EventManager.h"
#include "TextSelectorWindow.h"

VOID CEditableCondition::PostEvent(CEvent& cEvent, FLOAT fDelay)
{
    CEventManager::PostEvent(*this, cEvent, fDelay);
}

CEditableCondition::CEditableCondition() :
CUIContainer(),
m_uIndex(0),
m_psRow(NULL),
m_uRefreshEvent(0)
{
    memset(m_apcTxt, 0, sizeof(m_apcTxt));
}

CEditableCondition::~CEditableCondition()
{
    
}

VOID CEditableCondition::Initialize(UINT uIndex, SPatternRow& sRow, UINT uRefreshEvent)
{
    m_uIndex = uIndex;
    m_psRow = &sRow;
    m_uRefreshEvent = uRefreshEvent;
    const FLOAT afSizes[EBTN_NUM] = {
        E_CONDITION_SRC_DEPTH_WIDTH,
        E_CONDITION_STATUS_WIDTH,
        E_CONDITION_WAS_VALUE_01_WIDTH,
        E_CONDITION_WAS_VALUE_02_WIDTH,
        E_CONDITION_IS_VALUE_01_WIDTH,
        E_CONDITION_IS_VALUE_02_WIDTH,
        E_CONDITION_MATCH_WIDTH,
        E_CONDITION_MATCH_T_WIDTH,
    };
    FLOAT fX = 0.0f;
    for (UINT uIndex = 0; EBTN_NUM > uIndex; ++uIndex) {
        CUITextLabel* pcLbl = NEW_TEXT_LABEL;
        if (NULL == pcLbl) {
            ASSERT(FALSE);
            return;
        }
        pcLbl->SetFont(EGAMEFONT_SIZE_12);
        pcLbl->SetLocalPosition(afSizes[uIndex] * 0.5f, E_CONDITION_CELL_HEIGHT * 0.5f);
        pcLbl->SetAnchor(0.5f, 0.5f);
        pcLbl->SetString("-");
        m_apcTxt[uIndex] = pcLbl;
        CUIImage& cImg = m_acBtnImg[uIndex];
        cImg.SetLocalSize(afSizes[uIndex], E_CONDITION_CELL_HEIGHT);
        cImg.SetVisibility(FALSE);
        CUIButton& cBtn = m_acBtn[uIndex];
        cBtn.SetDisplayWidgets(cImg, cImg);
        cBtn.AddChild(*pcLbl);
        cBtn.SetID(uIndex);
        cBtn.SetLocalPosition(fX, 0.0f);
        AddChild(cBtn);
        fX += afSizes[uIndex];
    }
    UpdateLabels();
}

VOID CEditableCondition::Release(VOID)
{
    for (UINT uIndex = 0; EBTN_NUM > uIndex; ++uIndex) {
        m_acBtn[uIndex].RemoveFromParent();
        SAFE_REMOVE_RELEASE_DELETE(m_apcTxt[uIndex]);
    }
}

VOID CEditableCondition::SendRefreshParentEvent(VOID)
{
    CUIContainer* pcParent = GetParent();
    if (NULL != pcParent) {
        CEvent cRefresh(m_uRefreshEvent);
        pcParent->PostEvent(cRefresh, 0.0f);
    }
    UpdateLabels();
}

static VOID UpdateValueLabel(CUITextLabel& cLbl, BYTE byValue)
{
    CHAR szBuffer[32];
    if (PATTERN_UNUSED == byValue) {
        cLbl.SetString("-");
        return;
    }
    snprintf(szBuffer, 32, "%d", byValue);
    cLbl.SetString(szBuffer);
}
VOID CEditableCondition::UpdateLabels(VOID)
{
    CHAR szBuffer[32];
    if (NULL != m_apcTxt[EBTN_DEPTH]) {
        snprintf(szBuffer, 32, "%d", 9 - m_psRow->byDepthIndex);
        m_apcTxt[EBTN_DEPTH]->SetString(szBuffer);
    }
    if (NULL != m_apcTxt[EBTN_STATUS]) {
        m_apcTxt[EBTN_STATUS]->SetString(m_psRow->bUp ? "UP" : "DN");
    }
    if (NULL != m_apcTxt[EBTN_WAS_L]) {
        UpdateValueLabel(*m_apcTxt[EBTN_WAS_L], m_psRow->byWasLow);
    }
    if (NULL != m_apcTxt[EBTN_WAS_H]) {
        UpdateValueLabel(*m_apcTxt[EBTN_WAS_H], m_psRow->byWasHigh);
    }
    if (NULL != m_apcTxt[EBTN_IS_L]) {
        UpdateValueLabel(*m_apcTxt[EBTN_IS_L], m_psRow->byIsLow);
    }
    if (NULL != m_apcTxt[EBTN_IS_H]) {
        UpdateValueLabel(*m_apcTxt[EBTN_IS_H], m_psRow->byIsHigh);
    }
    if (NULL != m_apcTxt[EBTN_MATCH]) {
        switch (m_psRow->eMatch) {
            case EPATTERN_MATCH_TOP:
                m_apcTxt[EBTN_MATCH]->SetString("Top");
                break;
            case EPATTERN_MATCH_BOTTOM:
                m_apcTxt[EBTN_MATCH]->SetString("Bottom");
                break;
            case EPATTERN_MATCH_NONE:
                m_apcTxt[EBTN_MATCH]->SetString("None");
                break;
        }
    }
    if (NULL != m_apcTxt[EBTN_MATCH_T]) {
        snprintf(szBuffer, 32, "%d", 9 - m_psRow->byMatchTargetDepthIndex);
        m_apcTxt[EBTN_MATCH_T]->SetString(szBuffer);
    }
}



VOID CEditableCondition::OnReceiveEvent(CEvent& cEvent)
{
    const UINT uEventID = cEvent.GetIntParam(0);
    switch (uEventID) {
        case EBTN_DEPTH:
            OnTapDepthBtn();
            break;
        case EBTN_STATUS:
            OnTapStatusBtn();
            break;
        case EBTN_WAS_L:
        case EBTN_WAS_H:
        case EBTN_IS_H:
        case EBTN_IS_L:
            OnTapValueBtn(uEventID);
            break;
        case EBTN_MATCH:
            OnTapMatchBtn();
            break;
        case EBTN_MATCH_T:
            OnTapMatchDepthBtn();
            break;
        case EEVENT_REPLY_DEPTH:
            OnUserReplyDepth(cEvent.GetIntParam(2));
            break;
        case EEVENT_REPLY_VALUE:
            OnUserReplyValue(cEvent.GetIntParam(1), cEvent.GetIntParam(2));
            break;
        case EEVENT_REPLY_MATCH_DEPTH:
            OnUserReplyMatchDepth(cEvent.GetIntParam(2));
            break;
        default:
            break;
    }
}

VOID CEditableCondition::OnTapDepthBtn(VOID)
{
    const CHAR* szSelection[9] = {
        "9", "8", "7", "6", "5", "4", "3", "2", "1"
    };
    CTextSelectorWindow* pcWin = new CTextSelectorWindow(*this, EEVENT_REPLY_DEPTH, 0);
    if (NULL != pcWin) {
        const SHLVector2D& sPos = m_acBtn[EBTN_DEPTH].GetWorldPosition();
        pcWin->SetLocalPosition(sPos.x + 20.0f, sPos.y + 5.0f);
        pcWin->Initialize(szSelection, 9, m_psRow->byDepthIndex);
        pcWin->DoModal();
    }
}
VOID CEditableCondition::OnUserReplyDepth(UINT uDepth)
{
    if (9 <= uDepth) {
        ASSERT(FALSE);
        return;
    }
    m_psRow->byDepthIndex = uDepth;
    SendRefreshParentEvent();
}

VOID CEditableCondition::OnTapStatusBtn(VOID)
{
    m_psRow->bUp = !m_psRow->bUp;
    SendRefreshParentEvent();
}

VOID CEditableCondition::OnTapValueBtn(UINT uEventID)
{
    UINT uSelectedIndex = 0;
    switch (uEventID) {
        case EBTN_WAS_L:
            uSelectedIndex = m_psRow->byWasLow;
            break;
        case EBTN_WAS_H:
            uSelectedIndex = m_psRow->byWasHigh;
            break;
        case EBTN_IS_L:
            uSelectedIndex = m_psRow->byIsLow;
            break;
        case EBTN_IS_H:
            uSelectedIndex = m_psRow->byIsHigh;
            break;
        default:
            ASSERT(FALSE);
            return;
    }
    if (PATTERN_UNUSED == uSelectedIndex) {
        uSelectedIndex = 10;
    }        
    const CHAR* szSelection[11] = {
        "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "-"
    };
    CTextSelectorWindow* pcWin = new CTextSelectorWindow(*this, EEVENT_REPLY_VALUE, uEventID);
    if (NULL != pcWin) {
        const SHLVector2D& sPos = m_acBtn[uEventID].GetWorldPosition();
        pcWin->SetLocalPosition(sPos.x + 20.0f, sPos.y + 5.0f);
        pcWin->Initialize(szSelection, 11, uSelectedIndex);
        pcWin->DoModal();
    }
}

VOID CEditableCondition::OnUserReplyValue(UINT uEventID, UINT uValue)
{
    if (uValue == 10) {
        uValue = PATTERN_UNUSED;
    }
    switch (uEventID) {
        case EBTN_WAS_L:
            m_psRow->byWasLow = uValue;
            break;
        case EBTN_WAS_H:
            m_psRow->byWasHigh = uValue;
            break;
        case EBTN_IS_L:
            m_psRow->byIsLow = uValue;
            break;
        case EBTN_IS_H:
            m_psRow->byIsHigh = uValue;
            break;
    }
    SendRefreshParentEvent();
}

VOID CEditableCondition::OnTapMatchBtn(VOID)
{
    switch (m_psRow->eMatch) {
        case EPATTERN_MATCH_NONE:
            m_psRow->eMatch = EPATTERN_MATCH_TOP;
            break;
        case EPATTERN_MATCH_TOP:
            m_psRow->eMatch = EPATTERN_MATCH_BOTTOM;
            break;
        case EPATTERN_MATCH_BOTTOM:
            m_psRow->eMatch = EPATTERN_MATCH_NONE;
            break;
    }
    SendRefreshParentEvent();
}

VOID CEditableCondition::OnTapMatchDepthBtn(VOID)
{
    const CHAR* szSelection[6] = {
        "9", "8", "7", "6", "5", "4"
    };
    CTextSelectorWindow* pcWin = new CTextSelectorWindow(*this, EEVENT_REPLY_MATCH_DEPTH, 0);
    if (NULL != pcWin) {
        const SHLVector2D& sPos = m_acBtn[EBTN_MATCH_T].GetWorldPosition();
        pcWin->SetLocalPosition(sPos.x + 20.0f, sPos.y + 5.0f);
        pcWin->Initialize(szSelection, 6, m_psRow->byDepthIndex);
        pcWin->DoModal();
    }

}
VOID CEditableCondition::OnUserReplyMatchDepth(UINT uDepth)
{
    m_psRow->byMatchTargetDepthIndex = uDepth;
    SendRefreshParentEvent();
}
