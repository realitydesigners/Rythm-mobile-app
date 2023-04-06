#include "stdafx.h"
#include "ColorPicker.h"
#include "EventManager.h"

#define PICKER_W    (180.0f)
#define PICKER_H    (200.0f)
CColorPicker::CColorPicker() :
CUIContainer(),
m_uColor(0),
m_bGrayscaleMode(FALSE)
{
    memset(m_apcSlider, 0, sizeof(m_apcSlider));
}
    
CColorPicker::~CColorPicker()
{
    
}

VOID CColorPicker::Initialize(UINT uColor)
{
    m_uColor = uColor;
    
    SetLocalSize(PICKER_W, PICKER_H);
    m_cBG.SetLocalSize(PICKER_W, PICKER_H);
    m_cBG.SetColor(RGBA(0x4F, 0x4F, 0x4F, 0xFF));
    AddChild(m_cBG);
    
    m_cColor.SetAnchor(0.5f, 0.0f);
    m_cColor.SetLocalPosition(PICKER_W * 0.5f, 10.0f);
    m_cColor.SetLocalSize(80.0f, 50.f);
    m_cColor.SetColor(uColor);
    AddChild(m_cColor);
    
    const UINT auColors[3] = {
        RGBA(0xFF, 0, 0, 0xFF),
        RGBA(0, 0xFF, 0, 0xFF),
        RGBA(0, 0, 0xFF, 0xFF),
    };
    const UINT auColorsDark[3] = {
        RGBA(0x7F, 0, 0, 0xFF),
        RGBA(0, 0x7F, 0, 0xFF),
        RGBA(0, 0, 0x7F, 0xFF),
    };
    const BYTE abyValue[3] = {
        (BYTE)GET_RED(m_uColor),
        (BYTE)GET_GREEN(m_uColor),
        (BYTE)GET_BLUE(m_uColor)
    };
    for (UINT uIndex = 0; 3 > uIndex; ++uIndex) {
        m_acBar[uIndex].SetLocalPosition(5.0f, 80.0f + uIndex * 50.0f);
        m_acBar[uIndex].SetLocalSize(PICKER_W - 10.0f, 20.0f);
        m_acBar[uIndex].SetColor(auColorsDark[uIndex]);
        AddChild(m_acBar[uIndex]);
        
        m_apcSlider[uIndex] = new CSliderBar(0, EEVENT_RED_SLIDE + uIndex);
        if (NULL == m_apcSlider[uIndex]) {
            ASSERT(FALSE);
            return;
        }
        m_apcSlider[uIndex]->SetLocalSize(PICKER_W - 10.0f, 20.0f);
        m_apcSlider[uIndex]->SetLocalPosition(5.0f, 70.0f + uIndex * 50.0f);
        m_apcSlider[uIndex]->SetupBar(0, 0xFF, abyValue[uIndex], 40.0f, auColors[uIndex]);
        AddChild(*m_apcSlider[uIndex]);
    }
}
    
VOID CColorPicker::SetGrayscaleMode(VOID)
{
    m_bGrayscaleMode = TRUE;
}
VOID CColorPicker::Release(VOID)
{
    m_cBG.RemoveFromParent();
    m_cColor.RemoveFromParent();

    for (UINT uIndex = 0; 3 > uIndex; ++uIndex) {
        m_acBar[uIndex].RemoveFromParent();
        if (NULL != m_apcSlider[uIndex]) {
            m_apcSlider[uIndex]->Release();
            m_apcSlider[uIndex]->RemoveFromParent();
            delete m_apcSlider[uIndex];
            m_apcSlider[uIndex] = NULL;
        }
    }
    CUIContainer::Release();
}

VOID CColorPicker::PostEvent(CEvent& cEvent, FLOAT fDelay)
{
    CEventManager::PostEvent(*this, cEvent, fDelay);
}

VOID CColorPicker::OnReceiveEvent(CEvent& cEvent)
{
    BYTE byR = GET_RED(m_uColor);
    BYTE byG = GET_GREEN(m_uColor);
    BYTE byB = GET_BLUE(m_uColor);
    const UINT uEventID = cEvent.GetIntParam(0);
    switch (uEventID) {
        case EEVENT_RED_SLIDE:
            byR = cEvent.GetIntParam(1);
            if (m_bGrayscaleMode) {
                byG = byB = byR;
            }
            break;
        case EEVENT_GREEN_SLIDE:
            byG = cEvent.GetIntParam(1);
            if (m_bGrayscaleMode) {
                byR = byB = byG;
            }
            break;
        case EEVENT_BLUE_SLIDE:
            byB = cEvent.GetIntParam(1);
            if (m_bGrayscaleMode) {
                byR = byG = byB;
            }
            break;
            
        default:
            break;
    }
    m_uColor = RGBA(byR,byG,byB,0xFF);
    m_cColor.SetColor(m_uColor);
    if (m_bGrayscaleMode) {
        for (UINT uIndex = 0; 3 > uIndex; ++uIndex) {
            if (NULL != m_apcSlider[uIndex]) {
                m_apcSlider[uIndex]->SetupBar(0, 0xFF, byR, 40.0f, m_uColor);
            }
        }
    }
}
