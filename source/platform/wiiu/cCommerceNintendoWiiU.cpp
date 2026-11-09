#include "platform/wiiu/cCommerceNintendoWiiU.hpp"
#include "game/core/StringIds.hpp"

#include <cstdlib>
#include <cstring>

#include <coreinit/memdefaultheap.h>
#include <nn/act.h>
#include <nsysnet/nssl.h>
#include <curl/curl.h>

#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/C_AOC.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/cDLCInfo.hpp"
#include "game/core/cProductInfo.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include "game/ui/IUIScene_DlcSelectMenu.hpp"
#include "platform/wiiu/C4JThreadImpl.hpp"
#include "platform/wiiu/CInput.hpp"
#include "platform/wiiu/NQRNetworkManager_WiiU.hpp"

extern cProductInfo* g_pDLCProductInfo;

static const NNResult s_unsetResult = {-1};
static const unsigned int s_descriptionAlreadyOwned = 32384;
static const unsigned int s_descriptionStillPurchasable = 32512;
static const unsigned int s_descriptionAlreadyDownloaded = 33280;
static const int s_ignoredErrorDescriptionFirst = 27008;
static const int s_ignoredErrorDescriptionEnd = 27136;
static const int s_serverErrorDescriptionFirst = 256000;
static const int s_serverErrorDescriptionEnd = 384000;
static const long long s_newDLCPeriodSeconds = 30 * 24 * 60 * 60;
static const unsigned int s_catalogThreadStackSize = 0x10000;

namespace nn
{
namespace ec
{
extern "C" void Money_import_ctor(Money*, const char*, const char*, const char*) asm("__ct__Q3_2nn2ec5MoneyFPCcN21");
extern "C" void ShoppingCatalog_import_ctor(ShoppingCatalog*) asm("__ct__Q3_2nn2ec15ShoppingCatalogFv");
extern "C" void ShoppingCatalog_import_dtor(ShoppingCatalog*, unsigned int) asm("__dt__Q3_2nn2ec15ShoppingCatalogFv");
extern "C" void Query_import_ctor(Query*) asm("__ct__Q3_2nn2ec5QueryFv");
extern "C" void Query_import_dtor(Query*, unsigned int) asm("__dt__Q3_2nn2ec5QueryFv");
extern "C" void DateTime_import_ctor(DateTime*) asm("__ct__Q3_2nn2ec8DateTimeFv");

Money::Money(const char* currency, const char* amount, const char* unit)
{
    Money_import_ctor(this, currency, amount, unit);
}

ShoppingCatalog::ShoppingCatalog()
{
    ShoppingCatalog_import_ctor(this);
}

ShoppingCatalog::~ShoppingCatalog()
{
    ShoppingCatalog_import_dtor(this, 2);
}

Query::Query()
{
    Query_import_ctor(this);
}

Query::~Query()
{
    Query_import_dtor(this, 2);
}

DateTime::DateTime()
{
    DateTime_import_ctor(this);
}
}
}

static inline bool IsFailure(uint32_t result)
{
    return (result & 0x80000000U) != 0;
}

cCommerceNintendoWiiU::cCommerceNintendoWiiU() : m_balance(NULL, NULL, NULL), m_lastResult(s_unsetResult)
{
    m_balance = nn::ec::Money(NULL, NULL, NULL);
    m_errorOccurred = false;
    m_catalogData = NULL;
}

void cCommerceNintendoWiiU::GetBalance()
{
    nn::ec::GetBalance(&m_balance);
}

unsigned int cCommerceNintendoWiiU::LogIn()
{
    if (nn::act::IsNetworkAccount())
    {
        NNResult connectResult = nn::ac::Connect();
        nn::Result result(connectResult);
        if (NNResult_IsFailure(connectResult))
        {
            m_lastResult = result;
            m_errorOccurred = true;
            return 0;
        }
        result = nn::ec::Login();
        if (!result.IsFailure())
        {
            result = nn::ec::SynchronizeAocRights();
            GetBalance();
            if (!result.IsFailure())
            {
                SetLoggedIn(true);
                SetFirstLogIn(false);
                return 1;
            }
        }
        else if (!GetFirstLogIn())
        {
            m_lastResult = result;
            m_errorOccurred = true;
        }
        else
        {
            SetFirstLogIn(false);
        }
    }
    return 0;
}

