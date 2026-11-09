#include "game/core/Settings.hpp"

Settings::Settings() : mSettings(11)
{
}

Settings::~Settings()
{
    for (boost::unordered_map<EServerSetting, _SettingsValue*>::iterator it = mSettings.begin(); it != mSettings.end();
         ++it)
    {
        delete it->second;
    }
}

int Settings::getInt(EServerSetting setting, int defaultValue)
{
    if (mSettings.find(setting) == mSettings.end())
    {
        _SettingsValue* value = new _SettingsValue();
        value->mType = _SettingsValue::eSettingValueType_Int;
        value->mIntValue = defaultValue;
        mSettings[setting] = value;
    }
    return mSettings[setting]->mIntValue;
}

std::wstring Settings::getString(EServerSetting setting, const std::wstring& defaultValue)
{
    if (mSettings.find(setting) == mSettings.end())
    {
        _SettingsValue* value = new _SettingsValue();
        value->mType = _SettingsValue::eSettingValueType_String;
        value->mStringValue = defaultValue;
        mSettings[setting] = value;
    }
    return mSettings[setting]->mStringValue;
}

bool Settings::getBoolean(EServerSetting setting, bool defaultValue)
{
    if (mSettings.find(setting) == mSettings.end())
    {
        _SettingsValue* value = new _SettingsValue();
        value->mType = _SettingsValue::eSettingValueType_Boolean;
        value->mBooleanValue = defaultValue;
        mSettings[setting] = value;
    }
    return mSettings[setting]->mBooleanValue;
}

void Settings::setBooleanAndSave(EServerSetting setting, bool value)
{
    boost::unordered_map<EServerSetting, _SettingsValue*>::iterator it = mSettings.find(setting);
    if (it != mSettings.end())
    {
        delete it->second;
    }

    _SettingsValue* newValue = new _SettingsValue();
    newValue->mType = _SettingsValue::eSettingValueType_Boolean;
    newValue->mBooleanValue = value;
    mSettings[setting] = newValue;
}
