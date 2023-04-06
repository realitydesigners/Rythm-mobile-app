#include "stdafx.h"
#include "GameWindow.h"
#include "CMain.h"

struct SBtnInfo {
    BOOLEAN     b3PartBtn;
    ETEX::ID    eIdle;
    ETEX::ID    ePressed;
    FLOAT       fWidth;
    FLOAT       fHeight;
    FLOAT       fTextOffset;
    CGameWindow::ELABEL_TYPE eLabelType;
};

struct SLabelInfo {
    EGAMEFONT                   eTextFont;
    UINT                        uTextColor;
    UINT                        uShadowColor;
    CUITextLabel::EFFECT_TYPE   eEffType;
    FLOAT                       afShadowOffset[2];
};
static SLabelInfo s_asLabelInfo[CGameWindow::ELABEL_TYPE_NUM] = {
    // ELABEL_COMMON_BUTTON_LABEL
    { EGAMEFONT_SIZE_20,  RGBA(0xFF, 0xFF, 0xFF, 0xFF), RGBA(0, 0, 0, 0xFF), CUITextLabel::EFFECT_SHADOW,  { 1.0f, 1.0f } },
    // ELABEL_SMALL
    { EGAMEFONT_SIZE_14,  RGBA(0xFF, 0xFF, 0xFF, 0xFF), RGBA(0, 0, 0, 0xFF), CUITextLabel::EFFECT_SHADOW,  { 1.0f, 1.0f } },
};

static SBtnInfo s_asBtnInfo[CGameWindow::EBTN_TYPE_NUM] = {
//    EBTN_TYPE_DEBUG_BTN,
    { TRUE,	 ETEX::ui_9part_filter_off, ETEX::ui_9part_filter_off,  70.0f,   70.0f, 0.0f, CGameWindow::ELABEL_COMMON_BUTTON_LABEL },
};


VOID CGameWindow::SetLabel(CUITextLabel& cLabel, const CHAR* szTextLabel, ELABEL_TYPE etype)
{
    const SLabelInfo& sInfo = s_asLabelInfo[etype];
    cLabel.SetFont(sInfo.eTextFont);
    cLabel.SetAlignment(CUITextLabel::EALIGNMENT_CENTER);
    cLabel.SetAnchor(0.5f, 0.5f);
    cLabel.SetColor(sInfo.uTextColor);
    cLabel.SetEffectColor(sInfo.uShadowColor);
    cLabel.SetEffect(sInfo.eEffType, sInfo.afShadowOffset[0], sInfo.afShadowOffset[1]);
    cLabel.SetString(szTextLabel);
}

CGameWindow::CGameWindow(UINT uGlobalID)
: CUIWindow(uGlobalID)
{

}

CGameWindow::~CGameWindow()
{
    ASSERT(0 == m_cWidgetList.size());
}

VOID CGameWindow::InitializeInternals(VOID)
{
    
}

VOID CGameWindow::Release(VOID)
{
    CUIWindow::Release();
    // delete all allocated resources
    CWidgetList::iterator it = m_cWidgetList.begin();
    CWidgetList::const_iterator end = m_cWidgetList.end();
    for ( ; end != it; ++it) {
        CUIWidget* pcWidget = (*it);
        pcWidget->Release();
        delete pcWidget;
    }
    m_cWidgetList.clear();
}

VOID CGameWindow::AutoDestroy(CUIWidget& cWidget)
{
    m_cWidgetList.push_back(&cWidget);
}

CUI9PartImage* CGameWindow::SetCustom9PartBackground(ETEX::ID eID)
{
    CUI9PartImage* pcBG = new CUI9PartImage();
    if (NULL == pcBG) {
        ASSERT(FALSE);
        return NULL;
    }
    pcBG->Set9PartTexture(eID);
    const SHLVector2D& sLocalSize = GetLocalSize();
    pcBG->SetLocalSize(sLocalSize.x, sLocalSize.y);
    m_cWidgetList.push_front(pcBG);
    AddChild(*pcBG);
    return pcBG;
}
CUIImage* CGameWindow::SetColorBackground(UINT uColor)
{
    CUIImage* pcImg = new CUIImage();
    if (NULL == pcImg) {
        ASSERT(FALSE);
        return NULL;
    }
    pcImg->SetColor(uColor);
    pcImg->SetBorderColor(RGBA(0x7F, 0x7F, 0x7F, 0xFF));
    const SHLVector2D& sLocalSize = GetLocalSize();
    pcImg->SetLocalSize(sLocalSize.x, sLocalSize.y);
    m_cWidgetList.push_front(pcImg);
    AddChild(*pcImg);
    return pcImg;
}

