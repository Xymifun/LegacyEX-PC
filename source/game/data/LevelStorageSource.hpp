#ifndef GAME_DATA_LEVELSTORAGESOURCE_HPP
#define GAME_DATA_LEVELSTORAGESOURCE_HPP

#include <string>
#include <vector>

#include <boost/shared_ptr.hpp>

class ConsoleSaveFile;
class LevelData;
class LevelStorage;
class LevelSummary;
class ProgressListener;

class LevelStorageSource
{
public:
    virtual ~LevelStorageSource() {}

    virtual std::wstring getName() = 0;
    virtual boost::shared_ptr<LevelStorage> selectLevel(ConsoleSaveFile* saveFile, const std::wstring& levelId,
                                                        bool bCreateIfMissing) = 0;
    virtual std::vector<LevelSummary*>* getLevelList() = 0;
    virtual void clearAll() = 0;
    virtual LevelData* getDataTagFor(ConsoleSaveFile* saveFile, const std::wstring& levelId) = 0;
    virtual bool isNewLevelIdAcceptable(const std::wstring& levelId) = 0;
    virtual void deleteLevel(const std::wstring& levelId) = 0;
    virtual void renameLevel(const std::wstring& levelId, const std::wstring& levelName) = 0;
    virtual bool isConvertible(ConsoleSaveFile* saveFile, const std::wstring& levelId) = 0;
    virtual bool requiresConversion(ConsoleSaveFile* saveFile, const std::wstring& levelId) = 0;
    virtual bool convertLevel(ConsoleSaveFile* saveFile, const std::wstring& levelId,
                              ProgressListener* progressListener) = 0;
};

#endif
