#include "stdafx.h"
#include "EditConditionsTable.h"
#include "PatternData.h"
#include "EventManager.h"

CEditConditionsTable::CEditConditionsTable() :
CUIContainer()
{
    memset(m_apcTxt, 0, sizeof(m_apcTxt));
    memset(m_asPatternRow, 0, sizeof(m_asPatternRow));
    for (UINT uIndex = 0; MAX_CONDITION_NUM > uIndex; ++uIndex) {
        m_asPatternRow[uIndex].byDepthIndex = uIndex + 1;
        m_asPatternRow[uIndex].eMatch = EPATTERN_MATCH_NONE;
        m_asPatternRow[uIndex].byMatchTargetDepthIndex = 1;
        m_asPatternRow[uIndex].byWasLow =
        m_asPatternRow[uIndex].byWasHigh =
        m_asPatternRow[uIndex].byIsLow =
        m_asPatternRow[uIndex].byIsHigh = PATTERN_UNUSED;
    }
}
CEditConditionsTable::~CEditConditionsTable()
{
    
}

VOID CEditConditionsTable::Initialize(UINT uRefreshEvent)
{
    SetLocalSize(E_CONDITION_CELL_WIDTH, E_CONDITION_CELL_HEIGHT * (1 + MAX_CONDITION_NUM));
    
    const CHAR* szHeaders[8] = {
        "F", "U/D", "WasL", "WasH", "IsL", "IsH", "Match", "T"
    };
    const FLOAT afSizes[8] = {
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
    for (UINT uIndex = 0; 8 > uIndex; ++uIndex) {
        CUITextLabel* pcLbl = NEW_TEXT_LABEL;
        if (NULL == pcLbl) {
            ASSERT(FALSE);
            return;
        }
        pcLbl->SetFont(EGAMEFONT_SIZE_12);
        pcLbl->SetLocalPosition(fX + afSizes[uIndex] * 0.5f, E_CONDITION_CELL_HEIGHT * 0.5f);
        pcLbl->SetAnchor(0.5f, 0.5f);
        pcLbl->SetString(szHeaders[uIndex]);
        AddChild(*pcLbl);
        m_apcTxt[uIndex] = pcLbl;
        fX += afSizes[uIndex];
    }
    FLOAT fY = E_CONDITION_CELL_HEIGHT;
    for (UINT uIndex = 0; MAX_CONDITION_NUM > uIndex; ++uIndex, fY += E_CONDITION_CELL_HEIGHT) {
        m_acCells[uIndex].Initialize(uIndex, m_asPatternRow[uIndex], uRefreshEvent);
        m_acCells[uIndex].SetLocalPosition(0.0f, fY);
        AddChild(m_acCells[uIndex]);
    }
}

VOID CEditConditionsTable::SetMaxRow(UINT uRow)
{
    if (MAX_CONDITION_NUM < uRow) {
        ASSERT(FALSE);
        return;
    }
    for (UINT uIndex = 0; MAX_CONDITION_NUM > uIndex; ++uIndex) {
        m_acCells[uIndex].RemoveFromParent();
        if (uRow > uIndex) {
            AddChild(m_acCells[uIndex]);
        }
    }
}
VOID CEditConditionsTable::Refresh(VOID)
{

}

VOID CEditConditionsTable::Release(VOID)
{
    for (UINT uIndex = 0; 8 > uIndex; ++uIndex) {
        SAFE_REMOVE_RELEASE_DELETE(m_apcTxt[uIndex]);
    }
    for (UINT uIndex = 0; MAX_CONDITION_NUM > uIndex; ++uIndex) {
        m_acCells[uIndex].Release();
        m_acCells[uIndex].RemoveFromParent();
    }
}

#include "GameRenderer.h"
#define GREY_LINE RGBA(0x7F, 0x7F, 0x7F, 0xFF)
VOID CEditConditionsTable::OnRender(FLOAT fOffsetX, FLOAT fOffsetY) const
{
    CUIContainer::OnRender(fOffsetX, fOffsetY);
    const SHLVector2D& sWorldPos = GetWorldPosition();
    fOffsetX += sWorldPos.x;
    fOffsetY += sWorldPos.y;
    const SHLVector2D& sWorldSize = GetWorldSize();
    const FLOAT fTopY = fOffsetY;
    const FLOAT fBottomY = fOffsetY + sWorldSize.y;
    const FLOAT fLeftX = fOffsetX;
    const FLOAT fRightX = fOffsetX + sWorldSize.x;
    
    const FLOAT afSizes[8] = {
        E_CONDITION_SRC_DEPTH_WIDTH,
        E_CONDITION_STATUS_WIDTH,
        E_CONDITION_WAS_VALUE_01_WIDTH,
        E_CONDITION_WAS_VALUE_02_WIDTH,
        E_CONDITION_IS_VALUE_01_WIDTH,
        E_CONDITION_IS_VALUE_02_WIDTH,
        E_CONDITION_MATCH_WIDTH,
        E_CONDITION_MATCH_T_WIDTH,
    };
    FLOAT fX = fLeftX;
    for (UINT uIndex = 0; 8 > uIndex; ++uIndex) {
        CGameRenderer::DrawLine(fX, fTopY, fX, fBottomY, GREY_LINE);
        fX += afSizes[uIndex];
    }
    CGameRenderer::DrawLine(fX, fTopY, fX, fBottomY, GREY_LINE);
    
    
    CGameRenderer::DrawLine(fLeftX, 0.0f, fRightX, 0.0f, GREY_LINE);

    for (UINT uIndex = 0; (MAX_CONDITION_NUM + 1) >= uIndex; ++uIndex) {
        const FLOAT fY = fTopY + uIndex * E_CONDITION_CELL_HEIGHT;
        CGameRenderer::DrawLine(fLeftX, fY, fRightX, fY, GREY_LINE);
    }
}


