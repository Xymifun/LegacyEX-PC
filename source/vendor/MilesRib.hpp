#ifndef SOURCE_VENDOR_MILES_RIB_HPP
#define SOURCE_VENDOR_MILES_RIB_HPP

struct RIB_INTERFACE_ENTRY
{
    unsigned int uType;
    const char* pEntryName;
    void* pToken;
    unsigned int uSubType;
};

typedef unsigned int (*RIB_ALLOC_PROVIDER_HANDLE_CALLBACK)(void* pUserData);
typedef int (*RIB_REGISTER_INTERFACE_CALLBACK)(unsigned int uProvider, const char* pInterfaceName, int iEntryCount,
                                               const RIB_INTERFACE_ENTRY* pEntries);
typedef int (*RIB_UNREGISTER_INTERFACE_CALLBACK)(unsigned int uProvider, const char* pInterfaceName, int iEntryCount,
                                                 const RIB_INTERFACE_ENTRY* pEntries);
typedef int (*RIB_MAIN_CALLBACK)(unsigned int uProvider, int iUpDown,
                                 RIB_ALLOC_PROVIDER_HANDLE_CALLBACK pAllocProviderHandle,
                                 RIB_REGISTER_INTERFACE_CALLBACK pRegisterInterface,
                                 RIB_UNREGISTER_INTERFACE_CALLBACK pUnregisterInterface);
typedef int (*RIB_PROVIDER_PROPERTY_CALLBACK)(unsigned int uProperty, void* pBefore, const void* pNewValue,
                                              void* pAfter);

static const unsigned int RIB_PROVIDER_NAME = 0xFFFFFF9C;
static const unsigned int RIB_PROVIDER_VERSION = 0xFFFFFF9B;

extern "C" unsigned int RIB_alloc_provider_handle(void* pUserData);
extern "C" void RIB_free_provider_handle(unsigned int uProvider);
extern "C" int RIB_register_interface(unsigned int uProvider, const char* pInterfaceName, int iEntryCount,
                                      const RIB_INTERFACE_ENTRY* pEntries);
extern "C" int RIB_unregister_interface(unsigned int uProvider, const char* pInterfaceName, int iEntryCount,
                                        const RIB_INTERFACE_ENTRY* pEntries);
extern "C" int RIB_request_interface(unsigned int uProvider, const char* pInterfaceName, int iEntryCount,
                                     const RIB_INTERFACE_ENTRY* pEntries);
extern "C" int RIB_request_interface_entry(unsigned int uProvider, const char* pInterfaceName, unsigned int uEntryType,
                                           const char* pEntryName, void* pEntry);
extern "C" unsigned int RIB_load_static_provider_library(RIB_MAIN_CALLBACK pMain, const char* pLibraryName);
extern "C" void RIB_free_provider_library(unsigned int uProvider);
extern "C" int RIB_enumerate_providers(const char* pInterfaceName, unsigned int* puIndex, unsigned int* puProvider);
extern "C" unsigned int RIB_find_file_provider(const char* pInterfaceName, const char* pAttributeName,
                                               const char* pFileName);
extern "C" unsigned int RIB_find_file_dec_provider(const char* pInterfaceName, const char* pInputAttribute,
                                                   unsigned int uInputValue, const char* pOutputAttribute,
                                                   const char* pOutputValue);

#endif
