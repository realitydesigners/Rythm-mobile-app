#include "stdafx.h"
#include "Sema3D.h"
#include "GameRenderer.h"

struct SBoxText {
    BOOLEAN bActive;
    CUITextLabel* pcLbl;
    FLOAT fX;
    FLOAT fY;
    FLOAT fW;
    FLOAT fH;
    UINT uColor;
};
static SBoxText s_asBoxes[MEGAZZ_MAX_DEPTH][2];
static SBoxText s_asFinalBox[MEGAZZ_MAX_DEPTH];

struct SExperiment {
    BOOLEAN bUp; // up or down frame
    BOOLEAN bCrossed60Percent;
    FLOAT fStartX;
    FLOAT fTop;
    FLOAT fBottom;
    FLOAT fRetraceX;
    FLOAT fRetraceY;
};

static SExperiment s_asRetraces[MEGAZZ_MAX_DEPTH];

static VOID AddBoxText(FLOAT fX, FLOAT fY, FLOAT fW, FLOAT fH, UINT uDepthIndex, BOOLEAN bUp, BOOLEAN bFinal)
{
    const UINT uColor = bUp ? RGBA(0, 0xFF, 0, 0xFF) : RGBA(0xFF, 0, 0, 0xFF);
    SBoxText* psBox;
    if (bFinal) {
        psBox = &s_asFinalBox[uDepthIndex];
    }
    else {
        psBox = &s_asBoxes[uDepthIndex][bUp ? 0 : 1];
    }
    SBoxText& sBoxTxt = *psBox;
    sBoxTxt.bActive = TRUE;
    sBoxTxt.fX = fX;
    sBoxTxt.fY = fY;
    sBoxTxt.fW = fW;
    sBoxTxt.fH = fH;
    sBoxTxt.uColor = uColor;
    if (NULL == sBoxTxt.pcLbl) {
        sBoxTxt.pcLbl = NEW_TEXT_LABEL;
        if (NULL == sBoxTxt.pcLbl) {
            ASSERT(FALSE);
            return;
        }
        sBoxTxt.pcLbl->SetFont(EGAMEFONT_SIZE_12);
    }
    FLOAT fTextX = fX + fW * 0.5f;
    if (10.0f > fTextX) {
        fTextX = 10.0f;
    }
    sBoxTxt.pcLbl->SetLocalPosition(fTextX, bUp ? (fY + fH) : fY);
    sBoxTxt.pcLbl->SetAnchor(0.5f, bUp ? 0.0f : 1.0f);
    CHAR szBuffer[32];
    snprintf(szBuffer, 32, "D%d", 9 - uDepthIndex);
    sBoxTxt.pcLbl->SetString(szBuffer);
    sBoxTxt.pcLbl->SetColor(uColor);
    sBoxTxt.pcLbl->OnUpdate(0.0f);
}
VOID CSema3D::InitExperiment(VOID)
{
    memset(s_asBoxes, 0, sizeof(s_asBoxes));
    memset(s_asFinalBox, 0, sizeof(s_asFinalBox));
}

VOID CSema3D::ReleaseExperiment(VOID)
{
    for (UINT uIndex = 0; MEGAZZ_MAX_DEPTH > uIndex; ++uIndex) {
        for (UINT uI = 0; 2 > uI; ++uI) {
            SBoxText& sBoxText = s_asBoxes[uIndex][uI];
            SAFE_RELEASE_DELETE(sBoxText.pcLbl);
        }
        SBoxText& sBoxText = s_asFinalBox[uIndex];
        SAFE_RELEASE_DELETE(sBoxText.pcLbl);
    }
}

VOID CSema3D::RenderBoxText(FLOAT fOffsetX, FLOAT fOffsetY) const
{
    CGameGL::SetLineWidth(2.0f);
    for (UINT uIndex = 0; MEGAZZ_MAX_DEPTH > uIndex; ++uIndex) {
        for (UINT uI = 0; 2 > uI; ++uI) {
            const SBoxText& sBox = s_asBoxes[uIndex][uI];
            if (sBox.bActive) {
                if (NULL != sBox.pcLbl) {
                    CGameRenderer::DrawRectOutline(fOffsetX + sBox.fX, fOffsetY + sBox.fY, sBox.fW, sBox.fH, sBox.uColor);
                    const SHLVector2D& sWorldPos = sBox.pcLbl->GetWorldPosition();
                    const SHLVector2D& sWorldSize = sBox.pcLbl->GetWorldSize();
                    CGameRenderer::DrawRectNoTex(fOffsetX + sWorldPos.x, fOffsetY + sWorldPos.y, sWorldSize.x, sWorldSize.y, RGBA(0, 0, 0, 0xFF));
                    sBox.pcLbl->OnRender(fOffsetX, fOffsetY);
                }
            }
        }
        const SBoxText& sBox = s_asFinalBox[uIndex];
        if (sBox.bActive) {
            if (NULL != sBox.pcLbl) {
                CGameRenderer::DrawRectOutline(fOffsetX + sBox.fX, fOffsetY + sBox.fY, sBox.fW, sBox.fH, sBox.uColor);
                const SHLVector2D& sWorldPos = sBox.pcLbl->GetWorldPosition();
                const SHLVector2D& sWorldSize = sBox.pcLbl->GetWorldSize();
                CGameRenderer::DrawRectNoTex(fOffsetX + sWorldPos.x, fOffsetY + sWorldPos.y, sWorldSize.x, sWorldSize.y, RGBA(0, 0, 0, 0xFF));
                sBox.pcLbl->OnRender(fOffsetX, fOffsetY);
            }
        }
    }
    CGameGL::SetLineWidth(1.0f);
}

