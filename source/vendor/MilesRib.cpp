#include "vendor/MilesRib.hpp"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "vendor/MilesMem.hpp"

extern "C" int AIL_stricmp(const char* pFirst, const char* pSecond);

struct INTERFACE_DESCRIPTOR
{
    unsigned int uProvider;
    const char* pInterfaceName;
    const char* pEntryName;
    unsigned int uType;
    unsigned int uSubType;
    void* pToken;
};

struct ILIST_ENTRY
{
    ILIST_ENTRY* pNext;
    ILIST_ENTRY* pPrevious;
    ILIST_ENTRY* pSetNext;
    ILIST_ENTRY* pSetPrevious;
    unsigned int uSet;
    int iIndex;
    INTERFACE_DESCRIPTOR descriptor;
};

struct PVLIST_ENTRY
{
    PVLIST_ENTRY* pNext;
    PVLIST_ENTRY* pPrevious;
    int iIndex;
    void* pUserData;
    RIB_MAIN_CALLBACK pMain;
    char szName[512];
    int iReferenceCount;
    unsigned int uField218;
    unsigned int uField21C;
    unsigned int uField220;
    unsigned int uField224;
    unsigned int uField228;
    unsigned int uField22C;
    unsigned int uField230;
    unsigned int uField234;
    unsigned int uField238;
    unsigned int uField23C;
    unsigned int uField240;
    unsigned int uField244;
    unsigned int uField248;
    unsigned int uField24C;
    unsigned int uField250;
    unsigned int uField254;
};

static const int RIB_INTERFACE_SET_COUNT = 512;
static const int RIB_INTERFACE_ENTRY_COUNT = 400;
static const int RIB_INTERFACE_GROW_COUNT = 256;

static const int RIB_PROVIDER_COUNT = 40;
static const int RIB_PROVIDER_GROW_COUNT = 16;

struct InterfaceList
{
    int iCount;
    ILIST_ENTRY* pUsed;
    ILIST_ENTRY* pFree;
    ILIST_ENTRY* pSet[RIB_INTERFACE_SET_COUNT];
    ILIST_ENTRY* pBase;
    ILIST_ENTRY entries[RIB_INTERFACE_ENTRY_COUNT];
};

struct ProviderList
{
    int iCount;
    PVLIST_ENTRY* pUsed;
    PVLIST_ENTRY* pFree;
    PVLIST_ENTRY* pBase;
    PVLIST_ENTRY entries[RIB_PROVIDER_COUNT];
};

static const int RIB_NOERR = 0;
static const int RIB_ERROR = 1;
static const int RIB_NOT_FOUND = 2;

static const unsigned int RIB_ENTRY_NOT_FOUND = 0xFFFFFFFF;

static const unsigned int RIB_FUNCTION = 0;
static const unsigned int RIB_PROPERTY = 1;

#if __SIZEOF_POINTER__ == 8
static void StoreToken(void* pDestination, unsigned int uType, void* pToken)
{
    if (uType == RIB_FUNCTION)
    {
        *(void**)pDestination = pToken;
    }
    else
    {
        *(unsigned int*)pDestination = (unsigned int)(uintptr_t)pToken;
    }
}
#endif

static const unsigned int RIB_INVALID_PROVIDER = 0xFFFFFFFF;

static const unsigned int RIB_ENUMERATE_FIRST = 0xFFFFFFFF;

static const int RIB_MINIMUM_MAJOR_VERSION = 1;
static const int RIB_MINIMUM_MINOR_VERSION = 3;

static const char MILES_RIB_FILE[] = "C:\\devel\\projects\\mss\\src\\sdk\\shared\\rib.cpp";

static const char RIB_PROVIDER_PROPERTY_ENTRY[] = "PROVIDER_property";

static const char RIB_OBSOLETE_MESSAGE[] = "Attempt to load obsolete (pre-V1.30) RIB";
static const char RIB_MISSING_ENTRIES_MESSAGE[] = "One or more entries not found";
static const char RIB_MISSING_REQUESTED_MESSAGE[] = "One or more requested entries not found";

static int RIB_num_users = 0;

static InterfaceList RIB_interfaces;
static ProviderList RIB_providers;

static ILIST_ENTRY* RIB_last_entry;

static char RIB_error_text[256];

