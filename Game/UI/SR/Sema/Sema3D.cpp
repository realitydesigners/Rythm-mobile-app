#include "stdafx.h"
#include "AppMain.h"
#include "CMain.h"
#include "ColorPickerWindow.h"
#include "Event.h"
#include "EventManager.h"
#include "PlayerData.h"
#include "Sema3D.h"
#include "ShiftedRenko.h"

CSema3D::CSema3D() :
CSRBaseGraph(EType_Sema3D, EGLOBAL_SEMA3D_CHART),
m_pcMegaZZ(NULL),
m_bInitialized(FALSE),
m_bMaximized(FALSE),
m_bDisplaySemaphore(FALSE),
m_bAllowFlatline(TRUE),
m_bFlattenMode(TRUE),
m_bDisplayHistory(TRUE),
m_bHideUI(FALSE),
m_bNewMethod(TRUE),
m_bDrawRetraceBox(FALSE),
INITIALIZE_TEXT_LABEL(m_cMaximizeTxt),
INITIALIZE_TEXT_LABEL(m_cSemaphoreTxt),
INITIALIZE_TEXT_LABEL(m_cFlatTxt),
INITIALIZE_TEXT_LABEL(m_cZZLineTxt),
INITIALIZE_TEXT_LABEL(m_cHistoryTxt),
INITIALIZE_TEXT_LABEL(m_cDisplayToggleTxt),
INITIALIZE_TEXT_LABEL(m_cRetraceToggleTxt),
INITIALIZE_TEXT_LABEL(m_cBGColorTxt),
m_uUsedPoly2D(0),
m_uUsedLineNum(0),
m_uUsedSemaNum(0),
m_bGameUITapped(FALSE),
m_fPrevX(0.0f),
m_fPrevY(0.0f),
m_fPixelPerPrice(0.0f),
m_fReferenceValue(0.0f),
m_fReferencePrice(0.0),
m_uUsedChannelZZ(0),
m_uBiggestChannelIndex(0),
m_fCullTopLeftX(0.0f),
m_fCullTopLeftY(0.0f),
m_fCullBottomRightX(0.0f),
m_fCullBottomRightY(0.0f)
{
    memset(m_abDepthEnabled, 0xFF, sizeof(m_abDepthEnabled));
    m_bAllowFlatline = FALSE;
    memset(m_apcDepthtxt, 0, sizeof(m_apcDepthtxt));
    memset(m_apcChannelTxt, 0, sizeof(m_apcChannelTxt));
#if !defined(REMOVE_COLOR_BTNS)
    memset(m_apcColortxt, 0, sizeof(m_apcColortxt));
#endif // #if !defined(REMOVE_COLOR_BTNS)
    memset(m_afChannelPixelSize, 0, sizeof(m_afChannelPixelSize));
    
    m_sScrollOffset.x = m_sScrollOffset.y = 0.0f;
    
    const BYTE abyGradient[MEGAZZ_MAX_DEPTH] = {
        22, 33, 44, 55, 66, 77, 88, 99, 240
    };
    for (UINT uBoxIndex = 0; MEGAZZ_MAX_DEPTH > uBoxIndex; ++uBoxIndex) {
        const BYTE byGradient = abyGradient[uBoxIndex];
        m_auSelectableDepthColors[uBoxIndex] = RGBA(byGradient, byGradient, byGradient, 0xFF);
        const BYTE byInv = 0x7F < byGradient ? 0 : 0xFF;
        m_auLineDepthColors[uBoxIndex] = RGBA(byInv, byInv, byInv, 0xFF);
    }
    m_cBG.SetColor(RGBA(0x4F, 0x4F, 0x4F, 0xFF));
    
    InitExperiment();
}

CSema3D::~CSema3D()
{

}
    
