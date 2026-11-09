#include "platform/wiiu/cCommerceNintendoWiiU.hpp"

cCommerceNintendoWiiU::cCommerceNintendoWiiU()
{
}

void cCommerceNintendoWiiU::Init()
{
    SetCommerceInitialised(true);
    SetShoppingCatalogState(eCommerce_ShoppingCatalogState_Retrieved);
    SetItemCount(0);
    SetDLCCatalogListRetrieved(true);
    SetDLCCatalogListInfoRetrieved(true);
    SetDLCCatalogState(eDLCCatalogState_Retrieved);
}

unsigned int cCommerceNintendoWiiU::LogIn()
{
    return 1;
}

void cCommerceNintendoWiiU::tick()
{
}

unsigned int cCommerceNintendoWiiU::PurchaseDLCByIndex(unsigned int, int)
{
    return 1;
}

unsigned int cCommerceNintendoWiiU::PurchaseDLCByPackID(unsigned int)
{
    return 1;
}

unsigned int cCommerceNintendoWiiU::GetDLCProductInfo(cProductInfo** productInfo, unsigned int* n, unsigned int,
                                                      bool)
{
    if (productInfo != 0)
        *productInfo = 0;
    if (n != 0)
        *n = 0;
    return 0;
}

unsigned int cCommerceNintendoWiiU::AttemptLogIn()
{
    return 1;
}

void cCommerceNintendoWiiU::GetShoppingCatalog()
{
}

cCommerceNintendo::CatalogRecord* cCommerceNintendoWiiU::GetCommerceInfo()
{
    return 0;
}

namespace nn
{
namespace ec
{
namespace applet
{
bool IsExecuting() asm("IsExecuting__Q3_2nn2ec6appletFv");
void Finish() asm("Finish__Q3_2nn2ec6appletFv");
int GetReturnCode() asm("GetReturnCode__Q3_2nn2ec6appletFv");

bool IsExecuting()
{
    return false;
}

void Finish()
{
}

int GetReturnCode()
{
    return 0;
}
}
}
}