void cCommerceNintendoWiiU::Init()
{
    if (!GetCommerceInitialised())
    {
        NNResult connectResult = nn::ac::Connect();
        if (!IsFailure(connectResult.value))
        {
            if (NSSLInit() == 0)
            {
                curl_global_init(CURL_GLOBAL_ALL);
                nn::act::Initialize();
                nn::ec::Initialize(0x104);
                SetCommerceInitialised(true);
            }
        }
    }
}

unsigned char cCommerceNintendoWiiU::LoggedIn()
{
    return GetLoggedIn();
}

unsigned int cCommerceNintendoWiiU::Purchase(unsigned int index)
{
    if (!LoggedIn())
    {
        if (!LogIn())
        {
            SetProcessState(eCommerce_Process_State_LogInFailed);
            SetDLCCatalogState(cCommerce::eDLCCatalogState_LogInFailed);
            return 1;
        }
        SetShoppingCatalogState(eCommerce_ShoppingCatalogState_Retrieving);
        GetShoppingCatalog();
    }
    nn::ec::SynchronizeAocRights();
    if (index < m_shoppingCatalog.GetNumItems() && m_shoppingCatalog.GetNumItems() > 0)
    {
        nn::ec::Item* item = m_shoppingCatalog.GetItem(index);
        item->GetSalesStatus();
        item->GetAttribute("DLCType");
        nn::Result result = item->CheckPurchasable();
        if (!result.IsFailure())
        {
            nn::ec::applet::Purchase(m_shoppingCatalog.GetItem(index), NULL);
        }
        else if (result
                 == nn::Result(nn::Result::LEVEL_STATUS, nn::Result::RESULT_MODULE_NN_EC, s_descriptionAlreadyOwned))
        {
            result = m_shoppingCatalog.GetItem(index)->CheckDownloadable();
            if (!result.IsFailure())
            {
                result = nn::ec::applet::Download(m_shoppingCatalog.GetItem(index));
                if (result.IsFailure())
                {
                    m_lastResult = result;
                    m_errorOccurred = true;
                }
            }
            else if (result
                     == nn::Result(nn::Result::LEVEL_STATUS, nn::Result::RESULT_MODULE_NN_EC,
                                   s_descriptionAlreadyDownloaded))
            {
                return 0;
            }
            else
            {
                m_lastResult = result;
                m_errorOccurred = true;
            }
        }
        else
        {
            m_lastResult = result;
            m_errorOccurred = true;
        }
    }
    return 1;
}

unsigned int cCommerceNintendoWiiU::ReDownload(unsigned int index)
{
    nn::ec::SynchronizeAocRights();
    nn::ec::Query query;
    query.SetSalesStatusFilter(nn::ec::SalesStatusFilter_0);
    m_shoppingCatalog.RetrieveAocs(&query, 0, 100);
    if (m_shoppingCatalog.GetNumItems() > 0)
    {
        nn::Result result = m_shoppingCatalog.GetItem(index)->CheckDownloadable();
        if (!result.IsFailure())
        {
            nn::ec::applet::Download(m_shoppingCatalog.GetItem(index));
        }
        else if (result
                 == nn::Result(nn::Result::LEVEL_STATUS, nn::Result::RESULT_MODULE_NN_EC,
                               s_descriptionAlreadyDownloaded))
        {
            return 0;
        }
        else
        {
            m_lastResult = result;
            m_errorOccurred = true;
        }
    }
    return 1;
}

int cCommerceNintendoWiiU::GetShoppingCatalog_threadEntry(int, void* argument)
{
    cCommerceNintendoWiiU* commerce = static_cast<cCommerceNintendoWiiU*>(argument);
    if (!commerce->GetLoggedIn())
    {
        if (!commerce->LogIn())
        {
            commerce->SetProcessState(eCommerce_Process_State_LogInFailed);
            commerce->SetDLCCatalogState(cCommerce::eDLCCatalogState_LogInFailed);
            return 0;
        }
    }
    else
    {
        commerce->GetFirstLogIn();
    }
    commerce->SetShoppingCatalogState(eCommerce_ShoppingCatalogState_Retrieving);
    commerce->GetShoppingCatalog();
    return 0;
}

