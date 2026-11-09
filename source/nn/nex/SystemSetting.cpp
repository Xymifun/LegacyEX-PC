#include "nn/nex/SystemSetting.hpp"

#include <algorithm>

namespace nn
{
namespace nex
{

SystemSetting::SettingList* SystemSetting::GetList()
{
    static SettingList settings;
    return &settings;
}

SystemSetting::SystemSetting(const wchar_t* name, unsigned int value)
    : m_link(),
      m_name(name),
      m_value(value),
      m_state(false)
{
    GetList()->push_back(this);
}

SystemSetting::~SystemSetting()
{
    SettingList::iterator position;
    position = std::find(GetList()->begin(), GetList()->end(), this);
    GetList()->erase(position);
}

SystemSetting g_ddlConnectionEncoding(L"ConnectionEncoding", 1);

}
}
