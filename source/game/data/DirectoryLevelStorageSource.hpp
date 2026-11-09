#ifndef GAME_DATA_DIRECTORYLEVELSTORAGESOURCE_HPP
#define GAME_DATA_DIRECTORYLEVELSTORAGESOURCE_HPP

#include <string>
#include <vector>

#include <boost/shared_ptr.hpp>

#include "game/core/File.hpp"
#include "game/data/LevelStorageSource.hpp"

class CompoundTag;
class ConsoleSaveFile;
class ConsoleSavePath;
class DataFixerUpper;
class LevelData;
class LevelStorage;
class LevelSummary;
class ProgressListener;

class DirectoryLevelStorageSource : public LevelStorageSource
{
public:
    DirectoryLevelStorageSource(File file, DataFixerUpper* fixer);
    virtual ~DirectoryLevelStorageSource() {}

    static LevelData* getLevelData(ConsoleSaveFile* saveFile, const ConsoleSavePath& path, DataFixerUpper* fixer);

    virtual std::wstring getName();
    virtual boost::shared_ptr<LevelStorage> selectLevel(ConsoleSaveFile* saveFile, const std::wstring& levelId,
                                                        bool bCreateIfMissing);
    virtual std::vector<LevelSummary*>* getLevelList();
    virtual void clearAll();
    virtual LevelData* getDataTagFor(ConsoleSaveFile* saveFile, const std::wstring& levelId);
    virtual bool isNewLevelIdAcceptable(const std::wstring& levelId);
    virtual void deleteLevel(const std::wstring& levelId);
    virtual void renameLevel(const std::wstring& levelId, const std::wstring& levelName);
    virtual bool isConvertible(ConsoleSaveFile* saveFile, const std::wstring& levelId);
    virtual bool requiresConversion(ConsoleSaveFile* saveFile, const std::wstring& levelId);
    virtual bool convertLevel(ConsoleSaveFile* saveFile, const std::wstring& levelId,
                              ProgressListener* progressListener);

protected:
    static void deleteRecursive(std::vector<File*>* files);

    File mBaseDir;
    DataFixerUpper* mFixer;
};

#endif
