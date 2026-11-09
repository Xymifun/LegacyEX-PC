#ifndef GAME_DATA_DATAFIXERS_HPP
#define GAME_DATA_DATAFIXERS_HPP

#include <string>

class CompoundTag;
class DataFixer;
class DataFixerUpper;

class DataFixers
{
public:
    static DataFixerUpper* createFixerUpper();

    static CompoundTag* updateItem(DataFixer* fixer, CompoundTag* tag, int version, const std::wstring& name);
    static CompoundTag* updateItems(DataFixer* fixer, CompoundTag* tag, int version, const std::wstring& name);

private:
    static void addFixers(DataFixerUpper* fixerUpper);
};

#endif
