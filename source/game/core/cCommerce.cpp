#include "game/core/cCommerce.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"

#include "game/core/DLCManager.hpp"
#include "game/core/DLCPack.hpp"
#include "game/core/cProductInfo.hpp"

unsigned char g_bPurchasabilityUpdated = 0;
extern unsigned char g_bDLCCatalogListRetrieved;
extern unsigned char g_bDLCCatalogListInfoRetrieved;
extern cCommerce::eDLCCatalogState g_eDLCCatalogState;

unsigned char g_bDLCCatalogListRetrieved = 0;
unsigned char g_bDLCCatalogListInfoRetrieved = 0;
cCommerce::eDLCCatalogState g_eDLCCatalogState = cCommerce::eDLCCatalogState_Unknown;
cProductInfo* g_pDLCProductInfo = NULL;

cCommerce::cCommerce()
{
}

cCommerce::~cCommerce()
{
}

void cCommerce::ClearProductInfo()
{
    g_bPurchasabilityUpdated = false;
    if (g_pDLCProductInfo != NULL)
    {
        delete[] g_pDLCProductInfo;
    }
    g_pDLCProductInfo = NULL;
}

unsigned int cCommerce::PurchaseDLCByIndex(unsigned int index, int contentType)
{
    return 0;
}

bool cCommerce::OwnsBundleContainingPack(unsigned int packId)
{
    bool owned = false;

    switch (packId)
    {
    case 0x1002:
    case 0x1003:
    case 0x1004:
    case 0x1005:
    {
        DLCPack* pack = gApp.mDLCManager.getPackFromPackID(0x1006);
        if (pack != NULL && pack->hasPurchasedPack())
            owned = true;
        break;
    }
    case 0x22b:
    case 0x1011:
    case 0x1012:
    case 0x1013:
    {
        DLCPack* pack = gApp.mDLCManager.getPackFromPackID(0x1010);
        if (pack != NULL && pack->hasPurchasedPack())
            owned = true;
        break;
    }
    case 0x21e:
    case 0x409:
    case 0x100b:
    {
        DLCPack* pack = gApp.mDLCManager.getPackFromPackID(0x1194);
        if (pack != NULL && pack->hasPurchasedPack())
            owned = true;
        break;
    }
    case 0x21a:
    case 0x407:
    case 0x801:
    case 0x802:
    case 0x807:
    case 0x809:
    {
        DLCPack* pack = gApp.mDLCManager.getPackFromPackID(0x1195);
        if (pack != NULL && pack->hasPurchasedPack())
            owned = true;
        break;
    }
    case 0x206:
    case 0x405:
    case 0x100c:
    {
        DLCPack* pack = gApp.mDLCManager.getPackFromPackID(0x1196);
        if (pack != NULL && pack->hasPurchasedPack())
            owned = true;
        break;
    }
    case 0x214:
    case 0x219:
    {
        DLCPack* pack = gApp.mDLCManager.getPackFromPackID(0x1197);
        if (pack != NULL && pack->hasPurchasedPack())
            owned = true;
        break;
    }
    }

    return owned;
}

unsigned int cCommerce::PurchaseDLCByPackID(unsigned int packId)
{
    return 0;
}

unsigned int cCommerce::HasPurchased(unsigned int index, bool* purchased, bool* available)
{
    if (g_pDLCProductInfo != NULL)
    {
        *available = true;
        *purchased = !g_pDLCProductInfo[index].CanPurchase();
        return 1;
    }
    *purchased = false;
    *available = false;
    return 0;
}

unsigned int cCommerce::GetDLCProductInfo(cProductInfo** productInfo, unsigned int* count, unsigned int index,
                                          bool includeUnavailable)
{
    return 0;
}

unsigned int cCommerce::CheckForEmptyStore(int state)
{
    return 0;
}

unsigned int cCommerce::ProductListAvailable()
{
    return 0;
}

void cCommerce::GetProductTextureName(std::wstring& textureName, char* productName)
{
}

void cCommerce::ShowStoreIcon()
{
}

void cCommerce::HideStoreIcon()
{
}

unsigned char cCommerce::GetPurchasabilityUpdated()
{
    return g_bPurchasabilityUpdated;
}

void cCommerce::SetPurchasabilityUpdated(bool updated)
{
    g_bPurchasabilityUpdated = updated;
}

unsigned char cCommerce::GetDLCCatalogListRetrieved()
{
    return g_bDLCCatalogListRetrieved;
}

unsigned char cCommerce::GetDLCCatalogListInfoRetrieved()
{
    return g_bDLCCatalogListInfoRetrieved;
}

unsigned int cCommerce::GetDLCCatalogState()
{
    return static_cast<unsigned int>(g_eDLCCatalogState);
}

void cCommerce::SetDLCCatalogListRetrieved(bool retrieved)
{
    g_bDLCCatalogListRetrieved = retrieved;
    if (!retrieved)
    {
        return;
    }
    this->SetDLCCatalogState(eDLCCatalogState_Retrieved);
}

void cCommerce::SetDLCCatalogListInfoRetrieved(bool retrieved)
{
    g_bDLCCatalogListInfoRetrieved = retrieved;
}

void cCommerce::SetDLCCatalogState(eDLCCatalogState state)
{
    g_eDLCCatalogState = state;
}

unsigned int cCommerce::AttemptLogIn()
{
    return 1;
}
