#if !defined(UI_EDITOR_H)
#define UI_EDITOR_H
#if defined(MAC_PORT)

#include "UIEditorDefs.h"
#include "GameWindow.h"
#include "UIContainer.h"
#include "UIImage.h"
#include "NibManager.h"
#include <list>

#define WIDGET_LINE_BORDER  (2.0f)
#define WIDGET_RESIZE_SIZE  (10.0f)
#define NEW_NIB_FILENAME    "screens/new_window.json"


namespace Json {
    class Value;
};

class CUIEditor : public CGameWindow
{
public:
    enum EBTN_ID {
        EBTN_CHANGE_PATH=0,
        EBTN_NEW_JSON,
        EBTN_NEW_JSON_CONFIRM,
        EBTN_LOAD_JSON,
        EBTN_LOAD_JSON_CONFIRM,
        EBTN_SAVE_JSON,
        EBTN_CHANGE_NAME,
        EBTN_TOGGLE_FIXED_SIZE,
        EBTN_CHANGE_WIDTH,
        EBTN_CHANGE_HEIGHT,
        EBTN_CHANGE_ASPECT,
        EBTN_TOGGLE_POSITION_FIXED_SIZE,
        EBTN_TOGGLE_POSITION_X_ALIGNMENT,
        EBTN_TOGGLE_POSITION_Y_ALIGNMENT,
        EBTN_CHANGE_X,
        EBTN_CHANGE_Y,
        EBTN_CHANGE_AX,
        EBTN_CHANGE_AY,
        EBTN_TOGGLE_TYPE,
        EBTN_CHANGE_RESOLUTION,
        EBTN_CONVERT_IMAGE_TO_BUTTON,
        EBTN_TOGGLE_INPUT,
        EBTN_CREATE_IMAGE,
        EBTN_CREATE_TEXT,
        EBTN_CREATE_CONTAINER,
        EBTN_MOVE_SELECTED_TO_FRONT,
        EBTN_ATTACH_DETACH_CONTAINER,
        EBTN_DUPLICATE,
        EBTN_SET_CLASSNAME,
        EBTN_OUTPUT_BINARY,
        EBTN_NO_OP,
        EEVENT_TEXT_INPUT_REPLY,
        
        EEVENT_REFRESH_SELECTED_WIDGET,
        EEVENT_SNAP_IMAGE_TO_TEXTURE_SIZE,
        EEVENT_LOAD_TEX_ID_FILE,        
    };
    CUIEditor();
    virtual ~CUIEditor();
    
    virtual VOID OnRender(FLOAT fOffsetX, FLOAT fOffsetY) const OVERRIDE;
    
    virtual BOOLEAN OnTouchBegin(FLOAT fX, FLOAT fY) OVERRIDE;
    virtual BOOLEAN OnTouchMove(FLOAT fX, FLOAT fY) OVERRIDE;
    virtual BOOLEAN OnTouchEnd(FLOAT fX, FLOAT fY) OVERRIDE;

    const CHAR* GetWorkingPath(VOID) const { return m_szUIFolder; }
    CUIWidget* GetSelectedWidget(VOID) const;
protected:
    virtual VOID InitializeInternals(VOID) OVERRIDE;
    virtual VOID Release(VOID) OVERRIDE;
private:
    typedef std::list<CUIWidget*> CSelectionList;
    CUITextLabel    m_cWorkingDirectory;
    CUIImage        m_cBorder;
    CUIContainer    m_cBaseContainer;
    BOOLEAN         m_bEdited;
    CHAR            m_szUIFolder[1024];
    CUIContainer*   m_pcBaseWidget;
    CSelectionList  m_cSelectedWidget;
    BOOLEAN         m_bPickingMode;
    BOOLEAN         m_bResizeMode;
    BOOLEAN         m_bAnchorMode;
    FLOAT           m_afLastPickPos[2];
    
    // editor variables
    std::string     m_cClassName;
    CUITextLabel*   m_pcWindowTitle;
    CUITextLabel*   m_pcWidgetName;
    
    CUITextLabel*   m_pcWidgetFixedSize;
    CUITextLabel*   m_pcWidgetSizeX;
    CUITextLabel*   m_pcWidgetSizeY;
    CUITextLabel*   m_pcWidgetAspectRatio;
    
