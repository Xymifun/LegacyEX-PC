#ifndef GAME_DATA_DATAFIXERUPPER_HPP
#define GAME_DATA_DATAFIXERUPPER_HPP

#include <vector>

#include <boost/unordered_map.hpp>

#include "game/data/CompoundTag.hpp"
#include "game/data/DataFixTypes.hpp"
#include "game/data/DataFixer.hpp"

class DataWalker;
class DataFix;

class DataFixerUpper : public DataFixer
{
public:
    DataFixerUpper(int version);
    ~DataFixerUpper();

    CompoundTag* updateData(DataFixTypes::EDataFixTypes type, CompoundTag* tag);
    virtual CompoundTag* update(DataFixTypes::EDataFixTypes type, CompoundTag* tag, int version);

    void addWalker(DataFixTypes::EDataFixTypes type, DataWalker* walker);
    void addFixer(DataFixTypes::EDataFixTypes type, DataFix* fix);

private:
    typedef boost::unordered_map<DataFixTypes::EDataFixTypes, std::vector<DataWalker*>*> WalkerMap;
    typedef boost::unordered_map<DataFixTypes::EDataFixTypes, std::vector<DataFix*>*> FixMap;

    CompoundTag* applyFixes(DataFixTypes::EDataFixTypes type, CompoundTag* tag, int version);
    CompoundTag* applyWalkers(DataFixTypes::EDataFixTypes type, CompoundTag* tag, int version);

    std::vector<DataWalker*>* getOrCreateWalker(DataFixTypes::EDataFixTypes type);
    std::vector<DataFix*>* getOrCreateFix(DataFixTypes::EDataFixTypes type);

    int mVersion;
    WalkerMap mWalkers;
    FixMap mFixes;
};

#endif
