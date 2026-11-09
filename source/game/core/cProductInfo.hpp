#ifndef GAME_CORE_CPRODUCTINFO_HPP
#define GAME_CORE_CPRODUCTINFO_HPP

class cProductInfo
{
public:
    cProductInfo();
    ~cProductInfo();

    void Init(bool canPurchase, bool canDownload, unsigned int price, char* pName, char* pDescription,
              char* pPriceString, char* pProductIdString, char* pSKUId, char* pBanner, unsigned int packID,
              unsigned int sortIndex);

    bool CanPurchase();
    bool CanDownload();
    void SetCanPurchase(bool canPurchase);

    unsigned int GetSortIndex();
    unsigned int GetPackID();
    unsigned int GetPrice();

    char* GetName();
    char* GetDescription();
    char* GetPriceString();
    char* GetProductIdString();
    char* GetProductId();
    char* GetSKUId();
    char* GetBanner();

protected:
    bool m_bCanDownload;
    bool m_bCanPurchase;
    unsigned int m_SortIndex;
    unsigned int m_PackID;
    unsigned int m_Price;
    char* m_pName;
    char* m_pDescription;
    char* m_pPriceString;
    char* m_pProductIdString;
    char* m_pSKUId;
    char* m_pBanner;
};

#endif
