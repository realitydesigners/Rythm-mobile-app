#include "stdafx.h"
#include "ConditionCell.h"
#include "EventManager.h"

CConditionCell::CConditionCell() :
CUIContainer(),
INITIALIZE_TEXT_LABEL(m_cSrcDepthTxt),
INITIALIZE_TEXT_LABEL(m_cConditionTxt),
INITIALIZE_TEXT_LABEL(m_cWasValue01Txt),
INITIALIZE_TEXT_LABEL(m_cWasValue02Txt),
INITIALIZE_TEXT_LABEL(m_cIsValue01Txt),
INITIALIZE_TEXT_LABEL(m_cIsValue02Txt)
{
    
}
CConditionCell::~CConditionCell()
{
    
}

VOID CConditionCell::Initialize(VOID)
{
    m_cNumberBG.SetLocalSize(CONDITION_SRC_DEPTH_WIDTH, CONDITION_CELL_HEIGHT);
    m_cNumberBG.SetVisibility(FALSE);
    
    m_cConditionBG.SetLocalSize(CONDITION_CONDITION_WIDTH, CONDITION_CELL_HEIGHT);
    m_cConditionBG.SetVisibility(FALSE);
    
    FLOAT fX = CONDITION_SRC_DEPTH_WIDTH * 0.5f;
    m_cSrcDepthTxt.SetFont(EGAMEFONT_SIZE_12);
    m_cSrcDepthTxt.SetLocalPosition(fX, CONDITION_CELL_HEIGHT * 0.5f);
    m_cSrcDepthTxt.SetAnchor(0.5f, 0.5f);
    AddChild(m_cSrcDepthTxt);
    fX += CONDITION_SRC_DEPTH_WIDTH * 0.5f;
    fX += CONDITION_CONDITION_WIDTH * 0.5f;
    m_cConditionTxt.SetFont(EGAMEFONT_SIZE_12);
    m_cConditionTxt.SetLocalPosition(fX, CONDITION_CELL_HEIGHT * 0.5f);
    m_cConditionTxt.SetAnchor(0.5f, 0.5f);
    AddChild(m_cConditionTxt);
    fX += CONDITION_CONDITION_WIDTH * 0.5f;
    fX += CONDITION_WAS_VALUE_01_WIDTH * 0.5f;
    m_cWasValue01Txt.SetFont(EGAMEFONT_SIZE_12);
    m_cWasValue01Txt.SetLocalPosition(fX, CONDITION_CELL_HEIGHT * 0.5f);
    m_cWasValue01Txt.SetAnchor(0.5f, 0.5f);
    AddChild(m_cWasValue01Txt);
    fX += CONDITION_WAS_VALUE_01_WIDTH * 0.5f;
    fX += CONDITION_WAS_VALUE_02_WIDTH * 0.5f;
    
    m_cWasValue02Txt.SetFont(EGAMEFONT_SIZE_12);
    m_cWasValue02Txt.SetLocalPosition(fX, CONDITION_CELL_HEIGHT * 0.5f);
    m_cWasValue02Txt.SetAnchor(0.5f, 0.5f);
    AddChild(m_cWasValue02Txt);
    fX += CONDITION_WAS_VALUE_02_WIDTH * 0.5f;
    fX += CONDITION_IS_VALUE_01_WIDTH * 0.5f;
    
    m_cIsValue01Txt.SetFont(EGAMEFONT_SIZE_12);
    m_cIsValue01Txt.SetLocalPosition(fX, CONDITION_CELL_HEIGHT * 0.5f);
    m_cIsValue01Txt.SetAnchor(0.5f, 0.5f);
    AddChild(m_cIsValue01Txt);
    fX += CONDITION_IS_VALUE_01_WIDTH * 0.5f;
    fX += CONDITION_IS_VALUE_02_WIDTH * 0.5f;
    m_cIsValue02Txt.SetFont(EGAMEFONT_SIZE_12);
    m_cIsValue02Txt.SetLocalPosition(fX, CONDITION_CELL_HEIGHT * 0.5f);
    m_cIsValue02Txt.SetAnchor(0.5f, 0.5f);
    AddChild(m_cIsValue02Txt);
    Clear();
}

