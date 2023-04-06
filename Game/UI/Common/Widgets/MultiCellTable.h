#if !defined(MULTI_CELL_TABLE_H)
#define MULTI_CELL_TABLE_H

#include "UIButton.h"
#include "UIContainer.h"
#include "UIImage.h"
#include "UIScroller.h"
#include "UITextLabel.h"

class CMultiCellTable : public CUIContainer
{
public:
    CMultiCellTable();
    virtual ~CMultiCellTable();

    VOID Initialize(const CHAR** pszHeaders, const FLOAT* pfCellWidth, UINT uCellNum, FLOAT fTableHeight, UINT uSelectedEventID);
    VOID ClearEntries(VOID);
    VOID AddRow(const CHAR** pszCells, UINT uCellNum, UINT uID);

    VOID SetSelected(UINT uID);
    
    virtual VOID Release(VOID) OVERRIDE;
    virtual VOID OnRender(FLOAT fOffsetX, FLOAT fOffsetY) const OVERRIDE;
private:
    FLOAT*          m_pfCellWidth;
    CUITextLabel**  m_ppcHeaders;
    CUIScroller     m_cScroller;
    UINT            m_uCellNum;
    UINT            m_uSelectedEventID;
    FLOAT           m_fNextCellY;
};

class CMultiCellRow : public CUIButton
{
public:
    CMultiCellRow(UINT uID);
    virtual ~CMultiCellRow();
    
    VOID Initialize(const CHAR** pszData, const FLOAT* pfCellWidth, UINT uCellNum, UINT uBtnEventID);
    VOID SetSelected(BOOLEAN bSelected);
    UINT GetDataID(VOID) const { return m_uID; }
    
    virtual VOID Release(VOID) OVERRIDE;
private:
    const UINT      m_uID;
    CUIImage        m_cBG;
    CUITextLabel**  m_ppcData;
    UINT            m_uCellNum;
    
    virtual BOOLEAN CreateButtonEvent(CEvent& cEvent) OVERRIDE;
};
#endif // #if !defined(MULTI_CELL_TABLE_H)
