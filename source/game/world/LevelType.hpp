#ifndef GAME_WORLD_LEVELTYPE_HPP
#define GAME_WORLD_LEVELTYPE_HPP

#include <string>

class LevelType
{
public:
    static LevelType* DEFAULT;
    static LevelType* FLAT;
    static LevelType* LARGE_BIOMES;
    static LevelType* AMPLIFIED;
    static LevelType* CUSTOMIZED;
    static LevelType* DEFAULT_1_1;
    static void staticCtor();
    static LevelType* getLevelType(const std::wstring& generatorName);
    LevelType(int id, const std::wstring& generatorName, int version);
    LevelType(int id, const std::wstring& generatorName);
    int getId();
    std::wstring getGeneratorName();
    std::wstring getDescriptionId();
    int getVersion();
    LevelType* getReplacementForVersion(int version);
    LevelType* setHasReplacement();
    bool hasReplacement();
    LevelType* setHasHelpText();
    bool hasHelpText();
    LevelType* setSelectableByUser(bool selectableByUser);
    bool isSelectable();

private:
    static LevelType* levelTypes[16];
    void _init(int id, const std::wstring& generatorName, int version);
    int mId;
    std::wstring mGeneratorName;
    int mVersion;
    bool mSelectableByUser;
    bool mHasReplacement;
    bool mHasHelpText;
};

#endif
