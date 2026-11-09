#ifndef GAME_CORE_CCOMMERCE_HPP
#define GAME_CORE_CCOMMERCE_HPP

#include <string>

class cProductInfo;

extern cProductInfo* g_pDLCProductInfo;

class cCommerce
{
public:
    enum eDLCCatalogState
    {
        eDLCCatalogState_Unknown = 0,
        eDLCCatalogState_Retrieved = 1,
        eDLCCatalogState_LogInFailed = 2
    };

    cCommerce();
    virtual ~cCommerce();
    virtual void ClearProductInfo();
    virtual unsigned int PurchaseDLCByIndex(unsigned int index, int contentType);
    virtual bool OwnsBundleContainingPack(unsigned int packId);
    virtual unsigned int PurchaseDLCByPackID(unsigned int packId);
    virtual unsigned int HasPurchased(unsigned int index, bool* purchased, bool* available);
    virtual unsigned int GetDLCProductInfo(cProductInfo** productInfo, unsigned int* count, unsigned int index,
                                           bool includeUnavailable);
    virtual unsigned int CheckForEmptyStore(int state);
    virtual unsigned int ProductListAvailable();
    virtual void GetProductTextureName(std::wstring& textureName, char* productName);
    virtual void ShowStoreIcon();
    virtual void HideStoreIcon();
    unsigned char GetPurchasabilityUpdated();
    void SetPurchasabilityUpdated(bool updated);
    virtual unsigned char GetDLCCatalogListRetrieved();
    virtual unsigned char GetDLCCatalogListInfoRetrieved();
    unsigned int GetDLCCatalogState();
    void SetDLCCatalogListRetrieved(bool retrieved);
    void SetDLCCatalogListInfoRetrieved(bool retrieved);
    virtual void SetDLCCatalogState(eDLCCatalogState state);
    virtual unsigned int AttemptLogIn();
    unsigned char GetDLCCatalogCategoriesRetrieved();
    void SetDLCCatalogCategoriesRetrieved(bool retrieved);
};

#endif