CUIButton* CGameWindow::CreateButton(FLOAT fX, FLOAT fY, UINT uID, const CHAR* szTextLabel, EBTN_TYPE eType, FLOAT fOverrideWidth, BOOLEAN bAutoAdd)
{
    const SBtnInfo& sInfo = s_asBtnInfo[eType];
    CUITextLabel* pcText = CreateLabel(szTextLabel, sInfo.eLabelType);
    if (NULL == pcText) {
        ASSERT(FALSE);
        return NULL;
    }
    const FLOAT fWidth = (0.0f == fOverrideWidth) ? sInfo.fWidth : fOverrideWidth;
    pcText->SetLocalPosition(fWidth * 0.5f, sInfo.fHeight * 0.5f + sInfo.fTextOffset);
    CUIWidget* pcIdle = CreateButtonWidget(eType, TRUE, fOverrideWidth);
    CUIWidget* pcPressed;
    if (sInfo.eIdle == sInfo.ePressed) {
        pcPressed = pcIdle;
    }
    else {
        pcPressed = CreateButtonWidget(eType, FALSE, fOverrideWidth);
    }
    if (NULL == pcIdle || NULL == pcPressed) {
        ASSERT(FALSE);
        return NULL;
    }
    CUIButton* pcBtn = CreateButton(fX, fY, uID, *pcIdle, *pcPressed, bAutoAdd);
    if (NULL == pcBtn) {
        ASSERT(FALSE);
        return NULL;
    }
    pcBtn->AddChild(*pcText);
    return pcBtn;
}

CUIButton* CGameWindow::CreateButton(FLOAT fX, FLOAT fY, UINT uID, CUIWidget& cIdleImg, CUIWidget& cPressedImg, BOOLEAN bAutoAdd)
{
    CUIButton* pcBtn = new CUIButton();
    if (NULL == pcBtn) {
        ASSERT(FALSE);
        return NULL;
    }
    pcBtn->SetID(uID);
    pcBtn->SetDisplayWidgets(cIdleImg, cPressedImg);
    pcBtn->SetLocalPosition(fX, fY);
    m_cWidgetList.push_back(pcBtn);
    if (bAutoAdd) {
        AddChild(*pcBtn);
    }
    return pcBtn;
}

CUIButton* CGameWindow::CreateButton(FLOAT fX, FLOAT fY, UINT uID, CUIWidget& cIdleImg, CUIWidget& cPressedImg, const CHAR* szTextLabel, ELABEL_TYPE eLabelType, BOOLEAN bAutoAdd)
{
	CUITextLabel* pcText = CreateLabel(szTextLabel, eLabelType);
	if (NULL == pcText) {
		ASSERT(FALSE);
		return NULL;
	}
    const SHLVector2D& sLocalSize = cIdleImg.GetLocalSize();
    pcText->SetLocalSize(sLocalSize.x, sLocalSize.y);
	pcText->SetLocalPosition(sLocalSize.x * 0.5f, sLocalSize.y * 0.5f);
    CUIButton* pcBtn = CreateButton(fX, fY, uID, cIdleImg, cPressedImg, bAutoAdd);
	if (NULL == pcBtn) {
		ASSERT(FALSE);
		return NULL;
	}
	pcBtn->AddChild(*pcText);
	return pcBtn;
}
CUIButton* CGameWindow::CreateColorButton(FLOAT fX, FLOAT fY, FLOAT fWidth, FLOAT fHeight, UINT uID, UINT uColor, const CHAR* szTextLabel, BOOLEAN bAutoAdd)
{
    CUIImage* pcImg = CreateColorImage(uColor, fWidth, fHeight);
    if (NULL == pcImg) {
        ASSERT(FALSE);
        return NULL;
    }
    CUITextLabel * pcLabel = NULL;
    if (NULL != szTextLabel) {
        pcLabel = CreateLabel(szTextLabel, CGameWindow::ELABEL_SMALL);
    }
    CUIButton* pcButton = CreateButton(fX, fY, uID, *pcImg, *pcImg, bAutoAdd);
    if (NULL == pcButton) {
        ASSERT(FALSE);
        return FALSE;
    }
    pcButton->SetLocalSize(fWidth, fHeight);
    if (NULL != pcLabel) {
        pcLabel->SetLocalPosition(0.5f * fWidth, 0.5f * fHeight);
        pcLabel->SetLocalSize(fWidth, fHeight);
        pcButton->AddChild(*pcLabel);
    }
    return pcButton;
}

CUIButton* CGameWindow::CreateInvisibleButton(FLOAT fX, FLOAT fY, FLOAT fWidth, FLOAT fHeight, UINT uID, BOOLEAN bAutoAdd)
{
    CUIImage* pcImg = CreateColorImage(0, fWidth, fHeight);
    if (NULL == pcImg) {
        ASSERT(FALSE);
        return NULL;
    }
    pcImg->SetVisibility(FALSE);
    return CreateButton(fX, fY, uID, *pcImg, *pcImg, bAutoAdd);
}