BOOLEAN CSema3D::Initialize(UINT uBoxNum)
{
    if (!CSRBaseGraph::Initialize(uBoxNum)) {
        return FALSE;
    }
    if (!m_bInitialized) {
        m_bInitialized = TRUE;
        m_sOriginalPos = GetLocalPosition();
        m_sOriginalSize = GetLocalSize();
    }
    AddSnapLatestBtn(EBTN_SNAP_LATEST);
    AddShowHideBtn(EBTN_TOGGLE_HIDE);
    
    m_cBtnBG.SetColor(RGBA(0x7F, 0x7F, 0x7F, 0x7F));
    m_cBtnBG.SetLocalSize(44.0f, 22.0f);
    SetupLabel(m_cMaximizeTxt);
    m_cMaximizeBtn.SetDisplayWidgets(m_cBtnBG, m_cBtnBG);
    m_cMaximizeBtn.SetID(EBTN_TOGGLE_MAXIMIZE);
    m_cMaximizeBtn.SetAnchor(1.0f, 0.0f);
    m_cMaximizeBtn.RemoveFromParent();
    AddChild(m_cMaximizeBtn);
    m_cMaximizeBtn.AddChild(m_cMaximizeTxt);
    
    SetupLabel(m_cSemaphoreTxt);
    m_cSemaphoreTxt.SetString("Sema");
    m_cSemaphoreBtn.SetDisplayWidgets(m_cBtnBG, m_cBtnBG);
    m_cSemaphoreBtn.SetID(EBTN_TOGGLE_SEMA);
    m_cSemaphoreBtn.SetLocalPosition(3.0 + 47.0f, 3.0f);
    m_cSemaphoreBtn.RemoveFromParent();
    AddChild(m_cSemaphoreBtn);
    m_cSemaphoreBtn.AddChild(m_cSemaphoreTxt);
    
    SetupLabel(m_cFlatTxt);
    m_cFlatTxt.SetString("Flat");
    m_cFlatBtn.SetDisplayWidgets(m_cBtnBG, m_cBtnBG);
    m_cFlatBtn.SetID(EBTN_TOGGLE_FLATTEN);
    m_cFlatBtn.SetLocalPosition(3.0 + 47.0f * 2.0f, 3.0f);
    m_cFlatBtn.RemoveFromParent();
    AddChild(m_cFlatBtn);
    m_cFlatBtn.AddChild(m_cFlatTxt);
    
    SetupLabel(m_cZZLineTxt);
    m_cZZLineTxt.SetString("ZLine");
    m_cZZLineBtn.SetDisplayWidgets(m_cBtnBG, m_cBtnBG);
    m_cZZLineBtn.SetID(EBTN_TOGGLE_ZZ_LINE);
    m_cZZLineBtn.SetLocalPosition(3.0 + 47.0f * 3.0f, 3.0f);
    m_cZZLineBtn.RemoveFromParent();
    AddChild(m_cZZLineBtn);
    m_cZZLineBtn.AddChild(m_cZZLineTxt);
    
    SetupLabel(m_cHistoryTxt);
    m_cHistoryTxt.SetString("Hist");
    m_cHistoryBtn.SetDisplayWidgets(m_cBtnBG, m_cBtnBG);
    m_cHistoryBtn.SetID(EBTN_TOGGLE_HISTORY);
    m_cHistoryBtn.SetLocalPosition(3.0 + 47.0f * 4.0f, 3.0f);
    m_cHistoryBtn.RemoveFromParent();
    AddChild(m_cHistoryBtn);
    m_cHistoryBtn.AddChild(m_cHistoryTxt);
    
    SetupLabel(m_cDisplayToggleTxt);
    m_cDisplayToggleTxt.SetString("Dia");
    m_cDisplayToggleBtn.SetDisplayWidgets(m_cBtnBG, m_cBtnBG);
    m_cDisplayToggleBtn.SetID(EBTN_TOGGLE_NEW_DISPLAY);
    m_cDisplayToggleBtn.SetLocalPosition(3.0 + 47.0f * 5.0f, 3.0f);
    m_cDisplayToggleBtn.RemoveFromParent();
    AddChild(m_cDisplayToggleBtn);
    m_cDisplayToggleBtn.AddChild(m_cDisplayToggleTxt);
    
    SetupLabel(m_cRetraceToggleTxt);
    m_cRetraceToggleTxt.SetString("V");
    m_cRetraceToggleBtn.SetDisplayWidgets(m_cBtnBG, m_cBtnBG);
    m_cRetraceToggleBtn.SetID(EBTN_TOGGLE_RETRACE);
    m_cRetraceToggleBtn.SetLocalPosition(3.0 + 47.0f * 6.0f, 3.0f);
    m_cRetraceToggleBtn.RemoveFromParent();
    AddChild(m_cRetraceToggleBtn);
    m_cRetraceToggleBtn.AddChild(m_cRetraceToggleTxt);
    
    SetupLabel(m_cBGColorTxt);
    m_cBGColorTxt.SetString("BG");
    m_cBGBtn.SetDisplayWidgets(m_cBtnBG, m_cBtnBG);
    m_cBGBtn.SetID(EBTN_CHANGE_BG_COLOR);
    m_cBGBtn.SetLocalPosition(3.0 + 47.0f * 7.0f, 3.0f);
    m_cBGBtn.RemoveFromParent();
    AddChild(m_cBGBtn);
    m_cBGBtn.AddChild(m_cBGColorTxt);
    
    CHAR szBuffer[64];
    for (UINT uIndex = 0; MEGAZZ_MAX_DEPTH > uIndex; ++uIndex) {
        CUIButton& cBtn = m_acDepthBtn[uIndex];
        if (NULL == m_apcDepthtxt[uIndex]) {
            CUITextLabel* pcLbl = NEW_TEXT_LABEL;
            if (NULL == pcLbl) {
                ASSERT(FALSE);
                return FALSE;
            }
            SetupLabel(*pcLbl);
            snprintf(szBuffer, 64, "D%d", MEGAZZ_MAX_DEPTH - uIndex);
            pcLbl->SetString(szBuffer);
            m_apcDepthtxt[uIndex] = pcLbl;
            cBtn.SetDisplayWidgets(m_cBtnBG, m_cBtnBG);
            cBtn.SetID(EBTN_DEPTH_09 + uIndex);
            cBtn.AddChild(*pcLbl);
        }
        cBtn.RemoveFromParent();
        AddChild(cBtn);
        
#if !defined(REMOVE_COLOR_BTNS)
        CUIButton& cColorBtn = m_acColorBtn[uIndex];
        if (NULL == m_apcColortxt[uIndex]) {
            CUITextLabel* pcLbl = NEW_TEXT_LABEL;
            if (NULL == pcLbl) {
                ASSERT(FALSE);
                return FALSE;
            }
            SetupLabel(*pcLbl);
            pcLbl->SetString("Col");
            m_apcColortxt[uIndex] = pcLbl;
            cColorBtn.SetDisplayWidgets(m_cBtnBG, m_cBtnBG);
            cColorBtn.SetID(EBTN_COLOR_09 + uIndex);
            cColorBtn.AddChild(*pcLbl);
        }
        cColorBtn.RemoveFromParent();
        AddChild(cColorBtn);
#endif // #if !defined(REMOVE_COLOR_BTNS)
    }
    
    for (UINT uIndex = 0; MEGAZZ_MAX_DEPTH > uIndex; ++uIndex) {
        if (NULL != m_apcChannelTxt[uIndex]) {
            continue;
        }
        CUITextLabel* pcLbl = NEW_TEXT_LABEL;
        if (NULL == pcLbl) {
            ASSERT(FALSE);
            return FALSE;
        }
        snprintf(szBuffer, 64 , "D%d", MEGAZZ_MAX_DEPTH - uIndex);
        pcLbl->SetString(szBuffer);
        pcLbl->AutoSize();
        pcLbl->SetAnchor(1.0f, 0.5f);
        pcLbl->SetFont(EGAMEFONT_SIZE_10);
        pcLbl->SetEffect(CUITextLabel::EFFECT_4SIDE_DIAG, 1.0f, 1.0f);
        pcLbl->OnUpdate(0.0f);
        m_apcChannelTxt[uIndex] = pcLbl;
    }
    UpdateToggleMaxMinBtn();
    UpdateDepthLabel();
    UpdateDepthBtnPos();
    UpdateCullingBox();
    UpdateColorBtnColors();
    UpdateShowHideStatus(m_bHideUI);
    
    const SHLVector2D& sSize = GetLocalSize();
    m_cBG.SetLocalSize(sSize.x, sSize.y);
    m_cBG.OnUpdate(0.0f);
    return TRUE;
}
    
