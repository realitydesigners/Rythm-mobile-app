#include "stdafx.h"
#if defined(MAC_PORT)
#include "UIEditor.h"
#include "NibDefs.h"
#include "json.h"
#include "Texture.h"
#include "PngFile.h"
#include "AppResourceManager.h"
#include "AutoPtr.h"
#include "FontManager.h"
#include "NibManager.h"
#include <map>
#include <fstream>
#include <string.h>
#include <math.h>


typedef std::pair<CUIWidget*, SNibEditInfo*> CWidgetNibPair;
static CWidgetEditNibMap s_cNibMap;


SNibEditInfo* CUIEditor::FindNib(CUIWidget* pcWidget)
{
    CWidgetEditNibMap::iterator itFound = s_cNibMap.find(pcWidget);
    if (s_cNibMap.end() == itFound) {
        return NULL;
    }
    return itFound->second;
}

CWidgetEditNibMap& CUIEditor::GetNibMap(VOID)
{
    return s_cNibMap;
}

VOID CUIEditor::DoLoadNib(const CHAR* szFilePath)
{
    // release previously used nibs
    UnloadNib();
    std::string cErrString;
    CUIWidget* pcWidget = CNibManager::ExtractWidgetHierarchy(szFilePath, TRUE, m_cBaseContainer, s_cNibMap, cErrString, m_cClassName);
    if (NULL == pcWidget) {
        m_cWorkingDirectory.SetString(cErrString.c_str());
        return;
    }
    m_pcBaseWidget = CUIContainer::CastToMe(pcWidget);
    if (NULL == m_pcWidgetName) {
        m_cWorkingDirectory.SetString("main widget is not a container!");
        return;
    }
    CHAR szBuffer[1024];
    snprintf(szBuffer, 1024, "loaded %s", szFilePath);
    ToggleInput(m_bInputEnabled);
    m_cWorkingDirectory.SetString(szBuffer);
    m_bEdited = FALSE;
}

VOID CUIEditor::RecursiveReleaseWidget(CUIWidget& cWidget)
{
    CUIContainer* pcContainer = CUIContainer::CastToMe(&cWidget);
    if (NULL != pcContainer) { // if this is a container
        // recursively go into the child and release them
        CUIWidget* pcChild = pcContainer->GetChild();
        while (NULL != pcChild) {
            CUIWidget* pcTmp = pcChild;
            pcChild = pcChild->GetNextSibling();
            RecursiveReleaseWidget(*pcTmp);
        }
    }
    CWidgetEditNibMap::iterator itFound = s_cNibMap.find(&cWidget);
    if (s_cNibMap.end() != itFound) {
        SNibEditInfo* psNib = itFound->second;
        CNibManager::ReleaseNib(psNib);
        s_cNibMap.erase(itFound);
    }
    cWidget.RemoveFromParent();
    cWidget.Release();
    delete &cWidget;
}
VOID CUIEditor::UnloadNib(VOID)
{
    RefreshLabels();
    if (NULL != m_pcBaseWidget) {
        RecursiveReleaseWidget(*m_pcBaseWidget);
        m_pcBaseWidget = NULL;
    }
    ASSERT(0 == s_cNibMap.size());
    for ( auto& itNib : s_cNibMap ) {
        SNibEditInfo* psNib = itNib.second;
        CNibManager::ReleaseNib(psNib);
    }
    s_cNibMap.clear();
    m_cSelectedWidget.clear();
    m_cClassName.clear();
    if (NULL != m_pcWindowTitle) {
        m_pcWindowTitle->SetString("UI Editor");
    }
}

static CUIWidget* RecursivePick(CUIWidget* pcWidget, const SHLVector2D& sPos)
{
    if (NULL == pcWidget) {
        return NULL;
    }
    CUIWidget* pcPicked = NULL;
    if (pcWidget->HitTest(sPos)) {
        pcPicked = pcWidget;
    }
    CUIContainer* pcContainer = CUIContainer::CastToMe(pcWidget);
    if (NULL != pcContainer) {
        CUIWidget* pcChild = pcContainer->GetChild();
        while (NULL != pcChild) {
            CUIWidget* pcChildPicked = RecursivePick(pcChild, sPos);
            if (NULL != pcChildPicked) {
                pcPicked = pcChildPicked;
            }
            pcChild = pcChild->GetNextSibling();
        }
    }
    return pcPicked;
}
CUIWidget* CUIEditor::TryPick(FLOAT fX, FLOAT fY)
{
    const SHLVector2D sWorldPos = { fX, fY };
    // perform recursive pick
    return RecursivePick(m_pcBaseWidget, sWorldPos);
}
BOOLEAN CUIEditor::CanMoveAnchor(FLOAT fX, FLOAT fY)
{
    CUIWidget* pcSelectedWidget = GetSelectedWidget();
    if (NULL == pcSelectedWidget) {
        return FALSE;
    }
    const SHLVector2D& sWorldPos = pcSelectedWidget->GetWorldPosition();
    const SHLVector2D& sWorldSize = pcSelectedWidget->GetWorldSize();
    const SHLVector2D& sAnchor = pcSelectedWidget->GetAnchor();
    const FLOAT fRightX = sWorldPos.x + sWorldSize.x * sAnchor.x;
    const FLOAT fDiffX = fX - fRightX;
    if (WIDGET_RESIZE_SIZE < fabsf(fDiffX)) {
        return FALSE;
    }
    const FLOAT fBottomY = sWorldPos.y + sWorldSize.y * sAnchor.y;
    const FLOAT fDiffY = fY - fBottomY;
    return (WIDGET_RESIZE_SIZE >= fabsf(fDiffY));
}
BOOLEAN CUIEditor::CanDoResize(FLOAT fX, FLOAT fY)
{
    CUIWidget* pcSelectedWidget = GetSelectedWidget();
    if (NULL == pcSelectedWidget) {
        return FALSE;
    }
    const SHLVector2D& sWorldPos = pcSelectedWidget->GetWorldPosition();
    const SHLVector2D& sWorldSize = pcSelectedWidget->GetWorldSize();
    const FLOAT fRightX = sWorldPos.x + sWorldSize.x + WIDGET_LINE_BORDER;
    const FLOAT fDiffX = fX - fRightX;
    if ((WIDGET_RESIZE_SIZE * 0.5f) < fabsf(fDiffX)) {
        return FALSE;
    }
    const FLOAT fBottomY = sWorldPos.y + sWorldSize.y + WIDGET_LINE_BORDER;
    const FLOAT fDiffY = fY - fBottomY;
    return ((WIDGET_RESIZE_SIZE * 0.5f) >= fabsf(fDiffY));
}

