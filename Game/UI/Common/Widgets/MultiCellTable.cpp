#include "stdafx.h"
#include "Event.h"
#include "EventManager.h"
#include "GameWidgetDefs.h"
#include "MultiCellTable.h"

#define CELL_HEIGHT (30.0f)

#define SELECTED_FONT_COLOR        RGBA(0xFF,  0xFF,   0xFF,   0xFF)
#define NOT_SELECTED_FONT_COLOR    RGBA(0x7F,  0x7F,   0x7F,   0xFF)

#define SELECTED_CELL_COLOR RGBA(0x4F,0x4F,0x4F,0xFF)
#define GREY_COLOR          RGBA(0x7F,0x7F,0x7F,0xFF)

CMultiCellTable::CMultiCellTable() :
CUIContainer(),
m_pfCellWidth(NULL),
m_ppcHeaders(NULL),
m_uCellNum(0),
m_uSelectedEventID(0),
m_fNextCellY(0.0f)
{
}
    
CMultiCellTable::~CMultiCellTable()
{
    
}

VOID CMultiCellTable::Initialize(const CHAR** pszHeaders, const FLOAT* pfCellWidth, UINT uCellNum, FLOAT fTableHeight, UINT uSelectedEventID)
{
    ASSERT(NULL == m_pfCellWidth);
    m_pfCellWidth = new FLOAT[uCellNum];
    m_ppcHeaders = new CUITextLabel*[uCellNum];
    if (NULL == m_pfCellWidth || NULL == m_ppcHeaders) {
        ASSERT(FALSE);
        return;
    }
    memset(m_ppcHeaders, 0, sizeof(CUITextLabel*) * uCellNum);
    FLOAT fWidth = 0;
    const FLOAT fHeaderY = CELL_HEIGHT * 0.5f;
    for (UINT uIndex = 0; uCellNum > uIndex; ++uIndex) {
        m_pfCellWidth[uIndex] = pfCellWidth[uIndex];
        CUITextLabel* pcLbl = NEW_TEXT_LABEL;
        if (NULL == pcLbl) {
            ASSERT(FALSE);
            return;
        }
        pcLbl->SetFont(EGAMEFONT_SIZE_12);
        pcLbl->SetAnchor(0.5f, 0.5f);
        pcLbl->SetLocalSize(pfCellWidth[uIndex], CELL_HEIGHT);
        const FLOAT fHeaderX = fWidth + pfCellWidth[uIndex] * 0.5f;
        pcLbl->SetLocalPosition(fHeaderX, fHeaderY);
        pcLbl->SetString(pszHeaders[uIndex]);
        AddChild(*pcLbl);
        m_ppcHeaders[uIndex] = pcLbl;
        
        fWidth += pfCellWidth[uIndex];
    }
    
    SetLocalSize(fWidth, fTableHeight);
    m_cScroller.SetLocalSize(fWidth, fTableHeight - CELL_HEIGHT);
    m_cScroller.SetLocalPosition(0.0f, CELL_HEIGHT);
    AddChild(m_cScroller);
    
    m_uCellNum = uCellNum;
    m_uSelectedEventID = uSelectedEventID;
    
}
VOID CMultiCellTable::ClearEntries(VOID)
{
    CUIWidget* pcWidget = m_cScroller.GetChild();
    while (NULL != pcWidget) {
        CUIWidget* pcTmp = pcWidget;
        pcWidget = pcWidget->GetNextSibling();
        SAFE_REMOVE_RELEASE_DELETE(pcTmp);
    }
    m_fNextCellY = 0.0f;
}
VOID CMultiCellTable::AddRow(const CHAR** pszCells, UINT uCellNum, UINT uID)
{
    CMultiCellRow* pcRow = new CMultiCellRow(uID);
    if (NULL == pcRow) {
        ASSERT(FALSE);
        return;
    }
    pcRow->SetLocalPosition(0.0f, m_fNextCellY);
    pcRow->Initialize(pszCells, m_pfCellWidth, uCellNum, m_uSelectedEventID);
    m_cScroller.AddChild(*pcRow);
    m_fNextCellY += CELL_HEIGHT;
}

VOID CMultiCellTable::Release(VOID)
{
    if (NULL != m_ppcHeaders) {
        for (UINT uIndex = 0; m_uCellNum > uIndex; ++uIndex) {
            SAFE_REMOVE_RELEASE_DELETE(m_ppcHeaders[uIndex]);
        }
        SAFE_DELETE_ARRAY(m_ppcHeaders);
    }
    SAFE_DELETE_ARRAY(m_pfCellWidth);
    m_uCellNum = 0;

    ClearEntries();
    m_cScroller.RemoveFromParent();
    CUIContainer::Release();
}

