#include "platform/wiiu/cCommerceNintendo.hpp"

cCommerceNintendo::cCommerceNintendo() : cCommerce()
{
    SetLoggedIn(false);
    SetCommerceInitialised(false);
    SetFirstLogIn(true);
    SetProcessState(eCommerce_Process_State_Unknown);
    SetShoppingCatalogState(eCommerce_ShoppingCatalogState_Unknown);
    SetItemCount(0);
}

void cCommerceNintendo::SetLoggedIn(bool loggedIn)
{
    m_loggedIn = loggedIn;
}

void cCommerceNintendo::SetCommerceInitialised(bool initialised)
{
    m_commerceInitialised = initialised;
}

void cCommerceNintendo::SetFirstLogIn(bool firstLogIn)
{
    m_firstLogIn = firstLogIn;
}

void cCommerceNintendo::SetProcessState(eCommerce_Process_State state)
{
    m_processState = state;
}

void cCommerceNintendo::SetShoppingCatalogState(eCommerce_ShoppingCatalogState state)
{
    m_shoppingCatalogState = state;
}

void cCommerceNintendo::SetItemCount(unsigned int itemCount)
{
    m_itemCount = itemCount;
}

unsigned char cCommerceNintendo::GetLoggedIn()
{
    return static_cast<unsigned char>(m_loggedIn);
}

unsigned char cCommerceNintendo::GetFirstLogIn()
{
    return static_cast<unsigned char>(m_firstLogIn);
}

unsigned char cCommerceNintendo::GetCommerceInitialised()
{
    return static_cast<unsigned char>(m_commerceInitialised);
}

void cCommerceNintendo::SetNetworkConnected(bool connected)
{
    m_networkConnected = connected;
}

unsigned char cCommerceNintendo::GetNetworkConnected()
{
    return static_cast<unsigned char>(m_networkConnected);
}

cCommerceNintendo::eCommerce_Process_State cCommerceNintendo::GetProcessState()
{
    return m_processState;
}

unsigned int cCommerceNintendo::GetItemCount()
{
    return m_itemCount;
}

unsigned int cCommerceNintendo::PurchaseBedrock()
{
    return 0;
}
