#ifndef GAME_CORE_SETTINGS_HPP
#define GAME_CORE_SETTINGS_HPP

#include <string>

#include <boost/unordered_map.hpp>

#include "game/core/EServerSetting.hpp"

class Settings
{
public:
    struct _SettingsValue
    {
        enum ESettingValueType
        {
            eSettingValueType_Boolean = 0,
            eSettingValueType_Int = 1,
            eSettingValueType_String = 2
        };

        ESettingValueType mType;
        union
        {
            bool mBooleanValue;
            int mIntValue;
        };
        std::wstring mStringValue;
    };

    Settings();
    ~Settings();

    int getInt(EServerSetting setting, int defaultValue);
    std::wstring getString(EServerSetting setting, const std::wstring& defaultValue);
    bool getBoolean(EServerSetting setting, bool defaultValue);
    void setBooleanAndSave(EServerSetting setting, bool value);
    void saveProperties();
    void generateNewProperties();

private:
    boost::unordered_map<EServerSetting, _SettingsValue*> mSettings;
};

#endif