VOID CMultiCellTable::SetSelected(UINT uID)
{
    CMultiCellRow* pcWidget = (CMultiCellRow*)m_cScroller.GetChild();
    while (NULL != pcWidget) {
        pcWidget->SetSelected(pcWidget->GetDataID() == uID);
        pcWidget = (CMultiCellRow*)pcWidget->GetNextSibling();
    }
}

#include "GameRenderer.h"
VOID CMultiCellTable::OnRender(FLOAT fOffsetX, FLOAT fOffsetY) const
{
    CUIContainer::OnRender(fOffsetX, fOffsetY);
    if (NULL != m_pfCellWidth) {
        const SHLVector2D& sWorldPos = GetWorldPosition();
        fOffsetX += sWorldPos.x;
        fOffsetY += sWorldPos.y;
        const SHLVector2D& sWorldSize = GetWorldSize();
        CGameRenderer::DrawRectOutline(fOffsetX, fOffsetY, sWorldSize.x, sWorldSize.y, GREY_COLOR);
        const FLOAT fYTop = fOffsetY;
        const FLOAT fYBottom = fOffsetY + sWorldSize.y;
        const FLOAT fLeftX = fOffsetX;
        const FLOAT fRightX = fOffsetX + sWorldSize.x;
        const FLOAT fCellStartY = fYTop + CELL_HEIGHT;
        CGameRenderer::DrawLine(fLeftX, fCellStartY, fRightX, fCellStartY, GREY_COLOR);
        for (UINT uIndex = 1; m_uCellNum > uIndex; ++uIndex) {
            fOffsetX += m_pfCellWidth[uIndex-1];
            CGameRenderer::DrawLine(fOffsetX, fYTop, fOffsetX, fYBottom, GREY_COLOR);            
        }
    }
}


CMultiCellRow::CMultiCellRow(UINT uID) :
CUIButton(),
m_uID(uID),
m_ppcData(NULL),
m_uCellNum(0)
{
    
}

CMultiCellRow::~CMultiCellRow()
{
    
}


VOID CMultiCellRow::Initialize(const CHAR** pszData, const FLOAT* pfCellWidth, UINT uCellNum, UINT uBtnEventID)
{
    ASSERT(NULL == m_ppcData);
    m_ppcData = new CUITextLabel*[uCellNum];
    if (NULL == m_ppcData) {
        ASSERT(FALSE);
        return;
    }
    m_uCellNum = uCellNum;
    memset(m_ppcData, 0, sizeof(CUITextLabel*) * uCellNum);
    FLOAT fWidth = 0;
    const FLOAT fHeaderY = CELL_HEIGHT * 0.5f;
    for (UINT uIndex = 0; uCellNum > uIndex; ++uIndex) {
        CUITextLabel* pcLbl = NEW_TEXT_LABEL;
        if (NULL == pcLbl) {
            ASSERT(FALSE);
            return;
        }
        pcLbl->SetFont(EGAMEFONT_SIZE_12);
        pcLbl->SetAnchor(0.5f, 0.5f);
        pcLbl->SetLocalSize(pfCellWidth[uIndex], CELL_HEIGHT);
        const FLOAT fHeaderX = fWidth + pfCellWidth[uIndex] * 0.5f;
        pcLbl->SetLocalPosition(fHeaderX, fHeaderY);
        pcLbl->SetString(pszData[uIndex]);
        AddChild(*pcLbl);
        m_ppcData[uIndex] = pcLbl;
        fWidth += pfCellWidth[uIndex];
    }
    SetID(uBtnEventID);
    m_cBG.SetColor(SELECTED_CELL_COLOR);
    m_cBG.SetLocalSize(fWidth, CELL_HEIGHT);
    m_cBG.SetVisibility(FALSE);
    SetDisplayWidgets(m_cBG, m_cBG);
}

VOID CMultiCellRow::SetSelected(BOOLEAN bSelected)
{
    m_cBG.SetVisibility(bSelected);
}

VOID CMultiCellRow::Release(VOID)
{
    if (NULL != m_ppcData) {
        for (UINT uIndex = 0; m_uCellNum > uIndex; ++uIndex) {
            SAFE_REMOVE_RELEASE_DELETE(m_ppcData[uIndex]);
        }
        SAFE_DELETE_ARRAY(m_ppcData);
    }
    m_uCellNum = 0;
    CUIButton::Release();
}

BOOLEAN CMultiCellRow::CreateButtonEvent(CEvent& cEvent)
{
    cEvent.SetIntParam(0, GetID());
    cEvent.SetIntParam(1, m_uID);
    return TRUE;
}