void cCommerceNintendoWiiU::LaunchThread_GetShoppingCatalog()
{
    unsigned char* stackTop = static_cast<unsigned char*>(MEMAllocFromDefaultHeap(s_catalogThreadStackSize))
                              + s_catalogThreadStackSize;
    bool created = C4JThreadImpl::OSCreateThread4J(&m_catalogThread, GetShoppingCatalog_threadEntry, 0, this, stackTop,
                                                   s_catalogThreadStackSize, 0x10,
                                                   OS_THREAD_ATTRIB_AFFINITY_CPU2 | OS_THREAD_ATTRIB_DETACHED);
    if (created)
        OSResumeThread(&m_catalogThread);
}

void cCommerceNintendoWiiU::ClearShoppingCatalogRetrieved()
{
    SetShoppingCatalogState(eCommerce_ShoppingCatalogState_Unknown);
    if (m_catalogData != NULL)
    {
        std::free(m_catalogData);
        m_catalogData = NULL;
        static_cast<cCommerce*>((void*)gApp.GetCommerceInstance())->SetDLCCatalogListRetrieved(false);
        static_cast<cCommerce*>((void*)gApp.GetCommerceInstance())->SetDLCCatalogListInfoRetrieved(false);
    }
    SetItemCount(0);
}

void cCommerceNintendoWiiU::tick()
{
    if (NQRNetworkManager_WiiU::s_applicationConnected && !CGameNetworkManager::sInstance.usingLanMode())
    {
        if (!GetNetworkConnected())
            SetFirstLogIn(true);
        SetNetworkConnected(true);
        switch (GetProcessState())
        {
        case eCommerce_Process_State_Unknown:
            if (NQRNetworkManager_WiiU::s_applicationConnected && GetCommerceInitialised())
                SetProcessState(eCommerce_Process_State_GetCatalog);
            break;
        case eCommerce_Process_State_GetCatalog:
            SetProcessState(eCommerce_Process_State_GettingCatalog);
            LaunchThread_GetShoppingCatalog();
            break;
        case eCommerce_Process_State_ForegroundAcquired:
        {
            ClearShoppingCatalogRetrieved();
            nn::ec::SynchronizeAocRights();
            C_AOC* aoc = C_AOC::getInstance();
            aoc->m_eState = C_AOC::eAOCState_Start;
            aoc->Init();
            LaunchThread_GetShoppingCatalog();
            SetProcessState(eCommerce_Process_State_RefreshingCatalog);
            break;
        }
        case eCommerce_Process_State_CatalogRefreshed:
            gConsoleUIController.HandleDLCLicenseChange();
            SetProcessState(eCommerce_Process_State_CatalogReady);
            break;
        default:
            break;
        }
    }
    else
    {
        if (GetNetworkConnected())
        {
            SetNetworkConnected(false);
            ClearShoppingCatalogRetrieved();
            SetProcessState(eCommerce_Process_State_Unknown);
            SetLoggedIn(false);
        }
    }

    if (m_errorOccurred)
    {
        m_errorOccurred = false;
        if (m_lastResult.GetModule() == nn::Result::RESULT_MODULE_NN_EC
            && m_lastResult.GetDescription() >= s_ignoredErrorDescriptionFirst
            && m_lastResult.GetDescription() < s_ignoredErrorDescriptionEnd)
            return;
        if (m_lastResult.GetModule() == nn::Result::RESULT_MODULE_NN_EC
            && m_lastResult.GetDescription() >= s_serverErrorDescriptionFirst
            && m_lastResult.GetDescription() < s_serverErrorDescriptionEnd)
        {
            wchar_t errorCode[10];
            wchar_t errorMessage[512];
            nn::ec::GetLastServerErrorMessage(errorCode, errorMessage);
            g_pInput->RequestErrorViewer(0, errorCode, errorMessage, NULL, NULL);
            return;
        }
        if (m_lastResult.GetModule() != nn::Result::RESULT_MODULE_NN_EC
            || m_lastResult.GetDescription() < s_ignoredErrorDescriptionFirst
            || m_lastResult.GetDescription() >= s_ignoredErrorDescriptionEnd)
            g_pInput->RequestErrorViewer(0, nn::ec::GetErrorCode(m_lastResult), NULL, NULL);
    }
}

