#include "stdafx.h"
#include "PatternData.h"
#include "MainConditonsTable.h"
#include "EventManager.h"

#define DEFAULT_CELL_DISPLAY_NUM (3)

CMainConditionsTable::CMainConditionsTable() :
CUIContainer(),
m_uCellsVisible(0),
INITIALIZE_TEXT_LABEL(m_cDepthTxt),
INITIALIZE_TEXT_LABEL(m_cConditionTxt),
INITIALIZE_TEXT_LABEL(m_cWasLv01Txt),
INITIALIZE_TEXT_LABEL(m_cWasLv02Txt),
INITIALIZE_TEXT_LABEL(m_cIsLv01Txt),
INITIALIZE_TEXT_LABEL(m_cIsLv02Txt)
{
    
}
CMainConditionsTable::~CMainConditionsTable()
{
    
}

VOID CMainConditionsTable::Initialize(VOID)
{
    FLOAT fX = 0.0f;
    FLOAT fY = MAIN_CONDITION_CELL_HEIGHT * 0.5f;
    m_cDepthTxt.SetFont(EGAMEFONT_SIZE_12);
    m_cDepthTxt.SetString("F");
    m_cDepthTxt.SetLocalPosition(fX + CONDITION_SRC_DEPTH_WIDTH * 0.5f, fY);
    m_cDepthTxt.SetAnchor(0.5f, 0.5f);
    AddChild(m_cDepthTxt);
    fX += CONDITION_SRC_DEPTH_WIDTH;
    
    m_cConditionTxt.SetFont(EGAMEFONT_SIZE_12);
    m_cConditionTxt.SetString("U/D");
    m_cConditionTxt.SetLocalPosition(fX + CONDITION_CONDITION_WIDTH * 0.5f, fY);
    m_cConditionTxt.SetAnchor(0.5f, 0.5f);
    AddChild(m_cConditionTxt);
    fX += CONDITION_CONDITION_WIDTH;
    
    m_cWasLv01Txt.SetFont(EGAMEFONT_SIZE_12);
    m_cWasLv01Txt.SetString("WasL");
    m_cWasLv01Txt.SetLocalPosition(fX + CONDITION_WAS_VALUE_01_WIDTH * 0.5f, fY);
    m_cWasLv01Txt.SetAnchor(0.5f, 0.5f);
    AddChild(m_cWasLv01Txt);
    fX += CONDITION_WAS_VALUE_01_WIDTH;
    
    m_cWasLv02Txt.SetFont(EGAMEFONT_SIZE_12);
    m_cWasLv02Txt.SetString("WasH");
    m_cWasLv02Txt.SetLocalPosition(fX + CONDITION_WAS_VALUE_01_WIDTH * 0.5f, fY);
    m_cWasLv02Txt.SetAnchor(0.5f, 0.5f);
    AddChild(m_cWasLv02Txt);
    fX += CONDITION_WAS_VALUE_02_WIDTH;
    
    m_cIsLv01Txt.SetFont(EGAMEFONT_SIZE_12);
    m_cIsLv01Txt.SetString("IsL");
    m_cIsLv01Txt.SetLocalPosition(fX + CONDITION_IS_VALUE_01_WIDTH * 0.5f, fY);
    m_cIsLv01Txt.SetAnchor(0.5f, 0.5f);
    AddChild(m_cIsLv01Txt);
    fX += CONDITION_IS_VALUE_01_WIDTH;
    
    m_cIsLv02Txt.SetFont(EGAMEFONT_SIZE_12);
    m_cIsLv02Txt.SetString("IsH");
    m_cIsLv02Txt.SetLocalPosition(fX + CONDITION_IS_VALUE_02_WIDTH * 0.5f, fY);
    m_cIsLv02Txt.SetAnchor(0.5f, 0.5f);
    AddChild(m_cIsLv02Txt);

    
    fY = MAIN_CONDITION_CELL_HEIGHT;
    for (UINT uIndex = 0; MAX_CONDITION_NUM > uIndex; ++uIndex, fY += MAIN_CONDITION_CELL_HEIGHT) {
        m_acCells[uIndex].Initialize();
        m_acCells[uIndex].SetLocalPosition(0.0f, fY);
        AddChild(m_acCells[uIndex]);
    }
    Clear();
}