static unsigned int ILE_set(INTERFACE_DESCRIPTOR* pDescriptor)
{
    unsigned int uHash;
    const unsigned char* pName;

    pName = (const unsigned char*)pDescriptor->pEntryName;
    uHash = 0;

    while (*pName != 0)
    {
        uHash = uHash + *pName;
        pName = pName + 1;
    }

    return uHash & (RIB_INTERFACE_SET_COUNT - 1);
}

static ILIST_ENTRY* IL_alloc(InterfaceList* pList)
{
    ILIST_ENTRY* pEntry;
    ILIST_ENTRY* pOldBase;
    ILIST_ENTRY* pNewBase;
    int iOldCount;
    int iNewCount;
    int iDelta;
    int i;

    pEntry = pList->pFree;

    if (pEntry == NULL)
    {
        iOldCount = pList->iCount;
        iNewCount = iOldCount + RIB_INTERFACE_GROW_COUNT;
        pOldBase = pList->pBase;

        pNewBase = (ILIST_ENTRY*)AIL_mem_alloc_lock_info(iNewCount * (int)sizeof(ILIST_ENTRY), MILES_RIB_FILE, 354);
        if (pNewBase == NULL)
        {
            return NULL;
        }

        iDelta = (int)((char*)pNewBase - (char*)pOldBase);

        for (i = 0; i < iOldCount; i++)
        {
            memcpy(&pNewBase[i], &pOldBase[i], sizeof(ILIST_ENTRY));

            if (pNewBase[i].pSetNext != NULL)
            {
                pNewBase[i].pSetNext = (ILIST_ENTRY*)((char*)pNewBase[i].pSetNext + iDelta);
            }
            if (pNewBase[i].pSetPrevious != NULL)
            {
                pNewBase[i].pSetPrevious = (ILIST_ENTRY*)((char*)pNewBase[i].pSetPrevious + iDelta);
            }
            if (pNewBase[i].pNext != NULL)
            {
                pNewBase[i].pNext = (ILIST_ENTRY*)((char*)pNewBase[i].pNext + iDelta);
            }
            if (pNewBase[i].pPrevious != NULL)
            {
                pNewBase[i].pPrevious = (ILIST_ENTRY*)((char*)pNewBase[i].pPrevious + iDelta);
            }
        }

        for (i = 0; i < RIB_INTERFACE_SET_COUNT; i++)
        {
            if (pList->pSet[i] != NULL)
            {
                pList->pSet[i] = (ILIST_ENTRY*)((char*)pList->pSet[i] + iDelta);
            }
        }

        for (i = iOldCount; i < iNewCount; i++)
        {
            pNewBase[i].pNext = &pNewBase[i] + 1;
            pNewBase[i].pPrevious = &pNewBase[i] - 1;
            pNewBase[i].iIndex = -1;
        }

        pList->pFree = &pNewBase[iOldCount];
        pList->pFree->pPrevious = NULL;
        pNewBase[iNewCount - 1].pNext = NULL;

        pList->pUsed = (ILIST_ENTRY*)((char*)pList->pUsed + iDelta);
        pList->iCount = iNewCount;

        if (pList->pBase != pList->entries)
        {
            AIL_mem_free_lock(pList->pBase);
        }

        pEntry = pList->pFree;
        pList->pBase = pNewBase;
    }

    pList->pFree = pEntry->pNext;
    pEntry->pNext = pList->pUsed;
    pEntry->pPrevious = NULL;

    if (pList->pUsed != NULL)
    {
        pList->pUsed->pPrevious = pEntry;
    }

    pList->pUsed = pEntry;
    pEntry->iIndex = (int)(pEntry - pList->pBase);

    return pEntry;
}

static void IL_dealloc(InterfaceList* pList, ILIST_ENTRY* pEntry)
{
    if (pEntry->pSetNext != NULL)
    {
        pEntry->pSetNext->pSetPrevious = pEntry->pSetPrevious;
    }

    if (pEntry->pSetPrevious != NULL)
    {
        pEntry->pSetPrevious->pSetNext = pEntry->pSetNext;
    }

    pEntry->iIndex = -1;

    if (pEntry->pNext != NULL)
    {
        pEntry->pNext->pPrevious = pEntry->pPrevious;
    }

    if (pEntry->pPrevious != NULL)
    {
        pEntry->pPrevious->pNext = pEntry->pNext;
    }

    if (pList->pUsed == pEntry)
    {
        pList->pUsed = pEntry->pNext;
    }

    if (pList->pSet[pEntry->uSet] == pEntry)
    {
        pList->pSet[pEntry->uSet] = pEntry->pSetNext;
    }

    ILIST_ENTRY* pFree = pList->pFree;
    pEntry->pNext = pFree;
    pEntry->pPrevious = NULL;

    if (pList->pFree != NULL)
    {
        pList->pFree->pPrevious = pEntry;
    }

    pList->pFree = pEntry;
}

