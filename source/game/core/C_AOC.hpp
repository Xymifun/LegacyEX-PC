#ifndef GAME_CORE_C_AOC_HPP
#define GAME_CORE_C_AOC_HPP

#include "platform/wiiu/C4JThreadImpl.hpp"

#include <vector>

struct AOCTitle
{
    unsigned long long ullTitleId;
    unsigned int uiGroupId;
    unsigned short usTitleVersion;
    char szTitleName[90];
};

class C_AOC
{
public:
    enum EAOCState
    {
        eAOCState_Start,
        eAOCState_Error,
        eAOCState_ListTitles,
        eAOCState_OpenTitle,
        eAOCState_ReadDirectory,
        eAOCState_GetPurchaseInfo,
        eAOCState_CloseTitle,
        eAOCState_Finalise
    };

    enum
    {
        MAX_AOC_TITLES = 100,
        MAX_AOC_CONTENT = 256
    };

    C_AOC();

    static C_AOC* getInstance();
    static int GetPurchasedContent_threadEntry(int iArgc, void* pArgv);

    void Init();
    void GetPurchasedContent();

    static C_AOC* s_pInstance;

    void* m_pWorkBuffer;
    unsigned int m_uiWorkBufferSize;
    EAOCState m_eState;
    unsigned int m_uiCurrentTitle;
    unsigned int m_uiTitleCount;
    AOCTitle* m_pTitleList;
    OSThread m_thread;
    char m_szMountPath[90];
    unsigned short m_contentIndices[MAX_AOC_CONTENT];
    int* m_pPurchased;
    std::vector<const char*> m_contentNames;
    int m_iNumContentNames;
};

#endif
