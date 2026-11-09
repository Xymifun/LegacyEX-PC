#ifndef GAME_DATA_MCREGIONLEVELSTORAGESOURCE_HPP
#define GAME_DATA_MCREGIONLEVELSTORAGESOURCE_HPP

#include <string>
#include <vector>

#include <boost/shared_ptr.hpp>

#include "game/data/DirectoryLevelStorageSource.hpp"

class ConsoleSaveFile;
class DataFixerUpper;
class File;
class LevelStorage;
class LevelSummary;
class ProgressListener;

class McRegionLevelStorageSource : public DirectoryLevelStorageSource
{
public:
    class ChunkFile;

    McRegionLevelStorageSource(File file, DataFixerUpper* fixer);

    virtual std::wstring getName();
    virtual boost::shared_ptr<LevelStorage> selectLevel(ConsoleSaveFile* saveFile, const std::wstring& levelId,
                                                        bool bCreateIfMissing);
    virtual std::vector<LevelSummary*>* getLevelList();
    virtual void clearAll();
    virtual bool isConvertible(ConsoleSaveFile* saveFile, const std::wstring& levelId);
    virtual bool requiresConversion(ConsoleSaveFile* saveFile, const std::wstring& levelId);
    virtual bool convertLevel(ConsoleSaveFile* saveFile, const std::wstring& levelId,
                              ProgressListener* progressListener);

private:
    void eraseFolders(std::vector<File*>* folders, int current, int total, ProgressListener* progressListener);
    void convertRegions(File& regionFile, std::vector<ChunkFile*>* chunks, int current, int total,
                        ProgressListener* progressListener);
};

#endif