static ILIST_ENTRY* IL_lookup(InterfaceList* pList, INTERFACE_DESCRIPTOR* pDescriptor)
{
    ILIST_ENTRY* pEntry;
    int iMatch;

    pEntry = pList->pSet[ILE_set(pDescriptor)];

    while (pEntry != NULL)
    {
        if (*pDescriptor->pInterfaceName == '\0')
        {
            iMatch = pDescriptor->uType == pEntry->descriptor.uType
                     && pDescriptor->uProvider == pEntry->descriptor.uProvider
                     && strcmp(pDescriptor->pEntryName, pEntry->descriptor.pEntryName) == 0;
        }
        else
        {
            iMatch = pDescriptor->uType == pEntry->descriptor.uType
                     && pDescriptor->uProvider == pEntry->descriptor.uProvider
                     && strcmp(pDescriptor->pInterfaceName, pEntry->descriptor.pInterfaceName) == 0
                     && strcmp(pDescriptor->pEntryName, pEntry->descriptor.pEntryName) == 0;
        }

        if (iMatch)
        {
            return pEntry;
        }

        pEntry = pEntry->pSetNext;
    }

    return NULL;
}

static void PL_dealloc(ProviderList* pList, PVLIST_ENTRY* pEntry)
{
    pEntry->iIndex = -1;

    if (pEntry->pNext != NULL)
    {
        pEntry->pNext->pPrevious = pEntry->pPrevious;
    }

    if (pEntry->pPrevious != NULL)
    {
        pEntry->pPrevious->pNext = pEntry->pNext;
    }

    if (pList->pUsed == pEntry)
    {
        pList->pUsed = pEntry->pNext;
    }

    pEntry->pNext = pList->pFree;
    pEntry->pPrevious = NULL;

    if (pList->pFree != NULL)
    {
        pList->pFree->pPrevious = pEntry;
    }

    pList->pFree = pEntry;
}

static PVLIST_ENTRY* PL_alloc(ProviderList* pList)
{
    PVLIST_ENTRY* pEntry;
    PVLIST_ENTRY* pOldBase;
    PVLIST_ENTRY* pNewBase;
    int iOldCount;
    int iNewCount;
    int iDelta;
    int i;

    pEntry = pList->pFree;

    if (pEntry == NULL)
    {
        iOldCount = pList->iCount;
        iNewCount = iOldCount + RIB_PROVIDER_GROW_COUNT;
        pOldBase = pList->pBase;

        pNewBase = (PVLIST_ENTRY*)AIL_mem_alloc_lock_info(iNewCount * (int)sizeof(PVLIST_ENTRY), MILES_RIB_FILE, 523);
        if (pNewBase == NULL)
        {
            return NULL;
        }

        iDelta = (int)((char*)pNewBase - (char*)pOldBase);

        for (i = 0; i < iOldCount; i++)
        {
            memcpy(&pNewBase[i], &pOldBase[i], sizeof(PVLIST_ENTRY));

            if (pNewBase[i].pNext != NULL)
            {
                pNewBase[i].pNext = (PVLIST_ENTRY*)((char*)pNewBase[i].pNext + iDelta);
            }

            if (pNewBase[i].pPrevious != NULL)
            {
                pNewBase[i].pPrevious = (PVLIST_ENTRY*)((char*)pNewBase[i].pPrevious + iDelta);
            }
        }

        for (i = iOldCount; i < iNewCount; i++)
        {
            pNewBase[i].pNext = &pNewBase[i] + 1;
            pNewBase[i].pPrevious = &pNewBase[i] - 1;
            pNewBase[i].iIndex = -1;
        }

        pNewBase[iOldCount].pPrevious = NULL;
        pNewBase[iNewCount - 1].pNext = NULL;

        pList->pFree = &pNewBase[iOldCount];
        pList->iCount = iNewCount;
        pList->pUsed = (PVLIST_ENTRY*)((char*)pList->pUsed + iDelta);

        if (pList->pBase != pList->entries)
        {
            AIL_mem_free_lock(pList->pBase);
        }

        pEntry = pList->pFree;
        pList->pBase = pNewBase;
    }

    pList->pFree = pEntry->pNext;
    pEntry->pNext = pList->pUsed;
    pEntry->pPrevious = NULL;

    if (pList->pUsed != NULL)
    {
        pList->pUsed->pPrevious = pEntry;
    }

    pList->pUsed = pEntry;
    pEntry->iIndex = (int)(pEntry - pList->pBase);

    return pEntry;
}

