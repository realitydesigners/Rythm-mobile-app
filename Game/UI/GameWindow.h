#if !defined(GAME_WINDOW_H)
#define GAME_WINDOW_H

#include "UIWindow.h"
#include "UIImage.h"
#include "UI3PartImage.h"
#include "UI9PartImage.h"
#include "UIButton.h"
#include "UITextLabel.h"
#include "FontDefs.h"
#include <list>

class CUIWidget;
typedef std::list<CUIWidget*> CWidgetList;

class CGameWindow : public CUIWindow
{
public:
    enum EBTN_TYPE {
        EBTN_TYPE_DEBUG_BTN = 0,
        EBTN_TYPE_NUM,
    };
    enum ELABEL_TYPE {
        ELABEL_COMMON_BUTTON_LABEL=0,
        ELABEL_SMALL,
        ELABEL_TYPE_NUM
    };
    static VOID SetLabel(CUITextLabel& cLabel, const CHAR* szTextLabel, ELABEL_TYPE etype);

    CGameWindow(UINT uGlobalID = 0);
    virtual ~CGameWindow();
    
    VOID AutoDestroy(CUIWidget& cWidget);
    
    CUIButton*  CreateColorButton(FLOAT fX, FLOAT fY, FLOAT fWidth, FLOAT fHeight, UINT uID, UINT uColor, const CHAR* szTextLabel, BOOLEAN bAutoAdd = FALSE);
protected:
    virtual VOID InitializeInternals(VOID) OVERRIDE;
    virtual VOID Release(VOID) OVERRIDE;
    
    CUI9PartImage* SetCustom9PartBackground(ETEX::ID eID);
    CUIImage*      SetColorBackground(UINT uColor);
    CUIButton*  CreateButton(FLOAT fX, FLOAT fY, UINT uID, const CHAR* szTextLabel, EBTN_TYPE eType, FLOAT fOverrideWidth = 0.0f, BOOLEAN bAutoAdd = FALSE);
	CUIButton*  CreateButton(FLOAT fX, FLOAT fY, UINT uID, CUIWidget& cIdleImg, CUIWidget& cPressedImg, BOOLEAN bAutoAdd = FALSE);
	CUIButton*  CreateButton(FLOAT fX, FLOAT fY, UINT uID, CUIWidget& cIdleImg, CUIWidget& cPressedImg, const CHAR* szTextLabel, ELABEL_TYPE eLabelType, BOOLEAN bAutoAdd = FALSE);
    CUIButton*  CreateInvisibleButton(FLOAT fX, FLOAT fY, FLOAT fWidth, FLOAT fHeight, UINT uID, BOOLEAN bAutoAdd = FALSE);
    
    CUITextLabel*   CreateLabel(const CHAR* szTextLabel, ELABEL_TYPE etype);
    CUIImage*       CreateImage(ETEX::ID eTex);
    CUIImage*       CreateColorImage(UINT uColor, FLOAT fWidth, FLOAT fHeight);
    CUIImage*       CreateColorImage(FLOAT fX, FLOAT fY, FLOAT fWidth, FLOAT fHeight, UINT uColor, BOOLEAN bAutoAdd = FALSE);

    CUI3PartImage*  Create3PartImage(ETEX::ID eTex, BOOLEAN bHorizontal, FLOAT fSize);
    CUI9PartImage*  Create9PartImage(ETEX::ID eTex, FLOAT fWidth, FLOAT fHeight);
    
    CUITextLabel*   SetDefaultWindow(FLOAT fWidth, FLOAT fHeight, const CHAR* szTitle);
private:
    CWidgetList m_cWidgetList;

    CUIWidget* CreateButtonWidget(EBTN_TYPE eType, BOOLEAN bIdle, FLOAT fOverrideWidth = 0.0f);
};


#endif // #if !defined(GAME_WINDOW_H)