VOID CSema3D::Release(VOID)
{    
    ReleaseExperiment();
    
    m_cMaximizeTxt.Release();
    m_cMaximizeTxt.RemoveFromParent();
    m_cMaximizeBtn.RemoveFromParent();
    
    m_cSemaphoreTxt.Release();
    m_cSemaphoreTxt.RemoveFromParent();
    m_cSemaphoreBtn.RemoveFromParent();
    
    m_cFlatTxt.Release();
    m_cFlatTxt.RemoveFromParent();
    m_cFlatBtn.RemoveFromParent();
    
    m_cZZLineTxt.Release();
    m_cZZLineTxt.RemoveFromParent();
    m_cZZLineBtn.RemoveFromParent();
    
    m_cHistoryTxt.Release();
    m_cHistoryTxt.RemoveFromParent();
    m_cHistoryBtn.RemoveFromParent();
    
    m_cDisplayToggleTxt.Release();
    m_cDisplayToggleTxt.RemoveFromParent();
    m_cDisplayToggleBtn.RemoveFromParent();
    
    m_cRetraceToggleTxt.Release();
    m_cRetraceToggleTxt.RemoveFromParent();
    m_cRetraceToggleBtn.RemoveFromParent();
    
    m_cBGColorTxt.Release();
    m_cBGColorTxt.RemoveFromParent();
    m_cBGBtn.RemoveFromParent();
    
    for (UINT uIndex = 0; MEGAZZ_MAX_DEPTH > uIndex; ++uIndex) {
        SAFE_REMOVE_RELEASE_DELETE(m_apcChannelTxt[uIndex]);
    }
    for (UINT uIndex = 0; MEGAZZ_MAX_DEPTH > uIndex; ++uIndex) {
        SAFE_REMOVE_RELEASE_DELETE(m_apcDepthtxt[uIndex]);
        m_acDepthBtn[uIndex].RemoveFromParent();
#if !defined(REMOVE_COLOR_BTNS)
        SAFE_REMOVE_RELEASE_DELETE(m_apcColortxt[uIndex]);
        m_acColorBtn[uIndex].RemoveFromParent();
#endif // #if !defined(REMOVE_COLOR_BTNS)
    }
    CSRBaseGraph::Release();
}

