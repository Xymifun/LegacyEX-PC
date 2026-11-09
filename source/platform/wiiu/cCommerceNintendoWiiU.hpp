#ifndef PLATFORM_WIIU_CCOMMERCENINTENDOWIIU_HPP
#define PLATFORM_WIIU_CCOMMERCENINTENDOWIIU_HPP

#include <stdint.h>

#include "platform/wiiu/cCommerceNintendo.hpp"

#include <coreinit/thread.h>
#include <nn/result.h>
#include <nn/ac.h>

namespace nn
{
namespace ec
{
class Money
{
public:
    Money(const char*, const char*, const char*);
    const char* GetAmount() const asm("GetAmount__Q3_2nn2ec5MoneyCFv");

private:
    uint32_t m_data[16];
};

class Price
{
public:
    const Money* GetSalesPrice() const asm("GetSalesPrice__Q3_2nn2ec5PriceCFv");
};

class DateTime
{
public:
    DateTime();
    long long operator-(const DateTime&) const asm("__mi__Q3_2nn2ec8DateTimeCFRCQ3_2nn2ec8DateTime");

private:
    unsigned char m_data[8];
};

class Aoc
{
public:
    uint32_t GetNumContents() const asm("GetNumContents__Q3_2nn2ec3AocCFv");
    const uint16_t* GetContentIndexes() const asm("GetContentIndexes__Q3_2nn2ec3AocCFv");
    uint64_t GetDataSize() const asm("GetDataSize__Q3_2nn2ec3AocCFv");
    bool IsInstalled() const asm("IsInstalled__Q3_2nn2ec3AocCFv");
};

enum SalesStatus
{
    SalesStatus_2 = 2,
    SalesStatus_3 = 3
};

class Item
{
public:
    SalesStatus GetSalesStatus() const asm("GetSalesStatus__Q3_2nn2ec4ItemCFv");
    const char* GetAttribute(const char*) const asm("GetAttribute__Q3_2nn2ec4ItemCFPCc");
    nn::Result CheckPurchasable() const asm("CheckPurchasable__Q3_2nn2ec4ItemCFv");
    nn::Result CheckDownloadable() const asm("CheckDownloadable__Q3_2nn2ec4ItemCFv");
    DateTime GetReleaseDate() const asm("GetReleaseDate__Q3_2nn2ec4ItemCFv");
    const Aoc* GetAoc() const asm("GetAoc__Q3_2nn2ec4ItemCFv");
    const Price* GetPrice() const asm("GetPrice__Q3_2nn2ec4ItemCFv");
    const char* GetName() const asm("GetName__Q3_2nn2ec4ItemCFv");
    const char* GetDescription() const asm("GetDescription__Q3_2nn2ec4ItemCFv");
};

class Query;
enum SalesStatusFilter
{
    SalesStatusFilter_0 = 0
};

class ItemList
{
public:
    Item* GetItem(uint32_t) asm("GetItem__Q3_2nn2ec8ItemListFUi");
    uint32_t GetNumItems() const asm("GetNumItems__Q3_2nn2ec8ItemListCFv");
};

class ShoppingCatalog : public ItemList
{
public:
    ShoppingCatalog();
    ~ShoppingCatalog();
    uint32_t GetTotal() const asm("GetTotal__Q3_2nn2ec15ShoppingCatalogCFv");
    nn::Result RetrieveAocs(const Query*, uint32_t,
                            uint32_t) asm("RetrieveAocs__Q3_2nn2ec15ShoppingCatalogFPCQ3_2nn2ec5QueryUiT2");

private:
    unsigned char m_data[8];
};

class Query
{
public:
    Query();
    ~Query();
    uint32_t SetSalesStatusFilter(nn::ec::SalesStatusFilter) asm(
        "SetSalesStatusFilter__Q3_2nn2ec5QueryFQ3_2nn2ec17SalesStatusFilter");
};

nn::Result SynchronizeAocRights() asm("SynchronizeAocRights__Q2_2nn2ecFv");
uint32_t Initialize(uint32_t) asm("Initialize__Q2_2nn2ecFUi");
nn::Result Login() asm("Login__Q2_2nn2ecFv");
void GetBalance(Money*) asm("GetBalance__Q2_2nn2ecFPQ3_2nn2ec5Money");
void GetServerTime(DateTime*) asm("GetServerTime__Q2_2nn2ecFPQ3_2nn2ec8DateTime");
uint32_t GetErrorCode(const nn::Result&) asm("GetErrorCode__Q2_2nn2ecFRCQ2_2nn6Result");
void GetLastServerErrorMessage(wchar_t*, wchar_t*) asm("GetLastServerErrorMessage__Q2_2nn2ecFPwT1");
namespace applet
{
nn::Result Purchase(const Item*, Money*) asm("Purchase__Q3_2nn2ec6appletFPCQ3_2nn2ec4ItemPQ3_2nn2ec5Money");
nn::Result Download(const Item*) asm("Download__Q3_2nn2ec6appletFPCQ3_2nn2ec4Item");
}
}

}

struct DLC_INFO;

class cCommerceNintendoWiiU : public cCommerceNintendo
{
public:
    cCommerceNintendoWiiU();
    void GetBalance();
    virtual unsigned int LogIn();
    void Init();
    unsigned char LoggedIn();
    unsigned int Purchase(unsigned int index);
    unsigned int ReDownload(unsigned int index);
    static int GetShoppingCatalog_threadEntry(int argumentCount, void* argument);
    void LaunchThread_GetShoppingCatalog();
    void ClearShoppingCatalogRetrieved();
    virtual void tick();
    unsigned int PurchaseDLC(DLC_INFO* info);
    virtual unsigned int PurchaseDLCByIndex(unsigned int index, int controllerId);
    virtual unsigned int PurchaseDLCByPackID(unsigned int packId);
    static int OrderSortFunction(const void* first, const void* second);
    virtual unsigned int GetDLCProductInfo(cProductInfo** productInfo, unsigned int* count, unsigned int category,
                                           bool includeUnavailable);
    virtual void SetDLCCatalogState(cCommerce::eDLCCatalogState state);
    virtual unsigned int AttemptLogIn();
    virtual void GetShoppingCatalog();
    virtual CatalogRecord* GetCommerceInfo();
    void SetErrorOccurred(bool errorOccurred) { m_errorOccurred = errorOccurred; }
    bool GetErrorOccurred() { return m_errorOccurred; }
    void AsserteCommerceError(nn::Result& result);
    void Finalize();
    const wchar_t* GeteCommerceErrorString(nn::Result& result);

private:
    OSThread m_catalogThread;
    CatalogRecord* m_catalogData;
    nn::ec::Money m_balance;
    nn::ec::ShoppingCatalog m_shoppingCatalog;
    nn::Result m_lastResult;
    bool m_errorOccurred;
};

#endif
