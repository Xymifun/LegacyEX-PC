#ifndef GAME_DATA_CONSOLESAVEFILECONVERTER_HPP
#define GAME_DATA_CONSOLESAVEFILECONVERTER_HPP

#include "game/core/File.hpp"

class ConsoleSaveFile;
class FileEntry;
class ProgressListener;

class ConsoleSaveFileConverter
{
public:
    static void ConvertSave(ConsoleSaveFile* saveFileIn, ConsoleSaveFile* saveFileOut,
                            ProgressListener* progressListener);

private:
    static void ProcessSimpleFile(ConsoleSaveFile* saveFileIn, FileEntry* entryIn, ConsoleSaveFile* saveFileOut,
                                  FileEntry* entryOut);
    static void ProcessStandardRegionFile(ConsoleSaveFile* saveFileIn, File regionFileIn, ConsoleSaveFile* saveFileOut,
                                          File regionFileOut);
};

#endif
