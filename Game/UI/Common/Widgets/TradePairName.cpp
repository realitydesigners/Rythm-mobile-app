#include "stdafx.h"
#include "GameWidgetDefs.h"
#include "TextUtil.h"
#include "TradePairName.h"

#define SELECTED_FONT_COLOR        RGBA(0xFF,  0xFF,   0xFF,   0xFF)
#define NOT_SELECTED_FONT_COLOR    RGBA(0x7F,  0x7F,   0x7F,   0xFF)

#define SELECTED_NAME_FONT      EGAMEFONT_SIZE_16
#define NOT_SELECTED_NAME_FONT  EGAMEFONT_SIZE_14

#define SELECTED_NAME_FONT2     EGAMEFONT_SIZE_12
#define NOT_SELECTED_NAME_FONT2 EGAMEFONT_SIZE_10

CTradePairName::CTradePairName() :
CUIWidget(EGameWidget_TradePairName),
INITIALIZE_TEXT_LABEL(m_cName1),
INITIALIZE_TEXT_LABEL(m_cName2)
{
    m_cName1.SetFont(NOT_SELECTED_NAME_FONT);
    m_cName1.SetColor(NOT_SELECTED_FONT_COLOR);
    m_cName1.SetAnchor(0.0f, 0.5f);

    m_cName2.SetFont(NOT_SELECTED_NAME_FONT2);
    m_cName2.SetColor(NOT_SELECTED_FONT_COLOR);
    m_cName2.SetAnchor(0.0f, 0.5f);
}
    
CTradePairName::~CTradePairName()
{
    
}

VOID CTradePairName::SetName(ESOURCE_MARKET eSource, UINT uTradePairIndex)
{
    const SHLVector2D& sLocalSize = GetLocalSize();
    m_cName1.SetLocalPosition(0.0f, sLocalSize.y * 0.5f);
    m_cName2.SetLocalPosition(0.0f, sLocalSize.y * 0.5f);
    
    const CHAR* szPairName = CCryptoUtil::GetTradePairName(eSource, uTradePairIndex);
    CTextUtil::Tokenize(szPairName, "_");
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
        m_cName1.SetString(szPairName);
        m_cName2.ClearString();
    }
    RecalculateNamePosition();
}
    
VOID CTradePairName::SetSelected(BOOLEAN bSelected)
{
    if (bSelected) {
        m_cName1.SetFont(SELECTED_NAME_FONT);
        m_cName1.SetColor(SELECTED_FONT_COLOR);
        m_cName2.SetFont(SELECTED_NAME_FONT2);
        m_cName2.SetColor(SELECTED_FONT_COLOR);
    }
    else {
        m_cName1.SetFont(NOT_SELECTED_NAME_FONT);
        m_cName1.SetColor(NOT_SELECTED_FONT_COLOR);
        m_cName2.SetFont(NOT_SELECTED_NAME_FONT2);
        m_cName2.SetColor(NOT_SELECTED_FONT_COLOR);
    }
    RecalculateNamePosition();
}
VOID CTradePairName::Release(VOID)
{
    m_cName1.Release();
    m_cName2.Release();
}

VOID CTradePairName::RecalculateNamePosition(VOID)
{
    m_cName1.OnUpdate(0.0f);
    const SHLVector2D& sSize = m_cName1.GetLocalSize();
    m_cName2.SetLocalPosition(sSize.x, m_cName2.GetLocalPosition().y);
    m_cName2.OnUpdate(0.0f);
}


VOID CTradePairName::OnRender(FLOAT fOffsetX, FLOAT fOffsetY) const
{
    const SHLVector2D& sWorldPos = GetWorldPosition();
    fOffsetX += sWorldPos.x;
    fOffsetY += sWorldPos.y;
    m_cName1.OnRender(fOffsetX, fOffsetY);
    m_cName2.OnRender(fOffsetX, fOffsetY);
}

VOID CTradePairName::OnWorldCoordinatesUpdated(VOID)
{
    // do nothing
}