extern "C" unsigned int RIB_alloc_provider_handle(void* pUserData)
{
    PVLIST_ENTRY* pProvider;
    int i;

    if (RIB_num_users++ == 0)
    {
        RIB_interfaces.iCount = RIB_INTERFACE_ENTRY_COUNT;
        RIB_interfaces.pBase = RIB_interfaces.entries;

        for (i = 0; i < RIB_interfaces.iCount; i++)
        {
            RIB_interfaces.pBase[i].pPrevious = &RIB_interfaces.pBase[i] - 1;
            RIB_interfaces.pBase[i].pNext = &RIB_interfaces.pBase[i] + 1;
            RIB_interfaces.pBase[i].iIndex = -1;
        }

        RIB_interfaces.pBase[0].pPrevious = NULL;
        RIB_interfaces.pBase[RIB_interfaces.iCount - 1].pNext = NULL;
        RIB_interfaces.pUsed = NULL;
        RIB_interfaces.pFree = RIB_interfaces.pBase;

        for (i = 0; i < RIB_INTERFACE_SET_COUNT; i++)
        {
            RIB_interfaces.pSet[i] = NULL;
        }

        RIB_providers.iCount = RIB_PROVIDER_COUNT;
        RIB_providers.pBase = RIB_providers.entries;

        for (i = 0; i < RIB_providers.iCount; i++)
        {
            RIB_providers.pBase[i].pPrevious = &RIB_providers.pBase[i] - 1;
            RIB_providers.pBase[i].pNext = &RIB_providers.pBase[i] + 1;
            RIB_providers.pBase[i].iIndex = -1;
        }

        RIB_providers.pBase[0].pPrevious = NULL;
        RIB_providers.pBase[RIB_providers.iCount - 1].pNext = NULL;
        RIB_providers.pUsed = NULL;
        RIB_providers.pFree = RIB_providers.pBase;

        PL_alloc(&RIB_providers);
    }

    pProvider = PL_alloc(&RIB_providers);
    pProvider->pUserData = pUserData;
    pProvider->pMain = NULL;
    pProvider->iReferenceCount = 0;
    pProvider->szName[0] = '\0';

    return (unsigned int)pProvider->iIndex;
}

extern "C" void RIB_free_provider_handle(unsigned int uProvider)
{
    if (uProvider != RIB_INVALID_PROVIDER)
    {
        PVLIST_ENTRY* pEntry = &RIB_providers.pBase[uProvider];

        if (pEntry->iIndex != -1)
        {
            PL_dealloc(&RIB_providers, pEntry);
        }
    }

    if (RIB_num_users != 0 && --RIB_num_users == 0)
    {
        if (RIB_interfaces.pBase != NULL)
        {
            while (RIB_interfaces.pUsed != NULL)
            {
                IL_dealloc(&RIB_interfaces, RIB_interfaces.pUsed);
            }

            RIB_interfaces.iCount = 0;

            if (RIB_interfaces.pBase != RIB_interfaces.entries)
            {
                AIL_mem_free_lock(RIB_interfaces.pBase);
            }

            RIB_interfaces.pBase = NULL;
        }

        if (RIB_providers.pBase != NULL)
        {
            while (RIB_providers.pUsed != NULL)
            {
                PL_dealloc(&RIB_providers, RIB_providers.pUsed);
            }

            RIB_providers.iCount = 0;

            if (RIB_providers.pBase != RIB_providers.entries)
            {
                AIL_mem_free_lock(RIB_providers.pBase);
            }

            RIB_providers.pBase = NULL;
        }
    }
}

static ILIST_ENTRY* IL_find_provider(unsigned int uProvider)
{
    int i;

    for (i = 0; i < RIB_interfaces.iCount; i++)
    {
        if (RIB_interfaces.pBase[i].iIndex != -1 && uProvider == RIB_interfaces.pBase[i].descriptor.uProvider)
        {
            return &RIB_interfaces.pBase[i];
        }
    }

    return NULL;
}

