#ifndef GAME_CORE_FILEINPUTSTREAM_HPP
#define GAME_CORE_FILEINPUTSTREAM_HPP

#include "game/core/InputStream.hpp"
#include "game/world/biome/arrayWithLength.hpp"

class File;

class FileInputStream : public InputStream
{
public:
    FileInputStream(const File& file);
    virtual ~FileInputStream();

    virtual int read();
    virtual int read(arrayWithLength<unsigned char> data);
    virtual int read(arrayWithLength<unsigned char> data, unsigned int offset, unsigned int length);
    virtual void close();
    virtual long long skip(long long count);

    unsigned int filesize();

private:
    int mFile;
    bool mEndOfFile;
};

#endif
