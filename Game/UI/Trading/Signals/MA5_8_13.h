#if !defined(CMA_5_8_13_H)
#define CMA_5_8_13_H

#include "UIContainer.h"
#include "UITextLabel.h"
#include <list>

typedef std::list<CUITextLabel*> CLabelList;

class CMA_5_8_13 : public CUIContainer
{
public:
    CMA_5_8_13();
    virtual ~CMA_5_8_13();

    VOID Setup(VOID);
    
    virtual VOID Release(VOID) OVERRIDE;
private:
    CLabelList    m_cList;
};
#endif // #if !defined(CMA_5_8_13_H)