VOID CUIEditor::TryMoveSelectedWidget(FLOAT fX, FLOAT fY)
{
    for (auto& pcSelectedWidget : m_cSelectedWidget ) {
        SNibEditInfo* psNib = FindNib(pcSelectedWidget);
        if (NULL == psNib) {
            ASSERT(FALSE);
            continue;
        }
        CUIWidget* pcParent = pcSelectedWidget->GetParent();
        if (NULL == pcParent) {
            ASSERT(FALSE);
            continue;
        }
        const SHLVector2D& sParentSize = pcParent->GetLocalSize();
        if (m_bResizeMode) {
            const SHLVector2D& sSize = pcSelectedWidget->GetLocalSize();
            FLOAT fNewW = sSize.x + fX;
            FLOAT fNewH = sSize.y + fY;
            if (1.0f > fNewW) {
                fNewW = 1.0f;
            }
            if (1.0f > fNewH) {
                fNewH = 1.0f;
            }
            if (psNib->sSize.bFixedSize) {
                psNib->sSize.afValues[0] = fNewW;
                psNib->sSize.afValues[1] = fNewH;
            }
            else {
                if (0.0f < psNib->sSize.fAspectRatio) {
                    const FLOAT fNewRatio = fNewW / fNewH;
                    if (psNib->sSize.fAspectRatio < fNewRatio) {
                        // either x become smaller or y become bigger.
                        if (0.0f < fX) { // if user made x bigger
                            // scale up y as well
                            fNewH = fNewW / psNib->sSize.fAspectRatio;
                        }
                        else { // scale down x
                            fNewW = psNib->sSize.fAspectRatio * fNewH;
                        }
                    }
                    else if (psNib->sSize.fAspectRatio > fNewRatio) {
                            // either x become bigger, or y become smaller
                        if (0.0f < fY) { // if user made y become bigger.
                            fNewW = psNib->sSize.fAspectRatio * fNewH;
                        }
                        else {
                            fNewH = fNewW / psNib->sSize.fAspectRatio;
                        }
                    }
                }
                const FLOAT fNewXRatio = fNewW / sParentSize.x;
                const FLOAT fNewYRatio = fNewH / sParentSize.y;
                psNib->sSize.afValues[0] = fNewXRatio;
                psNib->sSize.afValues[1] = fNewYRatio;
            }
        }
        else {
            if (m_bAnchorMode) {
                const SHLVector2D& sSize = pcSelectedWidget->GetLocalSize();
                const FLOAT fOldAnchorX = psNib->sAnchor.x;
                psNib->sAnchor.x += fX / sSize.x;
                CLAMP(psNib->sAnchor.x, 0.0f, 1.0f);
                const FLOAT fAnchorXMoved = psNib->sAnchor.x - fOldAnchorX;
                const FLOAT fOldAnchorY = psNib->sAnchor.y;
                psNib->sAnchor.y += fY / sSize.y;
                CLAMP(psNib->sAnchor.y, 0.0f, 1.0f);
                const FLOAT fAnchorYMoved = psNib->sAnchor.y - fOldAnchorY;
                if (psNib->sSize.bFixedSize) {
                    psNib->sPos.afValues[0] += fAnchorXMoved * psNib->sSize.afValues[0] / sParentSize.x;
                    psNib->sPos.afValues[1] += fAnchorYMoved * psNib->sSize.afValues[1] / sParentSize.y;
                }
                else {
                    psNib->sPos.afValues[0] += fAnchorXMoved * psNib->sSize.afValues[0];
                    psNib->sPos.afValues[1] += fAnchorYMoved * psNib->sSize.afValues[1];
                }
            }
            else {
                FLOAT fMoveX = fX;
                FLOAT fMoveY = fY;
                if (EWidgetPosition_Right == psNib->sPos.abyAlign[0]) {
                    fMoveX = -fMoveX;
                }
                if (EWidgetPosition_Bottom == psNib->sPos.abyAlign[1]) {
                    fMoveY = -fMoveY;
                }
                if (psNib->sPos.bFixedSize) {
                    psNib->sPos.afValues[0] += fMoveX;
                    psNib->sPos.afValues[1] += fMoveY;
                }
                else {
                    psNib->sPos.afValues[0] += (fMoveX) / sParentSize.x;
                    psNib->sPos.afValues[1] += (fMoveY) / sParentSize.y;
                }
            }
        }
        CNibManager::UpdateWidgetFromNibWithParentSize(*psNib, *pcSelectedWidget, sParentSize, s_cNibMap);
    }
    m_bEdited = TRUE;
    RefreshLabels();
}

