#include "stdafx.h"
#if defined(MAC_PORT)
#include "UIEditor.h"
#include "EventManager.h"
#include "NibManager.h"
#include "MacSystem.h"
#include "AutoPtr.h"
#include <list>
VOID CUIEditor::TrySaveBinary(VOID)
{
    if (NULL == m_pcBaseWidget) {
        return;
    }
    if (0 == CNibManager::GetTexIDNum()) { // if tex id not yet loaded
        CHAR szBuffer[1024];
        snprintf(szBuffer, 1024, "%s/packed.h", m_szUIFolder);
        ;
        if (!CNibManager::LoadTextureIDFile(szBuffer)) {
            CHAR szError[1024];
            snprintf(szError, 1024, "unable to load %s", szBuffer);
            m_cWorkingDirectory.SetString(szError);
            return;
        }
    }
    DoSaveBinary();
}

VOID CUIEditor::OnReceiveMacPath(BOOLEAN bSuccess)
{
    const UINT uEventID = m_uLastEventID;
    m_uLastEventID = 0;
    if (!bSuccess) {
        return;
    }
    switch (uEventID) {
        case EBTN_CHANGE_PATH:
            strncpy(m_szUIFolder, CMacSystem::GetUserSelectedPath(), 1024);
            CNibManager::SetWorkingDirectory(m_szUIFolder);
            m_cWorkingDirectory.SetString(m_szUIFolder);
            break;
        case EBTN_LOAD_JSON:
            DoLoadNib(CMacSystem::GetUserSelectedPath());
            break;
        default:
            break;
    }
}
VOID CUIEditor::OnOutputPathChosen(BOOLEAN bSuccess)
{
    if (!bSuccess) {
        m_uLastEventID = 0;
        return;
    }
    switch (m_uLastEventID) {
        case EBTN_SAVE_JSON:
            DoSaveNib(CMacSystem::GetUserSelectedPath());
            break;
        default:
            break;
    }
    m_uLastEventID = 0;
}


VOID CUIEditor::DoSaveBinary(VOID)
{
    // commented out as we are now using binary conversion
//    if (NULL == m_pcBaseWidget) {
//        return;
//    }
//    if (0 == m_cClassName.length()) {
//        m_cWorkingDirectory.SetString("Classname not set! cannot save!!");
//        return;
//    }
//    CHAR szFilePath[1024];
//    snprintf(szFilePath, 1024, "%s/output", m_szUIFolder);
//    std::string cErr;
//    if (!CNibManager::WriteBinaryNib(*m_pcBaseWidget, GetNibMap(), cErr, szFilePath, m_cClassName.c_str())) {
//        m_cWorkingDirectory.SetString(cErr.c_str());
//        return;
//    }
}
#endif // #if defined(MAC_PORT)

