#include "stdafx.h"
#include "DepthDisplay.h"
#include "MegaZZ.h"

CDepthDisplay::CDepthDisplay() :
CUIContainer(),
m_uNum(0),
m_uDepthIndex(0)
{
    memset(m_abEnabled, 0xFF, sizeof(m_abEnabled));
    memset(m_apcDepth, 0, sizeof(m_apcDepth));
    memset(m_apcUnit, 0, sizeof(m_apcUnit));
}

CDepthDisplay::~CDepthDisplay()
{

}
 
VOID CDepthDisplay::SetEnabled(UINT uDepthIndex, BOOLEAN bEnabled)
{
    ASSERT(MEGAZZ_MAX_DEPTH > uDepthIndex);
    m_abEnabled[uDepthIndex] = bEnabled;
}

VOID CDepthDisplay::Release(VOID)
{
    for (UINT uIndex = 0; MEGAZZ_MAX_DEPTH > uIndex; ++uIndex) {
        SAFE_REMOVE_RELEASE_DELETE(m_apcDepth[uIndex]);
        SAFE_REMOVE_RELEASE_DELETE(m_apcUnit[uIndex]);
    }
    CUIContainer::Release();
}

VOID CDepthDisplay::Initialize(const FLOAT* pfSizes, UINT uStartDepthIndex, UINT uDepthNum, BOOLEAN bShowPercent)
{
    m_uNum = uDepthNum;
    m_uDepthIndex = uStartDepthIndex;
    CHAR szBuffer[64];
    UINT uIndex = 0;
    UINT uDisplayedNum = 0;
    const FLOAT fDisplayOffset = bShowPercent ? 40.0f : 30.0f;
    const FLOAT fStartX = bShowPercent ? 20.0f : 10.0f;
    for (; uDepthNum > uIndex; ++uIndex) {
        const UINT uCurrentDepthIndex = uStartDepthIndex + uIndex;
        if (MEGAZZ_MAX_DEPTH <= uIndex) {
            ASSERT(FALSE);
            return;
        }
        if (!m_abEnabled[uCurrentDepthIndex]) {
            if (NULL != m_apcDepth[uIndex]) {
                m_apcDepth[uIndex]->ClearString();
            }
            if (NULL != m_apcUnit[uIndex]) {
                m_apcUnit[uIndex]->ClearString();
            }
            continue;
        }
        const INT nDepthIndex = 9 - uCurrentDepthIndex;
        if (0 > nDepthIndex || 9 <= uCurrentDepthIndex) {
            break;
        }
        
        if (NULL == m_apcDepth[uIndex]) {
            CUITextLabel* pcLbl = NEW_TEXT_LABEL;
            if (NULL == pcLbl) {
                ASSERT(FALSE);
                return;
            }
            pcLbl->SetFont(EGAMEFONT_SIZE_12);
            pcLbl->SetAnchor(0.5f, 0.0f);
            AddChild(*pcLbl);
            m_apcDepth[uIndex] = pcLbl;
        }
        snprintf(szBuffer, 64, "F%d", 9 - uCurrentDepthIndex);
        m_apcDepth[uIndex]->SetLocalPosition(fStartX + uDisplayedNum * fDisplayOffset, 0.0f);
        m_apcDepth[uIndex]->SetString(szBuffer);
        
        if (NULL == m_apcUnit[uIndex]) {
            CUITextLabel* pcLbl = NEW_TEXT_LABEL;
            if (NULL == pcLbl) {
                ASSERT(FALSE);
                return;
            }
            pcLbl->SetFont(EGAMEFONT_SIZE_10);
            pcLbl->SetAnchor(0.5f, 0.0f);
            AddChild(*pcLbl);
            m_apcUnit[uIndex] = pcLbl;
        }
        if (9 > uCurrentDepthIndex) {
            if (bShowPercent) {
                if (1.0f > pfSizes[uCurrentDepthIndex]) {
                    snprintf(szBuffer, 64, "%.2f%%", pfSizes[uCurrentDepthIndex]);
                }
                else {
                    snprintf(szBuffer, 64, "%.1f%%", pfSizes[uCurrentDepthIndex]);
                }
            }
            else {
                snprintf(szBuffer, 64, "%.1f", pfSizes[uCurrentDepthIndex]);
            }
        }
        else {
            snprintf(szBuffer, 64, "-");
        }
        m_apcUnit[uIndex]->SetLocalPosition(fStartX + uDisplayedNum * fDisplayOffset, 15.0f);
        m_apcUnit[uIndex]->SetString(szBuffer);
        ++uDisplayedNum;
    }
    for (; MEGAZZ_MAX_DEPTH > uIndex; ++uIndex) {
        if (NULL != m_apcDepth[uIndex]) {
            m_apcDepth[uIndex]->ClearString();
        }
        if (NULL != m_apcUnit[uIndex]) {
            m_apcUnit[uIndex]->ClearString();
        }
    }
}

VOID CDepthDisplay::UpdateLabels(const CMegaZZ& cZZ)
{
    CHAR szBuffer[32];
    for (UINT uIndex = 0; m_uNum > uIndex; ++uIndex) {
        if (NULL != m_apcDepth[uIndex]) {
            const UINT uDepthIndex = m_uDepthIndex + uIndex;
            if (MEGAZZ_MAX_DEPTH <= uDepthIndex) {
                m_apcDepth[uIndex]->ClearString();
            }
            else {
                if (cZZ.GetCurrentChannel(uDepthIndex).byType == MegaZZ::EType_Up) {
                    snprintf(szBuffer, 32, "U%d", 9 - uDepthIndex);
                }
                else {
                    snprintf(szBuffer, 32, "D%d", 9 - uDepthIndex);
                }
                m_apcDepth[uIndex]->SetString(szBuffer);
            }
        }
    }
}