extern "C" int RIB_register_interface(unsigned int uProvider, const char* pInterfaceName, int iEntryCount,
                                      const RIB_INTERFACE_ENTRY* pEntries)
{
    INTERFACE_DESCRIPTOR descriptor;
    ILIST_ENTRY* pRegistered;
    ILIST_ENTRY* pEntry;
    RIB_PROVIDER_PROPERTY_CALLBACK pProperty;
    unsigned int uVersion;
    int iMajor;
    int iMinor;
    unsigned int uSet;
    int i;

    pRegistered = IL_find_provider(uProvider);

    if (pRegistered == NULL)
    {
        uVersion = 0;

        if (pEntries->uType != 0 || strcmp(pEntries->pEntryName, RIB_PROVIDER_PROPERTY_ENTRY) != 0)
        {
            AIL_set_error(RIB_OBSOLETE_MESSAGE);
            return RIB_NOT_FOUND;
        }

        pProperty = (RIB_PROVIDER_PROPERTY_CALLBACK)pEntries->pToken;

        if (pProperty(RIB_PROVIDER_VERSION, &uVersion, NULL, NULL) == 0)
        {
            AIL_set_error(RIB_OBSOLETE_MESSAGE);
            return RIB_NOT_FOUND;
        }

        iMajor = (uVersion >> 8) & 0xFF;
        iMinor = (uVersion >> 4) & 0xF;

        if (iMajor <= 0 || (iMajor == RIB_MINIMUM_MAJOR_VERSION && iMinor < RIB_MINIMUM_MINOR_VERSION))
        {
            AIL_set_error(RIB_OBSOLETE_MESSAGE);
            return RIB_NOT_FOUND;
        }
    }

    descriptor.uProvider = uProvider;
    descriptor.pInterfaceName = pInterfaceName;

    for (i = 0; i < iEntryCount; i++)
    {
        descriptor.pEntryName = pEntries[i].pEntryName;
        descriptor.uType = pEntries[i].uType;
        descriptor.uSubType = pEntries[i].uSubType;
        descriptor.pToken = pEntries[i].pToken;

        uSet = ILE_set(&descriptor);

        pEntry = IL_alloc(&RIB_interfaces);
        if (pEntry == NULL)
        {
            continue;
        }

        pEntry->uSet = uSet;
        pEntry->pSetPrevious = NULL;
        pEntry->pSetNext = RIB_interfaces.pSet[uSet];

        if (RIB_interfaces.pSet[uSet] != NULL)
        {
            RIB_interfaces.pSet[uSet]->pSetPrevious = pEntry;
        }

        RIB_interfaces.pSet[uSet] = pEntry;
        pEntry->descriptor = descriptor;
    }

    return RIB_NOERR;
}

extern "C" int RIB_unregister_interface(unsigned int uProvider, const char* pInterfaceName, int iEntryCount,
                                        const RIB_INTERFACE_ENTRY* pEntries)
{
    INTERFACE_DESCRIPTOR descriptor;
    ILIST_ENTRY* pEntry;
    ILIST_ENTRY* pNextEntry;
    int iMissing;
    int iSet;
    int i;

    iMissing = 0;

    if (pEntries != NULL && pInterfaceName != NULL)
    {
        descriptor.uProvider = uProvider;
        descriptor.pInterfaceName = pInterfaceName;

        for (i = 0; i < iEntryCount; i++)
        {
            descriptor.pEntryName = pEntries[i].pEntryName;
            descriptor.uType = pEntries[i].uType;

            pEntry = IL_lookup(&RIB_interfaces, &descriptor);

            if (pEntry == NULL)
            {
                iMissing = iMissing + 1;
            }
            else
            {
                IL_dealloc(&RIB_interfaces, pEntry);
            }
        }

        if (iMissing != 0)
        {
            strcpy(RIB_error_text, RIB_MISSING_ENTRIES_MESSAGE);
            return RIB_ERROR;
        }
    }
    else
    {
        for (iSet = 0; iSet < RIB_INTERFACE_SET_COUNT; iSet++)
        {
            pEntry = RIB_interfaces.pSet[iSet];

            while (pEntry != NULL)
            {
                pNextEntry = pEntry->pSetNext;

                if (pEntry->iIndex != -1 && pEntry->descriptor.uProvider == uProvider
                    && (pInterfaceName == NULL || strcmp(pInterfaceName, pEntry->descriptor.pInterfaceName) == 0))
                {
                    IL_dealloc(&RIB_interfaces, pEntry);
                }

                pEntry = pNextEntry;
            }
        }
    }

    return RIB_NOERR;
}

