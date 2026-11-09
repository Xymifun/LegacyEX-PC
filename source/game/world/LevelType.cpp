#include "game/world/LevelType.hpp"

LevelType* LevelType::DEFAULT;
LevelType* LevelType::FLAT;
LevelType* LevelType::LARGE_BIOMES;
LevelType* LevelType::AMPLIFIED;
LevelType* LevelType::CUSTOMIZED;
LevelType* LevelType::DEFAULT_1_1;
LevelType* LevelType::levelTypes[16];

LevelType* LevelType::setHasReplacement()
{
    mHasReplacement = true;
    return this;
}

void LevelType::_init(int id, const std::wstring& generatorName, int version)
{
    mId = id;
    mHasHelpText = false;
    mGeneratorName.assign(generatorName);
    mVersion = version;
    mSelectableByUser = true;
    levelTypes[id] = this;
}

LevelType::LevelType(int id, const std::wstring& generatorName, int version)
{
    _init(id, generatorName, version);
}

LevelType::LevelType(int id, const std::wstring& generatorName)
{
    _init(id, generatorName, 0);
}

LevelType* LevelType::setHasHelpText()
{
    mHasHelpText = true;
    return this;
}

LevelType* LevelType::setSelectableByUser(bool selectableByUser)
{
    mSelectableByUser = selectableByUser;
    return this;
}

void LevelType::staticCtor()
{
    for (int i = 0; i < 16; ++i)
    {
        levelTypes[i] = NULL;
    }
    DEFAULT = (new LevelType(0, L"default", 1))->setHasReplacement();
    FLAT = new LevelType(1, L"flat");
    LARGE_BIOMES = new LevelType(2, L"largeBiomes");
    AMPLIFIED = (new LevelType(3, L"amplified"))->setHasHelpText();
    CUSTOMIZED = new LevelType(4, L"customized");
    DEFAULT_1_1 = (new LevelType(8, L"default_1_1", 0))->setSelectableByUser(false);
}

std::wstring LevelType::getGeneratorName()
{
    return mGeneratorName;
}

std::wstring LevelType::getDescriptionId()
{
    return std::wstring(L"generator.") + mGeneratorName;
}

int LevelType::getVersion()
{
    return mVersion;
}

LevelType* LevelType::getReplacementForVersion(int version)
{
    if (this == DEFAULT && version == 0)
    {
        return DEFAULT_1_1;
    }
    return this;
}

bool LevelType::hasReplacement()
{
    return mHasReplacement;
}

bool LevelType::isSelectable()
{
    return mSelectableByUser;
}

bool LevelType::hasHelpText()
{
    return mHasHelpText;
}

int LevelType::getId()
{
    return mId;
}

LevelType* LevelType::getLevelType(const std::wstring& generatorName)
{
    int i;
    if (generatorName.length() == 0)
    {
        return NULL;
    }
    for (i = 0; i < 16; ++i)
    {
        std::wstring name;
        name.assign(levelTypes[i]->mGeneratorName, 0, std::wstring::npos);
        if (levelTypes[i] != NULL && name.compare(generatorName) == 0)
        {
            return levelTypes[i];
        }
    }
    return NULL;
}
