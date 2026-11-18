// Adapted from LCEReborn (GPLv3): source/platform/pc/shaderpack/ShaderPackManager.cpp
#include "platform/pc/shaderpack/ShaderPackManager.hpp"

#include <algorithm>
#include <dirent.h>
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <map>
#include <sstream>

#include "game/core/XLanguage.hpp"
#include "platform/pc/shaderpack/GlApi.hpp"
#include "platform/pc/shaderpack/Log.hpp"
#include "platform/pc/PcOs.hpp"
#include "platform/pc/PcPaths.hpp"

static const Logger sLog("Shaders");

namespace
{
std::string Lower(std::string text)
{
    std::transform(text.begin(), text.end(), text.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return text;
}

int ParseVersion(const char* text)
{
    if (text == nullptr)
        return 0;
    int major = 0;
    int minor = 0;
    const char* cursor = text;
    while (*cursor != 0 && !std::isdigit(static_cast<unsigned char>(*cursor)))
        ++cursor;
    major = std::atoi(cursor);
    const char* dot = std::strchr(cursor, '.');
    if (dot != nullptr)
        minor = std::atoi(dot + 1);
    while (minor >= 10)
        minor /= 10;
    return major * 100 + minor * 10;
}

bool ReadTextFile(const std::string& path, std::string& text)
{
    std::ifstream file(path.c_str(), std::ios::binary);
    if (!file)
        return false;
    std::ostringstream stream;
    stream << file.rdbuf();
    text = stream.str();
    return true;
}
}

ShaderPackManager& ShaderPackManager::Get()
{
    static ShaderPackManager instance;
    return instance;
}

ShaderPackManager::ShaderPackManager() : mGlVersion(330), mGlslVersion(330)
{
}

void ShaderPackManager::Initialise()
{
    const GLubyte* vendor = Gl::GetString(Gl::VENDOR);
    const GLubyte* renderer = Gl::GetString(Gl::RENDERER);
    mGlVendor = vendor ? reinterpret_cast<const char*>(vendor) : "";
    mGlRenderer = renderer ? reinterpret_cast<const char*>(renderer) : "";
    mGlVersion = std::max(330, ParseVersion(reinterpret_cast<const char*>(Gl::GetString(Gl::VERSION))));
    mGlslVersion =
        std::max(330, ParseVersion(reinterpret_cast<const char*>(Gl::GetString(Gl::SHADING_LANGUAGE_VERSION))));
    mGlExtensions.clear();
    GLint extensionCount = 0;
    Gl::GetIntegerv(Gl::NUM_EXTENSIONS, &extensionCount);
    for (GLint index = 0; index < extensionCount && Gl::GetStringi != nullptr; ++index)
    {
        const GLubyte* name = Gl::GetStringi(Gl::EXTENSIONS, static_cast<GLuint>(index));
        if (name != nullptr)
            mGlExtensions.push_back(reinterpret_cast<const char*>(name));
    }

    PcOsMakeDirectory("Windows64");
    PcOsMakeDirectory(PacksFolder().c_str());
    mSettings.Load(SettingsPath());
    LOG_INFO(sLog, "Shader packs: %s (GL %d, GLSL %d)", PacksFolder().c_str(), mGlVersion, mGlslVersion);
    if (mSettings.IsPackSelected())
        LoadPack(mSettings.shaderPack);
}

// Next to the save folder, which Emerald keeps when it updates the game (as it does Windows64/mods).
std::string ShaderPackManager::PacksFolder() const
{
    return "Windows64/shaderpacks";
}

std::string ShaderPackManager::SettingsPath() const
{
    return PcSaveRoot() + "/optionsshaders.txt";
}

std::string ShaderPackManager::OptionsPath(const std::string& pack) const
{
    return PacksFolder() + "/" + pack + ".txt";
}

std::vector<std::string> ShaderPackManager::ListPacks() const
{
    std::vector<std::string> packs;
    DIR* directory = opendir(PacksFolder().c_str());
    if (directory != nullptr)
    {
        while (struct dirent* entry = readdir(directory))
        {
            std::string name = entry->d_name;
            if (name != "." && name != ".." && ShaderPack::IsShaderPack(PacksFolder() + "/" + name))
                packs.push_back(name);
        }
        closedir(directory);
    }
    std::sort(packs.begin(), packs.end(), [](const std::string& left, const std::string& right) {
        return Lower(left) < Lower(right);
    });
    return packs;
}

bool ShaderPackManager::InstallPack(const std::string& source, std::string& name, std::string& error)
{
    size_t slash = source.find_last_of("/\\");
    name = slash == std::string::npos ? source : source.substr(slash + 1);
    if (!ShaderPack::IsShaderPack(source))
    {
        error = "not a shader pack";
        return false;
    }
    std::string target = PacksFolder() + "/" + name;
    std::ifstream existing(target.c_str(), std::ios::binary);
    if (existing)
    {
        error = "a shader pack with that name is already there";
        return false;
    }
    // a pack folder is copied into the packs folder by hand; zip files can be added here
    std::ifstream input(source.c_str(), std::ios::binary);
    if (!input)
    {
        error = "only zip files can be added; copy a pack folder into " + PacksFolder();
        return false;
    }
    PcOsMakeDirectory("Windows64");
    PcOsMakeDirectory(PacksFolder().c_str());
    std::ofstream output(target.c_str(), std::ios::binary | std::ios::trunc);
    output << input.rdbuf();
    if (!output)
    {
        error = "the file could not be copied";
        PcOsRemoveFile(target.c_str());
        LOG_WARN(sLog, "Could not add shader pack %s: %s", name.c_str(), error.c_str());
        return false;
    }
    LOG_INFO(sLog, "Added shader pack %s", name.c_str());
    return true;
}

bool ShaderPackManager::SaveSettings() const
{
    bool saved = mSettings.Save(SettingsPath());
    if (!saved)
        LOG_WARN(sLog, "Could not save %s", SettingsPath().c_str());
    return saved;
}

bool ShaderPackManager::SelectPack(const std::string& name)
{
    mSettings.shaderPack = name;
    SaveSettings();
    if (!mSettings.IsPackSelected())
    {
        UnloadPack();
        return true;
    }
    return LoadPack(name);
}

bool ShaderPackManager::UsePack(const std::string& name)
{
    mSettings.shaderPack = name;
    return LoadPack(name);
}

bool ShaderPackManager::SettingsSaved() const
{
    std::ifstream file(SettingsPath().c_str(), std::ios::binary);
    return static_cast<bool>(file);
}

void ShaderPackManager::UnloadPack()
{
    mPack.reset();
    mOptions = ShaderPackOptions();
    mProperties = ShaderPackProperties();
    mLang = ShaderPackLang();
    mErrors.clear();
}

bool ShaderPackManager::LoadPack(const std::string& name)
{
    UnloadPack();
    std::string path = PacksFolder() + "/" + name;
    if (!ShaderPack::IsShaderPack(path))
    {
        LOG_WARN(sLog, "Shader pack %s is missing or not a shader pack", name.c_str());
        return false;
    }
    std::string error;
    mPack = ShaderPack::Open(path, error);
    if (!mPack)
    {
        LOG_WARN(sLog, "Shader pack %s could not be opened: %s", name.c_str(), error.c_str());
        return false;
    }

    mOptions.Discover(*mPack, mErrors);
    std::string values;
    if (ReadTextFile(OptionsPath(name), values))
        mOptions.LoadValues(values);
    mLang.Load(*mPack, LocaleCode());
    RefreshProperties();
    LogSummary();
    return true;
}

void ShaderPackManager::RefreshProperties()
{
    mProperties = ParseProperties(mOptions);
}

ShaderPackProperties ShaderPackManager::ParseProperties(const ShaderPackOptions& options) const
{
    ShaderPackProperties properties;
    std::string text;
    if (mPack && mPack->ReadFile("shaders.properties", text))
        properties.Parse(text, StandardMacros(), options.Macros(), options);
    return properties;
}

bool ShaderPackManager::SetOption(const std::string& name, const std::string& value)
{
    if (!mOptions.SetValue(name, value))
        return false;
    RefreshProperties();
    return true;
}

void ShaderPackManager::ApplyProfile(const std::string& profile)
{
    const ShaderProfile* found = mProperties.Profile(profile);
    if (found == nullptr)
        return;
    ShaderPackProperties::ApplyProfile(*found, mOptions);
    RefreshProperties();
}

void ShaderPackManager::ResetOptions()
{
    mOptions.ResetAll();
    RefreshProperties();
}

void ShaderPackManager::LoadOptionValues(const std::string& values)
{
    mOptions.LoadValues(values);
    RefreshProperties();
}

bool ShaderPackManager::SaveOptions() const
{
    if (!mPack)
        return false;
    std::string path = OptionsPath(mPack->Name());
    std::string values = mOptions.SaveValues();
    if (values.empty())
    {
        PcOsRemoveFile(path.c_str());
        return true;
    }
    std::string temporary = path + ".tmp";
    {
        std::ofstream file(temporary.c_str(), std::ios::binary | std::ios::trunc);
        if (!file || !(file << values))
        {
            LOG_WARN(sLog, "Could not save %s", path.c_str());
            return false;
        }
    }
    return PcOsReplaceFile(temporary.c_str(), path.c_str());
}

ShaderMacros ShaderPackManager::StandardMacros() const
{
    return ShaderStandardMacros(mSettings, mGlVersion, mGlslVersion, mGlVendor, mGlRenderer, mGlExtensions);
}

std::string ShaderPackManager::LocaleCode()
{
    switch (XGetLanguage())
    {
    case XC_LANGUAGE_JAPANESE:
        return "ja_jp";
    case XC_LANGUAGE_GERMAN:
        return "de_de";
    case XC_LANGUAGE_FRENCH:
        return "fr_fr";
    case XC_LANGUAGE_SPANISH:
        return "es_es";
    case XC_LANGUAGE_ITALIAN:
        return "it_it";
    case XC_LANGUAGE_KOREAN:
        return "ko_kr";
    case XC_LANGUAGE_TCHINESE:
        return "zh_tw";
    case XC_LANGUAGE_PORTUGUESE:
        return "pt_br";
    case XC_LANGUAGE_SCHINESE:
    case XC_LANGUAGE_ZH_HANS:
        return "zh_cn";
    case XC_LANGUAGE_POLISH:
        return "pl_pl";
    case XC_LANGUAGE_RUSSIAN:
        return "ru_ru";
    case XC_LANGUAGE_SWEDISH:
        return "sv_se";
    case XC_LANGUAGE_TURKISH:
        return "tr_tr";
    case XC_LANGUAGE_NORWEGIAN:
        return "nb_no";
    case XC_LANGUAGE_DUTCH:
        return "nl_nl";
    case XC_LANGUAGE_FINNISH:
        return "fi_fi";
    case XC_LANGUAGE_GREEK:
        return "el_gr";
    case XC_LANGUAGE_DANISH:
        return "da_dk";
    case XC_LANGUAGE_CZECH:
        return "cs_cz";
    case XC_LANGUAGE_SLOVAK:
        return "sk_sk";
    default:
        return "en_us";
    }
}

void ShaderPackManager::LogSummary() const
{
    std::map<std::string, int> worlds;
    for (const std::string& program : mPack->ProgramFiles())
    {
        size_t slash = program.find('/');
        std::string folder = slash == std::string::npos ? std::string("shaders") : program.substr(0, slash);
        if (folder == "shaders" || folder.compare(0, 5, "world") == 0)
            ++worlds[folder];
    }
    std::string programs;
    for (const auto& world : worlds)
        programs += (programs.empty() ? "" : ", ") + world.first + " " + std::to_string(world.second);

    int booleans = 0;
    int values = 0;
    int constants = 0;
    int ambiguous = 0;
    for (const ShaderOption& option : mOptions.Options())
    {
        if (option.kind == ShaderOption::Boolean)
            ++booleans;
        else if (option.kind == ShaderOption::Value)
            ++values;
        else
            ++constants;
        if (option.ambiguous)
            ++ambiguous;
    }
    std::string active = mProperties.ActiveProfile(mOptions);
    LOG_INFO(sLog, "Shader pack %s: program files %s", mPack->Name().c_str(), programs.c_str());
    LOG_INFO(sLog, "Options: %d on/off, %d value, %d const, %d ambiguous; %zu screens, %zu profiles (current: %s); "
                   "%zu lang entries",
             booleans, values, constants, ambiguous, mProperties.Screens().size(), mProperties.Profiles().size(),
             active.empty() ? "Custom" : active.c_str(), mLang.Size());
    for (const std::string& error : mErrors)
        LOG_WARN(sLog, "%s", error.c_str());
}