CUITextLabel* CGameWindow::CreateLabel(const CHAR* szTextLabel, ELABEL_TYPE etype)
{
    CUITextLabel* pcText = NEW_TEXT_LABEL;
    if (NULL == pcText) {
        ASSERT(FALSE);
        return NULL;
    }
    m_cWidgetList.push_back(pcText);
    SetLabel(*pcText, szTextLabel, etype);
    return pcText;
}

CUIImage * CGameWindow::CreateImage(ETEX::ID eTex)
{
    CUIImage * pcImage = new CUIImage();
    if (NULL == pcImage) {
        return NULL;
    }
    m_cWidgetList.push_back(pcImage);
    pcImage->SetTexture(eTex);
    return pcImage;
}

CUIImage* CGameWindow::CreateColorImage(UINT uColor, FLOAT fWidth, FLOAT fHeight)
{
    CUIImage * pcImage = new CUIImage();
    if (NULL == pcImage) {
        return NULL;
    }
    m_cWidgetList.push_back(pcImage);
    pcImage->SetColor(uColor);
    pcImage->SetLocalSize(fWidth, fHeight);
    return pcImage;
}
CUIImage* CGameWindow::CreateColorImage(FLOAT fX, FLOAT fY, FLOAT fWidth, FLOAT fHeight, UINT uColor, BOOLEAN bAutoAdd)
{
    CUIImage* pcImg = CreateColorImage(uColor, fWidth, fHeight);
    if (NULL != pcImg) {
        pcImg->SetLocalPosition(fX, fY);
        if (bAutoAdd) {
            AddChild(*pcImg);
        }
    }
    return pcImg;
}

CUI3PartImage* CGameWindow::Create3PartImage(ETEX::ID eTex, BOOLEAN bHorizontal, FLOAT fSize)
{
    CUI3PartImage * pcImage = new CUI3PartImage;
    if (NULL == pcImage) {
        return NULL;
    }
    m_cWidgetList.push_back(pcImage);
    if (bHorizontal) {
        pcImage->Set3PartTexture(eTex);
        pcImage->SetLocalSize(fSize, pcImage->GetLocalSize().y);
    }
    else {
        pcImage->Set3PartVerticalTexture(eTex);
        pcImage->SetLocalSize(pcImage->GetLocalSize().x, fSize);
    }
    return pcImage;
}

CUI9PartImage* CGameWindow::Create9PartImage(ETEX::ID eTex, FLOAT fWidth, FLOAT fHeight)
{
    CUI9PartImage * pcImage = new CUI9PartImage;
    if (NULL == pcImage) {
        return NULL;
    }
    m_cWidgetList.push_back(pcImage);
    pcImage->Set9PartTexture(eTex);
    pcImage->SetLocalSize(fWidth, fHeight);
    return pcImage;
}

CUITextLabel* CGameWindow::SetDefaultWindow(FLOAT fWidth, FLOAT fHeight, const CHAR* szTitle)
{
    SetLocalSize(fWidth, fHeight);
    SetColorBackground(RGBA(0x1F, 0x1F, 0x1F, 0xFF));
    CUITextLabel* pcTitle = NULL;
    if (NULL != szTitle) {
        pcTitle = CreateLabel(szTitle, ELABEL_SMALL);
        if (NULL == pcTitle) {
            ASSERT(FALSE);
            return NULL;
        }
        pcTitle->SetLocalPosition(fWidth * 0.5f, 15.f);
        AddChild(*pcTitle);
    }
    return pcTitle;
}
CUIWidget* CGameWindow::CreateButtonWidget(EBTN_TYPE eType, BOOLEAN bIdle, FLOAT fOverrideWidth)
{
    const SBtnInfo& sInfo = s_asBtnInfo[eType];
    const ETEX::ID eTex = bIdle ? sInfo.eIdle : sInfo.ePressed;
    CUIWidget* pcWidget = NULL;
    if (sInfo.b3PartBtn) {
        CUI3PartImage* pc3Part = new CUI3PartImage();
        if (NULL != pc3Part) {
            pc3Part->Set3PartTexture(eTex);
            pcWidget = pc3Part;
        }
    }
    else {
        CUIImage* pcImg = new CUIImage();
        if (NULL != pcImg) {
            pcImg->SetTexture(eTex);
            pcWidget = pcImg;
        }
    }
    if (NULL == pcWidget) {
        ASSERT(FALSE);
        return NULL;
    }
    m_cWidgetList.push_back(pcWidget);
    const FLOAT fWidth = (0.0f == fOverrideWidth) ? sInfo.fWidth : fOverrideWidth;
    pcWidget->SetLocalSize(fWidth, sInfo.fHeight);
    return pcWidget;
}