VOID CSema3D::ClearChart(VOID)
{
    ClearTimeLines();
    m_uUsedPoly2D = 0;
    m_uUsedLineNum = 0;
    m_uUsedSemaNum = 0;
    
    for (UINT uIndex = 0; MEGAZZ_MAX_DEPTH > uIndex; ++uIndex) {
        if (NULL != m_apcChannelTxt[uIndex]) {
            m_apcChannelTxt[uIndex]->SetVisibility(FALSE);
        }
    }
}

VOID CSema3D::PrepareChart(const CMegaZZ& cMegaZZ, UINT uDepth) // oldest in back
{
    m_pcMegaZZ = &cMegaZZ;
    GenerateZZ(cMegaZZ);
}

VOID CSema3D::OnUpdate(FLOAT fLapsed)
{
    CSRBaseGraph::OnUpdate(fLapsed);
}


BOOLEAN CSema3D::OnTouchBegin(FLOAT fX, FLOAT fY)
{
    m_bGameUITapped = CUIContainer::OnTouchBegin(fX, fY);
    if (m_bGameUITapped) {
        return TRUE;
    }
    const SHLVector2D sPos = { fX, fY };
    if (HitTest(sPos)) {
        m_fPrevX = fX;
        m_fPrevY = fY;
        return TRUE;
    }
    return FALSE;
}

BOOLEAN CSema3D::OnTouchMove(FLOAT fX, FLOAT fY)
{
    if (m_bGameUITapped) {
        return CUIContainer::OnTouchMove(fX, fY);
    }
    const FLOAT fDiffX = fX - m_fPrevX;
    m_fPrevX = fX;
    const FLOAT fDiffY = fY - m_fPrevY;
    m_fPrevY = fY;
    m_sScrollOffset.x -= fDiffX;
    m_sScrollOffset.y += fDiffY;
    UpdateCullingBox();
    GenerateRenderables();
    return TRUE;
}

BOOLEAN CSema3D::OnTouchEnd(FLOAT fX, FLOAT fY)
{
    if (m_bGameUITapped) {
        return CUIContainer::OnTouchEnd(fX, fY);
    }
    return TRUE;
}

VOID CSema3D::UpdateDepthLabel(VOID)
{
    for (UINT uIndex = 0; MEGAZZ_MAX_DEPTH > uIndex; ++uIndex) {
        if (NULL == m_apcDepthtxt[uIndex]) {
            continue;
        }
        m_apcDepthtxt[uIndex]->SetColor(m_abDepthEnabled[uIndex] ? 0xFFFFFFFF : RGBA(0x2F, 0x2F, 0x2F, 0xFF));
    }
}