unsigned int cCommerceNintendoWiiU::PurchaseDLC(DLC_INFO* info)
{
    if (info != NULL)
    {
        if (info->Purchased)
            return ReDownload(info->CatalogIndex);
        return Purchase(info->CatalogIndex);
    }
    return 0;
}

unsigned int cCommerceNintendoWiiU::PurchaseDLCByIndex(unsigned int index, int controllerId)
{
    unsigned int result = PurchaseDLC(static_cast<cDLCInfo*>((void*)gApp.GetDLCInfoInstance())
                                          ->GetDLCInfoForPackID(g_pDLCProductInfo[index].GetPackID()));
    if (result == 0)
    {
        int options[1] = {IDS_CONFIRM_OK};
        gConsoleUIController.RequestErrorMessage(IDS_DLC_ALREADY_INSTALLED_TITLE, IDS_DLC_ALREADY_INSTALLED_TEXT,
                                                 options, 1, controllerId, NULL, NULL, NULL, true);
    }
    return result;
}

unsigned int cCommerceNintendoWiiU::PurchaseDLCByPackID(unsigned int packId)
{
    return PurchaseDLC(static_cast<cDLCInfo*>((void*)gApp.GetDLCInfoInstance())->GetDLCInfoForPackID(packId));
}

int cCommerceNintendoWiiU::OrderSortFunction(const void* first, const void* second)
{
    return ((cProductInfo*)second)->GetSortIndex() - ((cProductInfo*)first)->GetSortIndex();
}

unsigned int cCommerceNintendoWiiU::GetDLCProductInfo(cProductInfo** productInfo, unsigned int* count,
                                                      unsigned int category, bool)
{
    unsigned int productCount = 0;
    CatalogRecord* catalog = GetCommerceInfo();
    if (catalog != NULL)
    {
        ClearProductInfo();
        unsigned int itemCount = GetItemCount();
        if (itemCount > 0)
        {
            g_pDLCProductInfo = new cProductInfo[itemCount];
            if (category == eDLCContentType_All)
            {
                for (unsigned int index = 0; index < itemCount; ++index)
                {
                    DLC_INFO* info = static_cast<cDLCInfo*>((void*)gApp.GetDLCInfoInstance())
                                         ->GetDLCInfoByCatalogIndex(catalog[index].catalogIndex);
                    if (info != NULL)
                    {
                        g_pDLCProductInfo[productCount].Init(catalog[index].canPurchase, catalog[index].canPurchase, 0,
                                                             catalog[index].name, catalog[index].description,
                                                             catalog[index].price, "", "", info->BannerName,
                                                             info->PackID, info->SortIndex);
                        ++productCount;
                    }
                }
                qsort(g_pDLCProductInfo, productCount, sizeof(cProductInfo), OrderSortFunction);
            }
            else
            {
                for (unsigned int index = 0; index < itemCount; ++index)
                {
                    DLC_INFO* info = static_cast<cDLCInfo*>((void*)gApp.GetDLCInfoInstance())
                                         ->GetDLCInfoByCatalogIndex(catalog[index].catalogIndex);
                    if (info != NULL && info->ContentType == static_cast<int>(category))
                    {
                        g_pDLCProductInfo[productCount].Init(catalog[index].canPurchase, catalog[index].canPurchase, 0,
                                                             catalog[index].name, catalog[index].description,
                                                             catalog[index].price, "", "", info->BannerName,
                                                             info->PackID, info->SortIndex);
                        ++productCount;
                    }
                }
                qsort(g_pDLCProductInfo, productCount, sizeof(cProductInfo), OrderSortFunction);
            }
        }

        SetPurchasabilityUpdated(false);
        *count = productCount;
        *productInfo = g_pDLCProductInfo;
        return 1;
    }

    *count = 0;
    *productInfo = NULL;
    return 0;
}