#define GREY_COLOR RGBA(0x7F, 0x7F, 0x7F, 0xFF)
VOID CConditionCell::Refresh(const SPatternRow& sRow)
{
    CHAR szBuffer[256];
    snprintf(szBuffer, 256, "F%d", 9 - sRow.byDepthIndex);
    m_cSrcDepthTxt.SetString(szBuffer);
    m_cSrcDepthTxt.SetColor(0xFFFFFFFF);
    m_cConditionTxt.SetString(sRow.bUp ? "UP" : "DN");
    m_cConditionTxt.SetColor(0xFFFFFFFF);
    if (PATTERN_UNUSED == sRow.byWasLow) {
        m_cWasValue01Txt.SetString("-");
        m_cWasValue01Txt.SetColor(GREY_COLOR);
    }
    else {
        snprintf(szBuffer, 256, "%d", sRow.byWasLow);
        m_cWasValue01Txt.SetString(szBuffer);
        m_cWasValue01Txt.SetColor(0xFFFFFFFF);
    }
    if (PATTERN_UNUSED == sRow.byWasHigh) {
        m_cWasValue02Txt.SetString("-");
        m_cWasValue02Txt.SetColor(GREY_COLOR);
    }
    else {
        snprintf(szBuffer, 256, "%d", sRow.byWasHigh);
        m_cWasValue02Txt.SetString(szBuffer);
        m_cWasValue02Txt.SetColor(0xFFFFFFFF);
    }
    if (PATTERN_UNUSED == sRow.byIsLow) {
        m_cIsValue01Txt.SetString("-");
        m_cIsValue01Txt.SetColor(GREY_COLOR);
    }
    else {
        snprintf(szBuffer, 256, "%d", sRow.byIsLow);
        m_cIsValue01Txt.SetString(szBuffer);
        m_cIsValue01Txt.SetColor(0xFFFFFFFF);
    }
    if (PATTERN_UNUSED == sRow.byIsHigh) {
        m_cIsValue02Txt.SetString("-");
        m_cIsValue02Txt.SetColor(GREY_COLOR);
    }
    else {
        snprintf(szBuffer, 256, "%d", sRow.byIsHigh);
        m_cIsValue02Txt.SetString(szBuffer);
        m_cIsValue02Txt.SetColor(0xFFFFFFFF);
    }
}
VOID CConditionCell::Clear(VOID)
{
    m_cSrcDepthTxt.SetString("-");
    m_cConditionTxt.SetString("-");
    m_cWasValue01Txt.SetString("-");
    m_cWasValue02Txt.SetString("-");
    m_cIsValue01Txt.SetString("-");
    m_cIsValue02Txt.SetString("-");
    m_cSrcDepthTxt.SetColor(GREY_COLOR);
    m_cWasValue01Txt.SetColor(GREY_COLOR);
    m_cWasValue02Txt.SetColor(GREY_COLOR);
    m_cIsValue01Txt.SetColor(GREY_COLOR);
    m_cIsValue02Txt.SetColor(GREY_COLOR);
}

VOID CConditionCell::Release(VOID)
{
    m_cSrcDepthTxt.Release();
    m_cSrcDepthTxt.RemoveFromParent();
    
    m_cConditionTxt.Release();
    m_cConditionTxt.RemoveFromParent();
    
    m_cWasValue01Txt.Release();
    m_cWasValue01Txt.RemoveFromParent();

    m_cWasValue02Txt.Release();
    m_cWasValue02Txt.RemoveFromParent();

    m_cIsValue01Txt.Release();
    m_cIsValue01Txt.RemoveFromParent();
    
    m_cIsValue02Txt.Release();
    m_cIsValue02Txt.RemoveFromParent();
}