    CUITextLabel*   m_pcPositionType;
    CUITextLabel*   m_pcPosXAlign;
    CUITextLabel*   m_pcPosYAlign;
    CUITextLabel*   m_pcWidgetPosX;
    CUITextLabel*   m_pcWidgetPosY;

    CUITextLabel*   m_pcWidgetAnchorX;
    CUITextLabel*   m_pcWidgetAnchorY;
    
    CUITextLabel*   m_pcWidgetType;
    
    CUITextLabel*   m_pcToggleInput;
    CUITextLabel*   m_pcResolution;
    CUITextLabel*   m_pcAttachDetach;
    UINT            m_uLastEventID;
    BOOLEAN         m_bInputEnabled;
    BOOLEAN         m_bAttachingMode;
    

    virtual VOID    OnReceiveEvent(CEvent& cEvent) OVERRIDE;
    
    static CWidgetEditNibMap& GetNibMap(VOID);
    VOID OnReceiveMacPath(BOOLEAN bSuccess);
    
    VOID OnNewUI(BOOLEAN bOverride);
    VOID OnLoadBtnPressed(BOOLEAN bOverride);
    VOID DoLoadNib(const CHAR* szFilePath);
    VOID DoSaveNib(const CHAR* szFilePath);
    
    BOOLEAN WriteWidgetToFile(CUIWidget& cWidget, FILE* phOut, UINT uTabNum, BOOLEAN bDrawFinalComma);
    BOOLEAN WriteWidgetWithNibToFile(CUIWidget& cWidget, const SNibEditInfo& sInfo, FILE* phOut, UINT uTabNum, BOOLEAN bDrawFinalComma);
    
    VOID UnloadNib(VOID);
    
    CUIWidget*  TryPick(FLOAT fX, FLOAT fY);
    BOOLEAN     CanDoResize(FLOAT fX, FLOAT fY);
    BOOLEAN     CanMoveAnchor(FLOAT fX, FLOAT fY);
    VOID        TryMoveSelectedWidget(FLOAT fX, FLOAT fY);
    VOID        UpdateWidgetFromNib(const SNibEditInfo& sNib, CUIWidget& cWidget);
    
    VOID        InitEditor(VOID);
    VOID        AdjResolution(INT nAdj);
    VOID        ReleaseEditor(VOID);
    VOID        RefreshLabels(VOID);
    VOID        RequireTextInput(UINT uEventID);
    VOID        OnTextInput(VOID);
    
    VOID        OnToggleSizeType(VOID);
    VOID        OnTogglePosType(VOID);
    VOID        OnTogglePositionAlignment(BOOLEAN bX);
    
    VOID        OnKeyPressed(UINT uKeyCode);
    VOID        SelectNextWidget(VOID);
    VOID        SelectPrevWidget(VOID);
    
    VOID        OnToggleType(VOID);
    VOID        RefreshSelectedWidget(VOID);
    VOID        SnapImageToTextureSize(VOID);
    VOID        ConvertImageToButton(VOID);
    VOID        CreateNewImage(VOID);
    VOID        CreateNewText(VOID);
    VOID        CreateNewContainer(VOID);
    VOID        MoveSelectedToFront(VOID);
    VOID        ToggleAttachDetach(VOID);
    VOID        DoAttachPick(CUIWidget* pcPicked);
    VOID        PerformAttach(CUIWidget& cWidget, SNibEditInfo& sNib, CUIContainer& cParent);
    
    CUIWidget*  DuplicateResursively(CUIWidget& cWidget, const SNibEditInfo& sNib, CUIContainer& cParent);
    VOID        PerformDuplicate(VOID);
    
    SNibEditInfo* FindNib(CUIWidget* pcWidget);
    VOID ToggleInput(BOOLEAN bEnabled);
    
    VOID RecursiveReleaseWidget(CUIWidget& cWidget);
    
    VOID OnOutputPathChosen(BOOLEAN bSuccess);
    VOID TrySaveBinary(VOID);
    VOID DoSaveBinary(VOID);
};

#endif // #if defined(MAC_PORT)
#endif // #if !defined(UI_EDITOR_H)