VOID CUIEditor::UpdateWidgetFromNib(const SNibEditInfo& sNib, CUIWidget& cWidget)
{
    CUIWidget* pcParent = cWidget.GetParent();
    if (NULL == pcParent) {
        ASSERT(FALSE);
        return;
    }
    CNibManager::UpdateWidgetFromNibWithParentSize(sNib, cWidget, pcParent->GetLocalSize(), s_cNibMap);
}

VOID CUIEditor::DoSaveNib(const CHAR* szFilePath)
{
    CHAR szErrMsg[1024];
    FILE* phOut = fopen(szFilePath, "w");
    if (NULL == phOut) {
        snprintf(szErrMsg, 1024, "fail write %s", szFilePath);
        m_cWorkingDirectory.SetString(szErrMsg);
        return;
    }
    CAutoFile cFile(phOut); // to auto close.
    if (NULL != m_pcBaseWidget) {
        if (!WriteWidgetToFile(*m_pcBaseWidget, phOut, 0, FALSE)) {
            m_cWorkingDirectory.SetString("base widget output error!");
            return;
        }
    }
    m_bEdited = FALSE;
}

static VOID PrintLineToFile(FILE* phOut, const CHAR* szLine, UINT uTabNum)
{
    UINT uIndex = 0;
    for ( ; uTabNum > uIndex; ++uIndex ) {
        fprintf(phOut, "\t");
    }    
    fputs(szLine, phOut);
}
static std::string GetEscapeSlashString(const std::string& cString)
{
    std::string cNewString;
    const CHAR* szOldString = cString.c_str();
    const UINT uLen = (UINT)strlen(szOldString);
    UINT uIndex = 0;
    for ( ; uLen > uIndex; ++uIndex ) {
        if ('\\' == szOldString[uIndex]) {
            cNewString.push_back('\\');
        }
        cNewString.push_back(szOldString[uIndex]);
    }
    return cNewString;
    
}


