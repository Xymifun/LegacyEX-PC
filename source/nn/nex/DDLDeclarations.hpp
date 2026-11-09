#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_DDLDECLARATIONS_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_DDLDECLARATIONS_HPP

#include "nn/nex/PseudoGlobalVariable.hpp"
#include "nn/nex/RootObject.hpp"

namespace nn
{
namespace nex
{

class DDLDeclarations : public RootObject
{
public:
    explicit DDLDeclarations(bool bConstructionFlag);
    virtual ~DDLDeclarations();

    virtual void Init() = 0;

    void Load();
    void Unload();
    void RegisterIfRequired();
    void Unregister();

    static void LoadAll();
    static void UnloadAll();
    static void UnregisterAll();
    static void ResetDOClassIDs();

private:
    int m_iLoadCount;
    bool m_bRegistered;
    DDLDeclarations* m_pNext;
    bool m_bConstructionFlag;

    static PseudoGlobalVariable<DDLDeclarations*> s_pFirstDeclarations;
    static PseudoGlobalVariable<unsigned int> s_uiNextDOClassID;
};

class DynamicRunTimeInterface : public RootObject
{
public:
    DynamicRunTimeInterface();
    virtual ~DynamicRunTimeInterface();

    virtual void UnknownVirtual2() = 0;
    virtual void UnknownVirtual3() = 0;
    virtual void UnknownVirtual4() = 0;
    virtual void UnknownVirtual5() = 0;
    virtual void DoRegisterParseTree(unsigned char* pData, unsigned int uiSize) = 0;

    static DynamicRunTimeInterface* GetInstance();
    static void RegisterParseTree(unsigned char* pData, unsigned int uiSize);

private:
    static PseudoGlobalVariable<DynamicRunTimeInterface*> s_pInstance;
};

}
}

#endif
