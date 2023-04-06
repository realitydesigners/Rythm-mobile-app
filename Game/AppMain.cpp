#include "stdafx.h"
#include "AppMain.h"
#include "CMain.h"
#include "GameGL.h"
#include "GameRenderer.h"
#include "MessageData.h"
#include "MessageLog.h"
#include "NowLoadingWindow.h"
#include "TradeLog.h"
#include "SaveData.h"
#include "UIWindowManager.h"
#include <math.h>
#include "AutomationMgr.h"
#include "BinanceMgr.h"
#include "CryptoDotComMgr.h"
#include "OandaMgr.h"

#if defined(DEBUG)
#define DISPLAY_DEBUG_INFO
#endif // #if defined(DEBUG)

#if defined(DISPLAY_DEBUG_INFO)
#include "UITextLabel.h"
#include "FontRenderer.h"
static CUITextLabel*                s_pcDebugInfo   = NULL;
static BOOLEAN                      s_bDisplayInfo  = FALSE;
#endif // #if defined(DISPLAY_DEBUG_INFO)

#define SINE_WAVE_DURATION (1.5f)
static FLOAT                    s_fAngle        = 0.0f;
static FLOAT                    s_fSineValue    = 0.0f;
static FLOAT                    s_fGlobalLapsedTime = 0.0f;

#if defined(DEBUG)
VOID CAppMain::ToggleFPS(VOID)
{
#if defined(DISPLAY_DEBUG_INFO)
    s_bDisplayInfo = !s_bDisplayInfo;
#endif // #if defined(DISPLAY_DEBUG_INFO)
}
#endif // #if defined(DEBUG)

#include "TradeMainWindow.h"
#include "ForexWindow.h"
#if defined(MAC_PORT)
#include "UIEditor.h"
#endif // #if defined(MAC_PORT)

static_assert(sizeof(UINT64) == 8, "error");
VOID CAppMain::Start(VOID)
{

#if defined(MAC_PORT)
    if (1800 == CMain::GetScreenWidth() && 900 == CMain::GetScreenHeight()) {
        CUIEditor* pcEditor = new CUIEditor();
        pcEditor->DoModal();
        return;
    }
#endif // #if defined(MAC_PORT)
    if (!CSaveData::LoadData()) {
        TRACE("Create New Data!\n");
        CSaveData::CreateNewGame();
        CSaveData::SaveData();
    }
    
    // create the first window and DoModal
    CForexWindow* pcWin = new CForexWindow();
//    CTradingMainWindow* pcWin = new CTradingMainWindow();
    pcWin->DoModal();
}

BOOLEAN CAppMain::Initialize(VOID)
{
    CMain::SetEnableTouchEffect(TRUE, FALSE);
    // perform game level data structures initialization here
#if defined(DISPLAY_DEBUG_INFO)
    ASSERT(NULL == s_pcDebugInfo);
    s_pcDebugInfo = NEW_TEXT_LABEL;
    s_pcDebugInfo->SetFont(EGAMEFONT_SIZE_14);
    s_pcDebugInfo->AutoSize();
    s_pcDebugInfo->SetLocalPosition(CMain::GetDisplaySideMargin() + 10.0f, 10.0f);
#endif // #if defined(DISPLAY_DEBUG_INFO)
    CMessageData::OnChangeLanguage(EFONT_LANGUAGE_ENGLISH);
    CMessageLog::Initialize();
    CTradeLog::Initialize();
    CBinanceMgr::InitInstance();
    CCryptoDotComMgr::InitInstance();
    COandaMgr::InitInstance();
    CAutomationMgr::InitializeStatic();
    
    const FLOAT fGradient = 29.0f / 255.0f;
    const SHLVector3D sClearColor = { fGradient, fGradient, fGradient, 1.0f };
    CGameGL::SetGLClearColor(sClearColor);
    return TRUE;
}