BOOLEAN CUIEditor::WriteWidgetWithNibToFile(CUIWidget& cWidget, const SNibEditInfo& sInfo, FILE* phOut, UINT uTabNum, BOOLEAN bDrawFinalComma)
{
    CHAR szBuffer[1024];
    PrintLineToFile(phOut, "{\n", uTabNum);
    if (m_pcBaseWidget == &cWidget) { // if root widget, write classname
        snprintf(szBuffer, 1024, "\"classname\" : \"%s\",\n", m_cClassName.c_str());
        PrintLineToFile(phOut, szBuffer, uTabNum + 1);
    }
    // write name (optional)
    if (0 < sInfo.cName.length()) {
        snprintf(szBuffer, 1024, "\"name\" : \"%s\",\n", sInfo.cName.c_str());
        PrintLineToFile(phOut, szBuffer, uTabNum + 1);
    }
    // write type
    const CHAR* szType = CNibManager::GetWidgetTypeString(sInfo.eType);
    if (NULL == szType) {
        return FALSE;
    }
    snprintf(szBuffer, 1024, "\"type\" : \"%s\",\n", szType);
    PrintLineToFile(phOut, szBuffer, uTabNum + 1);
    
    // write size information
    const SNibSizeInfo& sSizeInfo = sInfo.sSize;
    snprintf(szBuffer, 1024, "\"size\" : { \"fixed\" : %s, \"values\" : [ %.3f, %.3f ], \"aspect\" : %.3f },\n",
             sSizeInfo.bFixedSize ? "true" : "false",
             sSizeInfo.afValues[0],
             sSizeInfo.afValues[1],
             sSizeInfo.fAspectRatio);
    PrintLineToFile(phOut, szBuffer, uTabNum + 1);
    
    // write position information
    const SNibPosInfo& sPosInfo = sInfo.sPos;
    snprintf(szBuffer, 1024, "\"pos\" : { \"fixed\" : %s, \"x\" : { \"align\" : \"%s\", \"value\" : %.3f  }, \"y\" : { \"align\" : \"%s\", \"value\" : %.3f } },\n",
             sPosInfo.bFixedSize ? "true" : "false",
             CNibManager::GetAlignmentTypeString((EWidgetPositionType)sPosInfo.abyAlign[0]),
             sPosInfo.afValues[0],
             CNibManager::GetAlignmentTypeString((EWidgetPositionType)sPosInfo.abyAlign[1]),
             sPosInfo.afValues[1]);
    PrintLineToFile(phOut, szBuffer, uTabNum + 1);
    
    // write image information if exists
    if (NULL != sInfo.psImg) {
        const SNibEditImageInfo& sImg = *sInfo.psImg;
        const BYTE byR = GET_RED(sImg.uColor);
        const BYTE byG = GET_GREEN(sImg.uColor);
        const BYTE byB = GET_BLUE(sImg.uColor);
        const BYTE byA = GET_ALPHA(sImg.uColor);
        PrintLineToFile(phOut, "\"img\" : {\n", uTabNum + 1);
        
        snprintf(szBuffer, 1024, "\"file\" : \"%s\",\n", sImg.cImgPath.c_str());
        PrintLineToFile(phOut, szBuffer, uTabNum + 2);
        snprintf(szBuffer, 1024, "\"color\" : [ %d, %d, %d, %d ],\n", byR, byG, byB, byA);
        PrintLineToFile(phOut, szBuffer, uTabNum + 2);
        snprintf(szBuffer, 1024, "\"rotation\" : %.1f,\n", sImg.fRotation);
        PrintLineToFile(phOut, szBuffer, uTabNum + 2);
        snprintf(szBuffer, 1024, "\"offset\" : [ %d, %d, %d, %d ],\n", sImg.abyOffset[0], sImg.abyOffset[1], sImg.abyOffset[2], sImg.abyOffset[3]);
        PrintLineToFile(phOut, szBuffer, uTabNum + 2);
        snprintf(szBuffer, 1024, "\"vertical\" : %s\n", sImg.bVertical ? "true" : "false");
        PrintLineToFile(phOut, szBuffer, uTabNum + 2);
        
        PrintLineToFile(phOut, "},\n", uTabNum + 1);
    }
    // write text information if exists
    if (NULL != sInfo.psText) {
        const SNibEditTextInfo& sText = *sInfo.psText;
        const BYTE byR = GET_RED(sText.uColor);
        const BYTE byG = GET_GREEN(sText.uColor);
        const BYTE byB = GET_BLUE(sText.uColor);
        const BYTE byA = GET_ALPHA(sText.uColor);
        
        const BYTE byEfxR = GET_RED(sText.uEfxColor);
        const BYTE byEfxG = GET_GREEN(sText.uEfxColor);
        const BYTE byEfxB = GET_BLUE(sText.uEfxColor);
        const BYTE byEfxA = GET_ALPHA(sText.uEfxColor);
        std::string cFormattedString = GetEscapeSlashString(sText.cDefaultString);
        
        PrintLineToFile(phOut, "\"text\" : {\n", uTabNum + 1);
        
        snprintf(szBuffer, 1024, "\"size\" : %d,\n", (UINT)CFontManager::GetFontSize(sText.eFontSize));
        PrintLineToFile(phOut, szBuffer, uTabNum + 2);
        snprintf(szBuffer, 1024, "\"default\" : \"%s\",\n", cFormattedString.c_str());
        PrintLineToFile(phOut, szBuffer, uTabNum + 2);
        snprintf(szBuffer, 1024, "\"align\" : \"%s\",\n", CNibManager::GetTextLabelAlignmentString(sText.uTextAlignment));
        PrintLineToFile(phOut, szBuffer, uTabNum + 2);
        snprintf(szBuffer, 1024, "\"color\" : [ %d, %d, %d, %d ],\n", byR, byG, byB, byA);
        PrintLineToFile(phOut, szBuffer, uTabNum + 2);
        snprintf(szBuffer, 1024, "\"effect\" : \"%s\",\n", CNibManager::GetTextLabelEffectString(sText.uEfxType));
        PrintLineToFile(phOut, szBuffer, uTabNum + 2);
        snprintf(szBuffer, 1024, "\"effect_params\" : [ %.1f, %.1f ],\n", sText.afEfxParams[0], sText.afEfxParams[1]);
        PrintLineToFile(phOut, szBuffer, uTabNum + 2);
        snprintf(szBuffer, 1024, "\"effect_color\" : [ %d, %d, %d, %d ]\n", byEfxR, byEfxG, byEfxB, byEfxA);
        PrintLineToFile(phOut, szBuffer, uTabNum + 2);
        
        PrintLineToFile(phOut, "},\n", uTabNum + 1);
    }
    // write button information if exists
    if (NULL != sInfo.psBtn) {
        CUIButton* pcBtn = CUIButton::CastToMe(&cWidget);
        if (NULL == pcBtn) {
            ASSERT(FALSE);
            return FALSE;
        }
        snprintf(szBuffer, 1024, "\"btn_offset\" : %.1f,\n", sInfo.psBtn->fPressedOffset);
        PrintLineToFile(phOut, szBuffer, uTabNum + 1);
        snprintf(szBuffer, 1024, "\"btn_toggle\" : %s,\n", sInfo.psBtn->bToggleBtn ? "true" : "false");
        PrintLineToFile(phOut, szBuffer, uTabNum + 1);
        PrintLineToFile(phOut, "\"btn\" : [\n", uTabNum + 1);
        
        CUIWidget* pcIdle = pcBtn->GetIdleWidget();
        CUIWidget* pcPressed = pcBtn->GetPressedWidget();
        if (NULL == pcIdle || NULL == pcPressed) {
            ASSERT(FALSE);
            return FALSE;
        }
        const SNibEditBtnInfo& sBtn = *sInfo.psBtn;
        WriteWidgetWithNibToFile(*pcIdle, sBtn.sIdleInfo, phOut, uTabNum + 2, TRUE);
        WriteWidgetWithNibToFile(*pcPressed, sBtn.sPressedInfo, phOut, uTabNum + 2, FALSE);
        PrintLineToFile(phOut, "],\n", uTabNum + 1);
    }
    
    CUIContainer* pcContainer = CUIContainer::CastToMe(&cWidget);
    if (NULL != pcContainer) { // if this is a container.
        CUIWidget* pcChild = pcContainer->GetChild();
        if (NULL != pcChild) { // if there is children inside
            PrintLineToFile(phOut, "\"children\" : [\n", uTabNum + 1);
            while (NULL != pcChild) {
                CUIWidget* pcTmp = pcChild;
                pcChild = pcChild->GetNextSibling();
                WriteWidgetToFile(*pcTmp, phOut, uTabNum + 2, (NULL != pcChild));
            }
            PrintLineToFile(phOut, "],\n", uTabNum + 1);
        }
    }
    // write anchor last
    snprintf(szBuffer, 1024, "\"anchor\" : [ %.3f, %.3f ]\n", sInfo.sAnchor.x, sInfo.sAnchor.y);
    PrintLineToFile(phOut, szBuffer, uTabNum + 1);
    
    // close bracket
    if (bDrawFinalComma) {
        PrintLineToFile(phOut, "},\n", uTabNum);
    }
    else {
        PrintLineToFile(phOut, "}\n", uTabNum);
    }
    return TRUE;
}
BOOLEAN CUIEditor::WriteWidgetToFile(CUIWidget& cWidget, FILE* phOut, UINT uTabNum, BOOLEAN bDrawFinalComma)
{
    const SNibEditInfo* psNib = FindNib(&cWidget);
    if (NULL == psNib) {
        return FALSE;
    }
    return WriteWidgetWithNibToFile(cWidget, *psNib, phOut, uTabNum, bDrawFinalComma);
}