extern "C" int RIB_request_interface(unsigned int uProvider, const char* pInterfaceName, int iEntryCount,
                                     const RIB_INTERFACE_ENTRY* pEntries)
{
    INTERFACE_DESCRIPTOR descriptor;
    ILIST_ENTRY* pRegistered;
    ILIST_ENTRY* pEntry;
    int iMissing;
    int i;

    pRegistered = NULL;

    for (i = 0; i < RIB_interfaces.iCount; i++)
    {
        if (RIB_interfaces.pBase[i].iIndex != -1 && RIB_interfaces.pBase[i].descriptor.uProvider == uProvider
            && strcmp(pInterfaceName, RIB_interfaces.pBase[i].descriptor.pInterfaceName) == 0)
        {
            pRegistered = &RIB_interfaces.pBase[i];
            break;
        }
    }

    if (pRegistered == NULL)
    {
        return RIB_NOT_FOUND;
    }

    iMissing = 0;

    if (iEntryCount <= 0)
    {
        return RIB_NOERR;
    }

    descriptor.uProvider = uProvider;
    descriptor.pInterfaceName = pInterfaceName;

    for (i = 0; i < iEntryCount; i++)
    {
        descriptor.pEntryName = pEntries[i].pEntryName;
        descriptor.uType = pEntries[i].uType;

        pEntry = IL_lookup(&RIB_interfaces, &descriptor);

        if (pEntry != NULL)
        {
#if __SIZEOF_POINTER__ == 8
            StoreToken(pEntries[i].pToken, pEntries[i].uType, pEntry->descriptor.pToken);
#else
            *(void**)pEntries[i].pToken = pEntry->descriptor.pToken;
#endif
        }
        else
        {
            if (pEntries[i].uType != 0)
            {
#if __SIZEOF_POINTER__ == 8
                StoreToken(pEntries[i].pToken, pEntries[i].uType, (void*)RIB_ENTRY_NOT_FOUND);
#else
                *(void**)pEntries[i].pToken = (void*)RIB_ENTRY_NOT_FOUND;
#endif
            }
            else
            {
                *(void**)pEntries[i].pToken = NULL;
            }

            iMissing = iMissing + 1;
        }
    }

    if (iMissing == 0)
    {
        return RIB_NOERR;
    }

    strcpy(RIB_error_text, RIB_MISSING_REQUESTED_MESSAGE);
    return RIB_ERROR;
}

extern "C" int RIB_request_interface_entry(unsigned int uProvider, const char* pInterfaceName, unsigned int uEntryType,
                                           const char* pEntryName, void* pEntry)
{
    INTERFACE_DESCRIPTOR descriptor;
    ILIST_ENTRY* pFound;

    descriptor.uProvider = uProvider;
    descriptor.pInterfaceName = pInterfaceName;
    descriptor.uType = uEntryType;
    descriptor.pEntryName = pEntryName;

    pFound = IL_lookup(&RIB_interfaces, &descriptor);

    if (pFound == NULL)
    {
#if __SIZEOF_POINTER__ == 8
        StoreToken(pEntry, uEntryType, NULL);
#else
        *(void**)pEntry = NULL;
#endif
        return RIB_NOT_FOUND;
    }

#if __SIZEOF_POINTER__ == 8
    StoreToken(pEntry, uEntryType, pFound->descriptor.pToken);
#else
    *(void**)pEntry = pFound->descriptor.pToken;
#endif
    RIB_last_entry = pFound;
    return RIB_NOERR;
}

extern "C" unsigned int RIB_load_static_provider_library(RIB_MAIN_CALLBACK pMain, const char* pLibraryName)
{
    unsigned int uProvider;
    unsigned int uIndex;
    int bFound;

    uProvider = 0;
    bFound = 0;

    if (RIB_num_users != 0)
    {
        for (uIndex = 0; uIndex < (unsigned int)RIB_providers.iCount; uIndex++)
        {
            if (uIndex != 0 && RIB_providers.pBase[uIndex].iIndex != -1 && RIB_providers.pBase[uIndex].pMain == pMain)
            {
                uProvider = uIndex;
                bFound = 1;
                break;
            }
        }
    }

    if (!bFound)
    {
        uProvider = RIB_alloc_provider_handle(NULL);
        RIB_providers.pBase[uProvider].pMain = pMain;
    }

    RIB_providers.pBase[uProvider].iReferenceCount++;

    if (RIB_providers.pBase[uProvider].iReferenceCount == 1)
    {
        if (RIB_providers.pBase[uProvider].pMain(uProvider, 1, RIB_alloc_provider_handle, RIB_register_interface,
                                                 RIB_unregister_interface)
            == 0)
        {
            RIB_free_provider_handle(uProvider);
            uProvider = 0;
        }
        else
        {
            RIB_providers.pBase[uProvider].pUserData = NULL;
            strcpy(RIB_providers.pBase[uProvider].szName, pLibraryName);
        }
    }

    return uProvider;
}