VOID CAppMain::Release(VOID)
{
    if (CNowLoadingWindow::IsDisplayed()) {
        CNowLoadingWindow::HideWindow();
    }
    CUIWindowManager::Release();
    CTradeLog::Release();
    CMessageLog::Release();
    CMessageData::Release();
    
    // release game level data structures
    CAutomationMgr::ReleaseStatic();
    COandaMgr::ReleaseInstance();
    CCryptoDotComMgr::ReleaseInstance();
    CBinanceMgr::ReleaseInstance();
    
#if defined(DISPLAY_DEBUG_INFO)
    SAFE_RELEASE_DELETE(s_pcDebugInfo);
#endif // #if defined(DISPLAY_DEBUG_INFO)
}

VOID CAppMain::ReleaseGPUResources(VOID)
{
    // release game level opengl resources here
}

VOID CAppMain::RestoreGPUResources(VOID)
{
    // restore game level opengl resources here
}


VOID CAppMain::OnPause(VOID)
{
    CCryptoDotComMgr::GetInstance().OnPaused();
}

VOID CAppMain::OnResume(VOID)
{
    CCryptoDotComMgr::GetInstance().OnResume();
}

VOID CAppMain::OnStop(VOID)
{

}
#if defined(MAC_PORT)
VOID CAppMain::OnAppMinimized(VOID)
{
}
VOID CAppMain::OnAppDeMinimized(VOID)
{
}
#endif // #if defined(MAC_PORT)


VOID CAppMain::OnUpdate(FLOAT fLapsed)
{
    s_fGlobalLapsedTime = fLapsed;
    s_fAngle += fLapsed;
    s_fSineValue = sinf((44.0f/7.0f) * (s_fAngle/SINE_WAVE_DURATION));

    CMessageLog::Update(fLapsed);
    CBinanceMgr::GetInstance().Update(fLapsed);
    COandaMgr::GetInstance().Update(fLapsed);
#if defined(DISPLAY_DEBUG_INFO)
    if (NULL != s_pcDebugInfo && s_bDisplayInfo) {
        static FLOAT fAverageFPS = 0.0f;
        if ( fLapsed > 0.0f ) {
            fAverageFPS = (1.0f / fLapsed) * 0.1f + fAverageFPS * 0.9f;
        }
        CHAR szBuffer[128];
        snprintf(szBuffer,
                 128,
                 "FPS:%02d, TextLabel:%d\n%s\nWidgets:%d",
                 (UINT)fAverageFPS,
                 CUITextLabel::GetTotalTextInMem(),
                 CFontRenderer::GetDebugString(),
                 CUIWidget::GetAliveInstanceNum());
        s_pcDebugInfo->SetString(szBuffer);
        s_pcDebugInfo->OnUpdate(0.0f);
    }
#endif // #if defined(DISPLAY_DEBUG_INFO)
}

VOID CAppMain::OnRender(VOID)
{
#if defined(DISPLAY_DEBUG_INFO)
    if (NULL != s_pcDebugInfo && s_bDisplayInfo) {
        const SHLVector2D& sPos = s_pcDebugInfo->GetWorldPosition();
        const SHLVector2D& sSize = s_pcDebugInfo->GetWorldSize();
        CGameRenderer::DrawRectNoTex(sPos.x, sPos.y, sSize.x, sSize.y, RGBA(0x1F, 0x1F, 0x1F, 0x7F));
        s_pcDebugInfo->OnRender(0.0f, 0.0f);
    }
#endif // #if defined(DISPLAY_DEBUG_INFO)
}

FLOAT CAppMain::GetGlobalLapsedTime(VOID)
{
    return s_fGlobalLapsedTime;
}

FLOAT CAppMain::GetGlobalSine(VOID)
{
    return s_fSineValue;
}

FLOAT CAppMain::GetGlobalSineAdjusted(FLOAT fSpeed)
{
     return sinf((44.0f/7.0f) * (s_fAngle/SINE_WAVE_DURATION * fSpeed));
}