VOID CUIEditor::RefreshSelectedWidget(VOID)
{
    CUIWidget* pcSelectedWidget = GetSelectedWidget();
    if (NULL == pcSelectedWidget) {
        return;
    }
    SNibEditInfo* psNib = FindNib(pcSelectedWidget);
    if (NULL == psNib) {
        return;
    }
    CUIContainer* pcParent = pcSelectedWidget->GetParent();
    if (NULL == pcParent) {
        return;
    }
    CWidgetEditNibMap::iterator itFound = s_cNibMap.find(pcSelectedWidget);
    if (s_cNibMap.end() != itFound) {
        s_cNibMap.erase(itFound);
    }
    std::string cErr;
    CUIWidget* pcNewWidget = CNibManager::CreateWidgetFromEditNib(*pcParent, *psNib, s_cNibMap, TRUE, cErr);
    if (NULL == pcNewWidget) {
        m_cWorkingDirectory.SetString(cErr.c_str());
        return;
    }
    pcNewWidget->SetEnabled(pcSelectedWidget->IsEnabled());
    s_cNibMap.insert(CWidgetNibPair(pcNewWidget, psNib));
    pcParent->AddChildAfter(*pcNewWidget, *pcSelectedWidget);
    pcNewWidget->UpdateWorldCoordinates(TRUE);
    // if this new nib is a container, transfer all children to this new nib
    CUIContainer* pcNewContainer = CUIContainer::CastToMe(pcNewWidget);
    if (NULL != pcNewContainer) {
        CUIContainer* pcOldContainer = CUIContainer::CastToMe(pcSelectedWidget);
        if (NULL == pcOldContainer) {
            ASSERT(FALSE);
        }
        else {
            CUIWidget* pcChild = pcOldContainer->GetChild();
            while (NULL != pcChild) {
                CUIWidget* pcTmp = pcChild;
                pcChild = pcChild->GetNextSibling();
                pcTmp->RemoveFromParent();
                pcNewContainer->AddChild(*pcTmp);
            }
        }
    }
    // delete old widget, add new widget
    pcSelectedWidget->RemoveFromParent();
    pcSelectedWidget->Release();
    delete pcSelectedWidget;
    m_cSelectedWidget.clear();
    m_cSelectedWidget.push_back(pcNewWidget);
    RefreshLabels();
    m_bEdited = TRUE;
}

VOID CUIEditor::SnapImageToTextureSize(VOID)
{
    CUIWidget* pcSelectedWidget = GetSelectedWidget();
    if (NULL == pcSelectedWidget) {
        return;
    }
    SNibEditInfo* psNib = FindNib(pcSelectedWidget);
    if (NULL == psNib) {
        return;
    }
    CTexture* pcTex = NULL;
    switch (psNib->eType) {
        case EBaseWidget_Image:
            {
                CUIImage* pcImg = CUIImage::CastToMe(pcSelectedWidget);
                if (NULL != pcImg) {
                    pcTex = pcImg->GetTexture();
                }
            }
            break;
        case EBaseWidget_3PartImage:
            {
                CUI3PartImage* pcImg = CUI3PartImage::CastToMe(pcSelectedWidget);
                if (NULL != pcImg) {
                    pcTex = pcImg->GetTexture();
                }
            }
            break;
        case EBaseWidget_9PartImage:
            {
                CUI9PartImage* pcImg = CUI9PartImage::CastToMe(pcSelectedWidget);
                if (NULL != pcImg) {
                    pcTex = pcImg->GetTexture();
                }
            }
            break;

        default:
            break;
    }
    if (NULL == pcTex) {
        return;
    }
    psNib->sSize.bFixedSize = TRUE;
    psNib->sSize.afValues[0] = pcTex->GetOriginalWidth();
    psNib->sSize.afValues[1] = pcTex->GetOriginalHeight();
    psNib->sSize.fAspectRatio = psNib->sSize.afValues[0] / psNib->sSize.afValues[1];
    UpdateWidgetFromNib(*psNib, *pcSelectedWidget);
    RefreshLabels();
}