void cCommerceNintendoWiiU::SetDLCCatalogState(cCommerce::eDLCCatalogState state)
{
    cCommerce::SetDLCCatalogState(state);
    if (state == cCommerce::eDLCCatalogState_Unknown)
        SetProcessState(eCommerce_Process_State_Unknown);
}

unsigned int cCommerceNintendoWiiU::AttemptLogIn()
{
    return LogIn();
}

void cCommerceNintendoWiiU::GetShoppingCatalog()
{
    nn::ec::Query query;
    query.SetSalesStatusFilter(nn::ec::SalesStatusFilter_0);
    nn::Result result = m_shoppingCatalog.RetrieveAocs(&query, 0, 100);
    if (result.IsFailure())
    {
        m_lastResult = result;
        m_errorOccurred = true;
        return;
    }

    unsigned int recordCount = 0;
    unsigned int total = m_shoppingCatalog.GetTotal();
    m_catalogData = static_cast<CatalogRecord*>(std::calloc(total * sizeof(CatalogRecord), 1));
    for (unsigned int index = 0; index < total; ++index)
    {
        nn::ec::Item* item = m_shoppingCatalog.GetItem(index);
        nn::ec::SalesStatus salesStatus = item->GetSalesStatus();
        if (salesStatus != nn::ec::SalesStatus_2 && salesStatus != nn::ec::SalesStatus_3)
            continue;

        nn::Result purchasable = item->CheckPurchasable();
        if (!purchasable.IsFailure())
        {
            m_catalogData[recordCount].canPurchase = true;
        }
        else if (purchasable
                 == nn::Result(nn::Result::LEVEL_STATUS, nn::Result::RESULT_MODULE_NN_EC, s_descriptionAlreadyOwned))
        {
            m_catalogData[recordCount].canPurchase = false;
        }
        else if (purchasable
                 == nn::Result(nn::Result::LEVEL_STATUS, nn::Result::RESULT_MODULE_NN_EC,
                               s_descriptionStillPurchasable))
        {
            m_catalogData[recordCount].canPurchase = true;
        }
        else
        {
            continue;
        }

        nn::ec::DateTime releaseDate = item->GetReleaseDate();
        if (salesStatus == nn::ec::SalesStatus_2)
        {
            nn::ec::DateTime serverTime;
            nn::ec::GetServerTime(&serverTime);
            if (serverTime - releaseDate <= s_newDLCPeriodSeconds)
                gApp.SetNewDLCAvailable(true);
        }

        m_catalogData[recordCount].catalogIndex = index;
        const nn::ec::Aoc* aoc = item->GetAoc();
        unsigned int contentCount = aoc->GetNumContents();
        const uint16_t* contentIndexes = aoc->GetContentIndexes();
        CatalogRecord& record = m_catalogData[recordCount];
        record.contentCount = contentCount;
        for (unsigned int content = 0; content < contentCount; ++content)
            record.contentIndexes[content] = contentIndexes[content] - 16;

        const nn::ec::Money* price = item->GetPrice()->GetSalesPrice();
        std::strncpy(m_catalogData[recordCount].name, item->GetName(), sizeof(record.name));
        std::strncpy(m_catalogData[recordCount].description, item->GetDescription(), sizeof(record.description));
        std::strncpy(m_catalogData[recordCount].price, price->GetAmount(), sizeof(record.price));
        aoc->GetDataSize();
        m_catalogData[recordCount].installed = aoc->IsInstalled();
        ++recordCount;
    }

    SetProcessState(GetProcessState() == eCommerce_Process_State_RefreshingCatalog
                        ? eCommerce_Process_State_CatalogRefreshed
                        : eCommerce_Process_State_CatalogReady);
    SetShoppingCatalogState(eCommerce_ShoppingCatalogState_Retrieved);
    static_cast<cCommerce*>((void*)gApp.GetCommerceInstance())->SetDLCCatalogListRetrieved(true);
    static_cast<cCommerce*>((void*)gApp.GetCommerceInstance())->SetDLCCatalogListInfoRetrieved(true);
    SetItemCount(recordCount);
}

cCommerceNintendo::CatalogRecord* cCommerceNintendoWiiU::GetCommerceInfo()
{
    return m_catalogData;
}