extern "C" void RIB_free_provider_library(unsigned int uProvider)
{
    unsigned int uIndex;

    if (uProvider == 0)
    {
        for (uIndex = 0; RIB_num_users != 0 && uIndex < (unsigned int)RIB_providers.iCount; uIndex++)
        {
            PVLIST_ENTRY* pProvider = &RIB_providers.pBase[uIndex];

            if (uIndex != 0 && pProvider->iIndex != -1)
            {
                RIB_free_provider_library(uIndex);
            }
        }
    }
    else
    {
        RIB_providers.pBase[uProvider].iReferenceCount--;

        if (RIB_providers.pBase[uProvider].iReferenceCount < 1)
        {
            if (RIB_providers.pBase[uProvider].pMain != NULL)
            {
                RIB_providers.pBase[uProvider].pMain(uProvider, 0, RIB_alloc_provider_handle, RIB_register_interface,
                                                     RIB_unregister_interface);
            }

            RIB_free_provider_handle(uProvider);
        }
    }
}

extern "C" int RIB_enumerate_providers(const char* pInterfaceName, unsigned int* puIndex, unsigned int* puProvider)
{
    unsigned int uLocalIndex;
    unsigned int uProvider;
    ILIST_ENTRY* pEntry;
    int iFound;
    int iSet;

    uLocalIndex = RIB_ENUMERATE_FIRST;

    if (puIndex == NULL)
    {
        puIndex = &uLocalIndex;
    }

    uProvider = 1;

    if (*puIndex != RIB_ENUMERATE_FIRST)
    {
        uProvider = *puIndex + 1;
    }

    if (pInterfaceName == NULL)
    {
        while (uProvider < (unsigned int)RIB_providers.iCount)
        {
            iFound = 0;

            for (iSet = 0; iSet < RIB_INTERFACE_SET_COUNT && iFound == 0; iSet++)
            {
                for (pEntry = RIB_interfaces.pSet[iSet]; pEntry != NULL; pEntry = pEntry->pSetNext)
                {
                    if (pEntry->iIndex != -1 && pEntry->descriptor.uProvider == uProvider)
                    {
                        iFound = 1;
                        break;
                    }
                }
            }

            if (iFound != 0)
            {
                *puIndex = uProvider;
                *puProvider = uProvider;
                return 1;
            }

            uProvider = uProvider + 1;
        }
    }
    else
    {
        for (; uProvider < (unsigned int)RIB_providers.iCount; uProvider++)
        {
            for (iSet = 0; iSet < RIB_INTERFACE_SET_COUNT; iSet++)
            {
                for (pEntry = RIB_interfaces.pSet[iSet]; pEntry != NULL; pEntry = pEntry->pSetNext)
                {
                    if (pEntry->iIndex != -1 && pEntry->descriptor.uProvider == uProvider
                        && strcmp(pInterfaceName, pEntry->descriptor.pInterfaceName) == 0)
                    {
                        *puIndex = uProvider;
                        *puProvider = uProvider;
                        return 1;
                    }
                }
            }
        }
    }

    *puIndex = RIB_ENUMERATE_FIRST;
    *puProvider = 0;
    return 0;
}

extern "C" int match_suffix(char* pFilterList, char* pFileName)
{
    char* pSuffix;
    size_t uLength;
    int iDot;
    int iSuffixLength;
    int iPatternLength;

    uLength = strlen(pFileName);
    iDot = (int)uLength - 1;

    for (; iDot > 0; iDot--)
    {
        if (pFileName[iDot] == '.')
        {
            break;
        }
    }

    pSuffix = pFileName + iDot;
    iSuffixLength = (int)strlen(pSuffix);

    for (;;)
    {
        pFilterList = pFilterList + strlen(pFilterList) + 1;
        iPatternLength = (int)strlen(pFilterList);

        if (iPatternLength == 0)
        {
            return 0;
        }

        if (iSuffixLength <= iPatternLength
            && AIL_stricmp(pFilterList + (iPatternLength - iSuffixLength), pSuffix) == 0)
        {
            return 1;
        }

        pFilterList = pFilterList + iPatternLength + 1;

        if (*pFilterList == '\0')
        {
            return 0;
        }
    }
}

