#ifndef PLATFORM_WIIU_CCOMMERCENINTENDOWIIU_HPP
#define PLATFORM_WIIU_CCOMMERCENINTENDOWIIU_HPP

#include <stdint.h>

#include "platform/wiiu/cCommerceNintendo.hpp"

#include <coreinit/thread.h>
#include <nn/ac.h>
#include <nn/result.h>

struct DLC_INFO;

class cCommerceNintendoWiiU : public cCommerceNintendo
{
public:
    cCommerceNintendoWiiU();
    void Init();
    virtual unsigned int LogIn();
    virtual void tick();
    virtual unsigned int PurchaseDLCByIndex(unsigned int index, int controllerId);
    virtual unsigned int PurchaseDLCByPackID(unsigned int packId);
    virtual unsigned int GetDLCProductInfo(cProductInfo** productInfo, unsigned int* count, unsigned int category,
                                           bool includeUnavailable);
    virtual unsigned int AttemptLogIn();
    virtual void GetShoppingCatalog();
    virtual CatalogRecord* GetCommerceInfo();
};

#endif
