#include "game/core/C_AOC.hpp"

#include "game/core/cDLCInfo.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "platform/wiiu/cNintendoDLCInfo.hpp"
#include "game/core/StringConversion.hpp"
#include "platform/wiiu/NintendoFileSystem_WiiU.hpp"
#include "platform/wiiu/WiiUFileSystemHelpers.hpp"

#include <string>
#include <stdlib.h>
#include <string.h>
#include <coreinit/memdefaultheap.h>

static const unsigned int AOC_THREAD_STACK_SIZE = 0x10000;
static const int AOC_THREAD_PRIORITY = 16;
static const unsigned short AOC_THREAD_ATTRIBUTES = 12;

int ReadDir(int* piHandle, FSDirEntry* pDirEntry);

extern "C"
{
int AOC_Initialize();
int AOC_Finalize();
unsigned int AOC_CalculateWorkBufferSize(unsigned int uiMaxTitles);
int AOC_ListTitle(unsigned int* puiTitleCount, AOCTitle* pTitleList, unsigned int uiMaxTitles, void* pWorkBuffer,
                  unsigned int uiWorkBufferSize);
int AOC_OpenTitle(char* pMountPath, AOCTitle* pTitle, void* pWorkBuffer, unsigned int uiWorkBufferSize);
int AOC_CloseTitle(AOCTitle* pTitle);
int AOC_GetPurchaseInfo(int* piPurchased, unsigned long long ullTitleId, unsigned short* pusContentIndices,
                        unsigned int uiContentCount, void* pWorkBuffer, unsigned int uiWorkBufferSize);
}

C_AOC* C_AOC::s_pInstance = new C_AOC();

C_AOC::C_AOC()
{
}

C_AOC* C_AOC::getInstance()
{
    return s_pInstance;
}

void C_AOC::Init()
{
    AOC_Initialize();

    m_uiWorkBufferSize = AOC_CalculateWorkBufferSize(MAX_AOC_TITLES);
    m_pWorkBuffer = MEMAllocFromDefaultHeapEx(m_uiWorkBufferSize, 64);

    m_eState = eAOCState_Start;
    m_uiTitleCount = 0;
    m_uiCurrentTitle = 0;
    m_iNumContentNames = 0;

    m_pTitleList = (AOCTitle*)MEMAllocFromDefaultHeap(sizeof(AOCTitle) * MAX_AOC_TITLES);

    char* pStack = (char*)MEMAllocFromDefaultHeap(AOC_THREAD_STACK_SIZE);

    bool bCreated = C4JThreadImpl::OSCreateThread4J(&m_thread, GetPurchasedContent_threadEntry, 0, this,
                                                    pStack + AOC_THREAD_STACK_SIZE, AOC_THREAD_STACK_SIZE,
                                                    AOC_THREAD_PRIORITY, AOC_THREAD_ATTRIBUTES);

    if (bCreated)
    {
        OSResumeThread(&m_thread);
    }
}

int C_AOC::GetPurchasedContent_threadEntry(int iArgc, void* pArgv)
{
    C_AOC* pAOC = (C_AOC*)pArgv;
    pAOC->GetPurchasedContent();
    return 0;
}

void C_AOC::GetPurchasedContent()
{
    while (true)
    {
        switch (m_eState)
        {
        case eAOCState_Start:
            m_eState = eAOCState_ListTitles;
            break;

        case eAOCState_Error:
            m_eState = eAOCState_Finalise;
            break;

        case eAOCState_ListTitles:
            if (AOC_ListTitle(&m_uiTitleCount, m_pTitleList, MAX_AOC_TITLES, m_pWorkBuffer, m_uiWorkBufferSize) == 0
                && m_uiTitleCount != 0)
            {
                m_eState = eAOCState_OpenTitle;
            }
            else
            {
                m_eState = eAOCState_Finalise;
            }
            break;

        case eAOCState_OpenTitle:
            if (AOC_OpenTitle(m_szMountPath, &m_pTitleList[m_uiCurrentTitle], m_pWorkBuffer, m_uiWorkBufferSize) != 0)
            {
                m_eState = eAOCState_Finalise;
            }
            else
            {
                unsigned int uiPathLength = strlen(m_szMountPath);

                if (m_szMountPath[uiPathLength - 1] != '/')
                {
                    m_szMountPath[uiPathLength] = '/';
                    m_szMountPath[uiPathLength + 1] = '\0';
                }

                m_eState = eAOCState_ReadDirectory;
            }
            break;

        case eAOCState_ReadDirectory:
        {
            int iDirHandle = -1;

            if (OpenDir(m_szMountPath, &iDirHandle) != 1)
            {
                m_eState = eAOCState_Error;
            }
            else
            {
                FSDirEntry dirEntry;

                while (ReadDir(&iDirHandle, &dirEntry) == 1 && (dirEntry.info.flags & FS_STAT_DIRECTORY) != 0)
                {
                    char* pContentName = new char[strlen(dirEntry.name)];
                    strcpy(pContentName, dirEntry.name);

                    m_contentNames.push_back(pContentName);
                    m_iNumContentNames++;
                }

                if (iDirHandle >= 0)
                {
                    CloseDir(&iDirHandle);
                }

                m_eState = eAOCState_GetPurchaseInfo;
            }
        }
        break;

        case eAOCState_GetPurchaseInfo:
        {
            int iContent;

            for (iContent = 0; iContent < m_iNumContentNames; iContent++)
            {
                const char* pContentName = m_contentNames[iContent];
                m_contentIndices[iContent] = (unsigned short)strtoull(pContentName, NULL, 16);
            }

            m_pPurchased = (int*)MEMAllocFromDefaultHeap(m_iNumContentNames * sizeof(int));

            if (AOC_GetPurchaseInfo(m_pPurchased, m_pTitleList[m_uiCurrentTitle].ullTitleId, m_contentIndices,
                                    m_iNumContentNames, m_pWorkBuffer, m_uiWorkBufferSize)
                == 0)
            {
                for (iContent = 0; iContent < m_iNumContentNames; iContent++)
                {
                    const char* pContentName = m_contentNames[iContent];
                    DLC_INFO* pDLCInfo = gApp.GetDLCInfoInstance()->GetDLCInfoByRomName(pContentName);

                    if (m_pPurchased[iContent] == 1 && pDLCInfo != NULL)
                    {
                        if (gApp.GetDLCInfoInstance()->SetDLCPurchased(pDLCInfo->CatalogIndex, true))
                        {
                            std::wstring packName = convStringToWstring(std::string(pDLCInfo->PackName));
                            gApp.HandleDLCLicenseChange(packName);
                        }
                    }
                }
            }

            m_iNumContentNames = 0;
            m_uiTitleCount = 0;
            m_uiCurrentTitle = 0;

            for (std::vector<const char*>::iterator it = m_contentNames.begin(); it != m_contentNames.end(); ++it)
            {
                delete *it;
            }

            m_contentNames.clear();

            m_eState = eAOCState_CloseTitle;
        }
        break;

        case eAOCState_CloseTitle:
        {
            int iResult = AOC_CloseTitle(&m_pTitleList[m_uiCurrentTitle]);

            m_uiCurrentTitle++;

            if (iResult == 0 && m_uiCurrentTitle < m_uiTitleCount)
            {
                m_eState = eAOCState_OpenTitle;
            }
            else
            {
                m_eState = eAOCState_Finalise;
            }
        }
        break;

        case eAOCState_Finalise:
            AOC_Finalize();
            return;
        }
    }
}