VOID CMainConditionsTable::Refresh(UINT uPatternIndex)
{
    const SPattern& sPattern = CPatternDataMgr::GetPattern(uPatternIndex);
    for (UINT uIndex = 0; sPattern.uPatternNum > uIndex; ++uIndex) {
        const SPatternRow* psRow = CPatternDataMgr::GetPatternRow(sPattern.uIndexOffset + uIndex);
        if (NULL != psRow) {
            m_acCells[uIndex].Refresh(*psRow);
        }
        else {
            m_acCells[uIndex].Clear();
        }
    }
    Resize(sPattern.uPatternNum);
}
VOID CMainConditionsTable::Clear(VOID)
{
    Resize(DEFAULT_CELL_DISPLAY_NUM);
    for (UINT uIndex = 0; MAX_CONDITION_NUM > uIndex; ++uIndex) {
        m_acCells[uIndex].Clear();
    }
}
VOID CMainConditionsTable::Resize(UINT uNumCells)
{
    m_uCellsVisible = uNumCells;
    const FLOAT fHeight = (uNumCells + 1) * MAIN_CONDITION_CELL_HEIGHT;
    SetLocalSize(MAIN_CONDITION_TABLE_WIDTH, fHeight);
    for (UINT uIndex = 0; MAX_CONDITION_NUM > uIndex; ++uIndex) {
        m_acCells[uIndex].SetVisibility(uIndex < uNumCells);
    }
}

VOID CMainConditionsTable::Release(VOID)
{
    m_cDepthTxt.Release();
    m_cDepthTxt.RemoveFromParent();
    
    m_cConditionTxt.Release();
    m_cConditionTxt.RemoveFromParent();
    
    m_cWasLv01Txt.Release();
    m_cWasLv01Txt.RemoveFromParent();
    m_cWasLv02Txt.Release();
    m_cWasLv02Txt.RemoveFromParent();
    
    m_cIsLv01Txt.Release();
    m_cIsLv01Txt.RemoveFromParent();
    m_cIsLv02Txt.Release();
    m_cIsLv02Txt.RemoveFromParent();
    
    for (UINT uIndex = 0; MAX_CONDITION_NUM > uIndex; ++uIndex) {
        m_acCells[uIndex].Release();
        m_acCells[uIndex].RemoveFromParent();
    }
}

#include "GameRenderer.h"
#define GREY_LINE RGBA(0x7F, 0x7F, 0x7F, 0xFF)
VOID CMainConditionsTable::OnRender(FLOAT fOffsetX, FLOAT fOffsetY) const
{
    CUIContainer::OnRender(fOffsetX, fOffsetY);
    const SHLVector2D& sWorldPos = GetWorldPosition();
    fOffsetX += sWorldPos.x;
    fOffsetY += sWorldPos.y;
    const FLOAT fTopY = fOffsetY;
    const FLOAT fLeftX = fOffsetX;
    const FLOAT fRightX = fOffsetX + GetWorldSize().x;
    for (UINT uIndex = 0; (m_uCellsVisible + 1) >= uIndex; ++uIndex) {
        const FLOAT fY = fTopY + uIndex * MAIN_CONDITION_CELL_HEIGHT;
        CGameRenderer::DrawLine(fLeftX, fY, fRightX, fY, GREY_LINE);
    }
    const FLOAT fBottomY = fTopY + (m_uCellsVisible + 1) * MAIN_CONDITION_CELL_HEIGHT;
    CGameRenderer::DrawLine(fLeftX, fTopY, fLeftX, fBottomY, GREY_LINE);
    CGameRenderer::DrawLine(fRightX, fTopY, fRightX, fBottomY, GREY_LINE);
    FLOAT fX = fLeftX + CONDITION_SRC_DEPTH_WIDTH;
    CGameRenderer::DrawLine(fX, fTopY, fX, fBottomY, GREY_LINE);
    fX += CONDITION_CONDITION_WIDTH;
    CGameRenderer::DrawLine(fX, fTopY, fX, fBottomY, GREY_LINE);
    fX += CONDITION_WAS_VALUE_01_WIDTH;
    CGameRenderer::DrawLine(fX, fTopY, fX, fBottomY, GREY_LINE);
    fX += CONDITION_WAS_VALUE_02_WIDTH;
    CGameRenderer::DrawLine(fX, fTopY, fX, fBottomY, GREY_LINE);
    fX += CONDITION_IS_VALUE_01_WIDTH;
    CGameRenderer::DrawLine(fX, fTopY, fX, fBottomY, GREY_LINE);
}