VOID CSema3D::OnReceiveEvent(CEvent& cEvent)
{
    const UINT uEventID = cEvent.GetIntParam(0);
    switch (uEventID) {
        case EBTN_SNAP_LATEST:
            OnTapSnapLatest();
            break;
        case EBTN_TOGGLE_MAXIMIZE:
            OnTapToggleMaximizeBtn();
            break;
        case EBTN_TOGGLE_SEMA:
            OnTapToggleSemaBtn();
            break;
        case EBTN_TOGGLE_FLATTEN:
            OnTapFlattenBtn();
            break;
        case EBTN_TOGGLE_ZZ_LINE:
            OnTapToggleZZLineBtn();
            break;
        case EBTN_TOGGLE_HISTORY:
            OnTapToggleHistoryBtn();
            break;
        case EBTN_TOGGLE_HIDE:
            OnTapToggleHideBtn();
            break;
        case EBTN_TOGGLE_NEW_DISPLAY:
            OnTapToggleNewDisplayBtn();
            break;
        case EBTN_TOGGLE_RETRACE:
            OnTapRetraceBtn();
            break;
        case EBTN_DEPTH_09:
        case EBTN_DEPTH_08:
        case EBTN_DEPTH_07:
        case EBTN_DEPTH_06:
        case EBTN_DEPTH_05:
        case EBTN_DEPTH_04:
        case EBTN_DEPTH_03:
        case EBTN_DEPTH_02:
        case EBTN_DEPTH_01:
            OnTapDepthBtn(uEventID - EBTN_DEPTH_09);
            break;
        case EBTN_COLOR_09:
        case EBTN_COLOR_08:
        case EBTN_COLOR_07:
        case EBTN_COLOR_06:
        case EBTN_COLOR_05:
        case EBTN_COLOR_04:
        case EBTN_COLOR_03:
        case EBTN_COLOR_02:
        case EBTN_COLOR_01:
            OnTapColorBtn(uEventID - EBTN_COLOR_09);
            break;
        case EBTN_CHANGE_BG_COLOR:
            OnTapChangeBGColor();
            break;
        case EEVENT_CHANGE_BG_COLOR:
            OnChangedBGColor(cEvent.GetIntParam(1));
            break;
        case EEVENT_CHANGE_COLOR_09:
        case EEVENT_CHANGE_COLOR_08:
        case EEVENT_CHANGE_COLOR_07:
        case EEVENT_CHANGE_COLOR_06:
        case EEVENT_CHANGE_COLOR_05:
        case EEVENT_CHANGE_COLOR_04:
        case EEVENT_CHANGE_COLOR_03:
        case EEVENT_CHANGE_COLOR_02:
        case EEVENT_CHANGE_COLOR_01:
            OnChangedColor(uEventID - EEVENT_CHANGE_COLOR_09, cEvent.GetIntParam(1));
            break;
    }
}

VOID CSema3D::UpdateToggleMaxMinBtn(VOID)
{
    m_cMaximizeTxt.SetString(m_bMaximized ? "-" : "+");
    const FLOAT fX = GetLocalSize().x - 3.0f;
    m_cMaximizeBtn.SetLocalPosition(fX, 3.0f);
}

VOID CSema3D::OnTapToggleMaximizeBtn(VOID)
{
    m_bMaximized = !m_bMaximized;
    if (m_bMaximized) {
        const FLOAT fMaxW = CMain::GetScreenWidth() - 20.0f;
        const FLOAT fMaxH = CMain::GetScreenHeight() - 10.0f - m_sOriginalPos.y;
        SetLocalPosition(10.0f, m_sOriginalPos.y);
        SetLocalSize(fMaxW, fMaxH);
        CUIContainer* pcParent = GetParent();
        if (NULL != pcParent) {
            RemoveFromParent();
            pcParent->AddChild(*this);
        }
        m_cBG.SetLocalSize(fMaxW, fMaxH);
    }
    else {
        SetLocalPosition(m_sOriginalPos.x, m_sOriginalPos.y);
        SetLocalSize(m_sOriginalSize.x, m_sOriginalSize.y);
        m_cBG.SetLocalSize(m_sOriginalSize.x, m_sOriginalSize.y);
    }
    m_cBG.OnUpdate(0.0f);
    UpdateToggleMaxMinBtn();
    UpdateDepthBtnPos();
    UpdateCullingBox();
    ReGenerateZZ();
}

