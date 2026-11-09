#include "nn/nex/DDLDeclarations.hpp"

namespace nn
{
namespace nex
{

namespace
{

const unsigned int FIRST_DYNAMIC_DO_CLASS_ID = 100;

}

PseudoGlobalVariable<DDLDeclarations*> DDLDeclarations::s_pFirstDeclarations(0);
PseudoGlobalVariable<unsigned int> DDLDeclarations::s_uiNextDOClassID(FIRST_DYNAMIC_DO_CLASS_ID);
PseudoGlobalVariable<DynamicRunTimeInterface*> DynamicRunTimeInterface::s_pInstance;

DDLDeclarations::DDLDeclarations(bool bConstructionFlag) : m_bConstructionFlag(bConstructionFlag)
{
    m_bRegistered = false;
    m_iLoadCount = 0;
    m_pNext = 0;
}

DDLDeclarations::~DDLDeclarations()
{
}

void DDLDeclarations::Load()
{
    if (++m_iLoadCount == 1)
        Init();
}

void DDLDeclarations::LoadAll()
{
    for (DDLDeclarations* pDeclarations = s_pFirstDeclarations.Get(); pDeclarations != 0;
         pDeclarations = pDeclarations->m_pNext)
        pDeclarations->Load();
}

void DDLDeclarations::Unload()
{
    --m_iLoadCount;
}

void DDLDeclarations::Unregister()
{
    m_pNext = 0;
    m_bRegistered = false;
}

void DDLDeclarations::UnregisterAll()
{
    DDLDeclarations* pDeclarations = s_pFirstDeclarations.Get();
    while (pDeclarations != 0)
    {
        DDLDeclarations* pNext = pDeclarations->m_pNext;
        pDeclarations->Unregister();
        pDeclarations = pNext;
    }

    s_pFirstDeclarations.Get() = 0;
}

void DDLDeclarations::UnloadAll()
{
    for (DDLDeclarations* pDeclarations = s_pFirstDeclarations.Get(); pDeclarations != 0;
         pDeclarations = pDeclarations->m_pNext)
        pDeclarations->Unload();

    UnregisterAll();
}

void DDLDeclarations::ResetDOClassIDs()
{
    s_uiNextDOClassID.Get() = FIRST_DYNAMIC_DO_CLASS_ID;
}

void DDLDeclarations::RegisterIfRequired()
{
    if (m_bRegistered)
        return;

    if (s_pFirstDeclarations.Get() == 0)
    {
        s_pFirstDeclarations.Get() = this;
    }
    else
    {
        DDLDeclarations* pLast = s_pFirstDeclarations.Get();
        while (pLast->m_pNext != 0)
            pLast = pLast->m_pNext;

        pLast->m_pNext = this;
    }

    m_bRegistered = true;
}

DynamicRunTimeInterface::DynamicRunTimeInterface()
{
    s_pInstance.Get() = this;
}

DynamicRunTimeInterface::~DynamicRunTimeInterface()
{
    s_pInstance.Get() = 0;
}

DynamicRunTimeInterface* DynamicRunTimeInterface::GetInstance()
{
    return s_pInstance.Get();
}

void DynamicRunTimeInterface::RegisterParseTree(unsigned char* pData, unsigned int uiSize)
{
    if (GetInstance() != 0)
        GetInstance()->DoRegisterParseTree(pData, uiSize);
}

}
}
