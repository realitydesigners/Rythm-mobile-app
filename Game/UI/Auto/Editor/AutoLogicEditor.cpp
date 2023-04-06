#include "stdafx.h"
#include "AutoLogicEditor.h"
#include "EventID.h"
#include "EventManager.h"
#include "PatternData.h"

VOID CAutoLogicEditor::PostEvent(CEvent& cEvent, FLOAT fDelay)
{
    // do nothing
}


CAutoLogicEditor::CAutoLogicEditor() :
CUIContainer(),
m_uPatternIndex(0),
INITIALIZE_TEXT_LABEL(m_cNameTxt)
{
    
}

CAutoLogicEditor::~CAutoLogicEditor()
{
    
}

VOID CAutoLogicEditor::Initialize(VOID)
{
    m_cBG.SetLocalSize(AUTO_LOGIC_EDITOR_WIDTH, AUTO_LOGIC_EDITOR_HEIGHT);
    m_cBG.SetColor(RGBA(0x1F, 0x1F, 0x1F, 0xFF));
    m_cBG.SetBorderColor(RGBA(0x7F, 0x7F, 0x7F, 0xFF));
    AddChild(m_cBG);
    
    m_cNameTxt.SetAnchor(0.0f, 0.5f);
    m_cNameTxt.SetFont(EGAMEFONT_SIZE_14);
    m_cNameTxt.SetLocalPosition(20.0f, 50.0f);
    AddChild(m_cNameTxt);
    
    m_cTable.SetLocalPosition(15.0f, 70.0f);
    const CHAR* szHeaders[8] = {
        "F", "Up/Dn", "WasL", "WasH", "IsL", "IsH", "Match", "T"
    };
    const FLOAT afSize[8] = {
        30.0f, 60.0f, 50.0f, 50.0f, 50.0f, 50.0f, 80.0f, 30.0f
    };
    m_cTable.Initialize(szHeaders, afSize, 8, 400.0f, 0);
    AddChild(m_cTable);
    
    Clear();
}

VOID CAutoLogicEditor::Refresh(UINT uPatternIndex)
{
    m_uPatternIndex = uPatternIndex;
    const SPattern& sPattern = CPatternDataMgr::GetPattern(uPatternIndex);
    CHAR szBuffer[128];
    snprintf(szBuffer, 128, "ID: %d    Name: %s", uPatternIndex, sPattern.szName);
    m_cNameTxt.SetString(szBuffer);

    const CHAR* szData[8];
    CHAR szFrame[32];
    CHAR szWasL[32], szWasH[32], szIsL[32], szIsH[32];
    CHAR szT[32];
    const SPatternRow* psRow = CPatternDataMgr::GetPatternRow(sPattern.uIndexOffset);
    m_cTable.ClearEntries();
    for (UINT uIndex = 0; sPattern.uPatternNum > uIndex; ++uIndex) {
        const SPatternRow& sRow = psRow[uIndex];
        snprintf(szFrame, 32, "%d", 9 - sRow.byDepthIndex);
        szData[0] = szFrame;
        szData[1] = sRow.bUp ? "Up" : "Dn";
        if (PATTERN_UNUSED == sRow.byWasLow) {
            szData[2] = "-";
        }
        else {
            snprintf(szWasL, 32, "%d", sRow.byWasLow);
            szData[2] = szWasL;
        }
        if (PATTERN_UNUSED == sRow.byWasHigh) {
            szData[3] = "-";
        }
        else {
            snprintf(szWasH, 32, "%d", sRow.byWasHigh);
            szData[3] = szWasH;
        }
        if (PATTERN_UNUSED == sRow.byIsLow) {
            szData[4] = "-";
        }
        else {
            snprintf(szIsL, 32, "%d", sRow.byIsLow);
            szData[4] = szIsL;
        }
        if (PATTERN_UNUSED == sRow.byIsHigh) {
            szData[5] = "-";
        }
        else {
            snprintf(szIsH, 32, "%d", sRow.byIsHigh);
            szData[5] = szIsH;
        }
        switch (sRow.eMatch) {
            case EPATTERN_MATCH_NONE:
                szData[6] = "None";
                szData[7] = "-";
                break;
            case EPATTERN_MATCH_TOP:
                szData[6] = "Top";
                snprintf(szT, 32, "%d", 9 - sRow.byMatchTargetDepthIndex);
                szData[7] = szT;
                break;
            case EPATTERN_MATCH_BOTTOM:
                szData[6] = "Bottom";
                snprintf(szT, 32, "%d", 9 - sRow.byMatchTargetDepthIndex);
                szData[7] = szT;
                break;
            default:
                ASSERT(FALSE);
                szData[6] = "Error";
                break;
        }
        m_cTable.AddRow(szData, 8, 0);
    }
}
VOID CAutoLogicEditor::Clear(VOID)
{
    m_cNameTxt.SetString("Empty");
}

VOID CAutoLogicEditor::Release(VOID)
{
    m_cBG.RemoveFromParent();
    m_cNameTxt.Release();
    m_cNameTxt.RemoveFromParent();
    
    m_cTable.Release();
    m_cTable.RemoveFromParent();
}