VOID CSema3D::UpdateDepthBtnPos(VOID)
{
    const SHLVector2D& sSize = GetLocalSize();
    FLOAT fBtnX = sSize.x - 47.0f * MEGAZZ_MAX_DEPTH - 3.0f;
    for (UINT uIndex = 0; MEGAZZ_MAX_DEPTH > uIndex; ++uIndex) {
        CUIButton& cBtn = m_acDepthBtn[uIndex];
        const FLOAT fX = fBtnX + 47.0f * uIndex;
        const FLOAT fY = sSize.y - 25.0f;
        cBtn.SetLocalPosition(fX, fY);
#if !defined(REMOVE_COLOR_BTNS)
        CUIButton& cColorBtn = m_acColorBtn[uIndex];
        cColorBtn.SetLocalPosition(fX, fY - 25.0f);
#endif // #if !defined(REMOVE_COLOR_BTNS)
    }
    SetIndicatorString("Sema3D");
}

VOID CSema3D::OnTapToggleSemaBtn(VOID)
{
    m_bDisplaySemaphore = !m_bDisplaySemaphore;
    GenerateRenderables();
}

VOID CSema3D::OnTapDepthBtn(UINT uIndex)
{
    ASSERT(MEGAZZ_MAX_DEPTH > uIndex);
    m_abDepthEnabled[uIndex] = !m_abDepthEnabled[uIndex];
    UpdateDepthLabel();
    ReGenerateZZ();
}

VOID CSema3D::ReGenerateZZ(VOID)
{
    if (NULL != m_pcMegaZZ) {
        GenerateZZ(*m_pcMegaZZ);
    }
}
VOID CSema3D::GenerateZZ(const CMegaZZ& cMegaZZ)
{
    ClearChart(); // clear for safety
    // calculate channel heights
    if (0 == cMegaZZ.GetFrameUsedNum()) { // make sure have data
        return;
    }
    UINT uBiggestDepthIndex = MEGAZZ_MAX_DEPTH;
    for (UINT uIndex = 0; MEGAZZ_MAX_DEPTH > uIndex; ++uIndex) {
        if (m_abDepthEnabled[uIndex]) {
            uBiggestDepthIndex = uIndex;
            break;
        }
    }
    if (MEGAZZ_MAX_DEPTH == uBiggestDepthIndex) { // none enabled?
        return;
    }
    m_uBiggestChannelIndex = uBiggestDepthIndex;
    const SHLVector2D& sSize = GetLocalSize();
    const FLOAT fMaxSize = sSize.y;
    const FLOAT fUsableSize = fMaxSize - 2.0f * CHART_PAD_PIXEL_NUM;
    const FLOAT fBiggestChannelSize = cMegaZZ.GetChannelSize(uBiggestDepthIndex);
    m_fPixelPerPrice = fUsableSize / (fBiggestChannelSize * 1.5f);
    // compute the channel heights
    for (UINT uIndex = 0; MEGAZZ_MAX_DEPTH > uIndex; ++uIndex) {
        const UINT uDepthIndex = uIndex;
        ASSERT(MEGAZZ_MAX_DEPTH > uDepthIndex);
        m_afChannelPixelSize[uIndex] = cMegaZZ.GetChannelSize(uDepthIndex) * m_fPixelPerPrice;
    }
    const MegaZZ::SChannel& sCurrentBiggest = cMegaZZ.GetCurrentChannel(uBiggestDepthIndex);
    m_fReferenceValue = 0.0f;
    m_fReferencePrice = (sCurrentBiggest.fTop + sCurrentBiggest.fBottom) * 0.5f;
    PrepareZZ(cMegaZZ);
}


VOID CSema3D::OnTapSnapLatest(VOID)
{
    m_sScrollOffset.x = m_sScrollOffset.y = 0.0f;
    UpdateCullingBox();
    GenerateRenderables();
}

VOID CSema3D::UpdateCullingBox(VOID)
{
    const SHLVector2D& sSize = GetLocalSize();
    const FLOAT fWidth = sSize.x * 0.5f;
    const FLOAT fHeight = sSize.y * 0.5f;
    m_fCullTopLeftX = m_sScrollOffset.x - fWidth;
    m_fCullTopLeftY = m_sScrollOffset.y + fHeight;
    m_fCullBottomRightX = m_sScrollOffset.x + fWidth;
    m_fCullBottomRightY = m_sScrollOffset.y - fHeight;
}

