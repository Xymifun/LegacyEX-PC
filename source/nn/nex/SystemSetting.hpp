#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_SYSTEMSETTING_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_SYSTEMSETTING_HPP

#include "nn/nex/RootObject.hpp"
#include "nn/nex/String.hpp"
#include "nn/nex/qChain.hpp"
#include <list>
#include "nn/nex/qList.hpp"

namespace nn
{
namespace nex
{

class SystemSetting : public RootObject
{
public:
    typedef qChain<SystemSetting*, DefaultChainPolicy<SystemSetting*> > SettingList;

    SystemSetting(const wchar_t* name, unsigned int value);
    virtual ~SystemSetting();
    static SettingList* GetList();

    friend class DefaultChainPolicy<SystemSetting*>;

    qChainLink<SystemSetting*> m_link;
    const wchar_t* m_name;
    unsigned int m_value;
    bool m_state;

    bool CheckIfMutable() const;
    void SetValue(unsigned int value);
    static SystemSetting* Find(const wchar_t* name);
    static qList<String> GetAllNames();
};

}
}

#endif