VOID CUIEditor::ConvertImageToButton(VOID)
{
    CUIWidget* pcSelectedWidget = GetSelectedWidget();
    if (NULL == pcSelectedWidget) {
        return;
    }
    CUIContainer* pcParent = pcSelectedWidget->GetParent();
    if (NULL == pcParent) {
        return;
    }
    SNibEditInfo* psOldNib = FindNib(pcSelectedWidget);
    if (NULL == psOldNib) {
        return;
    }
    switch (psOldNib->eType) {
        case EBaseWidget_9PartImage:
        case EBaseWidget_Image:
        case EBaseWidget_3PartImage:
            if (NULL == psOldNib->psImg) {
                return;
            }
            break;
        default:
            return;
    }
    SNibEditInfo* psNewNib = CNibManager::CreateNewNib();
    if (NULL == psNewNib) {
        return;
    }
    psNewNib->eType     = EBaseWidget_Button;
    psNewNib->sSize     = psOldNib->sSize;
    psNewNib->sPos      = psOldNib->sPos;
    psNewNib->sAnchor   = psOldNib->sAnchor;
    psNewNib->cName     = psOldNib->cName;
    SNibEditBtnInfo* psBtn = new SNibEditBtnInfo;
    if (NULL == psBtn) {
        delete psNewNib;
        return;
    }
    psBtn->fPressedOffset = 2.0f;
    psBtn->bToggleBtn = FALSE;
    psNewNib->psBtn = psBtn;
    psBtn->sIdleInfo.eType = psOldNib->eType;
    psBtn->sIdleInfo.sSize.bFixedSize = FALSE;
    psBtn->sIdleInfo.sSize.afValues[0] = 1.0f;
    psBtn->sIdleInfo.sSize.afValues[1] = 1.0f;
    psBtn->sIdleInfo.sSize.fAspectRatio = 0.0f;
    psBtn->sIdleInfo.sPos.bFixedSize = FALSE;
    psBtn->sIdleInfo.sPos.abyAlign[0] = EWidgetPosition_Left;
    psBtn->sIdleInfo.sPos.abyAlign[1] = EWidgetPosition_Top;
    psBtn->sIdleInfo.sPos.afValues[0] = 0.0f;
    psBtn->sIdleInfo.sPos.afValues[1] = 0.0f;
    psBtn->sIdleInfo.sAnchor.x = 0.0f;
    psBtn->sIdleInfo.sAnchor.y = 0.0f;
    psBtn->sIdleInfo.psText = NULL;
    psBtn->sIdleInfo.psBtn = NULL;
    psBtn->sIdleInfo.psImg = new SNibEditImageInfo;
    if (NULL == psBtn->sIdleInfo.psImg) {
        return;
    }
    const SNibEditImageInfo& sSrcImg = *psOldNib->psImg;
    SNibEditImageInfo& sIdleImg = *psBtn->sIdleInfo.psImg;
    sIdleImg.cImgPath = sSrcImg.cImgPath;
    sIdleImg.pcTexture = NULL;
    sIdleImg.uColor = sSrcImg.uColor;
    sIdleImg.fRotation = sSrcImg.fRotation;
    memcpy(sIdleImg.abyOffset, sSrcImg.abyOffset, 4);
    sIdleImg.bVertical = sSrcImg.bVertical;
    
    // copy over to pressed image as well
    psBtn->sPressedInfo = psBtn->sIdleInfo;
    // duplicate img pointer
    psBtn->sPressedInfo.psImg = new SNibEditImageInfo;
    if (NULL == psBtn->sPressedInfo.psImg) {
        return;
    }
    *(psBtn->sPressedInfo.psImg) = *(psBtn->sIdleInfo.psImg);
    
    CWidgetEditNibMap::iterator itFound = s_cNibMap.find(pcSelectedWidget);
    if (s_cNibMap.end() != itFound) {
        s_cNibMap.erase(itFound);
    }
    std::string cErr;
    CUIWidget* pcNewWidget = CNibManager::CreateWidgetFromEditNib(*pcParent, *psNewNib, s_cNibMap, TRUE, cErr);
    if (NULL == pcNewWidget) {
        m_cWorkingDirectory.SetString(cErr.c_str());
        return;
    }
    pcNewWidget->SetEnabled(m_bInputEnabled);
    s_cNibMap.insert(CWidgetNibPair(pcNewWidget, psNewNib));
    pcParent->AddChildAfter(*pcNewWidget, *pcSelectedWidget);
    pcNewWidget->UpdateWorldCoordinates(TRUE);
    
    // delete old widget, add new widget
    pcSelectedWidget->RemoveFromParent();
    pcSelectedWidget->Release();
    delete pcSelectedWidget;
    pcSelectedWidget = pcNewWidget;
    RefreshLabels();
    CNibManager::ReleaseNib(psOldNib);
    m_bEdited = TRUE;
}