extern "C" unsigned int RIB_find_file_provider(const char* pInterfaceName, const char* pAttributeName,
                                               const char* pFileName)
{
    ILIST_ENTRY* pEntry;
    RIB_PROVIDER_PROPERTY_CALLBACK pProperty;
    char* pExtensions;
    unsigned int uAttribute;
    int iSet;

    for (iSet = 0; iSet < RIB_INTERFACE_SET_COUNT; iSet++)
    {
        for (pEntry = RIB_interfaces.pSet[iSet]; pEntry != NULL; pEntry = pEntry->pSetNext)
        {
            if (pEntry->iIndex == -1 || pEntry->descriptor.uType != RIB_FUNCTION)
            {
                continue;
            }

            if (strcmp(pInterfaceName, pEntry->descriptor.pInterfaceName) != 0)
            {
                continue;
            }

            if (strcmp(pEntry->descriptor.pEntryName, RIB_PROVIDER_PROPERTY_ENTRY) != 0)
            {
                continue;
            }

            pProperty = (RIB_PROVIDER_PROPERTY_CALLBACK)pEntry->descriptor.pToken;

            if (RIB_request_interface_entry(pEntry->descriptor.uProvider, pEntry->descriptor.pInterfaceName,
                                            RIB_PROPERTY, pAttributeName, &uAttribute)
                != 0)
            {
                continue;
            }

            pExtensions = "";
            pProperty(uAttribute, &pExtensions, NULL, NULL);

            if (match_suffix(pExtensions, const_cast<char*>(pFileName)) != 0)
            {
                return pEntry->descriptor.uProvider;
            }
        }
    }

    return 0;
}

extern "C" unsigned int RIB_find_file_dec_provider(const char* pInterfaceName, const char* pInputAttribute,
                                                   unsigned int uInputValue, const char* pOutputAttribute,
                                                   const char* pOutputValue)
{
    ILIST_ENTRY* pEntry;
    RIB_PROVIDER_PROPERTY_CALLBACK pProperty;
    unsigned int uIdAttribute;
    unsigned int uExtensionAttribute;
    unsigned int uProviderValue;
    char* pExtensions;
    int iSet;

    for (iSet = 0; iSet < RIB_INTERFACE_SET_COUNT; iSet++)
    {
        for (pEntry = RIB_interfaces.pSet[iSet]; pEntry != NULL; pEntry = pEntry->pSetNext)
        {
            if (pEntry->iIndex == -1 || pEntry->descriptor.uType != RIB_FUNCTION)
            {
                continue;
            }

            if (strcmp(pInterfaceName, pEntry->descriptor.pInterfaceName) != 0)
            {
                continue;
            }

            if (strcmp(pEntry->descriptor.pEntryName, RIB_PROVIDER_PROPERTY_ENTRY) != 0)
            {
                continue;
            }

            pProperty = (RIB_PROVIDER_PROPERTY_CALLBACK)pEntry->descriptor.pToken;

            if (RIB_request_interface_entry(pEntry->descriptor.uProvider, pEntry->descriptor.pInterfaceName,
                                            RIB_PROPERTY, pInputAttribute, &uIdAttribute)
                != 0)
            {
                continue;
            }

            if (RIB_request_interface_entry(pEntry->descriptor.uProvider, pEntry->descriptor.pInterfaceName,
                                            RIB_PROPERTY, pOutputAttribute, &uExtensionAttribute)
                != 0)
            {
                continue;
            }

            uProviderValue = RIB_ENTRY_NOT_FOUND;
            pProperty(uIdAttribute, &uProviderValue, NULL, NULL);

            pExtensions = "";
            pProperty(uExtensionAttribute, &pExtensions, NULL, NULL);

            if (uInputValue == uProviderValue && match_suffix(pExtensions, const_cast<char*>(pOutputValue)) != 0)
            {
                return pEntry->descriptor.uProvider;
            }
        }
    }

    return 0;
}