VOID CSema3D::OnTapFlattenBtn(VOID)
{
    m_bFlattenMode = !m_bFlattenMode;
    GenerateRenderables();
}

VOID CSema3D::OnTapToggleZZLineBtn(VOID)
{
    m_bAllowFlatline = !m_bAllowFlatline;
    GenerateRenderables();
}

VOID CSema3D::OnTapToggleHistoryBtn(VOID)
{
    m_bDisplayHistory = !m_bDisplayHistory;
    GenerateRenderables();
}

VOID CSema3D::UpdateColorBtnColors(VOID)
{
#if !defined(REMOVE_COLOR_BTNS)
    for (UINT uIndex = 0; MEGAZZ_MAX_DEPTH > uIndex; ++uIndex) {
        if (NULL != m_apcColortxt[uIndex]) {
            m_apcColortxt[uIndex]->SetColor(m_auSelectableDepthColors[uIndex]);
            m_apcColortxt[uIndex]->SetEffectColor(m_auLineDepthColors[uIndex]);
        }
    }
#endif // #if !defined(REMOVE_COLOR_BTNS)
}

VOID CSema3D::OnTapColorBtn(UINT uIndex)
{
    if (MEGAZZ_MAX_DEPTH <= uIndex) {
        ASSERT(FALSE);
        return;
    }
    CColorPickerWindow* pcWin = new CColorPickerWindow(EGLOBAL_SEMA3D_CHART, EEVENT_CHANGE_COLOR_09 + uIndex, m_auSelectableDepthColors[uIndex]);
    if (NULL != pcWin) {
        pcWin->SetGrayscaleMode();
        pcWin->DoModal();
    }
}

VOID CSema3D::OnChangedColor(UINT uIndex, UINT uColor)
{
    if (MEGAZZ_MAX_DEPTH <= uIndex) {
        ASSERT(FALSE);
        return;
    }
    m_auSelectableDepthColors[uIndex] = uColor;
    const BYTE byGradient = GET_RED(uColor);
    const BYTE byInv = 0x7F < byGradient ? 0 : 0xFF;
    m_auLineDepthColors[uIndex] = RGBA(byInv, byInv, byInv, 0xFF);
    UpdateColorBtnColors();
    GenerateRenderables();
}

VOID CSema3D::OnTapChangeBGColor(VOID)
{
    CColorPickerWindow* pcWin = new CColorPickerWindow(EGLOBAL_SEMA3D_CHART, EEVENT_CHANGE_BG_COLOR, m_cBG.GetColor());
    if (NULL != pcWin) {
        pcWin->SetGrayscaleMode();
        pcWin->DoModal();
    }
}
VOID CSema3D::OnChangedBGColor(UINT uColor)
{
    m_cBG.SetColor(uColor);
}

VOID CSema3D::OnTapToggleHideBtn(VOID)
{
    m_bHideUI = !m_bHideUI;
    UpdateShowHideStatus(m_bHideUI);
    for (UINT uIndex = 0; MEGAZZ_MAX_DEPTH > uIndex; ++uIndex) {
        m_acDepthBtn[uIndex].SetVisibility(!m_bHideUI);
#if !defined(REMOVE_COLOR_BTNS)
        m_acColorBtn[uIndex].SetVisibility(!m_bHideUI);
#endif // #if !defined(REMOVE_COLOR_BTNS)
    }
    m_cSemaphoreBtn.SetVisibility(!m_bHideUI);
    m_cFlatBtn.SetVisibility(!m_bHideUI);
    m_cZZLineBtn.SetVisibility(!m_bHideUI);
    m_cHistoryBtn.SetVisibility(!m_bHideUI);
    m_cDisplayToggleBtn.SetVisibility(!m_bHideUI);
    m_cRetraceToggleBtn.SetVisibility(!m_bHideUI);
    m_cBGBtn.SetVisibility(!m_bHideUI);
}

VOID CSema3D::OnTapToggleNewDisplayBtn(VOID)
{
    m_bNewMethod = !m_bNewMethod;
    GenerateRenderables();
}

VOID CSema3D::OnTapRetraceBtn(VOID)
{
    m_bDrawRetraceBox = !m_bDrawRetraceBox;
    GenerateRenderables();
}
