#ifndef GAME_CORE_ACHIEVEMENT_HPP
#define GAME_CORE_ACHIEVEMENT_HPP

#include <string>

#include "game/core/Stat.hpp"
#include "game/item/not_null_ptr.hpp"

class Block;
class DescFormatter
{
public:
    virtual std::wstring format(const std::wstring& description) = 0;
};
class Item;
class ItemInstance;

class Achievement : public Stat
{
public:
    Achievement(int id, const std::wstring& name, int displayColumn, int displayRow, Item* item, Achievement* parent);
    Achievement(int id, const std::wstring& name, int displayColumn, int displayRow, Block* block, Achievement* parent);
    Achievement(int id, const std::wstring& name, int displayColumn, int displayRow, not_null_ptr<ItemInstance> icon,
                Achievement* parent);

    virtual Achievement* postConstruct(bool addToList);
    virtual bool isAchievement();

    Achievement* setExtended();

    void setGolden();
    bool isGolden();

    Achievement* setAwardLocallyOnly();

    void setDescFormatter(DescFormatter* descFormatter);

    std::wstring getDescription();
    int getAchievementID();

protected:
    void _init();

    int displayColumn;
    int displayRow;
    Achievement* parent;
    std::wstring description;
    DescFormatter* descFormatter;
    not_null_ptr<ItemInstance> icon;
    bool golden;
};

#endif
