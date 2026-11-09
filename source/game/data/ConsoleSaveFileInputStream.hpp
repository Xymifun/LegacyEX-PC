#ifndef GAME_DATA_CONSOLESAVEFILEINPUTSTREAM_HPP
#define GAME_DATA_CONSOLESAVEFILEINPUTSTREAM_HPP

#include "game/core/FileEntry.hpp"
#include "game/core/InputStream.hpp"
#include "game/data/ConsoleSaveFile.hpp"
#include "game/world/biome/arrayWithLength.hpp"

class ConsoleSaveFileInputStream : public InputStream
{
public:
    ConsoleSaveFileInputStream(ConsoleSaveFile* saveFile, const ConsoleSavePath& path);
    ConsoleSaveFileInputStream(ConsoleSaveFile* saveFile, FileEntry* fileEntry);
    virtual ~ConsoleSaveFileInputStream() {}

    virtual int read();
    virtual int read(arrayWithLength<unsigned char> data);
    virtual int read(arrayWithLength<unsigned char> data, unsigned int offset, unsigned int length);
    virtual void close();
    virtual long long skip(long long count);

private:
    ConsoleSaveFile* mSaveFile;
    FileEntry* mFileEntry;
};

#endif