VOID CUIEditor::CreateNewImage(VOID)
{
    if (NULL == m_pcBaseWidget) {
        return;
    }
    SNibEditInfo* psNewNib = CNibManager::CreateNewNib();
    if (NULL == psNewNib) {
        ASSERT(FALSE);
        return;
    }
    psNewNib->eType     = EBaseWidget_Image;
    psNewNib->sSize.bFixedSize = TRUE;
    psNewNib->sSize.fAspectRatio = 0.0f;
    psNewNib->sSize.afValues[0] = psNewNib->sSize.afValues[1] = 200.0f;
    
    psNewNib->sPos.bFixedSize = FALSE;
    psNewNib->sPos.abyAlign[0] = EWidgetPosition_Left;
    psNewNib->sPos.abyAlign[1] = EWidgetPosition_Top;
    psNewNib->sPos.afValues[0] = psNewNib->sPos.afValues[1] = 0.5f;
    psNewNib->sAnchor.x = psNewNib->sAnchor.y = 0.5f;

    SNibEditImageInfo* psNewImg = new SNibEditImageInfo;
    if (NULL == psNewImg) {
        ASSERT(FALSE);
        return;
    }
    psNewImg->cImgPath.clear();
    psNewImg->pcTexture = NULL;
    psNewImg->uColor = 0xFFFFFFFF;
    psNewImg->fRotation = 0.0f;
    psNewImg->abyOffset[0] = psNewImg->abyOffset[1] = psNewImg->abyOffset[2] = psNewImg->abyOffset[3] = 0;
    psNewImg->bVertical = FALSE;
    psNewNib->psImg = psNewImg;
    
    std::string cErr;
    CUIWidget* pcNewWidget = CNibManager::CreateWidgetFromEditNib(*m_pcBaseWidget, *psNewNib, s_cNibMap, TRUE, cErr);
    if (NULL == pcNewWidget) {
        m_cWorkingDirectory.SetString(cErr.c_str());
        ASSERT(FALSE);
        return;
    }
    s_cNibMap.insert(CWidgetNibPair(pcNewWidget, psNewNib));
    m_pcBaseWidget->AddChild(*pcNewWidget);
    pcNewWidget->UpdateWorldCoordinates(TRUE);
    m_cSelectedWidget.clear();
    m_cSelectedWidget.push_back(pcNewWidget);
    RefreshLabels();
}

VOID CUIEditor::CreateNewContainer(VOID)
{
    if (NULL == m_pcBaseWidget) {
        return;
    }
    SNibEditInfo* psNewNib = CNibManager::CreateNewNib();
    if (NULL == psNewNib) {
        ASSERT(FALSE);
        return;
    }
    psNewNib->eType     = EBaseWidget_Container;
    psNewNib->sSize.bFixedSize = TRUE;
    psNewNib->sSize.fAspectRatio = 0.0f;
    psNewNib->sSize.afValues[0] = psNewNib->sSize.afValues[1] = 100.0f;
    
    psNewNib->sPos.bFixedSize = FALSE;
    psNewNib->sPos.abyAlign[0] = EWidgetPosition_Left;
    psNewNib->sPos.abyAlign[1] = EWidgetPosition_Top;
    psNewNib->sPos.afValues[0] = psNewNib->sPos.afValues[1] = 0.5f;
    psNewNib->sAnchor.x = psNewNib->sAnchor.y = 0.5f;
    
    std::string cErr;
    CUIWidget* pcNewWidget = CNibManager::CreateWidgetFromEditNib(*m_pcBaseWidget, *psNewNib, s_cNibMap, TRUE, cErr);
    if (NULL == pcNewWidget) {
        m_cWorkingDirectory.SetString(cErr.c_str());
        ASSERT(FALSE);
        return;
    }
    s_cNibMap.insert(CWidgetNibPair(pcNewWidget, psNewNib));
    m_pcBaseWidget->AddChild(*pcNewWidget);
    pcNewWidget->UpdateWorldCoordinates(TRUE);
    m_cSelectedWidget.clear();
    m_cSelectedWidget.push_back(pcNewWidget);
    RefreshLabels();
}
VOID CUIEditor::CreateNewText(VOID)
{
    if (NULL == m_pcBaseWidget) {
        return;
    }
    SNibEditInfo* psNewNib = CNibManager::CreateNewNib();
    if (NULL == psNewNib) {
        ASSERT(FALSE);
        return;
    }
    psNewNib->eType     = EBaseWidget_TextLabel;
    psNewNib->sSize.bFixedSize = TRUE;
    psNewNib->sSize.fAspectRatio = 0.0f;
    psNewNib->sSize.afValues[0] = psNewNib->sSize.afValues[1] = 200.0f;
    
    psNewNib->sPos.bFixedSize = FALSE;
    psNewNib->sPos.abyAlign[0] = EWidgetPosition_Left;
    psNewNib->sPos.abyAlign[1] = EWidgetPosition_Top;
    psNewNib->sPos.afValues[0] = psNewNib->sPos.afValues[1] = 0.5f;
    psNewNib->sAnchor.x = psNewNib->sAnchor.y = 0.5f;
    
    SNibEditTextInfo* psText = new SNibEditTextInfo;
    if (NULL == psText) {
        ASSERT(FALSE);
        return;
    }
    psNewNib->psText = psText;
    psText->cDefaultString = "12345";
    psText->eFontSize = EGAMEFONT_SIZE_24;
    psText->uColor = 0xFFFFFFFF;
    psText->uTextAlignment = CUITextLabel::EALIGNMENT_CENTER;
    
    psText->uEfxType = CUITextLabel::EFFECT_NONE;
    psText->afEfxParams[0] = psText->afEfxParams[1] = 1.0f;
    psText->uEfxColor = RGBA(0, 0, 0, 0xFF);
    
    std::string cErr;
    CUIWidget* pcNewWidget = CNibManager::CreateWidgetFromEditNib(*m_pcBaseWidget, *psNewNib, s_cNibMap, TRUE, cErr);
    if (NULL == pcNewWidget) {
        m_cWorkingDirectory.SetString(cErr.c_str());
        ASSERT(FALSE);
        return;
    }
    s_cNibMap.insert(CWidgetNibPair(pcNewWidget, psNewNib));
    m_pcBaseWidget->AddChild(*pcNewWidget);
    pcNewWidget->UpdateWorldCoordinates(TRUE);
    m_cSelectedWidget.clear();
    m_cSelectedWidget.push_back(pcNewWidget);
    RefreshLabels();
}