VOID CSema3D::ClearExperiment(VOID)
{
    for (UINT uIndex = 0; MEGAZZ_MAX_DEPTH > uIndex; ++uIndex) {
        for (UINT uI = 0; 2 > uI; ++uI) {
            s_asBoxes[uIndex][uI].bActive = FALSE;
        }
        s_asFinalBox[uIndex].bActive = FALSE;
    }
}

VOID CSema3D::StartExperiment(const SChannelData& sChannel, FLOAT fNowX)
{
    for (UINT uIndex = 0; MEGAZZ_MAX_DEPTH > uIndex; ++uIndex) {
        SExperiment& sEx = s_asRetraces[uIndex];
        sEx.bUp = sChannel.abUp[uIndex];
        sEx.bCrossed60Percent = FALSE;
        sEx.fStartX = fNowX;
        sEx.fTop = sChannel.afTop[uIndex];
        sEx.fBottom = sChannel.afBottom[uIndex];
        sEx.fRetraceX = sEx.fStartX;
        sEx.fRetraceY = sEx.bUp ? sEx.fTop : sEx.fBottom;
    }
}

VOID CSema3D::ProcessExperiment(const SChannelData& sChannel, FLOAT fNowPriceX, FLOAT fNowPriceY)
{
    if (!m_bDrawRetraceBox) {
        return;
    }
    for (UINT uIndex = 0; (MEGAZZ_MAX_DEPTH-1) > uIndex; ++uIndex) {
        const FLOAT fChannelTop = sChannel.afTop[uIndex];
        SExperiment& sEx = s_asRetraces[uIndex];
        // if channel did not move
        if (sEx.fTop == fChannelTop) {
            const FLOAT fChannelSize = sEx.fTop - sEx.fBottom;
            const FLOAT f60Percent = fChannelSize * 0.6f;
            if (sEx.bUp) { // if was a up channel
                if (fNowPriceY < sEx.fRetraceY) {
                    sEx.fRetraceX = fNowPriceX;
                    sEx.fRetraceY = fNowPriceY;
                    sEx.bCrossed60Percent = (sEx.fTop - fNowPriceY) >= f60Percent;
                }
            }
            else {
                if (fNowPriceY > sEx.fRetraceY) {
                    sEx.fRetraceX = fNowPriceX;
                    sEx.fRetraceY = fNowPriceY;
                    sEx.bCrossed60Percent = (fNowPriceY - sEx.fBottom) >= f60Percent;
                }
            }
            continue;
        }
        // channel moved.
        if (sEx.bUp != sChannel.abUp[uIndex]) { // if channel changed direction
            sEx.bUp = sChannel.abUp[uIndex];
            sEx.bCrossed60Percent = FALSE;
            sEx.fStartX = fNowPriceX;
            sEx.fTop = sChannel.afTop[uIndex];
            sEx.fBottom = sChannel.afBottom[uIndex];
            sEx.fRetraceX = sEx.fStartX;
            sEx.fRetraceY = sEx.bUp ? sEx.fTop : sEx.fBottom;
            continue;
        }
        sEx.bUp = sChannel.abUp[uIndex];
        sEx.bCrossed60Percent = FALSE;
        sEx.fStartX = fNowPriceX;
        sEx.fTop = sChannel.afTop[uIndex];
        sEx.fBottom = sChannel.afBottom[uIndex];
        sEx.fRetraceX = sEx.fStartX;
        sEx.fRetraceY = sEx.bUp ? sEx.fTop : sEx.fBottom;
    }
}

VOID CSema3D::EndExperiment(FLOAT fEndX, FLOAT fNowPriceY, const SChannelData& sChannel)
{
    if (!m_bDrawRetraceBox) {
        return;
    }

    for (UINT uIndex = 0; (MEGAZZ_MAX_DEPTH-1) > uIndex; ++uIndex) {
        if (!m_abDepthEnabled[uIndex]) {
            continue;
        }
        SExperiment& sEx = s_asRetraces[uIndex];
        if (sEx.bCrossed60Percent) {
            BOOLEAN bNowCross50Percent;
            const FLOAT fChannelSize50Percent = (sEx.fTop + sEx.fBottom) * 0.5f;
            if (sEx.bUp) {
                bNowCross50Percent = fNowPriceY < fChannelSize50Percent;
            }
            else {
                bNowCross50Percent = fNowPriceY > fChannelSize50Percent;
            }
            if (!bNowCross50Percent) {
                const FLOAT fX = sChannel.afX[uIndex];
                const FLOAT fW = fEndX - fX;
                const FLOAT fH = sEx.fTop - sEx.fBottom;
                SHLVector2D sResult;
                Transform(sResult, fX, sEx.fTop, 0.0f);
                AddBoxText(sResult.x, sResult.y, fW, fH, uIndex, sEx.bUp, TRUE);
            }
        }
    }
}