static SNibEditInfo* CopyNib(SNibEditInfo& sNew, const SNibEditInfo& sNib)
{
    sNew.eType = sNib.eType;
    sNew.sSize = sNib.sSize;
    sNew.sPos = sNib.sPos;
    sNew.sAnchor = sNib.sAnchor;
    sNew.cName = sNib.cName;
    sNew.psImg = NULL;
    sNew.psText = NULL;
    sNew.psBtn = NULL;

    if (NULL != sNib.psImg) {
        SNibEditImageInfo* psImg = new SNibEditImageInfo;
        if (NULL == psImg) {
            ASSERT(FALSE);
            return NULL;
        }
        *(psImg) = *(sNib.psImg);
        psImg->pcTexture = NULL;
        sNew.psImg = psImg;
    }
    if (NULL != sNib.psText) {
        SNibEditTextInfo* psText = new SNibEditTextInfo;
        if (NULL == psText) {
            ASSERT(FALSE);
            return NULL;
        }
        *(psText) = *(sNib.psText);
        sNew.psText = psText;
    }
    if (NULL != sNib.psBtn) {
        SNibEditBtnInfo* psBtn = new SNibEditBtnInfo;
        if (NULL == psBtn) {
            ASSERT(FALSE);
            return NULL;
        }
        psBtn->fPressedOffset = sNib.psBtn->fPressedOffset;
        psBtn->bToggleBtn = sNib.psBtn->bToggleBtn;
        CopyNib(psBtn->sIdleInfo, sNib.psBtn->sIdleInfo);
        CopyNib(psBtn->sPressedInfo, sNib.psBtn->sPressedInfo);
        sNew.psBtn = psBtn;
    }
    return &sNew;
}
CUIWidget* CUIEditor::DuplicateResursively(CUIWidget& cWidget, const SNibEditInfo& sNib, CUIContainer& cParent)
{
    // duplicate self 1st.
    SNibEditInfo* psDuplicateNib = CNibManager::CreateNewNib();
    if (NULL == psDuplicateNib) {
        return NULL;
    }
    if (NULL == CopyNib(*psDuplicateNib, sNib)) {
        return NULL;
    }
    
    std::string cErr;
    CUIWidget* pcWidget = CNibManager::CreateWidgetFromEditNib(cParent, *psDuplicateNib, s_cNibMap, TRUE, cErr);
    if (NULL == pcWidget) {
        m_cWorkingDirectory.SetString(cErr.c_str());
        return NULL;
    }
    s_cNibMap.insert(CWidgetNibPair(pcWidget, psDuplicateNib));
    cParent.AddChild(*pcWidget);

    // then duplicate the children (recursively)
    CUIContainer* pcContainer = CUIContainer::CastToMe(pcWidget);
    if (NULL != pcContainer) {
        pcContainer->UpdateWorldCoordinates(TRUE);
        CUIWidget* pcChild = pcContainer->GetChild();
        while (NULL != pcChild) {
            const SNibEditInfo* psOldNib = FindNib(pcChild);
            if (NULL == psOldNib) {
                ASSERT(FALSE);
                return NULL;
            }
            DuplicateResursively(*pcChild, *psOldNib, *pcContainer);
            pcChild = pcChild->GetNextSibling();
        }
    }
    return pcWidget;
}
VOID CUIEditor::PerformDuplicate(VOID)
{
    CUIWidget* pcSelectedWidget = GetSelectedWidget();
    if (NULL == pcSelectedWidget) {
        return;
    }
    if (m_pcBaseWidget == pcSelectedWidget) {
        return;
    }
    SNibEditInfo* psNib = FindNib(pcSelectedWidget);
    if (NULL == psNib) {
        return;
    }
    CUIContainer* pcParent = pcSelectedWidget->GetParent();
    if (NULL == pcParent) {
        return;
    }
    CUIWidget* pcDuplicated = DuplicateResursively(*pcSelectedWidget, *psNib, *pcParent);
    if (NULL != pcDuplicated) {
        SNibEditInfo* psNib = FindNib(pcDuplicated);
        if (NULL == psNib) {
            return;
        }
        const SHLVector2D& sParentSize = pcParent->GetLocalSize();
        if (psNib->sPos.bFixedSize) {
            psNib->sPos.afValues[0] += 10.0f;
        }
        else {
            psNib->sPos.afValues[0] += 10.0f / sParentSize.x;
        }
        CNibManager::UpdateWidgetFromNibWithParentSize(*psNib, *pcDuplicated, sParentSize, s_cNibMap);
        m_cSelectedWidget.clear();
        m_cSelectedWidget.push_back(pcDuplicated);
        ToggleInput(m_bInputEnabled);
        RefreshLabels();
    }
}

#endif // #if defined(MAC_PORT)
